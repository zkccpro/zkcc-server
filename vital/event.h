#pragma once
#include"fd.h"
#include"base/threadpool.h"
#include<functional>
#include<sys/epoll.h>
#include<unordered_map>
#include<vector>
#include<base/base.h>

//封装一下所有需要的epoll事件
enum PollEvent{
    POLL_NULL = 0x000,//啥也没有的事件，这样设置方便和其他事件去或
    POLL_IN = EPOLLIN,
    POLL_OUT = EPOLLOUT,
    POLL_ERR = EPOLLERR,
    POLL_CLOSE = EPOLLRDHUP,
    POLL_ET = EPOLLET,
    CTL_ADD = EPOLL_CTL_ADD,
    CTL_DEL = EPOLL_CTL_DEL,
    CTL_UPD = EPOLL_CTL_MOD
};

/*@ may use across threads! */
class Event :NO_COPY{
public:
    using Unique_t = std::unique_ptr<Event>;
    using Ptr_t = Event*;
    Event():tcp_(new TcpFd),timer_(new TimerFd){initMap();}
    Event(TcpFd* fd):tcp_(fd),timer_(new TimerFd){initMap();}
    Event(TimerFd* fd):tcp_(new TcpFd),timer_(fd){initMap();}
    ~Event()=default;

    //当一个事件经过epoll监听准备好后，调用此函数执行其对应的回调
    //暂时设计成单线程依次执行准备好的事件，之后这里要考虑多线程的 2021.8.10
    //考虑了多线程，传入一个线程池指针，以此将任务分发到各个线程种执行 2021.8.18
    void onEvent(zkcc::ThreadPool::Ptr_t tp);
    //注册感兴趣的事件，事件到达时回调
    //传入参数必须是Args&& 如果不是的话，传入一个引用对象，会被推导成值
    /*@ guard with mutex */
    template<typename Func>
    void setEventCallBack(PollEvent event,Func&& cb){
        std::lock_guard<std::mutex> lock(mtx_);
        auto task=std::forward<Func>(cb);
        event_callback_[event].first=task;
        event_callback_[event].second=true;
    }
    //取消关注一个事件
    /*@ guard with mutex */
    void cancelEventCallBack(PollEvent event){
        std::lock_guard<std::mutex> lock(mtx_);
        event_callback_[event].first=null_;
        event_callback_[event].second=false;
    }

    void set_tcp(TcpFd* fd){tcp_.reset(fd);}
    TcpFd* tcpfd(){return tcp_.get();}
    void set_timer(TimerFd* fd){timer_.reset(fd);}
    TimerFd* timerfd(){return timer_.get();}
    int Tcpfd(){return tcp_==nullptr ? -1 : tcp_->fd();}
    int Timerfd(){return timer_==nullptr ? -1 : timer_->fd();}

    //返回该Event关注的epoll事件
    epoll_event events();
    //Poller来设置当前准备好的事件
    /*@ guard with mutex */
    void set_ready_event(const epoll_event& event);

private:
    void initMap();
private:
    std::unordered_map<PollEvent,std::pair<CallBack_t,bool>> event_callback_;
    //在这里用unique管理两个fd是因为在event里掌握fd的声明周期，Fd类都禁止拷贝必须用指针
    TcpFd::Unique_t tcp_;
    TimerFd::Unique_t timer_;
    std::vector<PollEvent> ready_event_;//当前准备好的事件类型
    std::mutex mtx_;
};