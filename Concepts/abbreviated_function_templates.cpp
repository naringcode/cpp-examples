// Update Date : 2025-02-24
// OS : Windows 10 64bit
// Program : Visual Studio 2022, vscode(gcc-14.2.0)
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <concepts>

using namespace std;

// 순서대로 볼 것
// 
// # concepts에 대한 기본적인 설명과 사용 방법
// 1. requires_clauses_need_bool_expr.cpp
// 2. built-in_concepts.cpp
// 3. abbreviated_function_templates.cpp <-----
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

int main()
{
    // 동일한 이름의 템플릿 함수를 작성했다고 해도 컴파일 할 때는 컴파일러가 알아서 가장 적절한 유형을 찾아서 코드를 생성한다.
    cout << Max(10, 3.14f) << "\n";
    cout << Max(100, 200) << "\n";

    return 0;
}
