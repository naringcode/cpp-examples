// #include <bits/stdc++.h>
#include <iostream>
#include <regex>

using namespace std;

#define MODE 1

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    std::string str = "Hello, World!";

    std::regex patternSub(R"(, \w{3})");
    std::regex patternFull(R"((\w+), (\w+)!)");

#if MODE == 0
    if (std::regex_match(str, patternSub))
    {
        std::cout << "Sub Match\n";
    }

    if (std::regex_match(str, patternFull))
    {
        std::cout << "Full Match\n";
    }

    if (std::regex_search(str, patternSub))
    {
        std::cout << "Sub Search\n";
    }

    if (std::regex_search(str, patternFull))
    {
        std::cout << "Full Search\n";
    }
#elif MODE == 1
    std::smatch smResult;

    if (std::regex_match(str, smResult, patternSub))
    {
        std::cout << "Sub Match\n";

        std::cout << "Size : " << smResult.size() << '\n';
        std::cout << "Str : " << smResult.str() << '\n';
        std::cout << "Prefix : " << smResult.prefix() << '\n';
        std::cout << "Suffix : " << smResult.suffix() << '\n';

        for (const auto& result : smResult)
        {
            cout << result << '\n';
        }

        cout << '\n';
    }

    if (std::regex_match(str, smResult, patternFull))
    {
        std::cout << "Full Match\n";

        std::cout << "Size : " << smResult.size() << '\n';
        std::cout << "Str : " << smResult.str() << '\n';
        std::cout << "Prefix : " << smResult.prefix() << '\n';
        std::cout << "Suffix : " << smResult.suffix() << '\n';

        for (const auto& result : smResult)
        {
            cout << result << '\n';
        }

        cout << '\n';
    }

    if (std::regex_search(str, smResult, patternSub))
    {
        std::cout << "Sub Search\n";

        std::cout << "Size : " << smResult.size() << '\n';
        std::cout << "Str : " << smResult.str() << '\n';
        std::cout << "Prefix : " << smResult.prefix() << '\n';
        std::cout << "Suffix : " << smResult.suffix() << '\n';

        for (const auto& result : smResult)
        {
            cout << result << '\n';
        }

        cout << '\n';
    }

    if (std::regex_search(str, smResult, patternFull))
    {
        std::cout << "Full Search\n";

        std::cout << "Size : " << smResult.size() << '\n';
        std::cout << "Str : " << smResult.str() << '\n';
        std::cout << "Prefix : " << smResult.prefix() << '\n';
        std::cout << "Suffix : " << smResult.suffix() << '\n';

        for (const auto& result : smResult)
        {
            cout << result << '\n';
        }
    }
#endif

    return 0;
}
