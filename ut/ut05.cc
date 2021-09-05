/* for compile: 
g++ -pthread -g log/zlog.cc base/thread.cc base/signal.cc base/threadpool.cc vital/buffer.cc vital/fd.cc vital/address.cc vital/poller.cc vital/event.cc vital/eventloop.cc ut/ut05.cc -I/home/zkcc/zkcc/ -o ut/ut05 && ./ut/ut05
*/

/*
review 2021.8.18-11:31 AM:
主从reactor+工作线程，问题待解决，可能出现死锁
*/
/*
issue 2021.8.18-11:31 AM:
1. 注意处理连接出错情况！（Tcpfd.acceptConn）
2. 发现死锁
*/

/*
review 2021.8.18-22:42 PM:
主从reactor+工作线程，解决了智能指针提前清除threadpool单例内存的问题
处理了连接出错的情况
多次建立连接后，极小概率会出现程序崩溃（大概建立2k次连接崩溃一次？）崩溃原因暂时没找到
由于还没有考虑断开连接的问题，所以一直占用描述符，导致valgrind一直会检测出内存泄漏
*/
/*
issue 2021.8.18-22:42 PM:
1. 隐含bug暂未找到原因，留待以后解决
2. 处理占用描述符的问题
*/

/*
review 2021.8.19-7:36 AM:
主从reactor+工作线程，解决了智能指针提前清除threadpool单例内存的问题
多次建立连接后，极小概率会出现程序崩溃（大概建立2k次连接崩溃一次？）崩溃原因暂时没找到
不是因为每关掉fd导致内存泄漏，而是因为Event的内存管理没有用智能指针导致泄漏，而且Fd的内存交给Event内部的智能指针管理，不需智能指针了（否则重复释放）
线程池需要手动在上层手动delete才会正常结束关闭线程，不会有内存泄漏问题（delete线程池），但如果不delete会造成线程池一直在运行的情况
*/
/*
issue 2021.8.19-7:36 AM:
1. 隐含bug暂未找到原因，留待以后解决
2. 处理占用描述符的问题（不会导致内存泄漏，但仍需处理）
3. 可以在loop类里稍微封装一下关闭线程池的操作（delete），但其实这个操作没啥用。。正常来说服务器不会主动关闭线程池的，如果要主动终止服务也是ctrlc直接终止就行了
*/

#include"vital/eventloop.h"
#include"base/threadpool.h"
void onUserConnCallBack(EventLoop& lp,TcpFd::Ptr_t conn){
    //和onConnCallBack一个线程，在从reactor线程跑
    ZLOG_INFO<<"user conn call back is running! fd= "<<conn->fd();
}
//在工作线程里跑
void onMsgCallBack(EventLoop& lp){
    ZLOG_INFO<<"msg is comming! handle in zthread "<<zkcc::Thread::GetNameBySelf();
    lp.stop();
}
//写到Acceptor或者Connction里，在从reactor线程跑
void onConnCallBack(TcpFd* listen,EventLoop& lp1){
    //lp1.stop();
    ZLOG_INFO<<"on Conn call back is running in zthread "<<zkcc::Thread::GetNameBySelf();
    TcpFd::Ptr_t conn(listen->acceptConn());//注意处理连接出错情况！（connfd=-1）
    if(conn->fd()==-1)
    {
        ZLOG_CRIT<<"conn error!";
        return;
    }
    ZLOG_INFO<<"conn is ready! fd= "<<conn->fd();
    EventLoop lp;//从reactor线程单独的loop
    //新添加一个事件
    Event::Unique_t msg_event(new Event(conn));
    msg_event->setEventCallBack(POLL_IN,onMsgCallBack,std::ref(lp));
    lp.addEvent(msg_event.get());
    onUserConnCallBack(lp,conn);
    lp.loop();
}
int main(){
    ZLog::initialize("/home/zkcc/zkcc/log/log", 10);
    //这部分写到Acceptor中
    TcpFd::Ptr_t listenfd=new TcpFd;//TcpFd不能用智能指针管理了，否则重复释放
    listen(listenfd->fd(),1024);
    ZLOG_INFO<<"listenfd is ready! fd="<<listenfd->fd();


    //EventLoop* lp1=new EventLoop;
    EventLoop lp1;

    //这部分也写到Acceptor中
    Event::Unique_t accept_event(new Event(listenfd));//Fd的内存由Event负责，Fd不用智能指针了
    //主从reactor，必须保证这里的onConnCallBack跑在另一个线程，所以需要改loop,event类的代码
    accept_event->setEventCallBack(POLL_IN,onConnCallBack,listenfd,std::ref(lp1));//这里要加上std::ref
    lp1.addEvent(accept_event.get());
    
    lp1.loop();
    delete lp1.thread_pool();//这个单例的内存不清理也罢，但是不清理线程池不会结束运行的
}