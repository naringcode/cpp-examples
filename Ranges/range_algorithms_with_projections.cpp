// Update Date : 2025-02-12
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <random>
#include <numbers>
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
// 7. range_algorithms_with_projections.cpp <-----
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
// # 응용하기
// 16. Functional/bind_and_mem_fn.cpp(Case03)
// 17. Functional/bind_front_and_bind_back.cpp(Case03)
// 18. STL/span.cpp(Case04)
//

// https://en.cppreference.com/w/cpp/numeric/constants

#define BEGIN_NS(name) namespace name {
#define END_NS };

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

BEGIN_NS(Case01)

// Projection을 통해 컨테이너 정렬하기.
// 정렬은 기본적으로 operator<()를 기준으로 진행된다.

void Run()
{
    std::vector<Point> data;

    for (int i = 0; i < 5; i++)
    {
        data.emplace_back(std::random_device{ }() % 10, std::random_device{ }() % 10);
    }

    auto printer = [](auto elem) { std::cout << "  " << elem << '\n'; };

    // unsorted
    std::cout << "############### Unsorted points ###############\n";
    std::ranges::for_each(data, printer);
    std::cout << '\n';

    // sorting by x(less)
    std::cout << "############### Sorting by x(less) ###############\n";
    std::ranges::sort(data, std::less{ }, &Point::x); // 멤버 변수를 전달하는 방식
    std::ranges::for_each(data, printer);
    std::cout << '\n';

    // sorting by x(greater)
    std::cout << "############### Sorting by x(greater) ###############\n";
    std::ranges::sort(data, std::greater{ }, &Point::GetX); // 멤버 함수를 전달하는 방식(멤버 변수가 private 안에 있을 경우 사용하면 됨)
    std::ranges::for_each(data, printer);
    std::cout << '\n';

    // sorting by y(less)
    std::cout << "############### Sorting by y(less) ###############\n";
    std::ranges::sort(data, std::less{ }, &Point::y); // 멤버 변수를 전달하는 방식
    std::ranges::for_each(data, printer);
    std::cout << '\n';

    // sorting by y(greater)
    std::cout << "############### Sorting by y(greater) ###############\n";
    std::ranges::sort(data, std::greater{ }, &Point::GetY); // 멤버 함수를 전달하는 방식(멤버 변수가 private 안에 있을 경우 사용하면 됨)
    std::ranges::for_each(data, printer);
    std::cout << '\n';

    // sorting by length(less)
    std::cout << "############### Sorting by length(less) ###############\n";
    std::ranges::sort(data, std::less{ }, &Point::Length); // 멤버 함수를 전달하는 방식
    std::ranges::for_each(data, printer);
    std::cout << '\n';

    // sorting by length(greater)
    std::cout << "############### Sorting by length(greater) ###############\n";
    std::ranges::sort(data, std::greater{ }, 
                      [](const Point& p) // 람다식을 전달하는 방식
                      { 
                          return sqrt(pow(p.x, 2) + pow(p.y, 2)); 
                      });
    std::ranges::for_each(data, printer);
    std::cout << '\n';
}

END_NS

BEGIN_NS(Case02)

// Projection을 통해 최소 값과 최대 값 구하기.

void Run()
{
    std::vector<Point> data;

    for (int i = 0; i < 5; i++)
    {
        data.emplace_back(std::random_device{ }() % 10, std::random_device{ }() % 10);
    }

    auto printer = [](auto elem) { std::cout << "  " << elem << '\n'; };

    // unsorted
    std::cout << "############### Unsorted points ###############\n";
    std::ranges::for_each(data, printer);
    std::cout << '\n';

    // min length
    std::cout << "############### Min length ###############\n";
    auto minRet = std::ranges::min_element(data, std::less{ }, &Point::Length);

    printer(*minRet);

    // max length(정렬되었을 때의 컨테이너 상태를 기준으로 생각하기)
    std::cout << "############### Max length ###############\n";
    auto maxRet = std::ranges::max_element(data, std::less{ }, &Point::Length);

    printer(*maxRet);
}

END_NS

BEGIN_NS(Case03)

// Projection으로 가공한 값을 출력하기.

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
    std::ranges::for_each(data, printer);
    std::cout << '\n';

    // point to degree
    std::cout << "############### Point to degree ###############\n";
    std::ranges::for_each(data, printer, // 가공하여 반환한 값을 printer를 통해 출력하기
                          [](const Point& p) {
                              std::cout << "  " << p << "  ->";

                              return atan2(p.y, p.x) * (180.0 / std::numbers::pi);
                          });
    std::cout << '\n';

    // original
    std::cout << "############### Original ###############\n";
    std::ranges::for_each(data, printer); // 원본을 변형하지 않았기에 원본 대상은 그대로임.
    std::cout << '\n';
}

END_NS

int main()
{
    // Case01::Run();
    // Case02::Run();
    Case03::Run();

    return 0;
}
