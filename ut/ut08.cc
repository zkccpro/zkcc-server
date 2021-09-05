/* for compile: 
g++ -pthread -g log/zlog.cc base/thread.cc base/signal.cc base/threadpool.cc vital/client.cc vital/buffer.cc vital/fd.cc vital/address.cc vital/poller.cc vital/event.cc vital/eventloop.cc vital/acceptor.cc vital/connection.cc vital/server.cc ut/ut08.cc -I/home/zkcc/zkcc/ -o ut/ut08 && ./ut/ut08
*/
#include"vital/client.h"
#include"vital/server.h"
using namespace zkcc;
/*
review:
初步封装客户端逻辑，并没有考虑收发，可以连接server，server端相应正常
*/
int main(){
    ZLog::initialize("/home/zkcc/zkcc/log/logc", 10);
    TcpClient client("172.25.222.68",50000);
    
}