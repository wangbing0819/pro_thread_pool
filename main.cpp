

#include <iostream>

#include <thread>
#include <chrono>
#include <future>
#include "thread_pool.h"

typedef unsigned long long uint64_t;

#if OLD
class Task : public I_Task
{
private:
    uint64_t begin;
    uint64_t end;

public:
    Task(uint64_t be = 0, uint64_t en = 10)
        : begin(be)
        , end(en)
    {}

    virtual Any run() override
    {
        std::cout << "tid[" << std::this_thread::get_id() <<"] begin!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        uint64_t sum = 0;
        for(uint64_t i = begin; i <= end; i++) sum+=i;
        std::cout << "tid[" << std::this_thread::get_id() <<"] end!" << std::endl;
        return sum;
    }
};

#endif

int sum(int a, int b)
{
    std::cout << "tid[" << std::this_thread::get_id() <<"] begin!" << std::endl;
    int res = 0;
    for(int i = a; i <= b; ++i)
        res += i;
        std::cout << "tid[" << std::this_thread::get_id() <<"] end!" << std::endl;
    return res;
}


int main()
{
    ThreadPool pool;
    pool.setMode(ThreadPoolMode::MODE_CACHE);
    pool.start(4);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::future<int> res1 = pool.submitTask(sum, 1, 10);
    std::future<int> res2 = pool.submitTask(sum, 11, 20);
    std::future<int> res3 = pool.submitTask(sum, 21, 30);
    std::future<int> res4 = pool.submitTask(sum, 31, 40);
    std::future<int> res5 = pool.submitTask(sum, 41, 50);

    int sum1 = res1.get();
    int sum2 = res2.get();
    int sum3 = res3.get();
    int sum4 = res4.get();
    int sum5 = res5.get();
    printf("sum1: %d, sum2: %d, sum3: %d, sum4: %d, sum5: %d\n", 
        sum1, sum2, sum3, sum4, sum5);

#if OLD
    Result res1 = pool.submitTask(std::shared_ptr<Task>(std::make_shared<Task>(1, 100000000)));
    Result res2 = pool.submitTask(std::shared_ptr<Task>(std::make_shared<Task>(1, 100000000)));
    Result res3 = pool.submitTask(std::shared_ptr<Task>(std::make_shared<Task>(1, 100000000)));
    Result res4 = pool.submitTask(std::shared_ptr<Task>(std::make_shared<Task>(1, 100000000)));
    Result res5 = pool.submitTask(std::shared_ptr<Task>(std::make_shared<Task>(1, 100000000)));
    Result res6 = pool.submitTask(std::shared_ptr<Task>(std::make_shared<Task>(1, 100000000)));

    uint64_t sum1 = res1.get().cast<uint64_t>();
    uint64_t sum2 = res2.get().cast<uint64_t>();
    uint64_t sum3 = res3.get().cast<uint64_t>();
    uint64_t sum4 = res4.get().cast<uint64_t>();
    uint64_t sum5 = res5.get().cast<uint64_t>();
    uint64_t sum6 = res6.get().cast<uint64_t>();

    printf("sum1: %llu, sum2: %llu, sum3: %llu, sum4: %llu, sum5: %llu, sum6: %llu", 
        sum1, sum2, sum3, sum4, sum5, sum6);
#endif

    std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}

