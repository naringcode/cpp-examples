#include <iostream>
#include <vector>

// 프로그래머가 move semantics를 사용할지 말지 결정하고 싶을 때?
// std::move() 사용하기

// Move는 복사가 아닌 소유권 이전이다.

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

            delete _data;
            _data = nullptr;
        }
        else
        {
            std::cout << "nullptr\n";
        }
    }

public:
    MyClass(const MyClass& myClass)
    {
        std::cout << "Copy Constructor : MyClass(MyClass& myClass)" << '\n';

        if (nullptr != myClass._data)
        {
            this->_data = new int{ *(myClass._data) };
        }
    }

    MyClass& operator=(const MyClass& myClass)
    {
        std::cout << "Copy Assignment : MyClass& operator=(MyClass& myClass)" << '\n';

        // 자기 자신에 대한 할당을 진행하는 경우
        if (&myClass == this)
            return *this;

        if (nullptr != myClass._data)
        {
            this->_data = new int{ *(myClass._data) };
        }
    }

public:
    // noexcept를 주석처리하면 vector reallocation 작업이 일어날 때 Copy Constructor가 호출됨.
    MyClass(MyClass&& myClass) // noexcept
        : _data(myClass._data)
    {
        std::cout << "Move Constructor : MyClass(MyClass&& myClass)" << '\n';

        myClass._data = nullptr;
    }

    // noexcept를 주석처리하면 vector reallocation 작업이 일어날 때 Copy Constructor가 호출됨.
    MyClass& operator=(MyClass&& myClass) // noexcept
    {
        std::cout << "Move Assignment : MyClass& operator=(MyClass&& myClass)" << '\n';

        // 자기 자신에 대한 할당을 진행하는 경우
        if (&myClass == this)
            return *this;

        if (nullptr != _data)
        {
            delete _data;
        }

        _data = myClass._data;

        myClass._data = nullptr;

        return *this;
    }

private:
    int* _data = nullptr;
};

MyClass make_my_class(int data)
{
    // MyClass의 소멸자 호출 시점을 확인하기 위한 RAII 클래스
    class RAIITest
    {
    public:
        ~RAIITest()
        {
            std::cout << "~~RAII Test~~\n";
        }
    } test;

    MyClass myClass(new int{ data });

    return myClass;
}

MyClass make_my_class_rvalue(int data)
{
    // MyClass의 소멸자 호출 시점을 확인하기 위한 RAII 클래스
    class RAIITest
    {
    public:
        ~RAIITest()
        {
            std::cout << "~~RAII Test~~\n";
        }
    } test;

    MyClass myClass(new int{ data });

    return std::move(myClass);
}

int main()
{
    using namespace std;

    {
        // 내 기억으론 함수가 반환하는 순간 메모리가 새로 할당되어서 복사 생성자가 호출되어야 하는데 그렇지 않음.
        // C++ 표준에서 제공하는 기능이 아닌 컴파일러에서 제공하는 기능이라 컴파일러의 최적화 수준에 영향을 받음.
        // RVO(Return Value Optimization)를 찾아보도록 하자.
        // RVO는 복사 생성자나 이동 생성자의 호출을 피하기 위해서 사용되는 최적화 기법이다.
        //
        // ----- Visual Studio 2015/2019 Debug -----
        // Constructor : MyClass(int* data)
        // Move Constructor : MyClass(MyClass&& myClass)
        // Destructor : ~MyClass()
        // ~~RAII Test~~
        //
        // ----- Visual Studio 2022 Debug -----
        // Constructor : MyClass(int* data)
        // ~~RAII Test~~
        // 
        // ----- Visual Studio 2015/2019/2022 Release -----
        // Constructor : MyClass(int* data)
        // ~~RAII Test~~
        // 
        MyClass myClassA = make_my_class(10);

        cout << "----\n";

        // Copy Constructor가 아닌 Move Constructor 발생
        MyClass myClassB(std::move(myClassA));

        cout << "----\n";
    }

    std::cout << "--------------------\n";

    {
        MyClass myClassA = make_my_class(10);

        cout << "----\n";

        // Copy Assignment가 아닌 Move Assignment 발생
        // 1. Constructor
        // 2. Move Assignment
        MyClass myClassB;
        myClassB = std::move(myClassA);

        cout << "----\n";
    }

    std::cout << "--------------------\n";

    {
        MyClass myClassA = make_my_class(10);

        cout << "----\n";

        // --- RVO가 적용되어도 다음과 같이 호출된다 -----
        // Constructor : MyClass(int* data)
        // Move Constructor : MyClass(MyClass&& myClass)
        // Destructor : ~MyClass() : nullptr
        // ~~RAII Test~~
        //
        // std::move()를 명시해서 RValue에 의한 Move Constructor를 유도했기 때문에 그런 것이다.
        // 가능하면 컴파일러를 믿고 make_my_class()처럼 그냥 반환하는 형태로 사용하자.
        MyClass myClassB = make_my_class_rvalue(100);

        cout << "----\n";
    }

    std::cout << "--------------------\n";

    {
        std::vector<MyClass> vec;

        // vec.reserve(20)를 주석처리하면 재밌는 현상을 볼 수 있음
        vec.reserve(20);

        // LValue Reference 버전 확인 용도
        auto myClass = make_my_class(10000);

        cout << "----\n";

        // LValue Reference -> forward() -> "Copy Constructor"
        vec.push_back(myClass); 

        for (int i = 0; i < 9; i++)
        {
            // 이 부분도 Return Value Optimization이 적용된다.
            // 1. make_my_class() -> MyClass myClass(new int{ data }) -> "Constructor"
            // 2. make_my_class() -> "~~RAII Test~~"
            // 3. main - push_back() -> RValue Reference -> forward() -> "Move Constructor"
            // 4. main - make_my_class()가 반환한 MyClass -> "Destructor"
            //
            // RVO가 적용되지 않을 경우에는 다음과 같이 호출된다.
            // 1. make_my_class() -> MyClass myClass(new int{ data }) -> "Constructor"
            // 2. make_my_class() -> return -> "Move Constructor"
            // 3. make_my_class() -> myClass -> "Destructor"
            // 4. make_my_class() -> "~~RAII Test~~"
            // 5. main - push_back() -> RValue Reference -> forward() -> "Move Constructor"
            // 6. main - make_my_class()가 반환한 MyClass -> "Destructor"
            //
            cout << "---\n";

            vec.push_back(make_my_class(10 + i));

            cout << "---\n";

            vec.push_back(std::move(make_my_class(100 + i)));

            // 위 두 코드는 같은 동작을 한다.
            // 명시적으로 std::move()를 타이핑하지 않아도 RValue Reference를 받는 push_back()를 호출하기 때문이다.
            // 
            // push_back()은 LValue Reference와 RValue Reference를 받는 것을 구분하고 있다.
            // 다음 코드의 경우엔 LValue Reference로 받아서 Copy Constructor가 호출된다.
            // ----------------------------------------
            // auto myClass = make_my_class(100);
            // vec.push_back(myClass); // LValue Reference -> forward() -> "Copy Constructor"
            // ----------------------------------------
            // 
            // ##### vec.push_back(make_my_class(10 + i)); #####
            // 
            // Case A) 2015_Debug, 2019_Debug
            // 1. Constructor : MyClass(int* data)
            // 2. Move Constructor : MyClass(MyClass&& myClass)
            // 3. Destructor : ~MyClass() : nullptr
            // 4. ~~RAII Test~~
            // 5. Move Constructor : MyClass(MyClass&& myClass)
            // 6. Destructor : ~MyClass() : nullptr
            // 
            // Case B) 2015_Release, 2019_Release, 2022_Debug, 2022_Release
            // 1. Constructor : MyClass(int* data)
            // 2. ~~RAII Test~~
            // 3. Move Constructor : MyClass(MyClass&& myClass)
            // 4. Destructor : ~MyClass() : nullptr
            //
            // ##### vec.push_back(std::move(make_my_class(100 + i))); #####
            // 
            // Case A) 2015_Debug, 2019_Debug
            // 1. Constructor : MyClass(int* data)
            // 2. Move Constructor : MyClass(MyClass&& myClass)
            // 3. Destructor : ~MyClass() : nullptr
            // 4. ~~RAII Test~~
            // 5. Move Constructor : MyClass(MyClass&& myClass)
            // 6. Destructor : ~MyClass() : nullptr
            //
            // Case B) 2015_Release, 2019_Release, 2022_Debug, 2022_Release
            // 1. Constructor : MyClass(int* data)
            // 2. ~~RAII Test~~
            // 3. Move Constructor : MyClass(MyClass&& myClass)
            // 4. Destructor : ~MyClass() : nullptr

            // 컴파일러 버전에 따라 다르지만 보통은 std::move()를 쓰지 않고 그냥 넘겼을 때가 훨씬 나은 성능을 보인다.
            // RVO가 적용되지 않는다면 반환에 따른 추가적인 생성자가 호출되긴 하지만 보통은 적용되니 이 부분은 크게 신경쓸 부분이 아니다.
            //
            // 프로젝트의 기본 속성에 맞춰서 진행한 것이다.
            // 최적화 수준에 따라 다르게 동작할 수 있으니 이 부분은 주의하자.
            //
        }

        cout << "----\n";
    }

    return 0;
}
