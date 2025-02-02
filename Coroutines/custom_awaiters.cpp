// Update Date : 2025-02-02
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
// 4. awaitable_and_awaiter.cpp
// 5. custom_awaiters.cpp <-----
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

// C++이 기본으로 제공하는 awaiter로는 std::suspend_always와 std::suspend_never가 있다.
// 
// struct suspend_never
// {
//     constexpr bool await_ready() const noexcept {
//         return true;
//     }
// 
//     constexpr void await_suspend(coroutine_handle<>) const noexcept {}
//     constexpr void await_resume() const noexcept {}
// };
// 
// struct suspend_always
// {
//     constexpr bool await_ready() const noexcept {
//         return false;
//     }
// 
//     constexpr void await_suspend(coroutine_handle<>) const noexcept {}
//     constexpr void await_resume() const noexcept {}
// };
// 
// C++이 제공하는 기본 awaiter인 std::suspend_always와 std::suspend_never를 보면 어떠한 인터페이스를 만족하고 있는 것을 볼 수 있다.
// 두 구조체의 유일한 차이점은 await_ready()에서 반환하고 있는 값이 다르다는 것 뿐이다.
// 
// suspend_never의 await_ready()는 true를 반환하고 있는데,
// 이 true의 의미는 "준비됨"이며 이는 코루틴을 유예(suspend)하지 않겠다는 뜻이다.
// 
// 반대로 suspend_always의 await_ready()는 false를 반환하고 있는데,
// 이 false의 의미는 "준비되지 않음"이며 따라서 코루틴을 유예(suspend)하겠다는 뜻이다.
// 
// !! await_ready()가 반환하는 값의 초점은 await가 아닌 ready에 있음. !!
// 

// # Custom Awaiter
// 
// awaiter 인터페이스는 3개의 함수를 정의해야 하며 이 조건을 만족하면 사용자가 만든 awaiter를 사용할 수 있다.
//
// - bool await_ready()
// : co_await에 의해 호출되며 코루틴의 유예 여부를 제어하는 함수(true를 반환하면 유예하지 않고, false를 반환하면 유예함)
// 
// - ReturnType await_suspend(coroutine_handle<>)
// : 코루틴이 유예되었을 때 호출되는 함수(ReturnType에 따라 세부적으로 동작하는 방식이 다름)
// 
// - ReturnType await_resume()
// : 코루틴이 재개되었을 때 호출되는 함수(ReturnType은 재개할 때 co_await, co_yield의 결과로 반환할 값에 대한 타입임)
// 
// struct Awaiter
// {
//     bool await_ready()
//     {
//         // true  : 준비가 완료되었기에 유예하지 않는다.
//         // false : 준비되지 않았기에 처리를 위해 유예한다.
//         return isReady;
//     }
// 
//     // 모든 코루틴 핸들은 std::coroutine_handle<>로 변환할 수 있다(convertible).
//     ReturnType await_suspend(std::coroutine_handle<> handle)
//     {
//         // suspension 상태에 진입할 때 실행할 로직
//     }
// 
//     ReturnType await_resume()
//     {
//         // suspension 이후 재개할 때 실행할 로직
//     }
// };
// 
// 어떠한 경로가 되었든 co_await가 최종적으로 받는 건 awaiter이다.
// 
// co_await가 awaiter를 받으면 가장 먼저 호출하는 함수는 await_ready()이다.
// 
// await_ready()가 false를 반환하면 코루틴은 await_suspend()를 호출하고,
// await_ready()가 true를 반환하면 코루틴은 await_resume()을 호출한다.
// 
// 코루틴이 유예되었으면 resume()으로 재개해야 하며, 이때 코루틴은 await_resume()을 호출한다.
// 
// await_ready()가 true를 반환할 때도 await_resume()을 호출하고, resume()으로 코루틴을 재개할 때도 await_resume()을 호출한다.
//
// --------------------------------------------------
// 
// @ bool await_ready()
//
// 코루틴 유예 여부를 제어하기 위해 핵심 함수이다.
// true를 반환하면 await_resume()을 호출하며, false를 반환하면 await_suspend()을 호출한다.
// 
// --------------------------------------------------
// 
// @ ReturnType await_suspend(coroutine_handle<>)
// 
// 코루틴이 유예될 때 호출되는 함수로 반환 타입에 따라 동작하는 방식이 다르다.
// 매개변수로 들어오는 핸들은 suspension 상태에 들어갈 코루틴의 핸들이며 모든 코루틴 핸들은 std::coroutine_handle<>로 변환할 수 있다.
// 
// 반환형에 따른 await_suspend()의 동작 방식을 보도록 하자.
// - 반환형이 void인 경우 : 코루틴의 실행을 유예한다.
// - 반환형이 bool인 경우 : true면 코루틴의 실행을 유예하고, false면 유예하지 않는다.
// - 다른 코루틴의 핸들을 반환하는 경우 : 해당 핸들이 가리키는 코루틴을 재개한다(다른 코루틴을 스케쥴링하겠다는 의미이며 이를 통해 코루틴 체이닝이 가능함).
//   - 반환된 핸들을 통해 코루틴을 재개할 때는 내부적으로 handle.resume()을 거침.
// 
// 조금 이상하긴 한데 매개변수로 들어온 핸들을 통해 명시적으로 destroy()를 호출하는 것도 가능하다.
// 
// destroy()는 코루틴 프레임을 해제하기 때문에 해당 함수를 호출하려거든
// 다른 쪽에서 더 이상 해당 핸들에 접근하지 않는다는 것이 보장되어야 한다.
// 
// --------------------------------------------------
// @ ReturnType await_resume()
// 
// await_resume()은 다음 상황에서 호출된다.
// - 유예된 코루틴이 재개된 경우(보통 handle.resume()을 통해 재개됨)
// - await_ready()가 true를 반환했을 경우
// - await_suspend()의 반환형이 bool이며, 해당 함수에서 false를 반환했을 경우
// - await_suspend()이 코루틴 핸들이며, 해당 함수에서 재개할 핸들을 반환했을 경우
//
// await_resume()은 다양한 상황에서 호출되지만 대부분의 경우 코루틴을 resume()으로 재개할 때 호출된다.
// 
// await_resume()은 값을 반환할 수 있으며 이는 "co_await expr"의 결과로 사용 가능하다. 
// 따라서 await_resume()의 반환형이 void가 아니라면 다음과 같이 awaiter를 통해 값을 받을 수 있다.
// 
// auto yieldRet = co_yield 10; // co_await promise.yield_value(10);
// auto awaitRet = co_await 10; // co_await promise.await_transform(10);
// auto awaitRet = co_await MyAwaitable{ }; // co_await awaitable.operator co_await();
// auto awaitRet = co_await MyAwaiter{ };
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

        // 코루틴 실행 시작 지점에서 유예하지 않고 마지막 지점에서 코루틴을 유예한다.
        std::suspend_never initial_suspend() { std::cout << "\t<initial_suspend()>\n"; return { }; }
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

    // 코루틴 핸들은 코루틴 프레임을 가리키기 위한 용도로 사용된다.
    std::coroutine_handle<promise_type> handle = nullptr;
};

BEGIN_NS(Case01)

// 코루틴을 유예하는 가장 기본이 되는 Awaiter(std::suspend_always와 동일한 기능을 수행)
struct MyAwaiter // 직접 구성한 Awaiter
{
    MyAwaiter()
    {
        std::cout << "\t<MyAwaiter()>\n";
    }

    ~MyAwaiter()
    {
        std::cout << "\t<~MyAwaiter()>\n\n";
    }

    bool await_ready()
    {
        std::cout << "\t<MyAwaiter - await_ready()>\n";

        return false; // 코루틴을 유예하기 위해 false 반환
    }

    // 모든 코루틴 핸들은 std::coroutine_handle<>로 변환할 수 있다(convertible).
    void await_suspend(std::coroutine_handle<> handle)
    {
        std::cout << "\t<MyAwaiter - await_suspend()>\n";
    }

    void await_resume()
    {
        std::cout << "\t<MyAwaiter - await_resume()>\n";
    }
};

CoroType CoroFunc()
{
    std::cout << "\n<Coroutine - Started>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    // MyAwaiter의 await_ready()를 호출하여 유예 여부를 제어한다.
    co_await MyAwaiter{ };
    std::cout << "<Coroutine - Resuming coroutine...>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    std::cout << "<Coroutine - Returning>\n";
    co_return;
}

void Run()
{
    std::cout << "main - Calling coroutine\n";
    auto task = CoroFunc();

    std::cout << "\nmain - Resuming coroutine\n";
    task.handle.resume();
    
    if (task.handle.done())
    {
        std::cout << "\nmain - Coroutine is done!\n";
    }
    else
    {
        std::cout << "\nmain - Coroutine isn't done!\n";
    }

    // main - Calling coroutine
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType()>
    //         <initial_suspend()>
    // 
    // <Coroutine - Started>
    // <Coroutine - Executing coroutine function>
    //         <MyAwaiter()>
    //         <MyAwaiter - await_ready()>   <----- co_await가 MyAwaiter를 받고 await_ready()를 호출
    //         <MyAwaiter - await_suspend()> <----- await_ready()가 false를 반환하여 코루틴을 유예하는 과정에서 await_suspend()를 호출
    // 
    // main - Resuming coroutine
    //         <MyAwaiter - await_resume()> <----- 핸들을 통해 resume()을 호출하여 재개하는 과정에서 await_resume()을 호출
    //         <~MyAwaiter()> <----- 임시 객체로 사용한 Awaiter의 소멸자 호출
    // 
    // <Coroutine - Resuming coroutine...>
    // <Coroutine - Executing coroutine function>
    // <Coroutine - Returning>
    //         <return_void()>
    //         <final_suspend()>
    // 
    // main - Coroutine is done!
    //         <~CoroType()>
    //         <~promise_type()>
}

END_NS

BEGIN_NS(Case02)

// 코루틴을 유예한 상태에서 코루틴 객체가 종료 절차에 들어가면 await_resume()은 호출되지 않는다.

// 코루틴을 유예하는 가장 기본이 되는 Awaiter(std::suspend_always와 동일한 기능을 수행)
struct MyAwaiter // 직접 구성한 Awaiter
{
    MyAwaiter()
    {
        std::cout << "\t<MyAwaiter()>\n";
    }

    ~MyAwaiter()
    {
        std::cout << "\t<~MyAwaiter()>\n";
    }

    bool await_ready()
    {
        std::cout << "\t<MyAwaiter - await_ready()>\n";

        return false; // 코루틴을 유예하기 위해 false 반환
    }

    // 모든 코루틴 핸들은 std::coroutine_handle<>로 변환할 수 있다(convertible).
    void await_suspend(std::coroutine_handle<> handle)
    {
        std::cout << "\t<MyAwaiter - await_suspend()>\n";
    }

    void await_resume()
    {
        std::cout << "\t<MyAwaiter - await_resume()>\n";
    }
};

CoroType CoroFunc()
{
    std::cout << "\n<Coroutine - Started>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    // MyAwaiter의 await_ready()를 호출하여 유예 여부를 제어한다.
    co_await MyAwaiter{ };
    std::cout << "<Coroutine - Resuming coroutine...>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    std::cout << "<Coroutine - Returning>\n";
    co_return;
}

void Run()
{
    std::cout << "main - Calling coroutine\n";
    auto task = CoroFunc();

    // 코루틴을 유예한 상태에서 코루틴 객체가 소멸 절차에 들어가면?
    // std::cout << "\nmain - Resuming coroutine\n";
    // task.handle.resume();

    if (task.handle.done())
    {
        std::cout << "\nmain - Coroutine is done!\n";
    }
    else
    {
        std::cout << "\nmain - Coroutine isn't done!\n";
    }

    // main - Calling coroutine
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType()>
    //         <initial_suspend()>
    // 
    // <Coroutine - Started>
    // <Coroutine - Executing coroutine function>
    //         <MyAwaiter()>
    //         <MyAwaiter - await_ready()>
    //         <MyAwaiter - await_suspend()>
    // 
    // main - Coroutine isn't done! <----- 코루틴 객체가 소멸 절차에 들어간다고 해도 "<MyAwaiter - await_resume()>"는 출력되지 않음.
    //         <~CoroType()>
    //         <~MyAwaiter()> <----- 임시 객체로 사용한 Awaiter의 소멸자 호출
    //         <~promise_type()>
}

END_NS

BEGIN_NS(Case03)

// 코루틴을 유예하지 않는 가장 기본이 되는 Awaiter(std::suspend_never와 동일한 기능을 수행)
struct MyAwaiter // 직접 구성한 Awaiter
{
    MyAwaiter()
    {
        std::cout << "\t<MyAwaiter()>\n";
    }

    ~MyAwaiter()
    {
        std::cout << "\t<~MyAwaiter()>\n\n";
    }

    bool await_ready()
    {
        std::cout << "\t<MyAwaiter - await_ready()>\n";

        return true; // 코루틴을 유예하지 않기 위해 true 반환
    }

    // 모든 코루틴 핸들은 std::coroutine_handle<>로 변환할 수 있다(convertible).
    void await_suspend(std::coroutine_handle<> handle)
    {
        std::cout << "\t<MyAwaiter - await_suspend()>\n";
    }

    void await_resume()
    {
        std::cout << "\t<MyAwaiter - await_resume()>\n";
    }
};

CoroType CoroFunc()
{
    std::cout << "\n<Coroutine - Started>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    // MyAwaiter의 await_ready()를 호출하여 유예 여부를 제어한다.
    co_await MyAwaiter{ };
    std::cout << "<Coroutine - Resuming coroutine...>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    std::cout << "<Coroutine - Returning>\n";
    co_return;
}

void Run()
{
    std::cout << "main - Calling coroutine\n";
    auto task = CoroFunc();

    // MyAwaiter의 await_ready()가 true를 반환하기 때문에 실행을 유예하지 않는다.
    // std::cout << "\nmain - Resuming coroutine\n";
    // task.handle.resume();
    
    if (task.handle.done())
    {
        std::cout << "\nmain - Coroutine is done!\n";
    }
    else
    {
        std::cout << "\nmain - Coroutine isn't done!\n";
    }

    // main - Calling coroutine
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType()>
    //         <initial_suspend()>
    // 
    // <Coroutine - Started>
    // <Coroutine - Executing coroutine function>
    //         <MyAwaiter()>
    //         <MyAwaiter - await_ready()>  <----- co_await가 MyAwaiter를 받고 await_ready()를 호출
    //         <MyAwaiter - await_resume()> <----- await_ready()가 true를 반환하여 실행을 유예하지 않고 그대로 재개
    //         <~MyAwaiter()> <----- 임시 객체로 사용한 Awaiter의 소멸자 호출
    // 
    // <Coroutine - Resuming coroutine...>
    // <Coroutine - Executing coroutine function>
    // <Coroutine - Returning>
    //         <return_void()>
    //         <final_suspend()>
    // 
    // main - Coroutine is done!
    //         <~CoroType()>
    //         <~promise_type()>
}

END_NS

BEGIN_NS(Case04)

// await_ready()가 false를 반환하여 실행을 유예하기 위해 await_suspend()를 호출하였는데,
// await_suspend()의 반환형이 bool이고, 해당 함수에서 false를 반환하면 실행은 유예되지 않는다.

// 결과적으로는 std::suspend_never와 동일한 기능을 수행한다.
struct MyAwaiter // 직접 구성한 Awaiter
{
    MyAwaiter()
    {
        std::cout << "\t<MyAwaiter()>\n";
    }

    ~MyAwaiter()
    {
        std::cout << "\t<~MyAwaiter()>\n\n";
    }

    bool await_ready()
    {
        std::cout << "\t<MyAwaiter - await_ready()>\n";

        // await_suspend()를 호출하기 위해 false 반환
        return false;
    }

    // 모든 코루틴 핸들은 std::coroutine_handle<>로 변환할 수 있다(convertible).
    bool await_suspend(std::coroutine_handle<> handle)
    {
        std::cout << "\t<MyAwaiter - await_suspend()>\n";

        return false; // false를 반환하면 실행을 유예하지 않음(true를 반환하면 실행을 유예함).
    }

    void await_resume()
    {
        std::cout << "\t<MyAwaiter - await_resume()>\n";
    }
};

CoroType CoroFunc()
{
    std::cout << "\n<Coroutine - Started>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    // MyAwaiter의 await_ready()가 false를 반환했다고 해도
    // bool await_suspend()에서 false를 반환하면 실행을 유예하지 않는다.
    co_await MyAwaiter{ };
    std::cout << "<Coroutine - Resuming coroutine...>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    std::cout << "<Coroutine - Returning>\n";
    co_return;
}

void Run()
{
    std::cout << "main - Calling coroutine\n";
    auto task = CoroFunc();

    // MyAwaiter가 실행을 유예하지 않기에 주석처리한다.
    // std::cout << "\nmain - Resuming coroutine\n";
    // task.handle.resume();
    
    if (task.handle.done())
    {
        std::cout << "\nmain - Coroutine is done!\n";
    }
    else
    {
        std::cout << "\nmain - Coroutine isn't done!\n";
    }

    // main - Calling coroutine
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType()>
    //         <initial_suspend()>
    // 
    // <Coroutine - Started>
    // <Coroutine - Executing coroutine function>
    //         <MyAwaiter()>
    //         <MyAwaiter - await_ready()>   <----- co_await가 MyAwaiter를 받고 await_ready()를 호출
    //         <MyAwaiter - await_suspend()> <----- await_ready()가 false를 반환하여 코루틴을 유예하는 과정에서 await_suspend()를 호출
    //         <MyAwaiter - await_resume()>  <----- await_suspend()에서 false를 반환했기에 실행을 유예하지 않고 실행을 재개
    //         <~MyAwaiter()> <----- 임시 객체로 사용한 Awaiter의 소멸자 호출
    // 
    // <Coroutine - Resuming coroutine...>
    // <Coroutine - Executing coroutine function>
    // <Coroutine - Returning>
    //         <return_void()>
    //         <final_suspend()>
    // 
    // main - Coroutine is done!
    //         <~CoroType()>
    //         <~promise_type()>
}

END_NS

BEGIN_NS(Case05)

// await_suspend()에서 다른 코루틴의 핸들을 반환하면 해당 핸들이 가리키는 코루틴을 재개한다.
// 반환된 핸들을 통해 코루틴을 재개할 때는 내부적으로 handle.resume()을 거치며,
// 이는 다른 코루틴을 스케쥴링하겠다는 의미이다(이를 통해 코루틴 체이닝 가능).

struct MyAwaiter // 직접 구성한 Awaiter
{
    MyAwaiter(CoroType& otherCoro)
        : otherCoro{ otherCoro }
    {
        std::cout << "\t<MyAwaiter()>\n";
    }

    ~MyAwaiter()
    {
        std::cout << "\t<~MyAwaiter()>\n";
    }

    bool await_ready()
    {
        std::cout << "\t<MyAwaiter - await_ready()>\n";

        // await_suspend()를 호출하기 위해 false 반환
        return false;
    }

    // 모든 코루틴 핸들은 std::coroutine_handle<>로 변환할 수 있다(convertible).
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle)
    {
        std::cout << "\t<MyAwaiter - await_suspend()>\n";

        // 다른 코루틴의 핸들을 반환하여 해당 코루틴을 재개한다.
        return otherCoro.handle;
    }

    void await_resume()
    {
        std::cout << "\t<MyAwaiter - await_resume()>\n";
    }

    CoroType& otherCoro;
};

CoroType CoroFuncA()
{
    std::cout << "\n<CoroutineA - Started>\n";
    std::cout << "<CoroutineA - Executing coroutine function>\n";

    co_await std::suspend_always{ };
    std::cout << "<CoroutineA - Resuming coroutine...>\n";
    std::cout << "<CoroutineA - Executing coroutine function>\n";

    std::cout << "<CoroutineA - Returning>\n";
    co_return;
}

CoroType CoroFuncB(CoroType& otherCoro)
{
    std::cout << "\n<CoroutineB - Started>\n";
    std::cout << "<CoroutineB - Executing coroutine function>\n";

    co_await MyAwaiter{ otherCoro };
    std::cout << "<CoroutineB - Resuming coroutine...>\n";
    std::cout << "<CoroutineB - Executing coroutine function>\n";

    std::cout << "<CoroutineB - Returning>\n";
    co_return;
}

void Run()
{
    std::cout << "main - Calling coroutine\n";
    auto taskA = CoroFuncA();

    std::cout << "\nmain - Calling other coroutine\n";
    auto taskB = CoroFuncB(taskA);

    // taskB를 재개했을 때 taskA도 재개하는지 확인하기 위한 코드
    std::cout << "\nmain - Resuming coroutine\n";
    taskB.handle.resume();
    
    if (taskA.handle.done() && taskB.handle.done())
    {
        std::cout << "\nmain - All coroutines are done!\n";
    }
    else
    {
        std::cout << "\nmain - At least one coroutine isn't done!\n";
    }

    // main - Calling coroutine
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType()>
    //         <initial_suspend()>
    // 
    // <CoroutineA - Started>
    // <CoroutineA - Executing coroutine function> <----- std::suspend_always로 실행 유예
    // 
    // main - Calling other coroutine
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType()>
    //         <initial_suspend()>
    // 
    // <CoroutineB - Started>
    // <CoroutineB - Executing coroutine function>
    //         <MyAwaiter()>
    //         <MyAwaiter - await_ready()>   <----- co_await가 MyAwaiter를 받고 await_ready()를 호출
    //         <MyAwaiter - await_suspend()> <----- await_suspend()에서 CoroutineA의 핸들 반환
    // <CoroutineA - Resuming coroutine...>  <----- CoroutineA의 실행을 재개 & CoroutineB는 실행을 유예
    // <CoroutineA - Executing coroutine function>
    // <CoroutineA - Returning>
    //         <return_void()>
    //         <final_suspend()>
    // 
    // main - Resuming coroutine <----- taskB.handle.resume()를 통한 실행 재개
    //         <MyAwaiter - await_resume()>
    //         <~MyAwaiter()> <----- CoroutineB 내 임시 객체로 사용한 Awaiter의 소멸자를 이제서야 호출
    // <CoroutineB - Resuming coroutine...>
    // <CoroutineB - Executing coroutine function>
    // <CoroutineB - Returning>
    //         <return_void()>
    //         <final_suspend()>
    // 
    // main - All coroutines are done! <----- 모든 코루틴의 실행이 완료됨!
    //         <~CoroType()>
    //         <~promise_type()>
    //         <~CoroType()>
    //         <~promise_type()>
}

END_NS

BEGIN_NS(Case06)

// await_resume()은 코루틴이 재개되었을 때 값을 반환할 수 있다.

struct MyAwaiter // 직접 구성한 Awaiter
{
    bool await_ready()
    {
        std::cout << "\t<MyAwaiter - await_ready()>\n";

        return false; // 코루틴을 유예하기 위해 false 반환
    }

    // 모든 코루틴 핸들은 std::coroutine_handle<>로 변환할 수 있다(convertible).
    void await_suspend(std::coroutine_handle<> handle)
    {
        std::cout << "\t<MyAwaiter - await_suspend()>\n";
    }

    // await_resume()은 재개 시 사용할 값을 반환할 수 있다.
    std::string await_resume()
    {
        std::cout << "\t<MyAwaiter - await_resume()>\n";

        return "Hello Awaiter"; // 여기서는 문자열을 반환하지만 다른 값을 반환해도 됨.
    }
};

CoroType CoroFunc()
{
    std::cout << "\n<Coroutine - Started>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";
    
    // await_resume()은 값을 반환할 수 있으며 이는 "co_await expr"의 결과로 간주한다.
    auto awaitRet = co_await MyAwaiter{ };
    std::cout << "<Coroutine - Resuming coroutine...>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";
    std::cout << "<Coroutine - co_await returned : " << awaitRet << ">\n";
    
    std::cout << "<Coroutine - Returning>\n";
    co_return;
}

void Run()
{
    std::cout << "main - Calling coroutine\n";
    auto task = CoroFunc();

    std::cout << "\nmain - Resuming coroutine\n";
    task.handle.resume();
    
    if (task.handle.done())
    {
        std::cout << "\nmain - Coroutine is done!\n";
    }
    else
    {
        std::cout << "\nmain - Coroutine isn't done!\n";
    }

    // main - Calling coroutine
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType()>
    //         <initial_suspend()>
    // 
    // <Coroutine - Started>
    // <Coroutine - Executing coroutine function>
    //         <MyAwaiter - await_ready()>
    //         <MyAwaiter - await_suspend()>
    // 
    // main - Resuming coroutine
    //         <MyAwaiter - await_resume()>
    // <Coroutine - Resuming coroutine...>
    // <Coroutine - Executing coroutine function>
    // <Coroutine - co_await returned : Hello World> <----- await_resume()이 반환한 값을 출력
    // <Coroutine - Returning>
    //         <return_void()>
    //         <final_suspend()>
    // 
    // main - Coroutine is done!
    //         <~CoroType()>
    //         <~promise_type()>
}

END_NS

BEGIN_NS(Case07)

// initial_suspend()와 final_suspend()는 awaiter를 반환해야 한다.
// 
// 일반적으로 기본 awaiter인 std::suspend_always와 std::suspend_never를 적용하지만
// 양식만 맞춘다면 사용자가 작성한 custom awaiter도 사용할 수 있다.

// namespace 밖에도 MyAwaiter가 있지만 안에 있는 것을 더 우선해서 사용하니까 문제 없다.
template <bool isReady>
struct MyAwaiter // final_suspend()가 noexcept 유형 때문에 모든 함수에 noexcept를 붙여야 함.
{
    bool await_ready() noexcept
    {
        std::cout << "\t<MyAwaiter - await_ready() : isReady = " << isReady << ">\n";

        return isReady;
    }
    
    // 모든 코루틴 핸들은 std::coroutine_handle<>로 변환할 수 있다(convertible).
    void await_suspend(std::coroutine_handle<> handle) noexcept
    {
        std::cout << "\t<MyAwaiter - await_suspend() : isReady = " << isReady << ">\n";
    }

    void await_resume() noexcept
    {
        std::cout << "\t<MyAwaiter - await_resume() : isReady = " << isReady << ">\n";
    }
};

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

        // MyAwaiter<true>  : std::suspend_never와 동일
        // MyAwaiter<false> : std::suspend_always와 동일
        MyAwaiter<true> initial_suspend() { std::cout << "\t<initial_suspend()>\n"; return { }; }
        MyAwaiter<false> final_suspend() noexcept { std::cout << "\t<final_suspend()>\n"; return { }; }

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

    // 코루틴 핸들은 코루틴 프레임을 가리키기 위한 용도로 사용된다.
    std::coroutine_handle<promise_type> handle = nullptr;
};

CoroType CoroFunc()
{
    std::cout << "\n<Coroutine - Started>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    co_await std::suspend_always{ };
    std::cout << "<Coroutine - Resuming coroutine...>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";
    
    std::cout << "<Coroutine - Returning>\n";
    co_return;
}

void Run()
{
    std::cout << "main - Calling coroutine\n";
    auto task = CoroFunc();

    std::cout << "\nmain - Resuming coroutine\n";
    task.handle.resume();

    // DISASTER : final_suspend()에서 종료 시점에 코루틴을 유예했어도 코루틴 함수는 전부 실행한 상태이기에 resume()을 호출하면 안 된다.
    // task.handle.done();
    
    if (task.handle.done())
    {
        std::cout << "\nmain - Coroutine is done!\n";
    }
    else
    {
        std::cout << "\nmain - Coroutine isn't done!\n";
    }

    // main - Calling coroutine
    //         <promise_type()>
    //         <get_return_object()>
    //         <CoroType()>
    //         <initial_suspend()>
    //         <MyAwaiter - await_ready() : isReady = 1>
    //         <MyAwaiter - await_resume() : isReady = 1> <----- await_ready()에서 true를 반환했으니 await_resume()을 호출
    // 
    // <Coroutine - Started>
    // <Coroutine - Executing coroutine function>
    // 
    // main - Resuming coroutine
    // <Coroutine - Resuming coroutine...>
    // <Coroutine - Executing coroutine function>
    // <Coroutine - Returning>
    //         <return_void()>
    //         <final_suspend()>
    //         <MyAwaiter - await_ready() : isReady = 0>
    //         <MyAwaiter - await_suspend() : isReady = 0> <----- await_ready()에서 false를 반환했으니 await_suspend()을 호출
    // 
    // main - Coroutine is done!
    //         <~CoroType()>
    //         <~promise_type()>
}

END_NS

int main()
{
    // Case01::Run();
    // Case02::Run();
    // Case03::Run();
    // Case04::Run();
    // Case05::Run();
    // Case06::Run();
    Case07::Run();

    return 0;
}
