#include <iostream>

// 전방 선언
class ClassA;

class ClassB
{
public:
    // 일단은 있다고 선언(ClassA가 알 수 있게)
    void DoSomething(const ClassA& a);

private:
    int _x = 200;
};

class ClassA
{
private:
    // ClassB::DoSomething()에 한해서 private 접근 허용
    friend void ClassB::DoSomething(const ClassA& a);

private:
    int _x = 100;
};

//
void ClassB::DoSomething(const ClassA& a)
{
    std::cout << a._x + this->_x << '\n';
}

int main()
{
    using namespace std;

    ClassA classA;
    ClassB classB;

    classB.DoSomething(classA);

    return 0;
}
