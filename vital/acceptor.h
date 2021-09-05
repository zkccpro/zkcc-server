#pragma once
#include"connection.h"
#include"base/base.h"
class Acceptor{
public:
    Acceptor(zkcc::ThreadPool::Ptr_t tp):tp_(tp){setListen();}
    Acceptor(int syn_size,zkcc::ThreadPool::Ptr_t tp):tp_(tp){setListen(syn_size);}
    ~Acceptor();

    //设置用户的connectCallback，同时把callback注册到event里
    template<typename FuncType>
    void set_ConnectCallBack(FuncType&& cb){
        is_set_=true;
        if(listenfd_->fd()==-1) return;
        if(!accept_event())
            accept_event_.reset(new Event(listenfd_));
        accept_event_->setEventCallBack(POLL_IN,std::bind(&Acceptor::onConnect,this));
        user_connect_callback_=std::forward<FuncType>(cb);
    }
    //设置断开连接用户的回调
    template<typename FuncType>
    void set_DisconnectCallback(FuncType&& cb){
        user_disconnect_callback_=std::forward<FuncType>(cb);
    }
    //设置写完毕用户回调
    template<typename FuncType>
    void set_WriteDoneCallback(FuncType&& cb){
        user_writedone_callback_=std::forward<FuncType>(cb);
    }
    template<typename FuncType>
    void set_MessageCallback(FuncType&& cb){
        user_message_callback_=std::forward<FuncType>(cb);
    }

    //停止关注可写事件（这里只是改变一下IO_event的设置）
    void cancelConnectCallBack();

    //accept一个TCPsockt，用这个包装一下用户的回调
    //关键的函数，从reactor线程
    void onConnect();
    //设置listenfd_为监听状态
    void setListen(int syn_size=1024);
    //accept_event_的接口，EventLoop调用
    Event::Ptr_t accept_event(){return accept_event_.get();}

    TcpFd::Ptr_t listenfd(){return listenfd_;}
    
private:
    bool is_set_{false};//是否set过？没set过需要手动释放listenfd
    TcpFd::Ptr_t listenfd_{new TcpFd};//构造函数创建一个监听描述符，之后把此描述符交给accept_event_
    Event::Unique_t accept_event_;//连接事件（包含监听描述符和一个事件回调）
    std::function<void(zkcc::Connection*)> user_connect_callback_{nullptr};//连接d到来事件,用户回调
    std::function<void(std::string&,zkcc::Connection*)> user_message_callback_{nullptr};//连接d到来事件,用户回调
    std::function<void(zkcc::Connection*)> user_disconnect_callback_{nullptr};//断开连接回调
    std::function<void(zkcc::Connection*)> user_writedone_callback_{nullptr};//写完毕用户回调
    zkcc::ThreadPool::Ptr_t tp_;
};