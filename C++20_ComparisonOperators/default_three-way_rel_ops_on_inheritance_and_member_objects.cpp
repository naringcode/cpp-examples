// Update Date : 2025-02-28
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

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
// 10. return_type_partial_ordering.cpp
// 11. changing_ordering_category.cpp
// 
// # default로 정의한 Three-way 비교 연산자를 기반으로 동작하는 객체의 부모 클래스와 멤버 객체 간 관계 연산
// 12. default_three-way_rel_ops_on_inheritance_and_member_objects.cpp <-----
// 
// # Furthermore
// 13. sorting_by_user_defined_three-way_comparison_operators.cpp
// 14. compare_fallback_funcs_to_synthesize_three-way_comp.cpp
// 15. lexicographical_compare_three_way_on_static_array.cpp
// 
// # 응용하기
// 16. implementation_of_case_insensitive_string.cpp
//

// 해당 연산에 대한 자세한 설명은 "c++20_comparison_operators_intro.txt"를 참고하도록 한다.
// - # default로 정의한 Three-way 비교 연산자를 기반으로 동작하는 객체의 부모 클래스와 멤버 객체 간 관계 연산

#define BEGIN_NS(name) namespace name {
#define END_NS }

BEGIN_NS(Case01)

// 객체가 상속 관계에 있고 operator<=>를 default로 정의했을 경우 부모는 다음 두 조건 중 하나를 만족해야 한다.
// 1. operator<=>를 지원해야 함(수동으로 정의한 경우 operator==도 정의해주는 것이 좋음).
// 2. 사용자 정의 operator<와 operator==를 지원해야 함(operator==는 default로 정의해도 됨).

struct Base
{
    int iValue{ };

    auto operator<=>(const Base& rhs) const = default;
};

struct Derived : Base
{
    float fValue{ };

    auto operator<=>(const Derived& rhs) const = default;
};

void Run()
{
    Derived a{ 100, 3.14f };
    Derived b{ 100, 3.14f };
    Derived c{ 150, 1.57f };
    Derived d{ 50,  6.28f };
    Derived e{ 100, 6.28f };
    Derived f{ 100, std::numeric_limits<float>::quiet_NaN() };

    std::cout << "ordering : " << typeid(a <=> b).name() << '\n'; // struct std::partial_ordering
    std::cout << "ordering : " << typeid(a <=> c).name() << '\n'; // struct std::partial_ordering

    std::cout << "-------------------------\n";

    std::cout << "{ 100, 3.14f } @ { 100, 3.14f }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < b)  " << (a < b)  << '\n'; // 0
    std::cout << "(a > b)  " << (a > b)  << '\n'; // 0
    std::cout << "(a <= b) " << (a <= b) << '\n'; // 1
    std::cout << "(a >= b) " << (a >= b) << '\n'; // 1

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == b) " << (a == b) << '\n'; // 1
    std::cout << "(a != b) " << (a != b) << '\n'; // 0

    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 3.14f } @ { 150, 1.57f }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < c)  " << (a < c)  << '\n'; // 1
    std::cout << "(a > c)  " << (a > c)  << '\n'; // 0
    std::cout << "(a <= c) " << (a <= c) << '\n'; // 1
    std::cout << "(a >= c) " << (a >= c) << '\n'; // 0

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == c) " << (a == c) << '\n'; // 0
    std::cout << "(a != c) " << (a != c) << '\n'; // 1

    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 3.14f } @ { 50, 6.28f }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < d)  " << (a < d)  << '\n'; // 0
    std::cout << "(a > d)  " << (a > d)  << '\n'; // 1
    std::cout << "(a <= d) " << (a <= d) << '\n'; // 0
    std::cout << "(a >= d) " << (a >= d) << '\n'; // 1

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == d) " << (a == d) << '\n'; // 0
    std::cout << "(a != d) " << (a != d) << '\n'; // 1

    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 3.14f } @ { 100, 6.28f }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < e)  " << (a < e)  << '\n'; // 1
    std::cout << "(a > e)  " << (a > e)  << '\n'; // 0
    std::cout << "(a <= e) " << (a <= e) << '\n'; // 1
    std::cout << "(a >= e) " << (a >= e) << '\n'; // 0

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == e) " << (a == e) << '\n'; // 0
    std::cout << "(a != e) " << (a != e) << '\n'; // 1

    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 3.14f } @ { 100, NaN }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < f)  " << (a < f)  << '\n'; // 0
    std::cout << "(a > f)  " << (a > f)  << '\n'; // 0
    std::cout << "(a <= f) " << (a <= f) << '\n'; // 0
    std::cout << "(a >= f) " << (a >= f) << '\n'; // 0

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == f) " << (a == f) << '\n'; // 0
    std::cout << "(a != f) " << (a != f) << '\n'; // 1

    std::cout << "-------------------------\n";
}

END_NS

BEGIN_NS(Case02)

// 자식 쪽에서 사용자 정의 operator<=>를 구현해도 된다.
// 이 경우 완전한 비교 연산(<, >, <=, >=, ==, !=)을 지원하기 위해 operator==도 정의하는 것이 좋다.

struct Base
{
    int iValue{ };

    auto operator<=>(const Base& rhs) const
    {
        std::cout << "operator<=> ";

        return iValue <=> rhs.iValue;
    }

    bool operator==(const Base& rhs) const
    {
        std::cout << "operator== ";

        return iValue == rhs.iValue;
    }
};

struct Derived : Base
{
    float fValue{ };

    // 비교 도중 사용자 정의 연산자를 사용한다면 이쪽의 operator<=>는 반드시 적절한 반환 타입을 명시해야 한다.
    std::partial_ordering operator<=>(const Derived& rhs) const = default;
};

void Run()
{
    Derived a{ 100, 3.14f };
    Derived b{ 100, 3.14f };
    Derived c{ 150, 1.57f };
    Derived d{ 50,  6.28f };
    Derived e{ 100, 6.28f };
    Derived f{ 100, std::numeric_limits<float>::quiet_NaN() };

    std::cout << "ordering : " << typeid(a <=> b).name() << '\n'; // struct std::partial_ordering
    std::cout << "ordering : " << typeid(a <=> c).name() << '\n'; // struct std::partial_ordering

    std::cout << "-------------------------\n";

    std::cout << "{ 100, 3.14f } @ { 100, 3.14f }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < b)  " << (a < b)  << '\n'; // 0
    std::cout << "(a > b)  " << (a > b)  << '\n'; // 0
    std::cout << "(a <= b) " << (a <= b) << '\n'; // 1
    std::cout << "(a >= b) " << (a >= b) << '\n'; // 1

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == b) " << (a == b) << '\n'; // 1
    std::cout << "(a != b) " << (a != b) << '\n'; // 0

    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 3.14f } @ { 150, 1.57f }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < c)  " << (a < c)  << '\n'; // 1
    std::cout << "(a > c)  " << (a > c)  << '\n'; // 0
    std::cout << "(a <= c) " << (a <= c) << '\n'; // 1
    std::cout << "(a >= c) " << (a >= c) << '\n'; // 0

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == c) " << (a == c) << '\n'; // 0
    std::cout << "(a != c) " << (a != c) << '\n'; // 1

    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 3.14f } @ { 50, 6.28f }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < d)  " << (a < d)  << '\n'; // 0
    std::cout << "(a > d)  " << (a > d)  << '\n'; // 1
    std::cout << "(a <= d) " << (a <= d) << '\n'; // 0
    std::cout << "(a >= d) " << (a >= d) << '\n'; // 1

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == d) " << (a == d) << '\n'; // 0
    std::cout << "(a != d) " << (a != d) << '\n'; // 1

    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 3.14f } @ { 100, 6.28f }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < e)  " << (a < e)  << '\n'; // 1
    std::cout << "(a > e)  " << (a > e)  << '\n'; // 0
    std::cout << "(a <= e) " << (a <= e) << '\n'; // 1
    std::cout << "(a >= e) " << (a >= e) << '\n'; // 0

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == e) " << (a == e) << '\n'; // 0
    std::cout << "(a != e) " << (a != e) << '\n'; // 1

    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 3.14f } @ { 100, NaN }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < f)  " << (a < f)  << '\n'; // 0
    std::cout << "(a > f)  " << (a > f)  << '\n'; // 0
    std::cout << "(a <= f) " << (a <= f) << '\n'; // 0
    std::cout << "(a >= f) " << (a >= f) << '\n'; // 0

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == f) " << (a == f) << '\n'; // 0
    std::cout << "(a != f) " << (a != f) << '\n'; // 1

    std::cout << "-------------------------\n";
}

END_NS

BEGIN_NS(Case03)

// 자식 쪽의 operator<=>가 default로 정의되어 있는 경우 부모 쪽은 operator<와 operator==만 정의해도 된다.

struct Base
{
    int iValue{ };

    // less와 greater 체크를 진행하는 과정에서 2번 호출될 수 있다.
    bool operator<(const Base& rhs) const
    {
        std::cout << "operator< ";

        return iValue < rhs.iValue;
    }

    bool operator==(const Base& rhs) const
    {
        std::cout << "operator== ";

        return iValue == rhs.iValue;
    }

    // 사용자 정의 operator== 대신 이 방법을 써도 된다.
    // bool operator==(const Base& rhs) const = default;
};

struct Derived : Base
{
    float fValue{ };

    // 비교 도중 사용자 정의 연산자를 사용한다면 이쪽의 operator<=>는 반드시 적절한 반환 타입을 명시해야 한다.
    std::partial_ordering operator<=>(const Derived& rhs) const = default;
};

void Run()
{
    Derived a{ 100, 3.14f };
    Derived b{ 100, 3.14f };
    Derived c{ 150, 1.57f };
    Derived d{ 50,  6.28f };
    Derived e{ 100, 6.28f };
    Derived f{ 100, std::numeric_limits<float>::quiet_NaN() };

    std::cout << "ordering : " << typeid(a <=> b).name() << '\n'; // struct std::partial_ordering
    std::cout << "ordering : " << typeid(a <=> c).name() << '\n'; // struct std::partial_ordering

    std::cout << "-------------------------\n";

    std::cout << "{ 100, 3.14f } @ { 100, 3.14f }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < b)  " << (a < b)  << '\n'; // 0
    std::cout << "(a > b)  " << (a > b)  << '\n'; // 0
    std::cout << "(a <= b) " << (a <= b) << '\n'; // 1
    std::cout << "(a >= b) " << (a >= b) << '\n'; // 1

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == b) " << (a == b) << '\n'; // 1
    std::cout << "(a != b) " << (a != b) << '\n'; // 0

    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 3.14f } @ { 150, 1.57f }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < c)  " << (a < c)  << '\n'; // 1
    std::cout << "(a > c)  " << (a > c)  << '\n'; // 0
    std::cout << "(a <= c) " << (a <= c) << '\n'; // 1
    std::cout << "(a >= c) " << (a >= c) << '\n'; // 0

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == c) " << (a == c) << '\n'; // 0
    std::cout << "(a != c) " << (a != c) << '\n'; // 1

    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 3.14f } @ { 50, 6.28f }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < d)  " << (a < d)  << '\n'; // 0
    std::cout << "(a > d)  " << (a > d)  << '\n'; // 1
    std::cout << "(a <= d) " << (a <= d) << '\n'; // 0
    std::cout << "(a >= d) " << (a >= d) << '\n'; // 1

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == d) " << (a == d) << '\n'; // 0
    std::cout << "(a != d) " << (a != d) << '\n'; // 1

    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 3.14f } @ { 100, 6.28f }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < e)  " << (a < e)  << '\n'; // 1
    std::cout << "(a > e)  " << (a > e)  << '\n'; // 0
    std::cout << "(a <= e) " << (a <= e) << '\n'; // 1
    std::cout << "(a >= e) " << (a >= e) << '\n'; // 0

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == e) " << (a == e) << '\n'; // 0
    std::cout << "(a != e) " << (a != e) << '\n'; // 1

    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 3.14f } @ { 100, NaN }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < f)  " << (a < f)  << '\n'; // 0
    std::cout << "(a > f)  " << (a > f)  << '\n'; // 0
    std::cout << "(a <= f) " << (a <= f) << '\n'; // 0
    std::cout << "(a >= f) " << (a >= f) << '\n'; // 0

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == f) " << (a == f) << '\n'; // 0
    std::cout << "(a != f) " << (a != f) << '\n'; // 1

    std::cout << "-------------------------\n";
}

END_NS

BEGIN_NS(Case04)

// 객체를 멤버 변수로 사용하는 형태도 자주 볼 수 있다.
// 이때 객체를 멤버 변수로 사용하는 쪽에서 operator<=>를 default로 정의했을 경우 멤버가 되는 객체는 다음 두 조건 중 하나를 만족해야 한다.
// 1. operator<=>를 지원해야 함(수동으로 정의한 경우 operator==도 정의해주는 것이 좋음).
// 2. 사용자 정의 operator<와 operator==를 지원해야 함(operator==는 default로 정의해도 됨).

struct Item
{
    int itemVal{ };

    auto operator<=>(const Item& rhs) const = default;
};

struct Object
{
    Item item;
    int  objVal{ };

    auto operator<=>(const Object& rhs) const = default;
};

void Run()
{
    Object a{ 100, 1000 };
    Object b{ 100, 1000 };
    Object c{ 150, 500 };
    Object d{ 50,  2000 };
    Object e{ 100, 2000 };

    std::cout << "ordering : " << typeid(a <=> b).name() << '\n'; // struct std::strong_ordering
    std::cout << "ordering : " << typeid(a <=> c).name() << '\n'; // struct std::strong_ordering
    
    std::cout << "-------------------------\n";

    std::cout << "{ 100, 1000 } @ { 100, 1000 }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < b)  " << (a < b)  << '\n'; // 0
    std::cout << "(a > b)  " << (a > b)  << '\n'; // 0
    std::cout << "(a <= b) " << (a <= b) << '\n'; // 1
    std::cout << "(a >= b) " << (a >= b) << '\n'; // 1

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == b) " << (a == b) << '\n'; // 1
    std::cout << "(a != b) " << (a != b) << '\n'; // 0
    
    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 1000 } @ { 150, 500 }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < c)  " << (a < c)  << '\n'; // 1
    std::cout << "(a > c)  " << (a > c)  << '\n'; // 0
    std::cout << "(a <= c) " << (a <= c) << '\n'; // 1
    std::cout << "(a >= c) " << (a >= c) << '\n'; // 0

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == c) " << (a == c) << '\n'; // 0
    std::cout << "(a != c) " << (a != c) << '\n'; // 1
    
    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 1000 } @ { 50, 2000 }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < d)  " << (a < d)  << '\n'; // 0
    std::cout << "(a > d)  " << (a > d)  << '\n'; // 1
    std::cout << "(a <= d) " << (a <= d) << '\n'; // 0
    std::cout << "(a >= d) " << (a >= d) << '\n'; // 1

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == d) " << (a == d) << '\n'; // 0
    std::cout << "(a != d) " << (a != d) << '\n'; // 1
    
    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 1000 } @ { 100, 2000 }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < e)  " << (a < e)  << '\n'; // 1
    std::cout << "(a > e)  " << (a > e)  << '\n'; // 0
    std::cout << "(a <= e) " << (a <= e) << '\n'; // 1
    std::cout << "(a >= e) " << (a >= e) << '\n'; // 0

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == e) " << (a == e) << '\n'; // 0
    std::cout << "(a != e) " << (a != e) << '\n'; // 1

    std::cout << "-------------------------\n";
}

END_NS

BEGIN_NS(Case05)

// 멤버 객체 차원에서 사용자 정의 operator<=>를 구현해도 된다.
// 마찬가지로 완전한 비교 연산(<, >, <=, >=, ==, !=)을 지원하기 위해 operator==도 정의하는 것이 좋다.

struct Item
{
    int itemVal{ };
    
    auto operator<=>(const Item& rhs) const
    {
        std::cout << "operator<=> ";

        return itemVal <=> rhs.itemVal;
    }

    bool operator==(const Item& rhs) const
    {
        std::cout << "operator== ";

        return itemVal == rhs.itemVal;
    }
};

struct Object
{
    Item item;
    int  objVal{ };

    // 비교 도중 사용자 정의 연산자를 사용한다면 이쪽의 operator<=>는 반드시 적절한 반환 타입을 명시해야 한다.
    std::strong_ordering operator<=>(const Object& rhs) const = default;
};

void Run()
{
    Object a{ 100, 1000 };
    Object b{ 100, 1000 };
    Object c{ 150, 500 };
    Object d{ 50,  2000 };
    Object e{ 100, 2000 };

    std::cout << "ordering : " << typeid(a <=> b).name() << '\n'; // struct std::strong_ordering
    std::cout << "ordering : " << typeid(a <=> c).name() << '\n'; // struct std::strong_ordering
    
    std::cout << "-------------------------\n";

    std::cout << "{ 100, 1000 } @ { 100, 1000 }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < b)  " << (a < b)  << '\n'; // 0
    std::cout << "(a > b)  " << (a > b)  << '\n'; // 0
    std::cout << "(a <= b) " << (a <= b) << '\n'; // 1
    std::cout << "(a >= b) " << (a >= b) << '\n'; // 1

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == b) " << (a == b) << '\n'; // 1
    std::cout << "(a != b) " << (a != b) << '\n'; // 0
    
    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 1000 } @ { 150, 500 }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < c)  " << (a < c)  << '\n'; // 1
    std::cout << "(a > c)  " << (a > c)  << '\n'; // 0
    std::cout << "(a <= c) " << (a <= c) << '\n'; // 1
    std::cout << "(a >= c) " << (a >= c) << '\n'; // 0

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == c) " << (a == c) << '\n'; // 0
    std::cout << "(a != c) " << (a != c) << '\n'; // 1
    
    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 1000 } @ { 50, 2000 }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < d)  " << (a < d)  << '\n'; // 0
    std::cout << "(a > d)  " << (a > d)  << '\n'; // 1
    std::cout << "(a <= d) " << (a <= d) << '\n'; // 0
    std::cout << "(a >= d) " << (a >= d) << '\n'; // 1

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == d) " << (a == d) << '\n'; // 0
    std::cout << "(a != d) " << (a != d) << '\n'; // 1
    
    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 1000 } @ { 100, 2000 }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < e)  " << (a < e)  << '\n'; // 1
    std::cout << "(a > e)  " << (a > e)  << '\n'; // 0
    std::cout << "(a <= e) " << (a <= e) << '\n'; // 1
    std::cout << "(a >= e) " << (a >= e) << '\n'; // 0

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == e) " << (a == e) << '\n'; // 0
    std::cout << "(a != e) " << (a != e) << '\n'; // 1

    std::cout << "-------------------------\n";
}

END_NS

BEGIN_NS(Case06)

// 멤버 객체를 사용하는 쪽의 operator<=>가 default로 정의되어 있다면 멤버 객체는 operator<와 operator==만 정의해도 된다.

struct Item
{
    int itemVal{ };
    
    // less와 greater 체크를 진행하는 과정에서 2번 호출될 수 있다.
    bool operator<(const Item& rhs) const
    {
        std::cout << "operator< ";

        return itemVal < rhs.itemVal;
    }

    bool operator==(const Item& rhs) const
    {
        std::cout << "operator== ";

        return itemVal == rhs.itemVal;
    }

    // 사용자 정의 operator== 대신 이 방법을 써도 된다.
    // bool operator==(const Item& rhs) const = default;
};

struct Object
{
    Item item;
    int  objVal{ };

    // 비교 도중 사용자 정의 연산자를 사용한다면 이쪽의 operator<=>는 반드시 적절한 반환 타입을 명시해야 한다.
    std::strong_ordering operator<=>(const Object& rhs) const = default;
};

void Run()
{
    Object a{ 100, 1000 };
    Object b{ 100, 1000 };
    Object c{ 150, 500 };
    Object d{ 50,  2000 };
    Object e{ 100, 2000 };

    std::cout << "ordering : " << typeid(a <=> b).name() << '\n'; // struct std::strong_ordering
    std::cout << "ordering : " << typeid(a <=> c).name() << '\n'; // struct std::strong_ordering
    
    std::cout << "-------------------------\n";

    std::cout << "{ 100, 1000 } @ { 100, 1000 }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < b)  " << (a < b)  << '\n'; // 0
    std::cout << "(a > b)  " << (a > b)  << '\n'; // 0
    std::cout << "(a <= b) " << (a <= b) << '\n'; // 1
    std::cout << "(a >= b) " << (a >= b) << '\n'; // 1

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == b) " << (a == b) << '\n'; // 1
    std::cout << "(a != b) " << (a != b) << '\n'; // 0
    
    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 1000 } @ { 150, 500 }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < c)  " << (a < c)  << '\n'; // 1
    std::cout << "(a > c)  " << (a > c)  << '\n'; // 0
    std::cout << "(a <= c) " << (a <= c) << '\n'; // 1
    std::cout << "(a >= c) " << (a >= c) << '\n'; // 0

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == c) " << (a == c) << '\n'; // 0
    std::cout << "(a != c) " << (a != c) << '\n'; // 1
    
    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 1000 } @ { 50, 2000 }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < d)  " << (a < d)  << '\n'; // 0
    std::cout << "(a > d)  " << (a > d)  << '\n'; // 1
    std::cout << "(a <= d) " << (a <= d) << '\n'; // 0
    std::cout << "(a >= d) " << (a >= d) << '\n'; // 1

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == d) " << (a == d) << '\n'; // 0
    std::cout << "(a != d) " << (a != d) << '\n'; // 1
    
    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 1000 } @ { 100, 2000 }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < e)  " << (a < e)  << '\n'; // 1
    std::cout << "(a > e)  " << (a > e)  << '\n'; // 0
    std::cout << "(a <= e) " << (a <= e) << '\n'; // 1
    std::cout << "(a >= e) " << (a >= e) << '\n'; // 0

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == e) " << (a == e) << '\n'; // 0
    std::cout << "(a != e) " << (a != e) << '\n'; // 1

    std::cout << "-------------------------\n";
}

END_NS

BEGIN_NS(Case07)

// 부모 클래스를 먼저 비교한 다음 멤버 변수를 Member-wise 방향으로 비교한다.
// 멤버 변수를 비교하는 단계에서 멤버 객체를 발견하면 객체 간 동등성 및 대소 관계를 파악하기 위한 검증을 진행한다.

struct Item
{
    int itemVal{ };
    
    // less와 greater 체크를 진행하는 과정에서 2번 호출될 수 있다.
    bool operator<(const Item& rhs) const
    {
        std::cout << "Item::operator< ";

        return itemVal < rhs.itemVal;
    }

    bool operator==(const Item& rhs) const
    {
        std::cout << "Item::operator== ";

        return itemVal == rhs.itemVal;
    }

    // 사용자 정의 operator== 대신 이 방법을 써도 된다.
    // bool operator==(const Item& rhs) const = default;
};

struct Base
{
    int iValue{ };

    // less와 greater 체크를 진행하는 과정에서 2번 호출될 수 있다.
    bool operator<(const Base& rhs) const
    {
        std::cout << "Base::operator< ";

        return iValue < rhs.iValue;
    }

    bool operator==(const Base& rhs) const
    {
        std::cout << "Base::operator== ";

        return iValue == rhs.iValue;
    }

    // 사용자 정의 operator== 대신 이 방법을 써도 된다.
    // bool operator==(const Base& rhs) const = default;
};

struct Derived : Base
{
    // Base -> Item -> fValue 순으로 비교한다.
    Item  item;
    float fValue{ };

    // 비교 도중 사용자 정의 연산자를 사용한다면 이쪽의 operator<=>는 반드시 적절한 반환 타입을 명시해야 한다.
    std::partial_ordering operator<=>(const Derived& rhs) const = default;
};

void Run()
{
    Derived a{ 100, 1000, 3.14f };
    Derived b{ 100, 1000, 3.14f };

    Derived c{ 100, 1000, 6.28f };
    Derived d{ 100, 1000, 1.57f };

    Derived e{ 100, 500,  std::numeric_limits<float>::quiet_NaN() };
    Derived f{ 100, 2000, std::numeric_limits<float>::quiet_NaN() };

    Derived g{ 150, 500,  std::numeric_limits<float>::quiet_NaN() };
    Derived h{ 50,  1500, std::numeric_limits<float>::quiet_NaN() };

    std::cout << "ordering : " << typeid(a <=> b).name() << '\n'; // struct std::partial_ordering

    std::cout << "-------------------------\n";

    std::cout << "{ 100, 1000, 3.14f } @ { 100, 1000, 3.14f }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < b)  " << (a < b)  << '\n'; // 0
    std::cout << "(a > b)  " << (a > b)  << '\n'; // 0
    std::cout << "(a <= b) " << (a <= b) << '\n'; // 1
    std::cout << "(a >= b) " << (a >= b) << '\n'; // 1

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == b) " << (a == b) << '\n'; // 1
    std::cout << "(a != b) " << (a != b) << '\n'; // 0

    std::cout << "-------------------------\n";

    std::cout << "{ 100, 1000, 3.14f } @ { 100, 1000, 6.28f }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < c)  " << (a < c)  << '\n'; // 1
    std::cout << "(a > c)  " << (a > c)  << '\n'; // 0
    std::cout << "(a <= c) " << (a <= c) << '\n'; // 1
    std::cout << "(a >= c) " << (a >= c) << '\n'; // 0

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == c) " << (a == c) << '\n'; // 0
    std::cout << "(a != c) " << (a != c) << '\n'; // 1

    std::cout << "-------------------------\n";

    std::cout << "{ 100, 1000, 3.14f } @ { 100, 1000, 1.57f }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < d)  " << (a < d)  << '\n'; // 0
    std::cout << "(a > d)  " << (a > d)  << '\n'; // 1
    std::cout << "(a <= d) " << (a <= d) << '\n'; // 0
    std::cout << "(a >= d) " << (a >= d) << '\n'; // 1

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == d) " << (a == d) << '\n'; // 0
    std::cout << "(a != d) " << (a != d) << '\n'; // 1

    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 1000, 3.14f } @ { 100, 500, NaN }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < e)  " << (a < e)  << '\n'; // 0
    std::cout << "(a > e)  " << (a > e)  << '\n'; // 1
    std::cout << "(a <= e) " << (a <= e) << '\n'; // 0
    std::cout << "(a >= e) " << (a >= e) << '\n'; // 1

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == e) " << (a == e) << '\n'; // 0
    std::cout << "(a != e) " << (a != e) << '\n'; // 1

    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 1000, 3.14f } @ { 100, 2000, NaN }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < f)  " << (a < f)  << '\n'; // 1
    std::cout << "(a > f)  " << (a > f)  << '\n'; // 0
    std::cout << "(a <= f) " << (a <= f) << '\n'; // 1
    std::cout << "(a >= f) " << (a >= f) << '\n'; // 0

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == f) " << (a == f) << '\n'; // 0
    std::cout << "(a != f) " << (a != f) << '\n'; // 1

    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 1000, 3.14f } @ { 150, 500, NaN }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < g)  " << (a < g)  << '\n'; // 1
    std::cout << "(a > g)  " << (a > g)  << '\n'; // 0
    std::cout << "(a <= g) " << (a <= g) << '\n'; // 1
    std::cout << "(a >= g) " << (a >= g) << '\n'; // 0

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == g) " << (a == g) << '\n'; // 0
    std::cout << "(a != g) " << (a != g) << '\n'; // 1

    std::cout << "-------------------------\n";
    
    std::cout << "{ 100, 1000, 3.14f } @ { 50, 1500, NaN }\n";
    
    // 비교는 operator<=>를 기반으로 한다.
    std::cout << "(a < h)  " << (a < h)  << '\n'; // 0
    std::cout << "(a > h)  " << (a > h)  << '\n'; // 1
    std::cout << "(a <= h) " << (a <= h) << '\n'; // 0
    std::cout << "(a >= h) " << (a >= h) << '\n'; // 1

    // 비교는 operator==를 기반으로 한다.
    std::cout << "(a == h) " << (a == h) << '\n'; // 0
    std::cout << "(a != h) " << (a != h) << '\n'; // 1

    std::cout << "-------------------------\n";
}

END_NS

int main()
{
    // Case01::Run();
    // Case02::Run();
    // Case03::Run();
    // Case04::Run();
    // Case05::Run();
    // Case06::Run();
    Case07::Run();

    return 0;
}
