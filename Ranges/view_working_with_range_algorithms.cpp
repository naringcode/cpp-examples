// Update Date : 2025-02-13
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20, C++23(experimental)
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <random>
#include <algorithm>
#include <ranges>
#include <vector>
#include <set>

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
// 13. lazy_evaluation_of_views.cpp
// 14. view_working_with_range_algorithms.cpp <-----
// 
// # Range Factories
// 15. range_factories.cpp
//
// # 응용하기
// 16. Functional/bind_and_mem_fn.cpp(Case03)
// 17. Functional/bind_front_and_bind_back.cpp(Case03)
// 18. STL/span.cpp(Case04)
//

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

// View 클래스는 그 자체로 begin()과 end()를 제공하는 Range이다.
// 따라서 Range-based for loop로 순회할 수 있으며 Ranges 알고리즘 함수의 인자로 사용하는 것도 가능하다.

int main()
{
    std::vector<int> data{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    std::vector<int> copies;

    // 짝수만 조회하는 view 생성
    auto evenView = data | std::views::filter([](int elem) { return elem % 2 == 0; });

    // Ranged-based for loop로 순회하며 출력
    std::cout << "Ranged-based for loop : ";
    for (int elem : evenView)
    {
        std::cout << elem << ' ';
    }

    std::cout << '\n';

    // Ranges 알고리즘을 사용하여 출력
    std::cout << "ranges::for_each() : ";
    std::ranges::for_each(evenView, [](int elem) { std::cout << elem << ' '; });
    std::cout << "\n\n";

    // Ranges 알고리즘을 사용하여 복사
    std::ranges::copy(evenView, std::back_inserter(copies));
    Print(copies, "Copies");
    std::cout << '\n';

    // View composition을 통해 짝수를 제곱한 다음 뒤집고 앞에서 3개의 요소만 가져오는 view 생성
    auto complexView = evenView | std::views::transform([](int elem) { return elem * elem; })
                                | std::views::reverse
                                | std::views::take(3);

    // 짝수를 제곱한 결과를 ranges::copy()를 통해 ostream_iterator로 출력
    std::cout << "ostream_iterator : ";
    std::ranges::copy(complexView, std::ostream_iterator<int>(std::cout, " "));
    std::cout << '\n';

    // view를 find_if()와 연계하여 홀수를 찾는 작업 진행
    auto iter = std::ranges::find_if(complexView, [](int elem) { return elem % 2 != 0; });

    // view도 range이기 때문에 end()를 호출하는 것이 가능하다.
    // 하지만 view에 따라서 Range의 끝을 파악하기 위해 범위 전체를 탐색하는 과정을 거치는 경우도 있으니 주의하자(상황에 따라 비효율적임).
    if (iter != complexView.end()) // end() 호출은 최적화 수준이나 view에 따라서 비효율적일 수 있음.
    {
        std::cout << "complexView has at least one odd number.\n\n";
    }
    else
    {
        std::cout << "all numbers in complexView are even.\n\n";
    }

    // C++23부터는 ranges::to()를 통해 view나 range가 반환하는 결과를 원하는 Range 컨테이너로 받을 수 있다.
    auto retVec = std::ranges::to<std::vector<int>>(complexView); // view -> range
    auto retSet = std::ranges::to<std::set<int>>(retVec); // range -> range

    Print(retVec, "view -> vector");
    Print(retSet, "vector -> set");

    return 0;
}
