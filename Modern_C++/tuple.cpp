#include <iostream>
#include <tuple>

int main()
{
    using namespace std;

    // C++11
    std::tuple<char, int, float> myTuple = std::make_tuple('A', 0, 0.0f);

    // C++14
    cout << std::get<0>(myTuple) << ' ' 
        << std::get<1>(myTuple) << ' '
        << std::get<2>(myTuple) << '\n';

    std::get<0>(myTuple) = 'C';
    std::get<1>(myTuple) = 100;
    std::get<2>(myTuple) = 3.14f;

    // C++17
    auto[ch, iVal, fVal] = myTuple;

    cout << ch << ' ' << iVal << ' ' << fVal << '\n';

    // C++14
    ch = 'A';
    iVal = 0;
    fVal = 0.0f;

    std::tie(ch, iVal, fVal) = myTuple;

    cout << ch << ' ' << iVal << ' ' << fVal << '\n';

    return 0;
}
