// Update Date : 2025-02-26
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>

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
// 5. equality_operators_after_c++20.cpp <-----
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

// C++20에서는 비교 연산자의 동작 방식이 개선되었기 때문에 동등 연산자(==, !=)에도 변화가 생겼다.
// 일단 operator==에도 재작성 규칙이 도입됨에 따라 != 연산은 덤으로 딸려 온다.

#define BEGIN_NS(name) namespace name {
#define END_NS }

BEGIN_NS(Case01)

// C++20부터는 operator==도 default로 정의하는 것이 가능하다(C++20 이전에는 불가능했음).
// 마찬가지로 이 경우에도 사전 순서에 따른 Member-wise 비교를 진행한다.

struct Item
{
    int x{ 1 };
    int y{ 2 };
    int z{ 3 };

    Item() = default;
    Item(int xyz) : Item(xyz, xyz, xyz)
    { }

    Item(int x, int y, int z)
        : x{ x }, y{ y }, z{ z }
    { }

    // 첫 번째 멤버부터 마지막 멤버까지 비교한다.
    bool operator==(const Item& rhs) const = default;
};

void Run()
{
    Item item1{ 1, 2, 3 };
    Item item2{ 1, 2, 3 };
    Item item3{ 1, 5, 3 };
    Item item4{ 1, 0, 3 };

    std::cout << "(item1 == item2) " << (item1 == item2) << '\n'; // 1
    std::cout << "(item1 != item2) " << (item1 != item2) << '\n'; // !(item1 == item2) : 0

    std::cout << '\n';
    
    std::cout << "(item1 == item3) " << (item1 == item3) << '\n'; // 0
    std::cout << "(item1 != item3) " << (item1 != item3) << '\n'; // !(item1 == item3) : 1

    std::cout << '\n';

    std::cout << "(item1 == item4) " << (item1 == item4) << '\n'; // 0
    std::cout << "(item1 != item4) " << (item1 != item4) << '\n'; // !(item1 == item4) : 0

    std::cout << '\n';

    // 표현식 재작성 규칙에 따라 생성자를 호출할 수 있으면 암묵적인 형변환을 허용한다.
    std::cout << "(item1 == 10)    " << (item1 == 10)    << '\n'; // (item1 == Number(10)) : 0
    std::cout << "(10 == item2)    " << (10 == item2)    << '\n'; // (item1 == Number(10)) : 0

    std::cout << '\n';
    
    std::cout << "(item1 != 20)    " << (item1 != 20)    << '\n'; // !(item1 == Number(20)) : 1
    std::cout << "(20 != item2)    " << (20 != item2)    << '\n'; // !(item1 == Number(20)) : 1
}

END_NS

BEGIN_NS(Case02)

// operator<=>를 default로 정의했을 때와 마찬가지로 operator==를 default로 정의하면 constexpr과 noexcept 옵션이 적용된다.

struct Number
{
    int num = 0;

    constexpr Number() = default;
    constexpr Number(int num)
        : num{ num }
    { }

    bool operator==(const Number& rhs) const = default;
};

void Run()
{
    constexpr Number x{ 5 };
    constexpr Number y{ 5 };

    // 아래 비교 연산은 컴파일 에러가 발생하지 않는다(constexpr 연산 가능).
    constexpr bool eqComp1 = (x == y);
    constexpr bool eqComp2 = (x != y);

    // operator==를 noexcept로 만들지 않았지만 noexcept 방식을 사용하는 것도 가능하다.
    static_assert(noexcept(x == y) == true);
    static_assert(noexcept(x != y) == true);
}

END_NS

BEGIN_NS(Case03)

// (주의) operator==를 수동으로 정의하면 constexpr과 noexcept 옵션은 적용되지 않으니까 주의해야 한다.

struct Number
{
    int num = 0;

    constexpr Number() = default;
    constexpr Number(int num)
        : num{ num }
    { }

    // operator==를 default가 아닌 수동으로 정의
    bool operator==(const Number& rhs) const
    {
        return num == rhs.num;
    }
};

void Run()
{
    constexpr Number x{ 5 };
    constexpr Number y{ 5 };

    // ERROR
    // constexpr bool eqComp1 = (x == y);
    // constexpr bool eqComp2 = (x != y);

    // ERROR
    // static_assert(noexcept(x == y) == true);
    // static_assert(noexcept(x != y) == true);
}

END_NS

BEGIN_NS(Case04)

// (중요) operator<=>를 수동으로 정의했으면 operator==를 만들어주지 않기 때문에 직접 정의해야 한다(default와 다른 점).

struct Number
{
    int num = 0;

    Number() = default;
    Number(int num)
        : num{ num }
    { }

    // 전역으로 구성하는 것도 가능하다.
    // friend auto operator<=>(const Number& lhs, const Number& rhs)
    // {
    //     std::cout << "operator<=> ";
    // 
    //     return lhs.num <=> rhs.num;
    // }
    // 
    // friend bool operator==(const Number& lhs, const Number& rhs)
    // {
    //     std::cout << "operator== ";
    // 
    //     return lhs.num == rhs.num;
    // }

    auto operator<=>(const Number& rhs) const
    {
        std::cout << "operator<=> ";

        return num <=> rhs.num;
    }

    // 이렇게 사용해도 된다.
    // bool operator==(const Number& rhs) const = default;

    bool operator==(const Number& rhs) const
    {
        std::cout << "operator== ";

        return num == rhs.num;
    }
};

void Run()
{
    Number x{ 5 };
    Number y{ 5 };

    std::cout << "(x < y)  " << (x < y)  << '\n'; // (x <=> y < 0) : 0
    std::cout << "(x > y)  " << (x > y)  << '\n'; // (x <=> y > 0) : 0
    std::cout << "(x <= y) " << (x <= y) << '\n'; // (x <=> y <= 0) : 1
    std::cout << "(x >= y) " << (x >= y) << '\n'; // (x <=> y >= 0) : 1

    std::cout << '\n';

    std::cout << "(x == y) " << (x == y) << '\n'; // (x == y) : 1
    std::cout << "(x != y) " << (x != y) << '\n'; // !(x == y) : 0

    std::cout << '\n';

    // Rewritten expressions
    std::cout << "(5 == x) " << (5 == x) << '\n'; // (x == Number(5)) : 1
    std::cout << "(5 != x) " << (5 != x) << '\n'; // !(x == Number(5)) : 0
}

END_NS

BEGIN_NS(Case05)

// operator<=>와 operator==를 사용하면 생성자가 explicit으로 되어 있어 암묵적으로 받을 수 없는 경우의 코드도 쉽게 처리할 수 있다.

struct Number
{
    int num = 0;

    Number() = default;
    Number(int num)
        : num{ num }
    { }

    // // C++ 내부에서 자주 사용하는 비교 연산자
    // friend bool operator<(const Number& lhs, const Number& rhs) { return lhs.num < rhs.num; }
    // friend bool operator<(int lhs, const Number& rhs) { return lhs < rhs.num; }
    // friend bool operator<(const Number& lhs, int rhs) { return lhs.num < rhs; }
    // 
    // friend bool operator==(const Number& lhs, const Number& rhs) { return lhs.num == rhs.num; }
    // friend bool operator==(int lhs, const Number& rhs) { return lhs == rhs.num; }
    // friend bool operator==(const Number& lhs, int rhs) { return lhs.num == rhs; }
    // 
    // // 나머지 비교 연산자는 위에서 정의한 두 유형의 비교 연산자로부터 유도할 수 있다.
    // friend bool operator!=(const Number& lhs, const Number& rhs) { return !(lhs == rhs); }
    // friend bool operator!=(int lhs, const Number& rhs) { return !(lhs == rhs); }
    // friend bool operator!=(const Number& lhs, int rhs) { return !(lhs == rhs); }
    // 
    // friend bool operator>(const Number& lhs, const Number& rhs) { return rhs < lhs; }
    // friend bool operator>(int lhs, const Number& rhs) { return rhs < lhs; }
    // friend bool operator>(const Number& lhs, int rhs) { return rhs < lhs; }
    // 
    // friend bool operator<=(const Number& lhs, const Number& rhs) { return !(rhs < lhs); }
    // friend bool operator<=(int lhs, const Number& rhs) { return !(rhs < lhs); }
    // friend bool operator<=(const Number& lhs, int rhs) { return !(rhs < lhs); }
    // 
    // friend bool operator>=(const Number& lhs, const Number& rhs) { return !(lhs < rhs); }
    // friend bool operator>=(int lhs, const Number& rhs) { return !(lhs < rhs); }
    // friend bool operator>=(const Number& lhs, int rhs) { return !(lhs < rhs); }

    auto operator<=>(const Number& rhs) const = default;

    // operator==(int num)이 정의되어 있기 컴파일러가 연산자를 독립적으로 인지할 수 있게 operator==(const Number& rhs)를 구성해야 한다.
    bool operator==(const Number& rhs) const = default;

    // 이렇게 처리하면 끝이다.
    auto operator<=>(int num) const
    {
        return this->num <=> num;
    }

    bool operator==(int num) const
    {
        return this->num == num;
    }
};

void Run()
{
    Number num1{ 10 };
    Number num2{ 20 };

    std::cout << "num1 < num2  : " << (num1 < num2) << '\n';  // 1
    std::cout << "num1 > num2  : " << (num1 > num2) << '\n';  // 0
    std::cout << "num1 <= num2 : " << (num1 <= num2) << '\n'; // 1
    std::cout << "num1 >= num2 : " << (num1 >= num2) << '\n'; // 0
    std::cout << "num1 == num2 : " << (num1 == num2) << '\n'; // 0
    std::cout << "num1 != num2 : " << (num1 != num2) << '\n'; // 1

    std::cout << "-------------------------\n";

    std::cout << "num1 < 5  : " << (num1 < 5) << '\n';  // 0
    std::cout << "num1 > 5  : " << (num1 > 5) << '\n';  // 1
    std::cout << "num1 <= 5 : " << (num1 <= 5) << '\n'; // 0
    std::cout << "num1 >= 5 : " << (num1 >= 5) << '\n'; // 1
    std::cout << "num1 == 5 : " << (num1 == 5) << '\n'; // 0
    std::cout << "num1 != 5 : " << (num1 != 5) << '\n'; // 1

    std::cout << "-------------------------\n";

    std::cout << "15 < num2  : " << (15 < num2) << '\n';  // 1
    std::cout << "15 > num2  : " << (15 > num2) << '\n';  // 0
    std::cout << "15 <= num2 : " << (15 <= num2) << '\n'; // 1
    std::cout << "15 >= num2 : " << (15 >= num2) << '\n'; // 0
    std::cout << "15 == num2 : " << (15 == num2) << '\n'; // 0
    std::cout << "15 != num2 : " << (15 != num2) << '\n'; // 1
}

END_NS

int main()
{
    // Case01::Run();
    // Case02::Run();
    // Case03::Run();
    // Case04::Run();
    Case04::Run();

    return 0;
}
