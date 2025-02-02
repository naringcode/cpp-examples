// Update Date : 2025-02-02
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <coroutine>
#include <concepts>

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
// 13. coroutine_example_generator.cpp <-----
// 14. coroutine_example_command_with_lambda.cpp
// 15. coroutine_example_coro_from_member_funcs.cpp
//

// https://en.cppreference.com/w/cpp/language/coroutines

// 대부분의 경우 코루틴은 Task 유형과 Generator 유형으로 구분된다.
// 
// 세부적으로 볼 때 두 유형에 포함되지 않더라도 넓은 관점으로 보면 Task 유형과 Generator 유형에 포함되는 경우가 많다.

// Task 유형에 해당하면 코루틴 타입 이름에 Task를 붙이는 것이 좋고,
// Generator 유형에 해당하면 Generator를 붙이는 것이 좋다.
//
// Task 유형 : co_await, co_yield를 활용하여 비동기 작업을 수행하는 유형의 코루틴이다.
// Generator 유형 : 코루틴을 재개할 때마다 일정한 패턴으로 값을 생성하는 유형의 코루틴이다.
// 

template <typename T>
class CoroGenerator
{
public:
    struct promise_type
    {
        CoroGenerator<T> get_return_object()
        {
            // return CoroGenerator<T>{ this };
            return this;
        }

        std::suspend_never initial_suspend() { return { }; }
        std::suspend_always final_suspend() noexcept { return { }; }

        void unhandled_exception()
        {
            std::rethrow_exception(std::current_exception());
        }

        void return_void()
        { }

        std::suspend_always yield_value(T value)
        {
            this->value = value;

            return { };
        }

        T value;
    };

public:
    CoroGenerator(promise_type* prom)
        : _handle{ std::coroutine_handle<promise_type>::from_promise(*prom) }
    { }

    ~CoroGenerator()
    {
        if (_handle != nullptr)
        {
            _handle.destroy();

            _handle = nullptr;
        }
    }

public:
    bool IsDone()
    {
        return _handle.done();
    }

public:
    // resume()으로 실행을 재개하고 값까지 산출해서 반환한다.
    T operator()()
    {
        T temp = _handle.promise().value;

        if (_handle.done() == false)
        {
            _handle.resume();
        }

        return temp;
    }

private:
    std::coroutine_handle<promise_type> _handle = nullptr;
};

CoroGenerator<int> CoroFibonacci()
{
    int x = 0;
    int y = 1;

    while (true)
    {
        co_yield x;

        int next = x + y;

        x = y;
        y = next;
    }

    // co_yield를 사용하고 있기 때문에 co_return은 생략해도 된다.
    // co_return;
}

CoroGenerator<int> CoroRange(int start, int end)
{
    while (start < end)
    {
        co_yield start++;
    }

    // co_yield를 사용하고 있기 때문에 co_return은 생략해도 된다.
    // co_return;
}

int main()
{
    auto gen = CoroFibonacci();

    for (int i = 0; i < 20; i++)
    {
        std::cout << "[" << i << "] : " << gen() << '\n';
    }

    std::cout << "--------------------------------------------------\n";

    auto gen2 = CoroRange(10, 20);

    while (!gen2.IsDone())
    {
        std::cout << "yield : " << gen2() << '\n';
    }

    return 0;
}
