// Update Date : 2025-01-22
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>

import BasicModule;

int main()
{
    // 해당 함수는 export가 적용되어 있지 않아 외부에 공개되지 않는다.
    // ModuleLinkageFunc();

    ExternalLinkageFunc();
    ExternalLinkageFuncInExportBlock();

    // ArithmeticConcept는 Module Linkage로 되어 있기에 외부에 노출되지 않는다.
    // std::cout << ArithmeticConcept<int> << '\n';

    std::cout << Add(20, 30) << "\n";
    std::cout << Add(123.f, 3.14) << "\n";

    PrintPoint(Point{ 10, 20 });

    return 0;
}
