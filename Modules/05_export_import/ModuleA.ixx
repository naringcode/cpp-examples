// Global Module Fragment : Optional
module;

#include <iostream>

// Module Preamble : Required
export module ModuleA; // 모듈 이름

// Module Purview / Module Interface : Optional
export void PrintA()
{
    std::cout << "PrintA()\n";
}

// Private Module Fragment : Optional
module: private;
