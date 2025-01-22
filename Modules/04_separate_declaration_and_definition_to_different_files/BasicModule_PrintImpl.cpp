// Global Module Fragment : Optional
module;

#include <iostream>
#include <string>

// Module Preamble : Required
module BasicModule; // 모듈 이름

using namespace std;

// Module Purview / Module Interface : Optional

// 인터페이스를 구현한다.
void Print(const string& str)
{
    std::cout << str << "\n";
}

// Private Module Fragment : Optional
// Private Module Fragment는 주 모듈(Primary Module) 쪽에서만 사용 가능하다.
// module: private;
