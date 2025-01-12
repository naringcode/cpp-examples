#include <iostream>
#include <random>

// Virtual Table이 작동하는 방식

/**
 * -------------------
 * |      Base       |
 * -------------------
 * |  Base::funcA()  |
 * |  Base::funcB()  |
 * |     _vfptr      |
 * -------------------
 * 
 * ----------------------
 * |      Derived       |
 * ----------------------
 * |  Derived::funcA()  |
 * |   Base::funcB()    |
 * |  Derived::funcC()  |
 * |      _vfptr        |
 * ----------------------
 * 
 * ------------------------
 * |  Base Virtual Table  |
 * ------------------------
 * |    Base::funcA()     |
 * |    Base::funcB()     |
 * ------------------------
 * 
 * ---------------------------
 * |  Derived Virtual Table  |
 * ---------------------------
 * |     Derived::funcA()    |
 * |      Base::funcB()      |
 * |     Derived::funcC()    |
 * ---------------------------
 * 
 * --------------------------------------------------
 * 
 * 클래스 내 virtual로 선언된 함수가 있다면?
 * 컴파일 단계에서는 해당 클래스에 대응되는 Virtual Table이 생성되며,
 * 런타임 시 가상 함수를 가진 클래스를 객체로 만들면 할당된 객체에 _vfptr이라는 멤버(?)가 추가된다.
 * 
 * (중요) 실제 프로그램을 실행할 때 다형성이란 기능을 수행하기 위해 핵심이 되는 것이 바로 이 _vfptr이다.
 * 
 * Virtual Table을 타지 않아도 되는 상황은 정적 바인딩되며(컴파일 타임에 결정),
 * Virtual Table을 타는 상황은 동적 바인딩된다(런타임에 결정).
 * 
 * --------------------------------------------------
 * 
 * Base* base = new Derived();
 * 
 * base의 _vfptr은 "Derived Virtual Table"를 가리킨다.
 * 
 * 이 상태에서 base->funcA()를 호출하면?
 * 1. Derived Virtual Table을 조회한다.
 * 2. funcA()를 찾고 호출한다.
 * 
 * 찾는다는 행위 자체가 비용이기 때문에 가상 함수를 빈번히 호출하면 성능이 저하될 수밖에 없다.
 * 따라서 성능이 중요한 로직이라면 Reverse OOP를 고려하던지 아예 가상 함수를 쓰지 않는 방향으로 개발하는 것이 좋을 수 있다.
 * 
 * !! 개발 속도와 유지보수를 따진다면 가상 함수를 쓰는 것이 좋긴 함. !! 
 */

class Base
{
public:
    virtual void funcA() { }
    virtual void funcB() { }
};

class Derived : public Base
{
public:
    void funcA() override { }
    
    virtual void funcC() { }
};

int main()
{
    using namespace std;

    // 디버깅으로 중단점 걸어서 확인할 것은...
    // 1. _vfptr이 어디를 가리키고 있을까?
    // 2. 디스어셈블리로 조회했을 때 어떻게 동작할까?

    // 멤버 변수는 없지만 _vfptr이 존재하기 때문에 크기가 조회된다.
    cout << sizeof(Base) << " " << sizeof(Derived) << "\n";

    // _vfptr이 가리키는 대상은 똑같다(derA, derB, derC).
    Derived derA;
    Derived derB;
    Derived derC;

    // _vfptr이 가리키는 대상은 똑같다(baseA, baseB, baseC).
    Base baseA;
    Base baseB;
    Base baseC;

    Base* vBase;
    
    if (random_device()() % 2)
    {
        // _vfptr이 가리키는 대상은 Derived의 것과 동일하다.
        vBase = new Derived();
    }
    else
    {
        // _vfptr이 가리키는 대상은 Base의 것과 동일하다.
        vBase = new Base();
    }

    vBase->funcA();

    delete vBase;

    return 0;
}
