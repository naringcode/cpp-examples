// Update Date : 2024-01-01
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

// 사양
// CPU : AMD Ryzen 5 7600 6-Core Processor 3.80 GHz
// RAM : 32GB

#include <iostream>
#include <Windows.h>

#include <vector>
#include <algorithm>

// 10억 번 연산을 한다고 했을 때 걸리는 시간
// 
// # Debug
//   Int    : 대략 600ms
//   Float  : 대략 5000ms
//   Faster : 약 8배
// 
// # Release
//   Int    : 대략 400ms
//   Float  : 대략 5000ms
//   Faster : 약 12배
// 

int main()
{
#ifdef _DEBUG
    constexpr int kInner = 10;
    constexpr int kOuter = 1'000'000'000;
#else
    constexpr int kInner = 10;
    constexpr int kOuter = 1'000'000'000;
#endif

    volatile int   intVal   = 0;
    volatile float floatVal = 0.0f;

    std::vector<uint64_t> intTimeVec;
    std::vector<uint64_t> floatTimeVec;

    for (int times = 0; times < kInner; times++)
    {
        uint64_t startTick = GetTickCount64();

        for (int i = 0; i < kOuter; i++)
        {
            intVal += 1;
            intVal -= 1;
        }

        uint64_t endTick = GetTickCount64();

        intTimeVec.push_back(endTick - startTick);

        std::cout << "Int Times... " << times << '\n';
    }

    for (int times = 0; times < kInner; times++)
    {
        uint64_t startTick = GetTickCount64();

        for (int i = 0; i < kOuter; i++)
        {
            floatVal += 1.0f;
            floatVal -= 1.0f;
        }

        uint64_t endTick = GetTickCount64();

        floatTimeVec.push_back(endTick - startTick);

        std::cout << "Float Times... " << kInner + times << '\n';
    }

    system("cls");

    std::sort(intTimeVec.begin(), intTimeVec.end());
    std::sort(floatTimeVec.begin(), floatTimeVec.end());

    std::cout << "Int Calc Time : \n";
    for (auto intTime : intTimeVec)
    {
        std::cout << '\t' << intTime << '\n';
    }

    std::cout << "\nFloat Calc Time : \n";
    for (auto floatTime : floatTimeVec)
    {
        std::cout << '\t' << floatTime << '\n';
    }

    std::cout << "\nHow Much Faster? : \n";
    for (int i = 0; i < intTimeVec.size(); i++)
    {
        std::cout << '\t' << float(floatTimeVec[i]) / intTimeVec[i] << '\n';
    }

    return 0;
}
