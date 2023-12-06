#include <iostream>
#include <chrono>

constexpr int kRow = 10000;
constexpr int kCol = 10000;

int arr[kCol][kRow];

// x 쪽에 접근하는 방식으로 설계하는 것이 훨씬 빠르게 동작한다.

int main() 
{
    using clock_t  = std::chrono::high_resolution_clock;
    using second_t = std::chrono::duration<double>;

    // 캐시 메모리에 올리기(L1, L2, L3 캐시의 용량이 다르다는 것에 유의)
    for (int y = 0; y < kCol; y++)
    {
        for (int x = 0; x < kRow; x++)
        {
            arr[y][x] = 0;
        }
    }

    // Row 먼저
    {
        auto tpStart = clock_t::now();

        int sum = 0;
        for (int y = 0; y < kCol; y++)
        {
            for (int x = 0; x < kRow; x++)
            {
                sum += arr[y][x];
            }
        }

        auto tpEnd = clock_t::now();

        std::cout << std::chrono::duration_cast<second_t>(tpEnd - tpStart).count() << '\n';
    }

    // Col 먼저
    {
        auto tpStart = clock_t::now();

        int sum = 0;
        for (int x = 0; x < kRow; x++)
        {
            for (int y = 0; y < kCol; y++)
            {
                sum += arr[y][x];
            }
        }

        auto tpEnd = clock_t::now();

        std::cout << std::chrono::duration_cast<second_t>(tpEnd - tpStart).count() << '\n';
    }

    return 0;
}