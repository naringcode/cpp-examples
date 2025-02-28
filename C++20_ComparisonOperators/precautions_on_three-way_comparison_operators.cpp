// Update Date : 2025-02-27
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
// 5. equality_operators_after_c++20.cpp
// 6. lexicographical_equality_comparisons.cpp
// 7. precautions_on_three-way_comparison_operators.cpp <-----
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

// Three-way 비교 연산자를 사용할 때 주의해야 하는 점

#define BEGIN_NS(name) namespace name {
#define END_NS }

BEGIN_NS(Case01)

// operator<=>를 default로 정의하는 건 다음과 같은 특징이 있다.
// 
// - 컴파일러 차원에서 operator==를 정의함(컴파일러가 정의한 operator==은 operator<=>와는 독립된 함수임).
//   - 관계 연산(<, >, <=, >=)은 operator<=>로 처리하고 동등 연산(==, !=)은 operator==로 처리함.
//   - 따라서 모든 비교 연산을 사용할 수 있음(<, >, <=, >=, ==, !=).
// 
// - Member-wise 방식과 더불어 사전 순서 비교(lexicographical comparison)를 진행함.
// 
// - (중요) 명시하지 않았어도 operator<=>와 operator==에는 constexpr과 noexcept 옵션이 적용됨.
//

struct Number
{
    int num = 0;

    constexpr Number() = default;
    constexpr Number(int num)
        : num{ num }
    { }

    auto operator<=>(const Number& rhs) const = default;
};

void Run()
{
    constexpr Number x{ 10 };
    constexpr Number y{ 20 };

    // 비교 연산(<, >, <=, >=) : operator<=>가 처리한다.
    bool relComp1 = (x < y);
    bool relComp2 = (x > y);
    bool relComp3 = (x <= y);
    bool relComp4 = (x >= y);

    // 동등 연산(==, !=) : operator==가 처리한다(operator==는 operator<=>를 default로 만들면 자동 생성됨).
    bool eqComp1 = (x == y);
    bool eqComp2 = (x != y);

    // 아래 비교 연산은 컴파일 에러가 발생하지 않는다(constexpr 연산 가능).
    constexpr bool relComp5 = (x < y);
    constexpr bool relComp6 = (x > y);
    constexpr bool relComp7 = (x <= y);
    constexpr bool relComp8 = (x >= y);

    // 컴파일러가 정의한 operator== 또한 constexpr 방식을 지원한다.
    constexpr bool eqComp3 = (x == y);
    constexpr bool eqComp4 = (x != y);

    // operator<=>를 noexcept로 만들지 않았지만 noexcept 방식을 사용하는 것도 가능하다.
    // 출력 결과를 보면 전부 1이 나온다.
    std::cout << "noexcept(x < y)  " << noexcept(x < y) << '\n';
    std::cout << "noexcept(x > y)  " << noexcept(x > y) << '\n';
    std::cout << "noexcept(x <= y) " << noexcept(x <= y) << '\n';
    std::cout << "noexcept(x >= y) " << noexcept(x >= y) << '\n' ;

    // 컴파일러가 정의한 operator== 또한 noexcept 방식을 지원한다.
    std::cout << "noexcept(x == y)  " << noexcept(x == y) << '\n';
    std::cout << "noexcept(x != y)  " << noexcept(x != y) << '\n';
}

END_NS

BEGIN_NS(Case02)

// operator<=>를 수동으로 정의하면 다음과 같은 특징이 있다.
// 
// - operator==를 정의해주지 않음(직접 정의해야 함).
//   - operator==를 수동으로 정의하지 않을 경우 관계 연산(<, >, <=, >=)만 사용할 수 있음.
// 
// - (중요) 명시하지 않았으면 operator<=>에는 constexpr과 noexcept 옵션이 적용되지 않음.
//

struct Number
{
    int num = 0;

    constexpr Number() = default;
    constexpr Number(int num)
        : num{ num }
    { }

    // operator<=>를 수동으로 정의
    auto operator<=>(const Number& rhs) const
    {
        return num <=> rhs.num;
    }
};

void Run()
{
    constexpr Number x{ 10 };
    constexpr Number y{ 20 };

    // 비교 연산(<, >, <=, >=) : operator<=>가 처리한다.
    bool relComp1 = (x < y);
    bool relComp2 = (x > y);
    bool relComp3 = (x <= y);
    bool relComp4 = (x >= y);

    // 동등 연산(==, !=) : operator<=>를 수동으로 정의했기에 컴파일러 차원에서 operator==를 정의하지 않아 컴파일 에러 발생
    // bool eqComp1 = (x == y);
    // bool eqComp2 = (x != y);

    // 아래 비교 연산은 컴파일 에러가 발생한다(constexpr을 적용하지 않았기에 컴파일 에러 발생).
    // constexpr bool relComp5 = (x < y);
    // constexpr bool relComp6 = (x > y);
    // constexpr bool relComp7 = (x <= y);
    // constexpr bool relComp8 = (x >= y);
    // 
    // constexpr bool eqComp3 = (x == y);
    // constexpr bool eqComp4 = (x != y);

    // operator<=>를 수동으로 정의하였지만 noexcept 옵션을 적용하지 않았기에 컴파일 에러 발생
    // 출력 결과를 보면 전부 0이 나온다.
    std::cout << "noexcept(x < y)  " << noexcept(x < y) << '\n';
    std::cout << "noexcept(x > y)  " << noexcept(x > y) << '\n';
    std::cout << "noexcept(x <= y) " << noexcept(x <= y) << '\n';
    std::cout << "noexcept(x >= y) " << noexcept(x >= y) << '\n' ;

    // operator== 자체가 없기 때문에 컴파일 에러 발생
    // std::cout << "noexcept(x == y)  " << noexcept(x == y) << '\n';
    // std::cout << "noexcept(x != y)  " << noexcept(x != y) << '\n';
}

END_NS

BEGIN_NS(Case03)

// 이런 이유로 operator<=>를 수동으로 정의했으면 operator==도 같이 정의하는 것이 좋다.
// 또한 constexpr과 noexcept 옵션을 적용하고 싶다면 수동으로 작성할 경우에는 반드시 명시해야 한다.

struct Number
{
    int num = 0;

    constexpr Number() = default;
    constexpr Number(int num)
        : num{ num }
    { }

    // operator<=>를 수동으로 정의하고 constexpr과 noexcept 적용
    constexpr auto operator<=>(const Number& rhs) const noexcept
    {
        return num <=> rhs.num;
    }
    
    // operator<=>를 default 방식이 아닌 수동으로 정의하면 컴파일러 차원에서 operator==를 정의해주지 않기 때문에 직접 정의해야 한다.
    bool operator==(const Number& rhs) const = default;
};

void Run()
{
    constexpr Number x{ 10 };
    constexpr Number y{ 20 };

    // 비교 연산(<, >, <=, >=) : operator<=>가 처리한다.
    bool relComp1 = (x < y);
    bool relComp2 = (x > y);
    bool relComp3 = (x <= y);
    bool relComp4 = (x >= y);

    // 동등 연산(==, !=) : operator==가 처리한다.
    bool eqComp1 = (x == y);
    bool eqComp2 = (x != y);

    // 아래 비교 연산은 컴파일 에러가 발생하지 않는다(constexpr 연산 가능).
    constexpr bool relComp5 = (x < y);
    constexpr bool relComp6 = (x > y);
    constexpr bool relComp7 = (x <= y);
    constexpr bool relComp8 = (x >= y);

    // operator==를 default 방식으로 정의하면 constexpr 방식을 지원한다.
    constexpr bool eqComp3 = (x == y);
    constexpr bool eqComp4 = (x != y);

    // operator<=>를 noexcept로 만들지 않았지만 noexcept 방식을 사용하는 것도 가능하다.
    // 출력 결과를 보면 전부 1이 나온다.
    std::cout << "noexcept(x < y)  " << noexcept(x < y) << '\n';
    std::cout << "noexcept(x > y)  " << noexcept(x > y) << '\n';
    std::cout << "noexcept(x <= y) " << noexcept(x <= y) << '\n';
    std::cout << "noexcept(x >= y) " << noexcept(x >= y) << '\n' ;

    // operator==를 default 방식으로 정의하면 noexcept 방식을 지원한다.
    std::cout << "noexcept(x == y)  " << noexcept(x == y) << '\n';
    std::cout << "noexcept(x != y)  " << noexcept(x != y) << '\n';
}

END_NS

BEGIN_NS(Case04)

// !! 중요 !!
// operator<=>가 있고(default가 적용되었든 아니든), 사용자 정의 비교 연산자가 둘 다 정의되어 있는 경우
// 컴파일러는 사용자 정의 비교 연산자를 우선시한다.

struct Number
{
    int num = 0;

    constexpr Number() = default;
    constexpr Number(int num)
        : num{ num }
    { }

    auto operator<=>(const Number& rhs) const = default;

    // 사용자 정의 비교 연산자
    bool operator<(const Number& rhs) const
    {
        std::cout << "operator< ";

        return num < rhs.num;
    }

    bool operator>(const Number& rhs) const
    {
        std::cout << "operator> ";

        return num > rhs.num;
    }

    bool operator==(const Number& rhs) const
    {
        std::cout << "operator!= ";

        return num == rhs.num;
    }

    // operator==의 재작성 규칙으로 Not equal(!=) 연산이 수행되기 때문에 사용 불가(컴파일 에러 발생)
    // bool operator!=(const Number& rhs) const
    // {
    //     std::cout << "operator== ";
    // 
    //     return num == rhs.num;
    // }
};

void Run()
{
    Number x{ 5 };
    Number y{ 5 };

    std::cout << "(x < y)  " << (x < y)  << '\n'; // (x < 0) : 0
    std::cout << "(x > y)  " << (x > y)  << '\n'; // (x > 0) : 0
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

int main()
{
    // Case01::Run();
    // Case02::Run();
    // Case03::Run();
    Case04::Run();

    return 0;
}
