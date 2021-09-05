#pragma once
#include<vector>
#include<string>
#include<mutex>
#include<memory>
#include <unistd.h>
#include"log/zlog.h"
struct Message{
    size_t length;
    std::string data;
};
/*@4个操作Buffer的函数都是在各种回调中执行的*/
/*@暂未考虑Buffer空间清理的问题！*/
class Buffer{
public:
    Buffer()=default;
    ~Buffer()=default;
    size_t size()const{return get_index_-put_index_;}
    bool isEmpty()const{return get_index_==put_index_;}
    size_t get_index()const{return get_index_;}
    size_t put_index()const{return put_index_;}
    /*@ thread safe with lock guard*/
    void cache(Message msg,int fd);//上层向buffer缓存数据;nonblock
    /*@ thread safe with lock guard*/
    std::pair<std::string,int> deliver(int fd);//buffer向上层递交数据;nonblock

public:
    /*@ thread safe with lock guard*/
    void send(int fd);//将buffer中数据传递给内核，发送到网络;nonblock
    int recv(int fd);//从内核接收数据，注意处理粘包;nonblock

private:
    std::vector<char> buffer_;
    //buffer视角的命名，递交和接受的对象既可以是上层，也可以是内核
    size_t get_index_{0};//接受指针，指向此次接受的最后一字节之后的位置(right)
    size_t put_index_{0};//递交指针，指向本次尚未交付的第一字节的位置(left)
    std::mutex mtx_;
    
private://deliver和recv用
    size_t header_len_{10};
    size_t recv_len_{0};//此次应该接受的报文长度
    size_t msg_len_{0};//此轮读取报文的长度
    std::string header_;//接受对端发来的头信息
    size_t recv_head_len_{header_len_};//此次应该接受的报文头长度
private://cache和send用
    int send_fd_{-1};//指定发送描述符
};