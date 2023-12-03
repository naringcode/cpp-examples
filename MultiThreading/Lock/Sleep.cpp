#include <Windows.h>

#include <iostream>

#include <thread>
#include <mutex>
#include <chrono>

class SpinLock
{
public:
    void lock()
    {
        bool expected = false;
        bool desired = true;

        // 뺑뺑이
        while (false == _locked.compare_exchange_strong(expected, desired))
        {
            // 이 부분을 빼먹지 않게 조심해야 한다.
            expected = false;

            // 정의된 오퍼레이터 연산자를 써서 대기 상태에 들어가는 것도 가능하다.
            // using namespace std::chrono;
            // using namespace std;
            // 둘 중 하나가 선행되어야 한다.
            // std::this_thread::sleep_for(100ms);

            // 100ms 동안 대기 상태에 들어간다.
            // std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // yield()는 sleep_for(0ms)와 동일한 기능을 수행한다.
            // sleep_for()에 특정 시간을 지정하면 그 시간 동안은 재스케줄링이 되지 않지만
            // yield()를 쓰면 자신의 타임 슬라이스를 포기하며 동시에 바로 재스케줄링이 될 수 있다.
            std::this_thread::yield();

            // 컴파일러의 정책에 따라 다르겠지만 yield()는 하이퍼스레딩과 관련이 있는 것은 아니다.
            // Windows에서 제공하는 _YIELD_PROCESSOR()와 혼동하지 말자.
            // yield()와 _YIELD_PROCESSOR()를 Release 모드에서 디스어셈블리로 보면 명령어가 다르다.
        }
    }

    void unlock()
    {
        _locked.store(false);
    }

private:
    std::atomic<bool> _locked = false;
};

int sum = 0;
std::mutex mtx;
SpinLock spinLock;

void Increase()
{
    for (int i = 0; i < 100'000; i++)
    {
        std::lock_guard<SpinLock> guard(spinLock);

        sum++;
    }
}

void Decrease()
{
    for (int i = 0; i < 100'000; i++)
    {
        std::lock_guard<SpinLock> guard(spinLock);

        sum--;
    }
}

int main()
{
    // std::this_thread::yield();

    std::atomic<bool> atomicLocked = false;

    // _YIELD_PROCESSOR();

    std::thread th1(Increase);
    std::thread th2(Decrease);

    th1.join();
    th2.join();

    std::cout << sum << '\n';

    return 0;
}
