#include "common.h"

// ---------------- I_Task ---------------------------
I_Task::I_Task()
    : _result(nullptr)
{}

void I_Task::setResult(Result* res)
{
    _result = res;
}

void I_Task::exec()
{
    if(_result != nullptr)
    {
        //std::cout << "begin run tid: " << std::this_thread::get_id() << std::endl;
        _result->setValue(run());
        //std::cout << "end run tid: " << std::this_thread::get_id() << std::endl;
    }    
}

// ---------------- Semaphore ---------------------------
Semaphore::Semaphore(int count)
    : _count(count)
{}

void Semaphore::wait()
{
    std::unique_lock<std::mutex> lck(_mtx);
    _cv.wait(lck, [&]()->bool{
        return _count > 0;
    });
    _count--;
}

void Semaphore::post()
{
    std::unique_lock<std::mutex> lck(_mtx);
    _count++;
    _cv.notify_all();
}

// ---------------- Result ---------------------------
Result::Result(std::shared_ptr<I_Task> ptr, bool valid)
    : _taskPtr(ptr)
    , _isValid(valid)
{
    _taskPtr->setResult(this);
}

Any Result::get()
{
    if(!_isValid)
    {
        LOG("result get invalid!");
        return "";
    }
    
    _sm.wait(); // 如果任务没有执行完，这里会阻塞等待用户任务完成
    return std::move(_any);
}

void Result::setValue(Any any)
{
    this->_any = std::move(any);
    _sm.post();
}