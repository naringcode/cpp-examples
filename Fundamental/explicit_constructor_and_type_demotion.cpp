#include <iostream>

using namespace std;

class Point
{
public:
    explicit Point(int x = 0, int y = 0) : _x(x), _y(y)
    { }

public:
    void Show() const
    {
        std::cout << _x << ", " << _y << "\n";
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
    // 생성자에 explicit을 제거하면 사용 가능
    // DoSomething(10);
    // DoSomething((10, 20)); // 이건 생성자가 아닌 "," 연산자로 왼쪽에 있는 값이 들어감(20는 무시됨).
    // DoSomething({ 10, 20 });
    
    // explicit을 적용하면 명시적으로 Point를 써서 어떤 타입을 사용할 것인지 명시해야 한다.
    DoSomething(Point(10, 20));
    DoSomething(Point{ 10, 20 });
    
    // 아래 코드는 축소 형변환으로 인한 에러가 발생하며 이것 자체는 explicit과는 무관하다.
    // DoSomething({ 10.0f, 20.0f });      // explicit으로 인한 에러가 아닌 축소 형변환으로 인한 에러 발생
    // DoSomething(Point{ 10.0f, 20.0f }); // 축소 형변환으로 인한 에러 발생

    // explicit을 쓰면 다음과 같은 암시적인 형변환을 허용하지 않는다(explicit을 제거하면 사용 가능).
    // Point p1 = 200; // Point{ 200 }으로 변환되어 적용

    // explicit을 적용하면 항상 생성자를 올바로 호출해야 한다.
    Point p2 = Point(100, 200);
    Point p3 = Point{ 100, 200 };
    Point p4(100, 200);
    Point p5{ 100, 200 };
    
    // 축소 형변환과 explicit은 다른 개념(이건 허용되는 문법이니 혼동하지 말 것)
    Point p6{ numeric_limits<short>::max(), numeric_limits<long>::max() };
    Point p7{ 'A', 'Z' };

    // 축소 형변환과 explicit 문법은 구분해서 보도록 한다.
    // Point p8{ 100.0, 200 };
    // Point p9{ 'A', 200.0 };
    // Point p10{ numeric_limits<long long>::max(), 50 };

    return 0;
}
