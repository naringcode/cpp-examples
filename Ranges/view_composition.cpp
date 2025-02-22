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
// 12. view_composition.cpp <-----
// 13. lazy_evaluation_of_views.cpp
// 14. view_working_with_range_algorithms.cpp
// 
// # Range Factories
// 15. range_factories.cpp
//
// # 응용하기
// 16. Functional/bind_and_mem_fn.cpp(Case03)
// 17. Functional/bind_front_and_bind_back.cpp(Case03)
// 18. STL/span.cpp(Case04)
//

// https://en.cppreference.com/w/cpp/ranges#Example
// https://hackingcpp.com/cpp/std/range_views_intro.html
// https://en.wikipedia.org/wiki/Syntactic_sugar

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

// [ Range ] -> 홀수 필터링 -> 첫 5개 요소만 가져오기 -> 제곱하기 -> 역순으로 조회
//
// 위 로직을 기존 알고리즘 방식으로 수행하기.

void Run()
{
    std::vector<int> data(20);
    std::ranges::generate(data, []() { return std::random_device{ }() % 100; });

    Print(data, "data");

    // 홀수 필터링
    std::vector<int> filterOdd;

    std::copy_if(data.begin(), data.end(), std::back_inserter(filterOdd),
                 [](int elem) {
                     return elem % 2 != 0;
                 });

    Print(filterOdd, "Filter Odd");

    // 첫 5개 요소만 가져오기(필터링된 영역의 최대 범위를 벗어나지 않게 처리해야 함)
    std::vector<int> takeFive{ 
        filterOdd.begin(), 
        filterOdd.begin() + std::min((size_t)5, filterOdd.size())
    };

    Print(takeFive, "Take 5");

    // 각 요소 제곱하기
    std::vector<int> squared;

    std::transform(takeFive.begin(), takeFive.end(), std::back_inserter(squared),
                   [](int elem) {
                       return elem * elem;
                   });

    Print(squared, "Squared");

    // 역순으로 조회
    std::vector<int> reversed{ squared.rbegin(), squared.rend() };

    Print(reversed, "Reversed");

    std::cout << '\n';
}

END_NS

BEGIN_NS(Case02)

// [ Range ] -> 홀수 필터링 -> 첫 5개 요소만 가져오기 -> 제곱하기 -> 역순으로 조회
//
// 이것을 View 기반으로 적용하면 복사하거나 번거로운 작업 없이 원본의 레퍼런스 참조하는 형태로 데이터를 정제할 수 있다.
// (중요) View도 일종의 Range이기 때문에 스스로를 인자로 전달하여 작업을 연계하는 것이 가능하다.
//
// 이처럼 View끼리 서로 연계하는 것을 View Composition 혹은 Composing Views라고 한다.

void Run()
{
    std::vector<int> data(20);
    std::ranges::generate(data, []() { return std::random_device{ }() % 100; });

    Print(data, "data");
    
    // 홀수 필터링
    auto filterOdd = std::views::filter(data, [](int elem) { return elem % 2 != 0; });

    Print(filterOdd, "Filter Odd");

    // 첫 5개 요소만 가져오기
    auto takeFive = std::views::take(filterOdd, 5); // 최대 영역을 벗어나지 않게 내부적으로 처리됨.

    Print(takeFive, "Take 5");

    // 각 요소 제곱하기
    auto squared = std::views::transform(takeFive, [](int elem) { return elem * elem; });

    Print(squared, "Squared");

    // 역순으로 조회
    auto reversed = std::views::reverse(squared);

    Print(reversed, "Reversed");

    // 지역에 거쳐가는 View를 임시 객체로 두지 않고 어댑터가 반환하는 View를 묶어서 바로 연계하는 것도 가능하다.
    // View의 기능이 결합되는 방식을 함수처럼 표현하면 이런 느낌이다
    // Reverse(Square(TakeFive(FilterOdd(data))));
    //
    // 따라서 기능이 결합되는 특징을 고려해서 코드를 작성한다면 이런 식으로 되야 한다.
    auto composition =
        std::views::reverse(
            std::views::transform(
                std::views::take(
                    std::views::filter(
                        data,
                        [](int elem) { return elem % 2 != 0; }),
                    5),
                [](int elem) { return elem * elem; }));

    Print(composition, "Composition");

    std::cout << '\n';
}

END_NS

BEGIN_NS(Case03)

// [ Range ] -> 홀수 필터링 -> 첫 5개 요소만 가져오기 -> 제곱하기 -> 역순으로 조회
//
// 지역에 거쳐가는 임시 객체를 만드는 방법이나 뷰를 하나로 묶어서 연계하는 방법은 가독성이 떨어지고 경우에 따라서 이해하기 힘들다.
//
// View는 Range를 결합할 목적으로 사용 가능한 일종의 구문 설탕(syntactic sugar)을 제공한다.
// 바로 Pipe 연산자로 이걸 사용하면 임시 객체를 생성하지도 않고 사람이 읽기도 편한 코드를 작성할 수 있다.

// https://youtu.be/L_bomNazb8M?si=YojUUl-5cvwlw_ja
// https://www.cppstories.com/2024/pipe-operator/
//
// Pipe 연산자로 기능을 결합하여 함수형 프로그래밍 방식을 적용하는 건 꽤 일반적인 방식이다.
// !! Ranges 라이브러리는 이걸 활용하여 View의 기능을 결합하는 것임. !!

void Run()
{
    std::vector<int> data(20);
    std::ranges::generate(data, []() { return std::random_device{ }() % 100; });

    Print(data, "data");

    // Reverse(Square(TakeFive(FilterOdd(data))));
    // 함수가 이렇게 호출된다고 가정했을 때 기능이 적용되는 순서는 data -> FilterOdd -> TakeFive -> Square -> Reverse이다.
    // Pipe 연산자로 이러한 로직을 작성한다고 하면 똑같은 순서를 따르면 된다.
    //
    // View composition by pipe operator
    auto composedView = data | std::views::filter([](int elem) { return elem % 2 != 0; })
                             | std::views::take(5)
                             | std::views::transform([](int elem) { return elem * elem; })
                             | std::views::reverse; // reverse는 괄호를 붙이면 안 됨(함수가 아닌 객체임).

    Print(composedView, "Composed");

    // 어댑터로 전달되는 과정을 보면 어댑터에 data를 직접적으로 넣지 않고 있다.
    // 
    // 각 filter(), take(), transform()이 반환하는 것을 보면 _Range_closure(MSVC 기준)로 되어 있는데
    // 이건 View가 아니라 Range를 결합하기 위해 생성하는 객체이다.
    // 
    // 공식 문서를 확인해 보면 해당 유형은 "range adaptor closure"를 반환한다고 되어 있다.
    // View를 반환하는 것이 아니기 때문에 reverse는 그냥 그대로 전달해야 한다.
    //
    // composedView를 조회해보면 reverse_view로 나오는데 이건 Pipe 연산자에 의해서 그렇게 생성된 것이다.
    // Pipe 연산자는 왼쪽에서 오른쪽으로 결합되는데 따라서 마지막으로 작업한 std::views::reverse에 맞는 view가 반환된 것이다.

    std::cout << '\n';
}

END_NS

BEGIN_NS(Case04)

// View를 사용하는 기타 방법에 대한 예시(tuple-like)

void Run()
{
    std::map<std::string, int> students{
        { "James", 11 }, { "Mary", 12 }, { "David", 13 }, { "Susan", 14 }, { "Michael", 15 }
    };

    // 이름 출력
    std::cout << "Names : ";
    for (auto names : students | std::views::keys)
    {
        std::cout << names << ' ';
    }

    std::cout << '\n';

    // 나이 출력
    std::cout << "Ages  : ";
    std::ranges::copy(students | std::views::values,
                      std::ostream_iterator<int>(std::cout, ", "));

    std::cout << '\n';

    // 사전으로 볼 때 'K' 이전에 오는 이름만 출력
    auto nameView = students | std::views::elements<0>
                             | std::views::filter([](const std::string& name) { return name[0] < 'K'; });

    Print(nameView, "Filter Names");

    std::cout << '\n';
}

END_NS

int main()
{
    Case01::Run();
    Case02::Run();
    Case03::Run();
    Case04::Run();

    return 0;
}
