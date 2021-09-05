/* for compile: 
g++ -pthread -g log/zlog.cc base/thread.cc base/signal.cc base/threadpool.cc vital/buffer.cc vital/fd.cc vital/address.cc vital/poller.cc vital/event.cc vital/eventloop.cc ut/ut04.cc -I/home/zkcc/zkcc/ -o ut/ut04 && ./ut/ut04
*/

/*review:
模拟单线程reactor的情况：先监听一个listenfd，连接到来时接受连接，
并新添加一个关注事件（可读），随后调用用户的回调；
当消息到来时，可以顺利触发指定的回调
*/
#include"vital/eventloop.h"
void onUserConnCallBack(EventLoop& lp,TcpFd* conn){
    ZLOG_INFO<<"user conn call back is running! fd= "<<conn->fd();
}
void onMsgCallBack(EventLoop& lp){
    ZLOG_INFO<<"msg is comming!";
    //lp.stop();
}
//写到Acceptor或者Connction里
void onConnCallBack(EventLoop& lp,TcpFd* listen){
    ZLOG_INFO<<"on Conn call back is running!";
    TcpFd::Ptr_t conn=listen->acceptConn();
    ZLOG_INFO<<"conn is ready! fd= "<<conn->fd();
    //新添加一个事件
    Event* msg_event=new Event(conn);
    msg_event->setEventCallBack(POLL_IN,onMsgCallBack,std::ref(lp));
    lp.addEvent(msg_event);
    onUserConnCallBack(lp,conn);
}

int main(){
    ZLog::initialize("/home/zkcc/zkcc/log/log", 10);
    //这部分写到Acceptor中
    TcpFd* listenfd=new TcpFd;
    listen(listenfd->fd(),1024);
    ZLOG_INFO<<"listenfd is ready! fd="<<listenfd->fd();


    //EventLoop* lp1=new EventLoop;
    EventLoop lp1;

    //这部分也写到Acceptor中
    Event* accept_event=new Event(listenfd);
    accept_event->setEventCallBack(POLL_IN,onConnCallBack,std::ref(lp1),listenfd);//这里要加上std::ref
    lp1.addEvent(accept_event);
    
    lp1.loop();

}