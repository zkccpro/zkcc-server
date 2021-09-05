#include"signal.h"

void Signal::wait(){
    std::unique_lock<std::mutex> lock(mtx_);
    --count_;
    while(count_<0){
        cond_.wait(lock);
    }
}

void Signal::signal(){
    std::lock_guard<std::mutex> guard(mtx_);
    if(++count_<1)
    {
        cond_.notify_one();
    }
}