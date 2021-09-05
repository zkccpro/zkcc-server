#include"client.h"
namespace zkcc{
void TcpClient::connect(){
    while(::connect(clientfd_->fd(),(sockaddr*)server_addr_.addr(),server_addr_.size())) ;
    ZLOG_INFO<<"connect success!";
    conn_.reset(new Connection(clientfd_));
    conn_->set_loop(&loop_);
    //如果用户回调的地址!=null_的地址，则向loop注册该回调
    if(user_message_callback_!=nullptr)
        conn_->set_MessageCallback(user_message_callback_);
    if(user_disconnect_callback_!=nullptr)
        conn_->set_DisconnectCallback(user_disconnect_callback_);
    if(user_writedone_callback_!=nullptr)
        conn_->set_WriteDoneCallback(user_writedone_callback_);
    if(user_connect_callback_!=nullptr)
        user_connect_callback_(conn_.get());
    loop_.loop();
}

}//namespace zkcc