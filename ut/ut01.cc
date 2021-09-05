/* for compile: 
g++ -pthread -g log/zlog.cc vital/buffer.cc vital/fd.cc vital/address.cc ut/ut01.cc -I/home/zkcc/zkcc/ -o ut/ut01 && ./ut/ut01
*/

/*review:
连接和发送没啥d大问题（阻塞情况抓包发送消息正常；非阻塞程序能跑通），接受还没测，感觉没有客户端不太好测接受情况，以后再说吧
在buffer没考虑空间清理的情况下做的测试
*/
#include"vital/fd.h"
int main(){
    ZLog::initialize("/home/zkcc/zkcc/log/log", 10);
    //Fd* timer=new TimerFd;
    //std::string str;
    //子类就算没有重写父类的虚函数，子类的虚表中也会有一份父类的虚函数
    //timer->readBuffer(str);

    TcpFd* listenfd=new TcpFd;
    listen(listenfd->fd(),1024);
    ZLOG_INFO<<"listenfd is ready!";
    //设置成非阻塞的listenfd和connfd，需要配合epoll用效率才更高！
    TcpFd::Ptr_t conn= listenfd->acceptConn();
    ZLOG_INFO<<"conn is ready! fd= "<<conn->fd();
    std::string data= "HTTP/1.1 200 OK\r\nServer: Reage webserver\r\nContent-Type: text/html\r\nConnection: Keep-Alive\r\nConnection-Length: %d\r\n\r\n<html><head><title>This is network programming</title></head><body><h1>%s</h1></body></html>\r\n\r\n";
    conn->writeBuffer(data);
    conn->write_buffer_.send(conn->fd());
}