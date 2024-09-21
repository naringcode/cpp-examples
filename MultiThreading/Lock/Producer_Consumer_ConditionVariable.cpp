#include <iostream>

#include <vector>
#include <queue>

#include <atomic>
#include <mutex>

using namespace std;

atomic<bool> g_IsRunning;

mutex mtx;
condition_variable cv; // CV는 User-Level Object(커널 오브젝트가 아니다)

queue<int> q;

void Producer()
{
    while (g_IsRunning.load())
    {
        //
        {
            unique_lock<mutex> lock(mtx);

            q.push(10000);
        }

        cv.notify_one();

        this_thread::sleep_for(500ms);
    }

    cout << "End 1\n";
}

void Consumer()
{
    while (g_IsRunning.load())
    {
        unique_lock<mutex> lock(mtx);

        cv.wait(lock, []() { 
            if (false == g_IsRunning.load())
                return true;

            return q.empty() == false;
        });

        // 종료 로직을 대비한 코드
        if (false == g_IsRunning.load())
            break;

        // true면 실행
        int val = q.front();
        q.pop();

        cout << val << '\n';
    }

    cout << "End 2\n";
}

int main()
{
    g_IsRunning = true;

    thread th1{ Producer };
    thread th2{ Consumer };

    this_thread::sleep_for(5000ms);

    //
    // g_IsRunning.store(false);
    // cv.notify_one();
    {
        // Lock을 걸고 값을 수정한 다음 notify_one()을 해야 안전한 느낌
        // notify()를 거는 것과 공유 자원을 대상으로 lock을 잡고 푸는 것은 원자적이지 않다.
        unique_lock<mutex> lock(mtx);

        g_IsRunning.store(false);
    }

    cv.notify_one();
    //

    th1.join();
    th2.join();

    return 0;
}
