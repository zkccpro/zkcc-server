#pragma once
#include"eventloop.h"
#include<string>
namespace zkcc{
class Connection{
public:
    Connection()=default;
    Connection(TcpFd::Ptr_t fd):connfd_(fd){}
    ~Connection()=default;
    using Unique_t = std::unique_ptr<Connection>;
    using Share_t = std::shared_ptr<Connection>;
    using Ptr_t = Connection*;

    //将fd buffer中的内容传递给用户的回调，同时设置event的读事件
    template<typename FuncType>
    void set_MessageCallback(FuncType&& cb){
        user_message_callback_=std::forward<FuncType>(cb);
        if(!IO_event())
            IO_event_.reset(new Event(connfd_));
        IO_event_->setEventCallBack(POLL_IN,std::bind(&Connection::onMessage,this));
        loop_->appendEvent(IO_event_.get());
    }
    //设置网络库内置断开连接回调
    void setDisconnectCallback(){
        if(!IO_event())
            IO_event_.reset(new Event(connfd_));
        IO_event_->setEventCallBack(POLL_CLOSE,std::bind(&Connection::onDisconnect,this));
        loop_->appendEvent(IO_event_.get());
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
    //需要学数据的时候关注一下可写事件
    void set_WritableCallback();

    void cancelMessageCallBack();
    //不需要写数据时，立即停止关注可写事件
    void cancelWritableCallBack();
    
    //用网络库内置的函数包装一下用户的回调，用来处理粘包，
    //确认不粘包了之后才会执行用户的回调
    void onMessage();
    //当可写时，负责非阻塞的发送buffer里剩余的数据，直到发送完毕（发送完毕后应该立即停止监听此事件）
    /* @ no thread compete in multi threads */
    void onWritable();
    //关闭连接回调，用这个来包装一下用户的回调
    void onDisconnect();
    //用户发送数据的接口，把数据包装成Message类交给buffer后立即返回
    /* @ no thread compete in multi threads */
    void send(const std::string& msg);

    void set_connfd(TcpFd::Ptr_t connfd){connfd_=connfd;}
    void set_loop(EventLoop* loop){loop_=loop;}
    void stop(){loop_->stop();}
    TcpFd::Ptr_t connfd(){return connfd_;}
    int fd(){return connfd_->fd();}
    in_addr_t dst_addr(){return connfd()->dst_addr().address();}
    in_port_t dst_port(){return connfd()->dst_addr().port();}
    Event::Ptr_t IO_event(){return IO_event_.get();}

private:
    TcpFd::Ptr_t connfd_;
    Event::Unique_t IO_event_;//建立连接以后的事件（读写事件集）
    EventLoop* loop_;//加一个loop是为了在从reactor的loop开始后，也可以自由更新事件
    std::function<void(std::string&,Connection*)> user_message_callback_{nullptr};//收到消息时用户的回调
    std::function<void(Connection*)> user_disconnect_callback_{nullptr};//断开连接回调
    std::function<void(Connection*)> user_writedone_callback_{nullptr};//写完毕用户回调
};

}//namespace zkcc
