// Update Date : 2025-02-24
// OS : Windows 10 64bit
// Program : vscode(gcc-14.2.0)
// Version : C++23
// Configuration : None

#include <print>
#include <numbers>

// https://en.cppreference.com/w/cpp/io/print
// https://en.cppreference.com/w/cpp/io/println
// https://en.cppreference.com/w/cpp/utility/format/spec

// C++23부터는 std::print()와 std::println()을 통해 보다 형식화된 출력을 진행할 수 있다.
// 포맷팅 스타일은 std::format()과 동일하다.

int main()
{
    std::print("Hello {}", "World");
    std::print("{} {}", "Hello", "World"); // std::print()는 개행을 수행하지 않음.
    std::print("\n");
    
    std::println("Hello {}", "World");
    std::println("{} {}", "Hello", "World"); // std::println()은 개행을 수행함.

    std::println("Pi value {0}, {0:.2f}", std::numbers::pi);

    std::println("{:<10} {}", "Hello", "World");
    std::println("{:^10} {}", "Hello", "World");
    std::println("{:>10} {}", "Hello", "World");

    for (int i = 1; i <= 1000; i *= 10)
    {
        std::println("{:*>6} {}", i, "Left");
        std::println("{:*^6} {}", i, "Center");
        std::println("{:*<6} {}", i, "Right");
    }

    for (int i = 1; i < 5; i++)
    {
        std::println("Pi value {} {:.{}f}", i, std::numbers::pi, i);
        std::println("Pi value {0} {1:.{0}f}", i, std::numbers::pi);
    }

    // https://en.cppreference.com/w/cpp/utility/format/spec
    // 포맷팅에 대한 자세한 내용은 위 링크에서 보도록 한다.

    return 0;
}
