#pragma once
#include"connection.h"
#include"acceptor.h"
#include"eventloop.h"
/*
* 建议的函数原型：
* 1. 消息准备好回调：
* void onMsgCallBack(std::string& data, Connection* conn);
* 2. 连接建立回调：
* void onConnCallBack(Connection* conn);
* 3. 连接断开回调：
* void onDisconnCallBack(Connection* conn);
* 4. 写完毕回调：
* void onWriteDoneCallBack(Connection* conn);
*/
namespace zkcc{
class TcpServer{
public:
    TcpServer()=default;
    TcpServer(int thread_num,int syn_size,int max_ready);
    ~TcpServer()=default;

    //设置回调函数并调用acceptor_,conn_设置相应的事件，并更新loop_的关注的事件集
    template<typename FuncType>
    void setMessageCallback(FuncType&& cb){
        acceptor_.set_MessageCallback(cb);
    }
    template<typename FuncType>
    void setConnectCallback(FuncType&& cb){
        acceptor_.set_ConnectCallBack(cb);
        loop_.appendEvent(acceptor_.accept_event());
    }
    template<typename FuncType>
    void setDisconnectCallback(FuncType&& cb){
        acceptor_.set_DisconnectCallback(cb);
    }
    template<typename FuncType>
    void setWriteDoneCallback(FuncType&& cb){
        acceptor_.set_WriteDoneCallback(cb);
    }

    void start(){loop_.loop();}

private:
    zkcc::ThreadPool::Unique_t thread_pool_{new zkcc::ThreadPool};//默认线程池初始线程数：10
    Acceptor acceptor_{thread_pool_.get()};//默认syn队列：1024
    EventLoop loop_{thread_pool_.get()};//默认最大就绪事件：10
};

}//namespace zkcc
