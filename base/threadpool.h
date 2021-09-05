#pragma once
#include"base.h"
#include"thread.h"
#include<vector>
#include<deque>
#include<atomic>
#include<functional>

namespace zkcc{
enum TStat:char{
    OVER,//已结束
    READY,//空闲
    RUN//正在运行
};

class ThreadPool: NO_COPY,NO_MOVE {
public:
    //空任务类型
    using NullTask_t = void(*)();
    using Shared_t = std::shared_ptr<ThreadPool>;
    using Unique_t = std::unique_ptr<ThreadPool>;
    using Ptr_t = ThreadPool*;
    ThreadPool();//默认10个线程
    ~ThreadPool();
    ThreadPool(size_t size);
    //重新设置线程池的大小，做法是先把原来的线程池release，然后再初始化一份
    void resize(size_t);
    //线程池中每个预先创建的线程跑的void(*)()函数
    static void run();
    //让线程池中的任一个线程执行给定的任务，可以有参，但必须返回void
    template<typename Func,typename ...Args>
    void sign(Func&& f,Args&& ...args){
        std::lock_guard<std::mutex> guard(mtx_);
        size_t cur_th=GetFisrtReady();
        //线程池满了再来任务重启一下已结束的线程，并新加一个线程
        if(cur_th==0)
        {
            reuse();
            size_.fetch_add(1);
            cur_th=size_.load()-1;
            pool_.emplace_back(new zkcc::Thread(std::to_string(cur_th),ThreadPool::run));
            stat_.emplace_back(TStat::READY);//这个必须得加
        }
        auto task=std::bind(std::forward<Func>(f),std::forward<Args>(args)...);
        task_=task;
        pool_[cur_th]->run();
        stat_[cur_th].store(TStat::RUN);
        count_.fetch_add(1);
        //这里要等线程执行到对应的函数指针再结束线程，否则可能来不及执行这个任务就被下一个覆盖了
        //我觉得完全没必要担心阻塞的性能问题，因为这个临界区可以说非常非常短了：只加载一个函数指针
        sig_.wait();
        task_=null_;
    }
    //尽力释放所有线程资源，尽力指的是有一些还在进行的任务会detach，不会让其立刻结束
    void release();
    //重启已经结束了的线程
    void reuse();
    //返回所有线程数
    size_t size()const{return size_.load();}
    //返回正在执行任务的线程数
    size_t count()const{return count_.load();}
    //返回空闲的线程数
    size_t idle_count()const;
    //返回已结束暂未被重新利用的线程数
    size_t over_count()const;
private:
    //线程池中0号线程执行的任务，等待其他线程并回收之
    static void WaitThread();
    //找到第一个空闲线程
    size_t GetFisrtReady()const;
    //封装一下初始化线程池的逻辑
    void InitPool(size_t size);
private:
    static std::vector<zkcc::Thread::Shared_t> pool_;//zkcc线程的容器，vector是为了快速随机访问
    static std::mutex mtx_;//sign函数加锁，用于在多个线程互斥的执行sign函数
    static std::atomic<size_t> size_;//线程池中线程总数
    static std::atomic<size_t> count_;//正在运行的线程数
    static std::function<void()> task_;//将要执行的任务
    static Signal sig_;//当run函数已经加载了当前的m_task后，才能调度下一个sign函数
    static std::deque<std::atomic<TStat>> stat_;//atomic数组一定用deque
    static std::atomic<bool> is_run_;//决定整个线程池是否运行的标志
};
}//namespace zkcc
