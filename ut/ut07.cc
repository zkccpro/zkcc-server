/* for compile: 
g++ -pthread -g log/zlog.cc base/thread.cc base/signal.cc base/threadpool.cc vital/buffer.cc vital/fd.cc vital/address.cc vital/poller.cc vital/event.cc vital/eventloop.cc vital/acceptor.cc vital/connection.cc vital/server.cc ut/ut07.cc -I/home/zkcc/zkcc/ -o ut/ut07 && ./ut/ut07
*/
/*
review 2021.8.25-15:16 PM:
客户端试验正常，下面的程序可以正常连接上服务端
*/
/*
issue 2021.8.25-15:16 PM:
1. TcpFd创建客户端fd时的接口设计不够灵活，应给出修改
2. 写client类时要注意考虑读写服务端的数据，客户端暂不考虑多线程，读写函数应该相对容易处理一些
*/
#include"vital/server.h"
#include<arpa/inet.h>
int main(){
    ZLog::initialize("/home/zkcc/zkcc/log/logc", 10);
    //TcpFd client(INADDR_ANY,40000,client_FD);
    //Address server_addr(inet_addr("172.25.213.103"),50000);

    int fd=socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,0);
    sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));//好像得加这个？
    server_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    server_addr.sin_port=htons(50000);
    server_addr.sin_family=AF_INET;
    //std::cout<<client.fd();

    while(connect(fd,(sockaddr*)&server_addr,sizeof(server_addr))) ;

        ZLOG_INFO<<"connect success!";
        const char* msg ="here we connect! client";
        //::write(fd,msg,24);
        char* tmp_data=new char[0];
        int read_s=::read(fd,tmp_data,10);

}