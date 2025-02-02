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
// 8. parameters_and_local_variables_in_coroutines.cpp <-----
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

// @ 코루틴 프레임(코루틴 상태) 소멸 단계
// 
// - final_suspend()에서 반환한 awaiter가 코루틴 종료 지점을 유예
//   -> promise_type 소멸자 호출
//   -> 매개변수의 소멸자 호출
//   -> promise_type의 operator delete(mem)를 호출하여 코루틴 프레임을 해제
// 
// - 코루틴 함수가 종료된 상황에서 handle.destroy() 호출
//   -> promise_type 소멸자 호출
//   -> 매개변수의 소멸자 호출
//   -> promise_type의 operator delete(mem)를 호출하여 코루틴 프레임을 해제
// 
// - 코루틴 함수가 "종료되지 않은" 상태에서 handle.destroy() 호출
//   -> "유효한 지역 변수의 소멸자 호출"
//   -> promise_type 소멸자 호출
//   -> 매개변수의 소멸자 호출
//   -> promise_type의 operator delete(mem)를 호출하여 코루틴 프레임을 해제
// 
// 위에 보이는 건 코루틴 생명주기를 설명하면서 언급한 내용이다.
// 
// 여기서 우리가 확인해볼 것은 3번째 유형이다.
// - 코루틴 함수가 "종료되지 않은" 상태에서 handle.destroy() 호출
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

        // 코루틴 객체 생성 후 함수 실행까지 진행하기 위해 std::suspend_never를 반환
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

struct FooObject
{
    int value;

    FooObject(int value)
        : value{ value }
    {
        std::cout << "\t<FooObject()> : " << value << "\n";
    }

    ~FooObject()
    {
        std::cout << "\t<~FooObject()> : " << value << "\n";
    }

    FooObject(const FooObject& rhs)
    {
        value = rhs.value + 1000; // 식별하기 위해 이렇게 함.

        std::cout << "\t<FooObject(const FooObject& rhs)> - copy : " << value << "\n";
    }

    FooObject(FooObject&& rhs) noexcept
    {
        value = rhs.value + 2000; // 식별하기 위해 이렇게 함.

        std::cout << "\t<FooObject(FooObject&& rhs)> - move : " << value << "\n";
    }
};

CoroType CoroFunc(FooObject param)
{
    std::cout << "<Coroutine - Started>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    FooObject local1{ 200 };

    co_await std::suspend_always{ }; // <----- 여기를 유예 지점(suspension point)으로 함.
    std::cout << "<Coroutine - Resuming coroutine...>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    FooObject local2{ 300 };

    std::cout << "<Coroutine - Returning>\n";
    co_return;
}

int main()
{
    std::cout << "main - Calling coroutine\n";
    auto task = CoroFunc(FooObject{ 100 });

    // 코루틴 함수가 종료되지 않은 상황을 유도하기 위해 재개해선 안 된다.
    // std::cout << "\nmain - Resuming coroutine\n";
    // task.handle.resume();

    // 코루틴 핸들의 done()을 통해서 코루틴 함수의 완료 여부를 확인할 수 있다.
    if (task.handle.done())
    {
        std::cout << "\nmain - Coroutine is done!\n";
    }
    else
    {
        std::cout << "\nmain - Coroutine isn't done!\n";
    }

    // main - Calling coroutine
    //         <FooObject()> : 100 <----- 함수의 인자로 전달하기 위한 임시 객체의 생성자 호출
    //         <operator new(size)>
    //         <FooObject(FooObject&& rhs)> - move : 2100 <----- 코루틴 프레임이 할당되었으면 함수의 매개변수에 대한 소유권을 이전
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType()>
    //         <initial_suspend()>
    // <Coroutine - Started>
    // <Coroutine - Executing coroutine function>
    //         <FooObject()> : 200 <----- 코루틴 함수 내 지역 변수의 생성자 호출
    //         <~FooObject()> : 100 <----- 최초 실행이 끝났으니 임시 객체의 소멸자 호출
    // 
    // main - Coroutine isn't done!
    //         <~CoroType()>
    //         <~FooObject()> : 200 <----- destroy()를 호출하면 먼저 유효한 지역 변수의 소멸자 호출
    //         <~promise_type()>
    //         <~FooObject()> : 2100 <----- 코루틴 프레임을 해제하기 직전에 매개변수의 소멸자 호출
    //         <operator delete(mem)>

    return 0;
}
