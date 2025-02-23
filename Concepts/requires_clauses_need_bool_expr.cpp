// Update Date : 2025-02-24
// OS : Windows 10 64bit
// Program : Visual Studio 2022, vscode(gcc-14.2.0)
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>

using namespace std;

// 순서대로 볼 것
// 
// # concepts에 대한 기본적인 설명과 사용 방법
// 1. requires_clauses_need_bool_expr.cpp <-----
// 2. built-in_concepts.cpp
// 3. abbreviated_function_templates.cpp
// 4. custom_concepts.cpp
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

// https://en.cppreference.com/w/cpp/language/constraints#Requires_clauses
// https://en.cppreference.com/w/cpp/concepts

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

int main()
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

    return 0;
}
