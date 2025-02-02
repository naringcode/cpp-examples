// Update Date : 2025-02-02
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <coroutine>
#include <chrono>
#include <thread>
#include <list>

using namespace std::chrono_literals;

// 다음 순서대로 보도록 하자.
// 
// # 코루틴 개요
// 1. coroutines_intro.txt
// 
// # 코루틴 구조
// 2. coroutine_infrastructures.cpp
// 3. get_return_object.cpp
// 
// # Awaitable & Awaiter
// 4. awaitable_and_awaiter.cpp
// 5. custom_awaiters.cpp
// 
// # 값 산출
// 6. get_value_from_co_return_co_yield.cpp
// 
// # 코루틴 생명주기
// 7. coroutine_lifecycle.cpp
// 8. parameters_and_local_variables_in_coroutines.cpp
// 9. renew_coroutines_by_move.cpp
// 
// # 코루틴 예외 처리
// 10. coroutine_exceptions_init_stages.cpp
// 11. coroutine_exceptions_exec_stages.cpp
// 
// # 코루틴 예제
// 12. coroutine_example_task.cpp
// 13. coroutine_example_generator.cpp
// 14. coroutine_example_command_with_lambda.cpp <-----
// 15. coroutine_example_coro_from_member_funcs.cpp
//

// https://en.cppreference.com/w/cpp/language/coroutines

// 람다로 코루틴 함수를 구성하는 것도 가능하다.
//
// 나중에 Command 패턴을 적용한 JobQueue를 코루틴으로 만들 때 참고하도록 하자.

struct LambdaTask
{
    struct promise_type
    {
        promise_type() { std::cout << "\t<promise_type()>\n"; }
        ~promise_type() { std::cout << "\t<~promise_type()>\n"; }

        LambdaTask get_return_object()
        {
            std::cout << "\t<get_return_object()>\n";

            // return Task{ this };
            return this;
        }

        std::suspend_always initial_suspend() { std::cout << "\t<initial_suspend()>\n"; return { }; }
        std::suspend_always final_suspend() noexcept { std::cout << "\t<final_suspend()>\n"; return { }; }

        void unhandled_exception()
        {
            std::cout << "\t<unhandled_exception()>\n";

            ::terminate();
        }

        void return_void()
        {
            std::cout << "\t<return_void()>\n";
        }

        auto yield_value(std::chrono::milliseconds durationMs)
        {
            std::cout << "\t<yield_value(T)>\n";

            this->durationMs = durationMs;

            return std::suspend_always{ };
        }

        void* operator new(std::size_t size)
        {
            std::cout << "\t<operator new(size)>\n";

            if (void* mem = std::malloc(size))
                return mem;

            return nullptr;
        }

        void operator delete(void* mem)
        {
            std::cout << "\t<operator delete(mem)>\n";

            std::free(mem);
        }

        std::chrono::milliseconds durationMs = 0ms;
    };

    LambdaTask(promise_type* prom)
        : handle{ std::coroutine_handle<promise_type>::from_promise(*prom) }
    {
        std::cout << "\t<LambdaTask()>\n";
    }

    
    // 이동 생성자 정의
    LambdaTask(LambdaTask&& rhs) noexcept // Move Constructor
    {
        std::cout << "\t<LambdaTask(LambdaTask&&)>\n";

        if (this != &rhs)
        {
            // 무언가 하고 있던 코루틴이라면 명시적으로 handle.destroy()를 호출
            if (handle != nullptr)
            {
                handle.destroy();

                handle = nullptr;
            }

            std::swap(handle, rhs.handle);
        }
    }

    // 이동 대입 연산자 정의
    LambdaTask& operator=(LambdaTask&& rhs) noexcept // Move Assignment Operator
    {
        std::cout << "\t<operator=(LambdaTask&&)>\n";

        if (this != &rhs)
        {
            // 무언가 하고 있던 코루틴이라면 명시적으로 handle.destroy()를 호출
            if (handle != nullptr)
            {
                handle.destroy();

                handle = nullptr;
            }

            std::swap(handle, rhs.handle);
        }

        return *this;
    }

    ~LambdaTask()
    {
        std::cout << "\t<~LambdaTask()>\n";

        if (handle != nullptr)
        {
            handle.destroy();

            handle = nullptr;
        }
    }

    std::coroutine_handle<promise_type> handle = nullptr;
};

int main()
{
    std::list<LambdaTask> commandList;
    
    commandList.push_back(
        []() -> LambdaTask {
            std::cout << "<First - This is the first lambda>\n";

            std::cout << "<First - Waiting for 2s>\n";
            co_yield 2s;

            std::cout << "<First - Waiting for 2s>\n";
            co_yield 2s;
        }());
    
    std::cout << '\n';

    commandList.push_back(
        []() -> LambdaTask {
            std::cout << "<Second - This is the second lambda>\n";

            std::cout << "<Second - Waiting for 3s>\n";
            co_yield 3s;

            std::cout << "<Second - Waiting for 3s>\n";
            co_yield 3s;
        }());
    
    std::cout << '\n';

    commandList.push_back(
        []() -> LambdaTask {
            std::cout << "<Third - This is the third lambda>\n";

            std::cout << "<Third - Waiting for 4s>\n";
            co_yield 4s;

            std::cout << "<Third - Waiting for 4s>\n";
            co_yield 4s;
        }());

    while (commandList.size() > 0)
    {
        auto& task = *commandList.begin();

        while (!task.handle.done())
        {
            std::this_thread::sleep_for(task.handle.promise().durationMs);

            std::cout << "\nmain - Resuming coroutine\n";
            task.handle.resume();
        }

        commandList.pop_front();
    }

    return 0;
}
