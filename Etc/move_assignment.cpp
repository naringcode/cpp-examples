#include <iostream>

// move sementics는 r-value를 대상으로 한다.
// l-value는 지속성(?)이 있지만 r-value는 바로 사라진다.

class MyClass
{
public:
    MyClass() 
    {
        std::cout << "Default Constructor A" << '\n';
    }

    MyClass(int* data)
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
    MyClass myClass(new int{data});

    return myClass;
}

int main()
{
    using namespace std;

    {
        MyClass myClass;
        myClass = make_my_class(10);
    }

    return 0;
}
