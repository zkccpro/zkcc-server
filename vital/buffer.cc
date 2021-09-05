#include"buffer.h"
#include<iostream>
std::pair<std::string,int> Buffer::deliver(int fd){
    std::lock_guard<std::mutex> guard(mtx_);
    int recv_ret=recv(fd);
    if(recv_ret==-1) return {{},-1};//头信息接受不全
    if(recv_ret==-2) return {{},-2};//什么也没收到，认为对端发送EOF
    if(get_index_-put_index_<msg_len_) return {{},0};//头信息接受完全，但消息没接收完全
    size_t put_tmp=put_index_;
    size_t len=msg_len_;
    put_index_+=msg_len_;
    msg_len_=0;
    recv_len_=0;
    ZLOG_INFO<<"deliver  "<<"put_index:(left)"<<(int)put_index_<<" ""get_index:(right)"<<(int)get_index_;
    return {{buffer_.begin()+put_tmp,buffer_.begin()+put_tmp+len},0};//消息接受完全
}


int Buffer::recv(int fd){
    if(msg_len_==0)
    {
        char* tmp_data=new char[recv_head_len_];
        int head_s=::read(fd,tmp_data,recv_head_len_);
        if(head_s==0) return -2;//头信息这里就什么也没收到，认为对端断开连接了
        for(int i=0;i<head_s;i++) header_.push_back(tmp_data[i]);
        if(header_.size()>=header_len_)
        {
            try
            {
                msg_len_=std::stoul(header_);
            }
            catch(const std::exception& e)
            {
                msg_len_=0;
                std::cerr << e.what() << '\n';
            }
            recv_len_=msg_len_;
            header_.clear();
            recv_head_len_=header_len_;
        }
        else
        {
            recv_head_len_-=head_s;
        }
        delete[] tmp_data;
    }
    ZLOG_INFO<<"recv len= "<<(int)recv_len_;
    if(recv_len_==0) return -1;//头部还没完全收到
    char* tmp_data=new char[recv_len_];
    int read_s=::read(fd,tmp_data,recv_len_);
    
    for(int i=0;i<read_s;i++){
        buffer_.push_back(tmp_data[i]);
    }
    recv_len_-=read_s;
    get_index_=buffer_.size();
    delete[] tmp_data;
    ZLOG_INFO<<"recv  "<<"put_index:(left)"<<(int)put_index_<<" ""get_index:(right)"<<(int)get_index_<<" should recv len: "<<(int)msg_len_;
    return 0;
}

void Buffer::cache(Message msg, int fd){
    std::lock_guard<std::mutex> guard(mtx_);
    send_fd_=fd;
    //报文长度
    std::string head=std::to_string(msg.length);
    //头部占位
    std::string place_holder=std::string(10-head.size(),' ');
    //head+body
    msg.data=head+place_holder+msg.data;
    for(int i=0;i<msg.data.size();i++)
        buffer_.push_back(msg.data[i]);
    get_index_=buffer_.size();
    ZLOG_INFO<<"cache  "<<"put_index:(left)"<<(int)put_index_<<" ""get_index:(right)"<<(int)get_index_;
}

void Buffer::send(int fd){
    std::lock_guard<std::mutex> guard(mtx_);
    if(send_fd_==-1) return;
    size_t send_s=this->size();
    if(send_s==0) return;
    char* tmp_send=new char[send_s];
    for(int i=0;i<send_s;i++)
        tmp_send[i]=buffer_[put_index_+i];
    int cur_write=::write(send_fd_,tmp_send,send_s);
    put_index_+=cur_write;
    ZLOG_INFO<<"send  "<<"put_index:(left)"<<(int)put_index_<<" ""get_index:(right)"<<(int)get_index_;
    delete[] tmp_send;
}