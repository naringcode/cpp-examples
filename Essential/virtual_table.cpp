#include <iostream>
#include <random>

// Virtual Table이 작동하는 방식

/*
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
 * ########################################
 * 
 * 클래스 내 virtual로 선언된 함수가 있다면?
 * 
 * 컴파일 단계에서 클래스에 대응되는 Virtual Table이 생성됨.
 * 
 * 또한 가상 함수를 가진 클래스로 객체를 만들면 _vfptr이 추가됨.
 * 
 * 가상 함수를 활용한 다형성의 핵심이 바로 _vfptr.
 * 
 * 
 * Virtual Table을 타지 않아도 되는 상황은 정적 바인딩(컴파일 타임에 결정).
 * 
 * Virtual Table을 타는 상황은 동적 바인딩(런타임에 결정).
 * 
 * ########################################
 * 
 * Base* base = new Derived();
 * 
 * base의 _vfptr은 "Derived Virtual Table"를 가리킴.
 * 
 * 이 상태에서 base->funcA()를 호출하면?
 * 
 * 1. Derived Virtual Table을 조회한다.
 * 
 * 2. funcA()를 찾고 호출한다.
 * 
 * 찾는다는 행위 자체가 비용이기 때문에
 * 
 * 빈번히 호출되며 또한 성능이 중요한 로직 쪽은 Reverse OOP를 고려하던지
 * 
 * 그냥 가상 함수를 쓰지 않는 방향으로 개발하는 것이 좋긴 함.
 * 
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

    // 디버깅으로 중단점 걸어서 확인할 것은!
    // 1. _vfptr이 어디를 가리키고 있을까?
    // 2. 디스어셈블리로 조회했을 때 어떻게 동작할까?

    return 0;
}
