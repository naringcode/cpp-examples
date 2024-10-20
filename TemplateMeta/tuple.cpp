#include <iostream>
// #include <tuple>

// 동일한 내용이 _Study/enable_shared_from_this_with_refCnt.cpp에 있음.
// 이건 북마크 용도임.

// !! 컴파일러에 따라 C++11을 사용할 때 MyTuple에 있는 constexpr 반환 함수에서 에러가 날 수 있음. !!
// !! 이 경우 멤버 함수 앞에 붙은 constexpr을 제거하면 되긴 함(온라인 컴파일러로 확인함). !!

/*************************
*      Size(Length)      *
*************************/

template <typename... Args>
struct ContainerSize;

// 재귀 탈출 조건
template <>
struct ContainerSize<>
{
    enum : int32_t { kSize = 0 };
};

// 재귀
template <typename T, typename... Rest>
struct ContainerSize<T, Rest...>
{
    enum : int32_t { kSize = 1 + ContainerSize<Rest...>::kSize };
};

/******************
*      Tuple      *
******************/

template <typename... Args>
struct MyTuple;

// 재귀 탈출 조건
template <>
struct MyTuple<>
{
    // enum : int32_t { kSize = 0 };
};

// 재귀
template <typename T, typename... Rest>
struct MyTuple<T, Rest...> : MyTuple<Rest...>
{
public:
    using HeadType = T;
    using BaseType = MyTuple<Rest...>;

public:
    MyTuple(HeadType&& headValue, Rest&&... rest)
        : _headValue{ std::forward<HeadType>(headValue) }, BaseType{ std::forward<Rest>(rest)... }
    { }

public:
    constexpr HeadType& GetHead() noexcept
    {
        return _headValue;
    }

    // 자식 클래스로 캐스팅한 다음 GetHead()를 쓰면 자식의 요소를 사용할 수 있음.
    constexpr BaseType& GetRest() noexcept
    {
        return *this;
    }

    // constexpr int32_t GetSize() noexcept
    // {
    //     return kSize;
    // }

    constexpr int32_t GetSize() noexcept
    {
        return ContainerSize<T, Rest...>::kSize;
    }

private:
    HeadType _headValue;

    // enum : int32_t { kSize = 1 + BaseType::kSize };
};

/************************************************************
*      Get(어떤 Tuple인지 캐스팅해서 멤버를 파악해야 함)      *
************************************************************/

template <int32_t Index, typename... Args>
struct MyTupleElement;

template <int32_t Index, typename Tuple>
using MyTupleElementHead = typename MyTupleElement<Index, Tuple>::HeadType;

// 재귀 탈출 조건 1(에러)
// template <int32_t Index>
// struct MyTupleElement<Index, MyTuple<>>
// {
//     using HeadType = int32_t;
// 
//     // 더 이상 내려갈 수 없는데 인덱스 값은 남은 상황
//     static_assert(false, "Tuple Out Of Bounds");
// };

// 재귀 탈출 조건 2
template <typename T, typename... Rest>
struct MyTupleElement<0, MyTuple<T, Rest...>>
{
    using HeadType = typename MyTuple<T, Rest...>::HeadType;

    // 이 조건을 만족하면 재귀 정의에서 이 타입이 정의된 MyTupleElement를 상속 받게 됨.
    using ThisType = MyTuple<T, Rest...>;
};

// 재귀 정의
template <int32_t Index, typename T, typename... Rest>
struct MyTupleElement<Index, MyTuple<T, Rest...>>
    : MyTupleElement<Index - 1, MyTuple<Rest...>>
{ };

// Get()
// C++20 이전 버전에서는 안 됨(반환 템플릿의 종속 타입을 취급하지 않는 것 같음).
// template <int32_t Index, typename... Args>
// constexpr MyTupleElement<Index, MyTuple<Args...>>::HeadType& MyGet(MyTuple<Args...>& tuple) noexcept
// {
//     // 탈출 조건 2에 의해사 자식 클래스로부터 상속받은 재정의
//     using ThisType = MyTupleElement<Index, MyTuple<Args...>>::ThisType;
// 
//     // 자식 클래스로 캐스팅하여 값을 가져옴.
//     return ((ThisType&)(tuple)).GetHead();
// }

// Get()
// C++20 이전 버전과의 호환 목적으로 만든 것.
template <int32_t Index, typename... Args>
constexpr MyTupleElementHead<Index, MyTuple<Args...>>& MyGet(MyTuple<Args...>& tuple) noexcept
{
    // 탈출 조건 2에 의해사 자식 클래스로부터 상속받은 재정의
    // C++20 이전 버전에서 종속 타입 앞에는 typename을 붙여줘야 함.
    using ThisType = typename MyTupleElement<Index, MyTuple<Args...>>::ThisType;

    // 자식 클래스로 캐스팅하여 값을 가져옴.
    return ((ThisType&)(tuple)).GetHead();
}

/*******************************************
*      Apply(C++17에 있는 기능을 모방)      *
*******************************************/

// 템플릿 인자를 인덱스로 사용함.
// (중요) MyIndexSequence<>는 재귀적으로 누적하는 것이 아닌 MyMakeIndexSequence<>에 의해서 한 번에 전달됨.
// ex) MyIndexSequence<0, 1, 2> : 0, 1, 2라는 템플릿 인자를 담고 있는 구조체
template <int32_t... Indices>
struct MyIndexSequence
{
    enum : int32_t { kNumIndices = sizeof...(Indices) };
};

// 재귀적으로 인자를 쌓아가며 인덱스를 생성함.
// 첫 번째 "Index - 1"은 재귀 탈출 체크 용도, 두 번째 "Index - 1"은 인자를 쌓는 용도임.
template <int32_t Index, int32_t... RestIndices>
struct MyMakeIndexSequence : MyMakeIndexSequence<Index - 1, Index - 1, RestIndices...>
{ };

// 인덱스가 0에 도달했을 때 누적한 인자 목록를 MyIndexSequence<>에 전달함.
template <int32_t... Indices>
struct MyMakeIndexSequence<0, Indices...> : MyIndexSequence<Indices...>
{ };

// 컴파일러는 이런 식으로 전개한다.
// 1. MyMakeIndexSequence<4>
// 2. MyMakeIndexSequence<3, 3>
// 3. MyMakeIndexSequence<2, 2, 3>
// 4. MyMakeIndexSequence<1, 1, 2, 3>
// 5. MyMakeIndexSequence<0, 0, 1, 2, 3>
// 5. MyIndexSequence<0, 1, 2, 3>
//
// 상속 구조
// MyMakeIndexSequence<4> 
//  ← MyMakeIndexSequence<3, 3> 
//   ← MyMakeIndexSequence<2, 2, 3>
//    ← MyMakeIndexSequence<1, 1, 2, 3> 
//     ← MyMakeIndexSequence<0, 0, 1, 2, 3>
//      ← MyIndexSequence<0, 1, 2, 3>

// // decltype(auto)를 반환형으로 쓰는 건 C++14부터 지원함(decltype은 생략해도 됨).
// template <typename Callable, typename Tuple, int32_t... Indices>
// decltype(auto) MyApplyImpl(Callable callable, Tuple tuple, MyIndexSequence<Indices...> /* seq */)
// {
//     // https://en.cppreference.com/w/cpp/language/parameter_pack
//     // Indices는 매개변수 이름이 아니라 그냥 템플릿 매개변수 타입이 묶인 가변 타입이다.
// 
//     // MyIndexSequence<0, 1, 2, 3>일 경우 Get<N> 구문은 이렇게 언팩됨.
//     // MyGet<0>(tuple), MyGet<1>(tuple), MyGet<2>(tuple), MyGet<3>(tuple)
// 
//     // 편의상 forward() 생략
//     return callable(MyGet<Indices>(tuple)...);
// }
// 
// // decltype(auto)를 반환형으로 쓰는 건 C++14부터 지원함(decltype은 생략해도 됨)..
// template <typename Callable, typename... Args>
// decltype(auto) MyApply(Callable callable, MyTuple<Args...> tup)
// {
//     // 편의상 forward() 생략
//     return MyApplyImpl(callable, tup, MyMakeIndexSequence<sizeof...(Args)>{ });
// }

// C++11 호환 버전(반환 타입 명시)
template <typename Callable, typename Tuple, int32_t... Indices>
auto MyApplyImpl(Callable callable, Tuple tuple, MyIndexSequence<Indices...> /* seq */) 
    -> decltype(callable(MyGet<Indices>(tuple)...))
{
    // https://en.cppreference.com/w/cpp/language/parameter_pack
    // Indices는 매개변수 이름이 아니라 그냥 템플릿 매개변수 타입이 묶인 가변 타입이다.

    // MyIndexSequence<0, 1, 2, 3>일 경우 Get<N> 구문은 이렇게 언팩됨.
    // MyGet<0>(tuple), MyGet<1>(tuple), MyGet<2>(tuple), MyGet<3>(tuple)

    // 편의상 forward() 생략
    return callable(MyGet<Indices>(tuple)...);
}

// C++11 호환 버전(반환 타입 명시)
template <typename Callable, typename... Args>
auto MyApply(Callable callable, MyTuple<Args...> tup)
    -> decltype(MyApplyImpl(callable, tup, MyMakeIndexSequence<sizeof...(Args)>{ }))
{
    // 편의상 forward() 생략
    return MyApplyImpl(callable, tup, MyMakeIndexSequence<sizeof...(Args)>{ });
}

// TEMP
int Print(char ch, int i, float f)
{
    std::cout << "Function | ch : " << ch << ", i : " << i << ", f : " << f << '\n';

    return 9999;
}

template <typename Ret>
struct Functor
{
    Ret operator()(char ch, int i, float f)
    {
        std::cout << "Functor | ch : " << ch << ", i : " << i << ", f : " << f << '\n';

        return 8888;
    }
};

int main()
{
    // auto ttp = std::make_tuple(10, 20, 30);
    // // std::get<2>(ttp)::;
    // std::tuple_element<0, std::tuple<int, int, int, int>>::_Ttype;

    MyTuple<char, int, float> tp('A', 10, 3.14f);
    
    // 템플릿이 구현된 형태로 받아오기
    auto valNaive0 = tp.GetHead();
    auto valNaive1 = tp.GetRest().GetHead();
    auto valNaive2 = tp.GetRest().GetRest().GetHead();
    // auto valNaive3 = tp.GetRest().GetRest().GetRest().GetHead();
    
    std::cout << valNaive0 << ' ' << valNaive1 << ' ' << valNaive2 << '\n';
    
    // std::get() 모방
    auto valGet0 = MyGet<0>(tp);
    auto valGet1 = MyGet<1>(tp);
    auto valGet2 = MyGet<2>(tp);
    // auto valGet3 = MyGet<3>(tp);
    
    std::cout << valGet0 << ' ' << valGet1 << ' ' << valGet2 << '\n';
    
    // 크기 구하기
    int32_t size = tp.GetSize();
    
    std::cout << size << '\n';

    auto seq = MyMakeIndexSequence<10>();

    std::cout << seq.kNumIndices << '\n';

    // std::apply() 모방
    int retFunction = MyApply(Print, tp);

    std::cout << "Ret : " << retFunction << '\n';

    MyGet<0>(tp) = 'B';

    int retFunctor = MyApply(Functor<int>{ }, tp);

    std::cout << "Ret : " << retFunctor << '\n';

    MyGet<0>(tp) = 'C';

    int retLambda = MyApply([](char ch, int i, float f) -> int 
    {
        std::cout << "Lambda | ch : " << ch << ", i : " << i << ", f : " << f << '\n';

        return 7777;
    }, tp);

    std::cout << "Ret : " << retLambda << '\n';

    return 0;
}
