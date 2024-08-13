#include <iostream>
#include <vector>

// ## Move는 복사가 아닌 소유권 이전이다. ##

// RVO(Return Value Optimization)와 Move 연산 전반에 대한 내용 연습하기.
// 이 둘이 혼합되었을 때 어떤 현상이 발생하는지에 대한 부분도 학습하자.

// RVO는 함수의 값을 반환할 때 복사 생성자나 이동 생성자의 호출을 피하기 위해 사용하는 최적화 기법.
// C++ 표준에서 지원하는 것이 아닌 컴파일러가 지원하는 기능이다.
//
// 최적화 설정에 따라 그리고 컴파일러의 지원 여부에 따라 RVO가 동작하지 않을 수 있으니 주의하자.
//
// ## Debug/Release 모드가 중요한 것이 아닌 세부적으로 설정된 최적화 설정에 영향을 받음 ##

class MyClass
{
public:
    MyClass()
    {
        std::cout << "Constructor : MyClass()" << '\n';
    }

    MyClass(int* data)
        : _data(data)
    {
        std::cout << "Constructor : MyClass(int* data)" << '\n';
    }

public:
    ~MyClass()
    {
        std::cout << "Destructor : ~MyClass() : ";

        if (nullptr != _data)
        {
            std::cout << *_data << '\n';

            // 예시 코드에서 반환형을 Reference로 했을 때 문제가 생기는 부분.
            // 반환형이 참조 형식이라면 자원을 해제한 객체를 인자로 받는 복사 생성자 혹은 이동 생성자를 호출하는 문제가 발생한다.
            delete _data;
            _data = nullptr;
        }
        else
        {
            std::cout << "nullptr\n";
        }
    }

public:
    MyClass(const MyClass& rhs)
    {
        std::cout << "Copy Constructor : MyClass(MyClass& rhs)" << '\n';

        if (nullptr != rhs._data)
        {
            this->_data = new int{ *(rhs._data) };
        }
    }

    MyClass& operator=(const MyClass& rhs)
    {
        std::cout << "Copy Assignment : MyClass& operator=(MyClass& rhs)" << '\n';

        // 자기 자신에 대한 할당을 진행하는 경우
        if (&rhs == this)
            return *this;

        if (nullptr != rhs._data)
        {
            this->_data = new int{ *(rhs._data) };
        }
    }

public:
    // noexcept를 주석처리하면 vector reallocation 작업이 일어날 때 Copy Constructor가 호출됨.
    MyClass(MyClass&& rhs) // noexcept
        : _data(rhs._data)
    {
        std::cout << "Move Constructor : MyClass(MyClass&& rhs)" << '\n';

        rhs._data = nullptr;
    }

    // noexcept를 주석처리하면 vector reallocation 작업이 일어날 때 Copy Constructor가 호출됨.
    MyClass& operator=(MyClass&& rhs) // noexcept
    {
        std::cout << "Move Assignment : MyClass& operator=(MyClass&& rhs)" << '\n';

        // 자기 자신에 대한 할당을 진행하는 경우
        if (&rhs == this)
            return *this;

        if (nullptr != _data)
        {
            delete _data;
        }

        _data = rhs._data;

        rhs._data = nullptr;

        return *this;
    }

private:
    int* _data = nullptr;
};

// MyClass의 소멸자 호출 시점을 확인하기 위한 RAII 클래스
class RAII
{
public:
    // RAII() { std::cout << "### RAII() ###\n"; }
    ~RAII() { std::cout << "### ~RAII() ###\n"; }
};

/**
 * 참조 형식 자체는 복사 생성자나 이동 생성자의 호출과는 관련이 없다.
 * 말 그대로 참조만 할 뿐이기에 객체를 생성하지 않는다는 뜻이다.
 * 반환형을 일반 타입으로 지정해야 복사 생성자가 호출되든 이동 생성자가 호출되든 한다.
 * 
 * 참조 형식으로 반환할 생각이라면 함수의 환경을 벗어나도 객체의 소멸자가 호출되지 않는,
 * 객체의 상태가 계속해서 유지될 수 있는 그런 조건을 충족할 때에만 써야 한다(클래스 객체 자신을 *this로 반환하거나 클래스 멤버 변수를 반환하는 등).
 * 
 * 반환형이 참조 형식이라면 함수의 환경을 벗어났을 때 소멸자가 호출된 객체를 참조하게 돼서 문제가 생긴다.
 * 엄밀히 말해서 반환된 객체를 복사하는데 소멸자가 호출된 객체를 복사하려고 하니 문제가 발생한다.
 * 
 * 이건 LValue Reference든, RValue Reference든 모든 참조 형식에 적용되는 주의사항이다.
 * 
 * 이를 순서로 정리하면 다음과 같은 일이 순차적으로 발생한다.
 * 1. 함수에서 결과를 반환하면 반환값을 대상으로 복사나 이동 또는 참조가 이루어진다.
 * 2. 함수의 몸체를 비우는 과정에서 함수의 환경에 저장된 객체는 소멸자를 호출한다.
 * 3. 만약 반환값이 참조 형식으로 되어 있는데 해당 객체의 순환주기가 함수의 환경에 한정되어 있을 경우 참조되는 대상은 소멸한다.
 * 4. 함수의 반환을 마치면 참조된 대상을 복사하든 이동하든 해야 하는데 여기서 문제가 발생한다(참조 대상은 소멸되었기 때문).
 * 
 * (Bad) MyClass& make_my_class(int data);
 * (Bad) MyClass&& make_my_class(int data);
 * (Good) MyClass make_my_class(int data);
 */
// MyClass& make_my_class(int data)
MyClass make_my_class(int data)
{
    RAII test;

    MyClass myClass{ new int{ data } };

    return myClass;
}

// MyClass&& make_my_class_rvalue(int data)
MyClass make_my_class_rvalue(int data)
{
    RAII test;

    MyClass myClass{ new int{ data } };

    // 이 방식에 대한 설명은 예시 코드의 주석을 참고할 것.
    return std::move(myClass);
}

int main()
{
    using namespace std;

    {
        // 내 기억으로는 함수가 반환하면 반환 값을 대상으로 하는 복사 생성자가 호출되어야 하는데 그렇지 않음.
        // 이건 RVO 기능 때문에 그런 것인데 이건 C++ 표준에서 제공하는 것이 아닌 컴파일러가 제공하는 기능이라 컴파일러의 최적화 수준에 영향을 받음.
        // RVO는 쉽게 말해서 복사 생성자나 이동 생성자의 호출을 피하기 위해서 사용하는 최적화 기법이다.
        // RVO(Return Value Optimization)에 대한 자료는 직접 찾아보도록 하자.
        
        // ----- Visual Studio 2015/2019 Debug ----- | RVO가 적용되지 않음.
        // 1. Constructor : MyClass(int* data)
        // 2. Move Constructor : MyClass(MyClass&& rhs)
        // 3. Destructor : ~MyClass()
        // 4. ### ~RAII() ###
        //
        // ----- Visual Studio 2022 Debug ----- | RVO가 적용됨.
        // 1. Constructor : MyClass(int* data)
        // 2. ### ~RAII() ###
        // 
        // ----- Visual Studio 2015/2019/2022 Release ----- | RVO가 적용됨.
        // 1. Constructor : MyClass(int* data)
        // 2. ### ~RAII() ###
        //
        MyClass myClassA = make_my_class(10);

        cout << "----\n";

        // Copy Constructor가 아닌 Move Constructor 발생
        MyClass myClassB(std::move(myClassA));

        cout << "----\n"; // Destructor 경계
    }

    std::cout << "--------------------\n";

    {
        MyClass myClassA = make_my_class(10);

        cout << "----\n";

        // 환경에 생성된 MyClass를 대상으로 Assignment를 적용하는 예시.
        //
        // 1. Constructor : MyClass()
        // 2. Copy Assignment : MyClass& operator=(MyClass& rhs)
        // 3. Move Assignment : MyClass& operator=(MyClass&& rhs)
        MyClass myClassB;

        myClassB = myClassA;
        myClassB = std::move(myClassA); // LValue를 대상으로 이동 대입 연산자를 호출하고 싶다면 명시적으로 std::move()를 써야 한다.

        cout << "----\n"; // Destructor 경계
    }

    std::cout << "--------------------\n";

    {
        // ## RVO가 적용되었어도 다음과 같이 출력된다. ##
        // 1. Constructor : MyClass(int* data)
        // 2. Move Constructor : MyClass(MyClass&& rhs)
        // 3. Destructor : ~MyClass() : nullptr
        // 4. ### ~RAII() ###
        // 
        // 반환할 때 std::move()를 명시해서 RValue를 인자로 받는 Move Constructor의 호출을 유도했기 때문에 그런 것이다.
        // 가능하면 컴파일러의 성능을 믿고 Reference Type으로 반환하는 것은 지양하도록 한다.
        MyClass myClassA = make_my_class_rvalue(100);

        cout << "----\n"; // Destructor 경계

        // 컴파일러 버전에 따라 약간 상이하지만 보통은 함수의 결과를 반환할 때 std::move()를 쓰지 않고 그냥 넘겼을 때 더 나은 성능을 보인다.
        // RVO를 의식하지 않는다면 "복사 생성자보단 이동 생성자를 호출해서 넘기면 더 좋지 않을까?"하는 함정에 빠질 수 있다.
        //
        // 다만 이건 컴파일러의 최적화 수준에 따라서 다르게 동작할 수 있는 부분이다.
        // 따라서 세부적인 최적화는 머신에 따라, 컴파일러에 따라 달라질 수 있음을 고려해야 한다.
    }

    std::cout << "--------------------\n";

    {
        std::vector<MyClass> vec;

        // 공간을 예약하는 vec.reserve(20)를 주석처리하면 재밌는 현상을 볼 수 있음.
        // 
        // 이 부분은 vector reallocation과 관련이 있다(더 나아가서 STL 컨테이너와 관련되어 있는 부분임).
        // STL 컨테이너는 예외 안정성을 보장하기 위해 noexcept가 적용되어 있는 경우에만 Move Semantics를 사용한다.
        // noexcept를 적용하지 않으면 예외 안정성을 보장하지 않으니 vector와 같이 원소를 재배치하는 그런 과정이 발생했을 때,
        // Move Semantics가 아닌 Copy Semantics를 사용한다.
        // 
        // 자세한 것은 vector_reallocation_by_copy_and_move.cpp를 참고할 것.
        vec.reserve(20);

        // push_back()의 LValue Reference 버전을 확인하기 위한 용도.
        //
        // * push_back()은 LValue Reference와 RValue Reference를 둘 다 받을 수 있게끔 함수 오버로딩이 적용되어 있음.
        // * emplace_back()은 전달한 가변 인자를 보편 참조 형식으로 넘기며 이 과정에서 std::forward()를 사용함.
        //
        MyClass myClass = make_my_class(10000);

        cout << "----\n";

        // 1. push_back() -> 함수 오버로딩 : LValue Reference -> "Copy Constructor"
        // 2. emplace_back() -> 보편 참조 : LValue Reference -> forward() -> "Copy Constructor"
        vec.push_back(myClass);
        // vec.emplace_back(myClass);

        for (int i = 0; i < 9; i++)
        {
            cout << "---\n";

            // 아래 push_back()도 Return Value Optimization 여부에 따라 다르게 동작함.
            //
            // * RVO가 적용되었을 경우
            // 1. make_my_class() -> MyClass myClass{ new int{ data } } -> "Constructor"
            // 2. make_my_class() -> "### ~RAII() ###"
            // 3. 함수 실행 종료
            // 4. main - push_back() -> 함수 오버로딩 : RValue Reference -> "Move Constructor"
            // 5. main - make_my_class()가 반환한 MyClass -> "Destructor"
            // 
            // * RVO가 적용되지 않았을 경우
            // 1. make_my_class() -> MyClass myClass{ new int{ data } } -> "Constructor"
            // 2. make_my_class() -> return -> "Move Constructor"
            // 3. make_my_class() -> myClass -> "Destructor"
            // 4. make_my_class() -> "### ~RAII() ###"
            // 5. 함수 실행 종료
            // 6. main - push_back() -> 함수 오버로딩 : RValue Reference -> "Move Constructor"
            // 7. main - make_my_class()가 반환한 MyClass -> "Destructor"
            //
            vec.push_back(make_my_class(10 + i));

            cout << "---\n";

            // 아래 코드는 위 vec.push_back()과 같은 동작을 한다.
            vec.push_back(std::move(make_my_class(100 + i)));

            // 위 두 코드는 같은 동작을 한다.
            // 명시적으로 std::move()를 타이핑하지 않아도 함수의 반환값은 RValue이기에 RValue Reference를 받는 push_back()을 호출하기 때문이다.
            //
            // push_back()은 LValue Reference와 RValue Reference를 받는 것을 함수 오버로딩으로 구현하여 구분하고 있다.
            //
            // 다음 코드처럼 함수의 반환값을 환경에 저장하여 push_back()에 식별자로 넘길 경우에는
            // LValue Reference로 받아서 Copy Constructor가 호출된다.
            // ----------------------------------------
            // auto myClass = make_my_class(100);
            // vec.push_back(myClass); // 함수 오버로딩 : LValue Reference -> "Copy Constructor"
            // ----------------------------------------
            // 
            // code : 함수의 반환값을 그대로 push_back()에 넘기기
            // vec.push_back(make_my_class(10 + i));
            // 
            // Case A) 2015_Debug, 2019_Debug
            // 1. Constructor : MyClass(int* data)
            // 2. Move Constructor : MyClass(MyClass&& rhs)
            // 3. Destructor : ~MyClass() : nullptr
            // 4. ### ~RAII() ###
            // 5. Move Constructor : MyClass(MyClass&& rhs)
            // 6. Destructor : ~MyClass() : nullptr
            // 
            // Case B) 2015_Release, 2019_Release, 2022_Debug, 2022_Release
            // 1. Constructor : MyClass(int* data)
            // 2. ### ~RAII() ###
            // 3. Move Constructor : MyClass(MyClass&& rhs)
            // 4. Destructor : ~MyClass() : nullptr
            // 
            // ----------------------------------------
            // 
            // code : 함수의 반환값에 std::move()를 적용하여 push_back()에 넘기기
            // vec.push_back(std::move(make_my_class(100 + i)));
            // 
            // Case A) 2015_Debug, 2019_Debug
            // 1. Constructor : MyClass(int* data)
            // 2. Move Constructor : MyClass(MyClass&& rhs)
            // 3. Destructor : ~MyClass() : nullptr
            // 4. ### ~RAII() ###
            // 5. Move Constructor : MyClass(MyClass&& rhs)
            // 6. Destructor : ~MyClass() : nullptr
            //
            // Case B) 2015_Release, 2019_Release, 2022_Debug, 2022_Release
            // 1. Constructor : MyClass(int* data)
            // 2. ### ~RAII() ###
            // 3. Move Constructor : MyClass(MyClass&& rhs)
            // 4. Destructor : ~MyClass() : nullptr
        }

        cout << "----\n";
    }

    return 0;
}
