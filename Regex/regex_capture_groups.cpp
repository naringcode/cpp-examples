// #include <bits/stdc++.h>
#include <iostream>
#include <regex>

using namespace std;

#define MODE 0

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    std::string str = "Hello, my email is 123example@gmail.com";

#if MODE == 0
    std::regex  pattern{ R"(\d+(\w+)@(\w+)\.(\w+))" }; // Capture groups: (\w+), (\w+), (\w+)
#elif MODE == 1
    std::regex  pattern{ R"((\d+(\w+))@(\w+)\.(\w+))" }; // Capture groups: (\d+(\w+)), (\w+), (\w+), (\w+)
#endif

    std::smatch matches;

    if (std::regex_search(str, matches, pattern)) 
    {
        // Access capture groups
#if MODE == 0
        std::cout << "Email address: " << matches[0] << std::endl; // Full match
        std::cout << "Username: "      << matches[1] << std::endl; // First capture group
        std::cout << "Domain: "        << matches[2] << std::endl; // Second capture group
        std::cout << "TLD: "           << matches[3] << std::endl; // Third capture group
#elif MODE == 1
        std::cout << "Email address: " << matches[0] << std::endl; // Full match
        std::cout << "Full Username: " << matches[1] << std::endl; // First capture group
        std::cout << "Username: "      << matches[2] << std::endl; // First capture group
        std::cout << "Domain: "        << matches[3] << std::endl; // Second capture group
        std::cout << "TLD: "           << matches[4] << std::endl; // Third capture group
#endif
    }

    return 0;
}
