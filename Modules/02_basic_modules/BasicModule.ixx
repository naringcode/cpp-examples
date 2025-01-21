// Global Module Fragment : Optional
module;

#include <iostream>

#define BASIC_MACRO

// Module Preamble : Required
export module BasicModule; // 모듈 이름

import AnotherModule;

// Module Purview / Module Interface : Optional
export void FuncFromBasic()
{
    std::cout << "Hello World!\n";

    // AnotherModule 내에 전처리기를 구성했어도 이건 그 모듈 파일에만 영향을 미칠 뿐
    // 해당 모듈을 포함하는 쪽에 영향을 미치는 것은 아니다(헤더 유닛은 영향을 미치니 혼동하지 말 것).
#ifdef ANOTHER_MACRO
    std::cout << "Yes ANOTHER_MACRO in Basic\n";
#else
    std::cout << "No ANOTHER_MACRO in Basic\n";
#endif

    FuncFromAnother();
}

// Private Module Fragment : Optional
module: private;
