#include <iostream>
#include <memory>

using namespace std;

class A
{
public:
    A() { cout << "A()\n"; }
    ~A() { cout << "~A()\n"; } // 의도적으로 소멸자를 가상화하지 않음.

private:
    int _val = 100;
};

class B
{
public:
    B() { cout << "B()\n"; }
    ~B() { cout << "~B()\n"; } // 의도적으로 소멸자를 가상화하지 않음.

private:
    int _val = 200;
};

class C
{
public:
    C() { cout << "C()\n"; }
    ~C() { cout << "~C()\n"; } // 의도적으로 소멸자를 가상화하지 않음.

private:
    int _val = 300;
};

class AEx
{
public:
    AEx() { cout << "AEx()\n"; }
    ~AEx() { cout << "~AEx()\n"; } // 의도적으로 소멸자를 가상화하지 않음.

private:
    int _val = 400;
};

// -----

class First
{
public:
    First() { cout << "First()\n"; }
    ~First() { cout << "~First()\n"; } // 의도적으로 소멸자를 가상화하지 않음.

private:
    A _a;
};

class Second : public First
{
public:
    Second() { cout << "Second()\n"; }
    virtual ~Second() { cout << "~Second()\n"; } // 소멸자 가상화를 여기서 진행함.

private:
    B _b;
};

class Third : public Second
{
public:
    Third() { cout << "Third()\n"; }
    virtual ~Third() { cout << "~Third()\n"; } // 가상화 소멸자가 적용됨.

private:
    C _c;
};

class FirstEx : public First
{
public:
    FirstEx() { cout << "FirstEx()\n"; }
    ~FirstEx() { cout << "~FirstEx()\n"; } // 가상화 소멸자가 적용됨.

private:
    AEx _aEx;
};

int main()
{
    {
        Second second;
    }

    cout << "--------------------------------------------------\n";

    {
        Third third;
    }

    cout << "--------------------------------------------------\n";

    // {
    //     // 에러 발생(가상 함수 테이블 관련 문제인 듯?)
    //     // First* first = new Third{ };
    //     // 
    //     // delete first;
    // }
    // 
    // cout << "--------------------------------------------------\n";

    {
        Second* second = new Third{ };

        delete second;
    }

    cout << "--------------------------------------------------\n";

    {
        Second* second = new Second{ };

        delete second;
    }

    cout << "--------------------------------------------------\n";

    {
        FirstEx* firstEx = new FirstEx{ };

        delete firstEx;
    }

    cout << "--------------------------------------------------\n";

    {
        // FirstEx로 생성했지만 소멸자는 호출하지 못 함.
        First* first = new FirstEx{ };

        delete first;
    }

    return 0;
}
