#include "RWLock.h"

#include <iostream>
#include <thread>
#include <chrono>

#include <queue>

class Test
{
public:
    int32_t Front()
    {
        ReadLockGuard lockGuard(_lock);

        if (true == _queue.empty())
            return -1;

        return _queue.front();
    }

    void Push()
    {
        WriteLockGuard lockGuard(_lock);

        _queue.push(rand() % 100);
    }

    void Pop()
    {
        WriteLockGuard lockGuard(_lock);

        if (false == _queue.empty())
        {
            _queue.pop();
        }
    }

private:
    RWLock _lock;

private:
    std::queue<int32_t> _queue;
};

Test test;
std::atomic<int32_t> incrementor = 1;

void ThreadWrite()
{
    tls_threadID = incrementor.fetch_add(1);

    while (true)
    {
        test.Push();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        test.Pop();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void ThreadRead()
{
    tls_threadID = incrementor.fetch_add(1);

    while (true)
    {
        int32_t value = test.Front();

        std::cout << value << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main()
{
    std::vector<std::thread> vec;

    tls_threadID = incrementor.fetch_add(1);
    
    for (int i = 0; i < 2; i++)
    {
        vec.push_back(std::thread(ThreadWrite));
    }
    
    for (int i = 0; i < 5; i++)
    {
        vec.push_back(std::thread(ThreadRead));
    }

    // Test Code
    // {
    //     RWLock lock;
    // 
    //     // lock.WriteLock();
    //     // {
    //     //     lock.ReadLock();
    //     // }
    //     // lock.WriteUnlock();
    // 
    //     lock.ReadUnlock();
    // }
    
    for (auto& thread : vec)
    {
        thread.join();
    }

    return 0;
}
