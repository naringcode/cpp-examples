#include <iostream>
#include <typeinfo>

struct MyStruct
{
    int a;
    int b;
};

class MyClass
{
private:
    int a;
    int b;
};

int main()
{
    using namespace std;

    cout << typeid(4.0).name() << '\n';
    cout << typeid(3.14f).name() << '\n';
    cout << typeid("string").name() << '\n';

    cout << typeid(MyStruct).name() << '\n';
    cout << typeid(MyClass).name() << '\n';

    return 0;
}
