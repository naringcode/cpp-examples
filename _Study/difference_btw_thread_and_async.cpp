#include <iostream>
#include <sstream>

#include <thread>
#include <future>

using namespace std;

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
    stringstream ss{};

    // 매번 ID가 다르게 나온다.
    ss << "[Thread ID] " << std::this_thread::get_id() << '\n';

    std::cout << ss.str();
}

void RunAsync()
{
    stringstream ss{};

    // 스레드 풀을 "스택 기반"으로 사용하기 때문에 같은 ID가 나올 확률이 높다.
    ss << "[Async ID] " << std::this_thread::get_id() << '\n';

    std::cout << ss.str();
}

int main() 
{
    constexpr int kMax = 20;

    std::thread threads[kMax];
    std::future<void> futures[kMax];

    for (int i = 0; i < kMax; i++)
    {
        threads[i] = std::thread{ RunThread };
        futures[i] = std::async(std::launch::async, RunAsync);
    }

    for (int i = 0; i < kMax; i++)
    {
        threads[i].join();
        futures[i].get();
    }

    return 0;
}
