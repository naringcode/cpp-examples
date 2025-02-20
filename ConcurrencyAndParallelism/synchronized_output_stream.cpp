// Update Date : 2025-02-20
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <format>
#include <thread>
#include <mutex>
#include <syncstream>
#include <vector>

// https://en.cppreference.com/w/cpp/io/basic_osyncstream
// https://en.wikipedia.org/wiki/Interleaving

// C++20부터는 여러 스레드에서 동일 stream을 대상으로 출력 작업을 진행할 때 사용할 동기화 매커니즘을 제공한다.
// 이를 통해 출력 도중 발생하는 인터리빙 문제를 방지할 수 있다.

#define BEGIN_NS(name) namespace name {
#define END_NS }

BEGIN_NS(Case01)

// 출력을 동기화하지 않으면 출력 사이에 다른 대상이 끼어드는 인터리빙 현상이 발생한다.

void Worker(int id)
{
    for (int i = 0; i < 10; i++)
    {
        std::cout << "[Thread " << id << "] " << "Hello " << "World " << "and " << "More !" << '\n';
    }
}

void Run()
{
    std::vector<std::jthread> jthreads;

    for (int i = 0; i < 10; i++)
    {
        jthreads.emplace_back(Worker, i);
    }
}

END_NS

BEGIN_NS(Case02)

// mutex를 사용하여 출력을 동기화하는 기존 방법

std::mutex mtx;

void Worker(int id)
{
    for (int i = 0; i < 10; i++)
    {
        std::lock_guard lock{ mtx };

        std::cout << "[Thread " << id << "] " << "Hello " << "World " << "and " << "More !" << '\n';
    }
}

void Run()
{
    std::vector<std::jthread> jthreads;

    for (int i = 0; i < 10; i++)
    {
        jthreads.emplace_back(Worker, i);
    }
}

END_NS

BEGIN_NS(Case03)

// C++20에서는 stream output을 동기적으로 진행하기 위한 클래스인 std::osyncstream을 지원한다.

void Worker(int id)
{
    for (int i = 0; i < 10; i++)
    {
        // 방법 1
        // std::osyncstream syncOut{ std::cout };
        // 
        // syncOut << "[Thread " << id << "] " << "Hello " << "World " << "and " << "More !" << '\n';

        // 방법 2
        std::osyncstream{ std::cout } << "[Thread " << id << "] " << "Hello " << "World " << "and " << "More !" << '\n';
    }
}

void Run()
{
    std::vector<std::jthread> jthreads;

    for (int i = 0; i < 10; i++)
    {
        jthreads.emplace_back(Worker, i);
    }
}

END_NS

int main()
{
    // Case01::Run();
    // Case02::Run();
    Case03::Run();

    return 0;
}
