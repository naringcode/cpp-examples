#include <iostream>

/**
 * Visitor 패턴은 개방-폐쇠 원칙(Open/Closed Principle)을 구현하는 방법 중 하나.
 * 해당 패턴을 이용하면 객체(Element) 구조를 변경하지 않고 새로운 기능을 추가할 수 있다.
 * 
 * 이용하고자 하는 실제 기능은 Visitor 인터페이스를 구체화하여 구현한다.
 * 즉, 자료는 Element에 기능은 Visitor에 구현하는 것이다.
 * 
 * - Element : 객체의 구조를 표현하기 위한 요소가 정의된 클래스, accept() 필요.
 * - Visitor : 새로운 연산이나 기능을 구현하기 위한 클래스, visit() 필요.
 * - 사용 방법 : Element 클래스가 구체화한 Visitor를 받고, Visitor의 visit()를 호출하여 실제 기능을 수행.
 * - 기능 추가 : 기존 Element 클래스 계층을 변경하지 않고 Visitor를 추가하여 새로운 기능을 확장 가능.
 */

/**
 * Visitor 패턴에는 방문하는 visit()와 받아들이는 accept()가 필요하지만 
 * 해당 패턴은 방문하는 행위와 무관하며 accept()라는 것도 명확한 이미지를 떠올리기엔 애매모호하다.
 * 
 * Visitor 패턴은 OOP 언어 내부에서 함수형 언어의 방식을 흉내내기 위한 패턴으로(접근 방식과 구현 방법에는 차이가 있음)
 * 기존 객체 구조는 그대로 두되 새로운 기능을 쉽게 추가하기 위해 사용한다.
 * 따라서 객체지향 설계에서 유연성을 높이고 객체 간 결합도는 낮출 수 있다.
 */

// Visitor 인터페이스
class Visitor
{
public:
    virtual ~Visitor() = default;

public:
    virtual void visitElementA(class ElementA* elementA) = 0;
    virtual void visitElementB(class ElementB* elementB) = 0;
};

// Element 인터페이스
class Element
{
public:
    virtual ~Element() = default;

public:
    virtual void accept(Visitor* visitor) = 0;
};

// Element 구현(1)
class ElementA : public Element
{
public:
    void accept(Visitor* visitor) override
    {
        visitor->visitElementA(this);
    }

    void someFunctionA()
    {
        std::cout << "ElementA::someFunctionA() called.\n";
    }
};

// Element 구현(2)
class ElementB : public Element
{
public:
    void accept(Visitor* visitor) override
    {
        visitor->visitElementB(this);
    }
    
    void someFunctionB()
    {
        std::cout << "ElementB::someFunctionB() called.\n";
    }
};

// Visitor 구현(1)
class VisitorA : public Visitor
{
public:
    void visitElementA(ElementA* elementA) override
    {
        std::cout << "ElementA visits VisitorA::visitElementA().\n";

        elementA->someFunctionA();
    }

    void visitElementB(ElementB* elementB) override
    {
        std::cout << "ElementB visits VisitorA::visitElementB().\n";

        elementB->someFunctionB();
    }
};

// Visitor 구현(1)
class VisitorB : public Visitor
{
public:
    void visitElementA(ElementA* elementA)
    {
        std::cout << "ElementA visits VisitorB::visitElementA().\n";

        elementA->someFunctionA();
    }

    void visitElementB(ElementB* elementB)
    {
        std::cout << "ElementB visits VisitorB::visitElementB().\n";

        elementB->someFunctionB();
    }
};

int main() 
{
    ElementA elemA;
    ElementB elemB;

    VisitorA visitorA;
    VisitorB visitorB;

    elemA.accept(&visitorA);
    elemA.accept(&visitorB);

    std::cout << '\n';
    
    elemB.accept(&visitorA);
    elemB.accept(&visitorB);

    return 0;
}