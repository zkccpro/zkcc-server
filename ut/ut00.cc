/* for compile: 
g++ -pthread -g base/thread.cc base/signal.cc base/threadpool.cc ut/ut00.cc -I/home/zkcc/zkcc/ -o ut/ut00 && ./ut/ut00
*/

/*
review:
测试了线程池，没啥大问题，resize最好只在创建线程池之后用一次，不要经常用
*/

#include"base/threadpool.h"
#include"base/singleton.h"
#include<iostream>
#include<vector>
int g_val=0;
void f1(){
    std::cout<<zkcc::Thread::GetTidBySelf()<<std::endl;
    //const std::string name = zkcc::Thread::__getName();
    zkcc::Thread* pth=zkcc::Thread::getThisBySelf();
    const std::string name = pth->GetName();
    for(auto c:name){
        std::cout<<c;
    }
    std::cout<<std::endl;
    g_val++;
}
void f2(){
    std::cout<<"this is f2"<<std::endl;
}
void f3(){
    std::cout<<"this is f3"<<std::endl;
}
void f4(){
    std::cout<<"this is f4"<<std::endl;
}
void f5(){
    std::cout<<"this is f5"<<std::endl;
}
void f6(int i){
    std::cout<<"this is f6: "<<i<<std::endl;
}

int main(){
    // zkcc::ThreadPool::Shared_t thp(Singleton<zkcc::ThreadPool>::get());
    // zkcc::ThreadPool::Ptr_t thp1=Singleton<zkcc::ThreadPool>::get();
    // thp1->resize(100);
    // thp1->sign(f6,1000);
    // thp1->sign(f6,1000);
    // thp1->sign(f6,1000);
    // thp1->sign(f6,1000);
    // thp1->sign(f6,1000);
    // thp1->sign(f6,1000);
    // zkcc::ThreadPool::Ptr_t ths(new zkcc::ThreadPool);
    // ths->resize(5);
    // ths->sign(f2);
    zkcc::ThreadPool::Shared_t ths(new zkcc::ThreadPool(10));
    ths->sign(f2);
    return 0;
}