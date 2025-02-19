// Update Date : 2025-02-19
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <format>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std::chrono_literals;

#define BEGIN_NS(name) namespace name {
#define END_NS }

BEGIN_NS(Case01)

// https://en.cppreference.com/w/cpp/thread

// 기존 스레드 방식은 스레드 사용하는 과정에서 detach()나 join()을 호출해야 문제가 발생하지 않는다.

void Task(int duration)
{
    for (int i = 0; i < duration; i++)
    {
        std::cout << std::format("cnt : {}\n", i);
        
        std::this_thread::sleep_for(0.2s);
    }
}

void Run()
{
    std::thread th{ Task, 10 };

    std::cout << "Thread started!\n";

    th.join();
    // th.detach();
}

END_NS

BEGIN_NS(Case02)

// https://en.cppreference.com/w/cpp/thread/jthread

// C++20부터는 기존 std::thread를 보강한 std::jthread를 제공한다.
// - std::jthread는 std::thread의 API를 그대로 지원함.
// 
// std::jthread는 RAII가 적용되어 소멸 과정에서 자체적으로 join()를 호출한다.

void Task(int duration)
{
    for (int i = 0; i < duration; i++)
    {
        std::cout << std::format("cnt : {}\n", i);

        std::this_thread::sleep_for(0.2s);
    }
}

void Run()
{
    std::jthread th{ Task, 10 };

    std::cout << "Thread started!\n";

    // 호출하지 않아도 된다.
    // th.join();
    // th.detach();
}

END_NS

BEGIN_NS(Case03)

// https://en.cppreference.com/w/cpp/thread/stop_token

// stop_token을 사용하면 실행 중인 스레드에게 취소 요청을 보낼 수 있다.
// 이를 통해 작업 스레드와 메인 스레드가 협력하여 스레드를 종료하는 로직을 구성할 수 있다.

void Task(std::stop_token token)
{
    // https://en.cppreference.com/w/cpp/thread/stop_callback
    // (선택) stop_token이 활성화될 때 실행할 콜백 함수를 지정하는 것도 가능하다.
    std::stop_callback callback{ token, [] { std::cout << "stop_callback from Task()\n"; } };

    int cnt = 0;

    while (true)
    {
        if (token.stop_requested())
        {
            std::cout << "Stop requested!\n";

            return;
        }

        std::cout << std::format("cnt : {}\n", cnt);
        std::this_thread::sleep_for(1s);

        cnt++;
    }
}

void Run()
{
    // stop_token을 생성하지 않아도 jthread 차원에서 stop_token을 전달한다.
    std::jthread th{ Task };

    // 이런 식으로 jthread에서 stop_token을 꺼내오는 것도 가능하다.
    std::stop_callback callback{ th.get_stop_token(), [] { std::cout << "stop_callback from Run()\n"; } };

    std::cout << "Thread started!\n";

    std::cout << "Press enter to stop the worker thread : \n";
    std::cin.get();

    // (주의) 해당 함수는 stop_token을 활성화할 뿐 작업 스레드를 종료시키진 않는다.
    th.request_stop(); // 주석처리해도 jthread 차원에서 request_stop()을 호출하지만 이 경우 Run()에 있는 callback는 호출하지 않음.
}

END_NS

BEGIN_NS(Case04)

// https://en.cppreference.com/w/cpp/thread/stop_source

// stop_token이 취소 요청을 받기 위한 객체(소비자)라면 stop_source는 취소 요청을 본기 위한 객체(생산자)이다.

void Run()
{
    auto srcInfo = [](const std::stop_source& source) {
        std::printf("stop_source stop_possible = %s, stop_requested = %s\n",
                    source.stop_possible() ? "true" : "false",
                    source.stop_requested() ? "true" : "false");
    };

    auto task = [](std::stop_token token, std::string name) {
        std::stop_callback callback{ token, [] { std::cout << "stop_callback from a lambda task\n"; } };

        int cnt = 0;

        while (true)
        {
            if (token.stop_requested())
            {
                std::cout << "Stop requested!\n";

                return;
            }

            std::cout << std::format("name : {}, cnt : {}\n", name, cnt);
            std::this_thread::sleep_for(1s);

            cnt++;
        }
    };

    // main code
    std::stop_source source;
    srcInfo(source);

    // (주의??) token을 직접 넘기는 방식을 쓰면 명시적으로 request_stop()을 호출해야 한다.
    // token을 직접 넘겼는데 request_stop()를 호출하지 않으면 stop_requested()는 계속 false를 반환한다.
    // 이 사항은 MSVC의 버그일 수도 있으니 나중에 컴파일러 버전이 올라가면 따로 테스트해봐야 한다.
    std::jthread threads[4];
    threads[0] = std::jthread(task, source.get_token(), "worker 1");
    threads[1] = std::jthread(task, source.get_token(), "worker 2");
    threads[2] = std::jthread(task, source.get_token(), "worker 3");
    threads[3] = std::jthread(task, source.get_token(), "worker 4");
    
    std::cout << "Thread started!\n";

    std::cout << "Press enter to stop the worker threads : \n";
    std::cin.get();

    // (주의) 해당 함수는 stop_token을 활성화할 뿐 작업 스레드를 종료시키진 않는다.
    source.request_stop(); // source를 통해 token을 받은 모든 스레드에게 종료 요청을 보낸다.

    srcInfo(source);
}

END_NS

BEGIN_NS(Case05)

void TaskWithInterrupt(std::stop_token token)
{
    int cnt = 0;

    while (cnt < 10)
    {
        if (token.stop_requested())
            return;

        std::this_thread::sleep_for(0.2s);

        std::cout << std::format("with interrupt : {}\n", cnt);

        cnt++;
    }
}

void TaskWithoutInterrupt()
{
    int cnt = 0;

    while (cnt < 10)
    {
        std::this_thread::sleep_for(0.2s);

        std::cout << std::format("without interrupt : {}\n", cnt);

        cnt++;
    }
}

void Run()
{
    // std::jthread interruptible(TaskWithInterrupt);
    // std::jthread nonInterruptible(TaskWithoutInterrupt);

    // MSVC의 버그인지는 모르겠는데 TaskWithInterrupt가 TaskWithoutInterrupt보다 선행된 상태에서
    // request_stop()을 호출하지 않고 jthread의 소멸 단계를 거치면 stop_requested()가 true를 반환하지 않는다.
    // jthread의 선언 순서가 실행 순서에 영향을 미치면 안 되는데 버그 같아 보인다.
    std::jthread nonInterruptible(TaskWithoutInterrupt);
    std::jthread interruptible(TaskWithInterrupt);

    std::this_thread::sleep_for(1.0s);

    // interruptible.request_stop();
    // nonInterruptible.request_stop();
}

END_NS

BEGIN_NS(Case06)

// https://en.cppreference.com/w/cpp/thread#Cooperative_cancellation
// https://en.cppreference.com/w/cpp/thread/condition_variable_any
// https://en.cppreference.com/w/cpp/thread/condition_variable_any/wait_for

// std::condition_variable_any를 통해서도 Cooperative cancellation을 적용할 수 있다.
// wait_for()를 보면 stop_token을 받는 케이스가 있다.

// condition_variable에 대한 내용은 "condition_variable_test_cases.cpp"를 참고하도록 한다.

std::mutex mtx;
std::condition_variable_any cv;

void Task(std::stop_token token)
{
    std::unique_lock lock{ mtx };

    int cnt = 0;

    while (true)
    {
        if (token.stop_requested())
            return;

        // 1. Predicate가 true를 반환하면 wait()를 빠져나오고, Predicate가 false를 반환하면 조건 변수는 대기 상태에 들어간다.
        // 2. 2초 동안 대기 or token이 활성화될 때까지 대기한다.
        cv.wait_for(lock, token, 2s, [] { return false; }); // 조건 변수의 대기 조건이 기준이지 lock을 걸 것인지 걸지 않을 것인지가 기준이 아님.

        // 대기 상태에 들어가면 mutex는 unlock 상태가 된다.
        // 하지만 대기 조건을 빠져나온 상태라면 mutex는 lock 상태가 된다.
        // 따라서 cv.wait_for() 이후는 lock의 소유권을 획득한 상태라고 봐야 한다.
        // 2초 후든 token이 활성화되었든 lock을 소유한 다음 wait_for()를 빠져나온다.

        std::cout << std::format("cnt : {}\n", cnt);

        cnt++;
    }
}

void Run()
{
    // stop_token을 생성하지 않아도 jthread 차원에서 stop_token을 전달한다.
    std::jthread th{ Task };

    // 이런 식으로 jthread에서 stop_token을 꺼내오는 것도 가능하다.
    std::stop_callback callback{ th.get_stop_token(), [] { std::cout << "stop_callback from Run()\n"; } };

    std::cout << "Thread started!\n";

    std::cout << "Press enter to stop the worker thread : \n";
    std::cin.get();

    // (주의) 해당 함수는 stop_token을 활성화할 뿐 작업 스레드를 종료시키진 않는다.
    th.request_stop(); // 주석처리해도 jthread 차원에서 request_stop()을 호출하지만 이 경우 Run()에 있는 callback는 호출하지 않음.
}

END_NS

int main()
{
    // Case01::Run();
    // Case02::Run();
    // Case03::Run();
    // Case04::Run();
    // Case05::Run();
    Case06::Run();

    return 0;
}
