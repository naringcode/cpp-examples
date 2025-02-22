// Update Date : 2025-01-16
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
// 4. custom_concepts.cpp <-----
// 5. combining_constraints.cpp
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

// https://en.cppreference.com/w/cpp/language/constraints
// https://en.cppreference.com/w/cpp/concepts
// https://en.cppreference.com/w/cpp/language/requires
// https://en.cppreference.com/w/cpp/keyword/typename
// https://en.cppreference.com/w/cpp/language/template_specialization
// https://en.cppreference.com/w/cpp/language/type_alias
// https://en.cppreference.com/w/cpp/language/noexcept
// https://en.cppreference.com/w/cpp/language/noexcept_spec

#define BEGIN_NS(name) namespace name {
#define END_NS };

BEGIN_NS(Case01)

// https://en.cppreference.com/w/cpp/language/requires

// requires는 제약을 걸기 위해 사용되는 필수 문법이며 다음 두 곳에서 사용되는 키워드이다.
// - concept을 정의하기 위해 requirements을 구성할 목적으로 사용하는 requires
// - 템플릿 코드를 생성할 목적으로 제약을 걸기 위한 requires clauses를 구성할 목적으로 사용하는 requires
// 
// 원문(Constraints and concepts) : Named sets of such requirements are called concepts.
//   - requirements는 Requires expression로 리다이렉트됨(https://en.cppreference.com/w/cpp/language/requires).
// 원문(requires clauses) : The keyword requires is used to introduce a requires clause, which specifies constraints on template arguments or on a function declaration.
//

// requires를 이용하면 직접 concept을 구성할 수 있는데 이때 사용되는 것이 requires expression이다.

// requires expression은 다음과 같이 구성할 수 있다.
//
// 1. requires { requirement-seq }
// 2. requires ( parameter-list ﻿(optional) ) { requirement-seq }
// 
// ex-code)------------------------------------------
// // 인자를 넣는 부분(T1 a, T2 b)는 생략할 수 있다.
// requires (T1 a, T2 b)
// {
//     one or more requirements
// }
// --------------------------------------------------
// 

// https://en.cppreference.com/w/cpp/language/requires#Requirements
// https://en.cppreference.com/w/cpp/language/requires#Note
// requirement 유형은 총 4가지가 있으며 익명 concept을 정의하기 위한 Ad-hoc Constraint도 지원한다.
// 
// 1. Simple Requirements
// 2. Type Requirements
// 3. Compound Requirements
// 4. Nested Requirements
// 5. Ad-hoc Constraints(일회용으로 사용하기 위한 익명 concept을 정의하기 위한 문법으로 requirement에 해당하는 것은 아님)
//

// # Simple Requirements
// 이 유형은 주어진 표현식이 문법적으로 유효한지만 검사할 뿐인 가장 간단한 requirement에 해당한다.
// 쉽게 말해서 표현식이 컴파일될 수 있는지만 검사한다(실행하는 건 아님).
// 
// requires (Args)
// {
//     expression;
// }
//
template <typename T>
concept Arithmetic = requires (T a, T b)
{
    a + b;
    a - b;
    a * b;
    a / b;
};

// 사용자 정의 concept을 축약 함수 템플릿에 적용하는 것도 가능하다.
// void PrintArithmeticValues(Arithmetic auto a, Arithmetic auto b)
// {
//     cout << a + b << ", " << a - b << ", " << a * b << ", " << a / b << "\n";
// }

template <Arithmetic T1, Arithmetic T2>
void PrintArithmeticValues(T1 a, T2 b)
{
    cout << a + b << ", " << a - b << ", " << a * b << ", " << a / b << "\n";
}

template <typename T>
concept GreaterThan = requires (T a, T b)
{
    a > b;
};

struct FooType
{
    bool operator>(const FooType& rhs)
    {
        return true; // 임시 객체니까 무조건 true를 반환하게 함.
    }
};

// auto IsGreater(GreaterThan auto a, GreaterThan auto b)
// {
//     return a > b;
// }

template <GreaterThan T1, GreaterThan T2>
auto IsGreater(T1 a, T2 b)
{
    return a > b;
}

template <typename T>
concept Dereferencable = requires (T a)
{
    *a;
};

template <Dereferencable T>
void PrintDereferencedValue(T t)
{
    cout << *t << "\n";
}

template <typename T>
concept CanGetBeginEndIter = requires (T a)
{
    // 함수를 사용할 수 있는지도 확인할 수 있으며 다수의 expressions를 나열하는 것도 가능하다.
    a.begin();
    a.end();
};

void Run()
{
    PrintArithmeticValues(8, 3.14);

    // 에러 발생
    // PrintArithmeticValues(FooType{ }, FooType{ });

    cout << IsGreater(10, 5) << "\n";

    // 연산자 오버로딩를 정의했기 때문에 에러가 발생하지 않는다.
    cout << IsGreater(FooType{ }, FooType{ }) << "\n";

    //
    shared_ptr<int> ptrA = make_shared<int>(100);
    int* ptrB = ptrA.get();
    
    int val = 200;

    PrintDereferencedValue(ptrA);
    PrintDereferencedValue(ptrB);
    
    // 에러 발생
    // PrintDereferencedValue(val);

    // 다음과 같이 concept을 값으로 쓰면 정의한 표현식이 유효한지 확인할 수 있다(bool 값으로 평가됨).
    cout << "CanGetBeginEndIter<int> : " << CanGetBeginEndIter<int> << "\n";
    cout << "CanGetBeginEndIter<vector<int>> : " << CanGetBeginEndIter<vector<int>> << "\n";
}

END_NS

BEGIN_NS(Case02)

// https://en.cppreference.com/w/cpp/keyword/typename
// https://en.cppreference.com/w/cpp/language/template_specialization
// https://en.cppreference.com/w/cpp/language/type_alias

// # Type Requirements
// 이 유형은 typename과 타입의 이름을 작성하는 방식으로 동작하는 requirement로
// typename 뒤에 오는 타입이 유효한지 검증한다.
// 
// requires (T)
// {
//     typename T::inner_type;
//     typename Specialized<T>;
//     typename Aliased<T>;
// }
//

// 범위 기반 for를 이용하여 순회할 수 있어여 하고 각 요소를 "+"로 더할 수 있는 컨테이너만 허용하는 concept
template <typename T>
concept ElemAddableContainer = requires (T a) {
    typename T::value_type;
    typename T::iterator;
    *(a.begin()) + *(a.begin());
    a.begin();
    a.end();
    
    // https://en.cppreference.com/w/cpp/iterator#Iterator_concepts
    // https://en.cppreference.com/w/cpp/iterator#Algorithm_concepts_and_utilities
    // 실제 iterator를 기반으로 concepts를 적용할 때는 built-in으로 제공하는 것을 쓰는 것이 좋지만 예시 코드니까 이렇게 한 것이다.
};

// 다음 방식은 타입이 auto라 value_type을 가져올 수 없어서 사용할 수 없다.
// template <ElemAddableContainer Container>
// auto Sum(const ElemAddableContainer auto& container)
// {
//     // 키워드를 대상으로 가져오려는 시도는 당연히 불가능하고
//     // typename auto::value_type sum{ };
// 
//     // decltype()을 이용해서 value_type을 가져오는 것도 불가능하다(2025-01-16 기준).
//     typename decltype(container)::value_type sum{ };
// 
//     for (auto& elem : container)
//     {
//         sum = sum + elem;
//     }
// 
//     return sum;
// }

// 어떤 타입을 사용할 것인지를 알고 있어야 value_type을 가져올 수 있다.
template <ElemAddableContainer Container>
auto Sum(const Container& container) -> typename Container::value_type
{
    typename Container::value_type sum{ };

    for (auto& elem : container)
    {
        sum = sum + elem;
    }

    return sum;
}

struct FooType
{
    FooType operator+(const FooType& rhs)
    {
        return FooType{ };
    }

    friend ostream& operator<<(ostream& os, const FooType& rhs)
    {
        os << "FooType";

        return os;
    }
};

void Run()
{
    int arr[]{ 1, 2, 3, 4, 5 };

    cout << Sum(vector<int>{ 1, 2, 3, 4, 5 }) << "\n";
    cout << Sum(list<float>{ 10.f, 20.f, 30.f, 40.f, 50.f }) << "\n";
    cout << Sum(set<double>{ 10.1, 20.2, 30.3, 40.4, 50.5 }) << "\n";

    // 사용자 정의 자료형도 연산자 오버로딩이 되어 있다면 계산할 수 있다.
    cout << Sum(vector{ FooType{ }, FooType{ } }) << "\n";

    // int[]는 concept에 정의한 제약 조건을 충족하지 못 한다.
    // cout << Sum(arr) << "\n";
    
    // 문자열을 대상으로 Sum()을 호출하는 건 약간 이상하다.
    // 이럴 때는 std::integral과 std::floating_point 제약을 추가적으로 묶어서 거는 것이 좋다.
    cout << Sum(vector<string>{ "Hello", "World" }) << "\n";
}

END_NS

BEGIN_NS(Case03)

// https://en.cppreference.com/w/cpp/language/noexcept
// https://en.cppreference.com/w/cpp/language/noexcept_spec

// # Compound Requirements
// 이 유형은 표현식을 중괄호로 감싼 형태의 requirement이다.
// 중괄호로 감싼 식의 결과 타입과 noexcept를 옵션으로 부여하는 것으로 제약을 걸 수 있다.
// 
// !! noexcept와 return-type-requirement는 optional임. !!
// requires (Args)
// {
//     { expression } noexcept -> return-type-requirement;
// }
// 
// noexcept가 붙으면 expression이 예외를 던지지 않는다고 한정지어 noexcept로 정의된 유형만 받게 제약을 거는 것이 가능하다.
// 원문(requires 쪽) : If noexcept is used, expression must not be potentially throwing;
// 원문(noexcept specifier 쪽) : If expression evaluates to true, the function is declared not to throw any exceptions.
//
template <typename T>
concept GreaterThanNoOptional = requires (T a, T b)
{
    // optional에 해당하는 noexcept와 return-type-requirement를 전부 생략하면 Simple Requirements와 사실상 동일하다고 봐야 한다.
    { a > b };
};

template <typename T>
concept GreaterThanNoexcept = requires (T a, T b)
{
    // 반환되는 값은 예외를 던지지 않는 유형으로 되어 있어야 한다.
    { a > b } noexcept;
};

template <typename T>
concept GreaterThanConvertibleToBool = requires (T a, T b)
{
    // operator>()가 꼭 true를 반환하란 법은 없으니 반환되는 값이 bool로 변환될 수 있는 것으로 한정한다.
    { a > b } -> std::convertible_to<bool>;
};

template <typename T>
concept GreaterThanNoexceptConvertibleToBool = requires (T a, T b)
{
    { a > b } noexcept -> std::convertible_to<bool>;
};

struct FooTypeReturnString
{
    // 이상하긴 해도 이렇게 반환형을 구성하는 것도 가능하다.
    string operator>(const FooTypeReturnString& rhs)
    {
        return "Hello World";
    }
};

struct FooTypeNoexceptStringReturn
{
    string operator>(const FooTypeNoexceptStringReturn& rhs) noexcept
    {
        return "Hello World";
    }
};

struct FooTypeBoolReturn
{
    bool operator>(const FooTypeBoolReturn& rhs)
    {
        return true;
    }
};

struct FooTypeNoexceptBoolReturn
{
    bool operator>(const FooTypeNoexceptBoolReturn& rhs) noexcept
    {
        return true;
    }
};

template <GreaterThanNoOptional T>
auto IsGreaterNoOptional(T a, T b)
{
    return a > b;
}

template <GreaterThanNoexcept T>
auto IsGreaterNoexcept(T a, T b)
{
    return a > b;
}

template <GreaterThanConvertibleToBool T>
auto IsGreaterConvertibleToBool(T a, T b)
{
    return a > b;
}

template <GreaterThanNoexceptConvertibleToBool T>
auto IsGreaterNoexceptConvertibleToBool(T a, T b)
{
    return a > b;
}

// 아직은 다루지 않은 유형이지만 이렇게 concept을 구성하기 위한 expression을 이렇게 만들 수도 있다(&&로 묶여도 어차피 bool 값으로 평가 가능).
template <typename T>
concept IntegralReference = std::is_reference_v<T> && std::integral<remove_reference_t<T>>;

template <typename T>
concept DereferencableInteger = requires (T a)
{
    // dereferencing이 가능하고 그 타입은 int&여야 한다는 제약을 건 상태
    // { *a } -> std::same_as<int&>;

    // 이렇게하면 일반적인 integral 유형을 dereferencing 할 수 있다.
    { *a } -> IntegralReference;
};

void PrintDereferencedIntValue(DereferencableInteger auto a)
{
    cout << "PrintDereferencedIntValue(DereferencableInteger auto a) : " << *a << "\n";
}

// FooConcept에서 사용하기 위한 사용자 정의 컨셉(포인터를 받는 유형의 built-in 컨셉이 아직은 없음)
template <typename T>
concept PointerType = std::is_pointer_v<T>;

// 특정 함수를 호출할 수 있는지 그리고 그 함수 호출로 반환되는 타입이 어떤 것인지에 대한 제약도 걸 수 있다.
template <typename T>
concept FooConcept = requires (T a)
{
    { a.GetThis() } -> PointerType;
    { a.FooFuncA() } -> std::same_as<int>;
    { a.FooFuncB() } -> std::convertible_to<int>;
};

class FooObjectA
{
public:
    void* GetThis()
    {
        return this;
    }

    int FooFuncA()
    {
        return 100;
    }

    float FooFuncB()
    {
        return 3.14f; // float는 int로 변환할 수 있음.
    }
};

class FooObjectB
{
public:
    void* GetThis()
    {
        return this;
    }

    float FooFuncA()
    {
        return 100;
    }

    string FooFuncB()
    {
        return "Hello World";
    }
};

void Run()
{
    // 옵션을 지정하지 않으면 다음 4가지 유형을 모두 컴파일할 수 있다.
    cout << "A : " << IsGreaterNoOptional(FooTypeReturnString{ }, FooTypeReturnString{ }) << "\n";
    cout << "A : " << IsGreaterNoOptional(FooTypeNoexceptStringReturn{ }, FooTypeNoexceptStringReturn{ }) << "\n";
    cout << "A : " << IsGreaterNoOptional(FooTypeBoolReturn{ }, FooTypeBoolReturn{ }) << "\n";
    cout << "A : " << IsGreaterNoOptional(FooTypeNoexceptBoolReturn{ }, FooTypeNoexceptBoolReturn{ }) << "\n";

    // noexcept가 옵션으로 지정되어 있으면 예외를 던지지 않는 경우만 컴파일할 수 있다.
    // cout << "B : " << IsGreaterNoexcept(FooTypeReturnString{ }, FooTypeReturnString{ }) << "\n";
    cout << "B : " << IsGreaterNoexcept(FooTypeNoexceptStringReturn{ }, FooTypeNoexceptStringReturn{ }) << "\n";
    // cout << "B : " << IsGreaterNoexcept(FooTypeBoolReturn{ }, FooTypeBoolReturn{ }) << "\n";
    cout << "B : " << IsGreaterNoexcept(FooTypeNoexceptBoolReturn{ }, FooTypeNoexceptBoolReturn{ }) << "\n";

    // 반환형을 bool 값이 될 수 있는 것으로만 한정했으면 문자열을 반환하는 경우는 컴파일할 수 없다.
    // cout << "C : " << IsGreaterConvertibleToBool(FooTypeReturnString{ }, FooTypeReturnString{ }) << "\n";
    // cout << "C : " << IsGreaterConvertibleToBool(FooTypeNoexceptStringReturn{ }, FooTypeNoexceptStringReturn{ }) << "\n";
    cout << "C : " << IsGreaterConvertibleToBool(FooTypeBoolReturn{ }, FooTypeBoolReturn{ }) << "\n";
    cout << "C : " << IsGreaterConvertibleToBool(FooTypeNoexceptBoolReturn{ }, FooTypeNoexceptBoolReturn{ }) << "\n";

    // 현재 제시된 경우의 수 중 noexcept와 반환형이 bool 값이 될 수 있는 경우는 하나 뿐이다.
    // cout << "D : " << IsGreaterNoexceptConvertibleToBool(FooTypeReturnString{ }, FooTypeReturnString{ }) << "\n";
    // cout << "D : " << IsGreaterNoexceptConvertibleToBool(FooTypeNoexceptStringReturn{ }, FooTypeNoexceptStringReturn{ }) << "\n";
    // cout << "D : " << IsGreaterNoexceptConvertibleToBool(FooTypeBoolReturn{ }, FooTypeBoolReturn{ }) << "\n";
    cout << "D : " << IsGreaterNoexceptConvertibleToBool(FooTypeNoexceptBoolReturn{ }, FooTypeNoexceptBoolReturn{ }) << "\n";

    //
    shared_ptr<int>   intPtr   = make_shared<int>(10);
    shared_ptr<float> floatPtr = make_shared<float>(3.14f);

    PrintDereferencedIntValue(intPtr);

    // 에러 발생
    // PrintDereferencedIntValue(floatPtr);

    // 함수 호출 여부와 반환형이 제약에 걸리는지 확인
    cout << "FooConcept<FooObjectA> : " << FooConcept<FooObjectA> << "\n";
    cout << "FooConcept<FooObjectB> : " << FooConcept<FooObjectB> << "\n";
}

END_NS

BEGIN_NS(Case04)

// # Nested Requirements
// 이 유형은 requirements를 나열하는 도중 중첩된 requires를 통해 제약 조건을 거는 형태의 requirement이다.
// 일반적으로 Args로 전달된 매개변수의 타입을 기반으로 세부적인 조건을 판단하는데 사용한다.
// 
// requires (Args)
// {
//     requires constraint-expression;
// }
//

// Sum()에서 사용할 concept 정의
// nested requirement를 이용해 인자는 무조건 2개 이상 받게 제약을 건다.
template <typename... T>
concept AddablePack = requires (T... args)
{
    { (... + args) } noexcept;
    requires sizeof...(T) > 1;
};

// 다음 방식은 Sum(AddablePack T1 x, AddablePack T2 y) 이런 느낌으로 받는 것이라서 인자를 2개 이상 받게 제약을 걸면 사용할 수 없다.
// auto Sum(AddablePack auto... args)
// {
//     // ((std::cout << typeid(args).name() << '\n'), ...);
// 
//     return (... + args);
// }

// 비슷한 이유로 다음 방식도 사용할 수 없다(concept이 적용된 인자를 하나씩 받는 형태).
// template <AddablePack... T>
// auto Sum(T... args)
// {
//     // ((std::cout << typeid(args).name() << '\n'), ...);
//     
//     return (... + args);
// }

// 조금 번거롭더라도 직접 requires를 지정하여 인자를 모두 넘기는 방식을 취해야 한다.
template <typename... T>
    requires AddablePack<T...> // 인자를 전개해서 넘김.
auto Sum(T... args)
{
    return (... + args);
}

// https://en.cppreference.com/w/cpp/language/fold
// 이건 참고용 코드로 fold expression을 활용해서 각 인자가 유효한지 확인하는 것도 가능하다.
template <typename... Args>
    requires AddablePack<Args...> && ((std::integral<Args> || ...) || (std::floating_point<Args> || ...))
auto SumIntFloat(Args... args)
{
    return (... + args);
}

// 컨테이너가 vector이고, 그 vector는 정수형을 기반으로 하는지 검사하는 concept
template <typename T>
concept IntegralVectorConcept = requires {
    requires std::same_as<vector<typename T::value_type>, T>;
    requires std::integral<typename T::value_type>;
};

void Run()
{
    cout << Sum(10, 20) << "\n";

    // 인자 2개를 전달하는 유형이 아니라서 컴파일 에러 발생
    // cout << Sum(3.14) << "\n";

    // "{ (... + args) } noexcept"에 의해 두 인자를 noexcept하게 더하는 기능이 없어서 컴파일 에러 발생
    // cout << Sum("Hello"s, "World"s) << "\n";

    // fold expression을 통해 인자를 전개한 것도 성공한다.
    cout << SumIntFloat('A', 20, 31.415) << "\n";
    
    // 다음 코드에서 "Hello"는 const char*로 처리되며 이 타입은 std::integral이나 std::floating_point로 변환될 수 없기에 컴파일 에러 발생
    // cout << SumIntFloat(100, 3.14, "Hello") << "\n";

    cout << "IntegralVectorConcept<vector<int>> : " << IntegralVectorConcept<vector<int>> << "\n";
    cout << "IntegralVectorConcept<vector<long>> : " << IntegralVectorConcept<vector<long>> << "\n";
    cout << "IntegralVectorConcept<vector<float>> : " << IntegralVectorConcept<vector<float>> << "\n";
    cout << "IntegralVectorConcept<list<int>> : " << IntegralVectorConcept<list<int>> << "\n";
}

END_NS

BEGIN_NS(Case05)

// https://en.cppreference.com/w/cpp/language/requires#Note

// # Ad-hoc Constraints
// 이 유형은 requirements로 분류되는 건 아니지만 유용한 문법이다.
// concept이 들어갈 자리에 이름 없는 concept을 정의하는 방식으로 동작한다.
// 
// template <typename T[, ...]>
//     requires requires (Args) // 익명으로 정의한 concept을 requires로 받음.
//     {
//         requirements
//     }
// [template]
// 
// 익명으로 정의한 concept은 requires clause에 해당하지 않는다.
// 원문 : The keyword requires is used to introduce a requires clause, which specifies constraints on template arguments or on a function declaration.
//
// "requires requires (Args)"에서 첫 번째 requires는 requires clause에 해당하니 혼동하면 안 된다.
//
template <typename T>
    requires requires(T x)
    {
        x + x;
    }
auto Add(T a, T b)
{
    return a + b;
}

struct FooObjectA
{ };

struct FooObjectB
{
    int operator+(const FooObjectB& rhs) // 본래라면 FooObjectB를 반환해야 하는데 테스트 코드이기 때문에 int를 반환하게 함.
    {
        return 100;
    }
};

void Run()
{
    cout << Add(10, 20) << "\n";
    cout << Add(3.14, 1.23) << "\n";

    // 에러 발생
    // cout << Add(FooObjectA{ }, FooObjectA{ }) << "\n";

    // 덧셈에 대한 연산을 정의한 구조체라서 컴파일할 수 있다.
    cout << Add(FooObjectB{ }, FooObjectB{ }) << "\n";
}

END_NS

int main()
{
    cout << "-------------------------#01#-------------------------\n";
    
    Case01::Run();
    
    cout << "-------------------------#02#-------------------------\n";

    Case02::Run();

    cout << "-------------------------#03#-------------------------\n";

    Case03::Run();

    cout << "-------------------------#04#-------------------------\n";

    Case04::Run();

    cout << "-------------------------#05#-------------------------\n"; 

    Case05::Run();

    cout << "------------------------------------------------------\n";

    return 0;
}
