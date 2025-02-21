// Update Date : 2025-02-22
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <type_traits> // std::is_constant_evaluated()

// https://en.cppreference.com/w/cpp/types/is_constant_evaluated

// std::is_constant_evaluated()는 C++20 이후부터 제공되는 함수이다.
// 이 함수를 쓰면 평가가 런타임에 이루어지는지 아니면 컴파일 타임에 이루어지는지를 판별할 수 있다.

#define BEGIN_NS(name) namespace name {
#define END_NS }

BEGIN_NS(Case01)

// std::is_constant_evaluated()을 쓰면 쉽게 런타임 코드와 컴파일 타임 코드를 구분할 수 있다.
// 런타임 최적화와 컴파일 타임 최적화를 구분해서 코드를 작성하거나 런타임 예외 처리 등을 작성할 때 효과적이다.
//
// !! 주의 !!
// if constexpr은 컴파일 타임 조건 분기를 결정하기 위한 용도이고, 
// std::is_constant_evaluated()는 평가가 런타임에 이루어지는지 아니면 컴파일 타임에 이루어지는지에 따라서 bool 값을 반환하는 함수이다.
// 두 기능은 용도가 다르니 구분해서 봐야 한다.

const char* GetString() // runtime
{
    if /*constexpr*/ (std::is_constant_evaluated())
        return "Compile-time call";

    return "Runtime call"; // <-----
}

consteval const char* GetStringConstEval() // compile-time
{
    if /*constexpr*/ (std::is_constant_evaluated())
        return "Compile-time call"; // <-----

    return "Runtime call";
}

constexpr const char* GetStringConstExpr() // runtime or compile-time
{
    if /*constexpr*/ (std::is_constant_evaluated())
        return "Compile-time call"; // <-----

    return "Runtime call"; // <-----
}

// (중요) 컴파일 타임에 평가가 이루어지면 값이 바로 들어간다(디스어셈블러로 확인할 것).
void Run()
{
    auto str1 = GetString();
    std::cout << "str1 : " << str1 << '\n'; // runtime

    // GetString()은 런타임 함수이기 때문에 constexpr로 받을 수 없다.
    // constexpr auto str2 = GetString();
    // std::cout << "str2 : " << str2 << '\n'; // not working

    auto str3 = GetStringConstEval();
    std::cout << "str3 : " << str3 << '\n'; // compile-time

    constexpr auto str4 = GetStringConstEval();
    std::cout << "str4 : " << str4 << '\n'; // compile-time

    auto str5 = GetStringConstExpr();
    std::cout << "str5 : " << str5 << '\n'; // runtime

    constexpr auto str6 = GetStringConstExpr();
    std::cout << "str6 : " << str6 << '\n'; // compile-time
}

END_NS

BEGIN_NS(Case02)

// !! 중요 !!
// if constexpr과 std::is_constant_evaluated()는 다르다.
// if constexpr은 컴파일 타임에 주어진 조건이 true로 평가될 수 있으면 해당 분기로 코드를 생성하기 위한 용도로 사용한다.

constexpr int AddIfConstexpr(int x, int y)
{
    if constexpr (std::is_constant_evaluated())
        return (x + y) * 100;

    return x + y;
}

constexpr int AddOnlyIf(int x, int y)
{
    if (std::is_constant_evaluated())
        return (x + y) * 100;

    return x + y;
}

void Run()
{
    constexpr int val1 = AddIfConstexpr(10, 20);
    int val2 = AddIfConstexpr(10, 20);

    constexpr int val3 = AddOnlyIf(10, 20);
    int val4 = AddOnlyIf(10, 20);

    std::cout << "val1 : " << val1 << '\n'; // compile-time
    std::cout << "val2 : " << val2 << '\n'; // compile-time

    std::cout << "val3 : " << val3 << '\n'; // compile-time
    std::cout << "val4 : " << val4 << '\n'; // runtime
}

END_NS

BEGIN_NS(Case03)

// std:is_constant_evaluated()를 쓰면 정적 최적화와 런타임 코드를 구분할 수 있다.

constexpr int Factorial(int num)
{
    if (std::is_constant_evaluated())
    {
        // 컴파일 타임에 평가를 진행하여 재귀 함수를 최적화
        if (num <= 1)
            return 1;

        return num * Factorial(num - 1);
    }
    else
    {
        // 런타임에 평가를 진행하기에 결과 자체는 최적화되지 않는다.
        if (num <= 1)
            return 1;

        return num * Factorial(num - 1);
    }
}

// 디스어셈블러를 통해 결과가 한 번에 평가되어 들어가는지 확인할 것
void Run()
{
    // constexpr int facto1 = Factorial(100); // 평가된 값이 int의 범위를 벗어난다고 에러 발생
    // int facto2 = Factorial(100); // 값이 int의 범위를 벗어나지만 에러는 발생하지 않음.

    constexpr int facto3 = Factorial(10); // 디스어셈블러로 보면 값이 바로 들어감!
    int facto4 = Factorial(5);

    std::cout << "facto3 : " << facto3 << '\n'; // compile-time
    std::cout << "facto4 : " << facto4 << '\n'; // runtime
}

END_NS

int main()
{
    // Case01::Run();
    // Case02::Run();
    Case03::Run();

    return 0;
}
