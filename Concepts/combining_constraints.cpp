// Update Date : 2025-01-17
// OS : Windows 10 64bit
// Program : Visual Studio 2022, Visual Studio 2019, https://godbolt.org/ + gcc-14.2 with the -std=c++20 option
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <concepts>
#include <vector>
#include <list>
#include <set>

using namespace std;

// 순서대로 볼 것
// 
// # concepts에 대한 기본적인 설명과 사용 방법
// 1. requires_clauses_need_bool_expr.cpp
// 2. built-in_concepts.cpp
// 3. abbreviated_function_templates.cpp
// 4. custom_concepts.cpp
// 5. combining_constraints.cpp <-----
// 6. callable_concepts.cpp
// 
// # 위 항목을 한데 묶어서 정리한 내용
// 7. concepts_details.cpp
//
// # 응용하기
// 8. STL/span.cpp(Case02)
//
// Concepts을 학습한 다음 Ranges를 학습하도록 한다.
// Ranges의 동작 방식을 잘 이해하기 위해선 Concepts에 대한 선행 학습이 필요하다.
//

// https://en.cppreference.com/w/cpp/language/constraints#Conjunctions
// https://en.cppreference.com/w/cpp/language/constraints#Disjunctions
// https://en.cppreference.com/w/cpp/language/constraints#Requires_clauses

// concept이나 requires clauses를 구성할 때 제약 조건은 &&(AND), ||(OR), !(NOT)으로 묶어서 표현할 수 있다.
// 이를 통해 조건을 유연하게 연결하여 제약 조건을 세밀하게 거는 것이 가능하다.

// 포인터의 크기에 따라 조건을 달리하고 싶을 때 사용할 법한 concept
template <typename T>
concept PointerType = std::is_pointer_v<T>;

template <typename T>
concept x86PointerType = PointerType<T> && (sizeof(T) == 4);

template <typename T>
concept x64PointerType = PointerType<T> && (sizeof(T) == 8);

void PrintPointerInfo(x86PointerType auto x)
{
    cout << "System uses x86 pointers.\n";
}

void PrintPointerInfo(x64PointerType auto x)
{
    cout << "System uses x64 pointers.\n";
}

// 다음과 같이 사용자 정의 concept을 묶어서 새로운 사용자 정의 concept을 만드는 것도 가능하다.
template <typename T>
concept SmallInteger = std::integral<T> && (sizeof(T) <= 2);

template <typename T>
concept SignedSmallInteger = SmallInteger<T> && !std::unsigned_integral<T>;

template <typename T>
concept UnsignedSmallInteger = SmallInteger<T> && std::unsigned_integral<T>;

void PrintSmallIntInfo(UnsignedSmallInteger auto x)
{
    cout << "This is Print(UnsignedSmallInteger auto x) : " << typeid(x).name() << "\n";
}

void PrintSmallIntInfo(SignedSmallInteger auto x)
{
    cout << "This is Print(SignedSmallInteger auto x) : " << typeid(x).name() << "\n";
}

// concept을 정의하기 위한 requires를 논리 연산자로 묶는 것도 가능하다.
template <typename T>
concept Addable = 
    requires(T x) { { x + x } -> std::same_as<T>; } ||
    requires(T x) { { x += x } -> std::same_as<T>; };

template <typename T>
concept Subtractable =
    requires(T x) { { x - x } -> std::same_as<T>; } ||
    requires(T x) { { x -= x } -> std::same_as<T>; };

template <typename T>
concept OStreamable = requires (std::ostream& os, T x)
{ 
    { os << x } -> std::same_as<ostream&>;
};

// 논리 연산자는 concept을 정의하는 용도 외 템플릿 코드에서 requires clause를 적용하는 쪽에서도 사용할 수 있다.
template <typename T>
    requires Addable<T> && Subtractable<T> && OStreamable<T>
void PrintAddAndSubtract(T a, T b)
{
    cout << "PrintAddAndSubtract(), Type[" << typeid(T).name() <<"] : " << a + b << ", " << a - b << "\n";
}

struct FooArithmeticA
{
    FooArithmeticA operator+(const FooArithmeticA& rhs)
    {
        return FooArithmeticA{ };
    }

    FooArithmeticA operator-(const FooArithmeticA& rhs)
    {
        return FooArithmeticA{ };
    }

    friend ostream& operator<<(ostream& os, const FooArithmeticA& rhs)
    {
        os << "FooArithmeticA";

        return os;
    }
};

struct FooArithmeticB
{
    FooArithmeticB operator+(const FooArithmeticB& rhs)
    {
        return FooArithmeticB{ };
    }

    FooArithmeticB operator-(const FooArithmeticB& rhs)
    {
        return FooArithmeticB{ };
    }
};

struct FooArithmeticC
{
    friend ostream& operator<<(ostream& os, const FooArithmeticC& rhs)
    {
        os << "FooArithmeticC";

        return os;
    }
};

int main()
{
    PrintPointerInfo(static_cast<void*>(nullptr));

    // 인자로 넘기는 값이 포인터 유형이 아니기 때문에 컴파일 에러 발생
    // PrintPointerInfo(100);

    PrintSmallIntInfo('A');
    PrintSmallIntInfo(short{ 10 });
    PrintSmallIntInfo(unsigned short{ 20 });

    // 다음 Integral 자료형은 2바이트를 넘어서는 크기이기 때문에 컴파일 에러 발생
    // PrintSmallIntInfo(100);
    // PrintSmallIntInfo(20ll);
    // PrintSmallIntInfo(30ul);

    PrintAddAndSubtract(100, 50);
    PrintAddAndSubtract(FooArithmeticA{ }, FooArithmeticA{ });

    // 출력 방법을 정의하지 않았기 때문에 컴파일 에러 발생
    // PrintAddAndSubtract(FooArithmeticB{ }, FooArithmeticB{ });
    
    // 덧셈과 뺄셈에 대한 연산자 오버로딩을 진행하지 않아 컴파일 에러 발생
    // PrintAddAndSubtract(FooArithmeticC{ }, FooArithmeticC{ });

    return 0;
}
