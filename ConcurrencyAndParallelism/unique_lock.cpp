#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

// https://cplusplus.com/reference/mutex/unique_lock/

// std::unique_lock은 std::lock_guard와 비슷하지만 더 나은 유연성을 제공한다.

std::mutex mtx;
std::timed_mutex timedMtx;

void deferLock()
{
    // 소유권을 가변적으로 제어하는 방식
    std::unique_lock<std::mutex> lock(mtx, std::defer_lock);

    lock.lock(); // 수동으로 lock을 잡는다.
    {
        // Do Something
    }
    lock.unlock(); // 수동으로 lock을 푼다.

    // ...

    // 다시 수동으로 lock을 잡는다.
    lock.lock(); 

    // unlock을 지정하지 않았더라도 함수를 빠져나가는 시점에서 lock이 해제된다.
}

void timeLimitLock()
{
    // 제한 시간을 두고 Lock을 시도하는 것이 가능하다.
    std::unique_lock<std::timed_mutex> lock(timedMtx, std::chrono::seconds(1));

    if (lock.owns_lock())
    {
        // 자원 획득 성공
        std::cout << "Succeeded\n";
    }
    else
    {
        // 자원 획득 실패
        std::cout << "Failed\n";
    }
}

int main()
{
    std::thread th = std::thread(deferLock);

    th.detach();

    timedMtx.lock();
    {
        th = std::thread(timeLimitLock);

        std::this_thread::sleep_for(std::chrono::seconds(2));

        th.join();
    }
    timedMtx.unlock();

    th = std::thread(timeLimitLock);

    th.join();

    return 0;
}
