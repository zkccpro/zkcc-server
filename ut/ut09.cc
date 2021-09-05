/* for compile: 
g++ -pthread -g log/zlog.cc base/thread.cc base/signal.cc base/threadpool.cc vital/client.cc vital/buffer.cc vital/fd.cc vital/address.cc vital/poller.cc vital/event.cc vital/eventloop.cc vital/acceptor.cc vital/connection.cc vital/server.cc ut/ut09.cc -I/home/zkcc/zkcc/ -o ut/ut09 && ./ut/ut09
*/

#include"vital/client.h"
using namespace zkcc;
void onWriteDoneCallBack(Connection* conn){
    ZLOG_INFO<<"on writedone call back!";
    //模拟可能的数据库操作耗时
    // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // conn->stop();
}
void onMsgCallBack(std::string& data, Connection* conn){
    ZLOG_INFO<<"on msg call back!";
    std::string send_data="recv data is: "+data;
    conn->send(send_data);
    std::cout<<"recv data is: ";
    for(auto c:data){
        std::cout<<c;
    }
    std::cout<<std::endl;
    //模拟可能的数据库操作耗时
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
}

int main(){
    ZLog::initialize("/home/zkcc/zkcc/log/logc", 10);
    TcpClient client("127.0.0.1",50000);
    client.setMessageCallback(onMsgCallBack);
    client.setWriteDoneCallback(onWriteDoneCallBack);
    client.setDisconnectCallback(nullptr);
    client.connect();
}