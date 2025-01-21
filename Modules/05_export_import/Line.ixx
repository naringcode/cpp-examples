// Global Module Fragment : Optional
module;

#include <iostream>

// Module Preamble : Required
export module Line; // 모듈 이름

// export import를 적용하면 Point는 해당 모듈 외 Line 모듈을 포함하는 쪽에서도 사용 가능하다.
export import Point;

// Module Purview / Module Interface : Optional
export class Line
{
public:
    Line(Point p1, Point p2)
        : _p1{ p1 }, _p2{ p2 }
    { }

public:
    friend std::ostream& operator<<(std::ostream& os, const Line& line)
    {
        os << "Line : [" 
            << line._p1.x << ", " << line._p1.y << "] -> ["
            << line._p2.x << ", " << line._p2.y << "]";

        return os;
    }

private:
    Point _p1;
    Point _p2;
};

// Private Module Fragment : Optional
module: private;
