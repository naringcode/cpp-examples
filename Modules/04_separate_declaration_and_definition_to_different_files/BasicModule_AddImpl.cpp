// Global Module Fragment : Optional
module;

// Module Preamble : Required
module BasicModule; // 모듈 이름

using namespace std;

// Module Purview / Module Interface : Optional

// 인터페이스를 구현한다.
double Add(double x, double y)
{
    return x + y;
}

// Private Module Fragment : Optional
// Private Module Fragment는 주 모듈(Primary Module) 쪽에서만 사용 가능하다.
// module: private;
