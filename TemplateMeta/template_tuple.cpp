#include <iostream>
#include <tuple>
#include <string>

// 동일한 내용이 _Study/Template/template_implementation_of_tuple.cpp에 있음.
// 이건 북마크 용도임.

// !! 컴파일러에 따라 C++11을 사용할 때 MyTuple의 함수 GetHead()와 GetRest()에 붙은 constexpr 때문에 에러가 날 수 있음. !!
// !! 이 경우 해당 멤버 함수 앞에 붙은 constexpr을 제거하면 되긴 함(온라인 컴파일러로 확인함). !!

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

    // Internal Use
    // C++20에 consteval이 있긴 한데 이것도 함수 오버로딩을 지원하지는 않음(키워드일 뿐).
    // 따라서 내부 사용 용도로 별도의 함수를 만들어야 함.
    constexpr static int32_t GetConstexprSize() noexcept
    {
        return ContainerSize<T, Rest...>::kSize;
    }

    static int32_t GetSize() noexcept
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
//     // 탈출 조건 2에 의해서 자식 클래스로부터 상속받은 재정의
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
    // 탈출 조건 2에 의해서 자식 클래스로부터 상속받은 재정의
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
// 6. MyIndexSequence<0, 1, 2, 3>
//
// 상속 구조
// MyMakeIndexSequence<4> 
//  ← MyMakeIndexSequence<3, 3> 
//   ← MyMakeIndexSequence<2, 2, 3>
//    ← MyMakeIndexSequence<1, 1, 2, 3> 
//     ← MyMakeIndexSequence<0, 0, 1, 2, 3>
//      ← MyIndexSequence<0, 1, 2, 3>
//
// MyMakeIndexSequence<4>은 MyIndexSequence<0, 1, 2, 3>를 상속 받아서 구현된 형태이기에
// 헬퍼 함수에 MyMakeIndexSequence<4>를 MyIndexSequence<...> 형태로 넘기면
// 가변 인자 <0, 1, 2, 3>을 인식할 수 있는 것이다.
//
// apply()는 이런 가변 인자를 푸는 방식으로 동작하는 함수이다.

#pragma region C++14 non-forward
// // decltype(auto)를 반환형으로 쓰는 건 C++14부터 지원함(decltype은 생략해도 됨).
// template <typename Callable, typename Tuple, int32_t... Indices>
// constexpr decltype(auto) MyApplyImpl(Callable callable, Tuple tuple, MyIndexSequence<Indices...> /* seq */)
// {
//     // https://en.cppreference.com/w/cpp/language/parameter_pack
//     // Indices는 매개변수 이름이 아니라 그냥 템플릿 매개변수 타입이 묶인 가변 타입이다.
// 
//     // MyIndexSequence<0, 1, 2, 3>일 경우 Get<N> 구문은 이렇게 언팩됨.
//     // MyGet<0>(tuple), MyGet<1>(tuple), MyGet<2>(tuple), MyGet<3>(tuple)
// 
//     return callable(MyGet<Indices>(tuple)...);
// }
// 
// // decltype(auto)를 반환형으로 쓰는 건 C++14부터 지원함(decltype은 생략해도 됨)..
// template <typename Callable, typename... Args>
// constexpr decltype(auto) MyApply(Callable callable, MyTuple<Args...> tuple)
// {
//     return MyApplyImpl(callable, tuple, MyMakeIndexSequence<sizeof...(Args)>{ });
// }
#pragma endregion

#pragma region C++14 forward
// // decltype(auto)를 반환형으로 쓰는 건 C++14부터 지원함(decltype은 생략해도 됨).
// template <typename Callable, typename Tuple, int32_t... Indices>
// constexpr decltype(auto) MyApplyImpl(Callable&& callable, Tuple&& tuple, MyIndexSequence<Indices...> /* seq */)
// {
//     // https://en.cppreference.com/w/cpp/language/parameter_pack
//     // Indices는 매개변수 이름이 아니라 그냥 템플릿 매개변수 타입이 묶인 가변 타입이다.
// 
//     // MyIndexSequence<0, 1, 2, 3>일 경우 Get<N> 구문은 이렇게 언팩됨.
//     // MyGet<0>(tuple), MyGet<1>(tuple), MyGet<2>(tuple), MyGet<3>(tuple)
// 
//     // MyGet<N>()은 템플릿 함수이다(인자 아님).
//     // std::forward(MyGet<Indices>(tuple)); // 못 씀.
//     return std::forward<Callable>(callable)(MyGet<Indices>(std::forward<Tuple>(tuple))...);
// }
// 
// // decltype(auto)를 반환형으로 쓰는 건 C++14부터 지원함(decltype은 생략해도 됨)..
// // template <typename Callable, typename... Args>
// // constexpr decltype(auto) MyApply(Callable&& callable, MyTuple<Args...>&& tuple) // MyTuple<Args...>&&에서 문제가 생김(&만 쓰면 인식하는데 참조를 인식해도 보편 참조는 인식하지 못 하는 듯)
// template <typename Callable, typename Tuple>
// constexpr decltype(auto) MyApply(Callable&& callable, Tuple&& tuple)
// {
//     // 보편 참조 인식 문제(?)
//     // 가변 팩 전개 문제(?)
//     // return MyApplyImpl(std::forward<Callable>(callable),
//     //                    std::forward<MyTuple<Args...>(tuple),
//     //                    MyMakeIndexSequence<sizeof...(Args)>{ });
// 
//     return MyApplyImpl(std::forward<Callable>(callable),
//                        std::forward<Tuple>(tuple),
//                        MyMakeIndexSequence<std::remove_reference_t<Tuple>::GetConstexprSize()>{ });
// }
#pragma endregion

#pragma region C++11 non-forward
// // C++11 호환 버전(반환 타입 명시)
// template <typename Callable, typename Tuple, int32_t... Indices>
// constexpr auto MyApplyImpl(Callable callable, Tuple tuple, MyIndexSequence<Indices...> /* seq */)
//     -> decltype(callable(MyGet<Indices>(tuple)...))
// {
//     // https://en.cppreference.com/w/cpp/language/parameter_pack
//     // Indices는 매개변수 이름이 아니라 그냥 템플릿 매개변수 타입이 묶인 가변 타입이다.
// 
//     // MyIndexSequence<0, 1, 2, 3>일 경우 Get<N> 구문은 이렇게 언팩됨.
//     // MyGet<0>(tuple), MyGet<1>(tuple), MyGet<2>(tuple), MyGet<3>(tuple)
// 
//     return callable(MyGet<Indices>(tuple)...);
// }
// 
// // C++11 호환 버전(반환 타입 명시)
// template <typename Callable, typename... Args>
// constexpr auto MyApply(Callable callable, MyTuple<Args...> tuple)
//     -> decltype(MyApplyImpl(callable, tuple, MyMakeIndexSequence<sizeof...(Args)>{ }))
// {
//     return MyApplyImpl(callable, tuple, MyMakeIndexSequence<sizeof...(Args)>{ });
// }
#pragma endregion

#pragma region C++11 forward
// C++11 호환 버전(반환 타입 명시)
template <typename Callable, typename Tuple, int32_t... Indices>
auto MyApplyImpl(Callable&& callable, Tuple&& tuple, MyIndexSequence<Indices...> /* seq */)
    -> decltype(std::forward<Callable>(callable)(MyGet<Indices>(std::forward<Tuple>(tuple))...))
{
    // https://en.cppreference.com/w/cpp/language/parameter_pack
    // Indices는 매개변수 이름이 아니라 그냥 템플릿 매개변수 타입이 묶인 가변 타입이다.

    // MyIndexSequence<0, 1, 2, 3>일 경우 Get<N> 구문은 이렇게 언팩됨.
    // MyGet<0>(tuple), MyGet<1>(tuple), MyGet<2>(tuple), MyGet<3>(tuple)

    // MyGet<N>()은 템플릿 함수이다(인자 아님).
    // std::forward(MyGet<Indices>(tuple)); // 못 씀.
    return std::forward<Callable>(callable)(MyGet<Indices>(std::forward<Tuple>(tuple))...);
}

// C++11 호환 버전(반환 타입 명시)
// template <typename Callable, typename... Args>
// auto MyApply(Callable&& callable, MyTuple<Args...>&& tuple) // MyTuple<Args...>&&에서 문제가 생김(&만 쓰면 인식하는데 참조를 인식해도 보편 참조는 인식하지 못 하는 듯)
template <typename Callable, typename Tuple>
constexpr auto MyApply(Callable&& callable, Tuple&& tuple)
    -> decltype(MyApplyImpl(std::forward<Callable>(callable),
                            std::forward<Tuple>(tuple),
                            MyMakeIndexSequence<std::remove_reference<Tuple>::type::GetConstexprSize()>{ }))
{
    // 보편 참조 인식 문제(?)
    // 가변 팩 전개 문제(?)
    // return MyApplyImpl(std::forward<Callable>(callable),
    //                    std::forward<MyTuple<Args...>(tuple),
    //                    MyMakeIndexSequence<sizeof...(Args)>{ });

    // remove_reference_t는 C++14부터 지원하는 헬퍼 기능임.
    // 따라서 std::remove_reference<T>::type으로 접근해야 함.
    return MyApplyImpl(std::forward<Callable>(callable),
                       std::forward<Tuple>(tuple),
                       MyMakeIndexSequence<std::remove_reference<Tuple>::type::GetConstexprSize()>{ });
}
#pragma endregion

// TEMP
int Print(char ch, int i, float f)
{
    std::cout << "Function | ch : " << ch << ", i : " << i << ", f : " << f << '\n';

    return 9999;
}

struct Functor
{
    std::string operator()(char ch, int i, float f)
    {
        std::cout << "Functor | ch : " << ch << ", i : " << i << ", f : " << f << '\n';

        return "Hello World";
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
    // auto retFunction = MyApplyImpl(Print, tp, MyMakeIndexSequence<3>{ });
    auto retFunction = MyApply(Print, tp);

    std::cout << "Ret : " << retFunction << '\n';
    
    MyGet<0>(tp) = 'B';
    
    std::string retFunctor = MyApply(Functor{ }, tp);
    
    std::cout << "Ret : " << retFunctor << '\n';
    
    MyGet<0>(tp) = 'C';
    
    /* int retLambda =*/ MyApply([](char ch, int i, float f) // -> int 
    {
        std::cout << "Lambda | ch : " << ch << ", i : " << i << ", f : " << f << '\n';
    
        // return 7777;
    }, tp);
    
    // std::cout << "Ret : " << retLambda << '\n';

    // TEMP : std::apply() with std::tuple
    // {
    //     std::tuple<char, int, float> tp('A', 10, 3.14f);
    // 
    //     int retFunction = std::apply(Print, tp);
    //     
    //     std::cout << "Ret : " << retFunction << '\n';
    //     
    //     std::get<0>(tp) = 'B';
    //     
    //     int retFunctor = std::apply(Functor<int>{ }, tp);
    //     
    //     std::cout << "Ret : " << retFunctor << '\n';
    //     
    //     std::get<0>(tp) = 'C';
    //     
    //     int retLambda = std::apply([](char ch, int i, float f) -> int
    //     {
    //         std::cout << "Lambda | ch : " << ch << ", i : " << i << ", f : " << f << '\n';
    //     
    //         return 7777;
    //     }, tp);
    //     
    //     std::cout << "Ret : " << retLambda << '\n';
    // }

    return 0;
}
