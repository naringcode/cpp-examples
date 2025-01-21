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
// export module 쪽이 아니기 때문에 사용 불가(Private Module Fragment는 인터페이스를 정의하는 쪽에서만 사용 가능)
// module: private;
