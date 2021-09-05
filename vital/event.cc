#include"event.h"

void Event::initMap(){
    event_callback_[POLL_NULL].first=null_;
    event_callback_[POLL_NULL].second=false;
    event_callback_[POLL_IN].first=null_;
    event_callback_[POLL_IN].second=false;
    event_callback_[POLL_OUT].first=null_;
    event_callback_[POLL_OUT].second=false;
    event_callback_[POLL_ERR].first=null_;
    event_callback_[POLL_ERR].second=false;
    event_callback_[POLL_CLOSE].first=null_;
    event_callback_[POLL_CLOSE].second=false;
}

epoll_event Event::events(){
    std::lock_guard<std::mutex> lock(mtx_);
    epoll_event set;
    set.events=POLL_NULL;
    for(auto& item:event_callback_){
        if(item.second.second==true)
        {
            set.events= set.events | item.first;
            if(tcp_->fd()!=-1) set.data.fd=tcp_->fd();
            else if(timer_->fd()!=-1) set.data.fd=timer_->fd();
        }
    }
    return set;
}

void Event::set_ready_event(const epoll_event& event){
    if(!ready_event_.empty()) ready_event_.clear();
    if(event.events&PollEvent::POLL_IN && !(event.events&PollEvent::POLL_CLOSE)) 
        ready_event_.push_back(POLL_IN);
    if(event.events&PollEvent::POLL_OUT) ready_event_.push_back(POLL_OUT);
    if(event.events&PollEvent::POLL_ERR) ready_event_.push_back(POLL_ERR);
    if(event.events&PollEvent::POLL_CLOSE) ready_event_.push_back(POLL_CLOSE);
}

void Event::onEvent(zkcc::ThreadPool::Ptr_t tp){
    if(ready_event_.empty()) return;
    for(auto ready:ready_event_){
        if(ready==POLL_CLOSE) event_callback_[ready].first();
        else tp->sign(event_callback_[ready].first);
    }
}