#include <iostream>

// https://www.learncpp.com/cpp-tutorial/overloading-typecasts/

class Point
{
public:
    // overloading typecasts
    operator int()
    {
        return _x + _y;
    }

    operator float()
    {
        return 10.0f * float(_x + _y);
    }

private:
    int _x = 10;
    int _y = 20;
};

int main()
{
    using namespace std;

    Point point;

    std::cout << (int)point << '\n';
    std::cout << static_cast<int>(point) << '\n';

    std::cout << (float)point << '\n';
    std::cout << static_cast<float>(point) << '\n';


    return 0;
}
