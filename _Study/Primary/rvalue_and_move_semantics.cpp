// Update Date : 2024-10-10
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++14
// Configuration : Debug-x64, Release-x64

#include <iostream>

// https://en.cppreference.com/w/cpp/utility/move

// 보편 참조에 대한 내용은 structured_binding_with_auto.cpp를 참고할 것
// RValue에 대한 기본적인 테스트 코드는 simple_rvalue_test_cases.cpp를 참고할 것

// 1. LValue와 LValue Reference는 다른 개념이다.
// 2. RValue와 RValue Reference는 다른 개념이다.
// 3. RValue Reference와 LValue Reference는 다른 개념이다.
// 4. RValue Reference와 Move Semantics는 다른 개념이다.
// 5. RValue Reference는 RValue Reference Type과 RValue Reference Casting을 포함하는 개념이다.
//    - 다만 RValue Reference Type과 RValue Reference Casting은 구분해서 볼 필요가 있다(중요).

// # LValue
// - Assignment 식의 왼쪽에 위치한 하위식, 또는 식별자를 의미.
// - 환경에 저장되어 지속적으로 존재할 수 있음.
// - 환경의 메모리에 접근하여 식별자에 대응되는 주소에 접근하여 읽기와 쓰기 둘 다 가능함.
// - (언어마다 다른 사항) 변수나 객체는 RValue에 위치해 있어도 환경에 대응되니 LValue로 취급 가능함.
//
// # RValues
// - Assignment 식의 오른쪽에 위치한 하위식 및 값을 의미.
// - 레지스터나 피연산자 스택에 저장되어 일시적으로 존재할 수 있음.
// - 읽기만 가능함(식별자가 올 경우 환경에 있는 값을 가져오기만 하지 갱신하진 않음).
// - 일시적인 값이라 수명이 매우 짧음.
//   ex) 임시 객체, 리터럴, 일시적인 값, 표현식의 결과(중요), 함수 반환 값 등
//
// ## 쉽게 이해하는 차이점 ##
// - 환경에 저장되면 LValue, 레지스터나 피연산자 스택에 올라가 일시적으로 보존되는 것이라면 RValue.
// - 피연산자 스택의 일부를 환경으로 쓰는 경우가 있긴 해도 이 경우에는 바운드리가 분명하게 구분된다는 것을 염두할 것.
//
// # LValue Reference
// - & 기호를 사용하여 정의하는 일반적임 참조 형태임.
// - LValue Reference는 참조 타입이지만 환경에 따로 저장된다(최적화를 끄고 디스어셈블리로 확인한 부분).
//   - 환경에 저장되는 식별자는 LValue로 취급한다.
// - ex) int& ref = x; // int&는 LValue Reference Type, ref는 LValue, x는 RValue
//
// # RValue Reference
// - RValue Reference Type과 RValue Reference Casting으로 구분해서 보는 것이 편함(자료형과 캐스팅을 구분해서 보기).
// - RValue를 참조하여 임시 객체의 수명을 연장하는 것을 가능케 함.
// - 자원을 복사하지 않고 이전(이동)시키는 컨텍스트에서 사용함.
// - 임시 객체가 가진 자원의 소유권을 양도하는 Move Semantics를 구현하기 위해 필요한 개념임(불필요한 복사 최소화).
// - 임시 객체가 가진 자원의 소유권을 이전(이동)하는 것으로 자원을 효율적으로 관리할 수 있음.
//   - 이때 소유권을 양도한 객체는 자원에 대한 소유권을 상실함.
// - && 기호를 사용해서 정의함.
//
// # RValue Reference Type(RValue Reference와 내용이 일부 겹침)
// - RValue를 참조하기 위한 참조 타입임(자료형이라는 뜻).
// - 임시 객체 혹은 곧 소멸이 예정된 객체가 속하는 유형인 RValue를 참조하기 위해서 사용함.
// - 변수 선언이나 함수의 매개변수로 넘기는 방식으로 사용할 수 있음.
// - RValue Reference Type은 타입이기에 환경에 저장될 수 있다(최적화를 끄고 디스어셈블리로 확인한 부분).
//   - LValue Reference와 마찬가지로 환경에 저장되는 식별자는 LValue로 취급하니 혼동하지 말 것.
// - 환경에 저장된 RValue Reference Type은 환경이란 저장소의 메모리 주소와 대응됨(타입 -> 자료형).
// - 환경에 저장된 RValue Reference Type은 LValue이기에 참조한 값을 수정할 수 있음(중요).
//  code)---------------------------------------------
//  // int&&는 RValue Reference Type, rref는 LValue, 10은 RValue
//  // 상수 10을 직접 참조하는 것이 아닌 표현식을 평가하여 생성한 임시 값 10을 참조하는 것(의미가 다르니 혼동주의)
//  int&& rref = 10;
//  rref = 200; // 수정 가능(환경에 저장된 LValue로 보기 때문이라고 생각하자).
// 
//  // 에러 발생(캐스팅하는 순간은 LValue가 아닌 RValue로 보기 때문이라고 생각하자).
//  (int&&)rref = 300;
// 
//  int testNum = 400;
// 
//  ReadFunc(100); // int&&
//  ReadFunc(rref); // ## int& ##
//  ReadFunc(std::move(rref)); // int&&
//  ReadFunc(testNum); // int&
//  --------------------------------------------------
//
// # RValue Reference Casting(RValue Reference와 내용이 일부 겹침)
// - 객체를 RValue Reference Type으로 변환하는 작업을 의미.
// - Move Semantics를 활성화하기 위한 목적으로 사용(중요).
//   - RValue Reference Casting에 영향을 받는 식의 결과는 RValue로 취급함.
// - RValue를 받는 함수를 호출하기 위한 용도나 객체의 소유권을 실질적으로 이전하기 위해서 사용함(중요).
// - std::move(), static_cast<T&&>(), 강제형변환을 통해 변환을 수행함.
//  code)---------------------------------------------
//  int x = 10;
//  int&& rref1 = std::move(x);
//  int&& rref2 = static_cast<int&&>(x);
//  int&& rref3 = (int&&)x;
//  --------------------------------------------------
//
// ## 참조 타입과 최적화 ##
// - 참조 타입은 환경에 별도의 공간을 할당하는 방식으로 동작한다.
//   하지만 최적화 수준 혹은 컴파일러의 구현 여부에 따라 별도의 공간을 할당하는 방식이 아닌
//   참조 대상에 대응되는 환경의 메모리를 그대로 사용하여 코드를 생성할 가능성도 있음을 망각해선 안 된다.
//
// ## 추가적으로 학습해서 익혀야 하는 사항 ##
// - RValue Reference Type(T&&)은 보편 참조를 통한 완벽한 전달(Perfect Forwarding)을 사용할 목적으로도 활용됨.
//   - 완벽한 전달은 함수 템플릿 사용 시 매개변수를 원래의 참조 유형으로 전달하는 방식을 말함.
//   - 템플릿이 LValue와 RValue를 구분 없이 처리하게 하는 방식이 완벽한 전달임.
//   - 관련 함수는 std::forward().
// - const로 지정된 LValue는 수정할 수 없음.
// - 참조 타입은 반드시 초기화까지 이루어져야 함.
// - Move Semantics에는 noexcept를 붙여주는 것이 권장됨.
//   - std::vector와 같은 C++ STL의 컨테이너들은 예외에 대한 안전성을 보장하기 위해 noexcept로 선언되어 있는 경우에만 move()를 적용함(중요).
//   - STL에선 noexcept가 적용되어 있지 않다면 Copy Semantics를 사용함(중요).
//   - noexcept를 적용하면 move()에서 예외를 던지지 않는다는 것이 보장되기에 성능 향상을 기대할 수 있음.

// # Copy Semantics
// - 복사 생성자
// - 복사 대입 연산자
// - LValue 또는 LValue Reference를 인자로 받음.
// - 대상을 복사하기 위해서 사용함.
//
// # Move Semantics
// - 이동 생성자
// - 이동 대입 연산자
// - RValue Reference를 인자로 받음.
// - 대상의 소유권을 이전하기 위해서 사용함.
//
// ## 중요한 사항 ##
// - Move는 복사가 아닌 소유권의 이전임.

// std::move() 구현 코드
// --------------------------------------------------
// template <class _Ty>
// _NODISCARD constexpr remove_reference_t<_Ty>&& move(_Ty&& _Arg) noexcept { 
//     return static_cast<remove_reference_t<_Ty>&&>(_Arg);
// }
// --------------------------------------------------
// * constexpr 유형 : 컴파일 타임에 완료될 수 있음.
// * noexcept 유형 : 예외를 던지지 않는 함수임.
// --------------------------------------------------
// * 반환형(remove_reference_t<_Ty>&&) : _Ty에서 참조를 제거한 기본 타입의 RValue Reference임.
//   - LValue Reference(&)든, RValue Reference(&&)든 참조를 제거하여 원래 타입을 구한 후 RValue Reference로 만듦.
//
// * 템플릿 매개변수(_Ty&& _Arg) : 완벽한 전달을 위한 보편 참조로 LValue Reference와 RValue Reference를 모두 받을 수 있음.
//   - 템플릿 함수의 타입을 대상으로 하고 있기에 &&는 보편 참조를 의미하지, RValue Reference Type이 아니니까 주의할 것.
//
// * 구현부의 반환 코드(return static_cast<remove_reference_t<_Ty>&&>(_Arg);)
//   - _Arg를 _Ty에서 참조를 제거한 기본 타입의 RValue Reference로 캐스팅하여 반환함.
// --------------------------------------------------
// std::move()는 인자로 받은 대상을 RValue Reference로 캐스팅하여 반환하는 함수이다.
// RValue가 아닌 RValue Reference로 캐스팅하는 함수니까 차이를 인지해야 한다(RValue와 RValue Reference는 다르다).
//
// 개발자는 std::move() 여부를 통해 Copy Semantics를 사용할지 Move Semantics를 사용할지 결정할 수 있다.
// RValue Reference Type이 환경에 저장되면 이건 LValue로 취급하니 주의할 것.
// --------------------------------------------------
// 보편 참조는 완벽한 전달(Perfect Forwarding)을 처리하기 위한 개념이며 전달된 인수의 유형에 따라 LValue Reference나 RValue Reference 중 하나로 해석됨.
// 보편 참조를 위한 기호는 &&로 RValue Reference를 표현하기 위한 기호 &&와 동일하며,
// 처음 마주하면 굉장히 혼동되는 개념이기 때문에 structured_binding_with_auto.cpp를 함께 보도록 할 것.
// --------------------------------------------------
// # 굳이 std::move()를 사용하지 않더라도 RValue라는 것이 확실하다면 RValue Reference로 받는다.
// # MyClass는 임시 객체(일회용) -> RValue Reference(보편 참조) -> push_back() 내부에서 forward()로 전달 -> Move Constructor
// vec.push_back(MyClass{ });
// --------------------------------------------------

// RValue와 RValue Reference는 다른 개념이니까 반드시 구분하고 넘어갈 것.
// RValue는 Assignment 식의 오른쪽에 있는 값을 의미하고, RValue Reference는 어떠한 임시 값을 참조하는 형태를 의미함.
//
// std::move()는 인자로 받은 대상을 RValue Reference로 캐스팅하여 반환함.
// 임시 값을 참조하는 것과 RValue Reference로 캐스팅하는 것은 의미가 미묘하게 다르니 짚고 넘어갈 것.

// RValue Reference는 명칭 그대로 RValue를 참조하기 위한 타입이며,
// 이동 생성자는 RValue의 소유권을 이전하기 위해서 사용하는 생성자임.
// 이동 생성자는 RValue Reference를 인자로 하는데 주의해야 할 점이 있다.
// 바로 환경에 저장된 RValue Reference Type의 식별자는 LValue로 취급하기에 이걸 넘기면 복사 생성자가 호출되니 이 부분을 꼭 기억해야 한다.
//
// std::move()는 단순히 캐스팅 함수일 뿐이며 이것을 호출하는 것만으로는 소유권의 상실까지 이어지진 않는다.
// 자원의 소유권을 이전(이동)해서 이전 객체가 가진 자원의 소유권을 상실시키는 로직은 직접 작성해줘야 한다.
// --------------------------------------------------
// MyClass(MyClass&& other) noexcept : data(other.data) {
//     other.data = nullptr; // 이동 후 원래 객체는 자원의 소유권을 상실
//     std::cout << "Move Constructor" << std::endl;
// }
//
// MyClass& operator=(MyClass&& other) noexcept {
//     if (this != &other) {
//         delete data; // 기존 자원을 해제
//         data = other.data;
//         other.data = nullptr; // 이동 후 원래 객체는 자원의 소유권을 상실
//         std::cout << "Move Assignment" << std::endl;
//     }
//     return *this;
// }
// --------------------------------------------------

// RValue Reference를 환경에 저장했다고 해서 이걸 곧바로 Move Semantics에 적용할 수 있는 것은 아니다.
// 환경에 저장된 RValue Reference는 LValue이다.
// 따라서 RValue Reference를 변수로 저장하여 식별자를 함수의 인자로 전달하면 LValue로 받는다.
// 식별자를 함수의 인자로 전달하되 RValue로 전달하고 싶다면 반드시 std::move()를 써야 한다.

// @@ 진짜 진짜 중요(가끔 망각해서 짤막하게 정리함) @@
// LValue != LValue Reference
// RValue != RValue Reference
// RValue Reference != Move Semantics
// RValue Reference = RValue Reference Type + RValue Reference Casting
//
// LValue는 대입 연산자 기준으로 왼쪽에 오는 것 혹은 환경에 저장되어 원하는 시점에서 재사용할 수 있는 것이고,
// RValue는 대입 연산자 기준으로 오른쪽에 오는 것이거나 값이 일시적인 것이다.
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
// - 둘 다 어떠한 것을 참조하기 위해서 사용함.
// - LValue를 참조하면 LValue Reference이고, RValue를 참조하면 RValue Reference임.
// - LValue라도 std::move()를 적용하면 RValue Reference로 캐스팅해서 사용할 수 있음.
//   - 이는 원하는 시점에 소유권의 이전을 유도할 수 있다는 것을 의미함.
// - LValue Reference든, RValue Reference든 환경에 저장되면 변수로 간주함(LValue로 취급하겠다는 의미).

class MyClass
{
public:
    explicit MyClass()
    {
        _id = s_Id++;

        std::cout << "Constructor : id(" << _id << ")\n";
    }

    ~MyClass()
    {
        std::cout << "Destructor: id(" << _id << ")\n";
    }

public:
    explicit MyClass(const MyClass& rhs)
    {
        this->_id = s_Id++;

        std::cout << "Copy Constructor : id(" << _id << ")\n";
    }

    MyClass& operator=(const MyClass& rhs)
    {
        // 자기 자신에 대한 할당을 진행하는 경우
        if (&rhs == this)
            return *this;

        this->_id = s_Id++;

        std::cout << "Copy Assignment : id(" << _id << ")\n";

        return *this;
    }

public:
    explicit MyClass(const MyClass&& rhs) noexcept
    {
        this->_id = s_Id++;

        std::cout << "Move Constructor : id(" << _id << ")\n";
    }

    MyClass& operator=(const MyClass&& rhs) noexcept
    {
        // 자기 자신에 대한 할당을 진행하는 경우
        if (&rhs == this)
            return *this;

        this->_id = s_Id++;

        std::cout << "Move Assignment : id(" << _id << ")\n";

        return *this;
    }

private:
    static int s_Id;

private:
    int _id;

public:
    int _val = 0;
};

int MyClass::s_Id = 0;

class RAII
{
public:
    RAII() { std::cout << "### RAII() ###\n"; }
    ~RAII() { std::cout << "### ~RAII() ###\n"; }
};

void CallFunc(MyClass& myClass)
{
    RAII test;

    std::cout << "CallFunc() : LValue Reference Type\n";

    // myClass는 함수의 환경에 저장되어 있는 식별자이기에 LValue로 사용한다.
    // ## LValue Reference Type과 LValue는 다르니 주의하도록 함 ##
    MyClass whatSemantics{ myClass }; // 복사 생성자

    // LValue Reference로 받아도 RValue Reference Casting을 적용해 이동 생성자를 호출할 수 있다.
    MyClass moveClass{ std::move(myClass) }; // 이동 생성자

    whatSemantics = myClass; // 복사 대입 연산자
    whatSemantics = std::move(myClass); // 이동 대입 연산자
}

void CallFunc(MyClass&& myClass)
{
    RAII test;

    std::cout << "CallFunc() : RValue Reference Type\n";

    // RValue Reference Type으로 받았다고 해서 Move Semantics를 사용하는 것은 아니다.
    // myClass가 RValue Reference Type이긴 하나 함수의 환경에 저장되었으니 식별자로 사용할 때는 LValue이다(!!!진짜진짜 중요중요!!!)
    MyClass whatSemantics{ myClass }; // ## 복사 생성자 ##

    // 환경에 저장되어 식별자로 접근하는 방식이라면 일단 LValue라고 생각하자.
    // 따라서 RValue Reference Type이라고 해도 다음과 같이 RValue Reference Casting을 거쳐야 한다.
    MyClass moveClass{ std::move(myClass) };

    // 환경에 저장된 RValue Reference가 RValue라면 _val의 값을 갱신할 수 없어야 한다.
    // 하지만 RValue Reference로 받았다고 해도 사용할 때는 LValue이니 다음 코드는 문제 없이 통과한다.
    myClass._val = 1000; // 컴파일 통과

    whatSemantics = myClass; // 복사 대입 연산자
    whatSemantics = std::move(myClass); // 이동 대입 연산자

    // RValue Reference Type으로 받는 것 또한 참조 형식이기 때문에 myClass에 대한 소멸자는 호출되지 않는다.
}

int main()
{
    // ## 중단점 걸고 하나씩 천천히 확인할 것 ##

    {
        std::cout << "1) ---------------------------\n";

        MyClass myClass;

        // 전형적인 LValue Reference 예제
        CallFunc(myClass);
    }

    std::cout << "--------------------------------------------------\n";
    
    {
        std::cout << "2) ---------------------------\n";

        MyClass myClass;

        // std::move()를 사용해서 RValue Reference 캐스팅을 유도하여 Move Semantics를 활성화시킬 수 있다.
        // std::move()는 RValue Reference로 캐스팅하는 함수이지 이동 생성자가 적용된 새로운 객체를 만드는 함수가 아니니 주의할 것.
        CallFunc(std::move(myClass));
    }
    
    std::cout << "--------------------------------------------------\n";

    {
        std::cout << "3) ---------------------------\n";

        // 태생이 RValue라면 캐스팅 과정을 거치지 않아도 RValue Reference로 받는다.
        // 임시 객체를 RValue Reference, 즉 참조 형식으로 받는 것이기 때문에 Move Constructor는 호출되지 않는다.
        // LValue Reference로 인자를 받을 때 Copy Constructor를 호출하지 않는 것과 같은 이치임.
        CallFunc(MyClass{ });
    }

    std::cout << "--------------------------------------------------\n";

    {
        std::cout << "4) ---------------------------\n";

        // 기본 생성자 호출
        MyClass myClass = MyClass{ }; // 이것도 함수를 명시적으로(explicitly) 초기화하는 방식 중 하나다(암시적인 초기화가 아니니 혼동주의).

        // 복사 생성자 호출
        MyClass copyClass{ myClass };

        // 이동 생성자 호출
        MyClass moveClass{ std::move(myClass) };

        /**
         * ## 짚고 넘어갈 것 ##
         * 
         * std::move()는 RValue로 캐스팅하는 함수가 아닌 RValue Reference로 캐스팅하는 함수이다(상단의 std::move() 구현 코드를 분석한 주석 참고).
         * RValue Reference로 캐스팅하는 것은 이동 생성자가 받는 인자 형식인 RValue Reference Type을 맞추기 위해서 사용하는 것이라 봐도 된다.
         *
         * LValue Reference로 참조했다고 해도 std::move()를 쓰면 RValue Reference로 캐스팅하여 이동 생성자가 호출되게 유도할 수 있다(CallFunc(MyClass& myClass) 참고).
         * 함수의 인자를 std::move()를 통해 RValue Reference로 캐스팅하여 받았다고 해도 환경에 저장되는 순간은 LValue로 취급한다.
         * 
         * --------------------------------------------------
         * 
         * std::move()는 이동 연산을 수행하는 것이 아닌 단순하게 우측값으로 캐스팅하는 것이 전부인 함수이다.
         * 차라리 move() 대신 명칭을 rvalue() 혹은 cast_rvalue()로 지었다면 덜 헷갈렸을 것이다.
         * 
         * C++의 창시자 Bjarne Stroustroup은 우측값 캐스팅을 수행하는 함수의 이름을 move()라고 지은 것을 후회한다고 언급한 적이 있다.
         */
    }

    std::cout << "--------------------------------------------------\n";

    return 0;
}
