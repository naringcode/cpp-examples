// Global Module Fragment : Optional
module;

#define ANOTHER_MACRO

// Module Preamble : Required
export module AnotherModule; // 모듈 이름

import <iostream>; // 헤더 유닛으로 사용하기

// Module Purview / Module Interface : Optional
export void FuncFromAnother()
{
    std::cout << "Hello Another World!\n";

#ifdef BASIC_MACRO
    std::cout << "Yes BASIC_MACRO in Another\n";
#else
    std::cout << "No BASIC_MACRO in Another\n";
#endif
}

// Private Module Fragment : Optional
module: private;
