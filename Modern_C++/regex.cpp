#include <iostream>
#include <regex> // C++11

// regular expresssions
// https://cplusplus.com/reference/regex/ECMAScript/
// https://en.cppreference.com/w/cpp/regex/regex_iterator

int main()
{
    using namespace std;

    regex reg("\\d"); // 숫자 하나
    // regex reg("\\d+"); // 한 개 이상의 숫자
    // regex reg("\\d*"); // 숫자 입력을 안 받아도 괜찮
    // regex reg("[[:digit:]]{3}"); // digit 3개 딱 맞춰서 받겠다.
    // regex reg("[A-Z]+"); // 알파벳 대문자 한 개 이상
    // regex reg("[A-Z]{3}"); // 알파벳 대문자 정확히 3개
    // regex reg("[A-Z]{1,5}"); // 알파벳 대문자 1개 ~ 5개
     
    // 0-9 숫자 하나
    // [-] 기호는 있어도 그만 없어도 그만
    // 0-9 숫자 1개 이상 4개 이하
    // regex reg("([0-9]{1})([-]?)([0-9]{1,4})");

    string str;

    while (true)
    {
        getline(cin, str);

        if (true == regex_match(str, reg))
        {
            cout << "Matched\n";
        }
        else
        {
            cout << "Not Matched\n";
        }

        // 매치하는 부분만 출력
        {
            auto begin = sregex_iterator(str.begin(), str.end(), reg);
            auto end   = sregex_iterator();

            for (auto iter = begin; iter != end; iter++)
            {
                smatch match = *iter;

                cout << match.str() << ' ';
            }

            cout << '\n';
        }
    }

    return 0;
}
