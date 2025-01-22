#pragma once

#include <iostream>;

#define SOMEHEADER_MACRO

void PrintFromSomeHeader()
{
    std::cout << "PrintFromBasicModule()\n";

    // 헤더 유닛 쪽은 외부에서 정의한 전처리기에 영향을 받지 않는다.
#ifdef BASICMODULE_MACRO
    std::cout << "Yes BASICMODULE_MACRO in Module\n";
#else
    std::cout << "No BASICMODULE_MACRO in Module\n";
#endif
}
