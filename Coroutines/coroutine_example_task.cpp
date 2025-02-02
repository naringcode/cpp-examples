// Update Date : 2025-02-02
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <fstream>
#include <string>
#include <coroutine>
#include <thread>

using namespace std::chrono_literals;

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
// 12. coroutine_example_task.cpp <-----
// 13. coroutine_example_generator.cpp
// 14. coroutine_example_command_with_lambda.cpp
// 15. coroutine_example_coro_from_member_funcs.cpp
//

// https://en.cppreference.com/w/cpp/language/coroutines
// https://en.wikipedia.org/wiki/Asynchrony_(computer_programming)
// https://en.wikipedia.org/wiki/Concurrency_(computer_science)

// 대부분의 경우 코루틴은 Task 유형과 Generator 유형으로 구분된다.
// 
// 세부적으로 볼 때 두 유형에 포함되지 않더라도 넓은 관점으로 보면 Task 유형과 Generator 유형에 포함되는 경우가 많다.

// Task 유형에 해당하면 코루틴 타입 이름에 Task를 붙이는 것이 좋고,
// Generator 유형에 해당하면 Generator를 붙이는 것이 좋다.
//
// Task 유형 : co_await, co_yield를 활용하여 비동기 작업을 수행하는 유형의 코루틴이다.
// Generator 유형 : 코루틴을 재개할 때마다 일정한 패턴으로 값을 생성하는 유형의 코루틴이다.
// 

// Task 유형은 비동기와 동시성 프로그래밍을 하기 적합한 유형이다.
//
// 비동기
// : 요청을 보냈을 때 보낸 요청에 대한 결과를 받을 때까지 기다리는 것이 아닌 나중에 요청한 결과를 받는 방식이다.
// : 요청한 작업이 완료될 때까지 대기하지 않고 다른 작업을 수행할 수 있다.
//
// C++의 코루틴은 실행을 유예하는 것이 가능하기 때문에 최종 결과를 나중에 받는 비동기 프로그래밍을 진행할 수 있다.
// 
// 동시성
// : 각 Task들을 빠르게 전환하면서 실행하여 여러 작업이 동시에 실행되는 것처럼 보이게 하는 방식이다.
// : 쉽게 말해 동시성이란 작업을 교대할 수 있는 성질을 말한다.
// 
// C++의 코루틴을 Task로 사용하여 동시성 프로그래밍을 구현하면
// 작업 전환이 같은 스레드 내에서 이루어지기 때문에 컨텍스트 스위칭이 발생하지 않는다.
// 
// C++의 코루틴을 사용하면 호출자 쪽을 blocking하지 않고 작업을 교대할 수 있다.
// 호출자와 코루틴은 동시성이란 성질을 통해 유예 지점(suspension point)을 기반으로 협력하여 작업을 진행할 수 있다.
// 
// (주의) 동시성(Concurrency)과 병렬성(Parallelism)은 다르다.
// 

class FileReadTask
{
public:
    struct promise_type
    {
        FileReadTask get_return_object()
        {
            // return Task{ this };
            return this;
        }

        std::suspend_always initial_suspend() { return { }; }
        std::suspend_always final_suspend() noexcept { return { }; }

        void unhandled_exception()
        {
            std::rethrow_exception(std::current_exception());
        }

        void return_void()
        { }

        std::ifstream inputStream{ };
        std::string   readString{ };
    };

public:
    FileReadTask(promise_type* prom)
        : _handle{ std::coroutine_handle<promise_type>::from_promise(*prom) }
    { }

    ~FileReadTask()
    {
        if (_handle != nullptr)
        {
            _handle.destroy();

            _handle = nullptr;
        }
    }

public:
    void OpenFile(const std::string& fileName)
    {
        _handle.promise().inputStream.open(fileName);

        if (!_handle.promise().inputStream)
        {
            throw std::logic_error{ "Could not open the file for reading" };
        }
    }

    std::string GetReadString() const
    {
        return _handle.promise().readString;
    }

public:
    bool IsDone()
    {
        return _handle.done();
    }

    void Resume()
    {
        if (IsDone())
            return;

        std::cout << "[ Switch to coroutine ]\n";

        _handle.resume();
    }

private:
    std::coroutine_handle<promise_type> _handle = nullptr;
};

struct FileReadAwaiter
{
    bool await_ready()
    {
        return false;
    }

    // 어떠한 코루틴인지 파악하기 위해 promise_type을 명시해야 한다.
    void await_suspend(std::coroutine_handle<FileReadTask::promise_type> handle)
    {
        auto& prom = handle.promise();

        prom.readString.resize(bytes);

        std::this_thread::sleep_for(3s); // delay
        prom.inputStream.read(prom.readString.data(), bytes);

        // 파일의 끝에 도달했는지 확인
        if (prom.inputStream.eof())
        {
            nextAvailable = false;

            return;
        }

        nextAvailable = true;
    }

    bool await_resume()
    {
        return nextAvailable;
    }

    int  bytes = 0;
    bool nextAvailable = false;
};

FileReadTask CoroReadFileAsyncConcurrency()
{
    bool nextAvailable = true;

    while (nextAvailable)
    {
        // awaiter : co_await 표현식에 의해 생성되는 객체
        // awaitable : 표현식의 피연산자로 사용되는 대상
        nextAvailable = co_await FileReadAwaiter{ 40 };
    }

    co_return;
}

int main()
{
    try
    {
        auto task = CoroReadFileAsyncConcurrency();

        // 코루틴 객체를 생성한 다음 파일을 연다.
        task.OpenFile(__FILE__);

        while (!task.IsDone())
        {
            task.Resume();

            std::cout << task.GetReadString() << "\n";

            std::cout << "[ Switch to main ]\n\n";
        }

        std::cout << "\n[ Coroutine is done! ]\n";
    }
    catch (std::exception& ex)
    {
        std::cout << "Exception : " << ex.what() << '\n';
    }

    return 0;
}
