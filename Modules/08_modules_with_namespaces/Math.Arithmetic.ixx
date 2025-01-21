// Global Module Fragment : Optional
module;

// Module Preamble : Required
export module Math.Arithmetic; // 모듈 이름

export namespace Math
{
    double Add(double x, double y)
    {
        return x + y;
    }

    double Sub(double x, double y);
}

namespace Math
{
    double Sub(double x, double y)
    {
        return x - y;
    }
}

// Module Purview / Module Interface : Optional

// Private Module Fragment : Optional
module: private;
