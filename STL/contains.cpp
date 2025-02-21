// Update Date : 2025-02-21
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20, C++23
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <algorithm>
#include <ranges>
#include <vector>
#include <list>
#include <map>
#include <set>

// https://en.cppreference.com/w/cpp/container/set/contains
// https://en.cppreference.com/w/cpp/container/multiset/contains
// https://en.cppreference.com/w/cpp/container/unordered_set/contains
// https://en.cppreference.com/w/cpp/container/unordered_multiset/contains
// https://en.cppreference.com/w/cpp/container/map/contains
// https://en.cppreference.com/w/cpp/container/multimap/contains
// https://en.cppreference.com/w/cpp/container/unordered_map/contains
// https://en.cppreference.com/w/cpp/container/unordered_multimap/contains
// https://en.cppreference.com/w/cpp/algorithm/ranges/contains

// C++20부터 set과 map 계열의 함수는 contains()를 지원한다.
// 이를 통해 find() -> iter != end()의 과정 없이 해당 키에 해당하는 요소가 있는지 판별할 수 있다.
//
// C++23부터는 Ranges 라이브러리 차원에서 std::ranges::contains()를 지원한다.
// 해당 함수는 set과 map 계열의 함수 외 다른 컨테이너도 받을 수 있다.
// 다른 컨테이너 편의 함수와 마찬가지로 원시 배열을 대상으로 Ranges의 contains()를 사용하는 것도 가능하다.

#define BEGIN_NS(name) namespace name {
#define END_NS }

BEGIN_NS(Case01)

// C++20 이전 스타일로 특정 요소가 있는지 확인하기

void Run()
{
    std::set<int> set{ 100, 200, 300, 400, 500 };

    std::map<int, std::string> map{
        { 100, "James" }, { 200, "Mary" }, { 300, "David" }, { 400, "Susan" }, { 500, "Michael" }
    };

    // set 조회
    if (auto iter = set.find(50); iter != set.end())
    {
        std::cout << "Found!\n";
    }
    else
    {
        std::cout << "Not found!\n";
    }

    // map 조회
    if (auto iter = map.find(200); iter != map.end())
    {
        std::cout << "Found!\n";
    }
    else
    {
        std::cout << "Not found!\n";
    }
}

END_NS

BEGIN_NS(Case02)

// C++20 이후 스타일로 특정 요소가 있는지 확인하기

void Run()
{
    std::set<int> set{ 100, 200, 300, 400, 500 };

    std::map<int, std::string> map{
        { 100, "James" }, { 200, "Mary" }, { 300, "David" }, { 400, "Susan" }, { 500, "Michael" }
    };

    // set 조회
    if (set.contains(50))
    {
        std::cout << "Found!\n";
    }
    else
    {
        std::cout << "Not found!\n";
    }

    // map 조회
    if (map.contains(200))
    {
        std::cout << "Found!\n";
    }
    else
    {
        std::cout << "Not found!\n";
    }
}

END_NS

BEGIN_NS(Case03)

// C++23 이후부터는 Ranges를 통해 다른 컨테이너를 대상으로 contains()를 호출하는 것이 가능하다.
// 이 함수는 원시 배열도 받는다.

void Run()
{
    std::vector<int> vec{ 10, 20, 30, 40, 50 };
    std::list<int> list{ 10, 20, 30, 40, 50 };

    int primitiveArr[]{ 10, 20, 30, 40, 50 };

    // set이나 map 계열 외의 컨테이너는 contains()를 멤버 함수로 가지지 않는다.
    // vec.contains(10);
    // list.contains(20);

    // 다음과 같이 멤버 함수 find()도 지원하지 않는다.
    // vec.find(10);
    // list.find(20);

    // 전역 함수 find()를 지원하긴 하지만 이 경우 iterator를 통한 후속 체크를 진행해야 한다.
    auto iterA = std::ranges::find(vec, 10);
    auto iterB = std::ranges::find(list, 20);
    auto iterC = std::ranges::find(primitiveArr, 30);

    if (iterA != vec.end())
    {
        std::cout << "found from vec : " << *iterA << '\n';
    }

    if (iterB != list.end())
    {
        std::cout << "found from list : " << *iterB << '\n';
    }

    if (iterC != std::end(primitiveArr))
    {
        std::cout << "found from arr : " << *iterC << '\n';
    }

    // 하지만 C++23의 Ranges가 제공하는 contains()을 사용하면 요소가 있는지 확인할 수 있다.

    // set 조회
    if (std::ranges::contains(vec, 10))
    {
        std::cout << "Found!\n";
    }
    else
    {
        std::cout << "Not found!\n";
    }

    // map 조회
    if (std::ranges::contains(vec, 20))
    {
        std::cout << "Found!\n";
    }
    else
    {
        std::cout << "Not found!\n";
    }

    // 원시 배열 조회
    if (std::ranges::contains(primitiveArr, 30))
    {
        std::cout << "Found!\n";
    }
    else
    {
        std::cout << "Not found!\n";
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
