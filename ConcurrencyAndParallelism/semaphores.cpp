// Update Date : 2025-02-20
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <format>
#include <string>
#include <random>
#include <thread>
#include <semaphore>
#include <vector>

// https://en.cppreference.com/w/cpp/thread#Semaphores
// https://en.wikipedia.org/wiki/Semaphore_(programming)

// C++20부터는 공식적으로 세마포어를 지원한다.
// - std::counting_semaphore
// - std::binary_semaphore

// Mutex는 스레드의 소유권이 있는 배타적인 잠금(Exclusive Lock)이기 때문에 Lock을 한 쪽에서 Unlock도 해야 한다.
// 또한 Mutex는 한 번에 하나의 스레드만 공유 자원을 사용할 수 있게 제한하는 동기화 기법을 말한다.
//
// Semaphore는 카운팅 기반이라 스레드의 소유권이 없기 때문에 acquire한 쪽이 아닌 다른 스레드에서 release를 처리해도 된다.
// Semaphore는 여러 스레드가 공유 자원에 접근하는 것을 허용하는 동기화 기법을 말한다.

using namespace std::chrono_literals;

#define BEGIN_NS(name) namespace name {
#define END_NS }

BEGIN_NS(Case01)

// 생성자의 인자를 반드시 기입해야 한다.
std::counting_semaphore sema{ 3 };

void Worker(std::string name, int workingTime)
{
    // 스레드를 얼마나 생성했는지는 관계 없이 세마포어에 기입한 desired 값 만큼만 접근을 허용한다.
    sema.acquire();
    {
        std::cout << std::format("# {} acquired a semaphore\n", name);

        for (int i = 0; i < workingTime; i++)
        {
            std::this_thread::sleep_for(1s);

            std::cout << std::format("Working {}[{}s]...\n", name, i + 1);
        }

        std::cout << std::format("Done {}!\n", name);
        
        std::cout << std::format("# {} released the semaphore\n", name);
    }
    sema.release();
}

void Run()
{
    std::vector<std::jthread> jthreads;

    for (int i = 0; i < 8; i++)
    {
        std::string name = "A";
        name[0] += i;

        jthreads.emplace_back(Worker, name, std::random_device{ }() % 4 + 2);
    }

    for (std::jthread& th : jthreads)
    {
        th.join();
    }

    std::cout << "All tasks have finished!\n";
}

END_NS

BEGIN_NS(Case02)

// using binary_semaphore = counting_semaphore<1>;
// binary_semaphore는 desired 값이 1인 세마포어를 말한다.
// 뮤텍스와 유사하게 동작하지만 세마포어는 스레드의 소유권을 갖지 않는다.
std::binary_semaphore sema{ 1 };

void Worker(std::string name, int workingTime)
{
    // 스레드를 얼마나 생성했는지는 관계 없이 세마포어에 기입한 desired 값 만큼만 접근을 허용한다.
    sema.acquire();
    {
        std::cout << std::format("# {} acquired a semaphore\n", name);

        for (int i = 0; i < workingTime; i++)
        {
            std::this_thread::sleep_for(1s);

            std::cout << std::format("Working {}[{}s]...\n", name, i + 1);
        }

        std::cout << std::format("Done {}!\n", name);
        
        std::cout << std::format("# {} released the semaphore\n", name);
    }
    sema.release();
}

void Run()
{
    std::vector<std::jthread> jthreads;

    for (int i = 0; i < 8; i++)
    {
        std::string name = "A";
        name[0] += i;

        jthreads.emplace_back(Worker, name, std::random_device{ }() % 4 + 2);
    }

    for (std::jthread& th : jthreads)
    {
        th.join();
    }

    std::cout << "All tasks have finished!\n";
}

END_NS

int main()
{
    // Case01::Run();
    Case02::Run();

    return 0;
}
