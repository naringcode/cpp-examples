// #include <bits/stdc++.h>
#include <iostream>
#include <regex>

using namespace std;

#define BOUNDARY 1
#define MODE 3

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    std::string str = 
        R"(Lorem ipsum dolor sit amet,
           consectetur adipiscing elit.
           Sed tristique orci orci, nec finibus lorem varius at.
           Aenean justo tortor, tincidunt vitae sapien gravida, consequat tincidunt dui.
           Aliquam eget erat convallis, vestibulum mauris ut, imperdiet libero.
           Sed lacinia erat congue maximus rutrum.)";

#if BOUNDARY == 0
    std::regex pattern(R"(\w{4})");
#elif BOUNDARY == 1
    // \b는 단어의 경계를 나타냄. 4문자가 정확히 일치하는 것을 추려내겠다는 뜻.
    // \bword\b -> sword, words, word 중 word만 찾아냄.
    std::regex pattern(R"(\b\w{4}\b)");
#endif

#if MODE == 0
    std::smatch smResult;

    while (std::regex_search(str, smResult, pattern))
    {
        std::cout << "pos : " << smResult.position() << ", length : " << smResult.length() << '\n';
        std::cout << smResult.str() << '\n';

        str = smResult.suffix();
    }
#elif MODE == 1
    std::smatch smResult;

    auto iter = str.cbegin();

    while (std::regex_search(iter, str.cend(), smResult, pattern))
    {
        std::cout << "pos : " << smResult.position() << ", length : " << smResult.length() << '\n';
        std::cout << smResult.str() << '\n';

        iter += smResult.position() + smResult.length();
    }

#elif MODE == 2
    std::smatch smResult;

    auto iter = str.cbegin();

    // match_continuous를 쓰면 iter의 첫 번째 문자는 패턴의 첫 번째 문자와 반드시 일치해야 함.
    // pattern(R"(\b\w{5}\b )"); <- 이걸로 테스트해볼 것(두 번째 \b 뒤에 띄어쓰기 필수)
    while (std::regex_search(iter, str.cend(), smResult, pattern, std::regex_constants::match_continuous))
    {
        std::cout << "pos : " << smResult.position() << ", length : " << smResult.length() << '\n';
        std::cout << smResult.str() << '\n';

        iter += smResult.position() + smResult.length();
    }

    // if (std::regex_search(..., std::regex_constants::match_continuous))
    // while로 순회하지 않고 이렇게 쓰는 방식이라면 match_continuous는 메타기호 ^와 동일한 역할을 수행한다고 봐도 됨.
    // 
    // 엄밀하게 말하자면 ^와 match_continuous는 기능의 차이가 있긴 함.
    // ^는 전체 패턴을 문자열의 시작부터 일치시키고자 할 때 사용하는 메타기호이며 정규 표현식의 시작 위치를 의미.
    // match_continuous는 패턴이 이전에 일치한 항목 뒤에서 연속해서 나타나는 경우만 매치되도록 하며
    // 이는 문자열의 시작 위치에만 국한되지 않고 이전 일치 항목 뒤에서도 연속하여 일치해야 함.
    // ex) str = "abcabcabc", pattern = "abc" -> 반복문 순회 -> "abc", "abc", "abc"

#elif MODE == 3
    std::smatch smResult;

    auto iter = str.cbegin();

    while (std::regex_search(iter, str.cend(), smResult, pattern))
    {
        // const auto& smIter = smResult.begin();
        // const auto& smElem = *smIter; // std::ssub_match(cmatch를 쓰면 std::csub_match로 받아오는데 이 부분은 tmi)

        std::cout << smResult.str() << " : ";

        for (auto iter = smResult[0].first; iter != smResult[0].second; iter++)
        {
            auto elem = *iter; // std::string::const_iterator::value_type : 문자 하나라는 뜻(문자열 말고). str.begin()으로 확인하면 됨.

            std::cout << elem << ' '; // 매칭된 것을 문자 하나 단위로 출력
        }

        std::cout << '\n';

        iter += smResult.position() + smResult.length();
    }
#endif

    return 0;
}
