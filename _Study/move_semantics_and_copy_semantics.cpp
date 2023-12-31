#include <iostream>
#include <vector>

// RValue Reference와 Move Semantics는 다른 개념이다.
//
// # RValue Reference
// - &&를 사용해서 정의한 RValue 참조 형태
// - Move Semantics를 구현하기 위해서 활용하기도 함
//
// # Move Semantics
// - 객체의 소유권을 이전하는 것을 나타냄
// - Copy Semantics와는 다른 개념임

// # Copy Semantics
// - 복사 생성자
// - 복사 대입 연산자
//
// # Move Semantics
// - 이동 생성자
// - 이동 대입 연산자

// Move는 복사가 아닌 소유권 이전이다.

// std::move()는 대상을 RValue Reference로 캐스팅하는 함수이다.
// LValue를 RValue로 바꿔주는 것은 아니니까 주의하도록 한다.
// 
// 프로그래머는 std::move()를 통해 Copy Semantics를 사용할지 Move Semantics를 사용할지 결정할 수 있다.
// RValue는 캐스팅 없이 사용하면 그 자체로 일회성의 의미가 강하다.
// -------------------------
// // MyClass는 재사용하지 않음(일회용) -> RValue Reference -> push_back() 내부에서 forward() -> Move Constructor
// vector.push_back(MyClass{});
// -------------------------

// RValue Reference는 말 그대로 RValue에 대한 참조이기 때문에 이것 자체로는 이동 생성자가 호출되지 않는다.
// 이동 생성자는 새로운 객체를 생성할 때 소유권을 이전할 때 사용하는 생성자이다.
// 가끔 망각해서 정리한다.

// @@ 진짜 중요 @@
// LValue != LValue Reference
// RValue != RValue Reference
// 
// LValue는 대입 연산자 기준으로 왼쪽에 오는 것이고(원하는 시점에서 재사용할 수 있음)
// RValue는 대입 연산자 기준으로 오른쪽에 오는 것이다(값이 일시적임).
// 
// LValue
// - 어떤 표현식의 결과를 담을 수 있음.
// - 변수와 같은 것이 이에 해당함.
//
// RValue
// - 일시적으로 존재했다 소멸하는 특성이 있음.
// - 임시 객체, 리터럴 값, 함수 호출로 반환되는 값 등.
//
// LValue Reference와 RValue Reference
// - 둘 다 특정 객체를 참조하기 위해서 사용한다.
// - LValue를 참조하면 LValue Reference이다.
// - RValue를 참조하면 RValue Reference이다.
// - LValue라도 

class MyClass
{
public:
    MyClass()
    {
        std::cout << "Constructor : MyClass()" << '\n';
    }

public:
    ~MyClass()
    {
        std::cout << "Destructor : ~MyClass()" << '\n';
    }

public:
    MyClass(const MyClass& myClass)
    {
        std::cout << "Copy Constructor : MyClass(MyClass& myClass)" << '\n';
    }

    MyClass& operator=(const MyClass& myClass)
    {
        std::cout << "Copy Assignment : MyClass& operator=(MyClass& myClass)" << '\n';

        // 자기 자신에 대한 할당을 진행하는 경우
        if (&myClass == this)
            return *this;
    }

public:
    MyClass(MyClass&& myClass) noexcept
    {
        std::cout << "Move Constructor : MyClass(MyClass&& myClass)" << '\n';
    }

    MyClass& operator=(MyClass&& myClass) noexcept
    {
        std::cout << "Move Assignment : MyClass& operator=(MyClass&& myClass)" << '\n';

        // 자기 자신에 대한 할당을 진행하는 경우
        if (&myClass == this)
            return *this;

        return *this;
    }

public:
    int data = 0;
};

void Function(MyClass& myClass)
{
    class RAII
    {
    public:
        RAII() { std::cout << "### RAII() ###\n"; }
        ~RAII() { std::cout << "### ~RAII() ###\n"; }
    } test;

    std::cout << "LValue Type\n";

    // 당연히 LValue로 사용한다(그렇다고 LValue Reference와 LValue가 동일한 의미인 것은 아니다).
    MyClass whatSemantics = myClass; // 복사 생성자

    // LValue Reference로 받아도 RValue Reference로 캐스팅해서 이동 생성자를 호출할 수 있다.
    MyClass moveClass = std::move(myClass); // 이동 생성자
}

void Function(MyClass&& myClass)
{
    class RAII
    {
    public:
        RAII() { std::cout << "### RAII() ###\n"; }
        ~RAII() { std::cout << "### ~RAII() ###\n"; }
    } test;

    std::cout << "RValue Type\n";

    // RValue Reference로 받았다고 해서 Move Semantics를 사용한다는 것은 아니다.
    // myClass를 RValue Reference로 받기는 했지만 사용할 때는 LValue이다(##진짜진짜 중요중요##).
    MyClass whatSemantics = myClass; // ## 복사 생성자 ##

    // 참고로 RValue Reference가 RValue라면 data의 값을 갱신할 수 없어야 한다.
    // 이는 RValue Reference로 받았다고 해도 사용할 때는 LValue라는 것을 의미한다.
    myClass.data = 1000;
}

int main()
{
    {
        MyClass myClass;

        std::cout << "-----\n";

        // 전형적인 LValue Reference 예제
        Function(myClass);
    }

    std::cout << "--------------------\n";

    {
        MyClass myClass;

        std::cout << "-----\n";

        // std::move()를 사용해서 RValue Reference를 유도할 수 있다.
        // std::move()는 RValue Reference로 캐스팅하는 함수이지 이동 생성자를 적용한 새로운 객체를 만드는 함수가 아니다.
        Function(std::move(myClass));
    }

    std::cout << "--------------------\n";

    {
        // 기본 생성자 호출
        MyClass myClass = MyClass{};

        std::cout << "-----\n";

        // 복사 생성자 호출
        MyClass copyClass = myClass;

        std::cout << "-----\n";

        // 이동 생성자 호출
        MyClass moveClass = std::move(myClass);

        // std::move()는 RValue로 캐스팅하는 함수가 아닌 RValue Reference로 캐스팅하는 함수.
        // RValue Reference로 캐스팅하는 것에 의미를 두기 보다는 이동 생성자가 받는 인자 형식을 맞추기 위해서 사용한다고 봐야 함.
        // LValue Reference로 받은 다음에 std::move()를 호출해서 이동 생성자가 호출되게 유도하는 것도 가능하다(Function(MyClass& myClass) 쪽 확인).
        // 함수의 인자를 std::move()를 통해 RValue Reference로 넘긴다고 해도 사용할 때는 LValue이다.
        
        // std::move()는 이동 연산을 수행하지 않고 우측값으로 캐스팅하는게 전부인 함수이다.
        // 차라리 move()라는 이름 대신 rvalue()로 명명했으면 덜 헷갈렸을 것 같다.
        // Bjarne Stroustroup 아저씨의 실수...
        // 우측값 캐스팅을 수행하는 함수의 이름을 move()라고 지은 것을 후회했다고 한다.
    }

    std::cout << "--------------------\n";

    // 임시 객체를 RValue Reference로 받을 뿐이지 Move Constructor는 호출하지 않는다.
    // LValue Reference를 인자로 받을 때 Copy Constructor를 호출하지 않는 것과 같은 이치임.
    Function(MyClass{});

    return 0;
}
