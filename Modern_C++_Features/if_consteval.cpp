// Update Date : 2025-02-23
// OS : Windows 10 64bit
// Program : vscode(gcc-14.2.0)
// Version : C++23
// Configuration : None

#include <iostream>

// https://en.cppreference.com/w/cpp/language/if#Consteval_if

// 예제 코드는 "is_constant_evaluated.cpp"에 있는 것을 그대로 썼다.
//
// C++23부터는 std::is_constant_evaluated()를 거치지 않고도 if consteval 문법을 통해
// 컴파일 타임에 평가되는 것을 판가름해서 런타임 코드와 구분하는 것이 가능하다.
//
// if consteval은 compound-statement을 요구하는데 쉽게 말해서 몸체를 생략해서는 안 된다는 뜻이다.
// 또한 컴파일 타임에 평가될 수 없는 일반 함수는 if consteval을 적용할 수 없다.

// 아래 코드는 cppreference에서 제공하는 예시 중 하나다.
// 
// constexpr bool is_constant_evaluated() noexcept
// {
//     if consteval { return true; } else { return false; }
// }
//  
// constexpr bool is_runtime_evaluated() noexcept
// {
//     if not consteval { return true; } else { return false; }
// }

#define BEGIN_NS(name) namespace name {
#define END_NS }

BEGIN_NS(Case01)

// 런타임 평가만 이루어지는 코드도 if consteval 문법을 사용하는 건 가능하다.
const char* GetString() // runtime
{
    // 해당 문법을 사용할 때 이런 식으로 몸체를 생략하는 건 안 된다.
    // if consteval
    //     return "Compile-time call";
    //
    // else // if consteval과 연계되는 else도 마찬가지로 몸체를 생략해선 안 된다.
    //     return "Runtime call"; // <-----

    if consteval
    {
        return "Compile-time call";
    }
    else
    {
        return "Runtime call"; // <-----
    }
}

consteval const char* GetStringConstEval() // compile-time
{
    if consteval
    {
        return "Compile-time call"; // <-----
    }
    else
    {
        return "Runtime call";
    }
}

constexpr const char* GetStringConstExpr() // runtime or compile-time
{
    if consteval
    {
        return "Compile-time call"; // <-----
    }

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

// constexpr int AddIfConstexpr(int x, int y)
// {
//     if constexpr (std::is_constant_evaluated())
//         return (x + y) * 100;
// 
//     return x + y;
// }
// 
// constexpr int AddOnlyIf(int x, int y)
// {
//     if (std::is_constant_evaluated())
//         return (x + y) * 100;
// 
//     return x + y;
// }

constexpr int Add(int x, int y)
{
    if consteval
    {
        return (x + y) * 100;
    }

    return x + y;
}

void Run()
{
    // constexpr int val1 = AddIfConstexpr(10, 20);
    // int val2 = AddIfConstexpr(10, 20);
    // 
    // constexpr int val3 = AddOnlyIf(10, 20);
    // int val4 = AddOnlyIf(10, 20);
    // 
    // std::cout << "val1 : " << val1 << '\n'; // compile-time
    // std::cout << "val2 : " << val2 << '\n'; // compile-time
    // 
    // std::cout << "val3 : " << val3 << '\n'; // compile-time
    // std::cout << "val4 : " << val4 << '\n'; // runtime
    
    constexpr int val1 = Add(10, 20);
    int val2 = Add(10, 20);
    
    std::cout << "val1 : " << val1 << '\n'; // compile-time
    std::cout << "val2 : " << val2 << '\n'; // runtime
}

END_NS

BEGIN_NS(Case03)

// std:is_constant_evaluated()를 쓰면 정적 최적화와 런타임 코드를 구분할 수 있다.

constexpr int Factorial(int num)
{
    if consteval // (std::is_constant_evaluated())
    {
        // 컴파일 타임에 평가를 진행하여 재귀 함수를 최적화
        if (num <= 1)
            return 1;

        return num * Factorial(num - 1);
    }
    else
    {
        // 런타임에 평가를 진행하기에 결과 최적화는 컴파일러의 최적화 수준에 의존적이다.
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
    int facto4 = Factorial(5); // 컴파일러의 최적화 수준에 따라 값이 바로 들어갈 수도 있고 함수를 호출할 수도 있음.

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
