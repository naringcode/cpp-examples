// Global Module Fragment : Optional
module;

#include <iostream>

// Module Preamble : Required
export module ModuleC; // 모듈 이름

// export 없이 import만 적용하면 ModuleB가 반영하고 있는 건 ModuleC에서만 사용 가능하다.
/* export */ import ModuleB;

// Module Purview / Module Interface : Optional
export void PrintC()
{
    std::cout << "PrintC()\n";

    PrintB();
    PrintA();
}

// Private Module Fragment : Optional
module: private;
