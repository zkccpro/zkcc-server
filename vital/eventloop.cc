#include"eventloop.h"
thread_local EventLoop* EventLoop::cur_loop_=nullptr;

EventLoop::EventLoop():
tid_(std::this_thread::get_id()){
    assertMultiInst();
    cur_loop_=this;
    ZLOG_INFO<<"loop is constructing in zthread";
}

EventLoop::EventLoop(zkcc::ThreadPool::Ptr_t pool,size_t max_ready):
tid_(std::this_thread::get_id()),thread_pool_(pool),
poller_(new Poller(max_ready)){
    assertMultiInst();
    cur_loop_=this;
    ZLOG_INFO<<"loop is constructing in zthread";
}

//没用断言，c++静态断言只能用于编译期，只接受一个常量表达式
//c语言的断言宏不够灵活，只是判断一下表达式是否成立，不成立就退出
//想实现你这个需求用try-catch会比较好，比断言宏更灵活
void EventLoop::assertMultiInst(){
    try
    {
        if(cur_loop_)//当前线程已经有loop了，程序崩掉
        {
            ZLOG_CRIT<<"multi loop in one thread! abort";
            throw "multi loop in one thread! abort";
        }
        else return;
    }
    catch(int nocatch){}
}

void EventLoop::assertRunInDiffThread(){
    try
    {
        if(tid_!=std::this_thread::get_id())//loop没用跑在创建的线程中，程序崩掉
        {
            ZLOG_CRIT<<"loop is in another thread! abort";
            throw "loop is in another thread! abort";
        }
        else return;
    }
    catch(int nocatch){}
}

void EventLoop::loop(){
    assertRunInDiffThread();
    if(!is_run_.load()) is_run_.store(true);
    ZLOG_INFO<<"start looping in zthread "<<zkcc::Thread::GetNameBySelf()<<"epollfd="<<poller_->fd();
    while(is_run_.load()){
        poller_->wait(ready_events_);
        for(auto ready:ready_events_){
            ready->onEvent(thread_pool_);

        }
    }
    ZLOG_INFO<<"loop stop";
}