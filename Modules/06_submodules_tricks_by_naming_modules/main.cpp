// Update Date : 2025-01-22
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>

// import Math.Arithmetic;
// import Math.Geometry;

import Math;

int main()
{
    double x = 3.14;
    double y = 2.0;

    std::cout << Add(x, y) << "\n";
    std::cout << Sub(x, y) << "\n";

    Point startP{ 10, 20 };
    Point endP{ 30, 40 };

    Line line{ startP, endP };

    std::cout << line << "\n";

    return 0;
}
