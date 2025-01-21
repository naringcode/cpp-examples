// Global Module Fragment : Optional
module;

#include <iostream>

// Module Preamble : Required
export module MainModule:PartitionB; // 모듈 이름

// Module Purview / Module Interface : Optional
export void PartitionBFunc()
{
    std::cout << "PartitionBFunc()\n";
}

// Private Module Fragment : Optional
// Private Module Fragment는 주 모듈(Primary Module) 쪽에서만 사용 가능하다.
// module: private;
