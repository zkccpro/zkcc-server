#include"poller.h"
void Poller::controlPoll(epoll_event event_type, int control_method){
	epoll_ctl(efd_.fd(), control_method, event_type.data.fd, &event_type);
}
void Poller::appendEvent(Event* event){
    //map中找不到，则添加
    if(events_.find(event->Tcpfd())==events_.end()) addEvent(event);
    //map中找得到，则更新
    else updateEvent(event);
}
void Poller::addEvent(Event* event){
    epoll_event events=event->events();
    controlPoll(events, PollEvent::CTL_ADD);
    if(event->Tcpfd()!=-1) events_[event->Tcpfd()]=event;
    if(event->Timerfd()!=-1) events_[event->Timerfd()]=event;
    ZLOG_INFO<<"add event "<<(int)events.events<< " in "<< events.data.fd;
}
void Poller::updateEvent(Event* event){
    epoll_event events=event->events();
    controlPoll(events, PollEvent::CTL_UPD);
    if(event->Tcpfd()!=-1) events_[event->Tcpfd()]=event;
    if(event->Timerfd()!=-1) events_[event->Timerfd()]=event;
    ZLOG_INFO<<"update event "<<(int)events.events<< "in "<< events.data.fd;
}
void Poller::delEvent(Event* event){
    epoll_event events=event->events();
    controlPoll(events, PollEvent::CTL_DEL);
    if(event->Tcpfd()!=-1) events_[event->Tcpfd()]=event;
    if(event->Timerfd()!=-1) events_[event->Timerfd()]=event;
    ZLOG_INFO<<"delete event "<<(int)events.events<< "in "<< events.data.fd;
}

void Poller::wait(EventVec_t& ready_events){
    int nfds=0;
    nfds=::epoll_wait(efd_.fd(),ready_epoll_events_,max_ready_,-1);
    if(nfds==0) ZLOG_INFO<<"nothing accur";
    else if(nfds==-1) ZLOG_CRIT<<"epoll_wait error!";
    else
    {
        ZLOG_INFO<<"poll awake and ready fd count is: "<<nfds;
        if(!ready_events.empty()) ready_events.clear();
        for(int i=0;i<nfds;i++){
            ZLOG_INFO<<"ready epoll event: "<<(int)ready_epoll_events_[i].events;
            Event::Ptr_t p_event = events_[ready_epoll_events_[i].data.fd];
            p_event->set_ready_event(ready_epoll_events_[i]);
            ready_events.push_back(p_event);
        }
    }
}
