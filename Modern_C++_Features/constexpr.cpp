#include <iostream>
#include <type_traits>

// if constexpr (C++17)
// https://en.cppreference.com/w/cpp/language/if
// https://learn.microsoft.com/en-us/cpp/cpp/if-else-statement-cpp?view=msvc-170
// https://modoocode.com/293
//
// constexpr function (C++11)
// https://en.cppreference.com/w/cpp/language/constexpr
// https://learn.microsoft.com/ko-kr/cpp/cpp/constexpr-cpp?view=msvc-170
// https://www.geeksforgeeks.org/understanding-constexper-specifier-in-cpp/
// https://modoocode.com/293
// - 컴파일 타임에 계산할 수 있으면 컴파일 타임에 계산을 끝내는 함수임.
// - 런타임에 결정되는 사항이라면 런타임 도중에 계산하는 것도 가능함.
//
// static_assert()
// https://learn.microsoft.com/ko-kr/cpp/cpp/static-assert?view=msvc-170
// 
// if constexpr (condition)
// {
//     ...
// }
// 
// constexpr return_type function_name(params...)
// {
//     ...
// }
//

template <typename T>
void PrintValue(T value)
{
    if constexpr (std::is_integral<T>::value)
    {
        std::cout << "Integer : " << value << '\n';
    }
    else if constexpr (std::is_floating_point<T>::value)
    {
        std::cout << "Float : " << value << '\n';
    }
    else if constexpr (std::is_pointer<T>::value)
    {
        std::cout << "Pointer : " << value << '\n';
    }
    else
    {
        static_assert(std::is_integral<T>::value, "Unknown Type");
    }
}

template <typename Integer>
void PrintOddEven(Integer num)
{
    if constexpr (std::is_integral<Integer>::value)
    {
        if (0 == num % 2)
        {
            std::cout << num << " is Even.\n";
        }
        else
        {
            std::cout << num << " is Odd.\n";
        }
    }
    else
    {
        static_assert(std::is_integral<Integer>::value, "Not Integer Type");
    }
}

constexpr bool Is64BitOperatingSystem()
{
    return sizeof(void*) == 8;
}

// if-constexpr 1
void Run01()
{
    int* pt        = new int{ 0 };
    std::pair pair = { 10, 3.14 };

    PrintValue(10);
    PrintValue(3.14);
    PrintValue(pt);
    PrintValue(&pair);
    // PrintValue(pair); // error!

    delete pt;
}

// if-constexpr 2
void Run02()
{
    PrintOddEven(10);
    PrintOddEven(11);
    // PrintOddEven(3.14); // error!
}

// constexpr function
void Run03()
{
    constexpr bool isCompileTime = Is64BitOperatingSystem();

    std::cout << "64bit? " << std::boolalpha << isCompileTime << '\n';
}

int main()
{
    Run01();

    std::cout << "-------------------------\n";

    Run02();

    std::cout << "-------------------------\n";

    Run03();

    return 0;
}
