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
// 2. coroutine_infrastructures.cpp <-----
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
// 15. coroutine_example_coro_from_member_funcs.cpp
//

// https://en.cppreference.com/w/cpp/language/coroutines

// 코루틴 함수를 사용하기 위해선 코루틴 타입이 필요하며 코루틴 타입은 최소한 다음 3가지 요소가 필요하다.
// - Promise type(user-defined promise_type)
// : 코루틴 행동 방식이 정의된 타입
// 
// - Coroutine handle(std::coroutine_handle<promise_type>)
// : 코루틴에 접근하기 위한 핸들
// 
// - Awaiter(std::suspend_always, std::suspend_never, user-defined awaiter)
// : 코루틴 유예 지점 발생 시 이를 처리할 방식이 담긴 대기자
// 
// 여기서 promise_type와 awaiter는 정해진 양식을 따라야 한다.
// 이 문서에서는 promise_type만 보도록 할 것이다.
// 
// --------------------------------------------------
// 
// promise_type는 다음 양식에 따라 코드를 작성해야 한다.
// 
// 1. 선택적인 인터페이스
// 
// - 기본 생성자 : 코루틴 함수를 invoke하여 std::coroutine_traits를 거치며 promise_type을 생성할 때 필요하다(반드시 기본 생성자로 되어 있어야 함).
// 
// - 소멸자 : promise_type이 소멸하는 과정에서 호출된다.
// 
// - operator new(size) : 코루틴 프레임을 할당할 때 호출되며 연산자 오버로딩을 통해 사용자가 할당 과정을 구성할 수 있다.
//   - promise_type을 할당하기 위해 호출되는 것이 아니니까 주의해야 함.
// 
// - operator delete(mem) : 코루틴 프레임을 해제할 때 호출되며 연산자 오버로딩을 통해 사용자가 해제 과정을 구성할 수 있다.
//   - promise_type을 해제하기 위해 호출되는 것이 아니니까 주의해야 함.
// 
// - get_return_object_on_allocation_failure() : 코루틴 프레임을 할당할 수 없을 때 호출된다.
//   - https://en.cppreference.com/w/cpp/language/coroutines#Dynamic_allocation
//   - 해당 함수가 정의되어 있지 않다면 std::bad_alloc을 던짐.
// 
// 2. 필수적인 인터페이스
// 
// - get_return_object() : 코루틴을 invoke했을 때 사용할 코루틴 객체를 반환하기 위한 함수이다.
// 
// - initial_suspend() : get_return_object() 호출 이후 호출되며 awaiter를 반환해야 한다.
//   - 반환하는 awaiter를 통해 코루틴 함수의 시작 지점에서 실행을 유예할 것인지를 제어할 수 있음.
// 
// - final_suspend() : 코루틴 함수의 실행을 끝마쳤을 때 호출되며 awaiter를 반환해야 한다.
//   - 반환하는 awaiter를 통해 코루틴 종료 지점에서 유예하여 코루틴 프레임 해제의 유예 여부를 제어할 수 있음.
// 
// - unhandled_exception() : 코루틴 실행 단계에서 예외가 발생했을 때 호출된다(코루틴 생성 단계일 때는 호출되지 않음).
// 
// 3. 둘 중 하나는 필수인 인터페이스(둘 중 하나만 구현해야 하며 둘 다 적용하는 건 안 됨)
// 
// - return_void() : co_return이 아무것도 반환하지 않거나 co_return이 명시되어 있지 않을 때 호출된다.
// 
// - return_value(T) : co_return이 값을 반환할 경우 호출되며 오버로딩이 가능하다.
// 
// 4. 조건부로 필수인 인터페이스
// 
// - yield_value(T) : co_yield에 대응되는 함수로 awaiter를 반환해야 한다.
//   - 해당 함수를 통해 중간 결과를 산출할 수 있으며 awaiter를 통해 코루틴 실행의 유예 여부를 판단할 수 있음.
//   - 해당 함수는 오버로딩이 가능함.
// 
// - await_transform(T) : co_await의 피연산자를 promise_type에서 받아 처리하고 awaiter를 반환하는 함수이다.
//   - 반환하는 awaiter를 통해 함수가 호출된 지점에서 코루틴 실행을 유예할 것인지를 제어할 수 있음.
//   - await_transform(T)가 적용된 promise_type은 awaitable로 간주함.
// 
// --------------------------------------------------
// 
// promise_type은 인터페이스에 맞춰서 최소한 다음 5개의 함수를 제공해야 한다.
// - get_return_object()
// - initial_suspend()
// - final_suspend()
// - unhandled_exception()
// - return_void() / return_value(T) : 택1
// 
// promise_type의 생성자는 반드시 기본 생성자의 형태로 제공되어야 한다.
// 
struct CoroType // 클래스로 해도 됨(promise_type은 공개되어(public) 있어야 함).
{
    // promise_type이라는 이름과 인터페이스가 되는 각 함수의 이름은 컴파일러에 의해 약속되어 있다.
    struct promise_type // 클래스로 해도 됨(인터페이스는 공개되어 있어야 함).
    {
        // 선택 : 생성자와 소멸자는 생략 가능(생성자는 기본 생성자만 가능)
        promise_type() { std::cout << "\t<promise_type()>\n"; }
        ~promise_type() { std::cout << "\t<~promise_type()>\n"; }

        // 필수 : 코루틴을 invoke했을 때 사용할 코루틴 객체를 반환하기 위한 함수
        CoroType get_return_object()
        {
            std::cout << "\t<get_return_object()>\n";

            // return CoroType{ this };
            return this;
        }

        // 필수 : 코루틴 객체가 생성되었을 때 호출되며 awaiter를 반환하여 코루틴 시작 지점에서 실행을 유예할 것인지를 제어하기 위한 함수
        // std::suspend_always를 반환하면 실행의 시작 지점(생성의 시작 지점이 아님)에서 실행을 유예한다.
        std::suspend_always initial_suspend()
        {
            std::cout << "\t<initial_suspend()>\n";
            
            return { };
        }

        // 필수 : 코루틴 함수의 실행을 끝마쳤을 때 호출되며 awaiter를 반환하여 코루틴 종료 지점에서 프레임 해제의 유예 여부를 제어하기 위한 함수
        // std::suspend_always를 반환하면 실행의 마지막 지점에서 실행을 유예한다.
        std::suspend_always final_suspend() noexcept // final_suspend()는 noexcept 유형으로 작성되어야 함.
        {
            std::cout << "\t<final_suspend()>\n";

            return { };
        }

        // 필수 : 코루틴 함수에서 예외가 발생했을 때 호출되는 함수
        void unhandled_exception()
        {
            std::cout << "\t<unhandled_exception()>\n";

            ::terminate();
        }

        // 둘 중 하나는 필수 : co_return이 아무것도 반환하지 않을 때 호출되는 함수(co_return이 명시되어 있지 않은 경우 해당 인터페이스가 필요함)
        void return_void()
        {
            std::cout << "\t<return_void()>\n";
        }

        // 둘 중 하나는 필수 : co_return이 값을 반환할 경우 호출되며 오버로딩 가능한 함수
        // void return_value(T value) { ... }

        // 조건부 필수 : co_yield에 의해 호출되는 함수로 코루틴 실행의 유예 여부 판단 및 중간 결과를 산출하기 위해 사용됨(오버로딩 가능).
        // Awaiter yield_value(T value) { ... }

        // 조건부 필수 : co_await의 피연산자를 promise_type에서 받아 처리하고 awaiter를 반환하는 함수(await_transform(T)가 적용된 promise_type은 awaitable로 간주함)
        // Awaiter await_transform(T value) { ... }

        // 선택 : 코루틴 프레임을 할당할 수 없을 때 호출되는 함수(해당 함수가 정의되어 있지 않다면 std::bad_alloc을 던짐)
        // static CoroType get_return_object_on_allocation_failure() { ... }

        // 선택 : 코루틴 프레임을 할당할 때 호출되며 연산자 오버로딩을 통해 사용자가 할당 과정을 구성 가능
        void* operator new(std::size_t size)
        {
            std::cout << "\t<operator new(size)>\n";

            if (void* mem = std::malloc(size))
                return mem;

            return nullptr;
        }

        // 선택 : 코루틴 프레임을 해제할 때 호출되며 연산자 오버로딩을 통해 사용자가 해제 과정을 구성 가능
        void operator delete(void* mem)
        {
            std::cout << "\t<operator delete(mem)>\n";

            std::free(mem);
        }
    };

    // get_return_object()에서 코루틴 객체를 구성하여 반환하는 과정에서 호출되는 생성자
    CoroType(promise_type* prom)
        : handle{ std::coroutine_handle<promise_type>::from_promise(*prom) }
    {
        std::cout << "\t<CoroType()>\n";
    }

    ~CoroType()
    {
        std::cout << "\t<~CoroType()>\n";

        // final_suspend()에서 코루틴 종료 시점을 유예시켰거나 아직 코루틴 함수 실행을 끝마치지 않은 상태에서 코루틴 객체가 소멸 과정에 들어간 경우
        // 소멸자 내부가 되었든 코루틴 객체를 사용하는 쪽이 되었든 코루틴 핸들에 접근해서 destroy()를 호출해야 메모리 누수가 나지 않는다.
        if (handle != nullptr)
        {
            // 코루틴 핸들이 소멸한다고 해도 코루틴 프레임의 소멸까지 자동적으로 연계되는 건 아니다.
            handle.destroy();

            handle = nullptr;
        }
    }

    // 코루틴 핸들은 코루틴 프레임을 가리키기 위한 용도로 사용된다.
    std::coroutine_handle<promise_type> handle = nullptr;
};

// CoroType : 코루틴 타입
// CoroFunc : 코루틴 함수
CoroType CoroFunc()
{
    std::cout << "<Coroutine - Started>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    // 실행을 중단하고 유예 지점(suspension point) 설정
    co_await std::suspend_always{ }; // 코루틴을 멈추는 목적으로 사용할 built-in awaiter
    std::cout << "<Coroutine - Resuming coroutine...>\n";
    std::cout << "<Coroutine - Executing coroutine function>\n";

    std::cout << "<Coroutine - Returning>\n";
    co_return; // co_return에 의해 코루틴 함수가 반환되면 더 이상 재개할 수 없음.
}

int main()
{
    std::cout << "main - Calling coroutine\n";

    // 1. 코루틴 함수 invoke
    // 2. std::coroutine_traits
    // 3. promise_type 생성
    // 4. get_return_object()을 호출하여 코루틴 객체 반환
    // 5. initial_suspend() 호출
    // 6. 코루틴 객체를 호출자에게 반환
    CoroType task = CoroFunc(); // 코루틴이 생성되었긴 해도 실행된 건 아님.

    std::cout << "\nmain - Resuming coroutine\n";
    task.handle.resume(); // 직접 재개해야 "<Coroutine - Started>"를 출력함.

    std::cout << "\nmain - Resuming coroutine\n";
    task.handle.resume(); // 유예 지점에서 실행을 재개함(std::suspend_always{ } 이후의 명령어 실햄).
    
    // DISASTER : 코루틴 함수가 끝난 상태에서 재개하려고 하면 크래시가 발생하니 주의해야 한다.
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

    return 0;
}
