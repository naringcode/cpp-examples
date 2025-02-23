// Update Date : 2025-01-19
// OS : Windows 10 64bit
// Program : Visual Studio 2022, vscode(gcc-14.2.0)
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <functional>
#include <algorithm>
#include <memory>
#include <concepts>
#include <iterator>
#include <array>
#include <vector>
#include <list>
#include <forward_list>
#include <set>
#include <map>

using namespace std;

// 순서대로 볼 것
// 
// # concepts에 대한 기본적인 설명과 사용 방법
// 1. requires_clauses_need_bool_expr.cpp
// 2. built-in_concepts.cpp
// 3. abbreviated_function_templates.cpp
// 4. custom_concepts.cpp
// 5. combining_constraints.cpp
// 6. callable_concepts.cpp
// 
// # 위 항목을 한데 묶어서 정리한 내용
// 7. concepts_details.cpp <-----
//
// # 응용하기
// 8. STL/span.cpp(Case02)
//
// Concepts을 학습한 다음 Ranges를 학습하도록 한다.
// Ranges의 동작 방식을 잘 이해하기 위해선 Concepts에 대한 선행 학습이 필요하다.
//

// C++이 제공하는 built-in concepts은 다음 링크에서 확인할 수 있다.
// https://en.cppreference.com/w/cpp/concepts#Standard_library_concepts
// https://en.cppreference.com/w/cpp/iterator#Iterator_concepts
// https://en.cppreference.com/w/cpp/iterator#Algorithm_concepts_and_utilities
// https://en.cppreference.com/w/cpp/ranges#Range_concepts

// https://en.cppreference.com/w/cpp/language/constraints
// https://en.cppreference.com/w/cpp/concepts
// https://en.cppreference.com/w/cpp/language/requires
// https://en.cppreference.com/w/cpp/language/function_template#Abbreviated_function_template
// https://en.cppreference.com/w/cpp/language/auto#Function_declarations
// https://en.cppreference.com/w/cpp/language/function#Return_type_deduction
// https://en.cppreference.com/w/cpp/keyword/typename
// https://en.cppreference.com/w/cpp/language/template_specialization
// https://en.cppreference.com/w/cpp/language/type_alias
// https://en.cppreference.com/w/cpp/language/noexcept
// https://en.cppreference.com/w/cpp/language/noexcept_spec
// https://en.cppreference.com/w/cpp/language/fold
// https://en.cppreference.com/w/cpp/concepts/boolean-testable
// https://ko.wikipedia.org/wiki/%EC%9D%B4%ED%95%AD_%EA%B4%80%EA%B3%84 | 이항 관계(binary relation)
// https://ko.wikipedia.org/wiki/%EB%8F%99%EC%B9%98_%EA%B4%80%EA%B3%84 | 동치 관계(equivalence relation)
// https://en.cppreference.com/w/cpp/types/enable_if
// https://en.cppreference.com/w/cpp/header/type_traits

#define BEGIN_NS(name) namespace name {
#define END_NS };

BEGIN_NS(Case01)

// C++11 이후부턴 특정 타입을 제한하는 SFINAE의 기능을 이용해 함수의 생성 여부를 제한하는 것이 가능하다.
// !! C++11 이전에는 enable_if가 없어서 직접 구현했어야 함(SFINAE가 불가능했던 건 아님). !!
template <typename F, enable_if_t<is_floating_point_v<F>>* = nullptr>
F DivideA(F a, F b)
{
    return a / b;   
}

// static_assert()를 활용해 에러 메시지를 생성하는 방법도 존재한다.
template <typename F>
F DivideB(F a, F b)
{
    static_assert(is_floating_point_v<F>, "Argument types must be floating points");

    return a / b;
}

// enable_if를 사용하는 유형은 복잡하게 생겨서 이해하기 힘들며 무엇이 잘못되었는지 에러를 발생시키지 않는다.
// static_assert()를 사용하는 방식은 해당 검사 함수가 함수를 호출하기 위해 사용되는 인터페이스에 위치하지 않기 때문에(몸체 내에 작성되어서)
// 코드가 컴파일 타임이 돼서야 에러가 발생하며 또 어디서 호출되어 에러가 발생했는지 파악하기 힘들다는 단점이 있다.

void Run()
{
    // 에러 발생
    // cout << DivideA(5, 2) << "\n";
    // cout << DivideB(5, 2) << "\n";

    // 타입을 제대로 명시해야 한다.
    cout << DivideA(5.0, 2.0) << "\n";
    cout << DivideB(5.0, 2.0) << "\n";
}

END_NS

BEGIN_NS(Case02)

class FooObject
{
public:
    FooObject() = default;
    FooObject(FooObject&&) = delete; // = default;
    FooObject(const FooObject&) = delete;
};

// 이 템플릿 함수를 호출하려고 하면 컴파일 시 C2280(삭제된 함수 참조) 에러가 뜨는데
// 문제는 에러 유형을 알아도 해당 에러가 발생한 위치를 특정하기 힘들다는 단점이 존재한다.
template <typename T>
void FooInsertA(T&& item)
{
    vector<T> vec;

    vec.emplace_back(std::forward<T>(item));
}

// 이러한 생성자가 삭제된 유형을 enable_if로 탐지하는 방법
// !! 일치하는 함수 템플릿의 인스턴스가 없다는 에러가 뜨긴 하는데 이것 자체만으로는 무엇이 잘못되었는지 파악하기 힘듦. !!
template <typename T, enable_if_t<is_move_constructible_v<T>>* = nullptr>
void FooInsertB(T&& item)
{
    vector<T> vec;

    vec.emplace_back(std::forward<T>(item));
}

void Run()
{
    // 에러 발생
    // FooInsertA(FooObject{ });

    // 에러 발생
    // FooInsertB(FooObject{ });
}

END_NS

BEGIN_NS(Case03)

// https://en.cppreference.com/w/cpp/language/constraints#Requires_clauses
// https://en.cppreference.com/w/cpp/concepts
// https://en.cppreference.com/w/cpp/language/requires

// C++20에 도입된 Concepts를 사용하면 템플릿에 제약 조건을 부여할 수 있다.
// Concepts를 활용하면 복잡한 enable_if나 static_assert()를 사용하지 않아도
// Concepts에 적용된 적절한 구문을 보거나 컴파일러가 출력하는 에러 메시지를 통해서 무엇이 잘못되었는지 판단할 수 있다(코드 가독성 증가).
// !! 다만 이걸 제대로 이해하기 위해서는 먼저 constraints(제약)란 개념을 파악해야 한다. !!
//
// (중요) 제약 자체는 컴파일 타임에 평가된다.
//
// concepts 기능이 없던 C++20 이전에는 enable_if + type_traits을 적용한 SFINAE를 통해 조건부 템플릿을 구현했다.

// requires는 제약을 걸기 위해 사용되는 필수 문법이며 다음 두 곳에서 사용되는 키워드이다.
// - concept을 정의하기 위해 requirements을 구성할 목적으로 사용하는 requires
// - 템플릿 코드를 생성할 목적으로 제약을 걸기 위한 requires clauses를 구성할 목적으로 사용하는 requires
// 
// 원문(Constraints and concepts) : Named sets of such requirements are called concepts.
//   - requirements는 Requires expression로 리다이렉트됨(https://en.cppreference.com/w/cpp/language/requires).
// 원문(requires clauses) : The keyword requires is used to introduce a requires clause, which specifies constraints on template arguments or on a function declaration.
//

// 다음 4가지 문법을 통해 requires를 사용하여 템플릿에 제약을 걸 수 있다.
//
// 1.
// template <typename T[, ...]>
//     requires expression
// [template]
// 
// 2.
// template <typename T[, ...]>
// ReturnType FunctionTemplate(T t[, ...]) requires expresssion
// {
//     ...
// }
// 
// 3.
// template <concept T>
// [template]
// 
// 4.
// ReturnType TemplateFunction(concept auto arg[, ...])
// {
//     ...
// }
//

// 다음 코드는 템플릿 코드를 생성할 목적으로 제약을 거는 requires 예시 코드이다.
constexpr bool kIsPrimaryTrue  = true;
constexpr bool kIsPrimaryFalse = false;

consteval bool IsFunctionalTrue() { return true; }

// 템플릿에 제약을 걸 때 사용하는 requires는 컴파일 타임에 bool 값으로 평가될 수는 "expression"이어야 한다.
template <typename T>
    requires kIsPrimaryTrue
void FooPrintTrueA(T t)
{
    cout << "A - " << typeid(t).name() << " : " << t << "\n";
}

template <typename T>
void FooPrintTrueB(T t) requires kIsPrimaryTrue // 함수 템플릿에 한정해서 requires를 뒤에 붙일 수 있음.
{
    cout << "B - " << typeid(t).name() << " : " << t << "\n";
}

// https://en.cppreference.com/w/cpp/language/expressions#Primary_expressions
// https://learn.microsoft.com/ko-kr/cpp/cpp/primary-expressions?view=msvc-170
// 아래 유형에서 requires에 적용된 것은 기본식(primary expression)으로 평가되지 않는다.
// 이건 컴파일러 설계와 관련이 있는 부분이기에 컴파일러 개론 서적을 보도록 한다.
// !! is_functional_true을 찾아서 인자를 넣지 않고 call하는 것이며 이는 postfix expression에 해당함. !!
// template <typename T>
// void FooPrintTrueC(T t) requires IsFunctionalTrue()
// {
//     cout << "C - " << typeid(t).name() << " : " << t << "\n";
// }

// 괄호로 묶인 표현식은 primary expression으로 간주하기 때문에 이런 방식으로 우회하면 된다.
template <typename T>
void FooPrintTrueD(T t) requires (IsFunctionalTrue())
{
    cout << "D - " << typeid(t).name() << " : " << t << "\n";
}

// 아래 코드는 requires에 적용된 평가값이 무조건 false로 나오기 때문에 코드 생성을 하는 것은 불가능하다.
template <typename T>
void FooPrintFalse(T t) requires kIsPrimaryFalse
{
    cout << "False - " << typeid(t).name() << " : " << t << "\n";
}

// requires에 type_traits를 적용하는 것도 가능하며 이는 enable_if를 사용한 것과 유사하다.
template <typename T>
void FooPrintIntegral(T t)
    requires is_integral_v<T>
{
    cout << "Integral - " << typeid(t).name() << " : " << t << "\n";
}

void Run()
{
    FooPrintTrueA(100);
    FooPrintTrueB(3.14);

    // requires의 제약을 걸기 위한 평가식이 primary expression이 아니기 때문에 사용조차 못 한다.
    // FooPrintTrueC(6.28f);

    FooPrintTrueD("Hello");

    // requires에 사용되는 평가식이 무조건 false로 나오기 때문에 에러가 발생한다.
    // FooPrintFalse("World");

    FooPrintIntegral(200);
    FooPrintIntegral(300ull);

    // 해당 유형은 integral 유형이 아니기 때문에 컴파일하는 것이 불가능하다.
    // FooPrintIntegral(3.14f);
}

END_NS

BEGIN_NS(Case04)

// https://en.cppreference.com/w/cpp/concepts

// 템플릿 코드를 생성할 목적으로 사용하는 requires의 평가식으로 "concept"을 적용할 수 있다.
// concept 자체도 코드를 보면 bool로 평가되는 문법이다.
// 
// concept을 사용해 템플릿 코드를 생성하면 concept에 붙은 이름을 기반으로 어떤 타입인지, 어떠한 기능이 담긴 것인지 등을 쉽게 유추할 수 있다.
//
// 사용자 정의 concept을 구성하는 방법도 있지만 <concepts>에 정의된 built-in concepts를 사용하는 것도 한 가지 방법이다.

// 코드를 생성할 목적으로 사용하는 requires에 concept를 적용하는 방식도 결국은 컴파일 타임에 bool 값으로 평가되는 "expression"이다.
template <typename F>
    // requires is_floating_point_v<F>
    requires floating_point<F> // floating_point<T>는 <type_traits> 헤더에 정의된 is_floating_point_v<T>와 동일한 built-in concept.
F DivideA(F a, F b)
{
    return a / b;
}

// requires는 템플릿 함수에 한정하여 뒤에 붙을 수 있다.
template <typename F>
F DivideB(F a, F b) requires floating_point<F>
{
    return a / b;
}

// concepts 자체를 템플릿 인자로 설정하는 것도 가능하다.
// 위에 있는 DivideA(), DivideB() 방식처럼 requires를 사용하여 템플릿에 제약을 거는 경우라면 bool로 평가될 수 있는 expression이어야 한다.
// 하지만 템플릿 인자로 전달하는 이 방식의 경우 해당 인자는 반드시 concept로 정의되어 있어야 한다(floating_point는 built-in concecpt).
// !! 여러 타입의 concepts를 템플릿 인자로 전달하는 경우에 유용하게 사용할 수 있음. !!
template <floating_point F>
F DivideC(F a, F b)
{
    return a / b;
}

class FooObject
{
public:
    FooObject() = default;
    FooObject(FooObject&&) = delete; // = default;
    FooObject(const FooObject&) = delete;
};

// 자료형의 특정한 생성자 존재 여부도 requires로 적용할 수 있다.
template <typename T>
    // requires is_move_constructible_v<T>
    requires move_constructible<T> // 이것 또한 built-in concept
void FooInsertA(T&& item) // requires move_constructible<T> // 이것도 뒤에 붙일 수 있음.
{
    vector<T> vec;

    vec.emplace_back(std::forward<T>(item));
}

// move_constructible도 built-in concept이기 때문에 템플릿 인자로 전달해서 쓸 수 있다.
template <move_constructible T>
void FooInsertB(T&& item)
{
    vector<T> vec;

    vec.emplace_back(std::forward<T>(item));
}

// 다음과 같이 템플릿 클래스를 구성할 때도 requires를 적용할 수 있다.
// !! 이름 뒤에 requires를 붙이는 Trailing 방식은 함수에만 허용되기 때문에 여기선 사용할 수 없음. !!
template <typename T, size_t size>
    requires std::integral<T>
class IntArrayA
{
public:
    array<T, size> _arr;
};

// 템플릿 클래스의 템플릿 인자를 concept으로 지정하는 것도 가능하다.
template <integral T, size_t size>
class IntArrayB
{
public:
    array<T, size> _arr;
};

void Run()
{
    // 에러 발생(C7062 - constraints not satisfied : 제약 조건이 충족되지 않았을 때 발생하는 에러)
    // 해당 에러는 컴파일러 버전과 조건에 따라서 안 뜨기도 하니 에러가 안 보인다고 혼동하면 안 된다.
    // 2015-01-15 일자 기준 확인한 컴파일러 목록
    // - Visual Studio 2022 : C++20 버전, 최신 C++ 초안의 기능(/std:c++latest) 다 확인해봤는데 에러가 안 뜸.
    // - Visual Studio 2019 : C++20 버전에서는 안 뜨고, 최신 C++ 초안의 기능(/std:c++latest)에서는 에러가 뜸.
    // - https://godbolt.org/ + gcc-14.2(-std=c++20 option)
    //   - template argument deduction/substitution failed: constraints not satisfied
    //   - 온라인 컴파일러로 확인했을 때 에러가 위 에러 메시지가 발생함.
    // cout << DivideA(5, 2) << "\n";

    // 타입을 제대로 명시해야 한다.
    cout << DivideA(5.0, 2.0) << "\n";

    // 에러 발생(컴파일러에 따라서 상세 메시지를 안 알려주기도 하니까 주의할 것)
    // cout << DivideB(5, 2) << "\n";

    // 타입을 제대로 명시해야 한다.
    cout << DivideB(5.0, 2.0) << "\n";

    // 에러 발생(컴파일러에 따라서 상세 메시지를 안 알려주기도 하니까 주의할 것)
    // cout << DivideC(5, 2) << "\n";

    // 타입을 제대로 명시해야 한다.
    cout << DivideC(5.0, 2.0) << "\n";

    // 에러 발생(컴파일러에 따라서 상세 메시지를 안 알려주기도 하니까 주의할 것)
    // FooInsertA(FooObject{ });

    // 에러 발생(컴파일러에 따라서 상세 메시지를 안 알려주기도 하니까 주의할 것)
    // FooInsertB(FooObject{ });

    // 에러 발생
    // IntArrayA<float, 10> fArrA;

    // 오직 integral 유형만 컴파일 통과 가능
    IntArrayA<int, 10> iArrA;

    // 에러 발생
    // IntArrayB<float, 10> fArrB;

    // 오직 integral 유형만 컴파일 통과 가능
    IntArrayB<int, 10> iArrB;
}

END_NS

BEGIN_NS(Case05)

// https://en.cppreference.com/w/cpp/language/function_template#Abbreviated_function_template
// https://en.cppreference.com/w/cpp/language/auto#Function_declarations
// https://en.cppreference.com/w/cpp/language/function#Return_type_deduction

// C++20부터는 abbreviated function templates(축약 함수 템플릿)을 활용하여 함수 매개변수를 auto로 작성하는 것이 가능하다.
// !! 코드는 auto로 작성하지만 컴파일 과정에서 template으로 처리됨. !!

// 아래 있는 Max(auto, auto)는 다음과 같은 template 문법으로 처리된다.
// 
// template <typename T1, typename T2>
// auto Max(T1 a, T2 b) // 반환형을 auto로 하는 건 C++11부터 지원하지만 trailing을 붙이지 않아도 되게 개선된 건 C++14 이후부터임.
// {
//     return a > b ? a : b;
// }
//
auto Max(auto a, auto b) // 인자에 제약이 없는 축약 함수 템플릿
{
    cout << "Call the auto Max(auto a, auto b)\n";

    return a > b ? a : b;
}

// 축약 함수 템플릿을 사용할 때 auto 매개변수 앞에 concept을 붙여서 제약을 걸 수 있으며 다음 코드는 동일하다.
// 
// template <integral T1, integral T2>
// auto Max(T1 auto a, T2 auto b)
// {
//     return a > b ? a : b;
// }
//
auto Max(std::integral auto a, std::integral auto b)
{
    cout << "Call the auto Max(std::integral auto a, std::integral auto b)\n";

    return a > b ? a : b;
}

void Run()
{
    // 동일한 이름의 템플릿 함수를 작성했다고 해도 컴파일 할 때는 컴파일러가 알아서 가장 적절한 유형을 찾아서 코드를 생성한다.
    cout << Max(10, 3.14f) << "\n";
    cout << Max(100, 200) << "\n";
}

END_NS

BEGIN_NS(Case06)

// https://en.cppreference.com/w/cpp/language/requires

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

BEGIN_NS(Case07)

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
typename Container::value_type Sum(const Container& container)
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

BEGIN_NS(Case08)

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

BEGIN_NS(Case09)

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

BEGIN_NS(Case10)


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

BEGIN_NS(Case11)

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

void Run()
{
    PrintPointerInfo(static_cast<void*>(nullptr));

    // 인자로 넘기는 값이 포인터 유형이 아니기 때문에 컴파일 에러 발생
    // PrintPointerInfo(100);

    PrintSmallIntInfo('A');
    PrintSmallIntInfo(short{ 10 });
#ifdef _MSC_VER
    PrintSmallIntInfo(unsigned short{ 20 });
#elif defined(__GNUC__)
    PrintSmallIntInfo((unsigned short){ 20 });
#endif

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
}

END_NS

BEGIN_NS(Case12)

// https://en.cppreference.com/w/cpp/concepts#Callable_concepts
// https://en.cppreference.com/w/cpp/concepts#Equality_preservation
// https://en.cppreference.com/w/cpp/iterator#Iterator_concepts
// https://en.cppreference.com/w/cpp/iterator#Algorithm_concepts_and_utilities
// https://ko.wikipedia.org/wiki/%EC%9D%B4%ED%95%AD_%EA%B4%80%EA%B3%84 | 이항 관계(binary relation)
// https://ko.wikipedia.org/wiki/%EB%8F%99%EC%B9%98_%EA%B4%80%EA%B3%84 | 동치 관계(equivalence relation)

// C++20에 도입된 concepts을 보면 built-in concepts 유형에는 Callable Concepts라는 것이 있다.
// 해당 유형의 concepts는 호출 가능 여부를 조회할 때 사용하면 좋다.

// https://en.cppreference.com/w/cpp/concepts/invocable
// # std::invocable & std::regular_invocable
// 
// template< class F, class... Args >
// concept invocable =
//     requires(F&& f, Args&&... args) {
//         std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
//             /* not required to be equality-preserving */
//     };
// 
// std::invocable은 callable 타입인 F가 Args...에 담긴 인자를 토대로 std::invoke()를 호출할 수 있는지 판단하기 위한 concept이다.
// 말 그대로 안정성은 고려하지 않고 호출될 수만 있다면 통과시키기 위한 목적으로 사용하는 concept이다.
// 
// --------------------------------------------------
// 
// template< class F, class... Args >
// concept regular_invocable = std::invocable<F, Args...>;
// 
// https://en.cppreference.com/w/cpp/concepts#Equality_preservation
// std::regular_invocable는 std::invocable을 그대로 받아 기능적으로는 동일하지만 Equality preservation의 의미론적(semantics)인 구분을 위해 따로 정의한 것이다.
// 
// Equality preservation(동등성 보존) : 동일한 입력에 대해 출력되는 결과가 항상 같아야 하고 어떠한 side-effects도 발생시키지 않아야 하며 연산이 동등성 관계를 깨지 않고 보존해야 함.
// 
// std::regular_invocable가 적용되면 전달된 callable과 인자의 원본이 수정되어선 안 된다.
// 원문 : The regular_invocable concept adds to the invocable concept by requiring the invoke expression to be equality-preserving and not modify either the function object or the arguments.
// 원문 : Note that regular_invocable requires the invocation to not modify either the callable object or the arguments and be equality-preserving.
// 
// 쉽게 생각해서 std::regular_invocable이라는 제약이 걸리면 동일 인수로 여러 번 호출해도 결과는 항상 같아야 한다.
// 내부적으로 인수를 대상으로 혹은 전역적으로 side-effects를 발생시켜 상태나 결과가 달라지면 Equality preservation의 성질을 깨지니 주의해야 한다.
// 원문 : A random number generator may satisfy invocable but cannot satisfy regular_invocable
// 
// std::invocable과 std::regular_invocable의 의미론적(semantics)인 내용은 다르지만 실제 동작은 같다.
// 둘의 차이를 판단하는 건 프로그래머의 몫이며 컴파일러는 이를 검증하지 않는다(2025-01-17 기준이며 추후 바뀔 수 있음).
// C++20에 추가된 기능을 보면 std::invocable을 요구하는 경우도 있고 std::regular_invocable을 요구하는 경우도 있으니 적절하게 요구사항에 맞게 코드를 작성하는 것이 중요하다.
// 

// https://en.cppreference.com/w/cpp/concepts/predicate
// # std::predicate
// 
// template< class F, class... Args >
// concept predicate =
//     std::regular_invocable<F, Args...> &&
//     boolean-testable<std::invoke_result_t<F, Args...>>;
// 
// std::predicate는 Equality preservation(동등성 보존)을 만족하면서 callable이 반환형이 boolean-testable을 만족하는지 확인하기 위한 concept이다.
// 
// --------------------------------------------------
// 
// https://en.cppreference.com/w/cpp/concepts/boolean-testable
// 
// template< class B >
// concept __boolean_testable_impl = std::convertible_to<B, bool>;
// 
// template< class B >
// concept boolean-testable =
//     __boolean_testable_impl<B> &&
//     requires (B&& b) {
//         { !std::forward<B>(b) } -> __boolean_testable_impl;
//     };
// 
// boolean-testable은 반환되는 값이 참(true)와 거짓(false)으로 사용될 수 있는지를 판단하기 위한 concept이다.
// 반환되는 값은 암시적으로 bool 타입으로의 변환이 가능해야 한다(반드시 반환되는 값이 bool 형일 필요는 없음).
// 
// 반환 값이 if 문이나 반복문 등에서 조건 판정을 위한 논리 연산에 사용될 수 있는지 검증하기 위한 concept이라고 보면 된다.
// 원문 : for which the logical operators have the usual behavior (including short-circuiting), even for two different boolean-testable types.
// 
// 또한 boolean-testable는 B가 decltype((e))인 식 e가 주어졌으면 bool(e) == !bool(!e)로 모델링된다.
// 원문 : Additionally, given an expression e such that decltype((e)) is B, boolean-testable is modeled only if bool(e) == !bool(!e).
// 
// https://en.wikipedia.org/wiki/Predicate_(mathematical_logic)
// https://en.wikipedia.org/wiki/Predication_(computer_architecture)
// https://stackoverflow.com/questions/3230944/what-does-predicate-mean-in-the-context-of-computer-science
// https://ko.wikipedia.org/wiki/%EC%88%A0%EC%96%B4
// https://www.ldoceonline.com/dictionary/predicate
// C++ 공식 사양을 정리한 문서를 보면 반환되는 값을 암묵적으로 bool 형식으로 변환 가능한 concept을 predicate라고 간주하고 있다.
// 하지만 컴퓨터 과학에서 말하는 predicate와 다른 프로그래밍에서 지원하는 predicate를 보면 C++의 predicate와는 약간 차이가 있다.
// 일반적으로 컴퓨터 쪽의 predicate는 "개는 동물이다"와 같이 true/false로 판정 가능한 결과를 반환하는 서브루틴을 말한다.
// 
// !! 정리 !!
// std::predicate를 bool이 아닌 암묵적으로 bool 형으로 변환 가능한 다른 타입을 반환하는 Callable에 적용하는 것도 가능하다.
// 하지만 의도 전달과 직관성을 위해서라도 std::predicate로 조건 대상이 되는 Callable은 특별한 이유가 없다면 bool 형을 반환하는 것이 좋다.
// 

// https://en.cppreference.com/w/cpp/concepts/relation
// # std::relation
// 
// template< class R, class T, class U >
// concept relation =
//     std::predicate<R, T, T> && std::predicate<R, U, U> &&
//     std::predicate<R, T, U> && std::predicate<R, U, T>;
// 
// https://ko.wikipedia.org/wiki/%EC%9D%B4%ED%95%AD_%EA%B4%80%EA%B3%84 | 이항 관계(binary relation)
// std::relation은 주어진 T, U를 대상으로 callable R을 적용했을 때 이항 관계를 만족하는지 확인하기 위한 concept이다.
// 
// 이항 관계가 되기 위한 중요한 4가지 요소는 다음과 같다.
// - 반사관계(reflexive relation) : 모든 x ∈ A에 대하여, x ~ x인 관계.
// - 대칭관계(symmetric relation) : 모든 x, y ∈ A에 대하여, x ~ y이면 y ~ x인 관계.
// - 추이관계(transitive relation) : 모든 x, y, z ∈ A에 대하여, x ~ y이고 y ~ z이면 x ~ z인 관계.
// - 반대칭관계(antisymmetric relation) : 모든 x, y ∈ A에 대하여, x ~ y이고 y ~ x이면 x = y인 관계.
//

// https://en.cppreference.com/w/cpp/concepts/equivalence_relation
// # std::equivalence_relation
// 
// template< class R, class T, class U >
// concept equivalence_relation = std::relation<R, T, U>;
// 
// https://ko.wikipedia.org/wiki/%EB%8F%99%EC%B9%98_%EA%B4%80%EA%B3%84 | 동치 관계(equivalence relation)
// std::equivalence_relation은 이름 그대로 동치 관계를 파악하기 위한 concept이다.
// 
// std::invocable과 std::regular_invocable의 관계와 유사하게 std::relation과 std::equivalence_relation의 의미론적인 내용은 다르지만 실제 동작은 같다.
// 마찬가지로 이를 구분하는 건 프로그래머의 역할이다.
// 
// 동치 관계는 이항 관계로 주어진 R이 반사적, 대칭적, 추이적이라는 논리적 관계를 만족시킬 때 성립하는 관계를 말한다.
// 두 객체가 완전히 같은 건 아니지만 관점(여기서는 R)에 따라 성질이 동일하다면 이는 동치 관계라고 볼 수 있다.
// 

// https://en.cppreference.com/w/cpp/concepts/strict_weak_order
// # std::strict_weak_order
// 
// template< class R, class T, class U >
// concept strict_weak_order = std::relation<R, T, U>;
// 
// std::relation과 std::strict_weak_order의 실제 동작은 같지만 마찬가지로 의미론적인 내용이 다르다.
// 
// A relation r is a strict weak ordering if
// - it is irreflexive: for all x, r(x, x) is false; <----- 비반사성
// - it is transitive: for all a, b and c, if r(a, b) and r(b, c) are both true then r(a, c) is true; <----- 추이성
// - let e(a, b) be !r(a, b) && !r(b, a), then e is transitive: e(a, b) && e(b, c) implies e(a, c).
//   - let e(a, b) be !r(a, b) && !r(b, a) <----- e(a, b)에서 a와 b를 r에 적용했을 때 동치 관계로 표현될 경우
//   - then e is transitive: e(a, b) && e(b, c) implies e(a, c). <----- 그럼 추이성이 따라 e(a, b) && e(b, c)는 e(a, c)로 표현 가능함.
// 
// 이러한 조건 하에 e는 동치 관계라고 볼 수 있으며, r은 e에 의해 결정된 동등 클래스에 대해 strict total ordering을 유도한다.
// 원문 : Under these conditions, it can be shown that e is an equivalence relation, and r induces a strict total ordering on the equivalence classes determined by e.
//

// (정리) C++20은 다양한 Callable concepts를 제공하지만 실제로 개발할 때는 std::invocable, std::regular_invocable, std::predicate 정도면 충분하다.
// - std::invocable : 호출 가능성만 판단하고 싶을 때
// - std::regular_invocable : 동일 입력에 대해 동일 출력을 보장하는 호출임을 명시하고 싶을 때
// - std::predicate : callable이 반환하는 값이 bool로 변환한 것임을 명시하고 싶을 때
// 
// std::relation, std::equivalence_relation, std::strict_weak_order는 구체적인 조건으로 인해 범용성이 제한되어 있기 때문에
// C++ 차원에서 관리하는 라이브러리라면 몰라도 일반적인 개발 상황에서 사용할 일은 드물다고 봐야 한다.

vector<int> MakeVectorFilledWithRandomNumbers(int cnt)
{
    vector<int> retVec(cnt);

    for (int i = 0; i < cnt; i++)
    {
        retVec[i] = rand() % 100;
    }
    
    return retVec;
}

template <typename... Args>
    requires (std::integral<Args> || ...) || (std::floating_point<Args> || ...)
auto Sum(Args... args)
{
    return (args + ...);
}

template <typename Callable, typename... Args>
    requires std::invocable<Callable, Args...>
auto InvokeWrapper(Callable&& callable, Args&&... args)
{
    return std::invoke(std::forward<Callable>(callable), std::forward<Args>(args)...);
}

template <typename Callable, typename... Args>
    requires std::regular_invocable<Callable, Args...>
auto RegularInvokeWrapper(Callable&& callable, Args&&... args)
{
    return std::invoke(std::forward<Callable>(callable), std::forward<Args>(args)...);
}

// 아래 출력 함수에서 사용하기 위한 concept
template <typename Container>
concept HasPairIter = 
    std::same_as<std::remove_cvref_t<std::iter_value_t<Container>>,
                 std::pair<typename std::iter_value_t<Container>::first_type,
                           typename std::iter_value_t<Container>::second_type>>;

// 가급적이면 아래 코드로 정의한 concept 말고 위 방식으로 정의한 concept를 쓰도록 하자.
// 기능적으로는 비슷할지 몰라도 위 방식으로 정의한 concept이 더 안정적으로 std::pair가 있는지 검증한다.
// !! 기능적으로 보면 동일한 것 같은데 아래 방식으로 concept을 사용하면 에러 위치가 깔끔하게 잡히지 않음. !!
// template <typename Container>
// concept HasPairIter = 
//     std::same_as<std::remove_cvref_t<typename Container::iterator::value_type>,
//                  std::pair<typename Container::iterator::value_type::first_type,
//                            typename Container::iterator::value_type::second_type>>;

// https://en.cppreference.com/w/cpp/iterator/input_iterator
// std::input_iterator는 iterator concepts 중 하나로 사용하고자 하는 iterator는 다음 조건을 충족해야 한다(자세한 건 공식 문서에 있는 예제를 참고).
// - operator*() : 읽을 수 있어야 함.
// - operator++(), operator++(int) : 단방향 접근이 가능해야 함.
//
// C++20에 정의된 iterator concepts는 C++20에 추가된 Ranges와 연관성이 매우 높다.
template <typename Container>
    requires std::input_iterator<typename Container::iterator> && (!HasPairIter<typename Container::iterator>) 
    // !HasPairIter는 괄호를 붙여서 완성된 primary expression으로 표현해야 함.(https://en.cppreference.com/w/cpp/language/constraints#Requires_clauses).
    // C++ 표준에서 괄호가 있는 표현식은 primary expression으로 간주함.
void PrintContainerElems(const Container& container)
{
    cout << "Elems : ";

    for (auto& elem : container)
    {
        cout << elem << ' ';
    }

    cout << "\n";
}

// 가변 인자 템플릿의 각 단일 요소를 평상으로 Callable을 호출하는 것의 유효성을 컴파일 타임에 검증하는 방법
// AllOf()와 OneOf()에사 평가하기 위해 사용되는 Callable은 bool 타입을 반환하는 것이 좋기 때문에 std::predicate로 제약을 걸었다.
template <typename Callable, typename... Args>
    requires (std::predicate<Callable, Args> && ...)
bool AllOf(Callable callable, Args... args)
{
    return (callable(args) && ...);
}

template <typename Callable, typename... Args>
    requires (std::predicate<Callable, Args> && ...)
bool OneOf(Callable callable, Args... args)
{
    return (callable(args) || ...);
}

bool IsEven(int x)
{
    return x % 2 == 0;
}

string IsOdd(int x)
{
    return x % 2 == 1 ? "TRUE" : "FALSE";
}

// Callable Concepts을 사용자 정의 concept에 묶어서 사용하는 방법
template <typename Pred, typename InputIterator>
concept InputIteratableWithPred =
    std::predicate<Pred, typename InputIterator::value_type> &&
    std::input_iterator<InputIterator>;

template <typename Pred, typename InputIterator>
    requires InputIteratableWithPred<Pred, InputIterator>
InputIterator FindIf(InputIterator beginIter, InputIterator endIter, Pred pred)
{
    while (beginIter != endIter)
    {
        if (pred(*beginIter))
            break;

        ++beginIter;
    }

    return beginIter;
}

void Run()
{
    auto containerA = InvokeWrapper(MakeVectorFilledWithRandomNumbers, 5);
    PrintContainerElems(containerA);

    // 아래 코드도 제대로 동작하긴 하지만 문제는 std::regular_invocable 기반으로 작성된 템플릿 함수를 호출하고 있다.
    // MakeVectorFilledWithRandomNumbers()는 동일한 입력이 들어왔어도 출력되는 결과가 같아서 Equality preservation(동등성 보존)을 깬다.
    // std::invocable과 std::regular_invocable는 그냥 의미론적인 내용일 뿐 실제 동작하는 방식은 동일하기 때문에 이는 프로그래머가 주의해야 하는 사항이다.
    auto containerB = RegularInvokeWrapper(MakeVectorFilledWithRandomNumbers, 5);
    PrintContainerElems(containerB);
    
    // 단방향으로 접근 가능하며 읽을 수 있는 iterator를 지원하기에 컴파일 성공
    PrintContainerElems(std::set{ 10, 20, 30 });
    PrintContainerElems(std::list{ 40, 50, 60 });
    PrintContainerElems(std::forward_list{ 40, 50, 60 });

    // 아래 방식은 !HasPairIter 제약을 만족하지 못 하기 때문에 에러가 발생한다.
    // PrintContainerElems(std::map<string, int>{ { "Hello", 1 }, { "World", 2 }, { "Hello World", 60 } });
    
    // TEMP : HasPairIter를 구성하는 과정에서 타입을 확인한 코드
    // using typeA = std::map<string, int>::iterator;
    // using typeB = std::iter_value_t<std::map<string, int>>;
    // using typeC = std::iter_value_t<std::map<string, int>::iterator>;
    // using typeD = std::iter_value_t<volatile std::map<string, int>::const_iterator>;
    // using typeE = std::iter_value_t<const std::map<string, const int>>;
    // using typeF = std::iter_value_t<std::map<string, int>::iterator>;
    // using typeG = std::remove_cvref_t<std::iter_value_t<std::map<string, const int>>>;
    // using typeH = std::remove_cvref_t<std::iter_value_t<std::map<string, int>::const_iterator>>;
    // using typeI = std::map<string, int>::iterator::value_type::first_type;
    // using typeJ = std::map<string, int>::iterator::value_type::second_type;
    // using TestContainer = std::map<string, int>;
    // bool typeChk = std::same_as<std::remove_cvref_t<std::iter_value_t<TestContainer>>,
    //                             std::pair<typename TestContainer::iterator::value_type::first_type,
    //                                       typename TestContainer::iterator::value_type::second_type>>;

    // 다음 코드는 컴파일러가 Sum의 인자를 추론할 수 있느냐 없느냐에 따라서 컴파일이 성공할 수도 있고 실패할 수도 있다.
    // cout << InvokeWrapper(Sum, 100, 200, 300) << "\n";

    // 언어가 중첩 템플릿 함수 추론을 지원한다고 명시되어 있지 않기 때문에
    // 중첩 템플릿 함수의 인자 타입을 하나하나 지정해줘야 제대로 컴파일 할 수 있다.
    cout << InvokeWrapper(Sum<int, int, int>, 100, 200, 300) << "\n";

    // Sum 함수 자체는 전달한 인자에 변형을 가하지도 않고 매번 호출할 때마다 같은 결과를 반환하기 때문에
    // std::regular_invocable로 제약을 건 템플릿 함수를 사용해도 괜찮다.
    cout << RegularInvokeWrapper(Sum<int, int, int>, 100, 200, 300) << "\n";

    // 다음 코드는 단일 인자를 받는 Callable을 통해 가변 인자 템플릿의 각 인자를 검증하기 위한 코드이다.
    cout << AllOf(IsEven, 10, 20, 30) << "\n";
    cout << AllOf([](int elem) { return elem % 2 == 0; }, 10, 20, 30, 15) << "\n";

    // 아래 코드는 IsOdd()가 string을 반환하는데 이는 bool 타입으로 암묵적인 변환이 불가능해서 컴파일 에러가 발생한다.
    // cout << OneOf(IsOdd, 10, 20, 30) << "\n";

    cout << OneOf([](int elem) { return elem % 2 == 1; }, 10, 20, 30) << "\n";

    // Functor 또한 Callable의 한 유형이기 때문에 
    struct OddFunctor
    {
        bool operator()(int elem)
        {
            return elem % 2 == 1;
        }
    };

    cout << OneOf(OddFunctor{ }, 10, 20, 30, 15) << "\n";

    //
    vector<string> strVec{ "Hello", "World", "Foo" };

    auto findIter = FindIf(strVec.begin(), strVec.end(), [ch = 'W'](string str) { return str[0] == ch; });

    cout << *findIter << "\n";
}

END_NS

int main()
{
    cout << "-------------------------#01#-------------------------\n";

    // Case01::Run();
    
    cout << "-------------------------#02#-------------------------\n";

    // Case02::Run();
     
    cout << "-------------------------#03#-------------------------\n";

    // Case03::Run();
    
    cout << "-------------------------#04#-------------------------\n";

    // Case04::Run();
    
    cout << "-------------------------#05#-------------------------\n";

    // Case05::Run();
    
    cout << "-------------------------#06#-------------------------\n";

    // Case06::Run();
    
    cout << "-------------------------#07#-------------------------\n";

    Case07::Run();
    
    cout << "-------------------------#08#-------------------------\n";

    // Case08::Run();
    
    cout << "-------------------------#09#-------------------------\n";

    // Case09::Run();

    cout << "-------------------------#10#-------------------------\n";

    // Case10::Run();

    cout << "-------------------------#11#-------------------------\n";

    // Case11::Run();

    cout << "-------------------------#12#-------------------------\n";

    Case12::Run();

    cout << "------------------------------------------------------\n";

    return 0;
}
