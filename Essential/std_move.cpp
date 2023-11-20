#include <iostream>
#include <vector>

// 프로그래머가 move semantics를 사용할지 말지 결정하고 싶을 때?
// std::move() 사용하기

// Move는 복사가 아닌 소유권 이전이다.

class MyClass
{
public:
    explicit MyClass()
    {
        std::cout << "Default Constructor A" << '\n';
    }

    explicit MyClass(int* data)
        : _data(data)
    {
        std::cout << "Default Constructor B" << '\n';
    }

    ~MyClass()
    {
        std::cout << "Destructor" << '\n';

        if (nullptr != _data)
        {
            delete _data;
            _data = nullptr;
        }
    }

public:
    MyClass(const MyClass& myClass) = delete;
    MyClass& operator=(const MyClass& myClass) = delete;


public:
    MyClass(MyClass&& myClass)
        : _data(myClass._data)
    {
        std::cout << "Move Constructor" << '\n';

        myClass._data = nullptr;
    }

    MyClass& operator=(MyClass&& myClass)
    {
        std::cout << "Move Assignment" << '\n';

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
    MyClass myClass(new int{ data });

    return myClass;
}

int main()
{
    using namespace std;

    {
        MyClass myClassA = make_my_class(10);

        // Copy Constructor가 아닌 Move Constructor 발생
        MyClass myClassB(std::move(myClassA));
    }

    std::cout << "--------------------\n";

    {
        MyClass myClassA = make_my_class(10);

        // Copy Assignment가 아닌 Move Assignment 발생
        MyClass myClassB;
        myClassB = std::move(myClassA);
    }

    std::cout << "--------------------\n";

    {
        std::vector<MyClass> vec;

        // vec.reserve(10)를 주석처리하면 재밌는 현상을 볼 수 있음
        vec.reserve(10);

        for (int i = 0; i < 10; i++)
        {
            vec.push_back(std::move(make_my_class(10 + i)));
        }
    }

    return 0;
}
