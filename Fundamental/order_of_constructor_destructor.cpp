#include <iostream>

class A
{
public:
    A()
    {
        std::cout << "Constructor A\n";
    }

    virtual ~A()
    {
        std::cout << "Destructor A\n";
    }
};

class B : public A
{
public:
    B()
    {
        std::cout << "Constructor B\n";
    }

    ~B() override
    {
        std::cout << "Destructor B\n";
    }
};

class C : public B
{
public:
    C()
    {
        std::cout << "Constructor C\n";
    }

    ~C() override
    {
        std::cout << "Destructor C\n";
    }
};

int main()
{
    using namespace std;

    // Constructor A -> B -> C
    // Destructor C -> B -> A
    A* test = new C();

    // 익숙하면 오히려 아리송 할 때가 있으니 주의
    // 뿌리(부모)부터 자란다고 생각하고 스택처럼 동작한다고 숙지

    delete test;

    return 0;
}
