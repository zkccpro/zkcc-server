#pragma once
#include<netinet/in.h>
#include<string.h>
//#include"/home/zkcc/zkcc/log/zlog.h"
#include"log/zlog.h"
class Address{
public:
    Address()=default;
    Address(in_addr_t addr,in_port_t port,int protocol=AF_INET);
    ~Address()=default;
    void setSrcAddr(in_addr_t addr=INADDR_ANY,in_port_t port=80,int protocol=AF_INET);//创建服务端地址
    void setDstAddr(in_addr_t addr,in_port_t port=80,int protocol=AF_INET);//创建客户端地址
    const sockaddr_in* addr()const{return &addr_;}
    size_t size(){return sizeof(addr_);}
    in_addr_t address(){return addr_.sin_addr.s_addr;}
    in_port_t port(){return addr_.sin_port;}
private:
    void setAddr(in_addr_t addr,in_port_t port,int protocol);
private:
    sockaddr_in addr_;
};