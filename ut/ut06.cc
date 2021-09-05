/* for compile: 
g++ -pthread -g log/zlog.cc base/thread.cc base/signal.cc base/threadpool.cc vital/buffer.cc vital/fd.cc vital/address.cc vital/poller.cc vital/event.cc vital/eventloop.cc vital/acceptor.cc vital/connection.cc vital/server.cc ut/ut06.cc -I/home/zkcc/zkcc/ -o ut/ut06 && ./ut/ut06
*/
/*
review 2021.8.27-10:20 AM:
服务端完整测试，单个连接未发现大问题，
但连接过多时偶尔程序会崩溃，猜测与不合理的线程池设计有关
*/
/*
issue 2021.8.27-10:20 AM:
1. 重新设计线程池的结构，设置线程数上限和任务等待机制
*/
#include"vital/server.h"
using namespace zkcc;
void onConnCallBack(Connection* conn){
    ZLOG_INFO<<"on conn call back!";
    in_addr_t dst_addr= conn->dst_addr();//对端地址
    in_port_t dst_port= conn->dst_port();//对端端口
    std::string data="conn establish!";
    conn->send(data);
    //模拟可能的数据库操作耗时
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
}

void onMsgCallBack(std::string& data, Connection* conn){
    ZLOG_INFO<<"on msg call back!";
    std::string send_data="server recv your data!";
    conn->send(send_data);
    //模拟可能的数据库操作耗时
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
}

void onWriteDoneCallBack(Connection* conn){
    ZLOG_INFO<<"writedone!";
    //模拟可能的数据库操作耗时
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
}

void onDisconnCallBack(Connection* conn){
    ZLOG_INFO<<"on disconn call back!";
    //模拟可能的数据库操作耗时
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
}

int main(){
    ZLog::initialize("/home/zkcc/zkcc/log/log", 10);
    TcpServer server;
    server.setConnectCallback(onConnCallBack);
    server.setMessageCallback(onMsgCallBack);
    server.setWriteDoneCallback(onWriteDoneCallBack);
    server.setDisconnectCallback(onDisconnCallBack);
    server.start();
}