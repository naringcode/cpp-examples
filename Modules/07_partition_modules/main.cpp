// Update Date : 2025-01-22
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>

import MainModule;

// 다음과 같이 파티션을 동일 모듈이 아닌 곳에서 import하는 건 허용하지 않는다.
// import MainModule:PartitionA;
// import MainModule:PartitionB;

int main()
{
    MainModuleFunc();

    std::cout << "--------------------------------------------------\n";

    // MainModule에서 PartitionA에 export import를 적용했기 때문에 공개되어 있다.
    PartitionAFunc();
    
    std::cout << "--------------------------------------------------\n";

    // 반면에 다음 함수는 export import가 아닌 단순 import를 한 쪽의 파티션이기 때문에 외부에서 사용할 수 없다.
    // PartitionBFunc();

    return 0;
}
