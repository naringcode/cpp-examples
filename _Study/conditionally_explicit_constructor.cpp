// Update Date : 2025-01-14
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <string>
#include <string_view>

using namespace std;

#define BEGIN_NS(name) namespace name {
#define END_NS };

// https://en.cppreference.com/w/cpp/language/explicit
// https://dev.to/pgradot/let-s-try-c-20-explicit-bool-7a

BEGIN_NS(Case01)

// C++20 이전에는 조건부로 암시적인 생성자를 적용시키고자 했다면 SFINAE와 explicit을 이용하는 방식을 써야 했다(조건부 생성자 적용).
template <typename T>
class FooWrapper
{
public:
    // From(U) -> To(T)로의 변환에 실패하는 경우라면 해당 생성자에 들어온다.
    // !! U를 "암시적인" 인자로 받을 수 있는 T의 생성자가 없으면 실패함. !!
    template <typename U, enable_if_t<!is_convertible_v<U, T>>* = nullptr>
    explicit FooWrapper(const U& arg)
        : _val{ arg }
    { }
    
    // From(U) -> To(T)로의 변환에 성공하는 경우라면 해당 생성자에 들어온다.
    template <typename U, enable_if_t<is_convertible_v<U, T>>* = nullptr>
    FooWrapper(const U& arg)
        : _val(arg)
    { }

public:
    const T Get() const
    {
        return _val;
    }
    
private:
    T _val;
};

void PrintString(const FooWrapper<string>& str)
{
    cout << str.Get() << "\n";
}

void Run()
{
    // string tempA = "HelloWorld"sv; // NO(암시적 형변환 불가능)
    // string tempB{ "HelloWorld" };  // OK(명시적 형변환만 가능)

    FooWrapper<string> strA = "Hello World A";
    // FooWrapper<string> strB = "Hello World B"sv; // 해당 유형의 생성자는 explicit이 적용된 상태라 사용 불가

    FooWrapper<string> strC = "Hello World C";
    FooWrapper<string> strD{ "Hello World D"sv };

    cout << strA.Get() << "\n";
    // cout << strB.Get() << "\n";

    cout << strC.Get() << "\n";
    cout << strD.Get() << "\n";

    // 인자를 넘기는 경우 암묵적으로 다음과 같이 전달하는 경우가 생길 수 있다.
    PrintString("Hello World E");
    // PrintString("Hello World F"sv); // 암시적으로 받을 수 없기 때문에 에러가 발생함.
}

END_NS

BEGIN_NS(Case02)

// C++20부터는 조건부 형변환을 제어하는 것이 가능하다(conditional explicit).
// 이건 explicit bool을 기반으로 하는 기능이다.
constexpr bool kEnableExplicitOn  = true;
constexpr bool kEnableExplicitOff = false;

// explicit bool의 조건에 들어가는 bool 값은 컴파일 타임에 평가될 수 있어야 한다.
struct FooExplicitOn
{
    explicit(kEnableExplicitOn) FooExplicitOn(int val)
    {
        cout << val << "\n";
    }
};

struct FooExplicitOff
{
    explicit(kEnableExplicitOff) FooExplicitOff(int val)
    {
        cout << val << "\n";
    }
};

template <bool explicitOn>
struct Foo
{
    explicit(explicitOn) Foo(int val)
    {
        cout << val << "\n";
    }
};

// 좀 특이한 형태이긴 하나 이렇게 인자의 개수를 제한하는 가변 인자 템플릿을 쓸 수도 있다.
struct FooArray
{
    template <typename... Args>
    explicit(sizeof...(Args) >= 3) FooArray(Args&&...)
    {
        cout << "Num Of Args : " << sizeof...(Args) << "\n";
    }
};

// 다음과 같이 특정 타입만 명시적으로 받게 하는 것도 가능하다.
struct FooType
{
    // int 타입만 명시적으로 생성자를 호출하게끔 한정하는 코드
    template <typename T>
    explicit(is_integral_v<T>) FooType(T val)
    {
        cout << val << "\n";
    }
};

void Run()
{
    // FooExplicitOn fooA = 100; // explicit을 킨 상태이기에 묵시적인 형변환을 허용하지 않음.
    FooExplicitOff fooB = 200;

    FooExplicitOn  fooC{ 300 }; // explicit을 적용한 상태면 명시적으로 형변환해야 함.
    FooExplicitOff fooD{ 400 };

    // Foo<true> fooE = 500;
    Foo<false> fooF = 600;

    Foo<true> fooG{ 700 };
    Foo<false> fooH{ 800 };

    // 인자의 개수가 3개 이하이기 때문에 explicit이 적용되지 않는다.
    FooArray arrA = { 1, 2 };
    FooArray arrB = { 'A', 'B' };

    // 인자의 개수가 3개 이상이기 때문에 explicit이 적용되어 암시적인 형변환을 허용하지 않는다.
    // FooArray arrC = { 1, 2, 3, 4 };
    // FooArray arrD = { 'A', 'B', 'C', 'D' };

    // 인자의 개수가 3개 이상이면 명시적으로 생성자를 호출하는 방식을 택해야 한다.
    FooArray arrC{ 1, 2, 3, 4 };
    FooArray arrD{ 'A', 'B', 'C', 'D' };

    // int 타입은 명시적으로 생성자를 호출하게 만든 상태
    // FooType typeA =  111;
    FooType typeB{ 111 };

    // int 외의 타입은 묵시적으로 생성자가 호출될 수 있게 한 상태
    // !! float 뿐만 아니라 ostream의 기능을 확장하면 구조체나 클래스도 받게 할 수 있음. !!
    FooType typeC = 3.14;
    FooType typeD{ 3.14 };
}

END_NS

BEGIN_NS(Case03)

// explicit bool을 이용하면 Case01에 나온 예시 클래스를 간소화할 수 있다.

// explicit bool의 조건에 들어가는 bool 값은 컴파일 타임에 평가될 수 있어야 한다.
template <typename T>
class FooWrapper
{
public:
    // From(U) -> To(T)로의 변환에 성공한다면 explicit을 적용하지 않고,
    // From(U) -> To(T)로의 변환에 실패한다면 explicit을 적용한다.
    // !! Case01에 나온 복잡한 절차를 조건부 explicit을 적용하여 간소화한 것임. !!
    template <typename U>
    explicit(!is_convertible_v<U, T>) FooWrapper(const U& arg)
        : _val{ arg }
    { }

public:
    const T Get() const
    {
        return _val;
    }

private:
    T _val;
};

void PrintString(const FooWrapper<string>& str)
{
    cout << str.Get() << "\n";
}

void Run()
{
    // string tempA = "HelloWorld"sv; // NO(암시적 형변환 불가능)
    // string tempB{ "HelloWorld" };  // OK(명시적 형변환만 가능)

    FooWrapper<string> strA = "Hello World A";
    // FooWrapper<string> strB = "Hello World B"sv; // 해당 유형의 생성자는 explicit이 적용된 상태라 사용 불가

    FooWrapper<string> strC = "Hello World C";
    FooWrapper<string> strD{ "Hello World D"sv };

    cout << strA.Get() << "\n";
    // cout << strB.Get() << "\n";

    cout << strC.Get() << "\n";
    cout << strD.Get() << "\n";

    // 인자를 넘기는 경우 암묵적으로 다음과 같이 전달하는 경우가 생길 수 있다.
    PrintString("Hello World E");
    // PrintString("Hello World F"sv); // 암시적으로 받을 수 없기 때문에 에러가 발생함.
}

END_NS

BEGIN_NS(Case04)

// https://en.cppreference.com/w/cpp/language/constraints
// https://en.cppreference.com/w/cpp/concepts

// 여담이긴 하나 C++20에서 제공하는 제약 조건을 사용해서 explicit 적용 여부를 결정하는 것도 가능하다.
template <typename T>
class FooWrapper
{
public:
    template <typename U>
        requires !is_convertible_v<U, T>
    explicit FooWrapper(const U& arg)
        : _val{ arg }
    { }

    template <typename U>
        requires is_convertible_v<U, T>
    FooWrapper(const U& arg)
        : _val(arg)
    { }

public:
    const T Get() const
    {
        return _val;
    }

private:
    T _val;
};

void PrintString(const FooWrapper<string>& str)
{
    cout << str.Get() << "\n";
}

void Run()
{
    // string tempA = "HelloWorld"sv; // NO(암시적 형변환 불가능)
    // string tempB{ "HelloWorld" };  // OK(명시적 형변환만 가능)

    FooWrapper<string> strA = "Hello World A";
    // FooWrapper<string> strB = "Hello World B"sv; // 해당 유형의 생성자는 explicit이 적용된 상태라 사용 불가

    FooWrapper<string> strC = "Hello World C";
    FooWrapper<string> strD{ "Hello World D"sv };

    cout << strA.Get() << "\n";
    // cout << strB.Get() << "\n";

    cout << strC.Get() << "\n";
    cout << strD.Get() << "\n";

    // 인자를 넘기는 경우 암묵적으로 다음과 같이 전달하는 경우가 생길 수 있다.
    PrintString("Hello World E");
    // PrintString("Hello World F"sv); // 암시적으로 받을 수 없기 때문에 에러가 발생함.
}

END_NS

int main()
{
    Case01::Run();

    cout << "--------------------------------------------------\n";

    Case02::Run();

    cout << "--------------------------------------------------\n";

    Case03::Run();

    cout << "--------------------------------------------------\n";

    Case04::Run();

    return 0;
}
