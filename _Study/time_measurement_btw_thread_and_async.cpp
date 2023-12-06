#include <iostream>
#include <sstream>

#include <thread>
#include <future>

// Debug 모드에서는 20 ~ 40배, Release 모드에서는 100배가 넘게 차이가 난다.
// 스레드의 개수를 늘리면 늘릴 수록 이 수치는 점점 더 벌어진다.
// 내가 병렬적으로 무언가를 관리할 것이 아니라 단순 일감을 던지는 용도라면 std::async를 쓰도록 하자.

using namespace std;

atomic<int> g_ThreadAtom;
atomic<int> g_AsyncAtom;

// main() 안에 있으면 조절하는 양에 따라 스택 오버플로우가 일어날 수도 있어서 여기에 만든다.
constexpr int kMax    = 100;
constexpr int kMaxLog = 10;

std::thread threads[kMax];
std::future<void> futures[kMax];

double threadLog[kMaxLog];
double asyncLog[kMaxLog];

// std::thread는 말 그대로 Thread-Based로 작업한다.
// 1. 저수준
// 2. 직접적인 제어가 필요할 때 사용(지속성)
// 3. 스레드를 생성하는 데 비용이 듦
// 4. 작업의 병렬성에 초점
// 5. 스레드 로컬 저장소를 사용하기에 적절

// std::async는 Task-Based로 작업한다(std::launch::async).
// 1. 고수준
// 2. 일감을 던지고 그 결과를 받고자 할 때 사용(단발성)
// 3. 스레드 풀을 사용하기 때문에 상대적으로 저비용
// 4. 작업의 비동기성에 초점
// 5. 스레드 로컬 저장소를 사용하기에 부적절

// 병렬성과 비동기성이 무슨 차이인지?
// CPU 코어가 하나만 있다고 해보자.
// - thread가 여러 개 있어도 실제로 수행되는 건 하나 뿐이라 병렬성을 달성하지 못 한다.
// - 반면에 일감을 던지고 이후에 받는 비동기적인 작업은 수행 가능하다.
// - 병렬성
//   - 데이터 병렬성 : 동일한 작업을 여러 곳에 분산해서 처리.
//   - 작업 병렬성 : 여러 작업을 독립적으로 실행.
// - 비동기 : 나는 할 거 하러 갈테니 나중에 결과만 알려줘.

// 주의 : std::async()는 클래스가 아닌 함수임.

void RunThread()
{
    g_ThreadAtom++;
}

void RunAsync()
{
    g_AsyncAtom++;
}

int main() 
{
    using clock_t  = std::chrono::high_resolution_clock;
    using second_t = std::chrono::duration<double>; // default period = std::ratio<1>

    // std::thread 시간 측정
    {
        for (int idx = 0; idx < kMaxLog; idx++)
        {
            auto startTimePoint = clock_t::now();

            for (int i = 0; i < kMax; i++)
            {
                threads[i] = std::thread{ RunThread };
            }

            for (int i = 0; i < kMax; i++)
            {
                threads[i].join();
            }

            auto endTimePoint = clock_t::now();

            double deltaTime = std::chrono::duration_cast<second_t>(endTimePoint - startTimePoint).count();

            threadLog[idx] = deltaTime;
        }
    }

    // std::async 시간 측정
    {
        for (int idx = 0; idx < kMaxLog; idx++)
        {
            auto startTimePoint = clock_t::now();

            for (int i = 0; i < kMax; i++)
            {
                futures[i] = std::async(std::launch::async, RunAsync);
            }

            for (int i = 0; i < kMax; i++)
            {
                futures[i].get();
            }

            auto endTimePoint = clock_t::now();

            double deltaTime = std::chrono::duration_cast<second_t>(endTimePoint - startTimePoint).count();

            asyncLog[idx] = deltaTime;
        }
    }

    // 출력
    {
        std::sort(threadLog, threadLog + kMaxLog, std::greater());
        std::sort(asyncLog, asyncLog + kMaxLog, std::greater());

        for (int idx = 0; idx < kMaxLog; idx++)
        {
            std::cout << "Thread Time : " << std::fixed<< threadLog[idx] << '\n';
        }

        for (int idx = 0; idx < kMaxLog; idx++)
        {
            std::cout << "Async Time  : " << std::fixed << asyncLog[idx] << '\n';
        }
    }

    return 0;
}
