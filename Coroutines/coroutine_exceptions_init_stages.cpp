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
// 10. coroutine_exceptions_init_stages.cpp <-----
// 11. coroutine_exceptions_exec_stages.cpp
// 
// # 코루틴 예제
// 12. coroutine_example_task.cpp
// 13. coroutine_example_generator.cpp
// 14. coroutine_example_command_with_lambda.cpp
// 15. coroutine_example_coro_from_member_funcs.cpp
//

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

// 여기서는 코루틴 생성 단계만 볼 것이다.
//
// 코루틴 생성 단계로 간주하는 구간은 다음과 같다.
// - operator new(size)
// - promise_type의 기본 생성자
// - get_return_object()
// - 코루틴 객체 생성자
// - initial_suspend()
// 
// initial_suspend()가 반환하는 awaiter는 코루틴 생성 단계와 코루틴 실행 단계가 혼재되어 있다.
// - await_ready(), await_suspend() 구간에서 발생하는 예외는 코루틴 생성 단계로 보고 호출자 쪽이 받는다.
// - await_resume()까지 오면 코루틴 생성 단계가 아닌 실행 단계로 보기에 여기서 예외가 발생하면 unhandled_exception()으로 받는다.
//   - unhandled_exception()의 실행을 제대로 마치면 final_suspend()를 호출함.
//     - 호출자 쪽에 코루틴 객체를 반환하지 않은 상태인데 await_resume()에서 예외가 발생하면 unhandled_exception()로 받게 됨.
//       - 이 경우에는 호출자 쪽에 코루틴 객체가 반환되지 않았어도 final_suspend()가 호출될 수 있음.
//   - (주의) unhandled_exception()에서 예외를 rethrow하여 전파하면 코루틴 로직 자체를 탈출하기 때문에 이 경우에는 final_suspend()가 호출되지 않음.
// 
// (중요) 시작 지점의 awaiter에서 발생하는 예외는 직관적이지 않아 예외 핸들링하기가 어렵다.
// -> initial_suspend()가 반환하는 awaiter는 예외를 던지지 않게 설계하는 것이 좋음.
// 
// (중요) 코루틴 생성 단계에서 발생하는 예외는 코루틴의 unhandled_exception()이 아닌 호출자 쪽의 try-catch로 받는다.
// 
// (매우 중요) 코루틴 생성 단계에서 예외가 발생하면 코루틴 차원에서 할당한 메모리는 코루틴이 알아서 해제한다.
// 
// 로그로 치면 다음 단계까지가 코루틴 객체 생성 단계라고 볼 수 있다.

// --------------------------------------------------
// main - Calling coroutine
//         <operator new(size)>
//         <promise_type()>
//         <get_return_object()>
//         <CoroType()>
//         <initial_suspend()>
//         <MyAwaiter - await_ready()>
//         <MyAwaiter - await_suspend()>
// ...
// --------------------------------------------------
// 
// 코루틴 생성 단계에서 예외를 던지면 이를 받는 건 promise_type의 unhandled_exception()이 아니다.
// 
// 코루틴 생성 단계에서 발생한 예외는 코루틴을 invoke한 쪽에 던져지기 때문에 호출자 쪽의 try-catch 문으로 받아야 한다.
// 

enum class InitStage
{
    OperatorNew,
    PromiseTypeConstructor,
    GetReturnObject,
    CoroTypeConstructor,
    InitialSuspend,
    AwaitReady,
    AwaitSuspend,
    AwaitResume
};

// 컴파일 분기를 나누기 위한 상수
constexpr InitStage g_kInitStage = InitStage::AwaitResume;

struct MyAwaiter // 직접 구성한 Awaiter
{
    bool await_ready()
    {
        std::cout << "\t<MyAwaiter - await_ready()>\n";

        if constexpr (g_kInitStage == InitStage::AwaitReady)
        {
            throw std::logic_error{ "Crash from await_ready()" };
        }

        return false;
    }

    // 모든 코루틴 핸들은 std::coroutine_handle<>로 변환할 수 있다(convertible).
    bool await_suspend(std::coroutine_handle<> handle)
    {
        std::cout << "\t<MyAwaiter - await_suspend()>\n";

        if constexpr (g_kInitStage == InitStage::AwaitSuspend)
        {
            throw std::logic_error{ "Crash from await_suspend()" };
        }

        return false;
    }

    void await_resume()
    {
        std::cout << "\t<MyAwaiter - await_resume()>\n";

        if constexpr (g_kInitStage == InitStage::AwaitResume)
        {
            throw std::logic_error{ "Crash from await_resume()" };
        }
    }
};

struct CoroType
{
    struct promise_type
    {
        promise_type()
        {
            std::cout << "\t<promise_type()>\n";

            if constexpr (g_kInitStage == InitStage::PromiseTypeConstructor)
            {
                throw std::logic_error{ "Crash from promise_type()" };
            }
        }

        ~promise_type() { std::cout << "\t<~promise_type()>\n"; }

        CoroType get_return_object()
        {
            std::cout << "\t<get_return_object()>\n";

            if constexpr (g_kInitStage == InitStage::GetReturnObject)
            {
                throw std::logic_error{ "Crash from get_return_object()" };
            }

            // return CoroType{ this };
            return this;
        }

        MyAwaiter initial_suspend()
        {
            std::cout << "\t<initial_suspend()>\n";
            
            if constexpr (g_kInitStage == InitStage::InitialSuspend)
            {
                throw std::logic_error{ "Crash from initial_suspend()" };
            }

            return { };
        }

        std::suspend_always final_suspend() noexcept { std::cout << "\t<final_suspend()>\n"; return { }; }

        void unhandled_exception()
        {
            std::cout << "\t<unhandled_exception()>\n";

            // 일단 예외를 코루틴 내에서만 처리하게 한다(호출자 쪽으로 던지지 않음).
            // std::rethrow_exception(std::current_exception());
        }

        void return_void()
        {
            std::cout << "\t<return_void()>\n";
        }

        void* operator new(std::size_t size)
        {
            std::cout << "\t<operator new(size)>\n";

            if constexpr (g_kInitStage == InitStage::OperatorNew)
            {
                throw std::logic_error{ "Crash from operator new(size)" };
            }
        
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

        if constexpr (g_kInitStage == InitStage::CoroTypeConstructor)
        {
            throw std::logic_error{ "Crash from CoroType()" };
        }
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
    }

    std::coroutine_handle<promise_type> handle = nullptr;
};

CoroType CoroFunc()
{
    std::cout << "<Coroutine - Started>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    co_await std::suspend_always{ }; // <----- 여기를 유예 지점(suspension point)으로 함.
    std::cout << "<Coroutine - Resuming coroutine...>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    std::cout << "<Coroutine - Returning>\n";
    co_return;
}

int main()
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

#pragma region 코루틴 생성 단계 예외 : operator new(size)
    // --------------------------------------------------
    // main - Calling coroutine
    //         <operator new(size)> <----- 여기서 예외 발생
    // 
    // #### EXCEPTION THROWN TO MAIN ####
    // Crash from operator new(size)
    // ##################################
    // --------------------------------------------------
#pragma endregion

#pragma region 코루틴 생성 단계 예외 : promise_type 기본 생성자
    // --------------------------------------------------
    // main - Calling coroutine
    //         <operator new(size)>
    //         <promise_type()> <----- 여기서 예외 발생
    //         <operator delete(mem)> <----- 코루틴 차원에서 호출
    //
    // #### EXCEPTION THROWN TO MAIN ####
    // Crash from promise_type()
    // ##################################
    // --------------------------------------------------
#pragma endregion

#pragma region 코루틴 생성 단계 예외 : get_return_object()
    // --------------------------------------------------
    // main - Calling coroutine
    //         <operator new(size)>
    //         <promise_type()>
    //         <get_return_object()> <----- 여기서 예외 발생
    //         <~promise_type()>       <----- 코루틴 차원에서 호출
    //         <operator delete(mem)>  <----- 코루틴 차원에서 호출
    // 
    // #### EXCEPTION THROWN TO MAIN ####
    // Crash from get_return_object()
    // ##################################
    // --------------------------------------------------
#pragma endregion

#pragma region 코루틴 생성 단계 예외 : 코루틴 객체 생성자
    // --------------------------------------------------
    // main - Calling coroutine
    //         <operator new(size)>
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType()> <----- 여기서 예외 발생
    //         <~promise_type()>      <----- 코루틴 차원에서 호출
    //         <operator delete(mem)> <----- 코루틴 차원에서 호출
    // 
    // #### EXCEPTION THROWN TO MAIN ####
    // Crash from CoroType()
    // ##################################
    // --------------------------------------------------
#pragma endregion

#pragma region 코루틴 생성 단계 예외 : initial_suspend()
    // --------------------------------------------------
    // main - Calling coroutine
    //         <operator new(size)>
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType()>
    //         <initial_suspend()> <----- 여기서 예외 발생
    //         <~CoroType()>          <----- 코루틴 차원에서 호출
    //         <~promise_type()>      <----- 코루틴 차원에서 호출
    //         <operator delete(mem)> <----- 코루틴 차원에서 호출
    // 
    // #### EXCEPTION THROWN TO MAIN ####
    // Crash from initial_suspend()
    // ##################################
    // --------------------------------------------------
#pragma endregion

#pragma region 코루틴 생성 단계 예외 : initial_suspend() -> awaiter -> await_ready()
    // --------------------------------------------------
    // main - Calling coroutine
    //         <operator new(size)>
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType()>
    //         <initial_suspend()>
    //         <MyAwaiter - await_ready()> <----- 여기서 예외 발생
    //         <~CoroType()>          <----- 코루틴 차원에서 호출
    //         <~promise_type()>      <----- 코루틴 차원에서 호출
    //         <operator delete(mem)> <----- 코루틴 차원에서 호출
    // 
    // #### EXCEPTION THROWN TO MAIN ####
    // Crash from await_ready()
    // ##################################
    // --------------------------------------------------
#pragma endregion

#pragma region 코루틴 생성 단계 예외 : initial_suspend() -> awaiter -> await_suspend()
    // --------------------------------------------------
    // main - Calling coroutine
    //         <operator new(size)>
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType()>
    //         <initial_suspend()>
    //         <MyAwaiter - await_ready()>
    //         <MyAwaiter - await_suspend()> <----- 여기서 예외 발생
    //         <~CoroType()>          <----- 코루틴 차원에서 호출
    //         <~promise_type()>      <----- 코루틴 차원에서 호출
    //         <operator delete(mem)> <----- 코루틴 차원에서 호출
    // 
    // #### EXCEPTION THROWN TO MAIN ####
    // Crash from await_suspend()
    // ##################################
    // --------------------------------------------------
#pragma endregion

#pragma region (주의) 다음 상황은 코루틴 생성 단계가 아님 : initial_suspend() -> awaiter -> await_resume()
    // awaiter가 해당 순서로 실행되면 코루틴 객체가 호출자에게 반환되지 않았어도
    // await_resume() 단계는 코루틴 생성 단계가 아닌 코루틴 실행 단계로 본다.
    // --------------------------------------------------
    // main - Calling coroutine
    //         <operator new(size)>
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType()>
    //         <initial_suspend()>
    //         <MyAwaiter - await_ready()>
    //         <MyAwaiter - await_suspend()>
    //         <MyAwaiter - await_resume() <----- 여기서 예외 발생
    //         <unhandled_exception()>
    //         <final_suspend()> <----- 종료 지점에서 코루틴이 유예되기에 자원은 아직 해제되지 않음.
    // 
    // main - Coroutine done!
    //         <~CoroType()> <----- handle.destroy()를 호출하지 않음.
    // --------------------------------------------------
    // 
    // 코루틴 실행 단계에서 던진 예외는 코루틴이 받지만 자원의 해제까지 담당하는 건 아니다.
    // 코루틴 실행 단계에서 예외를 던지면 할당된 코루틴 자원을 해제하는 것에 대한 책임은 사용자가 진다.
    // 
    // 1. 명시적으로 handle.destroy()를 호출하기
    // 2. final_suspend()가 반환하는 awaiter가 종료 지점에서 코루틴을 유예시키기 않게 코드를 작성하기
    //   - (주의) unhandled_exception()에서 예외를 던지면 final_suspend()가 호출되지 않으니 주의해야 함.
    // 
#pragma endregion

    return 0;
}
