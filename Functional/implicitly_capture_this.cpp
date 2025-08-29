// Update Date : 2025-08-29
// OS : Windows 10 64bit
// Program : Visual Studio 2022, vscode(gcc-13.1.0)
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <functional>
#include <string>

using namespace std;

// https://en.cppreference.com/w/cpp/language/lambda.html
// The current object (*this) can be implicitly captured if either capture default is present. 
// If implicitly captured, it is always captured by reference, even if the capture default is =.
// The implicit capture of *this when the capture default is = is deprecated.(since C++20).

// 인스턴스화된 클래스의 멤버 함수에서 람다식을 구성할 때 [=]를 사용하면 this가 암묵적으로 값으로 복사되어 캡처된다.
// - 지역 변수는 값으로 캡처함.
// - 멤버 변수는 값으로 캡처하지 않고 암묵적으로 캡처된 this를 통해 접근함.
// 즉, 멤버 변수를 값(value)으로 캡처할 용도로 [=]를 사용했는데 정작 this라는 숨은 포인터를 캡처해서 사용한다는 뜻이다.
//
// https://en.cppreference.com/w/cpp/language/lambda.html#Dangling_references
// 이에 따라 발생 가능한 문제는 this의 dangling 포인터화이다.
//
// C++20 이후부터는 이러한 문법적인 혼란을 줄이기 위해 *this를 지원한다(사용은 [=, *this]가 정석임).
// struct S2 { void f(int i); };
// void S2::f(int i)
// {
//     [=] {};        // OK: by-copy capture default
//     [=, &i] {};    // OK: by-copy capture, except i is captured by reference
//     [=, *this] {}; // until C++17: Error: invalid syntax
//                    // since C++17: OK: captures the enclosing S2 by copy
//     [=, this] {};  // until C++20: Error: this when = is the default
//                    // since C++20: OK, same as [=]
// }

// 인스턴스화된 클래스의 멤버 함수 내 람다식을 구성할 때 캡처 블록에 "="를 쓰면 this를 암묵적으로 값으로 복사해서 캡처한다.
// 따라서 인스턴스가 유효하지 않은 상태가 되었을 때 람다를 호출하는 상황이 되었을 때 문제가 발생한다.
// 
// C++20부터는 객체의 멤버 함수 내에서 [=]로 캡처하는 것을 피하고, [=, *this]와 같은 형태로 사용할 것을 권장한다.
// 물론 [=]나 [this]로 캡처하는 것도 여전히 유효하지만, C++20 이후부터 [=]를 사용할 경우 암묵적인 this 캡처가 발생한다고 컴파일러 경고가 뜰 확률이 매우 높아졌다.

class FooObject
{
public:
    ~FooObject()
    {
        _str = "Goodbye World";
    }

public:
    function<void()> GetBadLambda(int captureVal)
    {
        // 값으로 캡처하는 것이 아닌 암묵적으로 this를 캡처하여 this->_str와 this->callCnt로 접근한다.
        // Visual Studio에서는 Warning을 띄우지 않지만 vccode를 통해 GCC로 빌드하면 다음과 같은 경고문이 뜬다.
        // !! implicit capture of 'this' via '[=]' is deprecated in C++20 [-Wdeprecated] !!
        return [=]
            {
                cout << "In the lambda : " << captureVal << ", " << _str << ", " << _callCnt++ << "\n";
            };
    }

    function<void()> GetGoodLambda(int captureVal)
    {
        // C++20부터는 *this를 이용하여 인스턴스의 값을 캡처하는 것이 가능하다.
        // *this : simple by-copy capture of the current object.
        return [=, *this] // () mutable
            {
                // 다만 이 경우에는 멤버 변수를 const로 캡처하기 때문에 값을 수정하고자 한다면 lambda를 mutable 형식으로 만들어야 한다.
                // cout << "In the lambda : " << captureVal << ", " << _str << ", " << _callCnt++ << "\n";

                cout << "In the lambda : " << captureVal << ", " << _str << "\n";
            };
    }

    function<void()> GetBestLambda(int captureVal)
    {
        // this와 *this를 다음과 같이 별칭으로 받아서 사용하는 것도 가능하다.
        return [=, copy = *this, self = this]
            {
                cout << "In the lambda : " << captureVal << ", " << copy._str << ", " << self->_callCnt++ << "\n";
            };
    }

private:
     int    _callCnt = 0;
     string _str = "Hello World";
};

__declspec(noinline) void Run01()
{
    FooObject* foo = new FooObject{ };

    auto lambda = foo->GetBadLambda(100);

    // 정상적인 호출
    lambda();

    delete foo;

    // 객체를 소멸시킨 상태에서 람다를 호출하면 해제된 멤버 변수 _str에 접근한다(안전하지 않은 코드).
    // !! 컴파일러 버전에 따라서 또는 Debug나 Release 모드에 따라서 런타임 에러가 발생할 수 있음. !!
    lambda();
}

__declspec(noinline) void Run02()
{
    FooObject* foo = new FooObject{ };

    auto lambda = foo->GetGoodLambda(200);

    // 정상적인 호출
    lambda();

    delete foo;

    // 객체를 소멸시킨 상태여도 값으로 캡처했기 때문에 문제가 되지 않는다.
    lambda();
}

__declspec(noinline) void Run03()
{
    FooObject* foo = new FooObject{ };

    auto lambda = foo->GetBestLambda(300);

    // 정상적인 호출
    lambda();

    delete foo;

    // *this가 아닌 this로 캡처한 값이 있기 때문에 Run01에서 발생한 문제가 똑같이 발생한다.
    // lambda();
}

int main()
{
    Run01();

    cout << "--------------------------------------------------\n";

    Run02();

    cout << "--------------------------------------------------\n";

    Run03();

    return 0;
}
