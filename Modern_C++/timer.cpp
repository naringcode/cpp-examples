#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>

// https://learn.microsoft.com/ko-kr/cpp/standard-library/high-resolution-clock-struct?view=msvc-170
// https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

// https://learn.microsoft.com/ko-kr/cpp/standard-library/duration-class?view=msvc-170
// https://en.cppreference.com/w/cpp/chrono/duration

class Timer
{
private:
    using clock_t  = std::chrono::high_resolution_clock; // 고해상도 시계
    using second_t = std::chrono::duration<double, std::ratio<1>>;

public:
    void elapsed()
    {
        std::chrono::time_point<clock_t> endTime = clock_t::now();

        // time_point 간격을 초 단위로 변환해서 출력
        std::cout << std::chrono::duration_cast<second_t>(endTime - startTime).count() << '\n';
    }

private:
    std::chrono::time_point<clock_t> startTime = clock_t::now();
};

int main()
{
    using namespace std;

    Timer timer;

    for (int i = 0; i < 100'000'000; i++)
    {
        string str = "ABC";

        str += "DEF";
    }

    timer.elapsed();

    return 0;
}
