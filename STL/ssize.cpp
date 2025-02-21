// Update Date : 2025-02-21
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <ranges>
#include <vector>

// https://en.cppreference.com/w/cpp/iterator/size

// 컨테이너의 size()는 unsigned 값을 반환한다.
// 대부분의 개발자는 코드를 작성할 때 signed를 기준으로 작성하지 unsigned를 기준으로 작성하는 경우는 드물다.
// 이로 인해 타입 불일치에 따른 경고가 발생하기도 하고 언더플로우에 따른 기능에 이상이 발생하기 도한다.
//
// 이러한 단점을 보완하기 위해 C++20부터는 signed 값을 반환하는 std::ssize()를 제공하고 있다.
// 여타 다른 컨테이너 편의 함수와 마찬가지로 std::ssize()는 원시 배열을 대상으로도 적용할 수 있다.

template <typename Container>
void PrintExceptLastOne(Container&& container, std::string msg = "")
{
    // std::cout << msg << " : [ ";
    // 
    // // emptyVec를 인자로 받으면 언더플로우 발생
    // for (int idx = 0; idx < container.size() - 1; idx++)
    // {
    //     std::cout << container[idx] << ' ';
    // }
    // 
    // std::cout << "]\n";

    std::cout << msg << " : [ ";
    
    // 이렇게 하면 언더플로우가 발생하지 않는다.
    // 다른 컨테이너 편의 함수와 마찬가지로 원시 배열을 받는 것도 가능하다.
    for (int idx = 0; idx < std::ssize(container) - 1; idx++)
    {
        std::cout << container[idx] << ' ';
    }
    
    std::cout << "]\n";
}

int main()
{
    std::vector<int> vec{ 1, 2, 3, 4, 5 };
    std::vector<int> emptyVec{ };

    PrintExceptLastOne(vec, "vec");
    PrintExceptLastOne(emptyVec, "emptyVec");

    int primitiveArr[]{ 10, 20, 30, 40, 50 };

    PrintExceptLastOne(primitiveArr, "primitive");

    return 0;
}
