#pragma once
#include<mutex>
#include<condition_variable>

class Signal{
public:
    explicit Signal()=default;
    explicit Signal(size_t count):count_(count){}
    void wait();
    void signal();
private:
    int count_=0;
    std::mutex mtx_;
    std::condition_variable cond_;
};