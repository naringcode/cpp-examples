// Global Module Fragment : Optional
module;

#define BASICMODULE_MACRO

// Module Preamble : Required
export module BasicModule; // 모듈 이름

// 헤더 유닛으로 사용하기
import <iostream>;
import "SomeHeader.h";

// Module Purview / Module Interface : Optional
export void PrintFromBasicModule()
{
    std::cout << "PrintFromBasicModule()\n";

    // 헤더 유닛을 포함하는 쪽은 헤더 파일에 정의된 전처리기에 영향을 받는다.
#ifdef SOMEHEADER_MACRO
    std::cout << "Yes SOMEHEADER_MACRO in Module\n";
#else
    std::cout << "No SOMEHEADER_MACRO in Module\n";
#endif

    PrintFromSomeHeader();
}

// Private Module Fragment : Optional
module: private;
