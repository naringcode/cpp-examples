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
// 7. coroutine_lifecycle.cpp <-----
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

// https://en.cppreference.com/w/cpp/language/coroutines#Execution

// coroutines 문서의 "When a coroutine begins execution, it performs the following:" 쪽에 자세하게 설명되어 있다.
// 
// 문서를 보면 코루틴의 생명주기 자체는 나오지 않지만 나름대로 분석해서 정리하였다.
// 코루틴이 다양한 상황에서 유연하게 동작하는 만큼 생명주기 또한 복잡하다.
//
// (중요) 사용자가 다루는 코루틴 타입 객체와 핸들을 통해 관리되는 코루틴 프레임을 구분해서 봐야 한다.
// 
// 의사코드
// Coroutine::RunStateMachine()
// {
//     /************************************************
//     *      코루틴 프레임이 생성된 이후 들어온다.      *
//     ************************************************/
// 
//     // 엄밀히 말하자면 해당 awaiter의 await_resume()에서 발생한 예외는 unhandled_exception()이 받는다.
//     co_await promise.initial_suspend(); // <----- 코루틴 실행의 시작 지점
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
//     co_await promise.final_suspend(); // <----- 코루틴 실행의 종료 지점
// 
//     // handle의 멤버 함수 destroy()와 동일한 작업을 진행하는 함수이다.
//     // 코루틴이 알아서 코루틴 프레임을 해제했으면 사용자는 handle.destroy()를 호출하지 않게 주의해야 한다.
//     coroutine.destroy();
// }
// 
// --------------------------------------------------
// 
// @ 코루틴 객체 생성 단계
// 
// 1. 코루틴 함수 invoke
// 2. promise_type의 operator new(size)으로 코루틴 프레임(코루틴 상태) 생성
// 3. 매개변수에 이동(move)을 적용하여 코루틴 프레임에 저장
// 4. promise_type 기본 생성자 호출
// 5. get_return_object()를 호출하여 코루틴 객체 생성(이 과정에서 코루틴 객체의 생성자 호출)
// 6. initial_suspend()를 호출하여 awaiter 반환
// 
// 코루틴 작동 방식은 복잡하지만 적어도 코루틴 객체 생성 단계 만큼은 정형화된 절차를 따른다.
// 
// --------------------------------------------------
// 
// @ 코루틴 함수 실행 및 중단 단계
// 
// - initial_suspend()에서 반환한 awaiter의 await_ready()가 true를 반환 -> 코루틴을 유예하지 않고 실행
// - initial_suspend()에서 반환한 awaiter의 await_ready()가 false를 반환 -> 코루틴 실행을 유예하고 유예 지점 설정
// - 코루틴 실행 중 co_await, co_yield에서 받은 awaiter의 await_ready()가 true 반환 -> 코루틴을 유예하지 않고 실행
// - 코루틴 실행 중 co_await, co_yield에서 받은 awaiter의 await_ready()가 false 반환 -> 코루틴 실행을 유예하고 유예 지점 설정
// - 사용자가 handle.resume() 호출 -> 유예 지점에서 코루틴 실행 재개
// 
// 코루틴 함수 몸체에서 예외가 발생하면 해당 예외는 여지없이 unhandled_exception()으로 받는다.
//
// --------------------------------------------------
// 
// @ 코루틴 함수 종료 단계
// 
// - co_return으로 코루틴 함수 반환 -> return_void() or return_value(T) -> 유효한 지역 변수의 소멸자 호출 -> final_suspend()
// - 코루틴 실행 중 예외 발생 -> 유효한 지역 변수의 소멸자 호출 -> unhandled_exception() -> final_suspend()
//   - 일반적인 함수에서 스택을 푸는(unwinding) 느낌과 비슷하게 코루틴 함수를 빠져나간 다음 unhandled_exception()를 호출하는 것이라 봐야 함.
//   - 코루틴 생성 단계와 코루틴 실행 단계는 다름.
// 
// 코루틴 함수가 더 이상 유효하지 않은 상태가 되면 사용 중인 지역 변수의 소멸자가 호출된다.
// 
// 코루틴 함수를 더 이상 재개할 수 없는 상태가 되면 handle.done()은 true를 반환한다.
// 
// !! 주의 !!
// final_suspend()는 코루틴 함수 실행이 끝나서 더 이상 재개할 수 없는 상태일 때 호출된다.
// 다만 코루틴 객체의 소멸이나 코루틴 프레임의 소멸이 final_suspend()의 호출까지 이어지는 건 아니다.
// 
// !! 주의 !!
// 코루틴 함수가 종료되지 않은 상태, 쉽게 생각해서 handle.done()이 false를 반환하는 상태일 때
// handle.destroy()를 호출한다고 해도 final_suspend()는 호출되지 않는다.
// 
// final_suspend()에서 반환한 awaiter가 코루틴 종료 지점을 유예하지 않으면 코루틴 프레임이 해제된다.
// 
// 일반적으로 종료 지점을 유예하지 않으면 여러모로 불편한 점이 많기 때문에
// 대부분의 경우 final_suspend()는 std::suspend_always 같이 종료 지점을 유예하는 awaiter를 반환하는 것이 좋다.
// 
// --------------------------------------------------
// 
// @ 코루틴 객체 소멸 단계
// 
// - 코루틴 객체 소멸자 호출
// 
// 코루틴 객체의 소멸 단계에서는 코루틴 객체의 소멸자를 호출하는 게 전부이다.
// 코루틴 객체가 소멸된다고 해도 코루틴 프레임까지 같이 소멸되는 것은 아니니까 주의해야 한다.
// 
// (주의) 즉, 다음과 같은 상황에서는 메모리 누수가 발생할 수 있다.
// 
// 1. final_suspend()가 반환하는 awaiter가 코루틴 종료 지점을 유예한다.
// 2. handle.destroy()를 호출하지 않은 상태에서 코루틴 객체가 소멸한다.
// 3. 코루틴 프레임에 접근 가능한 handle을 어딘가에 보관한 상태가 아니라면 메모리 누수가 발생한다.
// 
// 보통은 코루틴 객체의 소멸자 내 handle.destroy()를 호출하여 메모리 누수를 방지하지만 이러한 코드를 작성하는 건 전적으로 사용자의 몫이다.
// 
// --------------------------------------------------
// 
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
// 정말 특별한 경우가 아니라면 final_suspend()는 std::suspend_always와 같이 종료 지점을 유예하기 위한 awaiter를 반환하는 것이 좋다.
// 
// 코루틴 소멸을 진행할 때는 명시적으로 handle.destroy()를 쓰는 방식을 쓰도록 하며
// 해당 함수는 가급적 코루틴 객체의 소멸자에 작성하는 것이 좋다.
// 
// handle.destroy()를 꼭 코루틴 객체의 소멸자 내에 작성하란 법은 없지만 그렇게 해야 하는 이유가 있다면 반드시 이유를 명시해야 한다.
// 
// task.handle.destroy(); // 이처럼 코루틴 객체의 소멸자 밖에서 코루틴 프레임을 해제하는 것도 가능함.
// 
// 이렇게 원하는 시점에 코루틴 프레임을 해제할 수 있지만 정말 특별한 이유가 없다면
// 코루틴 객체의 소멸자 내에서 handle.destroy()를 호출하게 코드를 작성하도록 한다.
// 
// (주의) 코루틴 프레임이 소멸된 상태일 경우 handle.destroy()를 호출하면 안 된다.
// 
// final_suspend()에서 종료 지점을 유예하지 않으면 코루틴 프레임이 해제되는데
// 코루틴 객체의 소멸자 내에서 handle.destroy()를 호출하는 코드를 작성하는 경우는 꽤 흔한 편이니 주의해야 한다.
// 

// 코루틴 객체 생성 단계를 보기 위한 예시

// using InitSuspendAwaiter = std::suspend_always;
using InitSuspendAwaiter = std::suspend_never;

// using FinalSuspendAwaiter = std::suspend_always;
using FinalSuspendAwaiter = std::suspend_never;

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

        InitSuspendAwaiter initial_suspend() { std::cout << "\t<initial_suspend()>\n"; return { }; }
        FinalSuspendAwaiter final_suspend() noexcept { std::cout << "\t<final_suspend()>\n"; return { }; }

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

        // FinalSuspendAwaiter가 suspend_never와 같지 않을 경우에만
        if constexpr (!std::is_same_v<FinalSuspendAwaiter, std::suspend_never>)
        {
            if (handle != nullptr)
            {
                handle.destroy();

                handle = nullptr;
            }
        }
    }

    std::coroutine_handle<promise_type> handle = nullptr;
};

CoroType CoroFunc()
{
    using Awaiter = std::suspend_always;
    // using Awaiter = std::suspend_never;

    std::cout << "<Coroutine - Started>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    co_await Awaiter{ }; // <----- 여기를 유예 지점(suspension point)으로 함.
    std::cout << "<Coroutine - Resuming coroutine...>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    std::cout << "<Coroutine - Returning>\n";
    co_return;
}

int main()
{
    std::cout << "main - Calling coroutine\n";
    auto task = CoroFunc();

    // FinalSuspendAwaiter가 suspend_never와 같지 않을 경우에만
    if constexpr (!std::is_same_v<FinalSuspendAwaiter, std::suspend_never>)
    {
        // 코루틴 함수의 작업이 완료될 때까지 계속해서 평가를 진행한다.
        while (!task.handle.done())
        {
            std::cout << "\nmain - Resuming coroutine\n";
        
            // 유예 지점인 suspension point에서 실행을 재개한다.
            // task.handle(); // task.handle()은 task.handle.resume()과 같은 기능을 수행함.
            task.handle.resume();
        }

        // 코루틴 핸들의 done()을 통해서 코루틴 함수의 완료 여부를 확인할 수 있다.
        if (task.handle.done())
        {
            std::cout << "\nmain - Coroutine is done!\n";
        }
        else
        {
            std::cout << "\nmain - Coroutine isn't done!\n";
        }
    }
    else
    {
        std::cout << "\nmain - Resuming coroutine\n";
        
        // CoroFunc() 내 Awaiter는 std::suspend_always로 되어 있어야 한다.
        task.handle.resume();

        std::cout << "\nmain - Coroutine is done!\n";
    }

#pragma region 코루틴 객체 생성 단계
    // 코루틴 객체 생성 단계에 해당하는 절차는 다음과 같다.
    // --------------------------------------------------
    // main - Calling coroutine
    //     <operator new(size)>
    //     <promise_type()>
    //     <get_return_object()>
    //     <CoroType()>
    //     <initial_suspend()>
    //     ...
    // --------------------------------------------------
#pragma endregion

#pragma region 코루틴 함수 실행 및 중단 단계(initial_suspend())
    // initial_suspend()가 반환하는 awaiter가 std::suspend_always일 경우 시작 지점에서 실행을 유예한다.
    // --------------------------------------------------
    // main - Calling coroutine
    //         <operator new(size)>
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType()>
    //         <initial_suspend()>
    // 
    // main - Resuming coroutine <----- 코루틴 함수의 실행이 유예된 것을 볼 수 있음.
    // <Coroutine - Started>
    // <Coroutine - Executing coroutine function>
    // ...
    // --------------------------------------------------


    // initial_suspend()가 반환하는 awaiter가 std::suspend_never일 경우 실행을 유예하지 않고 코루틴 함수를 평가한다.
    // --------------------------------------------------
    // main - Calling coroutine
    //         <operator new(size)>
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType()>
    //         <initial_suspend()>
    // <Coroutine - Started>
    // <Coroutine - Executing coroutine function>
    // 
    // main - Resuming coroutine <----- 코루틴 함수의 실행이 유예되지 않고 실행된 것을 볼 수 있음.
    // ...
    // --------------------------------------------------
#pragma endregion

#pragma region 코루틴 함수 실행 및 중단 단계(코루틴 함수 실행 중 co_await, co_yield 사용)
    // initial_suspend()가 시작 지점을 유예하는 awaiter(std::suspend_always)를 반환한 상태에서
    //
    // co_await Awaiter;
    // 
    // 위 코드를 마주할 경우 어떤 일이 벌어지는지 확인한다.


    // co_await, co_yield에서 받은 awaiter의 await_ready()가 false를 반환하는 경우
    // 
    // "co_await std::suspend_always{ };"를 지정하면 해당 지점에서 코루틴 실행이 유예된 것을 확인할 수 있다.
    // 
    // handle.resume()을 호출하면 유예 지점에서 코드를 재개한다.
    // --------------------------------------------------
    // ...
    // <Coroutine - Started>
    // <Coroutine - Executing coroutine function>
    // 
    // main - Resuming coroutine <----- 유예 지점으로부터 실행이 재개된 것을 확인
    // <Coroutine - Resuming coroutine...>
    // <Coroutine - Executing coroutine function>
    // <Coroutine - Returning>
    //         <return_void()>
    //         <final_suspend()>
    // ...
    // --------------------------------------------------


    // co_await, co_yield에서 받은 awaiter의 await_ready()가 true를 반환하는 경우
    // 
    // "co_await std::suspend_never{ };"를 지정하면 해당 지점에서 코루틴이 유예되지 않는 것을 확인할 수 있다.
    // --------------------------------------------------
    // ...
    // <Coroutine - Started>
    // <Coroutine - Executing coroutine function>
    // <Coroutine - Resuming coroutine...> <----- 유예 지점에서 코드를 유예하지 않고 그냥 실행을 재개한 것을 확인
    // <Coroutine - Executing coroutine function>
    // <Coroutine - Returning>
    //         <return_void()>
    //         <final_suspend()>
    // ...
    // --------------------------------------------------
#pragma endregion

#pragma region 코루틴 함수 종료 단계(final_suspend())
    // final_suspend()는 코루틴 함수의 실행을 끝마친 이후 호출되는 함수이다.
    //
    // 만약 코루틴 함수를 더 재개할 수 있는 상태에서 handle.destroy()를 호출하면 final_suspend()는 호출될 수 없으니 주의해야 한다.


    // final_suspend()가 반환하는 awaiter의 await_ready()가 false를 반환하는 경우(std::suspend_always 적용)
    // 
    // 정말 특별한 이유가 없다면 final_suspend()가 반환하는 awaiter는 std::suspend_always인 경우가 일반적이다.
    // 
    // 이 유형의 경우 코루틴 차원에서 코루틴 프레임을 해제하지 않으니 명시적으로 handle.destroy()를 어디선가 호출해야 한다.
    // 여기서는 코루틴 객체의 소멸자에서 handle.destroy()를 호출한다.
    // --------------------------------------------------
    // ...
    // <Coroutine - Returning>
    //         <return_void()>
    //         <final_suspend()>
    // 
    // main - Coroutine is done!
    //         <~CoroType()>
    //         <~promise_type()>
    //         <operator delete(mem)>
    // --------------------------------------------------


    // final_suspend()가 반환하는 awaiter의 await_ready()가 true를 반환하는 경우(std::suspend_never 적용)
    //
    // final_suspend()에서 std::suspend_never를 적용하는 건 정말 특이한 경우이다.
    // 이 유형을 쓰면 코루틴 실행 종료 이후 코루틴 핸들이 가리키는 코루틴 프레임은 해제되기에 더 이상 유효하지 않게 된다.
    // 
    // 코루틴 자원이 알아서 해제된다는 건 사용자 쪽에서 핸들을 통한 제어를 할 수 없다는 뜻이며 이는 일반적으로 안전하지 않다.
    // !! 코루틴 함수의 완료 여부를 확인하기 위한 handle.done()을 호출하는 것 또한 매우 위험해지기에 상태 추적이 불가능해질 수 있음. !!
    //
    // 특별한 이유가 있어서 코루틴 작업이 완료되었을 때 즉시 리소스를 해제해야 하는 그런 상황이 아니면 사용하지 않도록 한다.
    // --------------------------------------------------
    // std::suspend_never initial_suspend() { std::cout << "\t<initial_suspend()>\n"; return { }; }
    // std::suspend_never final_suspend() noexcept { std::cout << "\t<final_suspend()>\n"; return { }; }
    // 
    // co_await std::suspend_always{ };
    // --------------------------------------------------
    // ...
    // <Coroutine - Returning>
    //         <return_void()>
    //         <final_suspend()>
    //         <~promise_type()> <----- final_suspend() 이후 코루틴 자원이 해제되는 것을 볼 수 있음.
    //         <operator delete(mem)>
    // 
    // main - Coroutine is done!
    //         <~CoroType()>
    // --------------------------------------------------
#pragma endregion

    return 0;
}
