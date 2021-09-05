#pragma warning(disable : 4996)//Clang编译器需要加上这个，屏蔽对不安全函数的报错
#include"zlog.h"

ZLogLine::ZLogLine(ZLog::LogLevel _level,const char* _filename,const char* _funcname, size_t _line)
:m_bytes_used(0),m_cur_buffer_size(sizeof(m_stack_buffer)){
    push_to_buffer<uint64_t>(timestamp_now());//时间
    push_to_buffer<std::thread::id>(std::this_thread::get_id());//tid
    push_to_buffer<ZLog::LogLevel>(_level);//日志等级
    push_to_buffer<const char*>(_filename);//文件名
    push_to_buffer<const char*>(_funcname);//函数名
    push_to_buffer<size_t>(_line);//行号
}

void ZLogLine::pop_to_file(std::ostream& os){
    char* buffer=!m_heap_buffer ? m_stack_buffer:m_heap_buffer.get();//获取当前使用的buffer
    char* buffer_end=buffer+m_bytes_used;//buffer的结尾地址
    uint64_t time_now=*reinterpret_cast<uint64_t*>(buffer);buffer+=sizeof(uint64_t);
    std::thread::id tid=*reinterpret_cast<std::thread::id*>(buffer);buffer+=sizeof(std::thread::id);
    ZLog::LogLevel level=*reinterpret_cast<ZLog::LogLevel*>(buffer);buffer+=sizeof(ZLog::LogLevel);
    const char* filename=*reinterpret_cast<const char**>(buffer);buffer+=sizeof(const char*);
    const char* funcname=*reinterpret_cast<const char**>(buffer);buffer+=sizeof(const char*);
    size_t line=*reinterpret_cast<size_t*>(buffer);buffer+=sizeof(size_t);
    const char* level_c=nullptr;
    if(level==ZLog::LogLevel::INFO) level_c="INFO";
    if(level==ZLog::LogLevel::WARN) level_c="WARN";
    if(level==ZLog::LogLevel::CRIT) level_c="CRIT";
    format_timestamp(os, time_now);
    os<<"[tid: "<<tid<<']'
    <<'['<<level_c<<']'
    <<'['<<filename<<':'<<funcname<<':'<<line<<']'<<' ';
    if(level==ZLog::LogLevel::CRIT) os.flush();//如果日志等级为错误，则线程随时可能崩溃，这里将缓冲区残留的日志都压到文件中，以尽可能保留住关键信息
    if(buffer==buffer_end) {os<<std::endl;return;}
    int type_id=0;
    while(buffer<buffer_end){
        type_id=*reinterpret_cast<int*>(buffer);buffer+=sizeof(int);
        switch (type_id)
	    {
	    case 0:
	        continue;
	    case 1:
	        os<<*reinterpret_cast<const char**>(buffer);buffer+=sizeof(const char*);
	        continue;
	    case 2:
	        os<<*reinterpret_cast<char*>(buffer);buffer+=sizeof(char);
	        continue;
	    case 3:
	        os<<*reinterpret_cast<long long*>(buffer);buffer+=sizeof(long long);
	        continue;
	    case 4:
	        os<<*reinterpret_cast<long*>(buffer);buffer+=sizeof(long);
	        continue;
	    case 5:
	        os<<*reinterpret_cast<int*>(buffer);buffer+=sizeof(int);
	        continue;
	    case 6:
	        os<<*reinterpret_cast<double*>(buffer);buffer+=sizeof(double);
	        continue;
	    case 7:
	        os<<*reinterpret_cast<float*>(buffer);buffer+=sizeof(float);
	        continue;
	    }
    }
    os<<std::endl;
}
uint64_t ZLogLine::timestamp_now(){
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

void ZLogLine::format_timestamp(std::ostream & os, uint64_t timestamp){
	std::time_t time_t = timestamp / 1000000;
	auto gmtime = std::gmtime(&time_t);
	char buffer[32];
	strftime(buffer, 32, "%Y-%m-%d %T.", gmtime);
	char microseconds[7];
	sprintf(microseconds, "%06llu", timestamp % 1000000);
	os << '[' << buffer << microseconds << ']';
}

template<typename Arg>
void ZLogLine::push_to_buffer(Arg arg){
    resize_buffer(sizeof(Arg));
    *reinterpret_cast<Arg*>(get_buffer())=arg;
    m_bytes_used+=sizeof(Arg);
}

template<typename Arg>
void ZLogLine::push_to_buffer(Arg arg, int type_id){
    // if(!type_id) std::cout<<"输入类型不对！！！"<<std::endl;
    // else std::cout<<"输入类型为："<<(int)type_id<<std::endl;
    resize_buffer(sizeof(Arg)+sizeof(int));
    *reinterpret_cast<int*>(get_buffer())=type_id;
    m_bytes_used+=sizeof(int);
    *reinterpret_cast<Arg*>(get_buffer())=arg;
    m_bytes_used+=sizeof(Arg);
}

//返回的是当前堆区/栈区数组中第m_bytes_used个元素的指针（地址）
char* ZLogLine::get_buffer(){
    return !m_heap_buffer ? &m_stack_buffer[m_bytes_used]:&(m_heap_buffer.get())[m_bytes_used];
}

//输入参数是容量增加的大小
void ZLogLine::resize_buffer(size_t _enlarge){
    //当前栈/堆区buffer还够用，不用扩容
    if(_enlarge==0 || _enlarge+m_bytes_used <= m_cur_buffer_size) return;
    //启用堆区buffer：
    //1. 日志行还没用堆区存储
    if(!m_heap_buffer)
    {
        //如果需要容量小于512，那么就分配512B，以免多次分配；
        size_t heap_s=std::max(m_bytes_used+_enlarge,static_cast<size_t>(512));//注意输入max函数的两个参数类型必须完全一样！
        m_heap_buffer.reset(new char[heap_s]);
        memcpy(m_heap_buffer.get(),m_stack_buffer,m_bytes_used);
        m_cur_buffer_size=heap_s;
    }
    //2. 日志行之前已经用到过堆空间
    else
    {
        //如果扩容需求小于原容量的两倍，把容量置为原容量的2倍
        size_t heap_s=std::max(2*m_cur_buffer_size, m_bytes_used+_enlarge);
        //把内存中的内容从一个指针移到独占指针时要额外小心！
        //以下的处理不会造成多次释放内存
        std::unique_ptr<char[]> new_heap_buffer(new char[heap_s]);
        memcpy(new_heap_buffer.get(),m_heap_buffer.get(),m_bytes_used);
        m_heap_buffer.swap(new_heap_buffer);
        m_cur_buffer_size=heap_s;
    }
}

template<typename Arg>
int ZLogLine::resolve_type(){
    if(typeid(Arg)==typeid(const char*)) return 1;
    if(typeid(Arg)==typeid(char)) return 2;
    if(typeid(Arg)==typeid(long long)) return 3;
    if(typeid(Arg)==typeid(long)) return 4;
    if(typeid(Arg)==typeid(int)) return 5;
    if(typeid(Arg)==typeid(double)) return 6;
    if(typeid(Arg)==typeid(float)) return 7;
    return 0;//注意异常类型处理
}

ZLogLine& ZLogLine::operator<<(std::string _element){
    const char* element=_element.c_str();
    int type_id=resolve_type<const char*>();
    push_to_buffer(element, type_id);
    return *this;
}

ZLogLine& ZLogLine::operator<<(const char* _element){
    int type_id=resolve_type<const char*>();
    push_to_buffer(_element, type_id);
    return *this;
}

ZLogLine& ZLogLine::operator<<(char _element){
    int type_id=resolve_type<char>();
    push_to_buffer(_element, type_id);
    return *this;
}

ZLogLine& ZLogLine::operator<<(long long _element){
    int type_id=resolve_type<long long>();
    push_to_buffer(_element, type_id);
    return *this;
}

ZLogLine& ZLogLine::operator<<(long _element){
    int type_id=resolve_type<long>();
    push_to_buffer(_element, type_id);
    return *this;
}

ZLogLine& ZLogLine::operator<<(int _element){
    int type_id=resolve_type<int>();
    push_to_buffer(_element, type_id);
    return *this;
}
ZLogLine& ZLogLine::operator<<(double _element){
    int type_id=resolve_type<double>();
    push_to_buffer(_element, type_id);
    return *this;
}

ZLogLine& ZLogLine::operator<<(float _element){
    int type_id=resolve_type<float>();
    push_to_buffer(_element, type_id);
    return *this;
}





FileWriter::FileWriter(const char* file_addr, size_t max_Mbytes_per_file)
:m_file_name(file_addr),m_nfile(0),m_max_bytes_per_file(max_Mbytes_per_file*(1024*1024)),m_writen_bytes(0){
    roll_file();
}

void FileWriter::roll_file(){
    if(m_os)
    {
        m_os->flush();
        m_os->close();
    }
    m_writen_bytes=0;
    m_os.reset(new std::ofstream());
    std::string open_file_name=m_file_name;
    open_file_name.append(std::to_string(++m_nfile));
    open_file_name.append(".txt");//第一个文件的标号是1
    m_os->open(open_file_name, std::ofstream::out | std::ofstream::trunc);//写方式打开一个文件，如果文件存在则将文件原有内容清空
}

void FileWriter::pop_to_file(ZLogLine& _line){
    m_writen_bytes=m_os->tellp();//获取已经记录了多少字节
    if(m_writen_bytes>=m_max_bytes_per_file) roll_file();
    _line.pop_to_file(*m_os);
}





AbsFile::AbsFile():m_lines((Line*)malloc(m_max_line*sizeof(Line))){
    for(int i=0;i<m_max_line;i++) m_isWriten[i].store(false);
}

AbsFile::~AbsFile() {
	//这么写的话在linux上没问题，但win上的clang编译之后的运行在这里会访问野指针，原因很显然啊。。如果absfile不满m_max_line行，你这么析构明显要出问题啊。
	//但是linux里为什么不出问题我就不知道了。。
	//for (int i = 0; i < m_max_line; i++) m_lines[i].~Line();
	free(m_lines);
}

bool AbsFile::push_to_absfile(ZLogLine&& _zline,size_t cur_line){
    if(cur_line>m_max_line-1) return true;//当前写入行比最大行数大，当前行就写不进去了！（应该不可能出现这种情况）保险起见写上，防止溢出崩溃，一旦出现了不会崩溃但会丢失一条日志
    new(&m_lines[cur_line]) Line(std::move(_zline));//???搞懂他！为什么只有这么写可以啊？？？？
    m_isWriten[cur_line].store(true,std::memory_order_release);
    if(cur_line==m_max_line-1) return true;//这是一个AbsFile中能记录的最后一条日志，记录完后要通知上层换一个新的AbsFile，正常的话不会丢失任一条日志
    return false;
}

bool AbsFile::pop_to_abslog(ZLogLine& _zline, size_t cur_line){
    //断点
    // bool isw=m_isWriten[0].load(std::memory_order_acquire);
    //如果当前指定的行数还没有被push日志行，则说明不能pop此行
    if(!m_isWriten[cur_line].load(std::memory_order_acquire)) return false;
    Line& _line=m_lines[cur_line];
    _zline=std::move(_line.m_line);
    m_isWriten[cur_line].store(false,std::memory_order_release);
    return true;
}




AbsLog::AbsLog()
:m_cur_popfile(nullptr),m_cur_pushline(0),m_cur_popline(0),m_spin{ATOMIC_FLAG_INIT}{
    set_new_pushfile();
}

void AbsLog::push_to_abslog(ZLogLine&& _zline){
    size_t cur_push=m_cur_pushline.fetch_add(1);//这里注意，必须先+1，否则多线程会出现丢失日志行！！！
    if(cur_push<AbsFile::m_max_line)
    {
        //其实这里不设置内存序也ok,默认字节序是memory_order_seq_cst（所有的顺序不许打乱）
        if(m_cur_pushfile.load()->push_to_absfile(std::move(_zline),cur_push))
        {//push_to_absfile返回true说明需要换一个AbsFile了，处理队列
            set_new_pushfile();
        }
    }
    else
    {
        //在多线程中，切换absfile会导致丢日志，这里需要如下操作：
        //当m_cur_pushline>=32768，说明此时有另一个线程正在更换absfile，
        //此时，此线程应该在此轮询等待，直到那个线程更换absfile完毕（m_cur_pushline清零）
        while(m_cur_pushline.load()>=AbsFile::m_max_line) ;//其实这相当于一个自旋锁
        //切换好absfile之后，再递归调用此函数，重新将此线程的日志行push进去
        push_to_abslog(std::move(_zline));
    }
}

//为什么pop不需要保证操作的原子性呢？是因为在ZLog中始终只有一个线程在pop日志
//而用户可能添加无数个线程来push日志
bool AbsLog::pop_to_logger(ZLogLine& _zline){
    //断点
    // int curpush=m_cur_pushline.load();
    // int qsize=m_fileq.size();
    if(!m_cur_popfile) get_next_popfile();//开始时先获得队首的absfile
    //在这里确保cur_pop不会超过最大行数
    if(m_cur_popline>AbsFile::m_max_line-1)
    {//如果超过，需get下一个popfile并更新m_cur_popline值，处理队列
        m_fileq.pop();
        m_cur_popline=0;
        get_next_popfile();
    }
    //当前行已经被push日志行了，则可以成功pop，将m_cur_popline+1，返回true
    if(m_cur_popfile && m_cur_popfile->pop_to_abslog(_zline,m_cur_popline))
    {
        m_cur_popline++;
        return true;
    }
    else return false;//如果当前行还没有push日志或者下一个popfile为空，m_cur_popline不变，需要向上通知pop失败
}
//只有这种写法才能保证在外面能正确访问m_cur_pushfile和m_fileq队列的元素
//因为在退出作用域前已经把newfile指针的控制权“移动”给了队列m_fileq中的独占指针
//所以即使退出作用域后newfile被delete掉，但一开始newfile指向的区域实际上被队列中的独占指针继续指向了，所以仍然访问得到
//当队列中的独占指针该退出作用域时，这片区域自然会被合理的释放掉，不用你操心了！
void AbsLog::set_new_pushfile(){
    std::unique_ptr<AbsFile> newfile(new AbsFile());
    m_cur_pushfile.store(newfile.get());
    spinLock lock(m_spin);//这个函数始终不存在多个线程同时进入的情况，所以考虑这个锁可以不加，实验证明ut_abs时这个锁不加没有问题
    m_fileq.push(std::move(newfile));
    m_cur_pushline.store(0);
}

void AbsLog::get_next_popfile(){
    spinLock lock(m_spin);//这个锁也是，这个函数的运行环境是单线程的，加锁又有什么用呢？？
    m_cur_popfile=m_fileq.empty() ? nullptr:m_fileq.front().get();
}




ZLogger::ZLogger(const char* file_addr, size_t max_bytes):
m_state(state::INIT),
m_AbsLog(new AbsLog()),
m_writer(new FileWriter(file_addr,max_bytes)),
m_thread(&ZLogger::pop_to_writer,this)
{
    m_state.store(state::READY);
}

ZLogger::~ZLogger(){
    m_state.store(state::SHUT);
    m_thread.join();
}

void ZLogger::push_to_logger(ZLogLine&& _zline){
    m_AbsLog->push_to_abslog(std::move(_zline));
}

void ZLogger::pop_to_writer(){
    ZLogLine zline;
    //用户进程没结束前，一直轮询等待logger状态准备好、日志过来
    while(m_state.load() != state::SHUT){
        if(m_state.load()==state::READY && m_AbsLog->pop_to_logger(zline))
            m_writer->pop_to_file(zline);
        //如果logger没有准备好或当前没有日志过来，则此线程挂起50ms，因为很可能一时半会儿也没有日志，在此轮询很吃性能，且睡眠也不会丢失任何日志
        else std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
    //用户的进程结束后，此线程需要继续保留一小会儿，确保shut前没来得及记录的日志都记录上
    while(m_AbsLog->pop_to_logger(zline))
        m_writer->pop_to_file(zline);
}
//编译器会将“类、内联函数、const变量”视作文件私有；所以，原则上只有这三种东西可以在头文件中定义，其他东西应放在源文件种定义（否则多个源文件引用一个头文件会造成重定义错误）
//如果把下面的东西放在头文件，则会出现main.cpp和Logger.cpp两个源文件同时引用了Logger.hpp头文件，造成符号重定义链接错误
std::atomic<ZLogger*> at_logger;//多线程下唯一的全局变量
std::unique_ptr<ZLogger> logger;//很细节！由于ZLogger不允许拷贝，所以只能用一个独占指针先开辟一块区域，再把这块区域的控制权交给上面的at_logger裸指针，这样就避免了内存的拷贝，完成了初始化

bool LoggerHelper::operator+=(ZLogLine& _zline){
        at_logger.load()->push_to_logger(std::move(_zline));
        return true;
}

void ZLog::initialize(const char* file_addr, size_t max_bytes){
    logger.reset(new ZLogger(file_addr,max_bytes));
    at_logger.store(logger.get());
}