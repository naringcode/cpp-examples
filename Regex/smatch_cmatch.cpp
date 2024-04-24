// #include <bits/stdc++.h>
#include <iostream>
#include <regex>

using namespace std;

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    std::string str = "Hello, World!";
    char ptr[] = "Hello, World!";

    regex pattern(R"((\w+), (\w+)!)");
    // regex pattern(R"(\w+)");

    /**
     * smatch : std::string을 대상으로 함
     * cmatch : const char*를 대상으로 함 
     *
     * 결과는 원본 문자열의 복사본이 아닌 일종의 View
     * 원본이 수정되면 매치된 결과를 조회했을 때 바뀐 문자열로 보임
     */
    smatch smResult;
    if (std::regex_match(str, smResult, pattern))
    {
        str[0] = 'W'; str[1] = 'o'; str[2] = 'w'; str[3] = 'o'; str[4] = 'W';

        cout << "std::smatch :" << '\n';

        for (const auto& group : smResult)
        {
            cout << '\t' << group << '\n';
        }
    }

    cout << '\n';

    cmatch cmResult;
    if (std::regex_match(ptr, cmResult, pattern))
    {
        ptr[0] = 'W'; ptr[1] = 'o'; ptr[2] = 'w'; ptr[3] = 'o'; ptr[4] = 'W';

        cout << "std::cmatch :" << '\n';

        for (const auto& group : cmResult)
        {
            cout << '\t' << group << '\n';
        }
    }

    return 0;
}
