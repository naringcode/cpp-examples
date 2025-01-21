// Global Module Fragment : Optional
module;

#include <iostream>

// Module Preamble : Required
export module Math.Geometry; // 모듈 이름

export
{
    struct Point
    {
        int x;
        int y;
    };

    class Line
    {
    public:
        Line(Point p1, Point p2)
            : _p1{ p1 }, _p2{ p2 }
        {
        }

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
}

// Module Purview / Module Interface : Optional

// Private Module Fragment : Optional
module: private;
