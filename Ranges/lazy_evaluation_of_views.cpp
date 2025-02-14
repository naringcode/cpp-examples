// Update Date : 2025-02-14
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20, C++23
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <random>
#include <algorithm>
#include <ranges>
#include <vector>
#include <map>

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
// 4. legacy_algorithms_and_range_algorithms.cpp
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
// 13. lazy_evaluation_of_views.cpp <-----
// 14. view_working_with_range_algorithms.cpp
// 
// # Range Factories
// 15. range_factories.cpp
//

// https://en.wikipedia.org/wiki/Lazy_evaluation

#define BEGIN_NS(name) namespace name {
#define END_NS }

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

// [ Range ] -> 짝수 필터링 -> 제곱하기
//
// 위 로직을 View로 처리한다고 했을 때 어떻게 지연 평가가 이루어지는지 확인해 보자.

void Run()
{
    std::vector<int> data(10);
    std::ranges::generate(data, []() { return std::random_device{ }() % 100; });

    // (주의) Views가 lazy evaluation을 지원하는 것이지 Ranges 알고리즘 함수는 그 즉시 연산을 수행한다.
    std::ranges::sort(data);

    Print(data, "data");

    // 짝수 필터링
    auto even = [](int elem) {
            std::cout << "  check even : " << elem << '\n';

            return elem % 2 == 0;
        };

    auto filterEven = data | std::views::filter(even);

    std::cout << "########## Eval Check 1 ##########\n";

    // 제곱하기
    auto square = [](int elem) {
            std::cout << "    pass to square : " << elem << '\n';

            return elem * elem;
        };

    auto squaredView = filterEven | std::views::transform(square);

    std::cout << "########## Eval Check 2 ##########\n";

    // 지연 평가 확인하기
    // View가 생성될 때 평가가 이루어지는 것이 아닌 매번 data의 element에 접근할 때마다 평가가 이루어진다.
    // View는 생성되는 그 순간 프로그램의 흐름에 어떠한 영향도 끼치지 않는다.
    for (int elem : squaredView)
    {
        // elem으로 받아오는 건 접근 후 최종적으로 평가가 완료된 data의 개별 element이다. 
        // View를 쓰면 원본 data의 elements를 보관하는 그런 작업을 거치지 않아도 된다.
        std::cout << "      result : " << elem << '\n';
    }

    // 
}

END_NS

int main()
{
    Case01::Run();

    return 0;
}
