#include "thread.h"

int Thread::generator = 0;

Thread::Thread(ThreadFunc func)
    : _func(func)
    , _threadId(generator++)
{}

Thread::~Thread()
{}

void Thread::start()
{
    std::thread t(_func, _threadId);
    t.detach();
}

int Thread::getId()const
{
    return _threadId;
}
