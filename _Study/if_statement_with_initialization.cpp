// Update Date : 2025-01-13
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <mutex>
#include <vector>
#include <set>

// if statement with initializer (C++17)
// https://en.cppreference.com/w/cpp/language/if
// https://learn.microsoft.com/en-us/cpp/cpp/if-else-statement-cpp?view=msvc-170
// https://www.tutorialspoint.com/cplusplus17-if-statement-with-initializer
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0305r0.html
// https://www.geeksforgeeks.org/c17-new-feature-else-switch-statements-initializers/
//
// if (init; statement)
// {
//     ...
// }
// else if (init; statement)
// {
//     ...
// }
// else
// {
//     ...
// }
//

// 만약 init 구문에 포인터가 온다면 statement를 생략하여 nullptr이 아니라면 코드를 수행할 수 있다.
//
// 네트워크 패킷 처리나 운영체제의 비동기 관련 로직을 작성할 때 괜찮을 것 같다.

// 기본 형태 1
void Run01()
{
    if (int x = 50; x > 100)
    {
        std::cout << "x > 100\n";
    }
    else if (int y = 50; y > 100)
    {
        std::cout << "y > 100\n";
    }
    else if (x > 0 && y > 0)
    {
        std::cout << "x > 0 && y > 0\n";
    }
    else
    {
        std::cout << "None\n";
    }
}

// 기본 형태 2
void Run02()
{
    int x;
    int y;

    if (std::cin >> x; x > 0)
    {
        std::cout << "x is positive.\n";
    }
    else if (std::cin >> y; y > 0)
    {
        std::cout << "y is positive.\n";
    }
    else
    {
        std::cout << "[" << x << ", " << y << "]\n";
    }
}

// 이터레이터 판단
void Run03()
{
    std::set<int> mySet;

    if (auto [iter, success] = mySet.insert(100); true == success)
    {
        std::cout << "100 넣기 성공!\n";
    }
    else
    {
        std::cout << "100 넣기 실패!\n";
    }

    if (auto [iter, success] = mySet.insert(100); true == success)
    {
        std::cout << "100 넣기 성공!\n";
    }
    else
    {
        std::cout << "100 넣기 실패!\n";
    }

    if (auto iter = mySet.find(100); mySet.end() != iter)
    {
        std::cout << "100 찾음.\n";
    }
    else
    {
        std::cout << "100 못 찾음.\n";
    }
}

// while에 초기화 구문 넣는 것은 불가능하다.
// 하지만 switch에 초기화 구문을 넣는 것은 가능하다.
void Run04()
{
    // 아래 코드는 안 된다.
    // int x;
    // 
    // while (std::cin >> x; x > 0)
    // {
    //     x--;
    // 
    //     std::cout << x << " ";
    // }
    // 
    // std::cout << '\n';

    auto input = []() { int in; std::cin >> in; return in; };

    int x;

    // 후행의 x는 생략해도 된다(그냥 이런 식으로도 쓸 수 있다는 것을 보여주기 위한 것).
    switch (x = input(); x)
    {
        case 1:
        {
            std::cout << "switch : 1\n";

            break;
        }

        case 2:
        {
            std::cout << "switch : 2\n";

            break;
        }

        case 3:
        {
            std::cout << "switch : 3\n";

            break;
        }

        default:
        {
            std::cout << "switch : default\n";

            break;
        }
    }
}

// 스레드에선 이렇게 사용
void Run05()
{
    std::vector<int> vec;
    std::mutex mtx;

    if (std::lock_guard<std::mutex> lock(mtx); 0 == vec.size())
    {
        std::cout << "vector에 100 넣음.\n";

        vec.push_back(100);
    }
    else
    {
        std::cout << "vector top : " << vec.back() << '\n';
    }

    std::cout << "lock_guard의 소멸자는 호출되었는가? : ";

    if (true == mtx.try_lock())
    {
        std::cout << "Yes!\n";

        mtx.unlock();
    }
    else
    {
        std::cout << "No!\n";
    }
}

// Lock을 잡는 방법 그 두 번째
void Run06()
{
    std::mutex mtx;

    if (std::lock_guard<std::mutex> lock(mtx); true)
    {
        std::cout << "이 방식보다 좋은 듯?\n";
        std::cout << "{\n    lock_guard<mutex> lock(mtx);\n}\n";
    }
}

// 포인터 확인을 간략화하는 방법
// 굳이 nullptr을 확인하지 않아도 된다.
// 포인터만 가능하며 일반 변수는 안 된다.
int* ReturnNull()
{
    return nullptr;
}

int* ReturnAllocMem(int val)
{
    return new int(val);
}

void Run07()
{
    if (int* ptr = ReturnNull())
    {
        delete ptr;

        std::cout << "ReturnNull()\n";
    }
    
    if (int* ptr = ReturnAllocMem(10))
    {
        delete ptr;
        
        std::cout << "ReturnAllocMem()\n";
    }
}

int main()
{
    Run01();
    
    std::cout << "--------------------------------------------------\n";

    Run02();

    std::cout << "--------------------------------------------------\n";

    Run03();

    std::cout << "--------------------------------------------------\n";

    Run04();

    std::cout << "--------------------------------------------------\n";

    Run05();

    std::cout << "--------------------------------------------------\n";

    Run06();

    std::cout << "--------------------------------------------------\n";

    Run07();

    return 0;
}
