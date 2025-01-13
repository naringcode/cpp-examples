// Update Date : 2025-01-14
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>

using namespace std;

#define BEGIN_NS(name) namespace name {
#define END_NS };

// https://en.cppreference.com/w/cpp/language/constinit

BEGIN_NS(Case01)

class FooObject
{
public:
    FooObject(string keyword)
    {
        cout << "FooObject() : " << keyword << "\n";
    }
};

// static 변수는 컴파일 시점이 아닌 런타임 시 사용되는 시점에 초기화되어 적재된다.
// - lazy initialization
// - lazy loading
void RuntimeLazyLoadingStaticVariable()
{
    cout << "Begin RuntimeLoadingStatic()\n";

    static FooObject s_FuncFoo{ "RuntimeLazyLoadingStaticVariable()" };

    cout << "End RuntimeLoadingStatic()\n";
}

int GetValue()
{
    return 100;
}

// 런타임 시 GetValue()를 호출하여 초기화
int g_Val = GetValue();

// 마찬가지로 런타임 시 생성자를 호출하여 초기화(컴파일 타임에 평가되는 함수라면 cout을 쓸 수 없음)
FooObject g_GlobalFoo{ "Global" };

void Run()
{
    // Once(static 변수 초기화)
    RuntimeLazyLoadingStaticVariable();

    // Twice(초기화된 static 변수에 접근)
    RuntimeLazyLoadingStaticVariable();

    // 어떠한 경우가 되었든 static 변수는 사용하는 시점에 초기화된다.
    static FooObject s_MainFoo{ "Laze loading in the main func" };
}

END_NS

BEGIN_NS(Case02)

// C++20부터는 constinit을 사용해 변수를 컴파일 타임에 초기화할 수 있다.
// !! constinit은 컴파일 타임에 초기화하기 위해 지정하는 키워드이기 때문에 const와는 다름. !!

// constinit의 제약 사항
// - 전역, static, TLS에만 적용 가능(동적 초기화 불가능, 정적으로 수행될 수 있어야 함)
// - consteval이나 constexpr과 혼용해서 사용하는 것은 불가능("consteval constinit someVal;" 이런 형식을 허용하지 않음)
// - 함수의 반환값으로 constinit 변수를 초기화하고자 한다면 해당 함수는 constexpr나 consteval을 명시해야 함.

// constinit으로 받을 때는 런타임에 평가될 요소(ex. cout)를 함수에 넣으면 안 된다.
constexpr int GetValueA()
{
    return 100;
}

consteval int GetValueB()
{
    return 200;
}

int GetValueC()
{
    return 300;
}

constinit int g_ValA = GetValueA();
constinit int g_ValB = GetValueB();

// constinit을 함수 호출로 초기화하려면 대상 함수는 컴파일 타임에 진행되는 것이 명시된 constexpr이나 consteval이 붙어야 한다.
// constinit int g_ValC = GetValueC();

void Run()
{
    static constinit int funcValA = GetValueA();
    static constinit int funcValB = GetValueB();
    // static constinit int funcValC = GetValueC();

    cout << g_ValA << '\n';
    cout << g_ValB << '\n';
    // cout << g_ValC << '\n';

    cout << funcValA << '\n';
    cout << funcValB << '\n';
    // cout << funcValC << '\n';
}

END_NS

int main()
{
    Case01::Run();

    cout << "--------------------------------------------------\n";

    Case02::Run();

    return 0;
}
