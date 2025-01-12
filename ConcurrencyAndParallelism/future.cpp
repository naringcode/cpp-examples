#include <iostream>
#include <thread>
#include <future>

// https://en.cppreference.com/w/cpp/thread/async
// https://en.cppreference.com/w/cpp/thread/launch

int main()
{
    using namespace std;

    // task-based parallelism : future
    {
        // async의 연산이 끝났을 때 미래의 값을 가지고 오겠다.
        std::future<int> fut = std::async([] { return 1 + 2; });

        // get()을 쓰면 async가 결과를 반환할 때까지 기다린다.
        std::cout << fut.get() << '\n';
    }

    // 응용
    {
        constexpr int kTimes = 10;

        std::vector<std::future<int>> vec;

        auto work = [](int a, int b) {
            int res = 0;

            a += 1;
            b += 1;

            for (int x = a; x < b; x++)
            {
                res += x;
            }

            return res;
        };

        for (int i = 0; i < kTimes; i++)
        {
            vec.push_back(std::async(std::launch::async,
                work, i * 10, (i + 1) * 10));
        }

        int res = 0;

        // 등차수열 공식에 대입해서 결과가 맞나 확인해 볼 것!
        // n(1 + n) / 2
        for (int i = 0; i < kTimes; i++)
        {
            res += vec[i].get();
        }

        std::cout << res << '\n';
    }

    return 0;
}
