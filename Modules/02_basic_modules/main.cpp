// Update Date : 2025-01-21
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

import BasicModule;

// BasicModule에서 <iostream>을 포함하고 있지만 이건 해당 모듈에만 영향을 미친다.
// 이건 #include든 import든 둘 다 마찬가지인 사항이다(이때 필요한 것이 export import).
// 따라서 입출력을 진행하고 싶다면 main 쪽에서도 직접 포함하는 작업을 거쳐야 한다.
#include <iostream>;

int main()
{
    FuncFromBasic();

    // BasicModule이 AnotherModule을 import하고 있지만 이건 모듈 내부에서만 사용 가능할 뿐 외부로의 가시성을 제공하지는 않는다.
    // FuncFromAnother();

    // 모듈 내에서 정의한 전처리기는 이걸 포함하는 쪽에 영향을 미치지 않는다(헤더 유닛은 예외).
#ifdef BASIC_MACRO
    std::cout << "Yes BASIC_MACRO in Main\n";
#else
    std::cout << "No BASIC_MACRO in Main\n";
#endif

#ifdef ANOTHER_MACRO
    std::cout << "Yes ANOTHER_MACRO in Main\n";
#else
    std::cout << "No ANOTHER_MACRO in Main\n";
#endif

    return 0;
}
