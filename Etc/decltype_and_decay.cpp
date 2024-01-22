#include <iostream>
#include <typeinfo>

template <typename T, typename U>
auto Max(const T& x, const U& y) -> decltype(x + y)
{
    return (x > y) ? x : y;
}

template <typename T, typename U>
auto Add(const T& x, const U& y) -> decltype(x + y)
{
    return x + y;
}

void Run01()
{
    float val1 = 10.0f;
    int   val2 = 12;

    decltype(Max(val1, val2)) res1 = Max(val1, val2);
    auto res2 = Add(val1, val2);

    // float으로 캐스팅된 것을 확인
    std::cout << typeid(res1).name() << " : " << res1 << '\n';
    std::cout << typeid(res2).name() << " : " << res2 << '\n';
}

void Run02()
{
    // std::decay는 전달받은 타임을 기본형으로 변환하는 템플릿 메타 함수이다.

    /**
     * template에서는 다음과 같이 사용할 수 있음
     * using DecayType = typename std::decay<T>::type;
     */ 
    using Type = typename std::decay<decltype(10.0 + 10.0f)>::type;

    Type type = 100;

    std::cout << typeid(type).name() << " : " << type << '\n';
}

int main()
{
    Run01();

    std::cout << "--------------------\n";

    Run02();

    return 0;
}
