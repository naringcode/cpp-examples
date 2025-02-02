// Update Date : 2025-02-02
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <coroutine>
#include <memory>

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
// 13. coroutine_example_generator.cpp
// 14. coroutine_example_command_with_lambda.cpp
// 15. coroutine_example_coro_from_member_funcs.cpp <-----
//

// https://en.cppreference.com/w/cpp/language/coroutines

// 객체의 멤버 함수를 코루틴으로 만들어서 사용하는 것도 가능하다.
//
// (주의) 객체가 소멸하면 this 포인터는 더 이상 유효하지 않으니 각별히 주의해야 한다.
// (중요) 코루틴이 끝나기 전까지 객체는 살아있어야 한다.

struct MemberFuncTask
{
    struct promise_type
    {
        promise_type() { std::cout << "\t<promise_type()>\n"; }
        ~promise_type() { std::cout << "\t<~promise_type()>\n"; }

        MemberFuncTask get_return_object()
        {
            std::cout << "\t<get_return_object()>\n";

            // return MemberFuncTask{ this };
            return this;
        }

        // 코루틴 생성 시 시작 지점에서 실행을 유예하지 않고 shared_from_this()가 호출되게 유도한다.
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

    MemberFuncTask(promise_type* prom)
        : handle{ std::coroutine_handle<promise_type>::from_promise(*prom) }
    {
        std::cout << "\t<MemberFuncTask()>\n";
    }

    ~MemberFuncTask()
    {
        std::cout << "\t<~MemberFuncTask()>\n";

        if (handle != nullptr)
        {
            //handle.destroy();

            handle = nullptr;
        }
    }

    std::coroutine_handle<promise_type> handle = nullptr;
};

class FooObject : public std::enable_shared_from_this<FooObject>
{
public:
    FooObject(int x, int y)
        : _x{ x }, _y{ y }
    {
        std::cout << "<FooObject()>\n\n";
    }

    ~FooObject()
    {
        std::cout << "<~FooObject()>\n";
    }

public:
    MemberFuncTask MakeTask()
    {
        std::cout << "<MemberFuncTask - Started>\n";
        std::cout << "<MemberFuncTask - Executing coroutine function>\n";

        // 코루틴 실행을 끝마칠 때까지 객체를 살려둔다.
        // std::suspend_never initial_suspend() { ... }
        auto self = this->shared_from_this();

        co_await std::suspend_always{ };
        std::cout << "<MemberFuncTask - Resuming coroutine...>\n";
        std::cout << "<MemberFuncTask - Executing coroutine function>\n";

        std::cout << "<MemberFuncTask - x : " << _x << ">\n";

        co_await std::suspend_always{ };
        std::cout << "<MemberFuncTask - Resuming coroutine...>\n";
        std::cout << "<MemberFuncTask - Executing coroutine function>\n";

        std::cout << "<MemberFuncTask - y : " << _y << ">\n";

        std::cout << "<MemberFuncTask - Returning>\n";
        co_return;
    }

private:
    int _x;
    int _y;
};

int main()
{
    try
    {
        // FooObject fooObj{ 10, 20 }; // bad_weak_ptr
        std::shared_ptr<FooObject> fooObj = std::make_shared<FooObject>(10, 20);

        std::cout << "main - Calling coroutine\n";
        auto task = fooObj->MakeTask(); // fooObj.MakeTask();

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

    std::cout << "\nmain - Out of try-catch\n";

    return 0;
}
