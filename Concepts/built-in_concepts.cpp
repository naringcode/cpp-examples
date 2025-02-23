// Update Date : 2025-02-24
// OS : Windows 10 64bit
// Program : Visual Studio 2022, vscode(gcc-14.2.0)
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <concepts>
#include <array>
#include <vector>

using namespace std;

// 순서대로 볼 것
// 
// # concepts에 대한 기본적인 설명과 사용 방법
// 1. requires_clauses_need_bool_expr.cpp
// 2. built-in_concepts.cpp <-----
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

// C++이 제공하는 built-in concepts은 다음 링크에서 확인할 수 있다.
// https://en.cppreference.com/w/cpp/concepts#Standard_library_concepts
// https://en.cppreference.com/w/cpp/iterator#Iterator_concepts
// https://en.cppreference.com/w/cpp/iterator#Algorithm_concepts_and_utilities
// https://en.cppreference.com/w/cpp/ranges#Range_concepts

// https://en.cppreference.com/w/cpp/language/constraints
// https://en.cppreference.com/w/cpp/concepts

// 템플릿 코드를 생성할 목적으로 사용하는 requires의 평가식으로 "concept"을 적용할 수 있다.
// concept 자체도 코드를 보면 bool로 평가되는 문법이다.
// 
// concept을 사용해 템플릿 코드를 생성하면 concept에 붙은 이름을 기반으로 어떤 타입인지, 어떠한 기능이 담긴 것인지 등을 쉽게 유추할 수 있다.
//
// 사용자 정의 concept을 구성하는 방법도 있지만 <concepts>에 정의된 built-in concepts를 사용하는 것도 한 가지 방법이다.

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

// concepts 자체를 템플릿 인자로 하여 제약을 거는 것도 가능하다.
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

int main()
{
    // 에러 발생(C7062 - constraints not satisfied : 제약 조건이 충족되지 않았을 때 발생하는 에러)
    // 해당 에러는 컴파일러 버전과 조건에 따라서 안 뜨기도 하니 에러가 안 보인다고 혼동하면 안 된다.
    // 2025-01-15 일자 기준 확인한 컴파일러 목록
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

    return 0;
}
