/* for compile: 
g++ -pthread -g log/zlog.cc vital/buffer.cc vital/fd.cc vital/address.cc vital/poller.cc vital/event.cc ut/ut02.cc -I/home/zkcc/zkcc/ -o ut/ut02 && ./ut/ut02
*/

/*
review:
1. 单线程情况下测试poller-event体系能否正常执行用户回调，结果正确
2. valgrind监测整个测试过程未发现内存泄漏
*/
#include"vital/poller.h"
bool g_isrun=true;
void onCallBack(){
    ZLOG_INFO<<"on call back is running!";
    g_isrun=false;
}

int main(){
    ZLog::initialize("/home/zkcc/zkcc/log/log", 10);
    TcpFd* listenfd=new TcpFd;
    listen(listenfd->fd(),1024);
    ZLOG_INFO<<"listenfd is ready! fd="<<listenfd->fd();

    Poller poll;
    Event* accept_event=new Event(listenfd);
    accept_event->setEventCallBack(POLL_IN,onCallBack);
    poll.addEvent(accept_event);
    Poller::EventVec_t event_vec;
    ZLOG_INFO<<"start wait in thread xxx; "<<"epollfd="<<poll.fd();
    while(g_isrun){
        poll.wait(event_vec);
        for(auto ready:event_vec){
            //ready->onEvent();
        }
    }
    delete accept_event;
}