// Update Date : 2025-01-22
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>

import ModuleC;
import Line;

int main()
{
    // ModuleC 내 ModuleB를 export import가 아닌 import로 포함하고 있기 때문에 사용 불가능하다.
    // PrintA();
    // PrintB();

    PrintC();

    Point startP{ 10, 20 };
    Point endP{ 30, 40 };

    Line line{ startP, endP };

    std::cout << line << "\n";

    return 0;
}
