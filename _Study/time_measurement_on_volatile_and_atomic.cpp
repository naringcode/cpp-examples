#include <iostream>
#include <thread>
#include <future>
#include <chrono>
#include <vector>

// volatile_atomic_cache_coherence_and_memory_order.cpp
// volatile과 atomic의 차이는 위 파일에 적어두었음.

// !! 최적화 기능을 켠 상태에서 실행할 것 !!
// 
// 내 테스트 환경에서는 다음 차이를 보임
// - 그냥 int를 사용한 방식과 volatile을 적용한 방식의 시간 차이는 대략 6 ~ 8배(최대 10배까지 튐)
// - volatile을 적용한 방식과 atomic을 적용한 방식의 시간 차이는 대략 2.5 ~ 3배(거의 균일함)
// 
// 측정값은 컴파일러나 CPU 그리고 각종 환경에 따라 달라질 수 있음.
//
// 속도 순서(빠른 순)
// - val > volVal > atomVal

using namespace std;

constexpr int kCount = 100'000'000;

using MyClock  = std::chrono::high_resolution_clock;
using MySecond = std::chrono::duration<double>;

int val = 0;
volatile int volVal = 0;
std::atomic<int> atomVal;

int main() 
{
    // std::future<double> fut1 = std::async(std::launch::async, []() {
    //     
    //     auto startTime = MyClock::now();
    // 
    //     for (int i = 0; i < kCount; i++)
    //     {
    //         val++;
    //     }
    // 
    //     auto endTime = MyClock::now();
    // 
    //     return std::chrono::duration_cast<MySecond>(endTime - startTime).count();
    // });
    // 
    // std::future<double> fut2 = std::async(std::launch::async, []() {
    // 
    //     auto startTime = MyClock::now();
    // 
    //     for (int i = 0; i < kCount; i++)
    //     {
    //         volVal++;
    //     }
    // 
    //     auto endTime = MyClock::now();
    // 
    //     return std::chrono::duration_cast<MySecond>(endTime - startTime).count();
    // });
    // 
    // std::future<double> fut3 = std::async(std::launch::async, []() {
    // 
    //     auto startTime = MyClock::now();
    // 
    //     for (int i = 0; i < kCount; i++)
    //     {
    //         atomVal.fetch_add(1);
    //     }
    // 
    //     auto endTime = MyClock::now();
    // 
    //     return std::chrono::duration_cast<MySecond>(endTime - startTime).count();
    // });

    std::promise<double> prom1;
    std::future<double> fut1 = prom1.get_future();

    std::promise<double> prom2;
    std::future<double> fut2 = prom2.get_future();

    std::promise<double> prom3;
    std::future<double> fut3 = prom3.get_future();

    std::thread th1 = std::thread([&] {
        auto startTime = MyClock::now();

        // 최적화되면 for는 안 돈다.
        for (int i = 0; i < kCount; i++)
        {
            // Release 모드에서 최적화가 되긴 하는데 값이 2씩 증가함(중단점을 걸고 확인한 사항)
            // add    eax,2
            // sub    rcx,1
            // jne    `main'::`2'::<lambda_1>::operator()+0C1h // 이거 add eax,2 코드의 위치임
            // mov    dword ptr [val], eax // 레지스터를 대상으로 연산하고 값을 대입
            val++;
        }

        auto endTime = MyClock::now();

        prom1.set_value(std::chrono::duration_cast<MySecond>(endTime - startTime).count());

        // TEMP : 중단점 거는 용도
        while (true)
        {
            this_thread::sleep_for(10s);
        }
    });
    
    std::thread th2 = std::thread([&] {
        auto startTime = MyClock::now();

        for (int i = 0; i < kCount; i++)
        {
            // mov    eax,dword ptr [volVal] // 매번 메모리로부터 값을 가져온 다음
            // inc    eax // 값을 하나씩 증가시키고
            // mov    dword ptr [volVal],eax // 그 증가된 값을 다시 메모리에 넣음
            // sub    rcx,1
            // jne    `main'::`2'::<lambda_2>::operator()+0C0h
            volVal++;
        }

        auto endTime = MyClock::now();

        prom2.set_value(std::chrono::duration_cast<MySecond>(endTime - startTime).count());

        // TEMP : 중단점 거는 용도
        while (true)
        {
            this_thread::sleep_for(10s);
        }
    });

    std::thread th3 = std::thread([&] {
        auto startTime = MyClock::now();

        for (int i = 0; i < kCount; i++)
        {
            // 원자적으로 값을 하나씩 증가
            // lock inc    dword ptr [atomVal]
            atomVal.fetch_add(1);
        }

        auto endTime = MyClock::now();

        prom3.set_value(std::chrono::duration_cast<MySecond>(endTime - startTime).count());

        // TEMP : 중단점 거는 용도
        while (true)
        {
            this_thread::sleep_for(10s);
        }
    });

    std::cout << fut1.get() << '\n' << fut2.get() << '\n' << fut3.get();

    th1.join();
    th2.join();
    th3.join();

    return 0;
}
