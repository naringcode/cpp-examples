#include <iostream>

#include <thread>
#include <mutex>
#include <queue>

std::mutex mtx;
std::queue<int> q;

// CV는 User-Level Object(커널 오브젝트가 아니다)
std::condition_variable cv;

void Producer()
{
    using namespace std::chrono;

    while (true)
    {
        // Produce
        {
            std::unique_lock<std::mutex> lock(mtx);

            q.push(100);
        }

        cv.notify_one(); // Wait 상태인 스레드가 있으면 딱 하나만 깨운다.

        // 소비하는 쪽이 조금 더 빠르게 하기
        std::this_thread::yield();
    }
}

void Consumer()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(mtx);

        cv.wait(lock, []() { return false == q.empty(); });

        // while (false == q.empty())
        {
            int data = q.front();
            q.pop();

            // q에 쌓인 데이터를 출력하게 변경
            std::cout << q.size() << '\n';
        }
    }
}

int main()
{
    std::thread th1(Producer);
    std::thread th2(Consumer);

    th1.join();
    th2.join();

    return 0;
}
