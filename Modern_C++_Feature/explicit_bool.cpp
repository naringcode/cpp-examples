// Update Date : 2025-01-14
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>

using namespace std;

// https://en.cppreference.com/w/cpp/language/explicit
// https://dev.to/pgradot/let-s-try-c-20-explicit-bool-7a

// explicit constructor를 적용한 비교적 상세한 예시는 "conditionally_explicit_constructor.cpp"에 적어두었다.

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

int main()
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

    return 0;
}
