#include"connection.h"
namespace zkcc{
void Connection::set_WritableCallback(){
    if(!IO_event())
        IO_event_.reset(new Event(connfd_));
    IO_event_->setEventCallBack(POLL_OUT,std::bind(&Connection::onWritable,this));
    loop_->appendEvent(IO_event_.get());
}

void Connection::cancelMessageCallBack(){
    if(!IO_event()) return;
    IO_event_->cancelEventCallBack(POLL_IN);
    user_message_callback_=nullptr;
    loop_->updateEvent(IO_event_.get());
}

void Connection::cancelWritableCallBack(){
    if(IO_event_.get()==nullptr) return;
    IO_event_->cancelEventCallBack(POLL_OUT);
    loop_->updateEvent(IO_event_.get());
}

void Connection::onMessage(){
    ZLOG_INFO<<"msg recv!";
    std::string data;
    int ret=connfd_->readBuffer(data);
    //返回-2说明数据还没完全发过来，立即返回等待下轮回调
    if(ret==-2)
    {
        ZLOG_INFO<<"data not ready yet!";
        return;
    }
    //啥也没收到或头部还没完全发过来
    else if(ret==-1)
    {
        ZLOG_INFO<<"recv EOF!";
        onDisconnect();
        return;
    }
    //数据准备好了，执行用户回调
    else
    {
        ZLOG_INFO<<"data ready!";
        if(user_message_callback_!=nullptr)
            user_message_callback_(data,this);
    }
}

void Connection::onWritable(){
    ZLOG_INFO<<"kernel buffer is able to send!";
    //buffer中没有内容可发送了
    //不会和send函数中的cache出现竞争条件
    //因为send函数没调用set_WritableCallback时，不可能执行这个函数；
    //当执行这个函数后，write_buffer_肯定已经不是空了
    if(connfd_->write_buffer_.isEmpty())
    {
        ZLOG_INFO<<"nothing to send by now!";
        cancelWritableCallBack();
        if(user_writedone_callback_!=nullptr)
            user_writedone_callback_(this);//触发写完毕用户回调
        return;
    }
    connfd_->write_buffer_.send(connfd_->fd());
}
void Connection::send(const std::string& msg){
    connfd_->writeBuffer(msg);
    //向buffer中写好数据后，关注可写事件
    set_WritableCallback();
}

void Connection::onDisconnect(){
    ZLOG_INFO<<"remote disconnect!";
    cancelMessageCallBack();//需要取消关注消息事件，hup事件到来时in事件也会同时到来
    if(user_disconnect_callback_!=nullptr)
        user_disconnect_callback_(this);
    //这里有一个问题，如果用户设置的断开回调没有特殊处理的话
    //可能会导致缓冲区数据没完全发出去就关闭连接了，应该有一个等待缓冲区数据清空后再关闭的动作
    loop_->stop();//关闭从reactor处的loop循环，同时将释放conn指针内存，close描述符
}

}//namespace zkcc
