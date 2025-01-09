#include <iostream>

class MyClass
{
private:
    // DoSomething()에 한해선 private 접근 허용
    friend void DoSomething(const MyClass& myClass);

private:
    int _x = 100;
};

void DoSomething(const MyClass& myClass)
{
    std::cout << myClass._x << '\n';
}

int main()
{
    using namespace std;

    MyClass myClass;

    DoSomething(myClass);

    return 0;
}
