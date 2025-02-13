// Update Date : 2025-02-14
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20, C++23
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <random>
#include <tuple>
#include <algorithm>
#include <ranges>
#include <vector>
#include <map>

// Concepts에 대한 내용을 충분히 숙지한 다음 Ranges를 보도록 한다.
// Ranges의 동작 방식을 잘 이해하기 위해선 Concepts에 대한 선행 학습이 필요하다.
// 
// 다음 순서대로 보도록 하자.
// 
// # Ranges 개요
// 1. ranges_intro.txt
// 
// # Ranges 알고리즘
// 2. Concepts/CustomContainerIterators(사전지식)
// 3. range_concepts_and_iterator_concepts.cpp
// 4. legacy_algorithms_and_range_algorithms.cpp
// 5. return_types_by_range_algorithms.cpp
// 
// # Projections
// 6. implementation_of_invoke.cpp(사전지식)
// 7. range_algorithms_with_projections.cpp
// 8. implementation_of_projections.cpp
// 
// # Views
// 9. views_by_constructors.cpp <-----
// 10. views_by_range_adaptors.cpp
// 11. algorithms_with_and_without_views.cpp
// 12. view_composition.cpp
// 13. lazy_evaluation_of_views.cpp
// 14. view_working_with_range_algorithms.cpp
// 
// # Range Factories
// 15. range_factories.cpp
//

// https://learn.microsoft.com/en-us/cpp/standard-library/view-classes?view=msvc-170
// https://en.cppreference.com/w/cpp/ranges#Range_adaptors

// 모든 view 클래스는 ranges 네임스페이스 안에 있다.

#define BEGIN_NS(name) namespace name {
#define END_NS }

template <std::ranges::range Container>
void Print(Container&& container, std::string_view msg = "", char separator = ' ')
{
    if (msg == "")
    {
        std::cout << "[ ";
    }
    else
    {
        std::cout << std::format("{:<12} : [ ", msg);
    }

    for (const auto& elem : container)
    {
        std::cout << std::format("{}{}", elem, separator);
    }

    std::cout << "]\n";
}

BEGIN_NS(Case01)

// 컨테이너를 단일 원소의 시퀀스로 조회하는 View에 대한 예시

void Run()
{
    std::vector<int> data(10);
    std::ranges::generate(data, []() { return std::random_device{ }() % 100; });

    std::ranges::sort(data);

    // 정렬되지 않은 임의의 요소 추가
    data.push_back(30);
    data.push_back(40);
    data.push_back(50);
    data.push_back(60);
    data.push_back(70);

    Print(data, "data");

    // https://en.cppreference.com/w/cpp/ranges/ref_view
    // https://en.cppreference.com/w/cpp/ranges/all_view
    // ref_view : range를 view로 사용하기 위한 view
    {
        // 모든 view는 range가 될 수 있지만 반대로 모든 range를 view로 사용할 수 있는 건 아니다.
        // 하지만 range를 ref_view로 감싸면 view처럼 사용할 수 있다.
        std::ranges::ref_view<std::vector<int>> refView{ data };

        Print(refView, "ref_view");
    }

    std::cout << '\n';

    // https://en.cppreference.com/w/cpp/ranges/filter_view
    // filter_view : Predicate의 조건을 만족하는 element만 조회하기 위한 view
    {
        auto even = [](int elem) { return elem % 2 == 0; };
        auto odd  = [](int elem) { return elem % 2 != 0; };

        // (주의) 다음과 같이 컨테이너를 직접 지정하는 방식은 불가능하다.
        // std::ranges::filter_view<std::vector<int>, decltype(even)> filterEvenView{ data, even };

        // 컨테이너를 view로 사용할 때는 ref_view로 감싸야 한다(STL 컨테이너는 range이긴 하나 그 자체로 view인 것은 아님).
        std::ranges::filter_view<std::ranges::ref_view<std::vector<int>>, decltype(even)>
            filterEvenView{ data, even };

        // 템플릿 인자를 생략하여 컴파일러가 추론하는 방식을 사용하는 것도 가능하다(!! 안 되는 유형도 있으니 주의 !!)
        std::ranges::filter_view/*<std::ranges::ref_view<std::vector<int>>, decltype(odd)>*/
            filterOddView{ data, odd };

        // view 자체는 range이기 때문에 ranged-based 로직이나 알고리즘을 사용할 수 있다.
        std::cout << "filter_view(even) : ";

        // ranged-based for
        for (int elem : filterEvenView)
        {
            std::cout << elem << ' ';
        }

        // ranges::for_each()
        std::cout << "\nfilter_view(odd)  : ";
        std::ranges::for_each(filterOddView, [](int elem) { std::cout << elem << ' '; });
    }

    std::cout << "\n\n";

    // https://en.cppreference.com/w/cpp/ranges/transform_view
    // transform_view : 원본으로부터 원소를 받아 이를 변형시켜서 조회하기 위한 view
    {
        // 레퍼런스 형태로 받을 경우 elem에 변형을 가하면 원본 값도 바뀌니 주의해야 한다.
        auto twice = [](const int& elem) { return elem * 2; };

        std::ranges::transform_view/*<std::ranges::ref_view<std::vector<int>>, decltype(twice)>*/
            transformView{ data, twice };

        Print(transformView, "transform_view");
        
        // 원본은 바뀌지 않는다.
        Print(data, "original");
    }

    std::cout << '\n';

    // https://en.cppreference.com/w/cpp/ranges/take_view
    // take_view : 다른 view나 range로부터 처음 N개의 요소를 조회하는 view
    {
        // 이런 식으로 생성하는 것도 가능하다.
        std::ranges::take_view/*<std::ranges::ref_view<std::vector<int>>>*/ takeView =
            std::ranges::take_view/*<std::ranges::ref_view<std::vector<int>>>*/{data, 3};

        Print(takeView, "take_view");
    }

    std::cout << '\n';

    // https://en.cppreference.com/w/cpp/ranges/take_while_view
    // take_while_view : Predicate가 false를 반환할 때까지 요소를 계속해서 조회하는 view
    {
        // 마찬가지로 레퍼런스 형태로 받을 경우 elem에 변형을 가하면 원본 값도 바뀌니 주의해야 한다.
        auto under50 = [](const int& elem) { return elem < 50; };

        std::ranges::take_while_view/*<std::ranges::ref_view<std::vector<int>>, decltype(under50)>*/
            takeWhileView{ data, under50 };

        // 임의로 넣은 30, 40은 출력하지 않는다.
        Print(takeWhileView, "take_while_view");
    }

    std::cout << '\n';

    // https://en.cppreference.com/w/cpp/ranges/drop_view
    // drop_view : 다른 view나 range로부터 처음 N개의 요소를 스킵하고 그 이후 요소를 조회하는 view
    {
        auto dropView = std::ranges::drop_view/*<std::ranges::ref_view<std::vector<int>>>*/{ data, 5 };

        Print(dropView, "drop_view");
    }

    std::cout << '\n';

    // https://en.cppreference.com/w/cpp/ranges/drop_while_view
    // drop_while_view : Predicate가 false를 반환할 때까지 요소를 스킵한 다음 true를 반환한 지점부터 조회하는 view
    {
        // 마찬가지로 레퍼런스 형태로 받을 경우 elem에 변형을 가하면 원본 값도 바뀌니 주의해야 한다.
        auto under50 = [](const int& elem) { return elem < 50; };

        std::ranges::drop_while_view/*<std::ranges::ref_view<std::vector<int>>, decltype(under50)>*/
            dropWhileView{ data, under50 };

        // 임의로 넣은 모든 요소를 출력할 확률이 높다.
        Print(dropWhileView, "drop_while_view");
    }

    std::cout << '\n';

    // https://en.cppreference.com/w/cpp/ranges/join_view
    // join_view : view나 range의 요소를 평탄화(flattening)하여 조회하기 위한 view
    {
        // 단일 값 자체는 평탄화하는 것이 불가능하다(에러 발생).
        // std::ranges::join_view<std::ranges::ref_view<std::vector<int>>> joinView{ data };

        // 숫자 원소 평탄화
        std::vector<std::vector<int>> nestedData{
            { 1 }, { 2, 3 }, { 4, 5, 6 }, { 7, 8, 9, 10 }
        };

        std::ranges::join_view/*<std::ranges::ref_view<std::vector<std::vector<int>>>>*/
            joinViewNums{ nestedData };

        Print(joinViewNums, "join_view 1");

        // 문자열 원소 평탄화
        std::vector<std::string> names{
            "James", "Mary", "David", "Susan", "Michael"
        };

        std::ranges::join_view/*<std::ranges::ref_view<std::vector<std::string>>>*/
            joinViewNames{ names };

        Print(joinViewNames, "join_view 2"); // string을 컨테이너로 취급하여 문자 하나하나를 평탄화함.
    }

    std::cout << '\n';

    // https://en.cppreference.com/w/cpp/ranges/join_with_view
    // join_with_view (C++23) : join_view를 보강한 view로 delimiter를 추가할 수 있다.
    {
        using namespace std::string_literals;

        // 문자열 원소 평탄화
        std::vector<std::string> names{
            "James", "Mary", "David", "Susan", "Michael"
        };

        std::string delim = "###";
        auto singleViewDelim = std::ranges::single_view(delim);

        // owning_view는 아주 예외적으로 데이터를 가지는 view(r-value를 직접 전달한 유형)
        std::ranges::join_with_view<std::ranges::ref_view<std::vector<std::string>>, std::ranges::owning_view<std::string>>
            joinWithView1{ names, "###"s };

        Print(joinWithView1, "join_with_view 1"); // delimiter조차 평탄화 대상임.

        // r-value가 아닌 l-value로 전달하면 원본이 있다는 뜻이니 ref_view로 받는다.
        std::ranges::join_with_view<std::ranges::ref_view<std::vector<std::string>>, std::ranges::ref_view<std::string>>
            joinWithView2{ names, delim };        

        Print(joinWithView2, "join_with_view 2"); // delimiter조차 평탄화 대상임.

        // single_view는 Range factory로 단 하나의 요소를 Range로 감싼 view이다(여기선 char 하나를 감쌈).
        std::ranges::join_with_view<std::ranges::ref_view<std::vector<std::string>>, std::ranges::single_view<char>>
            joinWithView3{ names, '#' };

        Print(joinWithView3, "join_with_view 3"); // delimiter조차 평탄화 대상임.

        // 가능할 것 같은데 해당 유형은 사용할 수 없다.
        // Visual Studio 2022 기준 아직 C++23이 실험적인 기능이라 지원하지 않는 것으니 나중에 확인해봐야 한다.
        // std::ranges::join_with_view<std::ranges::ref_view<std::vector<std::string>>, std::ranges::single_view<std::string>>
        //     joinWithView4{ names, singleViewDelim };
        // 
        // Print(joinWithView4, "join_with_view 3"); // delimiter조차 평탄화 대상임.
    }

    std::cout << '\n';

    // https://en.cppreference.com/w/cpp/ranges/reverse_view
    // reverse_view : view나 range의 요소를 역순으로 조회하기 위한 view
    {
        auto reverseView = std::ranges::reverse_view<std::ranges::ref_view<std::vector<int>>>{ data };

        Print(reverseView, "reverse_view");
    }

    std::cout << '\n';
}

END_NS

BEGIN_NS(Case02)

// tuple(혹은 tuple-like)처럼 복합 원소로 구성된 컨테이너로부터 특정 위치의 값을 조회하기 위한 View에 대한 예시

void Run()
{
    using Tuple = std::tuple<int, char, std::string>;
    using Pair  = std::pair<int, std::string>;

    // data 1
    std::vector<Tuple> tuples{
        { 100, 'A', "James" },
        { 200, 'B', "Mary" },
        { 300, 'C', "David" },
        { 400, 'D', "Susan" },
        { 500, 'E', "Michael" },
    };

    // data 2
    std::vector<Pair> pairs{
        { 1, "one" },
        { 2, "two" },
        { 3, "three" },
        { 4, "four" }
    };

    // data 3
    std::map<std::string, std::string> mp{
        { "Korea", "Seoul" },
        { "Japan", "Tokyo" },
        { "China", "Beijing" },
    };

    // https://en.cppreference.com/w/cpp/ranges/elements_view
    // elements_view : tuple 스타일 자료형에서 특정 위치에 있는 요소를 조회하기 위한 view
    {
        // elements_view에서 특정 요소만 조회하려면 원본 컨테이너를 레퍼런스 형태로 알고 있어야 한다.
        std::ranges::elements_view<std::views::all_t<std::vector<Tuple>&>, 0>    tupleCol0_view{ tuples };
        std::ranges::elements_view<std::ranges::ref_view<std::vector<Tuple>>, 1> tupleCol1_view{ tuples };
        std::ranges::elements_view<std::ranges::ref_view<std::vector<Tuple>>, 2> tupleCol2_view{ tuples };

        // (주의) elements_view는 특정 위치를 템플릿 인자로 지정해야 하기 때문에 인자를 추론하는 방식은 사용하지 못 한다.
        // 다음 코드는 사용 불가
        // std::ranges::elements_view/*<std::views::all_t<std::vector<Tuple>&>, 0>*/    tupleCol0_view{ tuples };
        // std::ranges::elements_view/*<std::ranges::ref_view<std::vector<Tuple>>, 1>*/ tupleCol1_view{ tuples };

        Print(tupleCol0_view, "tuple col 0");
        Print(tupleCol1_view, "tuple col 1");
        Print(tupleCol2_view, "tuple col 2");

        std::cout << '\n';

        // pair 또한 tuple-like 자료형이기 때문에 elements_view를 사용할 수 있다.
        auto pairFirstView  = std::ranges::elements_view<std::views::all_t<std::vector<Pair>&>, 0>{ pairs };
        auto pairSecondView = std::ranges::elements_view<std::ranges::ref_view<std::vector<Pair>>, 1>{ pairs };
      
        Print(pairFirstView, "pair first");
        Print(pairSecondView, "pair second");

        std::cout << '\n';

        // map도 어떻게 보면 첫 번째 위치는 key, 두 번째 위치는 value로 볼 수 있기 때문에 elements_view를 사용할 수 있다.
        auto mapFirstView  = std::ranges::elements_view<std::views::all_t<std::map<std::string, std::string>&>, 0>{ mp };
        auto mapSecondView = std::ranges::elements_view<std::ranges::ref_view<std::map<std::string, std::string>>, 1>{ mp };
      
        Print(mapFirstView, "map first");
        Print(mapSecondView, "map second");
    }
    
    std::cout << '\n';

    // https://en.cppreference.com/w/cpp/ranges/keys_view
    // https://en.cppreference.com/w/cpp/ranges/values_view
    // keys_view와 values_view는 elements_view을 특수화한 별칭이다.
    // keys_view   : elements_view에서 첫 번째 값(0번)만 추출하는 view
    // values_view : elements_view에서 두 번째 값(1번)만 추출하는 view
    {
        // tuple은 엄밀히 말하자면 key, value가 묶인 pair와 같은 요소는 아니다.
        // 하지만 keys_view와 values_view는 elements_view를 특수화한 형태이기 때문에 아래 코드는 사용 가능하다.
        std::ranges::keys_view<std::views::all_t<std::vector<Tuple>&>>      tupleKeysView{ tuples };
        std::ranges::values_view<std::ranges::ref_view<std::vector<Tuple>>> tupleValuesView{ tuples };

        // (주의) 실제로는 위치를 템플릿 인자로 넣는 elements_view를 사용하기 때문에 인자를 추론하는 방식은 사용하지 못 한다.
        // 다음 코드는 사용 불가
        // std::ranges::keys_view/*<std::views::all_t<std::vector<Tuple>&>>*/      tupleKeysView{tuples};
        // std::ranges::values_view/*<std::ranges::ref_view<std::vector<Tuple>>>*/ tupleValuesView{tuples};

        Print(tupleKeysView, "tuple keys?");
        Print(tupleValuesView, "tuple values?");

        std::cout << '\n';

        // keys_view와 values_view를 사용하는 정석적인 예시 1
        auto pairKeysView   = std::ranges::keys_view<std::views::all_t<std::vector<Pair>&>>{ pairs };
        auto pairValuesView = std::ranges::values_view<std::ranges::ref_view<std::vector<Pair>>>{ pairs };

        Print(pairKeysView, "pair keys");
        Print(pairValuesView, "pair values");

        std::cout << '\n';

        // keys_view와 values_view를 사용하는 정석적인 예시 2
        auto mapKeysView   = std::ranges::keys_view<std::views::all_t<std::map<std::string, std::string>&>>{ mp };
        auto mapValuesView = std::ranges::values_view<std::ranges::ref_view<std::map<std::string, std::string>>>{ mp };

        Print(mapKeysView, "map keys");
        Print(mapValuesView, "map values");
    }
}

END_NS

// tuple-like 스타일로 맞췄는데 Visual Studio 2022에서 동작하지 않는다.
// 추후 Visual Studio 버전이나 컴파일러 사양이 올라갔을 때 확인해봐야 할 것 같다.
// 
// MSVC에서 elements_view를 적용하는 과정에서 _Tuple_like 특수화가 필요한 것 같아서 넣었는데 안 되고
// 또한 cppreference에서 확인해보면 tuple-like 스타일은 _Tuple_like를 요구하지 않는다.
// 
// elements_view에 tuple-like 타입을 넣는 건 비교적 최신 기능이라 아직 컴파일러가 제대로 지원하지 않는 것 같다.
/*
struct Point // C++23 tuple-like stype(https://en.cppreference.com/w/cpp/utility/tuple/tuple-like)
{
    int x;
    int y;
    int z;
    int w;

    Point(int x, int y, int z, int w)
        : x{ x }, y{ y }, z{ z }, w{ w }
    { }
};

namespace std
{
    // std::tuple_size 특수화
    template <>
    struct tuple_size<Point> : integral_constant<size_t, 4> { };

    // std::tuple_element 특수화
    template <size_t N>
    struct tuple_element<N, Point> { using type = int; };

    // std::tuple_element 특수화
    template <>
    struct tuple_element<0, Point> { using type = int; };

    // std::tuple_element 특수화
    template <>
    struct tuple_element<1, Point> { using type = int; };

    // std::tuple_element 특수화
    template <>
    struct tuple_element<2, Point> { using type = int; };

    // std::tuple_element 특수화
    template <>
    struct tuple_element<3, Point> { using type = int; };

    // _Tuple_like 특수화(MSVC 기준)
    template <>
    constexpr bool _Tuple_like_impl<Point> = true;

    template <std::size_t N>
        requires (N >= 0 && N <= 3)
    consteval int& get(Point& p)
    {
        if constexpr (N == 0)
        {
            return p.x;
        }
        else if constexpr (N == 1)
        {
            return p.y;
        }
        else if constexpr (N == 2)
        {
            return p.z;
        }
        else // if constexpr (N == 3)
        {
            return p.w;
        }
    }
    
    template <std::size_t N>
        requires (N >= 0 && N <= 3)
    consteval const int& get(const Point& p)
    {
        if constexpr (N == 0)
        {
            return p.x;
        }
        else if constexpr (N == 1)
        {
            return p.y;
        }
        else if constexpr (N == 2)
        {
            return p.z;
        }
        else // if constexpr (N == 3)
        {
            return p.w;
        }
    }
}

void Run()
{
    std::vector<Point> points;
    
    points.emplace_back(1001, 1002, 1003, 1004);
    points.emplace_back(2001, 2002, 2003, 2004);
    points.emplace_back(3001, 3002, 3003, 3004);
    points.emplace_back(4001, 4002, 4003, 4004);
    points.emplace_back(5001, 5002, 5003, 5004);
    points.emplace_back(6001, 6002, 6003, 6004);
    points.emplace_back(7001, 7002, 7003, 7004);
    points.emplace_back(8001, 8002, 8003, 8004);
    points.emplace_back(9001, 9002, 9003, 9004);

    // TEMP
    // auto val1 =
    //     std::tuple_size_v<std::ranges::range_value_t<std::views::all_t<std::vector<Point>&>>>;
    // 
    // auto val2 =
    //     std::_Tuple_like<std::ranges::range_value_t<std::views::all_t<std::vector<Point>&>>>;
    // 
    // auto val3 = std::ranges::_Has_tuple_element<
    //     std::ranges::range_value_t<std::views::all_t<std::vector<Point>&>>, 0>;

    // https://en.cppreference.com/w/cpp/ranges/elements_view
    // elements_view : tuple 스타일 자료형에서 특정 위치에 있는 요소를 조회하기 위한 view
    { 
        std::ranges::elements_view<std::views::all_t<std::vector<Point>&>, 0> elemIdx0_view{ points };

        // NOT WORKING
        Print(elemIdx0_view, "Idx 0");

        // NOT WORKING
        for (auto elem : elemIdx0_view)
        {
            std::cout << elem << ' ';
        }
    }
}
 */

int main()
{
    Case01::Run();
    Case02::Run();

    return 0;
}
