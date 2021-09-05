#pragma once
#include"functional"
#define NO_COPY zkcc::base::NonCopyable
#define NO_MOVE zkcc::base::NonMoveable
#define CallBack_t zkcc::base::CallBack
#define null_ zkcc::base::g_null_task
namespace zkcc{
namespace base{
using CallBack = std::function<void()>;
auto g_null_task=[]()->void {};//空任务
struct NonCopyable{
	NonCopyable() {}
	~NonCopyable() {}
	NonCopyable(const NonCopyable &) = delete;
	NonCopyable& operator=(const NonCopyable &) = delete;
};

struct NonMoveable{
	NonMoveable() {}
	~NonMoveable() {}
	NonMoveable(NonMoveable &&) = delete;
    NonMoveable& operator=(NonMoveable &&) = delete;
};

}//namespace base
}//namespace zkcc
