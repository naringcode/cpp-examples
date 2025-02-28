// Update Date : 2025-02-27
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
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
// 8. return_type_strong_ordering.cpp <-----
// 9. return_type_weak_ordering.cpp
// 10. return_type_partial_ordering.cpp
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

// https://en.cppreference.com/w/cpp/utility/compare/strong_ordering
// https://en.wikipedia.org/wiki/Total_order

// std::strong_ordering에 대한 수학적인 개념은 "c++20_comparison_operators_intro.txt"의 "@ std::strong_ordering" 쪽을 참고하도록 한다.

#define BEGIN_NS(name) namespace name {
#define END_NS }

BEGIN_NS(Case01)

// C++의 기본 타입 중 정수형과 string 계열은 std::strong_ordering 방식으로 동작한다.
// std::strong_ordering으로 동작하는 또 다른 타입으로는 bool, 포인터, enum이 있다.

void Run()
{
    std::string_view str1 = "Hello";
    std::string_view str2 = "World";

    int num1 = 10;
    int num2 = 10;

    std::cout << "ordering : " << typeid(str1 <=> str2).name() << '\n'; // struct std::strong_ordering
    std::cout << "ordering : " << typeid(num1 <=> num2).name() << '\n'; // struct std::strong_ordering

    std::cout << '\n';

    std::cout << "num1 < num2  : " << (num1 < num2)  << '\n'; // 1
    std::cout << "num1 > num2  : " << (num1 > num2)  << '\n'; // 0
    std::cout << "num1 <= num2 : " << (num1 <= num2) << '\n'; // 1
    std::cout << "num1 >= num2 : " << (num1 >= num2) << '\n'; // 0
    std::cout << "num1 == num2 : " << (num1 == num2) << '\n'; // 0
    std::cout << "num1 != num2 : " << (num1 != num2) << '\n'; // 1

    std::cout << '\n';

    auto ordering = num1 <=> num2;

    // std::strong_ordering은 다음 4가지 타입 중 하나와 비교할 수 있다.
    if (ordering == std::strong_ordering::less)
    {
        std::cout << "less : num1 < num2\n";
    }

    if (ordering == std::strong_ordering::greater)
    {
        std::cout << "greater : num1 > num2\n";
    }

    if (ordering == std::strong_ordering::equal)
    {
        std::cout << "equal : num1 == num2\n"; // <-----
    }

    if (ordering == std::strong_ordering::equivalent)
    {
        std::cout << "equivalent : num1 == num2\n"; // <-----
    }

    std::cout << '\n';

    // ordering 타입은 리터럴 0과 비교할 수 있다.
    if (ordering < 0)
    {
        std::cout << "ordering < 0 : num1 < num2\n";
    }

    if (ordering > 0)
    {
        std::cout << "ordering > 0 : num1 > num2\n";
    }

    if (ordering == 0)
    {
        std::cout << "ordering == 0 : num1 == num2\n"; // <-----
    }
}

END_NS

BEGIN_NS(Case02)

// std::strong_ordering은 본래 리터럴 0 값과 비교하는 것을 원칙으로 한다.
// 하지만 구현된 코드를 보면 컴파일 타임에 0으로 평가될 수 있는 값이라면 받아서 비교하는 것을 허용한다.
// 
// struct strong_ordering 
// {
//     ...
//     friend constexpr bool operator<(const strong_ordering _Val, _Literal_zero) noexcept {
//             return _Val._Value < 0;
//     }
//     ...
// };
// 
// struct _Literal_zero 
// {
//     template <class _Ty>
//         requires is_same_v<_Ty, int>
//     consteval _Literal_zero(_Ty _Zero) noexcept
//     {
//         // Can't use _STL_VERIFY because this is a core header
//         if (_Zero != 0) {
//             _Literal_zero_is_expected();
//         }
//     }
// };

void Run()
{
    int num1 = 10;
    int num2 = 20;

    auto ordering = num1 <=> num2;

    // int num = 0; // num은 컴파일 타임이 아닌 런타임에 평가됨.
    // 
    // if (ordering < num) // ERROR
    // {
    //     std::cout << "num1 < num2\n";
    // }

    // 아래 코드는 대상 변수가 컴파일 타임에 0으로 평가될 수 있기 때문에 비교하는 것을 허용한다.
    const     int zeroVal1 = 0;
    constexpr int zeroVal2 = 0;

    if (ordering < zeroVal1)
    {
        std::cout << "num1 < num2\n";
    }

    if (ordering < zeroVal2)
    {
        std::cout << "num1 < num2\n";
    }
}

END_NS

BEGIN_NS(Case03)

// 클래스나 구조체라고 해도 멤버 변수들이 전부 비교했을 때 std::strong_ordering을 만족한다면
// default가 적용된 operator<=>는 std::strong_ordering을 반환한다.
//
// (주의) std::strong_ordering으로 비교 불가능한 대상이 멤버 변수로 섞여 있다면 다른 ordering 타입을 반환한다.

struct FooObject
{
    int num{ 100 };
    std::string str{ "Hello" };

    auto operator<=>(const FooObject& rhs) const = default;
};

void Run()
{
    FooObject a;
    FooObject b;

    auto ordering = a <=> b;

    // std::strong ordering
    std::cout << typeid(ordering).name() << '\n';
}

END_NS

BEGIN_NS(Case04)

// 경우에 따라선 반환 타입을 std::strong_ordering으로 명시해야 하는 경우도 생긴다.
// 또한 이런 방식을 쓰는 게 가시성이 더 좋다.

struct FooObject
{
    int num{ 100 };

    // 지금은 int 값만 비교하지만 객체 구성이 복잡해지는 경우도 생각해야 한다.
    // default 방식이 아닌 경우 관계 연산(>, >=, <, <=)만 받을 수 있기 때문에 operator==를 직접 정의해야 한다.
    // std::strong_ordering operator<=>(const FooObject& rhs) const
    // {
    //     return num <=> rhs.num;
    // }
    
    std::strong_ordering operator<=>(const FooObject& rhs) const = default;
};

void Run()
{
    FooObject a;
    FooObject b;

    auto ordering = a <=> b;

    // std::strong ordering
    std::cout << typeid(ordering).name() << '\n';
}

END_NS

int main()
{
    // Case01::Run();
    // Case02::Run();
    // Case03::Run();
    Case04::Run();

    return 0;
}
