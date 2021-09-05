#include"thread.h"

namespace zkcc{
    thread_local std::string Thread::t_name_;//类内static，别忘类外声明
    thread_local Thread* Thread::t_thread_;
    Thread::Thread(const std::string& name,Func_t f):name_(name),func_(f){
        th_=std::thread(&Thread::RunPool,this,f);
    }
    Thread::~Thread(){
        if(th_.joinable())
            th_.detach();
    }

    void Thread::run(Func_t f){
        func_=f;
        //传入thread函数绝对不能是模板函数或模板类的类内函数
        //类内函数传入的时候要指定类，且类内非静态还需要加地址
        th_=std::thread(&Thread::RunThread,this,f);
    }
    void Thread::run(){
        sig_.signal();
    }

    void Thread::RunThread(Func_t f){
        t_name_=this->GetName();
        t_thread_=this;
        f();
    }

    void Thread::RunPool(Func_t f){
        t_name_=this->GetName();
        t_thread_=this;
        sig_.wait();
        f();
    }
}