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
// 9. renew_coroutines_by_move.cpp <-----
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

// 코루틴은 기본적으로 1회성 객체이기 때문에 재사용하는 건 불가능하다.
// 하지만 새로운 코루틴 객체를 생성하여 기존 코루틴 변수에 대입하는 것은 가능하다.
// 
// 이를 위해선 이동 생성자 혹은 이동 대입 연산자를 반드시 정의해야 한다.
// 
// (중요) 이동이 적용되기 이전의 대상이 가진 코루틴 프레임은 반드시 해제되어야 한다(안 그러면 메모리 누수 발생).
//
// 비슷한 이유로 코루틴 핸들을 복사로 받고자 한다면 handle.destroy()를 여러 번 호출하지 않게 자체적인 레퍼런스 카운팅을 구현해야 한다.
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

        // 코루틴 실행 시작 지점을 유예하지 않는다.
        std::suspend_never initial_suspend() { std::cout << "\t<initial_suspend()>\n"; return { }; }
        std::suspend_always final_suspend() noexcept { std::cout << "\t<final_suspend()>\n"; return { }; }

        void unhandled_exception()
        {
            std::cout << "\t<unhandled_exception()>\n";

            std::rethrow_exception(std::current_exception());
        }

        void return_void()
        {
            std::cout << "\t<return_void()>\n";
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
    };

    CoroType()
    {
        std::cout << "\t<CoroType()>\n";
    }

    CoroType(promise_type* prom)
        : handle{ std::coroutine_handle<promise_type>::from_promise(*prom) }
    {
        std::cout << "\t<CoroType(prom)>\n";
    }

    // 이동 생성자 정의
    CoroType(CoroType&& rhs) noexcept // Move Constructor
    {
        std::cout << "\t<CoroType(CoroType&&)>\n";

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
    CoroType& operator=(CoroType&& rhs) noexcept // Move Assignment Operator
    {
        std::cout << "\t<operator=(CoroType&&)>\n";

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

    ~CoroType()
    {
        std::cout << "\t<~CoroType()>\n";

        // 이동으로 소유권을 이전하였으면 handle은 nullptr이어야 한다.
        if (handle != nullptr)
        {
            handle.destroy();

            handle = nullptr;
        }
    }

    std::coroutine_handle<promise_type> handle = nullptr;
};

// 한 번 유예하는 코루틴 함수
CoroType CoroFunc1()
{
    std::cout << "<CoroutineA - Started>\n";
    std::cout << "<CoroutineA - Executing coroutine function>\n";

    std::cout << "<CoroutineA - This is CoroFunc1() - 1>\n";

    co_await std::suspend_always{ };
    std::cout << "<CoroutineA - Resuming coroutine...>\n";
    std::cout << "<CoroutineA - Executing coroutine function>\n";

    std::cout << "<CoroutineA - This is CoroFunc1() - 2>\n";

    std::cout << "<CoroutineA - Returning>\n";
    co_return;
}

// 유예하지 않고 모든 작업을 완료하는 코루틴 함수
CoroType CoroFunc2()
{
    std::cout << "<CoroutineB - Started>\n";
    std::cout << "<CoroutineB - Executing coroutine function>\n";

    std::cout << "<CoroutineB - This is CoroFunc2()>\n";

    std::cout << "<CoroutineB - Returning>\n";
    co_return;
}

int main()
{
    std::cout << "main - Making coroutine\n";
    CoroType task; // 빈 코루틴 변수 생성

    std::cout << "\nmain - Calling coroutine\n";
    task = CoroFunc1(); // 코루틴 할당

    if (task.handle.done())
    {
        std::cout << "\nmain - Coroutine is done!\n";
    }
    else
    {
        std::cout << "\nmain - Coroutine isn't done!\n";
    }

    std::cout << "\nmain - Calling coroutine\n";
    task = CoroFunc2(); // 코루틴 할당

    if (task.handle.done())
    {
        std::cout << "\nmain - Coroutine is done!\n";
    }
    else
    {
        std::cout << "\nmain - Coroutine isn't done!\n";
    }

    // main - Making coroutine
    //         <CoroType()>
    // 
    // main - Calling coroutine
    //         <operator new(size)>
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType(prom)> <----- 코루틴 생성 과정에서 발생한 임시 코루틴 객체의 생성자
    //         <initial_suspend()>
    // <CoroutineA - Started>
    // <CoroutineA - Executing coroutine function>
    // <CoroutineA - This is CoroFunc1() - 1>
    //         <operator=(CoroType&&)> <----- 임시 코루틴 객체의 핸들을 외부 코루틴 변수 task에 이동 대입
    //         <~CoroType()> <----- 코루틴 생성 과정에서 발생한 임시 코루틴 객체의 소멸자
    // 
    // main - Coroutine isn't done!
    // 
    // main - Calling coroutine
    //         <operator new(size)>
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType(prom)> <----- 코루틴 생성 과정에서 발생한 임시 코루틴 객체의 생성자
    //         <initial_suspend()>
    // <CoroutineB - Started>
    // <CoroutineB - Executing coroutine function>
    // <CoroutineB - This is CoroFunc2()>
    // <CoroutineB - Returning>
    //         <return_void()>
    //         <final_suspend()>
    //         <operator=(CoroType&&)> <----- 임시 코루틴 객체의 핸들을 외부 코루틴 변수 task에 이동 대입
    //         <~promise_type()>      <----- 기존 코루틴의 자원을 해제하기 위해 호출한 handle.destroy()로 인한 자원 해제(1)
    //         <operator delete(mem)> <----- 기존 코루틴의 자원을 해제하기 위해 호출한 handle.destroy()로 인한 자원 해제(2)
    //         <~CoroType()> <----- 코루틴 생성 과정에서 발생한 임시 코루틴 객체의 소멸자
    // 
    // main - Coroutine is done!
    //         <~CoroType()> <----- 코루틴 변수 task의 소멸자
    //         <~promise_type()>       <----- handle.destroy()로 인한 자원 해제(1)
    //         <operator delete(mem)>  <----- handle.destroy()로 인한 자원 해제(2)
    // --------------------------------------------------
    // 
    // 코루틴 생성 과정에서 만들어져 반환되는 코루틴 객체를 이동 대입 연산자로 받고 있는 것을 볼 수 있다.
    // 
    // 이전에는 함수의 반환 값이 변수에 직접적으로 반영되는 RVO 방식이 적용되었지만
    // 지금은 변수를 따로 정의하고 코루틴 함수를 invoke하는 부분도 따로 진행하고 있기 때문에 이러한 RVO가 적용되지 않는 것이다.
    // !! 다만 이는 컴파일러마다 차이가 있을 수 있음. !!
    //

    return 0;
}
