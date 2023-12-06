#include <iostream>
#include <thread>
#include <future>
#include <chrono>

// 최적화 기능을 켠 상태에서 실행할 것
// volatile을 사용하는 것과 atomic을 사용하는 것의 시간 차이는 대략 3배
// 캐시를 최대한 활용하는 방식(그냥 int)은 volatile을 붙이는 것에 비해 대략 6배 정도 빠르다.
//
// val(1) > volVal(6) > atomVal(18)

constexpr int kCount = 100'000'000;

using MyClock  = std::chrono::high_resolution_clock;
using MySecond = std::chrono::duration<double>;

int val;
volatile int volVal;
std::atomic<int> atomVal;

int main() 
{
    std::future<double> fut1 = std::async(std::launch::async, []() {
        
        auto startTime = MyClock::now();

        for (int i = 0; i < kCount; i++)
        {
            val++;
        }

        auto endTime = MyClock::now();

        return std::chrono::duration_cast<MySecond>(endTime - startTime).count();
    });

    std::future<double> fut2 = std::async(std::launch::async, []() {

        auto startTime = MyClock::now();

        for (int i = 0; i < kCount; i++)
        {
            volVal++;
        }

        auto endTime = MyClock::now();

        return std::chrono::duration_cast<MySecond>(endTime - startTime).count();
    });

    std::future<double> fut3 = std::async(std::launch::async, []() {

        auto startTime = MyClock::now();

        for (int i = 0; i < kCount; i++)
        {
            atomVal.fetch_add(1);
        }

        auto endTime = MyClock::now();

        return std::chrono::duration_cast<MySecond>(endTime - startTime).count();
    });


    std::cout << fut1.get() << '\n' << fut2.get() << '\n' << fut3.get();

    return 0;
}