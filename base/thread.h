#pragma once
#include"base.h"
#include<thread>
#include<string>
#include<memory>
//#include<iostream>
#include"signal.h"
namespace zkcc{

class Thread: NO_COPY, NO_MOVE {
public:
    using Shared_t = std::shared_ptr<Thread>;
    using Func_t = void(*)();
    
    //thread_local的作用域你需要仔细研究一下！
    //类外static thread_local的作用域是各个线程、各个栈独立的
    //类外static thread_local的作用域是各个线程独立的，但各个栈并不独立
    static thread_local std::string t_name_;
    //这里不能用智能指针，不要把this指针交给shared去管理，因为c++自己会管理this指针，用shared的话会重复释放的
    static thread_local Thread* t_thread_;
    Thread()=default;//std::thread的默认构造就是只构造一个对象，但不开辟线程
    Thread(const std::string& name):
    name_(name){ }
    Thread(const std::string& name,Func_t f);
    ~Thread();

    void SetName(const std::string& name){name_=name;}
    const std::string& GetName()const{return name_;}
    
    void run(Func_t f);
    void run();

    void detach(){th_.detach();}
    void wait(){th_.join();}//c++2.0后，内类实现函数较短的自动内联（考虑作用域），类外实现一定不会内联

    static void SetNameBySelf(const std::string& _name){
        t_name_=_name;
        t_thread_->SetName(_name);
    }
    //这里有一个隐含bug，如果不是zthread，getname会乱码，留待后改
    static const std::string GetNameBySelf(){return t_name_;}
    static Thread* getThisBySelf(){return t_thread_;}
    static std::thread::id GetTidBySelf(){return std::this_thread::get_id();}
    static void SleepBySelf(size_t ms){std::this_thread::sleep_for(std::chrono::milliseconds(ms));}

public:
    Func_t func_=nullptr;
private:
    //这个函数就已经在新的线程栈里跑了
    void RunThread(Func_t f);
    void RunPool(Func_t f);
private:
    std::thread th_;
    std::string name_="no name thread";
    Signal sig_{0};
};

}//namespace zkcc
