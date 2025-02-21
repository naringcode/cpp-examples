// Update Date : 2025-01-14
// OS : Windows 10 64bit
// Program : Visual Studio 2022, vscode(gcc-13.1.0)
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <string>
#include <string_view>

using namespace std;

// https://en.cppreference.com/w/cpp/language/enum#using_enum_declaration
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1099r5.html

// C++20부터 using enum declaration을 사용하는 것이 가능하다.
// 전역 스코프, namespace 스코프, 구조체나 클래스 내부, 함수나 switch 내부 같은 다양한 곳에서 using enum declaration을 적용할 수 있다.

enum class Color { Red, Green, Blue };

class Fruit
{
public:
    enum FruitType // enum class 아님!
    {
        Orange, Apple
    };

public:
    static Fruit* CreateFruit(FruitType type) { return nullptr; }
};

string_view ToStringSwitch(Color color)
{
    // 바깥에 쓰면 함수 전체에 영향을 미침.
    // using enum Color;

    switch (color)
    {
        // switch 내부에 쓰면 switch 내부에만 영향을 미침.
        using enum Color;

        case Red:   return "Red"sv;
        case Green: return "Green"sv;
        case Blue:  return "Blue"sv;
    }

    return "Unknown";
}

string_view ToStringIf(Color color)
{
    // 특성 열거값만 대상으로 한 using도 사용 가능하다.
    using Color::Red;
    using Color::Green;

    if (color == Red)
    {
        return "Red";
    }
    else if (color == Green)
    {
        return "Green";
    }
    else if (color == Color::Blue) // Blue는 using을 적용하지 않은 상태라 Color를 붙여야 함.
    {
        return "Blue";
    }

    return "Unknown";
}

int main()
{
    using enum Color;

    Color c = Red; // 기존에는 Color::Red로 써야 한 부분

    cout << ToStringSwitch(c) << "\n";

    cout << ToStringIf(Green) << "\n";
    cout << ToStringIf(Blue) << "\n";

    // 다음과 같이 특정 구조체나 클래스 스코프 내에 있는 enum 값을 using하는 것도 가능하다.
    // enum class가 아닌 일반 enum을 대상으로도 적용할 수 있다.
    using Fruit::Apple;

    Fruit::CreateFruit(Apple);

    return 0;
}
