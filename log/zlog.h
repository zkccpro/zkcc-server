#pragma once

#include<iostream>
#include<string>
#include<memory>
#include <chrono>
#include <ctime>
#include <thread>
#include<cstring>
#include<algorithm>
#include<typeinfo>
#include<fstream>
#include<atomic>
#include<queue>


//***********ZLog************
//这里还是得拆开命名空间。。。。
namespace ZLog{
    //日志等级
    enum LogLevel:char{
    //用char可以省三个字节
    INFO,//信息
    WARN,//警告
    CRIT//错误
    };
}

/*
功能：封装一个日志行结构，将用户输入的参数push进堆区或栈区开辟的buffer
开发者：zkcc
日期：2021.4.26
*/
class ZLogLine{
public:
    ZLogLine()=default;
    ZLogLine(ZLog::LogLevel _level,const char* _filename,const char* _funcname, size_t _line);
    //只有这里声明了需要编译器提供默认操作，才可以；如果没声明，就不允许这种操作了！
    ZLogLine(const ZLogLine&)=default;
    ZLogLine& operator=(const ZLogLine&)=default;
    ZLogLine(ZLogLine&&)=default;
    ZLogLine& operator=(ZLogLine&&)=default;
    ~ZLogLine()=default;//析构不用释放堆区buffer了啊。。堆区buffer是独占指针，自动给你释放好了
    //各输入类型的重载，为了保证实际存入buffer的字节数最少，需要严格控制输入的类型，保证用最小的空间存用户的输入参数
    ZLogLine& operator<<(std::string _element);//C++string不能用，需要转换成const char*
    ZLogLine& operator<<(const char* _element);
    ZLogLine& operator<<(char _element);
    ZLogLine& operator<<(long long _element);
    ZLogLine& operator<<(long _element);
    ZLogLine& operator<<(int _element);
    ZLogLine& operator<<(double _element);
    ZLogLine& operator<<(float _element);
    void pop_to_file(std::ostream& os);//FileWriter调用这个，直接将buffer的内容写入文件中的一行

private:
    //解析重载输入的类型，只是推导一下类型，提供模板参数即可，不必传入参数
    template<typename Arg>
    int resolve_type();
    //采用泛型强制转换技术，可输入绝大多数类型
    //要认识到，存入buffer各地址的是数据
    template<typename Arg>
    void push_to_buffer(Arg arg);
    //为了重载输入参数，解析参数类型的push_to_buffer重载
    //存储时，在数据前面加上1B的type_id，以便在解码时识别类型
    template<typename Arg>
    void push_to_buffer(Arg arg, int type_id);
    char* get_buffer();//获取当前正在使用的buffer指针，如果堆区buffer不为空，返回堆区；否则返回栈区buffer
    void resize_buffer(size_t _enlarge);//push_to_buffer里需要提前检查push后的m_bytes_used超没超栈区buffer的上限，超了需要切换堆区buffer
    uint64_t timestamp_now();//系统获取当前时间
    void format_timestamp(std::ostream & os, uint64_t timestamp);//格式化的时间输出

private:
    std::unique_ptr<char[]> m_heap_buffer;//堆区指针，相当于一个数组指针(堆区是一个指针指向一个数组，栈区就是一个裸数组）
    char m_stack_buffer[256-sizeof(m_heap_buffer)-2*sizeof(size_t)];//栈区buffer，这样设计大小是为了确保ZLogLine类可以256B对齐
    size_t m_bytes_used;//当前用了多少字节，用来判断需不需要换buffer
    size_t m_cur_buffer_size;//当前正在使用的buffer的大小（堆区/栈区）

};

/*
功能：向实际文件写入一个日志行
开发者：zkcc
日期：2021.4.26
*/
class FileWriter{
public:
    FileWriter(const char* file_addr, size_t max_Mbytes_per_file);
    FileWriter(const FileWriter&)=delete;
    FileWriter(FileWriter&&)=delete;
    ~FileWriter()=default;
    void pop_to_file(ZLogLine&);//向文件写入
private:
    void roll_file();//换下一个文件，关闭当前文件，新建一个文件，打开
private:
    //写入文件的名字
    //示例：/home/zkcc/nanolog/ZLog/Develop/log
    std::string m_file_name;
    size_t m_nfile;//第几个文件的标号
    std::unique_ptr<std::ofstream> m_os;//当前打开的文件（输出流）指针
    size_t m_max_bytes_per_file;//每个文件写入的最大字节数
    size_t m_writen_bytes;//当前写入当前文件的字节
};

/*
功能：自旋锁，切换抽象日志文件时需要自旋锁住
开发者：zkcc
日期：2021.5.1
*/
class spinLock{
public:
    spinLock(std::atomic_flag& atf):m_atf(atf){
        while(m_atf.test_and_set(std::memory_order_acquire)) ;//内存序选择为：阻止后面的语句到此句前执行
    }
    ~spinLock(){
        m_atf.clear(std::memory_order_release);//内存序选择为：阻止前面的语句到此句后执行
    }
private:
    std::atomic_flag& m_atf;
};

/*
功能：封装一个ZLogLine日志行对象，确保此类对象256B对齐
开发者：zkcc
日期：2021.5.1
*/
struct Line
{
    Line(ZLogLine&& _zline):m_line(std::move(_zline)){}
    ~Line()=default;
    ZLogLine m_line;
private:
    char align[256-sizeof(ZLogLine)];//保证此类对象256B对齐
};

/*
功能：一个抽象的日志文件，封装了一个日志行数组，支持对其中某一行的日志进行pop/push
开发者：zkcc
日期：2021.5.1
*/
class AbsFile{
public:
    AbsFile();
    ~AbsFile();
    //阻止拷贝
    AbsFile(const AbsFile&)=delete;
    AbsFile& operator=(const AbsFile&)=delete;
    //AbsLog对象会调用此函数将一个日志行存入到当前的AbsFile中
    bool push_to_absfile(ZLogLine&& _zline,size_t cur_line);
    //AbsLog会调用此函数，将某一行的日志行送出去，最终写入文件
    bool pop_to_abslog(ZLogLine& _zline, size_t cur_line);
public:
    static const size_t m_max_line=32768;//一个抽象文件的最大行数
private:
    Line* m_lines;//日志行指针，指向当前抽象文件中的所有日志行
    //注意一个细节，必须用静态成员变量类内初始化其他成员变量（非静态变量在实例化前还没被分配内存呢。。）
    std::atomic<bool> m_isWriten[m_max_line];//标记当前行是否已经存入日志行了？
};

/*
功能：一个抽象的日志文件，封装了一个日志行数组，支持对其中某一行的日志进行pop/push
开发者：zkcc
日期：2021.5.2
*/
class AbsLog{
public:
    AbsLog();
    ~AbsLog()=default;
    AbsLog(const AbsLog&)=delete;
    //把_zline存到m_cur_pushfile的m_cur_pushline行
    void push_to_abslog(ZLogLine&& _zline);
    //通过引用把m_cur_popfile的m_pop_line行的日志行通过引用参数送至ZLogger对象中
    bool pop_to_logger(ZLogLine& _zline);
private:
    //当队列中最后一个absfile也装满数据，需要new一个AbsFile对象push入队，m_cur_pushfile置为此元素
    void set_new_pushfile();
    //当前的AbsFile已读完，把此AbsFile pop出队，m_cur_popfile置为队首元素（AbsFile中的push_to_absfile方法会通知）
    void get_next_popfile();
private:
    std::queue<std::unique_ptr<AbsFile>> m_fileq;//AbsFile的队列，保证日志记录和写入文件全过程不会丢失日志，多线程同时读此队列时需加锁
    //下面两个指针没必要用独占指针啊，因为他俩总是队列中的独占指针get到的内容，队列中的独占指针会确保内存的正确释放
    //如果用独占指针反而会出错，因为两个独占指针不能指向同一片区域！
    //为什么pop相关的AbsFile和cur_line不用原子量呢，是因为写入文件的工作始终只由一个线程完成。。不需要同步
    std::atomic<AbsFile*> m_cur_pushfile;//当前正在记入日志的AbsFile
    AbsFile* m_cur_popfile;//当前正在写入文件的AbsFile
    std::atomic<size_t> m_cur_pushline;//当前push到AbsFile的行数
    size_t m_cur_popline;//当前pop到文件的行数
    std::atomic_flag m_spin;//用于创造一个自旋锁，当需要往队列中入队出队时需锁住队列
};

enum state{INIT,READY,SHUT};//全局ZLogger对象当前所处的状态，READY时可以开始向文件写入日志行

class ZLogger{
public:
    ZLogger()=default;
    ZLogger(const char* file_addr, size_t max_bytes);
    ZLogger(const ZLogger& ) = delete;//此类的拷贝构造一定是禁用的！就算是我不写，编译器也会禁用，因为有一个成员是阻止拷贝的！(std::thread)
    ~ZLogger();//将state置为shut
    void push_to_logger(ZLogLine&& _zline);
    void pop_to_writer();
private:
    std::unique_ptr<AbsLog> m_AbsLog;
    std::unique_ptr<FileWriter> m_writer;//所以这个地方可能会引起你的灵魂思考，设计一个类成员变量时，什么时候使用指针，什么时候不使用指针呢
    std::atomic<state> m_state;
    std::thread m_thread;
};

//一个空类，重载+= 调用了ZLogger::push_to_logger，用于把用户传入的日志行送至全局的at_logger对象
class LoggerHelper{
public:
    bool operator+=(ZLogLine& _zline);
};

namespace ZLog{
    #define ZLOG_INFO LoggerHelper()+=ZLogLine(ZLog::LogLevel::INFO,__FILE__,__func__,__LINE__)//记录信息
    #define ZLOG_WARN LoggerHelper()+=ZLogLine(ZLog::LogLevel::WARN,__FILE__,__func__,__LINE__)//记录警告
    #define ZLOG_CRIT LoggerHelper()+=ZLogLine(ZLog::LogLevel::CRIT,__FILE__,__func__,__LINE__)//记录错误
    //初始化日志框架，ZLog只应在主线程中被初始化一次！
    void initialize(const char* file_addr, size_t max_bytes);
}