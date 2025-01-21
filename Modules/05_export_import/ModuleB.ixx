// Global Module Fragment : Optional
module;

#include <iostream>

// Module Preamble : Required
export module ModuleB; // 모듈 이름

export import ModuleA;

// Module Purview / Module Interface : Optional
export void PrintB()
{
    std::cout << "PrintB()\n";

    PrintA();
}

// Private Module Fragment : Optional
module: private;
