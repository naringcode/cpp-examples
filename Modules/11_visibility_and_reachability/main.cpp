// Update Date : 2025-01-22
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>

import Math;

int main()
{
    // Visible하지 않기 때문에 명시적으로 선언할 수 없다.
    // Point p1{ 10, 20 };
    // Point p2{ 30, 40 };

    // Point라는 이름을 명시하지 않더라도 생성자는 간접적으로 초기화 인자를 받을 수 있다.
    Line line{ { 10, 20 }, { 30, 40 } };

    std::cout << line << "\n";

    // Visible하지 않기 때문에 명시적으로 참조할 수 없다.
    // Point  valP1 = line.GetP1();
    // Point& refP2 = line.GetP2();

    // Visible하지 않더라도 간접적으로 받는 것은 가능하기에 Reachable하다.
    auto  valP1 = line.GetP1();
    auto& refP2 = line.GetP2();

    // main에서 타입 이름은 모르지만 간접적으로 받은 대상의 멤버 변수는 Visible하다.
    valP1.x = 100;
    valP1.y = 200;

    refP2.x = 300;
    refP2.y = 400;

    std::cout << line << "\n";

    return 0;
}
