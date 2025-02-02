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
// 5. custom_awaiters.cpp
// 
// # 값 산출
// 6. get_value_from_co_return_co_yield.cpp <-----
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

// https://en.cppreference.com/w/cpp/language/coroutines#Execution
// https://en.cppreference.com/w/cpp/language/coroutines#co_yield

// co_return과 co_yield를 쓰면 반환 값은 코루틴 함수를 통해 들어온다.
// !! 바로 호출자에게 전달되는 개념이 아님. !!
//
// 따라서 코루틴 외부(호출자 쪽)에 값을 전달하려면
// 반환된 결과를 먼저 promise_type에 보관하든 코루틴 객체에 보관하든 해야 한다.
// 
// 이후 실행 제어권이 호출자에게 넘어갔을 때 값을 보관한 대상에 접근해서 값을 꺼내와야 한다.
// 
// --------------------------------------------------
// 
// co_return <value>는 다음과 같은 특징이 있다.
// - co_return <value>로 값을 반환하면 코루틴 함수의 실행은 끝난다(더 이상 재개 불가).
// - 반환하는 값에 대응하는 return_value(T)를 promise_type에 정의해야 한다.
// - (중요) 반환된 값이 호출자 쪽에 직접적으로 전달되는 개념이 아니다.
//   - 값을 호출자에게 전달하려면 우회해야 함.
// 
// co_return으로 값을 반환하면 코루틴 함수 실행을 종료하기 때문에 return_value(T) -> final_suspend()를 거친다.
// 
// --------------------------------------------------
// 
// co_yield를 쓰면 코루틴을 끝마치지 않고 중간 결과를 산출할 수 있다.
// co_yield를 쓰면 코루틴 실행을 유예시키고 재개할 때마다 매번 값을 생성할 수 있다.
// 
// co_yield이 반환한 값을 호출자 쪽으로 전파하려면 co_return과 마찬가지로 결과를 코루틴 내에 보관하고 있어야 한다.
// 
// co_yield <value>는 다음과 같은 특징이 있다.
// - co_yield <value>로 값을 반환해도 코루틴 실행이 끝나는 건 아니다.
// - co_yield <value>에 대응하는 yield_value(T)를 promise_type에 정의해야 한다.
// 
// - yield_value(T)는 awaiter를 반환하며 이를 통해 코루틴의 유예 상태를 제어할 수 있다.
//   - 반환하는 awaiter에 따라서 코루틴 실행을 유예시키지 않을 수도 있음.
//   - co_yield를 쓴다고 코루틴 실행이 반드시 유예되는 건 아님.
//   - 유예되었을 때와 재개하였을 때 호출되는 함수를 커스텀하여 세밀한 제어가 가능하다.
// 
// - "co_yield expr"는 컴파일되면 "co_await promise.yield_value(expr)"로 변환된다.
//   - co_yield 100; // 컴파일 과정에서 표현식은
//   - co_await promise.yield_value(100); // 이렇게 변환된다.
// 

BEGIN_NS(Case01)

// co_return <value>로 값을 반환하는 코루틴 예시 코드(promise_type에 보관)

struct CoroType
{
    struct promise_type
    {
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

            std::rethrow_exception(std::current_exception());
        }

        // co_return이 값을 반환하면 return_void()는 사용할 수 없다.
        // void return_void()
        // {
        //     std::cout << "\t<return_void()>\n";
        // }

        // co_return이 반환하는 값은 return_value(T)의 매개변수 타입으로 convertible 해야 한다.
        void return_value(int value)
        {
            std::cout << "\t<return_value(T)>\n";

            result = value;
        }

        // co_return으로 반환한 결과를 호출자 쪽에 전파하기 위해선 코루틴 차원에서 값을 보관하고 있어야 한다.
        int result;
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

    // 코루틴 타입에 함수를 만들어서 promise 객체에 접근하여 값을 가져오는 방식
    int GetResult() const
    {
        return handle.promise().result;
    }

    std::coroutine_handle<promise_type> handle = nullptr;
};

CoroType CoroAdd(int x, int y)
{
    std::cout << "<Coroutine - Started>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    std::cout << "<Coroutine - Returning>\n";
    co_return x + y;

    std::cout << "<Coroutine - Unable to resume after co_return>\n";
}

void Run()
{
    std::cout << "main - Calling coroutine\n";
    auto task = CoroAdd(10, 20);

    std::cout << "\nmain - Resuming coroutine\n";
    task.handle.resume();
    
    // co_return은 코루틴 실행을 끝마치기 때문에 더 이상 재개할 수 없다.
    if (!task.handle.done())
    {
        task.handle.resume(); // 들어오지 않음.
    }
    
    // co_return으로 반환한 결과를 호출자 쪽이 파악하기 위한 순서
    // 1. co_return으로 값을 반환하면 호출자 쪽이 아닌 코루틴이 받는다.
    // 2. co_return이 반환한 결과를 호출자에서 받고자 한다면 일단 반환된 값을 코루틴 내에 저장해야 한다.
    // 3. 실행 제어권이 호출자 쪽에 넘어왔을 때 값이 저장된 코루틴 프레임에 접근하여 값을 가져온다.

    // 코루틴 타입에 함수를 만들어서 값을 가져오는 기능을 캡슐화하는 방식
    std::cout << "\nmain - 1. Result is : " << task.GetResult() <<'\n';
    
    // 직접 핸들에 접근해서 값을 가져오는 방식
    std::cout << "main - 2. Result is : " << task.handle.promise().result <<'\n';
    
    if (task.handle.done())
    {
        std::cout << "\nmain - Coroutine is done!\n";
    }
    else
    {
        std::cout << "\nmain - Coroutine isn't done!\n";
    }
}

END_NS

BEGIN_NS(Case02)

// co_return <value>로 값을 반환하는 코루틴 예시 코드(코루틴 객체에 보관)

struct CoroType
{
    struct promise_type
    {
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

            std::rethrow_exception(std::current_exception());
        }

        // co_return이 값을 반환하면 return_void()는 사용할 수 없다.
        // void return_void()
        // {
        //     std::cout << "\t<return_void()>\n";
        // }

        // co_return이 반환하는 값은 return_value(T)의 매개변수 타입으로 convertible 해야 한다.
        void return_value(int value)
        {
            std::cout << "\t<return_value(T)>\n";

            // 코루틴 객체에 접근하여 결과 저장
            coroInst->result = value;
        }

        // 코루틴 객체를 promise 타입이 알 수 있게 한다.
        CoroType* coroInst = nullptr;
    };

    CoroType(promise_type* prom)
        : handle{ std::coroutine_handle<promise_type>::from_promise(*prom) }
    {
        std::cout << "\t<CoroType()>\n";

        // promise 객체가 코루틴 객체를 알 수 있게 설정
        handle.promise().coroInst = this;
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

    // 코루틴 객체 내에 값을 저장하고 이를 호출자에 전파하는 방식
    int GetResult() const
    {
        return result;
    }

    std::coroutine_handle<promise_type> handle = nullptr;

    // 값을 코루틴 객체에 보관한다.
    int result{ 0 };
};

CoroType CoroAdd(int x, int y)
{
    std::cout << "<Coroutine - Started>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    std::cout << "<Coroutine - Returning>\n";
    co_return x + y;

    std::cout << "<Coroutine - Unable to resume after co_return>\n";
}

void Run()
{
    std::cout << "main - Calling coroutine\n";
    auto task = CoroAdd(10, 20);

    std::cout << "\nmain - Resuming coroutine\n";
    task.handle.resume();
    
    // co_return은 코루틴 실행을 끝마치기 때문에 더 이상 재개할 수 없다.
    if (!task.handle.done())
    {
        task.handle.resume(); // 들어오지 않음.
    }
    
    // co_return으로 반환한 결과를 호출자가 받기 위한 순서
    // 1. co_return으로 값을 반환하면 호출자 쪽이 아닌 코루틴이 받는다.
    // 2. co_return이 반환한 결과를 호출자에서 받고자 한다면 일단 반환된 값을 코루틴 내에 저장해야 한다.
    // 3. 실행 제어권이 호출자 쪽에 넘어왔을 때 값이 저장된 코루틴 프레임에 접근하여 값을 가져온다.

    // 코루틴 타입에 함수를 만들어서 값을 가져오는 기능을 캡슐화하는 방식
    std::cout << "\nmain - 1. Result is : " << task.GetResult() <<'\n';
    
    // 직접 객체에 접근해서 값을 가져오는 방식
    std::cout << "main - 2. Result is : " << task.result <<'\n';
    
    if (task.handle.done())
    {
        std::cout << "\nmain - Coroutine is done!\n";
    }
    else
    {
        std::cout << "\nmain - Coroutine isn't done!\n";
    }
}

END_NS

BEGIN_NS(Case03)

// co_yield <value>로 중간중간 값을 산출하며 코루틴을 유예하는 예시 코드

struct CoroType
{
    struct promise_type
    {
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

            std::rethrow_exception(std::current_exception());
        }

        void return_void()
        {
            std::cout << "\t<return_void()>\n";
        }

        // co_return이 값을 반환하지 않으면 return_value(T)는 사용할 수 없다.
        // void return_value(int value)
        // {
        //     std::cout << "\t<return_value(T)>\n";
        // 
        //     result = value;
        // }

        // co_yield가 반환하는 값은 yield_value(T)의 매개변수 타입으로 convertible 해야 한다.
        // yield_value(T)는 유예 상태를 제어하기 위한 awaiter를 반환해야 한다.
        std::suspend_always yield_value(int value)
        {
            std::cout << "\t<yield_value(T)>\n";

            this->value = value;

            return { };
        }

        int value;
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

    int GetValue() const
    {
        // 코루틴 타입에 함수를 만들어서 promise 객체에 접근하여 값을 가져오는 방식
        return handle.promise().value;
    }

    std::coroutine_handle<promise_type> handle = nullptr;
};

CoroType CoroGenerateTwoNumbers()
{
    std::cout << "<Coroutine - Started>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    std::cout << "<Coroutine - Yielding value>\n";
    co_yield 100; // co_await promise.yield_value(100);

    std::cout << "<Coroutine - Yielding value>\n";
    co_yield 200; // co_await promise.yield_value(200);

    std::cout << "<Coroutine - Returning>\n";
    co_return;
}

void Run()
{
    std::cout << "main - Calling coroutine\n";
    auto gen = CoroGenerateTwoNumbers(); // 코루틴이 생성되었긴 해도 실행된 건 아님.

    // 코루틴 함수의 작업이 완료될 때까지 계속해서 평가를 진행한다.
    while (!gen.handle.done())
    {
        std::cout << "\nmain - Value : " << gen.GetValue() << "\n";

        std::cout << "\nmain - Resuming coroutine\n";

        // 유예 지점인 suspension point에서 실행을 재개한다.
        // task.handle(); // task.handle()은 task.handle.resume()과 같은 기능을 수행함.
        gen.handle.resume();
    }

    std::cout << "\nmain - Coroutine is done!\n";

    // main - Calling coroutine
    //         <get_return_object()>
    //         <CoroType()>
    //         <initial_suspend()>
    // 
    // main - Value : 0 <----- !! 이 값이 출력되면 안 됨. !!
    // 
    // main - Resuming coroutine
    // <Coroutine - Started>
    // <Coroutine - Executing coroutine function>
    // <Coroutine - Yielding value>
    //         <yield_value(T)>
    // 
    // main - Value : 100
    // 
    // main - Resuming coroutine
    // <Coroutine - Yielding value>
    //         <yield_value(T)>
    // 
    // main - Value : 200
    // 
    // main - Resuming coroutine
    // <Coroutine - Returning>
    //         <return_void()>
    //         <final_suspend()>
    // 
    // main - Coroutine is done!
    //         <~CoroType()>
}

END_NS

BEGIN_NS(Case04)

// Case03에서 출력된 결과를 보면 첫 번째 결과가 이상하게 나오는데
// 이는 코루틴 객체를 생성하고 시작 지점에서 실행을 유예했기 때문에 그런 것이다.
//
// 코루틴이 생성되었을 때 유예 여부를 판단하는 건 initial_suspend() 쪽이다.
// 
// 해당 함수가 반환하는 awaiter를 std::suspend_always에서 std::suspend_never로 변경하면
// 코루틴 객체가 생성된 이후 실행이 유예되지 않고 코루틴 함수를 실행할 것이며 이는 우리가 의도한 동작이다.
// 

struct CoroType
{
    struct promise_type
    {
        CoroType get_return_object()
        {
            std::cout << "\t<get_return_object()>\n";

            // return CoroType{ this };
            return this;
        }

        // 코루틴 생성 이후 실행이 유예되지 않게 initial_suspend()가 반환하는 awaiter를 std::suspend_never로 변경
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

        // co_return이 값을 반환하지 않으면 return_value(T)는 사용할 수 없다.
        // void return_value(int value)
        // {
        //     std::cout << "\t<return_value(T)>\n";
        // 
        //     result = value;
        // }

        // co_yield가 반환하는 값은 yield_value(T)의 매개변수 타입으로 convertible 해야 한다.
        // yield_value(T)는 유예 상태를 제어하기 위한 awaiter를 반환해야 한다.
        std::suspend_always yield_value(int value)
        {
            std::cout << "\t<yield_value(T)>\n";

            this->value = value;

            return { };
        }

        int value;
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

    int GetValue() const
    {
        // 코루틴 타입에 함수를 만들어서 promise 객체에 접근하여 값을 가져오는 방식
        return handle.promise().value;
    }

    std::coroutine_handle<promise_type> handle = nullptr;
};

CoroType CoroGenerateTwoNumbers()
{
    std::cout << "<Coroutine - Started>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    std::cout << "<Coroutine - Yielding value>\n";
    co_yield 100; // co_await promise.yield_value(100);

    std::cout << "<Coroutine - Yielding value>\n";
    co_yield 200; // co_await promise.yield_value(200);

    std::cout << "<Coroutine - Returning>\n";
    co_return;
}

void Run()
{
    std::cout << "main - Calling coroutine\n";
    auto gen = CoroGenerateTwoNumbers(); // 코루틴이 생성되었긴 해도 실행된 건 아님.

    // 코루틴 함수의 작업이 완료될 때까지 계속해서 평가를 진행한다.
    while (!gen.handle.done())
    {
        std::cout << "\nmain - Value : " << gen.GetValue() << "\n";

        std::cout << "\nmain - Resuming coroutine\n";

        // 유예 지점인 suspension point에서 실행을 재개한다.
        // task.handle(); // task.handle()은 task.handle.resume()과 같은 기능을 수행함.
        gen.handle.resume();
    }

    std::cout << "\nmain - Coroutine is done!\n";

    // main - Calling coroutine
    //         <get_return_object()>
    //         <CoroType()>
    //         <initial_suspend()>
    // <Coroutine - Started>
    // <Coroutine - Executing coroutine function>
    // <Coroutine - Yielding value>
    //         <yield_value(T)>
    // 
    // main - Value : 100
    // 
    // main - Resuming coroutine
    // <Coroutine - Yielding value>
    //         <yield_value(T)>
    // 
    // main - Value : 200
    // 
    // main - Resuming coroutine
    // <Coroutine - Returning>
    //         <return_void()>
    //         <final_suspend()>
    // 
    // main - Coroutine is done!
    //         <~CoroType()>
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
