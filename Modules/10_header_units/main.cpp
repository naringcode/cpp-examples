// Update Date : 2025-01-22
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

// 소스 코드 수준에서 헤더 유닛을 쓰기 위해선?
// C/C++ -> General(일반) -> Scan Sources for Module Dependencies(소스의 모듈 종속성 검사) -> Yes
import <iostream>; // 헤더 유닛으로 사용하기

import BasicModule;
// import "SomeHeader.h";

int main()
{
    PrintFromBasicModule();

    std::cout << "--------------------------------------------------" << "\n";

    // 모듈 내에서 정의한 전처리기는 이걸 포함하는 쪽에 영향을 미치지 않는다.
    // 정의한 전처리기도 모듈 자체에 캡슐화된다고 보면 된다.
#ifdef BASICMODULE_MACRO
    std::cout << "Yes BASICMODULE_MACRO in Module\n";
#else
    std::cout << "No BASICMODULE_MACRO in Module\n";
#endif

    // 상단의 import 구문에 달린 주석을 넣고 빼보면서 비교분석하도록 한다.
    // import "SomeHeader.h";
#ifdef SOMEHEADER_MACRO
    std::cout << "Yes SOMEHEADER_MACRO in Main\n";
#else
    std::cout << "No SOMEHEADER_MACRO in Main\n";
#endif

    return 0;
}
