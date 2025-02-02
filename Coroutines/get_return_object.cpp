// Update Date : 2025-02-01
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <coroutine>

// 다음 순서대로 보도록 하자.
// 
// # 코루틴 개요
// 1. coroutines_intro.txt
// 
// # 코루틴 구조
// 2. coroutine_infrastructures.cpp
// 3. get_return_object.cpp <-----
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
// 14. coroutine_example_command_with_lambda.cpp
// 15. coroutine_example_coro_from_member_funcs.cpp
//

#define BEGIN_NS(name) namespace name {
#define END_NS };

// https://en.cppreference.com/w/cpp/language/coroutines

BEGIN_NS(Case01)

struct CoroType
{
    struct promise_type
    {
        // promise_type의 포인터로 코루틴 객체를 생성하는 방식
        CoroType get_return_object()
        {
            // return CoroType{ this };
            return this;
        }

        std::suspend_always initial_suspend() { return { }; }
        std::suspend_always final_suspend() noexcept { return { }; }

        void unhandled_exception() { ::terminate(); }

        void return_void() { }
    };

    // get_return_object()에서 코루틴 객체를 구성하여 반환하는 과정에서 호출되는 생성자
    CoroType(promise_type* prom)
        : handle{ std::coroutine_handle<promise_type>::from_promise(*prom) }
    { }

    ~CoroType()
    {
        if (handle != nullptr)
        {
            handle.destroy();

            handle = nullptr;
        }
    }

    std::coroutine_handle<promise_type> handle = nullptr;
};

CoroType CoroFunc()
{
    std::cout << "<Coroutine - Started>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    co_await std::suspend_always{ };
    std::cout << "<Coroutine - Resuming coroutine...>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    std::cout << "<Coroutine - Returning>\n";
    co_return;
}

END_NS

BEGIN_NS(Case02)

struct CoroType
{
    struct promise_type
    {
        // promise_type의 포인터를 디레퍼런싱하여 참조를 통해 생성하는 방식
        CoroType get_return_object()
        {
            // return CoroType{ *this };
            return *this;
        }

        std::suspend_always initial_suspend() { return { }; }
        std::suspend_always final_suspend() noexcept { return { }; }

        void unhandled_exception() { ::terminate(); }

        void return_void() { }
    };

    // get_return_object()에서 코루틴 객체를 구성하여 반환하는 과정에서 호출되는 생성자
    CoroType(promise_type& prom)
        : handle{ std::coroutine_handle<promise_type>::from_promise(prom) }
    { }

    ~CoroType()
    {
        if (handle != nullptr)
        {
            handle.destroy();

            handle = nullptr;
        }
    }

    std::coroutine_handle<promise_type> handle = nullptr;
};

CoroType CoroFunc()
{
    std::cout << "<Coroutine - Started>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    co_await std::suspend_always{ };
    std::cout << "<Coroutine - Resuming coroutine...>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    std::cout << "<Coroutine - Returning>\n";
    co_return;
}

END_NS

BEGIN_NS(Case03)

struct CoroType
{
    struct promise_type
    {
        // get_return_object()에서 핸들을 생성하여 코루틴 객체의 생성자를 호출하는 방식
        CoroType get_return_object()
        {
            return { std::coroutine_handle<promise_type>::from_promise(*this) };
        }

        std::suspend_always initial_suspend() { return { }; }
        std::suspend_always final_suspend() noexcept { return { }; }

        void unhandled_exception() { ::terminate(); }

        void return_void() { }
    };

    // get_return_object()에서 코루틴 객체를 구성하여 반환하는 과정에서 호출되는 생성자
    CoroType(const std::coroutine_handle<promise_type>& handle) // const를 붙여야 함.
        : handle{ handle }
    { }

    ~CoroType()
    {
        if (handle != nullptr)
        {
            handle.destroy();

            handle = nullptr;
        }
    }

    std::coroutine_handle<promise_type> handle = nullptr;
};

CoroType CoroFunc()
{
    std::cout << "<Coroutine - Started>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    co_await std::suspend_always{ };
    std::cout << "<Coroutine - Resuming coroutine...>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    std::cout << "<Coroutine - Returning>\n";
    co_return;
}

END_NS

int main()
{
    std::cout << "main - Calling coroutine\n";

    // auto task = Case01::CoroFunc();
    // auto task = Case02::CoroFunc();
    auto task = Case03::CoroFunc();

    // 코루틴 함수의 작업이 완료될 때까지 계속해서 평가를 진행한다.
    while (!task.handle.done())
    {
        std::cout << "\nmain - Resuming coroutine\n";

        // 유예 지점인 suspension point에서 실행을 재개한다.
        // task.handle(); // task.handle()은 task.handle.resume()과 같은 기능을 수행함.
        task.handle.resume();
    }
    
    std::cout << "\nmain - Coroutine is done!\n";

    return 0;
}
