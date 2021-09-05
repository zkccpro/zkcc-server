#include"threadpool.h"
namespace zkcc{
std::vector<zkcc::Thread::Shared_t> ThreadPool::pool_;
std::deque<std::atomic<TStat>> ThreadPool::stat_;
std::function<void()> ThreadPool::task_{nullptr};
Signal ThreadPool::sig_;
std::mutex ThreadPool::mtx_;
std::atomic<size_t> ThreadPool::count_{0};
std::atomic<size_t> ThreadPool::size_{0};
std::atomic<bool> ThreadPool::is_run_{true};

ThreadPool::ThreadPool(){
    InitPool(10);
}

ThreadPool::ThreadPool(size_t _size){
    InitPool(_size);
}

ThreadPool::~ThreadPool(){
    release();
}

void ThreadPool::InitPool(size_t size){
    is_run_.store(true);
    if(size==0) return;
    size_.store(size);
    task_=null_;
    //用第一个线程来作为一个线程池的管理线程，负责回收线程池的资源
    stat_.emplace_back(TStat::RUN);
    pool_.emplace_back(new zkcc::Thread(std::to_string(0)));
    for(int i=1;i<size;i++){
        pool_.emplace_back(new zkcc::Thread(std::to_string(i),ThreadPool::run));
        stat_.emplace_back(TStat::READY);
    }

    pool_[0]->run(ThreadPool::WaitThread);
    count_.fetch_add(1);
}

void ThreadPool::release(){
    if(is_run_.load()) is_run_.store(false);
    int size=size_.load();
    for(int i=0;i<size;i++){
        //这里不同的网络库有不同的处理方式
        //muduo在线程池需要销毁所有线程时用join，sylar用的是detach
        if(stat_[i].load()==TStat::RUN) pool_[i]->wait();
        if(stat_[i].load()==TStat::READY) pool_[i]->run();
        stat_[i].store(TStat::OVER);
    }
    size_.store(0);
    count_.store(0);
    pool_.clear();
    stat_.clear();
}

void ThreadPool::resize(size_t size){
    release();
    InitPool(size);
}

void ThreadPool::run(){
    sig_.signal();
    if(task_!=nullptr)
        task_();
}

void ThreadPool::reuse(){
    int size=size_.load();
    for(int i=0;i<size;i++){
        if(stat_[i].load()==TStat::OVER)
        {
            pool_[i].reset(new zkcc::Thread(std::to_string(i),ThreadPool::run));
            stat_[i].store(TStat::READY);
        }
    }
}

size_t ThreadPool::idle_count()const{
    size_t count=0;
    int size=size_.load();
    for(int i=1;i<size;i++){
        if(stat_[i].load()==READY) count++;
    }
    return count;
}

size_t ThreadPool::over_count()const{
    size_t count=0;
    int size=size_.load();
    for(int i=1;i<size;i++){
        if(stat_[i].load()==OVER) count++;
    }
    return count;
}

void ThreadPool::WaitThread(){
    bool is_all_idle=true;
    while(is_run_.load()){
        is_all_idle=true;
        for(int i=1;i<pool_.size();i++){
            if(stat_[i].load()==TStat::RUN)
            {
                is_all_idle=false;
                pool_[i]->wait();
                stat_[i].store(TStat::OVER);
                count_.fetch_sub(1);
            }
        }
        //如果所有线程都空闲，说明此时还没有来任务，那么管理线程休息50ms避免占用大量CPU
        if(is_all_idle)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

size_t ThreadPool::GetFisrtReady()const{
    int size=size_.load();
    for(size_t i=0;i<size;i++){
        if(stat_[i].load()==TStat::READY) return i;
    }
    return 0;//说明此时已经没有空闲线程了
}

}//namespace zkcc
