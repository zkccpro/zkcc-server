#include"fd.h"

TcpFd::TcpFd(){
    fd_=socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,0);
    int ret=bind(fd_,(sockaddr*)& src_addr_,sizeof(src_addr_));
    if(ret<0) ZLOG_CRIT<<"bind fail!";
}
//不能在类的声明和定义处同时指定默认参数
TcpFd::TcpFd(in_addr_t addr,in_port_t port,TcpType type,int protocol)
:type_(type){
    
    if(type==TcpType::client_FD)
    {
        fd_=socket(protocol,SOCK_STREAM|SOCK_NONBLOCK,0);
        if(fd_==-1) ZLOG_CRIT<<"socket fail!";
        dst_addr_.setDstAddr(addr,port,protocol);
        dst_port=port;
        int ret=bind(fd_,(sockaddr*)& dst_addr_,sizeof(src_addr_));
        if(ret<0) ZLOG_CRIT<<"bind fail!";
        //客户端不需要bind
    }
    else
    {
        fd_=socket(protocol,SOCK_STREAM|SOCK_NONBLOCK,0);
        if(fd_==-1) ZLOG_CRIT<<"socket fail!";
        src_addr_.setDstAddr(addr,port,protocol);
        src_port=port;
        int ret=bind(fd_,(sockaddr*)& src_addr_,sizeof(src_addr_));
        if(ret<0) ZLOG_CRIT<<"bind fail!";
    }
}

TcpFd::~TcpFd(){
    if(fd_!=-1) 
        ::close(fd_);
}

int TcpFd::readBuffer(std::string& data) {
    std::pair<std::string,int> ret=read_buffer_.deliver(this->fd_);
    if(ret.second==-2) return -1;//认为收到EOF
    data=ret.first;
    if(!data.empty()) return data.size();//这里说明数据完全准备好了
    else return -2;//数据还没准备好，上层应该再等等
}

int TcpFd::writeBuffer(const std::string& data){
    Message msg;
    msg.length=data.size();
    msg.data=data;
    write_buffer_.cache(msg,this->fd_);
    ZLOG_INFO<<"msg length: "<<(int)msg.length;
    return 0;
}

TcpFd::Ptr_t TcpFd::acceptConn(){
    sockaddr_in client_addr;
    socklen_t client_len=sizeof(client_addr);
    int conn_fd=::accept(fd_,(sockaddr*) &client_addr,&client_len);
	fcntl(conn_fd, F_SETFL, fcntl(conn_fd, F_GETFL, 0) | O_NONBLOCK);
    Address conn_addr(client_addr.sin_addr.s_addr,client_addr.sin_port,(int)client_addr.sin_family);
    TcpFd::Ptr_t p_conn=new TcpFd(conn_fd,this->src_addr(),conn_addr);
    return p_conn;
}

void TimerFd::set_timer(int first_overtime,int period, TimeSet set){
    struct itimerspec new_val;
    if(set=TimeSet::NANO_SEC)
    {
        new_val.it_value.tv_sec = 0;
        new_val.it_value.tv_nsec = period;
        new_val.it_interval.tv_sec = 0;
        new_val.it_interval.tv_nsec = first_overtime;
    }
    else
    {
        new_val.it_value.tv_sec = period;
        new_val.it_value.tv_nsec = 0;
        new_val.it_interval.tv_sec = first_overtime;
        new_val.it_interval.tv_nsec = 0;
    }
    int ret=timerfd_settime(fd_, 0, &new_val, NULL);
    if(ret<0) ZLOG_CRIT<<"set timer fail!";
}
