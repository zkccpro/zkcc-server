#pragma once
#include"poller.h"
#include"base/singleton.h"
#include<thread>
#include<atomic>
class EventLoop{
public:
    EventLoop();//默认ready数组大小为10
    EventLoop(zkcc::ThreadPool::Ptr_t pool,size_t max_ready=10);//指定ready数组大小
    ~EventLoop()=default;
    //用于判断当前线程是不是有多个EventLoop对象
    //一般情况下this指针不能交给智能指针管理（除非设置this指针是shared）
    static thread_local EventLoop* cur_loop_;
    std::thread::id tid_;//用于判断是否loop函数是否跑在EventLoop创建的线程里
    void loop();//实现事件等待和回调分发的核心函数
    //停止loop
    void stop(){is_run_.store(false);ZLOG_INFO<<"loop will stop";}

    void assertMultiInst();//断言在一个线程中是否有多个EventLoop实例
    void assertRunInDiffThread();//断言创建和loop是否在同一个线程
    //如果已经有事件了就更新，否则就添加，上层调用这个接口更方便
    void appendEvent(Event* event){poller_->appendEvent(event);}
    //向poller添加事件
    void addEvent(Event* event){poller_->addEvent(event);}
    //向poller添加事件
    void updateEvent(Event* event){poller_->updateEvent(event);}
    //向poller添加事件
    void delEvent(Event* event){poller_->delEvent(event);}

    zkcc::ThreadPool::Ptr_t thread_pool(){return thread_pool_;}
    void set_pool(zkcc::ThreadPool::Ptr_t pool){thread_pool_=pool;}
private:
    std::atomic<bool> is_run_{true};//当前是否在运行
    Poller::EventVec_t ready_events_;//数组存放当前准备好的事件(数组里是裸指针，poller和loop都不掌握event的生命)
    Poller::Unique_t poller_{new Poller};//使用poller来控制事件等待和回调分发
    //但这里一定不能用智能指针！否则智能指针会提前释放掉单例，造成程序崩溃
    //（这里的设计有问题，既然loop声明周期结束了都不能释放内存，那为什么要把threadpool放在loop里呢？？应该放在更上层类中）
    zkcc::ThreadPool::Ptr_t thread_pool_;//loop可以向线程池种分发IO-reactor和工作
};