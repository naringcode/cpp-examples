#include <iostream>

class Point
{
public:
    explicit Point(int x = 0, int y = 0) : _x(x), _y(y)
    { }

    // 실수 형식은 받지 못 하게 막기
    Point(float, float) = delete;

public:
    void Show() const
    {
        std::cout << _x << ", " << _y;
    }

private:
    int _x;
    int _y;
};

void DoSomething(const Point point)
{
    point.Show();
}

int main()
{
    using namespace std;

    // 생성자에 explicit을 제거하면 사용 가능
    // DoSomething(10);
    // DoSomething({ 10, 20 });

    // explicit을 적용하면 명시적으로 Point를 써야 함
    DoSomething(Point{ 10, 20 });

    // Point(float, float) = delete
    // 형변환되어 들어가는 방식을 막았다.
    // DoSomething(Point{ 10.0f, 20.0f });

    return 0;
}
