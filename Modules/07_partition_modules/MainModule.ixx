// Global Module Fragment : Optional
module;

#include <iostream>

// Module Preamble : Required
export module MainModule; // 모듈 이름

export import :PartitionA; // PartitionA는 외부에 공개
import :PartitionB; // PartitionB는 모듈 내부에서만 사용

// Module Purview / Module Interface : Optional
export void MainModuleFunc()
{
    std::cout << "MainModuleFunc()\n";

    PartitionAFunc();
    PartitionBFunc();
}

// Private Module Fragment : Optional
module: private;
