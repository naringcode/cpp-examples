// Update Date : 2025-01-13
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>

using namespace std;

// https://en.cppreference.com/w/cpp/language/consteval

// consteval은 항상 컴파일 타임에 수행되어야 하는 함수에 붙는 키워드이다.
// !! constexpr는 컴파일 타임에 평가할 수 없으면 런타임에 평가하는데 consteval은 이런 상황에선 에러를 발생시킴. !!
consteval int Add(int x, int y)
{
    return x + y;
}

int main()
{
    // OK
    cout << Add(10, 20) << "\n";

    // OK
    const int temp1 = 10;
    cout << Add(temp1, 20) << "\n";

    // NO(temp2는 런타임에 변형될 여지가 있음)
    // int temp2 = 100;
    // cout << Add(temp2, 200) << "\n";

    // OK
    constexpr int temp3 = Add(123, 456);
    cout << temp3 << "\n";

    // OK(반환되는 값은 컴파일 타임에 파악할 수 있음)
    int temp4 = Add(111, 222);
    cout << temp4 << "\n";

    // NO(temp2는 런타임에 변형될 여지가 있음)
    // cout << Add(temp4, 333) << "\n";

    return 0;
}
