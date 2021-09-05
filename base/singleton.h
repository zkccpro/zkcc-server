#pragma once
#include"base.h"
#include<memory>
#include<mutex>

template<typename T>
class Singleton: NO_COPY,NO_MOVE{
public:
    using Ptr_t = T*;
    ~Singleton()=default;
    static Ptr_t get(){
        std::call_once(init_flag_,&Singleton::init);
        return inst_;
    }
private:
    Singleton()=default;
    static void init(){
        inst_=new T;
    }
    static std::once_flag init_flag_;
    static Ptr_t inst_;
};
template<typename T>
std::once_flag Singleton<T>::init_flag_;

//这里必须在模板类前面加上说明符typename，通知编译器Singleton<T>::Shared_t是个类型
//否则编译器解析Shared_t时可能有歧义（mumber or type???）
template<typename T>
typename Singleton<T>::Ptr_t Singleton<T>::inst_;
