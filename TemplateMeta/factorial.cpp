#include <bits/stdc++.h>

using namespace std;

template <int tN>
struct Factorial
{
    static constexpr int val = tN * Factorial<tN - 1>::val;
};

// 템플릿 특수화
template <>
struct Factorial<0>
{
    static constexpr int val = 1;
};

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    cout << Factorial<5>::val << '\n';
    cout << Factorial<4>::val << '\n';
    cout << Factorial<3>::val << '\n';
    cout << Factorial<2>::val << '\n';
    cout << Factorial<1>::val << '\n';

    return 0;
}
