#pragma once
#include<vector>
#include<map>
#include<memory>
#include"event.h"
#include"fd.h"
#include"base/base.h"

/*@ never use across threads */
class Poller: NO_COPY {
public:
    using Unique_t = std::unique_ptr<Poller>;
    using EventVec_t= std::vector<Event::Ptr_t>;

    Poller()=default;
    //ready_epoll_events_的大小，同一时刻最多返回多少准备好的事件
    Poller(size_t max_ready_events):
    ready_epoll_events_(new epoll_event[max_ready_events]),
    max_ready_(max_ready_events){}
    ~Poller(){
        delete[] ready_epoll_events_;
        ready_epoll_events_=nullptr;
    }
    void setMaxReady(size_t max_ready_events){
        delete[] ready_epoll_events_;
        ready_epoll_events_=nullptr;
        ready_epoll_events_=new epoll_event[max_ready_events];
        max_ready_=max_ready_events;
    }
    //创建epoll
    void createPoll(){efd_.create();}
    //epoll_wait循环在此，更新当前准备好的事件
    void wait(EventVec_t& ready_events);
    //如果已经有事件了就更新，否则就添加，上层调用这个接口更方便
    void appendEvent(Event* event);
    //向poller添加事件
    void addEvent(Event* event);
    //向poller更新事件
    void updateEvent(Event* event);
    //向poller删除事件
    void delEvent(Event* event);
    
    int fd(){return efd_.fd();}
private:
    //控制epoll，用于封装增删改事件的逻辑
    void controlPoll(epoll_event event_type, int control_method);
private:
    //epoll运行时所监视的所有事件，到时候epoll_wait准备好了返回一个fd
    //需要在这里快速根据fd查找事件封装
    //Poller不掌握event生命周期的控制权，而应该交给Acceptor或Connection掌握
    std::map<int,Event::Ptr_t> events_;
    EpollFd efd_{};//createPoll之后产生的epollfd
    int max_ready_{10};
    //epoll_wait会把当前时刻准备好的epoll_event放在这里（一个堆区数组）
    //这里必须用原始指针，不能用智能指针，否则epoll_wait疯狂返回-1，所以必须手动管理内存，需额外小心
    epoll_event* ready_epoll_events_{new epoll_event[max_ready_]};
};