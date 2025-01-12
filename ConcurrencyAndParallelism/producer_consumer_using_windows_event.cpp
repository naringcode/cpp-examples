#include <Windows.h>

#include <iostream>

#include <thread>
#include <mutex>
#include <queue>

// Event 기반의 Producer-Consumer 패턴

std::mutex mtx;
std::queue<int> q;

HANDLE hHandle = NULL;

void Producer()
{
    using namespace std::chrono;

    while (true)
    {
        auto waitMS = rand() % 4000 + 1000;

        // Produce
        {
            std::unique_lock<std::mutex> guard(mtx);

            q.push(waitMS);
        }

        ::SetEvent(hHandle); // 상태를 Signal로 전환

        std::this_thread::sleep_for(std::chrono::milliseconds(waitMS));
    }
}

void Consumer()
{
    while (true)
    {
        // Event가 Signal 상태가 될 때까지 무한정 대기
        ::WaitForSingleObject(hHandle, INFINITE);

        std::unique_lock<std::mutex> guard(mtx);

        // 실제로는 while을 돌면서 뽑아와야 한다.
        // 1. push()
        // 2. SetEvent()
        // 3. push()
        // 4. SetEvent()
        // 5. WaitForSingleObject() 반응
        // 6. 하나만 뽑아서 처리하면 queue에 데이터가 쌓일 수 있다.
        if (false == q.empty())
        {
            int data = q.front();
            q.pop();

            std::cout << data << '\n';
        }
    }
}

int main()
{
    // 커널 오브젝트
    hHandle = ::CreateEvent(NULL, FALSE, FALSE, NULL);

    std::thread th1(Producer);
    std::thread th2(Consumer);

    th1.join();
    th2.join();

    ::CloseHandle(hHandle);

    return 0;
}
