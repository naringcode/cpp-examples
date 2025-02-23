// Update Date : 2025-01-13
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <string>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

// https://en.cppreference.com/w/cpp/language/string_literal

void PrintString(const char* pStr)
{
    cout << "PrintString(const char* pStr) : " << pStr << '\n';
}

#if _HAS_CXX20 >= 1
// C++20 이후부턴 원시적인 char과 UTF-8(char8_t)을 구분해서 처리할 수 있다.
void PrintString(const char8_t* u8pStr)
{
    // C++20 기준 iostream은 UTF-8을 다루는 방법을 제공하지 않기에 캐스팅해서 사용해야 한다.
    // UTF-8의 특성을 고려하면 char8_t을 char로 캐스팅하는 건 문제가 생길 소지가 ASCII 범위 내라면 문제가 생기진 않는다. 
    cout << "PrintString(const char8_t* u8pStr) : " << reinterpret_cast<const char*>(u8pStr) << '\n';
}
#endif

int main()
{
    char u8ch = u8'A';

#if _HAS_CXX20 >= 1
    const char8_t* u8pStr = u8"Hello World"; // u8로 생성한 문자열은 C++20 기준 const char8_t[]로 받는다(const char*로 캐스팅해서 쓰는 것이 가능하긴 함).
    u8string u8str{ u8"Hello World" }; // 문자열도 마찬가지로 string 타입으로 받으려면 const char*로 캐스팅해야 한다.

    PrintString((const char*)u8pStr);
    PrintString((const char*)u8str.c_str());

    PrintString(u8pStr);
    PrintString(u8str.c_str());
#else
    const char* u8pStr = u8"Hello World"; // 그 이전 버전에서는 const char[]로 받는다.
    string u8str{ u8"Hello World" };

    PrintString(u8pStr);
    PrintString(u8str.c_str());
#endif

    fs::path currPath{ u8"./" };
    
#if _HAS_CXX20 >= 1
    // C++20 이후부턴 std::string으로 받을 수 없다.
    u8string currPathStr = currPath.u8string();
#else
    string currPathStr = currPath.u8string();
#endif

    return 0;
}
