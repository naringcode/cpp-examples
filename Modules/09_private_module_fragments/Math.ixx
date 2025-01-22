// Global Module Fragment : Optional
module;

// Module Preamble : Required
export module Math; // 모듈 이름

// Module Purview / Module Interface : Optional

// 인터페이스 이름만 선언한다.
export namespace Math
{
    double Add(double x, double y);
    double Sub(double x, double y);
}

// Private Module Fragment : Optional
module: private;

namespace Math
{
    double Add(double x, double y)
    {
        return x + y;
    }

    double Sub(double x, double y)
    {
        return x - y;
    }
}
