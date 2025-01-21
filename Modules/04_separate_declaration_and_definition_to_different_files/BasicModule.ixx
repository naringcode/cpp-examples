// Global Module Fragment : Optional
module;

#include <string>

// Module Preamble : Required
export module BasicModule; // 모듈 이름

using namespace std;

// Module Purview / Module Interface : Optional

// 인터페이스 이름만 선언한다.
export void Print(const string& str);
export double Add(double x, double y);

// Private Module Fragment : Optional
module: private;
