#include "thread_pool.h"

const int TASK_MAX_SIZE = 1024;
const int THREAD_INIT_SIZE = 4;
const int THREAD_MAX_SIZE = 10;
const int THREAD_MAX_IDLE_TIME = 10; // 单位s

ThreadPool::ThreadPool()
    : _initThreadSize(THREAD_INIT_SIZE)
    , _maxThreadSize(THREAD_MAX_SIZE)
    , _idleThreadSize(0)   
    , _counthreadSize(THREAD_INIT_SIZE)   
    , _taskSize(0)
    , _taskMaxSize(TASK_MAX_SIZE)
    , _poolMode(ThreadPoolMode::MODE_FIXED)
    , _poolRun(false)
{}

ThreadPool::~ThreadPool()
{
    printf("thread pool: \n\tthread max size[%zu]\n\tthread count size[%d]\n\tthread init size[%zu]\n",
        _maxThreadSize, _counthreadSize.load(), _initThreadSize);

    // #1 等待线程池里面所有的线程返回  两种状态： 阻塞 & 执行任务中
    _poolRun = false;
       
    // #2 线程池中的线程都杀掉
    std::unique_lock<std::mutex> lck(_taskMtx);  
    _notEmpty.notify_all();
    _notRun.wait(lck, [&]()->bool{
        return _threads.size() == 0;
    });
}

void ThreadPool::start(size_t size)
{
    _initThreadSize = size;
    _idleThreadSize.store(size);
    _counthreadSize.store(size);
    _poolRun = true;
    printf("thread pool: \n\tthread max size[%zu]\n\tthread count size[%d]\n\tthread init size[%zu]\n",
        _maxThreadSize, _counthreadSize.load(), _initThreadSize);

    // 创建线程对象
    _threads.reserve(_maxThreadSize);
    for(int i = 0; i < _initThreadSize; ++i)
    {
        // 创建thread对象时，把线程函数给到thread对象
        std::unique_ptr<Thread> ptr(new Thread(std::bind(&ThreadPool::threadHandler, this, std::placeholders::_1)));
        //auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadHandler, this, std::placeholders::_1)); 
        _threads.emplace(ptr->getId(), std::move(ptr));
    }

    // 启动所有线程
    for(int i = 0; i < _initThreadSize; ++i)
    {
        _threads[i]->start();
    }
}

void ThreadPool::threadHandler(int threadId)
{
    auto lastTime = std::chrono::high_resolution_clock().now();
    do
    {
        Task task;
        {
            // 获取锁
            std::unique_lock<std::mutex> lck(_taskMtx);
            while(_tasks.size() == 0) 
            {
                if(!_poolRun)
                {
                    _threads.erase(threadId);
                    _notRun.notify_all();
                    return;
                }
                // cache模式下，多余创建的长时间空闲线程需要回收(超过60s)
                if(_poolMode == ThreadPoolMode::MODE_CACHE)
                {
                    // 超时返回 -> 1s之内当前线程还没有分配到任务对象
                    if(std::cv_status::timeout == _notEmpty.wait_for(lck, std::chrono::seconds(1)))
                    {
                        auto current = std::chrono::high_resolution_clock().now();
                        auto dura = std::chrono::duration_cast<std::chrono::seconds>(current - lastTime);
                        if(dura.count() >= THREAD_MAX_IDLE_TIME && _counthreadSize > _initThreadSize)
                        {
                            // #1 更新记录当前线程数量变量
                            _counthreadSize--;
                            _idleThreadSize--;
                            // #2 当前线程对象从线程容器中删除? -> 设置 threadHandler任务对象 和 thread线程对象 的映射关系
                            _threads.erase(threadId);
                            std::cout << "threadId[" << std::this_thread::get_id() <<"] exit!" << std::endl;
                            return;
                        }
                    }                        
                }
                else
                {
                    // 线程通信，等待任务队列不为空
                    _notEmpty.wait(lck);
                }
            }
                      
            // 从任务队列获取任务
            task = _tasks.front();
            _tasks.pop();
            _taskSize--;
            _idleThreadSize--;

            // 如果依然存储剩余任务等待中，继续通知其他线程继续处理任务
            if(_taskSize  > 0)  _notEmpty.notify_all();
            
            // 释放锁，通知生产者
            _notFull.notify_all();       
        }
       
        // 执行任务
        task();

        // 更新线程执行完任务时间和空闲线程数量
        lastTime = std::chrono::high_resolution_clock().now();
        _idleThreadSize++;
    }while(true);

}

#if 0
Result ThreadPool::submitTask(std::shared_ptr<I_Task> task)
{
    // 获取锁
    std::unique_lock<std::mutex> lck(_taskMtx);
    // 线程通信，等待任务队列有空余
    if(false == _notFull.wait_for(lck, std::chrono::milliseconds(1000), [&]()->bool{
        return _tasks.size() < _taskMaxSize;}))
    {
        LOG("submit task timeout!");
        return Result(task, false);
    }
    // 新任务入队
    _tasks.push(task);
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

    return Result(task);
}
#endif

// 设置线程池模式
void ThreadPool::setMode(ThreadPoolMode mode)
{
    _poolMode = mode;
}

// 设置任务队列的最大上限
void ThreadPool::setTaskMaxSize(size_t size)
{
    _taskMaxSize = size;
}



