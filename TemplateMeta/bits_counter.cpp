#include <bits/stdc++.h>

using namespace std;

template <int tValue, int tBits>
struct RequiredBitsHelper
{
    // tValue의 값을 2로 나누고, tBits를 1 증가시킨다(2를 곱한다는 뜻).
    static constexpr int val = RequiredBitsHelper<(tValue >> 1), tBits + 1>::val;
};

// 탈출 기저 조건(tValue의 값이 0이 되는 순간)
template <int tBits>
struct RequiredBitsHelper<0, tBits>
{
    static constexpr int val = tBits;
};

// 진입 구조체
template <int tValue>
struct RequiredBits
{
    enum
    {
        val = RequiredBitsHelper<tValue, 0>::val
    };
};

// test suite enum
enum class TestEnum
{
    First,
    Second,
    Third,
    Fourth,
    Fifth,
    Max
};

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    // 3 bits(0 ~ 7 | 111)
    cout << RequiredBits<(int)TestEnum::Max>::val << '\n';

    cout << RequiredBits<1022>::val << '\n'; // 10 bits(0 ~ 1023 | 11'1111'1111)
    cout << RequiredBits<1023>::val << '\n'; // 10 bits(0 ~ 1023 | 11'1111'1111)
    cout << RequiredBits<1024>::val << '\n'; // 11 bits(1024 ~ 2047 | 111'1111'1111)
    cout << RequiredBits<1025>::val << '\n'; // 11 bits(1024 ~ 2047 | 111'1111'1111)
    cout << RequiredBits<1026>::val << '\n'; // 11 bits(1024 ~ 2047 | 111'1111'1111)
    cout << RequiredBits<2048>::val << '\n'; // 12 bits(2048 ~ 4095 | 1111'1111'1111)

    return 0;
}
