// Update Date : 2025-02-20
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <filesystem>
#include <source_location>

// https://en.cppreference.com/w/cpp/io/basic_osyncstream
// https://en.wikipedia.org/wiki/Interleaving

// C++20부터는 매크로에 의존하여 사용했던 파일 정보 출력(?)을 대체하기 위한 클래스를 제공한다.

#define BEGIN_NS(name) namespace name {
#define END_NS }

BEGIN_NS(Case01)

// 기존 방식은 매크로를 이용해서 파일 정보를 출력해야 했다.

class FooObject
{
public:
    FooObject()
    {
        std::cout <<"File : " << __FILE__ << ", Function : " << __func__ << ", Line : " << __LINE__ << '\n';
    }

    ~FooObject()
    {
        std::cout << "File : " << __FILE__ << ", Function : " << __func__ << ", Line : " << __LINE__ << '\n';
    }

public:
    void Func()
    {
        std::cout << "File : " << __FILE__ << ", Function : " << __func__ << ", Line : " << __LINE__ << '\n';
    }
};

void FooFunc()
{
    std::cout << "File : " << __FILE__ << ", Function : " << __func__ << ", Line : " << __LINE__ << '\n';
}

void Run()
{
    FooObject fooObj;

    fooObj.Func();

    FooFunc();

    std::cout << "File : " << __FILE__ << ", Function : " << __func__ << ", Line : " << __LINE__ << '\n';
}

END_NS

BEGIN_NS(Case02)

// C++20은 파일 정보를 담기 위한 std::source_location를 제공한다.
// std::source_location 자체는 아무런 기능을 하지 않기 때문에 current() 함수와 연계해서 사용해야 한다.
// current()는 consteval로 작성되어 있으며 필요한 값을 컴파일 타임에 결정한다.

// https://en.cppreference.com/w/cpp/utility/source_location
// 위 링크에서 제공하는 예제를 그대로 쓴 것.
void log(const std::string_view message,
         const std::source_location location =
               std::source_location::current())
{
    // source_location가 제공하는 함수는 current()를 포함해서 5개가 끝이다.
    std::clog << "file: "
              << location.file_name() << '('
              << location.line() << ':'
              << location.column() << ") `"
              << location.function_name() << "`: "
              << message << '\n';
}
 
template<typename T>
void fun(T x)
{
    log(x);
}
 
void Run()
{
    log("Hello world!");
    fun("Hello C++20!");

    // 이해를 돕기 위해 첨부한 내용
    std::source_location sourceLoc{ std::source_location::current() };
    log("Test", sourceLoc);
}

END_NS

BEGIN_NS(Case03)

// current()로 출력할 내용을 결정하는 건 컴파일러에 의존적이다.
// MSVC 기준으로 결정되는 파일 이름을 보면 경로까지 포함되어 있어서 복잡하게 출력하는 것을 볼 수 있다.

// 파일 이름만 따로 빼서 출력하고 싶을 때 filesystem을 쓰면 좋다.
void Log(std::source_location sourceLoc = std::source_location::current())
{
    std::cout << "##################################################\n";
    std::cout << std::format("File : {}({}:{})\n",
                             std::filesystem::path{ sourceLoc.file_name() }.filename().string(),
                             sourceLoc.line(), sourceLoc.column());
    std::cout << std::format("Function : {}\n", sourceLoc.function_name());
}

class FooObject
{
public:
    FooObject()
    {
        Log();
    }

    ~FooObject()
    {
        Log();
    }

public:
    void Func()
    {
        Log();
    }

    static void StaticFunc()
    {
        Log();
    }
};

void FooFunc()
{
    Log();
}

void Run()
{
    FooObject fooObj;

    fooObj.Func();
    FooObject::StaticFunc();

    FooFunc();

    auto fooLambda1 = []() {
        Log();
    };

    auto fooLambda2 = [](int x) {
        Log();

        return x;
    };

    fooLambda1();
    fooLambda2(10);

    Log();
}

END_NS

int main()
{
    // Case01::Run();
    // Case02::Run();
    Case03::Run();

    return 0;
}
