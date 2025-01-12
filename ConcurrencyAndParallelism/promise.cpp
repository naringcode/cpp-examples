#include <iostream>
#include <thread>
#include <future>

int main()
{
    using namespace std;

    // task-based parallelism : future and promise
    {
        // future를 스레드와 함께 사용하기 위해선 promise가 필요하다.
        // std::thread의 반환형은 future가 아니다.
        //
        // promise에 값을 넣으면 future를 통해 빼올 수 있다.
        //
        // promise를 thread의 인자로 넣는 것이 핵심이다.

        std::promise<int> prom;
        std::future<int> fut = prom.get_future();

        std::thread th = std::thread([](std::promise<int>&& prom)
        {
            prom.set_value(10);
        }, std::move(prom));

        std::cout << fut.get();

        th.join(); // 스레드라서 main을 빠져나가지 않게 대기하는 뭔가가 필요
    }


    return 0;
}
