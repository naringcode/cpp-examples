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
// 11. coroutine_exceptions_exec_stages.cpp <-----
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

// 코루틴의 promise_type을 보면 unhandled_exception()이 있는데 코루틴에서 발생하는 모든 예외가 항상 여기로 던져지는 건 아니다.
// 
// 1. 코루틴 생성 과정에서 발생한 예외
// 2. 코루틴 실행 과정에서 발생한 예외
// 
// 두 유형의 예외는 엄연히 다르다.
// 
// 아래 의사코드를 기반으로 예외를 던져서 테스트 해보면 얼추 들어맞는다.
// 
// 의사코드
// Coroutine::RunStateMachine()
// {
//     /************************************************
//     *      코루틴 프레임이 생성된 이후 들어온다.      *
//     ************************************************/
// 
//     // 엄밀히 말하자면 해당 awaiter의 await_resume()에서 발생한 예외는 unhandled_exception()이 받는다.
//     co_await promise.initial_suspend(); // <----- 코루틴 실행의 시작 지점을 제어하여 유예 여부 결정
// 
//     try
//     {
//         coroutine.func();
//     }
//     catch(...)
//     {
//         promise.unhandled_exception();
//     }
//     
//     coroutine.isResumable = false;
//     coroutine.isDone      = true;
// 
//     // (주의) final_suspend()가 호출되어 코루틴이 유예되었을 경우 resume()으로 재개하는 건 불가능하다.
//     // (주의) 이 경우에는 명시적으로 handle.destroy()를 호출해야 한다.
//     co_await promise.final_suspend(); // <----- 코루틴 실행의 종료 지점을 제어하여 유예 여부 결정
// 
//     // handle의 멤버 함수 destroy()와 동일한 작업을 진행하는 함수이다.
//     // 코루틴이 알아서 코루틴 프레임을 해제했으면 사용자는 handle.destroy()를 호출하지 않게 주의해야 한다.
//     coroutine.destroy();
// }
// 
// --------------------------------------------------
// 
// unhandled_exception()은 코루틴을 실행하는 과정에서 던지는 예외만 받을 수 있다.
// 즉, 코루틴 생성 단계과 코루틴 실행 단계는 구분해야 한다.
// 
// final_suspend()는 noexcept 유형으로 작성되어야 하며 소멸자에서 예외를 던지는 건 매우 이상한 상황이다.
// 따라서 코루틴 소멸 단계에서 예외가 발생하는 상황은 고려하지 않아도 된다.
// 
// 사용자는 코루틴 생성 단계와 코루틴 실행 단계만 잘 구분해서 예외처리하면 된다.
// - 코루틴 생성 단계에서 발생하는 예외 : 호출자 쪽에서 try-catch로 받는다.
// - 코루틴 실행 단계에서 발생하는 예외 : 코루틴의 unhandled_exception()로 받는다.
// - 코루틴 소멸 단계에서 예외가 발생하는 건 이상한 상황이니 다루지 않아도 된다.
//   - final_suspend()는 noexcept로 되어있기에 예외를 허용하지 않음.
//   - 소멸자나 operator delete(mem) 같은 곳에서 예외를 던지면 던지지 말라고 컴파일러 경고가 발생함.
// 
// !! 코루틴 생성 단계와 코루틴 실행 단계를 구분하여 작업하는 것이 특히 중요함. !!
// 

// 여기서는 코루틴 실행 단계만 볼 것이다.
// 
// (중요) 코루틴 실행 단계에서 예외를 던지면 할당한 코루틴 자원은 사용자가 해제해야 한다.
// 
// 사용자 쪽에서는 다양한 방식으로 코루틴 자원을 해제할 수 있다.
// - final_suspend()에서 std::suspend_never처럼 종료 지점을 유예하지 않는 awaiter를 반환하기
//   - 이 방식은 unhandled_exception()에서 예외를 던지지 않아야 함.
// 
// - 코루틴 객체 소멸자에서 명시적으로 handle.destroy() 호출하기
// - 외부에서 코루틴 핸들을 보관하고 있다가 특정 시점에 handle.destroy() 호출하기
// 
// 코루틴 실행 단계에서 발생한 예외는 여지없이 promise_type의 unhandled_exception()에 전달된다.
// 정말 특별한 일이 없다면 unhandled_exception()은 코루틴 함수 내에서 발생한 예외로 인해 호출될 것이다.
// 
// 예외를 unhandled_exception()으로 받으면 다양한 방식으로 핸들링 할 수 있다.
// 1. 빈 상태로 두기
// 2. 자원의 일부를 정리하고 플래그 변수를 통해 호출자가 알 수 있게 하기
// 3. 프로그램을 종료하기
// 4. rethrow하여 예외를 호출자에게 전파하기(추천)
// 
// 1번과 2번 방식은 unhandled_exception() 호출을 마친 이후 final_suspend()를 이어서 호출하지만,
// 3번과 4번 방식은 unhandled_exception() 호출 도중에 던지는 방식이라 final_suspend()를 호출하지 않는다.
// 
// (중요) 4번 방식을 제외하면 코루틴에서 발생한 예외는 호출자에게 전파되지 않는다(코루틴은 그 자체로 독립적인 로직으로 간주하기 때문임).
// 
// unhandled_exception()에 들어온 순간부터 handle.done()은 true를 반환한다.
// 
// struct CoroType
// {
//     struct promise_type
//     {
//         ...
//         void unhandled_exception()
//         {
//             std::cout << "\t<unhandled_exception()>\n";
// 
//             // unhandled_exception()이 호출되면 handle.done()은 true를 반환한다.
//             if (coroInst->handle.done())
//             {
//                 std::cout << "\t<handle.done() returns true>\n";
//             }
// 
//             // 일단 예외를 코루틴 내에서만 처리하게 한다(호출자 쪽으로 던지지 않음).
//             // std::rethrow_exception(std::current_exception());
//         }
//         ...
//         CoroType* coroInst;
//     };
//     ...
//     CoroType(promise_type* prom)
//         : handle{ std::coroutine_handle<promise_type>::from_promise(*prom) }
//     {
//         std::cout << "\t<CoroType()>\n";
// 
//         handle.promise().coroInst = this;
//     }
//     ...
//     std::coroutine_handle<promise_type> handle = nullptr;
// };
// 

BEGIN_NS(Case01)

// 코루틴 생성 단계 예외 : 빈 상태로 두기

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

        std::suspend_always initial_suspend() { std::cout << "\t<initial_suspend()>\n"; return { }; }
        std::suspend_always final_suspend() noexcept { std::cout << "\t<final_suspend()>\n"; return { }; }

        void unhandled_exception()
        {
            std::cout << "\t<unhandled_exception()>\n";

            // unhandled_exception()을 빈 상태로 둔다.
            // 이 방식은 어떤 에러가 발생했는지에 대한 정보가 유실되기 때문에 가급적 쓰면 안 된다.
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

        // 생성 단계에서 예외를 던지면 코루틴은 자신이 할당한 메모리를 알아서 해제한다.
        // if (handle != nullptr)
        // {
        //     handle.destroy();
        // 
        //     handle = nullptr;
        // }

        // 코루틴 실행 단계에서 예외를 던지면 할당한 메모리는 사용자가 해제해야 한다.
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

    // 실행 도중 예외 발생
    throw std::logic_error{ "Crash from CoroFunc()" };

    std::cout << "<Coroutine - Returning>\n";
    co_return;
}

void Run()
{
    // 코루틴 생성 단계에서 발생한 예외는 코루틴을 invoke한 쪽에 던져지기 때문에 호출자 쪽의 try-catch 문으로 받아야 한다.
    try
    {
        std::cout << "main - Calling coroutine\n";
        auto task = CoroFunc();

        while (!task.handle.done())
        {
            std::cout << "\nmain - Resuming coroutine\n";
        
            task.handle.resume();
        }

        std::cout << "\nmain - Coroutine is done!\n";
    }
    catch (const std::exception& ex)
    {
        std::cout << "\n#### EXCEPTION THROWN TO MAIN ####\n";
        std::cout << ex.what() << '\n';
        std::cout << "##################################\n";
    }

    // main - Calling coroutine
    //         <operator new(size)>
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType()>
    //         <initial_suspend()>
    // 
    // main - Resuming coroutine
    // <Coroutine - Started>
    // <Coroutine - Executing coroutine function>
    //         <unhandled_exception()>
    //         <final_suspend()> <----- unhandled_exception() 호출을 무사히 마치면 final_suspend()가 호출됨.
    // 
    // main - Coroutine is done!
    //         <~CoroType()> <----- 여기서 handle.destroy()을 호출하고 있음.
    //         <~promise_type()>
    //         <operator delete(mem)>
}

END_NS

BEGIN_NS(Case02)

// 코루틴 생성 단계 예외 : 자원의 일부를 정리하고 플래그 변수를 통해 호출자가 알 수 있게 하기

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

        std::suspend_always initial_suspend() { std::cout << "\t<initial_suspend()>\n"; return { }; }
        std::suspend_always final_suspend() noexcept { std::cout << "\t<final_suspend()>\n"; return { }; }

        void unhandled_exception()
        {
            std::cout << "\t<unhandled_exception()>\n";

            // 예외 플래그 활성화
            coroInst->exceptionOccurred = true;
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

        CoroType* coroInst = nullptr;
    };

    CoroType(promise_type* prom)
        : handle{ std::coroutine_handle<promise_type>::from_promise(*prom) }
    {
        std::cout << "\t<CoroType()>\n";

        // 해당 방식은 코루틴 객체와 promise 객체 간 강한 커플링을 유발한다.
        handle.promise().coroInst = this;
    }

    ~CoroType()
    {
        std::cout << "\t<~CoroType()>\n";

        // 생성 단계에서 예외를 던지면 코루틴은 자신이 할당한 메모리를 알아서 해제한다.
        // if (handle != nullptr)
        // {
        //     handle.destroy();
        // 
        //     handle = nullptr;
        // }

        // 코루틴 실행 단계에서 예외를 던지면 할당한 메모리는 사용자가 해제해야 한다.
        if (handle != nullptr)
        {
            handle.destroy();

            handle = nullptr;
        }
    }

    std::coroutine_handle<promise_type> handle = nullptr;

    bool exceptionOccurred = false;
};

CoroType CoroFunc()
{
    std::cout << "<Coroutine - Started>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    // 실행 도중 예외 발생
    throw std::logic_error{ "Crash from CoroFunc()" };

    std::cout << "<Coroutine - Returning>\n";
    co_return;
}

void Run()
{
    // 코루틴 생성 단계에서 발생한 예외는 코루틴을 invoke한 쪽에 던져지기 때문에 호출자 쪽의 try-catch 문으로 받아야 한다.
    try
    {
        std::cout << "main - Calling coroutine\n";
        auto task = CoroFunc();

        while (!task.handle.done())
        {
            std::cout << "\nmain - Resuming coroutine\n";
        
            task.handle.resume();
        }

        // 예외가 발생했는지 확인
        if (task.exceptionOccurred == true)
        {
            std::cout << "\nmain - Exception occurred in coroutine\n";
        }

        std::cout << "\nmain - Coroutine is done!\n";
    }
    catch (const std::exception& ex)
    {
        std::cout << "\n#### EXCEPTION THROWN TO MAIN ####\n";
        std::cout << ex.what() << '\n';
        std::cout << "##################################\n";
    }

    // main - Calling coroutine
    //         <operator new(size)>
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType()>
    //         <initial_suspend()>
    // 
    // main - Resuming coroutine
    // <Coroutine - Started>
    // <Coroutine - Executing coroutine function>
    //         <unhandled_exception()>
    //         <final_suspend()> <----- unhandled_exception() 호출을 무사히 마치면 final_suspend()가 호출됨.
    // 
    // main - Exception occurred in coroutine <----- 예외 플래그 변수가 활성화된 것을 확인 가능함.
    // 
    // main - Coroutine is done!
    //         <~CoroType()> <----- 여기서 handle.destroy()을 호출하고 있음.
    //         <~promise_type()>
    //         <operator delete(mem)>
}

END_NS

BEGIN_NS(Case03)

// 코루틴 생성 단계 예외 : 프로그램을 종료하기

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

        // 생성 단계에서 예외를 던지면 코루틴은 자신이 할당한 메모리를 알아서 해제한다.
        // if (handle != nullptr)
        // {
        //     handle.destroy();
        // 
        //     handle = nullptr;
        // }

        // 코루틴 실행 단계에서 예외를 던지면 할당한 메모리는 사용자가 해제해야 한다.
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

    // 실행 도중 예외 발생
    throw std::logic_error{ "Crash from CoroFunc()" };

    std::cout << "<Coroutine - Returning>\n";
    co_return;
}

void Run()
{
    // 코루틴 생성 단계에서 발생한 예외는 코루틴을 invoke한 쪽에 던져지기 때문에 호출자 쪽의 try-catch 문으로 받아야 한다.
    try
    {
        std::cout << "main - Calling coroutine\n";
        auto task = CoroFunc();

        while (!task.handle.done())
        {
            std::cout << "\nmain - Resuming coroutine\n";
        
            task.handle.resume();
        }

        std::cout << "\nmain - Coroutine is done!\n";
    }
    catch (const std::exception& ex)
    {
        std::cout << "\n#### EXCEPTION THROWN TO MAIN ####\n";
        std::cout << ex.what() << '\n';
        std::cout << "##################################\n";
    }

    // main - Calling coroutine
    //         <operator new(size)>
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType()>
    //         <initial_suspend()>
    // 
    // main - Resuming coroutine
    // <Coroutine - Started>
    // <Coroutine - Executing coroutine function>
    //         <unhandled_exception()> <----- 여기서 프로그램을 종료함.
}

END_NS

BEGIN_NS(Case04)

// 코루틴 생성 단계 예외 : rethrow하여 예외를 호출자에게 전파하기(추천)

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

        std::suspend_always initial_suspend() { std::cout << "\t<initial_suspend()>\n"; return { }; }
        std::suspend_always final_suspend() noexcept { std::cout << "\t<final_suspend()>\n"; return { }; }

        void unhandled_exception()
        {
            std::cout << "\t<unhandled_exception()>\n";

            // 코루틴 실행 도중 발생한 예외를 호출자에게 전파한다.
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

        // 생성 단계에서 예외를 던지면 코루틴은 자신이 할당한 메모리를 알아서 해제한다.
        // if (handle != nullptr)
        // {
        //     handle.destroy();
        // 
        //     handle = nullptr;
        // }

        // 코루틴 실행 단계에서 예외를 던지면 할당한 메모리는 사용자가 해제해야 한다.
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

    // 실행 도중 예외 발생
    throw std::logic_error{ "Crash from CoroFunc()" };

    std::cout << "<Coroutine - Returning>\n";
    co_return;
}

void Run()
{
    // 코루틴 생성 단계에서 발생한 예외는 코루틴을 invoke한 쪽에 던져지기 때문에 호출자 쪽의 try-catch 문으로 받아야 한다.
    try
    {
        std::cout << "main - Calling coroutine\n";
        auto task = CoroFunc();

        while (!task.handle.done())
        {
            std::cout << "\nmain - Resuming coroutine\n";
        
            task.handle.resume();
        }

        std::cout << "\nmain - Coroutine is done!\n";
    }
    catch (const std::exception& ex)
    {
        std::cout << "\n#### EXCEPTION THROWN TO MAIN ####\n";
        std::cout << ex.what() << '\n';
        std::cout << "##################################\n";
    }

    // main - Calling coroutine
    //         <operator new(size)>
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType()>
    //         <initial_suspend()>
    // 
    // main - Resuming coroutine
    // <Coroutine - Started>
    // <Coroutine - Executing coroutine function>
    //         <unhandled_exception()> <----- unhandled_exception()에서 예외를 던진 것이기 때문에 final_suspend()를 호출하지 않음.
    //         <~CoroType()> <----- try 블록을 빠져나오면서 코루틴 객체의 소멸자 호출(여기서 handle.destroy()을 호출하고 있음)
    //         <~promise_type()>
    //         <operator delete(mem)>
    // 
    // #### EXCEPTION THROWN TO MAIN ####
    // Crash from CoroFunc()
    // ##################################
}

END_NS

int main()
{
    // Case01::Run();
    // Case02::Run();
    // Case03::Run();
    Case04::Run();

    return 0;
}
