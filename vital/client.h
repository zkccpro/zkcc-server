#pragma once
#include<arpa/inet.h>
#include"vital/connection.h"
namespace zkcc{
class TcpClient{
public:
    TcpClient():server_addr_(INADDR_ANY,50000){}
    TcpClient(const char* server_addr,int port):
    server_addr_(inet_addr(server_addr),port){}
    ~TcpClient()=default;

    void connect();//连接服务器，block
    void disconnect(){loop_.stop();}
    //设置用户的connectCallback，同时把callback注册到event里
    template<typename FuncType>
    void setConnectCallback(FuncType&& cb){
        user_connect_callback_=std::forward<FuncType>(cb);
    }
    //设置断开连接用户的回调
    template<typename FuncType>
    void setDisconnectCallback(FuncType&& cb){
        user_disconnect_callback_=std::forward<FuncType>(cb);
    }
    //设置写完毕用户回调
    template<typename FuncType>
    void setWriteDoneCallback(FuncType&& cb){
        user_writedone_callback_=std::forward<FuncType>(cb);
    }
    template<typename FuncType>
    void setMessageCallback(FuncType&& cb){
        user_message_callback_=std::forward<FuncType>(cb);
    }

private:
    Address server_addr_;
    TcpFd::Ptr_t clientfd_{new TcpFd(INADDR_ANY,40000,client_FD)};
    zkcc::ThreadPool::Unique_t thread_pool_{new zkcc::ThreadPool};//默认线程池初始线程数：10
    EventLoop loop_{thread_pool_.get()};//默认最大就绪事件：10
    Connection::Unique_t conn_;

private:
    std::function<void(Connection*)> user_connect_callback_{nullptr};//连接d到来事件,用户回调
    std::function<void(std::string&,Connection*)> user_message_callback_{nullptr};//连接d到来事件,用户回调
    std::function<void(Connection*)> user_disconnect_callback_{nullptr};//断开连接回调
    std::function<void(Connection*)> user_writedone_callback_{nullptr};//写完毕用户回调
};

}//namespace zkcc