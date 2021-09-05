/* for compile: 
g++ -g ut/test01.cc -o ut/test && ./ut/test
*/
#include<iostream>
using namespace std;
#include"string"
using namespace std;
struct A{
//并不是这样的变量就一定开辟在栈上，只要A开辟在堆上它就在堆上
//即使A在堆上不会造成内存泄漏啊，只要A在堆上安全的释放了，OS就知道哪些空间是要释放的了
//即使A的析构函数没有处理变量a空间的清理，但它会自动调用a的析构，把需要清理的堆空间释放掉的
//如果变量a中没有其他指针，那么就不必调用free了，直接把空间归还给OS就安全了
//总之，类内的非指针变量的内存根本不需考虑内存安全问题，一点问题没有
//就是类内的指针变量才需要考虑，因为它指向了别处的堆区空间，如果不free，OS不知道那里还有一片空间没释放
int a=0;
};

//返回局部变量的地址或引用可没有RVO优化啊。。会出错的
int& fun(){
    int aa=0;
    return aa;
}
int main(){
    // A* pa=new A;
    // cout<<&pa<<endl;//栈上的地址
    // cout<<&pa->a<<endl;//堆上的地址

    // int ra=fun();
    // cout<<ra<<endl;//segmentation fault

    int size=123444;
    string sizestr=to_string(size);
    int ret;
    try
    {
        ret=stoi(sizestr);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    cout<<sizestr.size()<<endl;
    cout<<ret<<endl;
}