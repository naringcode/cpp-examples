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
// 3. get_return_object.cpp
// 
// # Awaitable & Awaiter
// 4. awaitable_and_awaiter.cpp <-----
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

// https://en.cppreference.com/w/cpp/language/coroutines#co_await

// # co_await expr;
//
// co_await는 코루틴 유예 여부를 결정하고 제어권을 호출자나 다른 코루틴으로 넘길 때 사용하는 키워드이다.
// co_await는 unary 연산자로 취급하기에 연산자 오버로딩이 가능하다.
//
// --------------------------------------------------
//
// # Awaitable
// 
// awaitable은 co_await의 피연산자로 전달된 식이나 대상을 말한다.
// 
// 1. promise_type 내 await_transform(T)이 정의되어 있는지 확인
// 2. 피연산자가 되는 awaitable 객체에 operator co_await()가 정의되어 있는지 확인
// 3. awaitable이 awaiter로 직접 지정된 것인지 확인
// 
// 다음 3가지 중 하나라도 해당한다면 컴파일러는 전달된 식을 awaitable이라 판단한다.
// 
// 1번과 2번은 awaitable을 통해 awaiter를 반환하여 co_await에 awaiter를 전달하는 방식이지만,
// 3번은 co_await 연산자에 awaiter가 직접 지정되는 방식이다.
// 
// (주의) 2번과 3번은 서로 혼용해서 사용할 수 있지만 promise_type 내 await_transform(T)을 지정하는 1번 방식을 쓰면 다른 유형은 사용할 수 없다.
// 
// 위 3가지 사항 중 만족하는 유형이 단 하나라도 없으면 컴파일 에러가 발생한다.
// 
// --------------------------------------------------
//
// # Awaiter
// 
// awaitable은 co_await 연산에 대한 입력 대상일 뿐 실질적으로 co_await에 적용되는 건 awaiter이다.
// 
// awaiter를 통해 코루틴 유예 여부를 제어할 수 있기에 awaiter에는 정해진 함수가 정의되어 있어야 한다.
// 
// C++이 기본적으로 제공하는 내장된 awaiter로는 std::suspend_always와 std::suspend_never가 있다.
// - std::suspend_always : 유예 지점에서 실행 중단
// - std::suspend_never : 유예 지점에서 실행을 중단하지 않고 그대로 재개
// 

BEGIN_NS(Case01)

// # promise_type 내 await_transform(T)가 정의되어 있는지 확인하는 유형
//
// 컴파일러가 가장 먼저 확인하는 유형이다.
// 코루틴의 promise_type에 await_transform(T)가 멤버 함수로 정의되어 있다면 해당 유형을 사용한다.
// 
// co_await <value>로 작성된 코드에서 값은 promise.await_transform(<value>)로 변환된다.
// co_await <value>; // co_await promise.await_transform(<value>);
// 
// await_transform(T)는 awaiter를 반환해야 한다.
//
struct CoroType
{
    struct promise_type
    {
        promise_type() { std::cout << "\t<promise_type()>\n"; }
        ~promise_type() { std::cout << "\t<~promise_type()>\n"; }

        CoroType get_return_object()
        {
            std::cout << "\t<get_return_object()>\n";

            // return CoroType{ this };
            return this;
        }

        // 코루틴 실행 시작과 마지막 지점에서 코루틴을 유예한다.
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

        // "co_await 0;"에 의해 호출될 멤버 함수(함수 이름은 반드시 await_transform이어야 함).
        auto await_transform(int value)
        {
            std::cout << "\t<await_transform(T)>\n";

            // await_transform(T)는 awaiter를 반환해야 한다.
            // std::suspend_always를 반환하면 co_await 연산이 발생한 지점에서 코루틴 실행을 유예한다.
            return std::suspend_always{ };
        }
    };

    CoroType(promise_type* prom)
        : handle{ std::coroutine_handle<promise_type>::from_promise(*prom) }
    {
        std::cout << "\t<CoroType()>\n";
    }

    ~CoroType()
    {
        std::cout << "\t<~CoroType()>\n";

        if (handle != nullptr)
        {
            handle.destroy();

            handle = nullptr;
        }
    }

    // 코루틴 핸들은 코루틴 프레임을 가리키기 위한 용도로 사용된다.
    std::coroutine_handle<promise_type> handle = nullptr;
};

CoroType CoroFunc()
{
    std::cout << "\n<Coroutine - Started>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    // 컴파일 단계에서 
    // 1. promise.await_transform(0);으로 변환된다.
    // 2. await_transform(0)에서 awaiter를 반환한다.
    // 3. co_await std::suspend_always{ };로 최종 적용된다.
    co_await 0; // co_await promise.await_transform(0);
    std::cout << "<Coroutine - Resuming coroutine...>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";
    
    std::cout << "<Coroutine - Returning>\n";
    co_return;
}

END_NS

BEGIN_NS(Case02)

// # 피연산자가 되는 awaitable 객체에 operator co_await()가 정의되어 있는지 확인하는 유형
// 
// awaitable에 연산자 co_await를 오버로딩한 operator co_await()가 정의되어 있으면 해당 유형을 사용한다.
//
// co_await awaitable이라는 코드는 다음과 같이 변환된다.
// co_await awaitable; // co_await awaitable.operator co_await();
// 
// 마찬가지로 operator co_await() 또한 awaiter를 반환해야 한다.
//
struct CoroType
{
    struct promise_type
    {
        promise_type() { std::cout << "\t<promise_type()>\n"; }
        ~promise_type() { std::cout << "\t<~promise_type()>\n"; }

        CoroType get_return_object()
        {
            std::cout << "\t<get_return_object()>\n";

            // return CoroType{ this };
            return this;
        }

        // 코루틴 실행 시작과 마지막 지점에서 코루틴을 유예한다.
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
    };

    CoroType(promise_type* prom)
        : handle{ std::coroutine_handle<promise_type>::from_promise(*prom) }
    {
        std::cout << "\t<CoroType()>\n";
    }

    ~CoroType()
    {
        std::cout << "\t<~CoroType()>\n";

        if (handle != nullptr)
        {
            handle.destroy();

            handle = nullptr;
        }
    }

    std::coroutine_handle<promise_type> handle = nullptr;
};

// co_await에 전달하기 위한 awaitable 타입(awaiter 타입 아님)
struct MyAwaitable
{
    auto operator co_await()
    {
        std::cout << "\t<operator co_await()>\n";

        // co_await 연산자를 오버로딩하면 반드시 awaiter를 반환해야 한다.
        // co_await 연산자를 호출한 지점에 유예하기 위해 std::suspend_always를 반환한다.
        return std::suspend_always{ };
    }
};

CoroType CoroFunc()
{
    std::cout << "\n<Coroutine - Started>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    // 컴파일 단계에서
    // 1. awaitable.operator co_await();로 변환된다.
    // 2. awaitable.operator co_await();는 awaiter를 반환한다.
    // 3. co_await std::suspend_always{ };로 최종 적용된다.
    co_await MyAwaitable{ };
    std::cout << "<Coroutine - Resuming coroutine...>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";
    
    std::cout << "<Coroutine - Returning>\n";
    co_return;
}

END_NS

BEGIN_NS(Case03)

// # awaitable이 awaiter로 직접 지정된 것인지 확인하는 유형
// 
// Case01 유형도 Case02 유형도 아닐 경우 컴파일러가 확인하는 유형으로
// 직접적으로 awaiter를 피연산자로 사용하는 방식이다.
// 
// 내장 awaiter를 적용한 예시 코드를 보면 이해하기 쉽다.
// co_await std::suspend_always{ }; // 코루틴을 유예함.
// co_await std::suspend_never{ }; // 코루틴을 유예하지 않음.
// 
struct CoroType
{
    struct promise_type
    {
        promise_type() { std::cout << "\t<promise_type()>\n"; }
        ~promise_type() { std::cout << "\t<~promise_type()>\n"; }

        CoroType get_return_object()
        {
            std::cout << "\t<get_return_object()>\n";

            // return CoroType{ this };
            return this;
        }

        // 코루틴 실행 시작과 마지막 지점에서 코루틴을 유예한다.
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
    };

    CoroType(promise_type* prom)
        : handle{ std::coroutine_handle<promise_type>::from_promise(*prom) }
    {
        std::cout << "\t<CoroType()>\n";
    }

    ~CoroType()
    {
        std::cout << "\t<~CoroType()>\n";

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
    std::cout << "\n<Coroutine - Started>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    // co_await MyAwaiter{ }; // awaiter가 되기 위한 조건을 충족하면 사용자 정의 awaiter도 사용 가능
    // co_await std::suspend_never{ };
    co_await std::suspend_always{ }; // 여기서 코루틴 실행을 유예함.

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
