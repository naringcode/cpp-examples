// Update Date : 2025-02-27
// OS : Windows 10 64bit
// Program : Visual Studio 2022, vscode(gcc-14.2.0)
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <cmath>
#include <iostream>
#include <numeric>
#include <compare>

// 단계별 학습을 위해서라도 다음 순서대로 보도록 하자.
// 
// # C++20 비교 연산자 개요(Three-way 비교 연산자를 위주로 다룸)
// 1. c++20_comparison_operators_intro.txt
// 
// # C++20 이전의 비교 연산
// 2. user_defined_comparison_operators_before_c++20.cpp
// 
// # C++20 이후의 비교 연산
// 3. three-way_comparison_operators_on_standard_types.cpp
// 4. default_three-way_comparison_operators.cpp
// 5. equality_operators_after_c++20.cpp
// 6. lexicographical_equality_comparisons.cpp
// 7. precautions_on_three-way_comparison_operators.cpp
// 
// # operator<=> 반환 타입
// 8. return_type_strong_ordering.cpp
// 9. return_type_weak_ordering.cpp
// 10. return_type_partial_ordering.cpp <-----
// 11. changing_ordering_category.cpp
// 
// # default로 정의한 Three-way 비교 연산자를 기반으로 동작하는 객체의 부모 클래스와 멤버 객체 간 관계 연산
// 12. default_three-way_rel_ops_on_inheritance_and_member_objects.cpp
// 
// # Furthermore
// 13. sorting_by_user_defined_three-way_comparison_operators.cpp
// 14. compare_fallback_funcs_to_synthesize_three-way_comp.cpp
// 15. lexicographical_compare_three_way_on_static_array.cpp
// 
// # 응용하기
// 16. implementation_of_case_insensitive_string.cpp
//

// https://en.cppreference.com/w/cpp/utility/compare/partial_ordering
// https://en.wikipedia.org/wiki/Partially_ordered_set

// std::partial_ordering에 대한 수학적인 개념은 "c++20_comparison_operators_intro.txt"의 "@ std::partial_ordering" 쪽을 참고하도록 한다.

#define BEGIN_NS(name) namespace name {
#define END_NS }

BEGIN_NS(Case01)

// https://en.wikipedia.org/wiki/NaN

// C++의 기본 타입 중 실수형은 NaN 값의 존재로 인해 std::partial_ordering 방식으로 동작한다.

void Run()
{
    constexpr double nan1{ std::numeric_limits<double>::quiet_NaN() };
    constexpr double nan2{ std::numeric_limits<double>::quiet_NaN() };

    double x{ 10.0f };
    double y{ 20.0f };

    std::cout << "ordering : " << typeid(x <=> y).name() << '\n'; // struct std::partial_ordering

    std::cout << "-------------------------\n";

    std::cout << "x < y  : " << (x < y)  << '\n'; // 1
    std::cout << "x > y  : " << (x > y)  << '\n'; // 0
    std::cout << "x <= y : " << (x <= y) << '\n'; // 1
    std::cout << "x >= y : " << (x >= y) << '\n'; // 0
    std::cout << "x == y : " << (x == y) << '\n'; // 0
    std::cout << "x != y : " << (x != y) << '\n'; // 1

    std::cout << '\n';

    auto ordering = x <=> y;

    // std::partial_ordering은 다음 4가지 타입 중 하나와 비교할 수 있다.
    if (ordering == std::partial_ordering::less)
    {
        std::cout << "less : x < y\n"; // <-----
    }

    if (ordering == std::partial_ordering::greater)
    {
        std::cout << "greater : x > y\n";
    }

    if (ordering == std::partial_ordering::equivalent)
    {
        std::cout << "equivalent : x == y\n";
    }

    if (ordering == std::partial_ordering::unordered)
    {
        std::cout << "unordered : x, y\n";
    }

    // ordering 타입은 리터럴 0과 비교할 수 있다.
    if (ordering < 0)
    {
        std::cout << "ordering < 0 : x < y\n"; // <-----
    }

    if (ordering > 0)
    {
        std::cout << "ordering > 0 : x > y\n";
    }

    if (ordering == 0)
    {
        std::cout << "ordering == 0 : x == y\n";
    }

    if (ordering != 0)
    {
        std::cout << "ordering != 0 : x != y\n"; // <-----
    }

    std::cout << "-------------------------\n";

    std::cout << "x < nan1  : " << (x < nan1)  << '\n'; // 0
    std::cout << "x > nan1  : " << (x > nan1)  << '\n'; // 0
    std::cout << "x <= nan1 : " << (x <= nan1) << '\n'; // 0
    std::cout << "x >= nan1 : " << (x >= nan1) << '\n'; // 0
    std::cout << "x == nan1 : " << (x == nan1) << '\n'; // 0
    std::cout << "x != nan1 : " << (x != nan1) << '\n'; // 1

    std::cout << '\n';

    ordering = x <=> nan1;

    // std::weak_ordering은 다음 3가지 타입 중 하나와 비교할 수 있다.
    if (ordering == std::weak_ordering::less)
    {
        std::cout << "less : x < nan1\n";
    }

    if (ordering == std::weak_ordering::greater)
    {
        std::cout << "greater : x > nan1\n";
    }

    if (ordering == std::weak_ordering::equivalent)
    {
        std::cout << "equivalent : x == nan1\n";
    }

    if (ordering == std::partial_ordering::unordered)
    {
        std::cout << "unordered : x, nan1\n"; // <-----
    }

    // ordering 타입은 리터럴 0과 비교할 수 있다.
    if (ordering < 0)
    {
        std::cout << "ordering < 0 : x < nan1\n";
    }

    if (ordering > 0)
    {
        std::cout << "ordering > 0 : x > nan1\n";
    }

    if (ordering == 0)
    {
        std::cout << "ordering == 0 : x == nan1\n";
    }

    if (ordering != 0)
    {
        std::cout << "ordering != 0 : x != nan1\n"; // <-----
    }

    std::cout << "-------------------------\n";
    
    // IEEE 754에 나온 내용에 따르면 NaN은 자기 자신과의 Not equal(!=) 연산을 제외하고 전부 false를 반환해야 한다.
    // NaN != NaN : true (NaN은 자신과 다르다고 간주)
    // 
    // 그런데 MSVC 컴파일러의 경우 부동소수점 모델에 따라 결과가 다르게 나온다.
    // 이론을 따져봤을 때 부동소수점 연산은 IEEE 754를 따라야 하는 것 같은데 실제로는 완벽히 따르진 않는 듯 하다.
    std::cout << "nan1 < nan2  : " << (nan1 < nan2)  << '\n'; // gcc : 0, fp:precise : 1, fp:strict : 0, fp:fast : 1
    std::cout << "nan1 > nan2  : " << (nan1 > nan2)  << '\n'; // gcc : 0, fp:precise : 0, fp:strict : 0, fp:fast : 0
    std::cout << "nan1 <= nan2 : " << (nan1 <= nan2) << '\n'; // gcc : 0, fp:precise : 1, fp:strict : 0, fp:fast : 1
    std::cout << "nan1 >= nan2 : " << (nan1 >= nan2) << '\n'; // gcc : 0, fp:precise : 0, fp:strict : 0, fp:fast : 0
    std::cout << "nan1 == nan2 : " << (nan1 == nan2) << '\n'; // gcc : 0, fp:precise : 0, fp:strict : 0, fp:fast : 0
    std::cout << "nan1 != nan2 : " << (nan1 != nan2) << '\n'; // gcc : 1, fp:precise : 1, fp:strict : 1, fp:fast : 1

    std::cout << '\n';

    ordering = nan1 <=> nan2;

    // std::weak_ordering은 다음 3가지 타입 중 하나와 비교할 수 있다.
    if (ordering == std::weak_ordering::less)
    {
        std::cout << "less : nan1 < nan2\n";
    }

    if (ordering == std::weak_ordering::greater)
    {
        std::cout << "greater : nan1 > nan2\n";
    }

    if (ordering == std::weak_ordering::equivalent)
    {
        std::cout << "equivalent : nan1 == nan2\n";
    }

    if (ordering == std::partial_ordering::unordered)
    {
        std::cout << "unordered : nan1, nan2\n"; // <-----
    }

    // ordering 타입은 리터럴 0과 비교할 수 있다.
    if (ordering < 0)
    {
        std::cout << "ordering < 0 : nan1 < nan2\n";
    }

    if (ordering > 0)
    {
        std::cout << "ordering > 0 : nan1 > nan2\n";
    }

    if (ordering == 0)
    {
        std::cout << "ordering == 0 : nan1 == nan2\n";
    }

    if (ordering != 0)
    {
        std::cout << "ordering != 0 : nan1 != nan2\n"; // <----- NaN이 섞인 비교는 전부 false를 반환함.
    }

    std::cout << "-------------------------\n";
}

END_NS

BEGIN_NS(Case02)

// operator<=>를 default로 하고 반환 타입을 auto로 정의했을 때
// 객체의 멤버 변수에 실수형이 섞여 있다면 반환 타입은 std::partial_ordering으로 추론된다.

struct FooObject
{
    int num{ 100 };
    std::string str{ "Hello" };
    float fp{ 3.14f };

    auto operator<=>(const FooObject& rhs) const = default;
};

void Run()
{
    FooObject a;
    FooObject b;

    auto ordering = a <=> b;

    // std::partial_ordering
    std::cout << typeid(ordering).name() << '\n';
}

END_NS

BEGIN_NS(Case03)

// 멤버 변수의 대소 관계를 모두 만족할 경우에만 less, greater, equal을 출력하고 싶다고 해보자.
// 하지만 두 피연산자를 비교할 때 멤버 변수 간의 대소 관계가 다른 구간이 생길 것이다.

struct Pair
{
    int first;
    int second;

    // Member-wise comparison(lexicographically)
    auto operator<=>(const Pair& rhs) const = default;
};

void Run()
{
    // Pair p1{ 1, 2 }; // less(p1.first < p2.first && p1.second < p2.second)
    // Pair p1{ 10, 6 }; // greater(p1.first > p2.first && p1.second > p2.second)
    // Pair p1{ 3, 4 }; // equal(p1.first == p2.first && p1.second == p2.second)
    Pair p1{ 1, 10 }; // less but strange(p1의 first는 p2의 first보다 작고, p1의 second는 p2의 second보다 큼)
    Pair p2{ 3, 4 };

    if (p1 < p2)
    {
        std::cout << "less\n";
    }

    if (p1 > p2)
    {
        std::cout << "greater\n";
    }

    if (p1 == p2)
    {
        std::cout << "equal\n";
    }
}

END_NS

BEGIN_NS(Case04)

// 둘의 대소 관계가 같지 않을 경우 비교할 수 없다는 힌트를 주면 좋을 것이다.
// 이럴 때 사용할 수 있는 것이 unordered이다.

struct Pair
{
    int first;
    int second;

    // Member-wise comparison(lexicographically)
    // auto operator<=>(const Pair& rhs) const = default;

    std::partial_ordering operator<=>(const Pair& rhs) const
    {
        if (first < rhs.first && second < rhs.second)
            return std::partial_ordering::less;

        if (first > rhs.first && second > rhs.second)
            return std::partial_ordering::greater;

        if (first == rhs.first && second == rhs.second)
            return std::partial_ordering::equivalent;

        return std::partial_ordering::unordered;
    }

    // (중요) operator<=>을 수동으로 만들면 operator==를 직접 정의해야 한다.
    bool operator==(const Pair& rhs) const = default;
};

void Run()
{
    // Pair p1{ 1, 2 }; // less(p1.first < p2.first && p1.second < p2.second)
    // Pair p1{ 10, 6 }; // greater(p1.first > p2.first && p1.second > p2.second)
    // Pair p1{ 3, 4 }; // equal(p1.first == p2.first && p1.second == p2.second)
    Pair p1{ 1, 10 }; // unordered(p1의 first는 p2의 first보다 작고, p1의 second는 p2의 second보다 큼)
    Pair p2{ 3, 4 };

    if (p1 < p2)
    {
        std::cout << "less\n";
    }
    else if (p1 > p2)
    {
        std::cout << "greater\n";
    }
    else if (p1 == p2)
    {
        std::cout << "equal\n";
    }
    else
    {
        std::cout << "unordered\n";
    }
}

END_NS

BEGIN_NS(Case05)

// 두 정점을 원점으로부터의 거리로 비교한다고 해보자.
// 이때 정점의 좌표를 (-100, -100)에서 (+100, +100)까지의 영역으로 한정한다고 가정하면
// 해당 영역을 벗어나는 경우 순서를 정의할 수 없다는 의미인 unordered를 반환해야 한다.

class BoundedPoint
{
public:
    BoundedPoint() = default;

    BoundedPoint(int xy)
        : _x{ xy }, _y{ xy }
    { }

    BoundedPoint(int x, int y)
        : _x{ x }, _y{ x }
    { }

public:
    std::partial_ordering operator<=>(const BoundedPoint& rhs) const
    {
        if (this->isBounded() == false || rhs.isBounded() == false)
            return std::partial_ordering::unordered; // 범위를 벗어나는 경우
        
        auto lhsLen = this->length();
        auto rhsLen = rhs.length();

        if (lhsLen < rhsLen)
        {
            return std::partial_ordering::less;
        }
        else if (lhsLen > rhsLen)
        {
            return std::partial_ordering::greater;
        }
        else // if (lhsLen == rhsLen)
        {
            return std::partial_ordering::equivalent;
        }
    }

    bool operator==(const BoundedPoint& rhs) const
    {
        return length() == rhs.length();
    }

private:
    bool isBounded() const
    {
        return (abs(_x) < 100) && (abs(_y) < 100);
    }

    double length() const
    {
        return sqrt((_x * _x) + (_y * _y));
    }

private:
    int _x{ };
    int _y{ };
};

void Run()
{
    BoundedPoint p1(110, 110);
    BoundedPoint p2(20, 20);
    
    std::cout << "p1 < p2  : " << (p1 < p2)  << '\n'; // 0
    std::cout << "p1 > p2  : " << (p1 > p2)  << '\n'; // 0
    std::cout << "p1 <= p2 : " << (p1 <= p2) << '\n'; // 0
    std::cout << "p1 >= p2 : " << (p1 >= p2) << '\n'; // 0
    std::cout << "p1 == p2 : " << (p1 == p2) << '\n'; // 0
    std::cout << "p1 != p2 : " << (p1 != p2) << '\n'; // 1
}

END_NS

int main()
{
    // Case01::Run();
    Case02::Run();
    // Case03::Run();
    // Case04::Run();
    // Case05::Run();

    return 0;
}
