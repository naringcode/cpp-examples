#include <iostream>

// Structured Binding (C++ 17)
// https://en.cppreference.com/w/cpp/language/structured_binding
//
// 1. auto []
// 2. auto& []
// 3. auto&& []

// 배열 타입으로 받는 경우
void Run01()
{
    int arr[3] = { 1, 10, 100 };

    auto [a, b, c] = arr;

    arr[0] = 2;
    arr[1] = 20;
    arr[2] = 200;

    std::cout << "[" << arr[0] << ", " << arr[1] << ", " << arr[2] << "] / ";
    std::cout << "[" << a << ", " << b << ", " << c << "]\n";

    auto& [d, e, f] = arr;

    arr[0] = 3;
    arr[1] = 30;
    arr[2] = 300;

    std::cout << "[" << arr[0] << ", " << arr[1] << ", " << arr[2] << "] / ";
    std::cout << "[" << d << ", " << e << ", " << f << "]\n";
}

// 구조체 멤버로 받는 경우
void Run02()
{
    struct Test
    {
        int a;
        int b;
    };

    Test test{ 1, 10 };

    auto [a, b] = test;

    test.a = 2;
    test.b = 20;

    std::cout << "[" << test.a << ", " << test.b << "] / ";
    std::cout << "[" << a << ", " << b << "]\n";
}

// tuple이나 pair로 받는 경우
void Run03()
{
    std::tuple<int, int, int> myTuple{ 1, 10, 100 }; // pair로 써도 똑같다.

    // auto&& 형태로 받는다(실수하면 dangling pointer 문제가 발생할 수도 있을 것 같은데?).
    auto [a, b, c] = myTuple;

    a = 2;
    b = 20;
    c = 200;

    std::cout << "[" << std::get<0>(myTuple) << ", " << std::get<1>(myTuple) << ", " << std::get<2>(myTuple) << "] / ";
    std::cout << "[" << a << ", " << b << ", " << c << "]\n";

    auto& [d, e, f] = myTuple;

    d = 2;
    e = 20;
    f = 200;

    std::cout << "[" << std::get<0>(myTuple) << ", " << std::get<1>(myTuple) << ", " << std::get<2>(myTuple) << "] / ";
    std::cout << "[" << d << ", " << e << ", " << f << "]\n";

}

// 독립된 변수로 받는 경우
void Run04()
{
    int a = 10;
    int b = 100;
    
    // & 타입으로 받는다(int&).
    // 이건 std::tie()의 특성 때문에 그런 것이다.
    auto [x, y] = std::tie(a, b);

    a = 20;
    b = 200;

    std::cout << "[" << a << ", " << b << "]" << " / [" << x << ", " << y << "]\n";

    // 독립된 형태로 받는다(int)
    auto [z, w] = std::make_pair(a, b); // std::make_tuple()로 써도 됨.

    a = 30;
    b = 300;

    std::cout << "[" << a << ", " << b << "]" << " / [" << z << ", " << w << "]\n";
}

int main()
{
    Run01();

    std::cout << "----------------------------------------\n";

    Run02();

    std::cout << "----------------------------------------\n";

    Run03();

    std::cout << "----------------------------------------\n";

    Run04();

    return 0;
}
