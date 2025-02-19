// Update Date : 2025-02-20
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <format>
#include <thread>
#include <latch>
#include <barrier>
#include <atomic>
#include <vector>
#include <functional>

// https://en.cppreference.com/w/cpp/thread#Latches_and_Barriers
// https://en.wikipedia.org/wiki/Barrier_(computer_science)

// C++20에는 스레드 동기화를 위한 배리어 타입을 제공한다.
// - std::latch
// - std::barrier

// 배리어는 어떠한 지점에서 특정 조건을 만족할 때까지 다수의 스레드를 대시하게 하는 동기화 매커니즘을 말한다.
// 보통은 스레드의 동기화 지점을 지정하고 모든 스레드가 해당 지점에 도달할 경우 대기 상태를 푼다.
//
// std::latch와 std::barrier는 둘 다 count-down 기반이며 내부의 카운터가 0에 도달할 때까지 스레드를 잠근다.

// 각 스레드가 특정 작업을 끝낼 때까지 기다려야 하는 경우 배리어를 사용하면 좋다.

using namespace std::chrono_literals;

#define BEGIN_NS(name) namespace name {
#define END_NS }

BEGIN_NS(Case01)

// https://en.cppreference.com/w/cpp/thread/latch
// std::latch는 재사용 불가능한 스레드 배리어이다.

// 값을 동기화하는 방식
void Worker(int id, std::atomic<int>& counter, std::latch& latch)
{
    std::cout << std::format("Worker[{}] is starting...\n", id);

    counter++;

    // 값 동기화 지점
    latch.wait();

    std::cout << std::format("Worker[{}] has passed the barrier and the counter is {}\n", id, counter.load());
}

void Run()
{
    std::vector<std::jthread> jthreads;
    jthreads.resize(10);

    std::atomic<int> counter;
    std::latch latch{ (ptrdiff_t)jthreads.size() };

    for (int idx = 0; idx < jthreads.size(); idx++)
    {
        // (중요) thread 계열에 인자를 참조 형태로 넘기고자 한다면 std::ref()를 써야 한다.
        jthreads[idx] = std::jthread{ Worker, idx, std::ref(counter), std::ref(latch) };
    }

    std::cout << "Sleep for 2 seconds...\n";
    std::this_thread::sleep_for(2s);

    // 2초 뒤 명시적으로 카운트 다운 진행
    // latch.count_down(100); // (주의) 정해진 카운트보다 많이 차감하면 오류가 발생함.
    latch.count_down(jthreads.size());

    std::cout << "End of Main\n";
}

END_NS

BEGIN_NS(Case02)

// https://en.cppreference.com/w/cpp/thread/latch
// std::latch는 재사용 불가능한 스레드 배리어이다.

// 값을 동기화하는 방식
void Worker(int id, std::atomic<int>& counter, std::latch& latch)
{
    std::cout << std::format("Worker[{}] is starting...\n", id);

    counter++;

    // 값 동기화 지점
    // latch.wait();
    latch.arrive_and_wait(); // arrive_and_wait()를 쓰면 동기화 지점에 도달했을 시 바로 내부의 카운터를 차감한다.

    std::cout << std::format("Worker[{}] has passed the barrier and the counter is {}\n", id, counter.load());
}

void Run()
{
    std::vector<std::jthread> jthreads;
    jthreads.resize(10);

    std::atomic<int> counter;
    std::latch latch{ (ptrdiff_t)jthreads.size() };

    for (int idx = 0; idx < jthreads.size(); idx++)
    {
        // (중요) thread 계열에 인자를 참조 형태로 넘기고자 한다면 std::ref()를 써야 한다.
        jthreads[idx] = std::jthread{ Worker, idx, std::ref(counter), std::ref(latch) };
    }

    std::cout << "Sleep for 2 seconds...\n";
    std::this_thread::sleep_for(2s);

    // 2초 뒤 명시적으로 카운트 다운 진행
    // latch.count_down(jthreads.size()); // arrive_and_wait() 방식을 사용했기 때문에 명시적인 차감 코드는 없어도 됨.

    std::cout << "End of Main\n";
}

END_NS

BEGIN_NS(Case03)

// https://en.cppreference.com/w/cpp/thread/latch
// 값 동기화 외 작업이 완료될 때까지 대기하는 용도로 사용하기도 적합하다.

void LoadSometing(const std::string& name, int loadingTime)
{
    for (int i = 0; i < loadingTime; i++)
    {
        std::this_thread::sleep_for(1s);

        std::cout << std::format("Loading {}[{}s]...\n", name, i + 1);
    }
}

void WorkSomething(const std::string& name, int workingTime)
{
    for (int i = 0; i < workingTime; i++)
    {
        std::this_thread::sleep_for(1s);

        std::cout << std::format("Working {}[{}s]...\n", name, i + 1);
    }
}

void CleanUpSomething(const std::string& name)
{
    std::cout << std::format("Cleaning up {}...\n", name);
}

// 작업을 동기화하는 방식
void Worker(std::string name, 
            int loadingTime, int workingTime, 
            std::latch& loadingLatch, std::latch& cleanupLatch)
{
    std::cout << std::format("# {} started initialization\n", name);
    LoadSometing(name, loadingTime);
    std::cout << std::format("# {} finished initialization\n", name);

    // 동기화 지점
    loadingLatch.arrive_and_wait();
    
    std::cout << std::format("# {} started working\n", name);
    WorkSomething(name, workingTime);
    std::cout << std::format("# {} finished working\n", name);

    // 동기화 지점
    cleanupLatch.arrive_and_wait();

    std::cout << std::format("# {} started cleaning up\n", name);
    CleanUpSomething(name);
    std::cout << std::format("# {} finished cleaning up\n", name);
}

void Run()
{
    std::vector<std::jthread> jthreads(4);

    std::latch loadingLatch{ (ptrdiff_t)jthreads.size() };
    std::latch cleanupLatch{ (ptrdiff_t)jthreads.size() };

    jthreads[0] = std::jthread{ Worker, "A", 2, 5, std::ref(loadingLatch), std::ref(cleanupLatch) };
    jthreads[1] = std::jthread{ Worker, "B", 4, 3, std::ref(loadingLatch), std::ref(cleanupLatch) };
    jthreads[2] = std::jthread{ Worker, "C", 3, 4, std::ref(loadingLatch), std::ref(cleanupLatch) };
    jthreads[3] = std::jthread{ Worker, "D", 5, 2, std::ref(loadingLatch), std::ref(cleanupLatch) };
}

END_NS

BEGIN_NS(Case04)

// https://en.cppreference.com/w/cpp/thread/barrier
// std::barrier는 기능만 놓고 보면 std::latch와 동일하다.
// std::latch는 재사용할 수 없지만 std::barrier는 몇 번이든 재사용 가능하다.

// std::barrier는 내부의 카운터가 0에 도달하면 카운터를 초기 설정 값으로 초기화한다.

// std::barrier는 작업이 완료되었을 때 호출할 함수의 타입을 템플릿 인자로 받기 때문에 std::barrier<> 형식으로 전달해야 한다.
void Worker(std::string name, int waitingTime, std::barrier<>& barrier)
{
    constexpr int kMaxCnt = 5;

    for (int i = 0; i < kMaxCnt; i++)
    {
        for (int j = 0; j < waitingTime; j++)
        {
            std::this_thread::sleep_for(1s);

            std::cout << std::format("#{}# Waiting {}[{}s]...\n", i, name, j + 1);
        }
        
        std::cout << std::format("#{}# Ready {}...\n", i, name);

        barrier.arrive_and_wait();

        std::cout << std::format("#{}# Working {}!\n", i, name, i + 1);
    }

    std::cout << std::format("{} finished its task\n", name);
}

void Run()
{
    std::vector<std::jthread> threads{ };
    
    // callback은 optional
    std::barrier barrier{ 4 };
    
    threads.emplace_back(Worker, "A", 2, std::ref(barrier));
    threads.emplace_back(Worker, "B", 3, std::ref(barrier));
    threads.emplace_back(Worker, "C", 4, std::ref(barrier));
    threads.emplace_back(Worker, "D", 5, std::ref(barrier));
    
    for (std::jthread& th : threads)
    {
        th.join();
    }
    
    std::cout << "All tasks have finished!\n";
}

END_NS

BEGIN_NS(Case05)

// https://en.cppreference.com/w/cpp/thread/barrier#Template_parameters
// std::barrier는 작업이 완료되었을 때 호출할 함수를 지정할 수 있다.
// 이러한 함수는 예외를 던지지 않는 noexcept로 작성되어야 한다.

// 호출할 함수를 지정한 std::barrier를 인자로 전달할 때는 유형을 명시해야 한다(여기서는 함수 포인터).
void Worker(std::string name, int waitingTime, std::barrier<void(*)() noexcept>& barrier)
{
    constexpr int kMaxCnt = 5;

    for (int i = 0; i < kMaxCnt; i++)
    {
        for (int j = 0; j < waitingTime; j++)
        {
            std::this_thread::sleep_for(1s);

            std::cout << std::format("#{}# Waiting {}[{}s]...\n", i, name, j + 1);
        }
        
        std::cout << std::format("#{}# Ready {}...\n", i, name);

        barrier.arrive_and_wait();

        std::cout << std::format("#{}# Working {}!\n", i, name, i + 1);
    }

    std::cout << std::format("{} finished its task\n", name);
}

// std::barrier에 전달하는 함수는 noexcept로 작성되어야 한다.
void OnCompleted() noexcept
{
    std::cout << "##### On Completed! #####\n";
}

void Run()
{
    std::vector<std::jthread> threads{ };
    
    // 템플릿 인자는 함수 포인터로 추론된다.
    std::barrier barrier{ 4, OnCompleted };
    
    threads.emplace_back(Worker, "A", 2, std::ref(barrier));
    threads.emplace_back(Worker, "B", 3, std::ref(barrier));
    threads.emplace_back(Worker, "C", 4, std::ref(barrier));
    threads.emplace_back(Worker, "D", 5, std::ref(barrier));
    
    for (std::jthread& th : threads)
    {
        th.join();
    }
    
    std::cout << "All tasks have finished!\n";
}

END_NS

BEGIN_NS(Case06)

// std::barrier에 완료되었을 때 호출할 함수를 람다로 지정할 때는 생성할 때 자료형에 타입을 명시해야 한다.

// 호출할 함수를 지정한 std::barrier를 인자로 전달할 때는 유형을 명시해야 한다.
void Worker(std::string name, int waitingTime, std::barrier<void(*)() noexcept>& barrier)
{
    constexpr int kMaxCnt = 5;

    for (int i = 0; i < kMaxCnt; i++)
    {
        for (int j = 0; j < waitingTime; j++)
        {
            std::this_thread::sleep_for(1s);

            std::cout << std::format("#{}# Waiting {}[{}s]...\n", i, name, j + 1);
        }
        
        std::cout << std::format("#{}# Ready {}...\n", i, name);

        barrier.arrive_and_wait();

        std::cout << std::format("#{}# Working {}!\n", i, name, i + 1);
    }

    std::cout << std::format("{} finished its task\n", name);
}

void Run()
{
    std::vector<std::jthread> threads{ };

    // int completionCnt = 0; // 아래를 코드 [&]()나 [&completionCnt]로 사용하는 게 안 됨.
    auto onCompletion = []() noexcept { std::cout << "##### On Completed! #####\n"; };
    
    // std::function의 템플릿 인자로 "void() noexcept"를 전달하는 건 불가능하다.
    // std::barrier는 내부에서 Callable을 noexcept 형태로 invoke하는데 std::function은 noexcept로 invoke하는 것을 지원하지 않는다.
    // 
    // 캡처가 있는 람다는 멤버 변수를 가지는 함수 객체처럼 동작하는데,
    // 이를 함수 포인터로 변환하는 것이 불가능한 익명 함수의 한계라고 봐야할 것 같다.
    // std::barrier<std::function<void() noexcept>> barrier{ 4, onCompletion };

    // Worker 쪽 std::barrier의 템플릿 인자에 익명 함수 타입을 직접적으로 전달할 수 있는 방법이 없기 때문에
    // 템플릿 인자를 추론하는 방식을 사용하여 std::barrier를 생성하면 이를 Worker 쪽에 전달할 수 있는 방법이 없다.
    //
    // 이것저것 테스트 해봤는데 캡처 블록을 비운 상태 외엔 사용이 불가능했다.
    std::barrier<void(*)() noexcept> barrier{ 4, onCompletion };
    
    threads.emplace_back(Worker, "A", 2, std::ref(barrier));
    threads.emplace_back(Worker, "B", 3, std::ref(barrier));
    threads.emplace_back(Worker, "C", 4, std::ref(barrier));
    threads.emplace_back(Worker, "D", 5, std::ref(barrier));
    
    for (std::jthread& th : threads)
    {
        th.join();
    }
    
    std::cout << "All tasks have finished!\n";
}

END_NS

BEGIN_NS(Case07)

// completionCnt을 전역에 두면 가능하긴 해도 이렇게 하면 프로그램 흐름 전체에 영향을 미친다.
int completionCnt = 0;
    
// 호출할 함수를 지정한 std::barrier를 인자로 전달할 때는 유형을 명시해야 한다.
void Worker(std::string name, int waitingTime, std::barrier<void(*)() noexcept>& barrier)
{
    constexpr int kMaxCnt = 5;

    for (int i = 0; i < kMaxCnt; i++)
    {
        for (int j = 0; j < waitingTime; j++)
        {
            std::this_thread::sleep_for(1s);

            std::cout << std::format("#{}# Waiting {}[{}s]...\n", i, name, j + 1);
        }
        
        std::cout << std::format("#{}# Ready {}...\n", i, name);

        barrier.arrive_and_wait();

        std::cout << std::format("#{}# Working {}!\n", i, name, i + 1);
    }

    std::cout << std::format("{} finished its task\n", name);
}

void Run()
{
    std::vector<std::jthread> threads{ };

    // 이렇게 사용하면 동작하긴 해도 전역적으로 영향을 미치는 코드가 된다(의도한 동작이 아님).
    auto onCompletion = []() noexcept { std::cout << std::format("##### [{}] On Completed! #####\n", completionCnt++); };

    // std::barrier는 Callable을 noexcept 방식으로 invoke하기 때문에 std::function은 사용할 수 없다.
    std::barrier<void(*)() noexcept> barrier{ 4, onCompletion };
    
    threads.emplace_back(Worker, "A", 2, std::ref(barrier));
    threads.emplace_back(Worker, "B", 3, std::ref(barrier));
    threads.emplace_back(Worker, "C", 4, std::ref(barrier));
    threads.emplace_back(Worker, "D", 5, std::ref(barrier));
    
    for (std::jthread& th : threads)
    {
        th.join();
    }
    
    std::cout << "All tasks have finished!\n";
}

END_NS

BEGIN_NS(Case08)

// [&]() noexcept
// 람다식을 이렇게 구성하여 std::barrier에 전달하면 컴파일러 에러가 발생했다.

// 함수 객체를 통해 noexcept 기반의 호출을 하는 건 가능하기 때문에 이를 통해 원하는 기능을 우회해서 구현하면 된다.
struct CompletionFunctor
{
    int phaseCnt = 1;

    // noexcept로 구성해야 한다.
    void operator()() noexcept
    {
        std::cout << std::format("##### [{}] On Completed! #####\n", phaseCnt++);
    }
};

// 호출할 함수를 지정한 std::barrier를 인자로 전달할 때는 유형을 명시해야 한다.
void Worker(std::string name, int waitingTime, std::barrier<CompletionFunctor>& barrier)
{
    constexpr int kMaxCnt = 5;

    for (int i = 0; i < kMaxCnt; i++)
    {
        for (int j = 0; j < waitingTime; j++)
        {
            std::this_thread::sleep_for(1s);

            std::cout << std::format("#{}# Waiting {}[{}s]...\n", i, name, j + 1);
        }
        
        std::cout << std::format("#{}# Ready {}...\n", i, name);

        barrier.arrive_and_wait();

        std::cout << std::format("#{}# Working {}!\n", i, name, i + 1);
    }

    std::cout << std::format("{} finished its task\n", name);
}

void Run()
{
    std::vector<std::jthread> threads{ };

    int completionCnt = 0;
    
    // 함수 객체의 형태로 전달하는 방법
    std::barrier barrier{ 4, CompletionFunctor{ } };
    
    threads.emplace_back(Worker, "A", 2, std::ref(barrier));
    threads.emplace_back(Worker, "B", 3, std::ref(barrier));
    threads.emplace_back(Worker, "C", 4, std::ref(barrier));
    threads.emplace_back(Worker, "D", 5, std::ref(barrier));
    
    for (std::jthread& th : threads)
    {
        th.join();
    }
    
    std::cout << "All tasks have finished!\n";
}

END_NS

int main()
{
    // Case01::Run();
    // Case02::Run();
    // Case03::Run();
    // Case04::Run();
    // Case05::Run();
    // Case06::Run();
    // Case07::Run();
    Case08::Run();

    return 0;
}
