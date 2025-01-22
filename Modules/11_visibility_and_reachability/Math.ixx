// Global Module Fragment : Optional
module;

#include <iostream>

// Module Preamble : Required
export module Math; // 모듈 이름

// Module Purview / Module Interface : Optional

// export를 적용하지 않았기 때문에 Visible하지 않다.
struct Point
{
    int x;
    int y;
};

// export가 적용된 엔터티는 Visible하다.
export class Line
{
public:
    Line(Point p1, Point p2)
        : _p1{ p1 }, _p2{ p2 }
    { }

public:
    // Point는 Visible하지 않더라도 받은 허용하기 때문에 Reachable하다.
    Point& GetP1() { return _p1; }
    Point& GetP2() { return _p2; }

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
