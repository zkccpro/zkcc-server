/* for compile: 
g++ -pthread -g log/zlog.cc base/thread.cc base/signal.cc base/threadpool.cc vital/buffer.cc vital/fd.cc vital/address.cc vital/poller.cc vital/event.cc vital/eventloop.cc ut/ut03.cc -I/home/zkcc/zkcc/ -o ut/ut03 && ./ut/ut03
*/

/*
review:
1. 测试了 阻止loop在不同线程中使用或在同一线程中使用多次的功能，表现正常
2. 测试了 单线程下使用loop进行单个事件分发，表现正常
3. 测试了 在另一个线程停止loop，表现正常
*/
#include"vital/eventloop.h"
#include"base/threadpool.h"

//EventLoop严禁设置为全局变量，有可能造成在不同线程loop的风险，而且其初始化早于日志初始化，会导致记录日志出错
//EventLoop g_lp;
void runThread(EventLoop* lp){//loop不允许拷贝
    //lp->loop();//不允许在别的线程创建loop而在此线程跑loop
}
void onCallBack(EventLoop& lp){
    ZLOG_INFO<<"on call back is running!";
    lp.stop();//不在另一个线程里跑loop就没事
}

void xxx(int a){}
int main(){
    ZLog::initialize("/home/zkcc/zkcc/log/log", 10);
    //这部分写到Acceptor中
    TcpFd* listenfd=new TcpFd;
    listen(listenfd->fd(),1024);
    ZLOG_INFO<<"listenfd is ready! fd="<<listenfd->fd();


    //EventLoop* lp1=new EventLoop;
    EventLoop lp1;
    int a;
    //这部分也写到Acceptor中
    Event* accept_event=new Event(listenfd);
    accept_event->setEventCallBack(POLL_IN,onCallBack,std::ref(lp1));//这里最好加上std::ref
    //对于可拷的类可以不加ref，对于不可拷的类，引用折叠+完美转发时一定要加ref，否则编译报错！！！
    accept_event->setEventCallBack(POLL_IN,xxx,a);
    lp1.addEvent(accept_event);
    //EventLoop* lp2=new EventLoop;一个线程不能出现两个loop，否则让程序崩溃


    lp1.loop();


    // zkcc::ThreadPool thp1(10);//唯一正确的做法是在本线程创建一个loop，然后跑这个loop
    // thp1.sign(runThread,lp1);
}
