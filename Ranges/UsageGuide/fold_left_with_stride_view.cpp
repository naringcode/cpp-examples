// Update Date : 2025-06-23
// OS : Windows 10 64bit
// Program : vscode(gcc-14.2.0)
// Version : C++23
// Configuration : None

#include <bits/stdc++.h>

using namespace std;

// https://en.cppreference.com/w/cpp/algorithm/ranges/fold_left
// https://en.cppreference.com/w/cpp/algorithm/ranges/fold_right
// https://en.cppreference.com/w/cpp/ranges/stride_view.html

// fold_left 계산 방식 : ((1 @ 2) @ 3) @ 4
// fold_right 계산 방식 : 1 @ (2 @ (3 @ 4))

int main()
{
    // 홀수와 짝수를 5개 가져오는 뷰(filter 방식)
    // auto oddView  = views::iota(1) | views::filter([](int n) {
    //     return n % 2 == 1;
    // }) | views::take(5);
    //
    // auto evenView  = views::iota(1) | views::filter([](int n) {
    //     return n % 2 == 0;
    // }) | views::take(5);

    // 홀수와 짝수를 5개 가져오는 뷰(stride 방식)
    auto oddView  = views::iota(1) | views::stride(2) | views::take(5);
    auto evenView = views::iota(2) | views::stride(2) | views::take(5);

    // basic usages
    // int oddSum  = ranges::fold_left(oddView,  0, std::plus{ });
    // int evenSum = ranges::fold_left(evenView, 0, std::plus{ });

    // functor usages
    // struct MyPlus
    // {
    //     string tag;
    //     int cnt = 0;
    //
    //     int operator()(int lhs, int rhs)
    //     {
    //         println("{} - {} : {} + {} = {}", tag, cnt++, lhs, rhs, lhs + rhs);
    //
    //         return lhs + rhs;
    //     }
    // };
    //
    // int oddSum  = ranges::fold_left(oddView, 0, MyPlus{ "Odd" });
    // int evenSum = ranges::fold_left(oddView, 0, MyPlus{ "Even" });
    
    // lambda usages
    int oddSum  = ranges::fold_left(oddView, 0, [](int lhs, int rhs) {
        println("Odd : {} + {} = {}", lhs, rhs, lhs + rhs);

        return lhs + rhs;
    });

    int evenSum = ranges::fold_left(evenView, 0, [](int lhs, int rhs) {
        println("Even : {} + {} = {}", lhs, rhs, lhs + rhs);

        return lhs + rhs;
    });

    println("oddSum : {}", oddSum);
    println("evenSum : {}", evenSum);

    return 0;
}
