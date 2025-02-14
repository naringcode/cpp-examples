// Update Date : 2025-02-14
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20, C++23
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <sstream>
#include <fstream>
#include <ranges>

// Concepts에 대한 내용을 충분히 숙지한 다음 Ranges를 보도록 한다.
// Ranges의 동작 방식을 잘 이해하기 위해선 Concepts에 대한 선행 학습이 필요하다.
// 
// 다음 순서대로 보도록 하자.
// 
// # Ranges 개요
// 1. ranges_intro.txt
// 
// # Ranges 알고리즘
// 2. Concepts/CustomContainerIterators(사전지식)
// 3. range_concepts_and_iterator_concepts.cpp
// 4. legacy_algorithms_and_range_algorithms.cpp
// 5. return_types_by_range_algorithms.cpp
// 
// # Projections
// 6. implementation_of_invoke.cpp(사전지식)
// 7. range_algorithms_with_projections.cpp
// 8. implementation_of_projections.cpp
// 
// # Views
// 9. views_by_constructors.cpp
// 10. views_by_range_adaptors.cpp
// 11. algorithms_with_and_without_views.cpp
// 12. view_composition.cpp
// 13. lazy_evaluation_of_views.cpp
// 14. view_working_with_range_algorithms.cpp
// 
// # Range Factories
// 15. range_factories.cpp <-----
//

// https://en.cppreference.com/w/cpp/ranges#Range_factories

#define BEGIN_NS(name) namespace name {
#define END_NS }

template <std::ranges::range Container>
void Print(Container&& container, std::string_view msg = "", char separator = ' ')
{
    if (msg == "")
    {
        std::cout << "[ ";
    }
    else
    {
        std::cout << std::format("{:<12} : [ ", msg);
    }

    for (const auto& elem : container)
    {
        std::cout << std::format("{}{}", elem, separator);
    }

    std::cout << "]\n";
}

BEGIN_NS(Case01)

void Run()
{
    // https://en.cppreference.com/w/cpp/ranges/empty_view
    // empty_view : 요소를 가지지 않는 빈 View
    {
        auto emptyView = std::views::empty<int>; // 인자를 넣으면 안 됨.

        // std::views::empty<T>는 그 자체로 클래스의 인스턴스이기에 그냥 복사하는 방식으로 사용해야 한다.
        // namespace views {
        //     template<class T>
        //     constexpr empty_view<T> empty{};
        // }

        // empty_view는 비어있는 Range를 표현하기 위해 사용하며 Range 계의 nullptr이라고 보면 된다.
        // 단순하게 View를 받는 함수를 호출하고자 할 때 유용하게 사용할 수 있다.
        Print(emptyView, "empty");
    }

    std::cout << '\n';

    // https://en.cppreference.com/w/cpp/ranges/single_view
    // single_view : 단 하나의 요소만 가지는 View
    {
        auto singleView = std::views::single(100);

        // single_view는 하나의 값을 View로 래핑해서 사용하고자 할 때 유용하게 쓸 수 있다.
        Print(singleView, "single");
    }

    std::cout << '\n';

    // https://en.cppreference.com/w/cpp/ranges/iota_view
    // iota_view : 순차적으로 증가하는 값을 생성하는 View
    {
        // iota_view는 단독으로 인자 하나만 넣어서 사용하면 값을 무한히 생성하기 때문에 조심해야 한다.
        // Print(std::views::iota(1), "iota"); // 값을 무한히 생성함.
        Print(std::views::iota(1, 10), "iota(1, 10)"); // 1 ~ 9의 값을 순차적으로 생성함.
        Print(std::views::iota(1) | std::views::take(10), "iota + take"); // 이렇게 take()를 통해 제한을 주는 것도 가능함.

        // 값을 누적해서 반환하는 개념이 아닌 begin()부터 시작하면 초기화된다.
        auto iotaView = std::views::iota(1);
        Print(iotaView | std::views::take(10), "iota 1");
        Print(iotaView | std::views::take(10), "iota 2");
    }

    std::cout << '\n';

    // https://en.cppreference.com/w/cpp/ranges/basic_istream_view
    // basic_istream_view : operator>>를 통해 요소를 input stream으로 사용하기 위한 View
    {
        std::string str{ "This is a string" };

        // string 자체는 operator>>를 쓸 수 없으니 input stream으로 변환한 다음 넘겨야 한다.
        std::istringstream istream{ str };
        auto istreamView = std::views::istream<std::string>(istream); // stream으로 표현된 문자열을 view로 생성

        Print(istreamView, "istream"); // istream_view는 항상 공백(space)을 delimiter로 사용함.

        // istream_view는 input stream 유형이라면 어떠한 것이라도 받을 수 있다.
        std::ofstream fileOutStream{ "data.txt" };
        fileOutStream << "1 2 3\nOne\nTwo\nThree";
        fileOutStream.close();

        // std::ifstream fileInStream{ __FILE__ };
        std::ifstream fileInStream{ "data.txt" };

        if (!fileInStream)
        {
            std::cout << "Could not open a file\n";

            return;
        }

        istreamView = std::views::istream<std::string>(fileInStream);

        Print(istreamView, "file input stream");

        fileInStream.close();

        // cin도 input stream이기 때문에 istream_view로 적용할 수 있다.
        // auto commandView = std::views::istream<std::string>(std::cin);
        // 
        // std::cout << "Enter commands(exit to stop) :\n";
        // 
        // for (const std::string& command : commandView)
        // {
        //     if (command == "exit")
        //         break;
        // 
        //     std::cout << "Received : " << command << '\n';
        // }
    }

    std::cout << '\n';

    // https://en.cppreference.com/w/cpp/ranges/repeat_view
    // repeat_view (C++23) : 계속해서 똑같은 값을 생성하는 View
    {
        // iota_view처럼 단독으로 인자 하나만 넣어서 사용하면 값을 무한히 생성하기 때문에 조심해야 한다.
        // Print(std::views::repeat("Hello World!"), "repeat");
        
        // iota_view처럼 반복 제한을 repeat_view 차원에서 걸 수도 있고 take()를 통해 걸 수도 있다.
        Print(std::views::repeat("Hello World!", 3), "repeat 3 times");

        // 문자열 외 사용자가 지정한 요소라면 어떤 것이든 반복하는 것이 가능하다.
        Print(std::views::repeat(100) | std::views::take(5), "repeat + take");
    }
}

END_NS

int main()
{
    Case01::Run();

    return 0;
}
