#include <iostream>
#include <fstream>

class Point
{
private:
    friend std::ostream& operator<<(std::ostream& out, const Point& point)
    {
        out << "[" << point._x << ", " << point._y << "]";

        return out;
    }

    friend std::istream& operator>>(std::istream& in, Point& point)
    {
        in >> point._x >> point._y;

        return in;
    }

private:
    int _x;
    int _y;
};

int main()
{
    using namespace std;

    ofstream outFile("out.txt");

    Point p1;
    Point p2;

    cin >> p1 >> p2;

    cout << p1 << ", " << p2;
    outFile << "Result : " << p1 << ", " << p2;

    outFile.close();

    return 0;
}
