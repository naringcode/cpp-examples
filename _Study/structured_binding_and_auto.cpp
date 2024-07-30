#include <iostream>
#include <vector>

// Structured Binding (C++ 17)
// https://en.cppreference.com/w/cpp/language/structured_binding
//
// 1. auto [] : Copy
// 2. auto& [] : Reference
// 3. auto&& [] : **Universal Reference** (RValue Reference 아님)
//
// int&& 이런 건 RValue Reference 형태로 받지 Universal Reference 형태로 받는 것이 아니다.
// auto&&가 Universal Reference 형태로 받아서 참으로 헷갈리는 부분.
//
// 보편 참조는 왼쪽값 참조(LValue Refefence)와 오른값 참조(RValue Reference)를 동시에 처리하기 위한 문법이다.
// 보편 참조는 템플릿에서도 유용하게 활용된다(이 경우에는 std::move()가 아닌 std::forward()로 완벽한 전달이 필요).
// 자세한 사항은 "Modern Effective C++"의 "5장 오른값 참조, 이동 의미론, 완벽 전달" 참고.

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

// 독립된 변수로 받는 경우
void Run02()
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

// 구조체 멤버로 받는 경우
void Run03()
{
    struct Test
    {
        int a;
        int b;
    };

    Test test{ 1, 10 };

    // 복사
    auto [a, b] = test;

    test.a = 2;
    test.b = 20;

    std::cout << "[" << test.a << ", " << test.b << "] / ";
    std::cout << "[" << a << ", " << b << "]\n";

    std::cout << "-----\n";

    // 참조
    auto& [c, d] = test;

    c = 3;
    d = 30;

    std::cout << "[" << test.a << ", " << test.b << "] / ";
    std::cout << "[" << c << ", " << d << "]\n";

    std::cout << "-----\n";

    // LValue Reference 형태로 받는 경우
    auto&& [e, f] = test;

    e = 4;
    f = 40;

    std::cout << "[" << test.a << ", " << test.a << "] / ";
    std::cout << "[" << e << ", " << f << "]\n";

    std::cout << "-----\n";

    // RValue Reference 형태로 받는 경우
    auto&& [g, h] = std::move(test);

    // RValue가 아닌 RValue Reference 형태로 받은 것이다.
    // g, h가 구조체를 받을 때는 RValue Reference 형태이지만 사용할 때는 LValue이다.
    g = 5;
    h = 50;

    std::cout << "[" << test.a << ", " << test.a << "] / ";
    std::cout << "[" << g << ", " << h << "]\n";
}

// tuple이나 pair로 받는 경우
void Run04()
{
    class Data
    {
    public:
        int data;

        Data(int data) : data(data) 
        {
            std::cout << "Constructor : " << data << '\n';
        }

        ~Data()
        {
            std::cout << "Destructor : " << data << '\n';
        }

        Data(const Data& rhs) : data(rhs.data)
        {
            std::cout << "Copy Constructor : " << data << '\n';
        }
        
        Data& operator=(const Data& rhs)
        {
            data = rhs.data;
        
            std::cout << "Copy Assignment : " << data << '\n';
        
            return *this;
        }

        Data(Data&& rhs) noexcept : data(rhs.data)
        {
            std::cout << "Move Constructor : " << data << '\n';
        }
        
        Data& operator=(Data&& rhs) noexcept
        {
            data = rhs.data;
        
            std::cout << "Move Assignment : " << data << '\n';
        
            return *this;
        }
    };

    std::tuple<Data, Data, Data> myTuple{ Data{1}, Data{10}, Data{100} }; // pair로 써도 똑같다.

    std::cout << "-----\n";

    // 값을 복사하는 형태로 받는다.
    auto [a, b, c] = myTuple;

    a.data = 2;
    b.data = 20;
    c.data = 200;

    std::cout << "[" << std::get<0>(myTuple).data << ", " << std::get<1>(myTuple).data << ", " << std::get<2>(myTuple).data << "] / ";
    std::cout << "[" << a.data << ", " << b.data << ", " << c.data << "]\n";

    std::cout << "-----\n";

    // auto&는 참조 형태로 받는다.
    // 값을 LValue Reference하는 형태로 받는다.
    auto& [d, e, f] = myTuple;

    d.data = 3;
    e.data = 30;
    f.data = 300;

    std::cout << "[" << std::get<0>(myTuple).data << ", " << std::get<1>(myTuple).data << ", " << std::get<2>(myTuple).data << "] / ";
    std::cout << "[" << d.data << ", " << e.data << ", " << f.data << "]\n";

    std::cout << "-----\n";

    // auto&&에는 보편 참조가 적용되기 때문에 무조건 RValue Reference로 받는 것은 아니다.
    // 다음 형태는 RValue Reference로 받는다.
    auto&& [g, h, i] = myTuple;

    g.data = 4;
    h.data = 40;
    i.data = 400;

    std::cout << "[" << std::get<0>(myTuple).data << ", " << std::get<1>(myTuple).data << ", " << std::get<2>(myTuple).data << "] / ";
    std::cout << "[" << g.data << ", " << h.data << ", " << i.data << "]\n";

    std::cout << "-----\n";

    // auto&&에 RValue Reference를 적용시키기 위해선 말 그대로 RValue 형태로 넘어와야 한다.
    // Reference 형태로 받기 때문에 이동 생성자는 호출되지 않는다(std::move()의 이름 때문에 더 헷갈림).
    // 매번 생각하는 내용이지만 왜 std::rvalue()가 아닌 std::move()라고 했는지 모르겠다.
    auto&& [j, k, l] = std::move(myTuple);

    j.data = 5;
    k.data = 50;
    l.data = 500;

    std::cout << "[" << std::get<0>(myTuple).data << ", " << std::get<1>(myTuple).data << ", " << std::get<2>(myTuple).data << "] / ";
    std::cout << "[" << j.data << ", " << k.data << ", " << l.data << "]\n";

    // 이동 생성자는 다음과 같이 복사 형태로 새로운 객체를 만들 때 그 대상이 RValue여야 호출된다.
    auto [m, n, o] = std::move(myTuple);

    m.data = 6;
    n.data = 60;
    o.data = 600;

    std::cout << "[" << std::get<0>(myTuple).data << ", " << std::get<1>(myTuple).data << ", " << std::get<2>(myTuple).data << "] / ";
    std::cout << "[" << m.data << ", " << n.data << ", " << o.data << "]\n";

    std::cout << "-----\n";
}

// 범위 기반 for 문에서 사용하는 경우
void Run05()
{
    std::vector<std::pair<int, int>> pairVec;
    {
        pairVec.push_back({ 1, 10 });
        pairVec.push_back({ 2, 20 });
        pairVec.push_back({ 3, 30 });
        pairVec.push_back({ 4, 40 });
        pairVec.push_back({ 5, 50 });
    }

    for (auto [x, y] : pairVec)
    {
        std::cout << x << ", " << y << '\n';
    }

    std::cout << "-----\n";

    struct Point
    {
        int a;
        int b;
    };

    std::vector<Point> pointVec;
    {
        pointVec.push_back({ 6, 60 });
        pointVec.push_back({ 7, 70 });
        pointVec.push_back({ 8, 80 });
        pointVec.push_back({ 9, 90 });
        pointVec.push_back({ 10, 100 });
    }

    for (auto [x, y] : pointVec)
    {
        std::cout << x << ", " << y << '\n';
    }
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

    std::cout << "----------------------------------------\n";

    Run05();

    return 0;
}
