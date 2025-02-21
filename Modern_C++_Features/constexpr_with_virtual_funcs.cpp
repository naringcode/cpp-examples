// Update Date : 2025-01-14
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>

using namespace std;

// C++20부터는 virtual 함수에 constexpr을 붙일 수 있다(const 한정 함수에서 써야 함).

class BaseA
{
public:
    virtual constexpr int GetValue() const
    {
        return 100;
    }
};

class DerivedB : public BaseA
{
public:
    constexpr int GetValue() const override
    {
        return 200;
    }
};

int main()
{
    constexpr BaseA    baseA;
    constexpr DerivedB derivedB;

    // 컴파일 타임에 계산
    static_assert(baseA.GetValue() == 100, "static_assert error 1");
    static_assert(derivedB.GetValue() == 200, "static_assert error 2");

    // 런타임에 계산
    cout << baseA.GetValue() << "\n";
    cout << derivedB.GetValue() << "\n";

    // 컴파일 타임에 계산하여 constinit 변수에 넣는 것도 가능하다.
    static constinit int s_ValA = baseA.GetValue();
    static constinit int s_ValB = derivedB.GetValue();

    cout << s_ValA << "\n";
    cout << s_ValB << "\n";

    return 0;
}
