#pragma once
#include<memory>
#include<mutex>
#include<sys/epoll.h>
#include<sys/timerfd.h>
#include"address.h"
#include"buffer.h"
#include"base/base.h"
#include<iostream>
#include<sys/fcntl.h>
enum TcpType{
    client_FD,//客户端socket
    server_FD//服务端socket
};

enum TimeSet{
    NANO_SEC,//毫秒
    SEC//秒
};

//TCP套接字文件（监听描述符+连接套接字）
class TcpFd: NO_COPY{
public:
    using Shared_t = std::shared_ptr<TcpFd>;
    using Unique_t = std::unique_ptr<TcpFd>;
    using Ptr_t = TcpFd*;
    TcpFd();//默认建立一个服务端socket,port=50000
    //设置listenfd地址端口+bind listenfd
    TcpFd(in_addr_t addr,in_port_t port,TcpType type,int protocol=AF_INET);
    TcpFd(int fd,Address src,Address dst){
        src_addr_=src;
        dst_addr_=dst;
        fd_=fd;
    }
    ~TcpFd();
    /*@ thread safe with lock guard*/
    int readBuffer(std::string& data);
    /*@ thread safe with lock guard*/
    int writeBuffer(const std::string& msg);

    Ptr_t acceptConn();//接受连接
    int connect();
    Address src_addr(){return src_addr_;}
    Address dst_addr(){return dst_addr_;}
    int fd(){return fd_;}

private:
    TcpType type_{TcpType::server_FD};
    Address src_addr_{INADDR_ANY,50000,AF_INET};
    Address dst_addr_{};
    int src_port{50000};
    int dst_port{0};
    int fd_{-1};
public:
    //上层视角的命名
    Buffer read_buffer_;//读缓冲（接收缓存），从内核向上接收的缓冲
    Buffer write_buffer_;//写缓冲（发送缓存），从上层往内核发送的缓冲
};
//timerfd文件
class TimerFd:NO_COPY{
public:
    using Shared_t = std::shared_ptr<TimerFd>;
    using Unique_t = std::unique_ptr<TimerFd>;
    using Ptr_t = TimerFd*;
    TimerFd(){fd_=timerfd_create(CLOCK_MONOTONIC,0);}
    void set_timer(int first_overtime,int period, TimeSet set=TimeSet::NANO_SEC);
    int fd(){return fd_;}
private:
    int fd_{-1};
};
//epoll文件描述符
class EpollFd:NO_COPY{
public:
    using Shared_t = std::shared_ptr<EpollFd>;
    using Unique_t = std::unique_ptr<EpollFd>;
    using Ptr_t = EpollFd*;
    EpollFd(){fd_=epoll_create(1);}
    void create(){fd_=epoll_create(1);}
    int fd(){return fd_;}
private:
    int fd_{-1};
};