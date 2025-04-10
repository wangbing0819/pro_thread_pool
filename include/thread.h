#ifndef _THREAD_H_
#define _THREAD_H_

#include <functional>
#include <thread>
#include <ctime>

class Thread
{
public:
    // 定义线程函数类型
    using ThreadFunc = std::function<void(int)>;
    // 
    Thread(ThreadFunc func);

    ~Thread();

    // 启动线程
    void start();

    int getId() const;

private:
    static int generator;

private:
    ThreadFunc _func;
    int        _threadId;

};

#endif