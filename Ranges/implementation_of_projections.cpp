// Update Date : 2025-02-12
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <random>
#include <concepts>
#include <algorithm>
#include <ranges>
#include <vector>

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
// 8. implementation_of_projections.cpp <-----
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
// # 응용하기
// 16. Functional/bind_and_mem_fn.cpp(Case03)
// 17. Functional/bind_front_and_bind_back.cpp(Case03)
// 18. STL/span.cpp(Case04)
//

#define BEGIN_NS(name) namespace name {
#define END_NS }

struct Point
{
    Point() = default;
    Point(double x, double y)
        : x{ x }, y{ y }
    { }

    friend std::ostream& operator<<(std::ostream& os, const Point& p)
    {
        os << "Point[ x : " << p.x << ", y : " << p.y << ", length : " << p.Length() << " ]";

        return os;
    }

    double Length() const
    {
        // distance from the origin point
        return sqrt(pow(x, 2) + pow(y, 2));
    }

    double GetX() const
    {
        return x;
    }

    double GetY() const
    {
        return y;
    }

    double x{ };
    double y{ };
};

// Projection의 기본 값으로 사용할 구조체
struct Identity
{
    // 반환 타입을 반드시 명시해야 하며 단순 auto로 지정하면 안 된다.
    // auto로 지정하면 반환한 값을 값 타입으로 추론한다.
    // 반환 타입을 원본 그대로(값이면 값, 참조면 참조) 반환하고자 한다면 T&&, auto&&, decltype(auto)처럼 반환형을 확실하게 명시해야 한다.
    template <typename T>
    constexpr T&& operator()(T&& elem) const noexcept
    {
        return std::forward<T>(elem);
    }

    // 반환형을 auto로 지정하면 Case02의 결과가 제대로 나오지 않을 것이다.
    // T&&, auto&&, decltype(auto)는 결과를 제대로 출력한다.
    // 
    // (중요) auto로 인한 추론은 참조를 보장하지 않는다(값으로 처리함).
};

template <typename Range, typename Functor, typename Projection = Identity>
void ForEach(Range&& range, Functor functor, Projection proj = { })
{
    auto beginIter = range.begin();
    auto endIter   = range.end();

    while (beginIter != endIter)    
    {
        // 특정 연산을 수행하기 전에 컨테이너의 원소를 변환한다.
        auto&& elem = std::invoke(proj, *beginIter++);

        functor(elem);
    }
}

BEGIN_NS(Case01)

// ForEach() 사용 예시(std::ranges::for_each()와 사용 방법이 똑같음)

void Run()
{
    std::vector<Point> data;

    for (int i = 0; i < 5; i++)
    {
        data.emplace_back(std::random_device{ }() % 10, std::random_device{ }() % 10);
    }

    auto printer = [](auto elem) { std::cout << "  " << elem << '\n'; };

    // sorting by length(less)
    std::cout << "############### Sorting by length(less) ###############\n";
    std::ranges::sort(data, std::less{ }, &Point::Length); // 멤버 함수를 전달하는 방식
    ForEach(data, printer); // 기본 Projection을 사용하는 방식
    std::cout << '\n';

    // x 값만 출력
    std::cout << "############### Print x ###############\n";
    ForEach(data, printer, &Point::x); // 멤버 변수를 전달하는 방식
    std::cout << '\n';
    
    // y 값만 출력
    std::cout << "############### Print y ###############\n";
    ForEach(data, printer, &Point::GetY); // 멤버 함수를 전달하는 방식(멤버 변수가 private 안에 있을 경우 사용하면 됨)
    std::cout << '\n';

    // length 값만 출력
    std::cout << "############### Print length ###############\n";
    ForEach(data, printer, [](const Point& p) { return sqrt(pow(p.x, 2) + pow(p.y, 2)); }); // 람다식을 전달하는 방식
    std::cout << '\n';
}

END_NS

BEGIN_NS(Case02)

// ForEach() 안에 ForEach()를 호출하는 형태

void Run()
{
    std::vector<std::string> names{
        "James", "Mary", "David", "Susan", "Michael"
    };

    auto printer = [](auto elem) { std::cout << "  " << elem << '\n'; };

    // 이름 기본 출력
    std::cout << "############### Names ###############\n";
    ForEach(names, printer); // 기본 Projection을 사용하는 방식
    std::cout << '\n';

    // 대문자 출력
    auto upper = [](const std::string& name) {
        std::string temp = name;
        
        // ForEach()에 의해 순회되고 있는데 내부에서 추가적으로 ForEach() 호출
        ForEach(temp, [](char& ch) { ch = std::toupper(ch); });

        return temp;
    };

    std::cout << "############### Upper ###############\n";
    ForEach(names, printer, upper); // Projection을 통해 가공한 값을 전달하는 방식
    std::cout << '\n';

    // 소문자 출력
    auto lower = [](const std::string& name) {
        std::string temp = name;
        
        // ForEach()에 의해 순회되고 있는데 내부에서 추가적으로 ForEach() 호출
        ForEach(temp, [](char& ch) { ch = std::tolower(ch); });

        return temp;
    };

    std::cout << "############### Lower ###############\n";
    ForEach(names, printer, lower); // Projection을 통해 가공한 값을 전달하는 방식 
    std::cout << '\n';
}

END_NS

int main()
{
    Case01::Run();
    Case02::Run();

    return 0;
}
