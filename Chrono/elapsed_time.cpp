#include <iostream>
#include <chrono>

// using namespace std;

int main()
{
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point endTime;

    startTime = std::chrono::high_resolution_clock::now();

    while (true)
    {
        char ch;

        // Do Something
        std::cin >> ch;

        if ('0' == ch)
            break;

        endTime = std::chrono::high_resolution_clock::now();

        double elapsedTime = (double)std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

        startTime = endTime;

        std::cout << "Elapsed Time : " << elapsedTime / 1000.0 << "ms\n";
    }

    return 0;
}