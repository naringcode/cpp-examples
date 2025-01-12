// #include <bits/stdc++.h>
#include <iostream>
#include <regex>

using namespace std;

struct RegUnit
{
    std::regex  pattern;
    std::string type  = ""; // "" means skip!
};

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    // Whitespaces : ^\s+
    // Single-line comment : ^\/\/.*
    // Multi-line comments : ^\/\*[\s\S]*?\*\/
    // Number : ^\d+
    // String : ^"[^"]*", ^'[^']*'
    // Keyword : ^(var|let)
    // Identifier : ^\w+

    vector<RegUnit> regUnits;

    regUnits.push_back({ std::regex(R"(\s+)"), "" });
    regUnits.push_back({ std::regex(R"(=)"), "" });
    regUnits.push_back({ std::regex(R"(\/\/.*)"), "Single-line comment" });
    regUnits.push_back({ std::regex(R"(\/\*[\s\S]*?\*\/)"), "Multi-line comments" });
    regUnits.push_back({ std::regex(R"(\d+)"), "Number" });
    regUnits.push_back({ std::regex(R"("[^"]*")"), "String" });
    regUnits.push_back({ std::regex(R"(var|let)"), "Keyword" });
    regUnits.push_back({ std::regex(R"(\w+)"), "Identifier" });

    string str = R"(
        // this is a single-line comment // in a line
        /**
         * this is multi-line comments
         */
        var num = 10;
        let str = "string";
    )";

    // cout << str;

    for (auto iter = str.cbegin(); iter != str.end();)
    {
        std::smatch smResult;
        bool hasToken = false;

        for (auto& regUnit : regUnits)
        {
            if (std::regex_search(iter, str.cend(), smResult, regUnit.pattern, std::regex_constants::match_continuous))
            {
                hasToken = true;

                // both lines of code accomplish the same task
                // iter += smResult.position() + smResult.length();
                iter = smResult[0].second;

                if ("" == regUnit.type)
                    break;

                std::cout << regUnit.type << '\n';
                std::cout << smResult.str() << '\n';
                std::cout << "-------------------------\n";

                break;
            }
        }

        if (false == hasToken)
        {
            // std::cout << "cannot find any token.\n";
            //
            // break;

            iter++; // skips ';'
        }
    }

    return 0;
}
