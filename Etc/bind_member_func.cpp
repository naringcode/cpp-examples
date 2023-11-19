#include <iostream>
#include <cstdarg> // ellipsis

class MyClass
{
public:
    int Calculate()
    {
        return _x + _y;
    }

public:
    void SetValues(int x, int y)
    {
        _x = x;
        _y = y;
    }

private:
    int _x;
    int _y;
};

int main()
{
    using namespace std;

    MyClass a;
    a.SetValues(10, 20);

    MyClass b;
    b.SetValues(100, 200);

    int (MyClass::*funcPtr)() = &MyClass::Calculate;

    // 인스턴스의 this 포인터를 가져와 사용하는 형태
    // 어떤 멤버 함수든 첫 번째 인자는 보이지 않는 this이다.
    cout << (a.*funcPtr)() << '\n';
    cout << (b.*funcPtr)() << '\n';

    return 0;
}
