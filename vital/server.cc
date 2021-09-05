#include"server.h"
namespace zkcc{
TcpServer::TcpServer(int thread_num,int syn_size,int max_ready):
thread_pool_(new zkcc::ThreadPool(thread_num)),
acceptor_(syn_size,thread_pool_.get()),loop_(thread_pool_.get(),max_ready)
{}

}//namespace zkcc
