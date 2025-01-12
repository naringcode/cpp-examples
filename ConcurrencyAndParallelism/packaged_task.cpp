#include <iostream>
#include <future>

// packaged_task는 future를 다루는 방법 중 하나.
// 원하는 함수의 실행 결과를 원하는 시점에 실행해서 받을 수 있다.
// 보통은 스레드를 생성해서 실행한다.
//
// task를 한 번 실행한 이후 나중에 다시 실행하면 예외(future_error)가 발생한다.

int main() 
{
    std::packaged_task<int()> task([]() { return 100; });
    std::future<int> future = task.get_future();

    // 실행
    task();

    try 
    {
        // 다시 실행 시도하면 예외 발생
        task();
    }
    catch (const std::future_error& e) 
    {
        std::cout << "Exception : " << e.what() << '\n';
    }

    std::cout << future.get() << '\n';

    return 0;
}