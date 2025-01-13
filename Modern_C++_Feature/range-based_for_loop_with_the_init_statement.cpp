
// Update Date : 2025-01-13
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

using namespace std;

mutex mtx;
vector<int> vec;

int main()
{
    thread th1{ [] {
        while (true)
        {
            // range-based for loop with the init statement(C++20)
            // 범위 기반 for 문의 초기화 구문에 변수를 선언하는 건 C++20부터 도입된 문법이다.
            for (lock_guard<mutex> lock{ mtx }; auto& elem : vec)
            {
                cout << elem << "\n";
            }
        }
    } };

    thread th2{ [] {
        while (true)
        {
            // 너무 빨리 lock을 잡지 않게 sleep을 위에서 걸게 한다.
            this_thread::sleep_for(1s);

            lock_guard<mutex> lock{ mtx };

            vec.clear();

            static int s_Cnt = 0;

            vec.push_back(s_Cnt++);
        }
    } };

    th1.join();
    th2.join();

    return 0;
}
