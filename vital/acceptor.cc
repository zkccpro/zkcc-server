#include"acceptor.h"
Acceptor::~Acceptor(){
    if(!is_set_) delete listenfd_;
}

void Acceptor::cancelConnectCallBack(){
    if(!accept_event()) return;
    accept_event_->cancelEventCallBack(POLL_IN);
    user_connect_callback_=nullptr;
}

void Acceptor::setListen(int syn_size){
    int ret=listen(listenfd_->fd(),syn_size);
    if(ret==-1) ZLOG_CRIT<<"listenfd create error!";
    else
        ZLOG_INFO<<"listenfd is ready! fd="<<listenfd_->fd();
}

void Acceptor::onConnect(){
    ZLOG_INFO<<"connection is coming and handle in zthread "<<zkcc::Thread::GetNameBySelf();
    zkcc::Connection::Unique_t conn(new zkcc::Connection(listenfd_->acceptConn()));
    if(conn->fd()==-1)
    {
        ZLOG_CRIT<<"conn error!";
        return;
    }
    ZLOG_INFO<<"conn is ready! fd= "<<conn->fd();
    EventLoop lp(tp_);
    conn->set_loop(&lp);
    conn->setDisconnectCallback();
    //如果用户回调的地址!=null_的地址，则向loop注册该回调
    if(user_message_callback_!=nullptr)
        conn->set_MessageCallback(user_message_callback_);
    if(user_disconnect_callback_!=nullptr)
        conn->set_DisconnectCallback(user_disconnect_callback_);
    if(user_writedone_callback_!=nullptr)
        conn->set_WriteDoneCallback(user_writedone_callback_);

    //处理连接请求之后执行用户的回调
    if(user_connect_callback_!=nullptr)
        user_connect_callback_(conn.get());
    lp.loop();//开始从reactor线程-loop
    ZLOG_INFO<<"fd "<< conn->fd()<<"closed!";
}