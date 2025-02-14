// Update Date : 2025-02-14
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20, C++23
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <sstream>
#include <random>
#include <algorithm>
#include <ranges>
#include <vector>

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
// 11. algorithms_with_and_without_views.cpp <-----
// 12. view_composition.cpp
// 13. lazy_evaluation_of_views.cpp
// 14. view_working_with_range_algorithms.cpp
// 
// # Range Factories
// 15. range_factories.cpp
//

template <std::ranges::range Container>
void Print(Container&& container, std::string_view msg = "", std::string separator = " ")
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

// View를 사용하면 원본의 복사본을 만들 필요가 없이 결과를 정제하는 것이 가능하다.

namespace Join
{
    void BeforeCpp20()
    {
        std::vector<std::string> words{ "C++", "is", "a", "general", "purpose", "programming", "language" };
        Print(words, "Words");

        std::string result;
        std::string delim = ""; // " ";

        auto builder = [&result, delim](const std::string& word) {
            result += word;
            result += delim;
        };

        std::for_each(words.begin(), words.end(), builder);

        Print(result, "Joined"); // std::string을 char 단위로 출력(words가 평탄화되어 있음)
    }

    void AfterCpp20()
    {
        std::vector<std::string> words{ "C++", "is", "a", "general", "purpose", "programming", "language" };
        Print(words, "Words");

        // https://en.cppreference.com/w/cpp/ranges/join_view
        // join_view 사용
        auto joined = std::views::join(words);

        Print(joined, "Joined"); // 결과를 char 단위로 출력(words가 평탄화되어 있음)
    }

    void AfterCpp23()
    {
        // C++20의 join_view에는 delimiter를 부여할 수 있는 기능이 없다.
        std::vector<std::string> words{ "C++", "is", "a", "general", "purpose", "programming", "language" };
        Print(words, "Words");

        // https://en.cppreference.com/w/cpp/ranges/join_with_view
        // C++23에는 join_with_view가 추가되었는데 이걸 사용하면 delimiter를 부여할 수 있다.
        auto joined = std::views::join_with(words, ' ');

        Print(joined, "Joined", ""); // 결과를 char 단위로 출력(words가 평탄화되어 있음)
    }
}

namespace Split
{
    void BeforeCpp20()
    {
        std::string str = "C++ is a general purpose programming language";
        std::istringstream inputStream{ str };

        std::vector<std::string> words;
        std::string word;

        while (inputStream >> word)
        {
            words.emplace_back(word);
        }

        Print(words, "Splited", "#");
    }

    void AfterCpp20_23()
    {
        // https://en.cppreference.com/w/cpp/ranges/split_view
        // split_view를 쓰면 delimiter를 기준으로 문자열을 쪼갤 수 있다.

        // !! 주의사항 !!
        // https://en.cppreference.com/w/cpp/ranges/split_view#Notes
        // The delimiter pattern generally should not be an ordinary string literal, 
        // as it will consider the null terminator to be necessary part of the delimiter; 
        // therefore, it is advisable to use a std::string_view literal instead.
        // -> string 말고 string_view를 delimiter로 쓰라는 뜻이다.
        // 
        // 일반적인 string은 '\0'을 자동으로 포함하는데 이것도 delimiter의 일부로 보기 때문인 것 같다.
        // 컴파일러의 수준에 따라서 string와 string_view 둘 다 문제 없이 잘 될 가능성이 있다.
        // using namespace std::string_literals;
        using namespace std::string_view_literals;

        std::string str = "C++ is a general purpose programming language";
        std::vector<std::string> words;

        // delimiter는 Range와 호환 가능해야 한다.
        auto splitView = std::views::split(str, " "sv); // 순회하면 std::string이 아닌 ranges::subrange로 반환함.

        // (불가능) ranges::subrange는 std::cout으로 출력할 수 없다.
        // std::cout << "Splited 1 : ";
        // 
        // for (const auto& subRange : splitView)
        // {
        //     std::cout << subRange << ' ';
        // }

        // (가능) ranges::subrange는 std::format()으로 받아서 문자열로 변환할 수 있다.
        std::cout << "Splited 1 : ";

        for (const auto& subRange : splitView)
        {
            std::cout << std::format("{} ", subRange) << ' ';
        }

        std::cout << '\n';

        // 내부에서 std::format()으로 ranges::subrange를 처리한다.
        // Print(splitView, "Splited 1");

        for (const auto& subRange : splitView)
        {
            // 가능
            Print(subRange, "subrange of split_view");

            // 불가능(std::string이 아닌 ranges::subrange로 받기 때문)
            // words += subRange;

            // 번거롭더라도 하나씩 순회하는 것이 정석이다.
            // std::string word;
            // 
            // for (char ch : subRange)
            // {
            //     word.push_back(ch);
            //     std::cout << ch;
            // }
            // 
            // words.push_back(word);

            // https://en.cppreference.com/w/cpp/ranges/to
            // C++23부터는 ranges::to()를 통해 View나 Range가 반환하는 결과를 원하는 Range 컨테이너로 받을 수 있다.
            words.push_back(std::ranges::to<std::string>(subRange));
        }

        Print(words, "Splited 2"); // words는 문자열 하나가 아닌 vector<string>으로 구성되어 있음.

        // C++23의 ranges::to()를 사용하면 split_view를 순회하지 않고 바로 결과를 받아오는 것이 가능하다.
        auto wordsGood = std::ranges::to<std::vector<std::string>>(splitView);

        Print(wordsGood, "Splited 3");
    }
}

namespace Filter
{
    void BeforeCpp20()
    {
        std::vector<int> data(10);
        std::ranges::generate(data, []() { return std::random_device{ }() % 100; });

        Print(data, "data");

        std::vector<int> filterEven;
        std::ranges::copy_if(data, std::back_inserter(filterEven), [](int elem) {
            return elem % 2 == 0;
        });

        Print(filterEven, "Filtered");
    }

    void AfterCpp20()
    {
        std::vector<int> data(10);
        std::ranges::generate(data, []() { return std::random_device{ }() % 100; });

        Print(data, "data");

        // https://en.cppreference.com/w/cpp/ranges/filter_view
        auto filtered = std::views::filter(data, [](int elem) { return elem % 2 == 0; });

        Print(filtered, "Filtered"); // 데이터 복사 없이 필터링 가능
    }
}

namespace Transform
{
    void BeforeCpp20()
    {
        std::string str = "C++ is a general purpose programming language";

        // toupper()의 반환형은 int이기 때문에 문자를 출력하려면 char로 캐스팅해야 한다.
        std::cout << "upper case   : ";
        std::for_each(str.begin(), str.end(), [](char ch) { std::cout << (char)toupper(ch); });
    }

    void AfterCpp20()
    {
        std::string str = "C++ is a general purpose programming language";

        // https://en.cppreference.com/w/cpp/ranges/transform_view
        // transform_view를 쓰면 Range의 요소를 쉽게 변환할 수 있다.
        auto upper = std::views::transform(str, [](char ch) { return (char)toupper(ch); });
        Print(upper, "upper case");

        // std::for_each()로 순회하는 방식은 자료를 순회하며 하나하나 변형하는 느낌이 강하지만
        // std::views::transform()을 써서 View를 반환하면 그 자체로 하나의 결과인 느낌이 강하다.
        //
        // 두 유형 중 후자의 경우가 훨씬 코드 가독성이 좋고 기능에 대한 밀집도가 높다.
    }
}

int main()
{
    Join::BeforeCpp20(); std::cout << '\n';
    Join::AfterCpp20(); std::cout << '\n';
    Join::AfterCpp23(); std::cout << '\n';

    Split::BeforeCpp20(); std::cout << '\n';
    Split::AfterCpp20_23(); std::cout << '\n';

    Filter::BeforeCpp20(); std::cout << '\n';
    Filter::AfterCpp20(); std::cout << '\n';

    Transform::BeforeCpp20(); std::cout << '\n';
    Transform::AfterCpp20(); std::cout << '\n';

    return 0;
}
