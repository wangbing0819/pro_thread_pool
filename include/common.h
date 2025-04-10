#ifndef COMMON_H_
#define COMMON_H_

#include <iostream>
#include <memory>
#include <mutex>
#include <atomic>
#include <future>
#include <condition_variable>

#ifndef LOG
#define LOG(str)                                    \
    std::cout << __FILE__ << " : " << __LINE__ << " " << \
    __TIMESTAMP__ << " : " << str << std::endl;
#endif

#ifndef RECHECK_POINT_NULL
#define RECHECK_POINT_NULL(ptr, str)    \
    if(nullptr == ptr)                  \
    {                                   \
        LOG(str)                        \
        return false;                   \
    }     
#endif 

#define BEGIN_NAMESPACE(ns) namespace ns {
#define END_NAMESPACE(ns)  }

// 抽象任务类
class Any;
class Result;
class I_Task
{
public:
    I_Task();
     
    ~I_Task() = default;

    void setResult(Result* res);

    void exec(); // #1.执行run方法， #2将返回结果setValue给Result

    virtual Any run() = 0;
private:
    Result* _result;
};

// 可以接收任意数据的类型
class Any
{
public:
    Any() = default;
    ~Any() = default;
    Any(const Any&) = delete;
    Any& operator= (const Any&) = delete;
    Any(Any&&) = default;
    Any& operator= (Any&&) = default;

    template<typename T>
    Any(T data) : _base(std::make_unique<Derive<T>>(data)) {}

    template<typename T>
    T cast()
    {
        // 派生类  -》 基类
        Derive<T>* pd = dynamic_cast<Derive<T>*>(_base.get()); 
        if(nullptr == pd) 
        {
            std::cout << "derive nullptr" << std::endl;
            throw "type is unmatch!";
        }
        return pd->getData();
    }

private:
    class Base
    {
    public:
        virtual ~Base() = default;
    };

    template<typename T>
    class Derive : public Base
    {
    public:
        Derive(T data) : _data(data) {}

        T getData() const {return _data;}
    private:
        T _data;
    }; 
    
private:
    std::unique_ptr<Base> _base;
};

// 信号量
class Semaphore
{
public:
    Semaphore(int count = 0);

    ~Semaphore() = default;

    // 减少信号量
    void wait();

    // 增加信号量
    void post();

private:
    std::atomic<int>                _count;
    std::mutex                      _mtx;
    std::condition_variable         _cv;
};

// 实现接收提交到线程池task任务执行完成后的返回值类型Result
class Result
{
public:
    Result() = default;
    ~Result() = default;

    Result(std::shared_ptr<I_Task> ptr, bool valid = true);

    // get(): 用户调用这个方法获取task的返回值
    Any get();

    // setValue(): 获取任务执行完的返回值
    void setValue(Any any);

private:
    std::shared_ptr<I_Task>     _taskPtr;
    std::atomic<bool>           _isValid;

    Any                         _any;
    Semaphore                   _sm;
};

#endif


