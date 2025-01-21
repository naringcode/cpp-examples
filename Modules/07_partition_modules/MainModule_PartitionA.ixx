// Global Module Fragment : Optional
module;

#include <iostream>

// Module Preamble : Required
export module MainModule:PartitionA; // 모듈 이름

// 같은 모듈이라면 다른 파티션을 import해서 사용할 수 있다.
import :PartitionB;

// Module Purview / Module Interface : Optional
export void PartitionAFunc()
{
    std::cout << "PartitionAFunc()\n";

    PartitionBFunc();
}

// Private Module Fragment : Optional
// Private Module Fragment는 주 모듈(Primary Module) 쪽에서만 사용 가능하다.
// module: private;
