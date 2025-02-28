// Update Date : 2025-02-24
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++17
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <utility>
#include <vector>

// 단계별 학습을 위해서라도 다음 순서대로 보도록 하자.
// 
// # C++20 비교 연산자 개요(Three-way 비교 연산자를 위주로 다룸)
// 1. c++20_comparison_operators_intro.txt
// 
// # C++20 이전의 비교 연산
// 2. user_defined_comparison_operators_before_c++20.cpp <-----
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

// https://en.cppreference.com/w/cpp/language/operator_comparison
// https://en.cppreference.com/w/cpp/utility/rel_ops/operator_cmp

#define BEGIN_NS(name) namespace name {
#define END_NS }

BEGIN_NS(Case01)

// 사용자 정의 비교 연산자를 멤버 함수로 작성하는 경우
// 다만 이 경우에는 정수가 왼쪽에 왔을 경우 적절한 함수를 찾지 못 한다.

struct Number
{
    int num = 0;

    Number() = default;
    Number(int num)
        : num{ num }
    { }

    // C++ 내부에서 자주 사용하는 비교 연산자
    bool operator<(const Number& rhs) const
    {
        return num < rhs.num;
    }

    bool operator==(const Number& rhs) const
    {
        return num == rhs.num;
    }

    // 나머지 비교 연산자는 위에서 정의한 두 개의 비교 연산자로부터 유도할 수 있다.
    bool operator!=(const Number& rhs) const
    {
        return !(*this == rhs);
    }

    bool operator>(const Number& rhs) const
    {
        return rhs < *this;
    }

    bool operator<=(const Number& rhs) const
    {
        return !(rhs < *this);
    }

    bool operator>=(const Number& rhs) const
    {
        return !(*this < rhs);
    }
};

void Run()
{
    Number num1{ 10 };
    Number num2{ 20 };

    std::cout << "num1 < num2  : " << (num1 < num2) << '\n';
    std::cout << "num1 > num2  : " << (num1 > num2) << '\n';
    std::cout << "num1 <= num2 : " << (num1 <= num2) << '\n';
    std::cout << "num1 >= num2 : " << (num1 >= num2) << '\n';
    std::cout << "num1 == num2 : " << (num1 == num2) << '\n';
    std::cout << "num1 != num2 : " << (num1 != num2) << '\n';

    std::cout << "-------------------------\n";

    // num1과 정수 5를 비교하면 비교 연산자를 num1.operator@(5)로 받는다.
    // 이 과정에서 operator@가 받는 Number의 생성자가 5를 받을 수 있기 때문에 암묵적으로 5를 Number로 변환해서 받을 수 있다.
    std::cout << "num1 < 5  : " << (num1 < 5) << '\n';
    std::cout << "num1 > 5  : " << (num1 > 5) << '\n';
    std::cout << "num1 <= 5 : " << (num1 <= 5) << '\n';
    std::cout << "num1 >= 5 : " << (num1 >= 5) << '\n';
    std::cout << "num1 == 5 : " << (num1 == 5) << '\n';
    std::cout << "num1 != 5 : " << (num1 != 5) << '\n';

    std::cout << "-------------------------\n";

    // 하지만 정수가 먼저 위치하게 되면 정수에 해당하는 멤버 변수는 찾을 수 없기 때문에 컴파일 에러가 발생한다.
    // 15.operator@(num2) <----- 이런 코드는 존재할 수 없음.
    // operator@(15, num2) <----- 이런 식으로 대응하는 함수도 없음.
    // std::cout << "15 < num2  : " << (15 < num2) << '\n';
    // std::cout << "15 > num2  : " << (15 > num2) << '\n';
    // std::cout << "15 <= num2 : " << (15 <= num2) << '\n';
    // std::cout << "15 >= num2 : " << (15 >= num2) << '\n';
    // std::cout << "15 == num2 : " << (15 == num2) << '\n';
    // std::cout << "15 != num2 : " << (15 != num2) << '\n';
}

END_NS

BEGIN_NS(Case02)

// 사용자 정의 비교 연산자를 전역 함수로 작성하는 경우

struct Number
{
    int num = 0;

    Number() = default;
    Number(int num)
        : num{ num }
    { }

    // C++ 내부에서 자주 사용하는 비교 연산자
    friend bool operator<(const Number& lhs, const Number& rhs)
    {
        return lhs.num < rhs.num;
    }

    friend bool operator==(const Number& lhs, const Number& rhs)
    {
        return lhs.num == rhs.num;
    }

    // 나머지 비교 연산자는 위에서 정의한 두 개의 비교 연산자로부터 유도할 수 있다.
    friend bool operator!=(const Number& lhs, const Number& rhs)
    {
        return !(lhs == rhs);
    }

    friend bool operator>(const Number& lhs, const Number& rhs)
    {
        return rhs < lhs;
    }

    friend bool operator<=(const Number& lhs, const Number& rhs)
    {
        return !(rhs < lhs);
    }

    friend bool operator>=(const Number& lhs, const Number& rhs)
    {
        return !(lhs < rhs);
    }
};

void Run()
{
    Number num1{ 10 };
    Number num2{ 20 };

    std::cout << "num1 < num2  : " << (num1 < num2) << '\n';
    std::cout << "num1 > num2  : " << (num1 > num2) << '\n';
    std::cout << "num1 <= num2 : " << (num1 <= num2) << '\n';
    std::cout << "num1 >= num2 : " << (num1 >= num2) << '\n';
    std::cout << "num1 == num2 : " << (num1 == num2) << '\n';
    std::cout << "num1 != num2 : " << (num1 != num2) << '\n';

    std::cout << "-------------------------\n";

    // num1과 정수 5를 비교하면 비교 연산자를 num1.operator@(5)로 받는다.
    // 이 과정에서 operator@가 받는 Number의 생성자가 5를 받을 수 있기 때문에 암묵적으로 5를 Number로 변환해서 받을 수 있다.
    std::cout << "num1 < 5  : " << (num1 < 5) << '\n';
    std::cout << "num1 > 5  : " << (num1 > 5) << '\n';
    std::cout << "num1 <= 5 : " << (num1 <= 5) << '\n';
    std::cout << "num1 >= 5 : " << (num1 >= 5) << '\n';
    std::cout << "num1 == 5 : " << (num1 == 5) << '\n';
    std::cout << "num1 != 5 : " << (num1 != 5) << '\n';

    std::cout << "-------------------------\n";

    // 비교 연산자를 전역으로 정의하고 암묵적으로 형변환을 통해 대응 관계를 사용할 수 있다면 컴파일러는 이를 허용한다.
    // 15.operator@(num2) <----- 이런 식의 코드는 찾을 수 없지만
    // operator@(15, num2) <----- 이런 식으로 대응하는 함수는 찾을 수 있음.
    std::cout << "15 < num2  : " << (15 < num2) << '\n';
    std::cout << "15 > num2  : " << (15 > num2) << '\n';
    std::cout << "15 <= num2 : " << (15 <= num2) << '\n';
    std::cout << "15 >= num2 : " << (15 >= num2) << '\n';
    std::cout << "15 == num2 : " << (15 == num2) << '\n';
    std::cout << "15 != num2 : " << (15 != num2) << '\n';
}

END_NS

BEGIN_NS(Case03)

// 인자를 받기 위한 생성자가 explicit으로 되어 있으면 암묵적인 형변환을 허용하지 않으니 주의해야 한다.

struct Number
{
    int num = 0;

    // 생성자에 explicit 적용
    explicit Number() = default;
    explicit Number(int num)
        : num{ num }
    { }

    // C++ 내부에서 자주 사용하는 비교 연산자
    friend bool operator<(const Number& lhs, const Number& rhs)
    {
        return lhs.num < rhs.num;
    }

    friend bool operator==(const Number& lhs, const Number& rhs)
    {
        return lhs.num == rhs.num;
    }

    // 나머지 비교 연산자는 위에서 정의한 두 개의 비교 연산자로부터 유도할 수 있다.
    friend bool operator!=(const Number& lhs, const Number& rhs)
    {
        return !(lhs == rhs);
    }

    friend bool operator>(const Number& lhs, const Number& rhs)
    {
        return rhs < lhs;
    }

    friend bool operator<=(const Number& lhs, const Number& rhs)
    {
        return !(rhs < lhs);
    }

    friend bool operator>=(const Number& lhs, const Number& rhs)
    {
        return !(lhs < rhs);
    }
};

void Run()
{
    Number num1{ 10 };
    Number num2{ 20 };

    std::cout << "num1 < num2  : " << (num1 < num2) << '\n';
    std::cout << "num1 > num2  : " << (num1 > num2) << '\n';
    std::cout << "num1 <= num2 : " << (num1 <= num2) << '\n';
    std::cout << "num1 >= num2 : " << (num1 >= num2) << '\n';
    std::cout << "num1 == num2 : " << (num1 == num2) << '\n';
    std::cout << "num1 != num2 : " << (num1 != num2) << '\n';

    std::cout << "-------------------------\n";

    // explicit으로 인한 암묵적인 형변환 적용 불가

    // std::cout << "num1 < 5  : " << (num1 < 5) << '\n';
    // std::cout << "num1 > 5  : " << (num1 > 5) << '\n';
    // std::cout << "num1 <= 5 : " << (num1 <= 5) << '\n';
    // std::cout << "num1 >= 5 : " << (num1 >= 5) << '\n';
    // std::cout << "num1 == 5 : " << (num1 == 5) << '\n';
    // std::cout << "num1 != 5 : " << (num1 != 5) << '\n';

    std::cout << "-------------------------\n";

    // std::cout << "15 < num2  : " << (15 < num2) << '\n';
    // std::cout << "15 > num2  : " << (15 > num2) << '\n';
    // std::cout << "15 <= num2 : " << (15 <= num2) << '\n';
    // std::cout << "15 >= num2 : " << (15 >= num2) << '\n';
    // std::cout << "15 == num2 : " << (15 == num2) << '\n';
    // std::cout << "15 != num2 : " << (15 != num2) << '\n';
}

END_NS

BEGIN_NS(Case04)

// 생성자는 explicit으로 하고 싶지만 다른 타입을 비교 연산자로 받고 싶으면
// 대응되는 사용자 정의 비교 연산자를 전부 정의해야 한다.

struct Number
{
    int num = 0;

    explicit Number() = default;
    explicit Number(int num)
        : num{ num }
    { }

    // C++ 내부에서 자주 사용하는 비교 연산자
    friend bool operator<(const Number& lhs, const Number& rhs) { return lhs.num < rhs.num; }
    friend bool operator<(int lhs, const Number& rhs) { return lhs < rhs.num; }
    friend bool operator<(const Number& lhs, int rhs) { return lhs.num < rhs; }

    friend bool operator==(const Number& lhs, const Number& rhs) { return lhs.num == rhs.num; }
    friend bool operator==(int lhs, const Number& rhs) { return lhs == rhs.num; }
    friend bool operator==(const Number& lhs, int rhs) { return lhs.num == rhs; }

    // 나머지 비교 연산자는 위에서 정의한 두 유형의 비교 연산자로부터 유도할 수 있다.
    friend bool operator!=(const Number& lhs, const Number& rhs) { return !(lhs == rhs); }
    friend bool operator!=(int lhs, const Number& rhs) { return !(lhs == rhs); }
    friend bool operator!=(const Number& lhs, int rhs) { return !(lhs == rhs); }

    friend bool operator>(const Number& lhs, const Number& rhs) { return rhs < lhs; }
    friend bool operator>(int lhs, const Number& rhs) { return rhs < lhs; }
    friend bool operator>(const Number& lhs, int rhs) { return rhs < lhs; }

    friend bool operator<=(const Number& lhs, const Number& rhs) { return !(rhs < lhs); }
    friend bool operator<=(int lhs, const Number& rhs) { return !(rhs < lhs); }
    friend bool operator<=(const Number& lhs, int rhs) { return !(rhs < lhs); }

    friend bool operator>=(const Number& lhs, const Number& rhs) { return !(lhs < rhs); }
    friend bool operator>=(int lhs, const Number& rhs) { return !(lhs < rhs); }
    friend bool operator>=(const Number& lhs, int rhs) { return !(lhs < rhs); }
};

void Run()
{
    Number num1{ 10 };
    Number num2{ 20 };

    std::cout << "num1 < num2  : " << (num1 < num2) << '\n';
    std::cout << "num1 > num2  : " << (num1 > num2) << '\n';
    std::cout << "num1 <= num2 : " << (num1 <= num2) << '\n';
    std::cout << "num1 >= num2 : " << (num1 >= num2) << '\n';
    std::cout << "num1 == num2 : " << (num1 == num2) << '\n';
    std::cout << "num1 != num2 : " << (num1 != num2) << '\n';

    std::cout << "-------------------------\n";

    // explicit 생성자로 인해 암묵적인 형변환을 받을 수 없다면 모든 유형의 비교 연산자를 직접 정의해야 한다.

    std::cout << "num1 < 5  : " << (num1 < 5) << '\n';
    std::cout << "num1 > 5  : " << (num1 > 5) << '\n';
    std::cout << "num1 <= 5 : " << (num1 <= 5) << '\n';
    std::cout << "num1 >= 5 : " << (num1 >= 5) << '\n';
    std::cout << "num1 == 5 : " << (num1 == 5) << '\n';
    std::cout << "num1 != 5 : " << (num1 != 5) << '\n';

    std::cout << "-------------------------\n";

    std::cout << "15 < num2  : " << (15 < num2) << '\n';
    std::cout << "15 > num2  : " << (15 > num2) << '\n';
    std::cout << "15 <= num2 : " << (15 <= num2) << '\n';
    std::cout << "15 >= num2 : " << (15 >= num2) << '\n';
    std::cout << "15 == num2 : " << (15 == num2) << '\n';
    std::cout << "15 != num2 : " << (15 != num2) << '\n';
}

END_NS

BEGIN_NS(Case05)

// C++은 operator<와 operator==를 사용할 수 있을 때 나머지 연산자를 정의해주는 rel_ops 네임스페이스를 제공한다.
//
// (주의) C++20부터 Three-way 비교 연산자가 도입됨에 따라 rel_ops는 deprecated된 상태이다.
// 원문 : As of C++20, std::rel_ops are deprecated in favor of operator<=>.

// namespace rel_ops
// {
//     template<class T>
//     bool operator!=(const T& lhs, const T& rhs)
//     {
//         return !(lhs == rhs);
//     }
// 
//     template<class T>
//     bool operator>(const T& lhs, const T& rhs)
//     {
//         return rhs < lhs;
//     }
// 
//     template<class T>
//     bool operator<=(const T& lhs, const T& rhs)
//     {
//         return !(rhs < lhs);
//     }
// 
//     template<class T>
//     bool operator>=(const T& lhs, const T& rhs)
//     {
//         return !(lhs < rhs);
//     }
// }

// C++20으로 해당 네임스페이스를 사용하려고 하면 에러가 뜨기 때문에 다음 매크로를 추가해야 한다.
#define _SILENCE_CXX20_REL_OPS_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX20_DEPRECATION_WARNINGS

// rel_ops는 템플릿 추론 인자에 대응되는 비교 연산자를 찾지 못 한다는 단점이 있다.
// 
// 무엇보다 rel_ops 자체는 C++98에 정의된 오래된 표준이다.
// 따라서 모던 C++이 지원하는 최신 기능을 적용하지 못 한다는 치명적인 단점도 있다.
// 
// rel_ops에 정의된 템플릿 함수를 보면 constexpr도 없고, noexcept도 없다.
// - 컴파일 타임에 평가하기 위한 기능 적용 불가
// - 예외를 던지지 않는 유형에는 적용 불가
// 
// 이는 최적화 및 기능 세분화를 해야 하는 입장에서 보면 굉장히 좋지 못한 측면이다.
//

struct Number
{
    int num = 0;

    Number() = default;
    Number(int num)
        : num{ num }
    { }

    // rel_ops 네임스페이스를 사용하기 위해선 최소한 다음 2가지 비교 연산자를 직접 정의해야 한다.
    friend bool operator<(const Number& lhs, const Number& rhs)
    {
        return lhs.num < rhs.num;
    }

    friend bool operator==(const Number& lhs, const Number& rhs)
    {
        return lhs.num == rhs.num;
    }
};

void Run()
{
    // 이렇게 가져다 쓰면 된다.
    using namespace std::rel_ops;

    Number num1{ 10 };
    Number num2{ 20 };

    // Number에 나머지 비교 연산자가 적용된 것을 볼 수 있다.
    std::cout << "num1 < num2  : " << (num1 < num2) << '\n';
    std::cout << "num1 > num2  : " << (num1 > num2) << '\n';
    std::cout << "num1 <= num2 : " << (num1 <= num2) << '\n';
    std::cout << "num1 >= num2 : " << (num1 >= num2) << '\n';
    std::cout << "num1 == num2 : " << (num1 == num2) << '\n';
    std::cout << "num1 != num2 : " << (num1 != num2) << '\n';

    std::cout << "-------------------------\n";

    // (주의) rel_ops는 템플릿 추론을 기반으로 동작하기 추론 인자에 대응되는 비교 연산자를 찾지 못 한다.
    // bool operator!=(const Number& lhs, const int& rhs) <----- 코드가 이런 식으로 컴파일됨.

    std::cout << "num1 < 5  : " << (num1 < 5) << '\n';
    // std::cout << "num1 > 5  : " << (num1 > 5) << '\n';
    // std::cout << "num1 <= 5 : " << (num1 <= 5) << '\n';
    // std::cout << "num1 >= 5 : " << (num1 >= 5) << '\n';
    std::cout << "num1 == 5 : " << (num1 == 5) << '\n';
    // std::cout << "num1 != 5 : " << (num1 != 5) << '\n';

    std::cout << "-------------------------\n";

    std::cout << "15 < num2  : " << (15 < num2) << '\n';
    // std::cout << "15 > num2  : " << (15 > num2) << '\n';
    // std::cout << "15 <= num2 : " << (15 <= num2) << '\n';
    // std::cout << "15 >= num2 : " << (15 >= num2) << '\n';
    std::cout << "15 == num2 : " << (15 == num2) << '\n';
    // std::cout << "15 != num2 : " << (15 != num2) << '\n';
}

END_NS

int main()
{
    // Case01::Run();
    // Case02::Run();
    // Case03::Run();
    // Case04::Run();
    Case05::Run();

    return 0;
}
