#include <iostream>
#include <thread>

// 최적화 코드를 적용하지 않으면 sum.fetch_add()는 함수를 호출한다.
// 다만 최적화 코드를 적용하면 CPU 인스트럭션으로 연산을 수행한다.

// atomic으로 연산을 수행할 때는 일반 변수와는 차별화되게 fetch_xxx(), store(), load()를 쓰는 것이 좋다.

std::atomic<int32_t> sum = 0;

void Increase()
{
    for (int32_t i = 0; i < 1'000'000; i++)
    {
        sum.fetch_add(1);

        // sum++;
    }

    // long a = 10;
    // _InterlockedIncrement(&a);
}

void Decrease()
{
    for (int32_t i = 0; i < 1'000'000; i++)
    {
        sum.fetch_add(-1);

        // sum--;
    }
}

int main()
{
    std::thread th1(Increase);
    std::thread th2(Decrease);

    th1.join();
    th2.join();

    std::cout << sum << '\n';

    return 0;
}
