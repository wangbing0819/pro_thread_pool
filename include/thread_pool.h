#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include "common.h"
#include "thread.h"
#include <future>
#include <queue>
#include <vector>
#include <unordered_map>


enum class ThreadPoolMode
{
    MODE_FIXED = 0,     // 固定数量的线程
    MODE_CACHE          // 线程数量动态增长
};

class ThreadPool
{
public:
    ThreadPool();

    ~ThreadPool();
    
    // 开启线程池
    void start(size_t size = 4);

    // 设置线程池模式
    void setMode(ThreadPoolMode mode);       
    
    // 设置初始线程数量
    // void setInitThreadSize(size_t size);

    // 设置任务队列的最大上限
    void setTaskMaxSize(size_t size);

    // 给线程池提交任务(生产者)
    template<typename Func, typename... Args>
    auto submitTask(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>
    {
        using RType = decltype(func(args...));
        auto task = std::make_shared<std::packaged_task<RType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...)); // (*task)(args) --bind--> (*task)()调用   
        std::future<RType> result = task->get_future();
        
        // 获取锁
        std::unique_lock<std::mutex> lck(_taskMtx);
        // 线程通信，等待任务队列有空余
        if(false == _notFull.wait_for(lck, std::chrono::milliseconds(1000), [&]()->bool{
            return _tasks.size() < _taskMaxSize;}))
        {
            LOG("submit task timeout!");
            auto timeoutTask = std::make_shared<std::packaged_task<RType()>>(
                []()->RType{ return RType(); });
            return timeoutTask->get_future();
        }
        // 新任务入队  void()
        _tasks.push([task]()->void{ (*task)(); });
        _taskSize++;

        // 通知线程池进行任务处理
        _notEmpty.notify_all();

        // cache模式：小而快的场景
        if(_poolMode == ThreadPoolMode::MODE_CACHE)
        {
            if(_taskSize > _idleThreadSize && _counthreadSize < _maxThreadSize )
            {    
                // #1 创建新线程对象
                std::cout << "create new thread" << std::endl;      
                auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadHandler, this, std::placeholders::_1));    
                int threadId = ptr->getId();
                _threads.insert(std::make_pair(threadId, std::move(ptr)));
                // #2 启动线程
                _threads[threadId]->start();
                // #3 更新线程变量
                _counthreadSize++; 
                _idleThreadSize++;            
            }
        }

        return result;
    }

    // 禁止拷贝和赋值
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    void threadHandler(int threadId);

private:
    size_t                                      _initThreadSize;    // 初始线程数量
    size_t                                      _maxThreadSize;     // 最大线程数量
    std::atomic<int>                            _idleThreadSize;    // 空闲线程数量
    std::atomic<int>                            _counthreadSize;    // 当前线程数量                                    _
    // std::vector<std::unique_ptr<Thread>>        _threads;           // 存储线程容器
    std::unordered_map<int, std::unique_ptr<Thread>> _threads;
    
    using Task = std::function<void()>;
    std::queue<Task>                            _tasks;             // 任务队列
    std::atomic<int>                            _taskSize;          // 任务数量
    int                                         _taskMaxSize;       // 任务最大数量
    std::mutex                                  _taskMtx;
    std::condition_variable                     _notFull;
    std::condition_variable                     _notEmpty;
    
    ThreadPoolMode                              _poolMode;

    std::condition_variable                     _notRun;
    bool                                        _poolRun;

};


#endif