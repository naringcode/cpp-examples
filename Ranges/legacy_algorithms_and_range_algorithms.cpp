// Update Date : 2025-02-12
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <format>
#include <random>
#include <algorithm>
#include <ranges>
#include <execution> // C++17(std::execution::)
#include <string>
#include <vector>
#include <list>

// Concepts에 대한 내용을 충분히 숙지한 다음 Ranges를 보도록 한다.
// Ranges의 동작 방식을 잘 이해하기 위해선 Concepts에 대한 선행 학습이 필요하다.
// 
// 다음 순서대로 보도록 하자.
// 
// # Ranges 개요
// 1. ranges_intro.txt
// 
// # Ranges 알고리즘
// 2. Concepts/CustomContainerIterators(사전지식)
// 3. range_concepts_and_iterator_concepts.cpp
// 4. legacy_algorithms_and_range_algorithms.cpp <-----
// 5. return_types_by_range_algorithms.cpp
// 
// # Projections
// 6. implementation_of_invoke.cpp(사전지식)
// 7. range_algorithms_with_projections.cpp
// 8. implementation_of_projections.cpp
// 
// # Views
// 9. views_by_constructors.cpp
// 10. views_by_range_adaptors.cpp
// 11. algorithms_with_and_without_views.cpp
// 12. view_composition.cpp
// 13. lazy_evaluation_of_views.cpp
// 14. view_working_with_range_algorithms.cpp
// 
// # Range Factories
// 15. range_factories.cpp
//

// https://en.cppreference.com/w/cpp/algorithm
// https://en.cppreference.com/w/cpp/algorithm/ranges
// https://en.cppreference.com/w/cpp/algorithm/execution_policy_tag

#define BEGIN_NS(name) namespace name {
#define END_NS };

template <std::ranges::range Container>
void Print(Container&& container, std::string_view msg = "", char separator = ' ')
{
    if (msg == "")
    {
        std::cout << "[ ";
    }
    else
    {
        std::cout << std::format("{:<12} : [ ", msg);
    }

    for (const auto& elem : container)
    {
        std::cout << std::format("{}{}", elem, separator);
    }

    std::cout << "]\n";
}

BEGIN_NS(Case01)

// 알고리즘 함수를 사용할 때 대부분 컨테이너 전체를 대상으로 작업하는 경우가 많다.
// 특정 sub-range를 대상으로 작업하는 건 극히 희귀한 케이스이다.

void Run()
{
    // 둘 다 테스트해볼 것
    // std::list<int> data(10);
    std::vector<int> data(10);

    std::generate(/* std::execution::par, */ data.begin(), data.end(), [] { return std::random_device{ }() % 10; });
    Print(data, "Generated");
    
    // 순회하며 홀수를 찾아 2를 곱하기
    std::for_each(/* std::execution::par, */ data.begin(), data.end(), [](int& elem) {
        if (elem % 2 != 0)
        {
            elem *= 2;
        }
    });
    
    Print(data, "Doubled odds");

    // 정렬
    // std::list의 경우 random access를 지원하지 않기 때문에 사용 불가능하지만 컴파일을 해야 에러를 파악할 수 있다.
    // 또한 에러가 발생해도 제약 조건이 없기 때문에 어디서 호출해서 문제가 발생했는지에 대한 원인을 특정하기 힘들다.
    std::sort(/* std::execution::par, */ data.begin(), data.end());

    std::cout << "\nSorted  : ";
    Print(data);

    // all_of()
    if (auto found = std::all_of(/* std::execution::par, */ data.begin(), data.end(), 
                                 [](int elem) { return elem % 2 != 0; }))
    {
        std::cout << "\nAll elements in data are odd";
    }
    else
    {
        std::cout << "\nAt least one element in data is even";
    }

    // any_of()
    if (auto found = std::any_of(/* std::execution::par, */ data.begin(), data.end(), 
                                 [](int elem) { return elem % 2 != 0; }))
    {
        std::cout << "\nAt least one element in data is odd";
    }
    else
    {
        std::cout << "\nAll elements in data are even";
    }
    
    // 특정 단일 요소 찾기
    std::cout << "\n\nFind 8  : ";

    if (auto iter = std::find(/* std::execution::par, */ data.begin(), data.end(), 8); 
        iter != data.end())
    {
        std::cout << "Found\n";
    }
    else
    {
        std::cout << "Not found\n";
    }

    // 특정 단일 요소 카운팅
    std::cout << "Count 8 : ";

    if (auto cnt = std::count(/* std::execution::par, */ data.begin(), data.end(), 8); 
        cnt != 0)
    {
        std::cout << "Count - " << cnt << "\n";
    }
    else
    {
        std::cout << "No count\n";
    }

    // 순회하며 5와 10 사이에 있는 원소를 출력
    std::cout << "\nPrint if btw 5 & 10 : ";

    std::for_each(/* std::execution::par, */ data.begin(), data.end(), [](int elem) {
        if (elem >= 5 && elem <= 10)
        {
            std::cout << elem << ' ';
        }
    });

    // 대상의 기준이 복합적인 경우 find_if()나 count_if() 사용
    std::cout << "\nFind if Btw 5 & 10  : ";

    if (auto iter = std::find_if(/* std::execution::par, */ data.begin(), data.end(),
                                 [](int elem) { return elem >= 5 && elem <= 10; }); 
        iter != data.end())
    {
        std::cout << "Found and the first is - " << *iter << "\n";
    }
    else
    {
        std::cout << "Not found\n";
    }

    //
    std::cout << "Count if Btw 5 & 10 : ";

    if (auto cnt = std::count_if(/* std::execution::par, */ data.begin(), data.end(),
                                 [](int elem) { return elem >= 5 && elem <= 10; }); 
        cnt != 0)
    {
        std::cout << "Count - " << cnt << "\n";
    }
    else
    {
        std::cout << "No count\n";
    }
    
    // 첫 5개 원소 출력
    // data.begin() + 5는 random_access를 지원하는 컨테이너만 사용할 수 있기 때문에
    // 범용적으로 지원하고 싶다면 std::advance()를 사용해야 한다.
    std::cout << "\nFirst 5 elements : ";

    auto endIter = data.begin();
    std::advance(endIter, 5); // 반환하는 함수가 아닌 내부적으로 iterator를 증가시키는 함수임.

    std::for_each(/* std::execution::par, */ data.begin(), endIter, [](int elem) { std::cout << elem << ' '; });
}

END_NS

BEGIN_NS(Case02) 

// Case01을 보면 마지막 케이스 첫 5개 원소를 출력하는 쪽만 제외하고 전부 컨테이너 전체를 대상으로 작업하고 있다.
// C++20 이전 알고리즘 함수들은 직접 시작과 끝 반복자를 지정해야 했지만
// C++20에 도입된 Ranges 알고리즘 함수를 쓰면 컨테이너를 직접 인자로 넘길 수 있다.
//
// (중요) C++20 기준 Ranges 알고리즘은 병렬 실행 정책을 지원하지 않는다.
//
// C++20의 Ranges 알고리즘 함수는 이전에 사용되었던 iterator pair를 지정하는 방식도 지원한다.

void Run()
{
    // 둘 다 테스트해볼 것
    // std::list<int> data(10);
    std::vector<int> data(10);

    std::ranges::generate(data, [] { return std::random_device{ }() % 10; });
    Print(data, "Generated");
    
    // 순회하며 홀수를 찾아 2를 곱하기
    std::ranges::for_each(data, [](int& elem) {
        if (elem % 2 != 0)
        {
            elem *= 2;
        }
    });
    
    Print(data, "Doubled odds");

    // 정렬
    // std::list의 경우 random access를 지원하지 않기 때문에 사용 불가능하다.
    // 이전에는 제약 조건이 걸려있지 않아서 에러가 발생해도 어디서 호출해서 문제가 발생한 것인지 원인을 특정하기 힘들었다.
    // 하지만 Ranges 알고리즘 함수의 경우에는 제약 조건이 걸려있기 때문에 어떤 함수를 어디서 호출해서 문제가 발생했는지 파악하기 쉽다.
    std::ranges::sort(data);

    std::cout << "\nSorted  : ";
    Print(data);

    // all_of()
    if (auto found = std::ranges::all_of(data, [](int elem) { return elem % 2 != 0; }))
    {
        std::cout << "\nAll elements in data are odd";
    }
    else
    {
        std::cout << "\nAt least one element in data is even";
    }

    // any_of()
    if (auto found = std::ranges::any_of(data, [](int elem) { return elem % 2 != 0; }))
    {
        std::cout << "\nAt least one element in data is odd";
    }
    else
    {
        std::cout << "\nAll elements in data are even";
    }
    
    // 특정 단일 요소 찾기
    std::cout << "\n\nFind 8  : ";

    if (auto iter = std::ranges::find(data, 8); 
        iter != data.end())
    {
        std::cout << "Found\n";
    }
    else
    {
        std::cout << "Not found\n";
    }

    // 특정 단일 요소 카운팅
    std::cout << "Count 8 : ";

    if (auto cnt = std::ranges::count(data, 8); 
        cnt != 0)
    {
        std::cout << "Count - " << cnt << "\n";
    }
    else
    {
        std::cout << "No count\n";
    }

    // 순회하며 5와 10 사이에 있는 원소를 출력
    std::cout << "\nPrint if btw 5 & 10 : ";

    std::ranges::for_each(data, [](int elem) {
        if (elem >= 5 && elem <= 10)
        {
            std::cout << elem << ' ';
        }
    });

    // 대상의 기준이 복합적인 경우 find_if()나 count_if() 사용
    std::cout << "\nFind if Btw 5 & 10  : ";

    if (auto iter = std::ranges::find_if(data.begin(), data.end(),
                                         [](int elem) { return elem >= 5 && elem <= 10; }); 
        iter != data.end())
    {
        std::cout << "Found and the first is - " << *iter << "\n";
    }
    else
    {
        std::cout << "Not found\n";
    }

    //
    std::cout << "Count if Btw 5 & 10 : ";

    if (auto cnt = std::ranges::count_if(data.begin(), data.end(),
                                         [](int elem) { return elem >= 5 && elem <= 10; }); 
        cnt != 0)
    {
        std::cout << "Count - " << cnt << "\n";
    }
    else
    {
        std::cout << "No count\n";
    }
    
    // 첫 5개 원소 출력
    // data.begin() + 5는 random_access를 지원하는 컨테이너만 사용할 수 있기 때문에
    // 범용적으로 지원하고 싶다면 std::advance()를 사용해야 한다.
    std::cout << "\nFirst 5 elements : ";

    auto endIter = data.begin();
    std::advance(endIter, 5); // 반환하는 함수가 아닌 내부적으로 iterator를 증가시키는 함수임.

    // 기존 iterator pair를 지정하는 방식도 지원한다.
    // 기존 알고리즘 함수와는 달리 제약 조건이 적용되어 있어 손쉽게 에러 핸들링이 가능하다.
    std::ranges::for_each(data.begin(), endIter, [](int elem) { std::cout << elem << ' '; });
}

END_NS

int main()
{
    // Case01::Run();
    Case02::Run();

    return 0;
}
