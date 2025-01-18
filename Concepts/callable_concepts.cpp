// Update Date : 2025-01-19
// OS : Windows 10 64bit
// Program : Visual Studio 2022, Visual Studio 2019, https://godbolt.org/ + gcc-14.2 with the -std=c++20 option
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <algorithm>
#include <concepts>
#include <iterator>
#include <vector>
#include <list>
#include <forward_list>
#include <set>
#include <map>

using namespace std;

// 순서대로 볼 것
// 
// # concepts에 대한 기본적인 설명과 사용 방법
// 1. requires_clauses_need_bool_expr.cpp
// 2. built-in_concepts.cpp
// 3. abbreviated_function_templates.cpp
// 4. custom_concepts.cpp
// 5. combining_constraints.cpp
// 6. callable_concepts.cpp <-----
// 
// # 위 항목을 한데 묶어서 정리한 내용
// 7. concepts_details.cpp
//
// Concepts을 학습한 다음 Ranges를 학습하도록 한다.
// Ranges의 동작 방식을 잘 이해하기 위해선 Concepts에 대한 선행 학습이 필요하다.
//

// https://en.cppreference.com/w/cpp/concepts#Callable_concepts
// https://en.cppreference.com/w/cpp/concepts#Equality_preservation
// https://en.cppreference.com/w/cpp/iterator#Iterator_concepts
// https://en.cppreference.com/w/cpp/iterator#Algorithm_concepts_and_utilities
// https://ko.wikipedia.org/wiki/%EC%9D%B4%ED%95%AD_%EA%B4%80%EA%B3%84 | 이항 관계(binary relation)
// https://ko.wikipedia.org/wiki/%EB%8F%99%EC%B9%98_%EA%B4%80%EA%B3%84 | 동치 관계(equivalence relation)

// C++20에 도입된 concepts을 보면 built-in concepts 유형에는 Callable Concepts라는 것이 있다.
// 해당 유형의 concepts는 호출 가능 여부를 조회할 때 사용하면 좋다.

// https://en.cppreference.com/w/cpp/concepts/invocable
// # std::invocable & std::regular_invocable
// 
// template< class F, class... Args >
// concept invocable =
//     requires(F&& f, Args&&... args) {
//         std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
//             /* not required to be equality-preserving */
//     };
// 
// std::invocable은 callable 타입인 F가 Args...에 담긴 인자를 토대로 std::invoke()를 호출할 수 있는지 판단하기 위한 concept이다.
// 말 그대로 안정성은 고려하지 않고 호출될 수만 있다면 통과시키기 위한 목적으로 사용하는 concept이다.
// 
// --------------------------------------------------
// 
// template< class F, class... Args >
// concept regular_invocable = std::invocable<F, Args...>;
// 
// https://en.cppreference.com/w/cpp/concepts#Equality_preservation
// std::regular_invocable는 std::invocable을 그대로 받아 기능적으로는 동일하지만 Equality preservation의 의미론적(semantics)인 구분을 위해 따로 정의한 것이다.
// 
// Equality preservation(동등성 보존) : 동일한 입력에 대해 출력되는 결과가 항상 같아야 하고 어떠한 side-effects도 발생시키지 않아야 하며 연산이 동등성 관계를 깨지 않고 보존해야 함.
// 
// std::regular_invocable가 적용되면 전달된 callable과 인자의 원본이 수정되어선 안 된다.
// 원문 : The regular_invocable concept adds to the invocable concept by requiring the invoke expression to be equality-preserving and not modify either the function object or the arguments.
// 원문 : Note that regular_invocable requires the invocation to not modify either the callable object or the arguments and be equality-preserving.
// 
// 쉽게 생각해서 std::regular_invocable이라는 제약이 걸리면 동일 인수로 여러 번 호출해도 결과는 항상 같아야 한다.
// 내부적으로 인수를 대상으로 혹은 전역적으로 side-effects를 발생시켜 상태나 결과가 달라지면 Equality preservation의 성질을 깨지니 주의해야 한다.
// 원문 : A random number generator may satisfy invocable but cannot satisfy regular_invocable
// 
// std::invocable과 std::regular_invocable의 의미론적(semantics)인 내용은 다르지만 실제 동작은 같다.
// 둘의 차이를 판단하는 건 프로그래머의 몫이며 컴파일러는 이를 검증하지 않는다(2025-01-17 기준이며 추후 바뀔 수 있음).
// C++20에 추가된 기능을 보면 std::invocable을 요구하는 경우도 있고 std::regular_invocable을 요구하는 경우도 있으니 적절하게 요구사항에 맞게 코드를 작성하는 것이 중요하다.
// 

// https://en.cppreference.com/w/cpp/concepts/predicate
// # std::predicate
// 
// template< class F, class... Args >
// concept predicate =
//     std::regular_invocable<F, Args...> &&
//     boolean-testable<std::invoke_result_t<F, Args...>>;
// 
// std::predicate는 Equality preservation(동등성 보존)을 만족하면서 callable이 반환형이 boolean-testable을 만족하는지 확인하기 위한 concept이다.
// 
// --------------------------------------------------
// 
// https://en.cppreference.com/w/cpp/concepts/boolean-testable
// 
// template< class B >
// concept __boolean_testable_impl = std::convertible_to<B, bool>;
// 
// template< class B >
// concept boolean-testable =
//     __boolean_testable_impl<B> &&
//     requires (B&& b) {
//         { !std::forward<B>(b) } -> __boolean_testable_impl;
//     };
// 
// boolean-testable은 반환되는 값이 참(true)와 거짓(false)으로 사용될 수 있는지를 판단하기 위한 concept이다.
// 반환되는 값은 암시적으로 bool 타입으로의 변환이 가능해야 한다(반드시 반환되는 값이 bool 형일 필요는 없음).
// 
// 반환 값이 if 문이나 반복문 등에서 조건 판정을 위한 논리 연산에 사용될 수 있는지 검증하기 위한 concept이라고 보면 된다.
// 원문 : for which the logical operators have the usual behavior (including short-circuiting), even for two different boolean-testable types.
// 
// 또한 boolean-testable는 B가 decltype((e))인 식 e가 주어졌으면 bool(e) == !bool(!e)로 모델링된다.
// 원문 : Additionally, given an expression e such that decltype((e)) is B, boolean-testable is modeled only if bool(e) == !bool(!e).
// 
// https://en.wikipedia.org/wiki/Predicate_(mathematical_logic)
// https://en.wikipedia.org/wiki/Predication_(computer_architecture)
// https://stackoverflow.com/questions/3230944/what-does-predicate-mean-in-the-context-of-computer-science
// https://ko.wikipedia.org/wiki/%EC%88%A0%EC%96%B4
// https://www.ldoceonline.com/dictionary/predicate
// C++ 공식 사양을 정리한 문서를 보면 반환되는 값을 암묵적으로 bool 형식으로 변환 가능한 concept을 predicate라고 간주하고 있다.
// 하지만 컴퓨터 과학에서 말하는 predicate와 다른 프로그래밍에서 지원하는 predicate를 보면 C++의 predicate와는 약간 차이가 있다.
// 일반적으로 컴퓨터 쪽의 predicate는 "개는 동물이다"와 같이 true/false로 판정 가능한 결과를 반환하는 서브루틴을 말한다.
// 
// !! 정리 !!
// std::predicate를 bool이 아닌 암묵적으로 bool 형으로 변환 가능한 다른 타입을 반환하는 Callable에 적용하는 것도 가능하다.
// 하지만 의도 전달과 직관성을 위해서라도 std::predicate로 조건 대상이 되는 Callable은 특별한 이유가 없다면 bool 형을 반환하는 것이 좋다.
// 

// https://en.cppreference.com/w/cpp/concepts/relation
// # std::relation
// 
// template< class R, class T, class U >
// concept relation =
//     std::predicate<R, T, T> && std::predicate<R, U, U> &&
//     std::predicate<R, T, U> && std::predicate<R, U, T>;
// 
// https://ko.wikipedia.org/wiki/%EC%9D%B4%ED%95%AD_%EA%B4%80%EA%B3%84 | 이항 관계(binary relation)
// std::relation은 주어진 T, U를 대상으로 callable R을 적용했을 때 이항 관계를 만족하는지 확인하기 위한 concept이다.
// 
// 이항 관계가 되기 위한 중요한 4가지 요소는 다음과 같다.
// - 반사관계(reflexive relation) : 모든 x ∈ A에 대하여, x ~ x인 관계.
// - 대칭관계(symmetric relation) : 모든 x, y ∈ A에 대하여, x ~ y이면 y ~ x인 관계.
// - 추이관계(transitive relation) : 모든 x, y, z ∈ A에 대하여, x ~ y이고 y ~ z이면 x ~ z인 관계.
// - 반대칭관계(antisymmetric relation) : 모든 x, y ∈ A에 대하여, x ~ y이고 y ~ x이면 x = y인 관계.
//

// https://en.cppreference.com/w/cpp/concepts/equivalence_relation
// # std::equivalence_relation
// 
// template< class R, class T, class U >
// concept equivalence_relation = std::relation<R, T, U>;
// 
// https://ko.wikipedia.org/wiki/%EB%8F%99%EC%B9%98_%EA%B4%80%EA%B3%84 | 동치 관계(equivalence relation)
// std::equivalence_relation은 이름 그대로 동치 관계를 파악하기 위한 concept이다.
// 
// std::invocable과 std::regular_invocable의 관계와 유사하게 std::relation과 std::equivalence_relation의 의미론적인 내용은 다르지만 실제 동작은 같다.
// 마찬가지로 이를 구분하는 건 프로그래머의 역할이다.
// 
// 동치 관계는 이항 관계로 주어진 R이 반사적, 대칭적, 추이적이라는 논리적 관계를 만족시킬 때 성립하는 관계를 말한다.
// 두 객체가 완전히 같은 건 아니지만 관점(여기서는 R)에 따라 성질이 동일하다면 이는 동치 관계라고 볼 수 있다.
// 

// https://en.cppreference.com/w/cpp/concepts/strict_weak_order
// # std::strict_weak_order
// 
// template< class R, class T, class U >
// concept strict_weak_order = std::relation<R, T, U>;
// 
// std::relation과 std::strict_weak_order의 실제 동작은 같지만 마찬가지로 의미론적인 내용이 다르다.
// 
// A relation r is a strict weak ordering if
// - it is irreflexive: for all x, r(x, x) is false; <----- 비반사성
// - it is transitive: for all a, b and c, if r(a, b) and r(b, c) are both true then r(a, c) is true; <----- 추이성
// - let e(a, b) be !r(a, b) && !r(b, a), then e is transitive: e(a, b) && e(b, c) implies e(a, c).
//   - let e(a, b) be !r(a, b) && !r(b, a) <----- e(a, b)에서 a와 b를 r에 적용했을 때 동치 관계로 표현될 경우
//   - then e is transitive: e(a, b) && e(b, c) implies e(a, c). <----- 그럼 추이성이 따라 e(a, b) && e(b, c)는 e(a, c)로 표현 가능함.
// 
// 이러한 조건 하에 e는 동치 관계라고 볼 수 있으며, r은 e에 의해 결정된 동등 클래스에 대해 strict total ordering을 유도한다.
// 원문 : Under these conditions, it can be shown that e is an equivalence relation, and r induces a strict total ordering on the equivalence classes determined by e.
//

// (정리) C++20은 다양한 Callable concepts를 제공하지만 실제로 개발할 때는 std::invocable, std::regular_invocable, std::predicate 정도면 충분하다.
// - std::invocable : 호출 가능성만 판단하고 싶을 때
// - std::regular_invocable : 동일 입력에 대해 동일 출력을 보장하는 호출임을 명시하고 싶을 때
// - std::predicate : callable이 반환하는 값이 bool로 변환한 것임을 명시하고 싶을 때
// 
// std::relation, std::equivalence_relation, std::strict_weak_order는 구체적인 조건으로 인해 범용성이 제한되어 있기 때문에
// C++ 차원에서 관리하는 라이브러리라면 몰라도 일반적인 개발 상황에서 사용할 일은 드물다고 봐야 한다.

vector<int> MakeVectorFilledWithRandomNumbers(int cnt)
{
    vector<int> retVec(cnt);

    for (int i = 0; i < cnt; i++)
    {
        retVec[i] = rand() % 100;
    }
    
    return retVec;
}

template <typename... Args>
    requires (std::integral<Args> || ...) || (std::floating_point<Args> || ...)
auto Sum(Args... args)
{
    return (args + ...);
}

template <typename Callable, typename... Args>
    requires std::invocable<Callable, Args...>
auto InvokeWrapper(Callable&& callable, Args&&... args)
{
    return std::invoke(std::forward<Callable>(callable), std::forward<Args>(args)...);
}

template <typename Callable, typename... Args>
    requires std::regular_invocable<Callable, Args...>
auto RegularInvokeWrapper(Callable&& callable, Args&&... args)
{
    return std::invoke(std::forward<Callable>(callable), std::forward<Args>(args)...);
}

// 아래 출력 함수에서 사용하기 위한 concept
template <typename Container>
concept HasPairIter = 
    std::same_as<std::remove_cvref_t<std::iter_value_t<Container>>,
                 std::pair<typename std::iter_value_t<Container>::first_type,
                           typename std::iter_value_t<Container>::second_type>>;

// 가급적이면 아래 코드로 정의한 concept 말고 위 방식으로 정의한 concept를 쓰도록 하자.
// 기능적으로는 비슷할지 몰라도 위 방식으로 정의한 concept이 더 안정적으로 std::pair가 있는지 검증한다.
// !! 기능적으로 보면 동일한 것 같은데 아래 방식으로 concept을 사용하면 에러 위치가 깔끔하게 잡히지 않음. !!
// template <typename Container>
// concept HasPairIter = 
//     std::same_as<std::remove_cvref_t<typename Container::iterator::value_type>,
//                  std::pair<typename Container::iterator::value_type::first_type,
//                            typename Container::iterator::value_type::second_type>>;

// https://en.cppreference.com/w/cpp/iterator/input_iterator
// std::input_iterator는 iterator concepts 중 하나로 사용하고자 하는 iterator는 다음 조건을 충족해야 한다(자세한 건 공식 문서에 있는 예제를 참고).
// - operator*() : 읽을 수 있어야 함.
// - operator++(), operator++(int) : 단방향 접근이 가능해야 함.
//
// C++20에 정의된 iterator concepts는 C++20에 추가된 Ranges와 연관성이 매우 높다.
template <typename Container>
    requires std::input_iterator<typename Container::iterator> && (!HasPairIter<typename Container::iterator>) 
    // !HasPairIter는 괄호를 붙여서 완성된 primary expression으로 표현해야 함.(https://en.cppreference.com/w/cpp/language/constraints#Requires_clauses).
    // C++ 표준에서 괄호가 있는 표현식은 primary expression으로 간주함.
void PrintContainerElems(const Container& container)
{
    cout << "Elems : ";

    for (auto& elem : container)
    {
        cout << elem << ' ';
    }

    cout << "\n";
}

// 가변 인자 템플릿의 각 단일 요소를 평상으로 Callable을 호출하는 것의 유효성을 컴파일 타임에 검증하는 방법
// AllOf()와 OneOf()에사 평가하기 위해 사용되는 Callable은 bool 타입을 반환하는 것이 좋기 때문에 std::predicate로 제약을 걸었다.
template <typename Callable, typename... Args>
    requires (std::predicate<Callable, Args> && ...)
bool AllOf(Callable callable, Args... args)
{
    return (callable(args) && ...);
}

template <typename Callable, typename... Args>
    requires (std::predicate<Callable, Args> && ...)
bool OneOf(Callable callable, Args... args)
{
    return (callable(args) || ...);
}

bool IsEven(int x)
{
    return x % 2 == 0;
}

string IsOdd(int x)
{
    return x % 2 == 1 ? "TRUE" : "FALSE";
}

// Callable Concepts을 사용자 정의 concept에 묶어서 사용하는 방법
template <typename Pred, typename InputIterator>
concept InputIteratableWithPred =
    std::predicate<Pred, typename InputIterator::value_type> &&
    std::input_iterator<InputIterator>;

template <typename Pred, typename InputIterator>
    requires InputIteratableWithPred<Pred, InputIterator>
InputIterator FindIf(InputIterator beginIter, InputIterator endIter, Pred pred)
{
    while (beginIter != endIter)
    {
        if (pred(*beginIter))
            break;

        ++beginIter;
    }

    return beginIter;
}

int main()
{
    auto containerA = InvokeWrapper(MakeVectorFilledWithRandomNumbers, 5);
    PrintContainerElems(containerA);

    // 아래 코드도 제대로 동작하긴 하지만 문제는 std::regular_invocable 기반으로 작성된 템플릿 함수를 호출하고 있다.
    // MakeVectorFilledWithRandomNumbers()는 동일한 입력이 들어왔어도 출력되는 결과가 같아서 Equality preservation(동등성 보존)을 깬다.
    // std::invocable과 std::regular_invocable는 그냥 의미론적인 내용일 뿐 실제 동작하는 방식은 동일하기 때문에 이는 프로그래머가 주의해야 하는 사항이다.
    auto containerB = RegularInvokeWrapper(MakeVectorFilledWithRandomNumbers, 5);
    PrintContainerElems(containerB);
    
    // 단방향으로 접근 가능하며 읽을 수 있는 iterator를 지원하기에 컴파일 성공
    PrintContainerElems(std::set{ 10, 20, 30 });
    PrintContainerElems(std::list{ 40, 50, 60 });
    PrintContainerElems(std::forward_list{ 40, 50, 60 });

    // 아래 방식은 !HasPairIter 제약을 만족하지 못 하기 때문에 에러가 발생한다.
    // PrintContainerElems(std::map<string, int>{ { "Hello", 1 }, { "World", 2 }, { "Hello World", 60 } });
    
    // TEMP : HasPairIter를 구성하는 과정에서 타입을 확인한 코드
    // using typeA = std::map<string, int>::iterator;
    // using typeB = std::iter_value_t<std::map<string, int>>;
    // using typeC = std::iter_value_t<std::map<string, int>::iterator>;
    // using typeD = std::iter_value_t<volatile std::map<string, int>::const_iterator>;
    // using typeE = std::iter_value_t<const std::map<string, const int>>;
    // using typeF = std::iter_value_t<std::map<string, int>::iterator>;
    // using typeG = std::remove_cvref_t<std::iter_value_t<std::map<string, const int>>>;
    // using typeH = std::remove_cvref_t<std::iter_value_t<std::map<string, int>::const_iterator>>;
    // using typeI = std::map<string, int>::iterator::value_type::first_type;
    // using typeJ = std::map<string, int>::iterator::value_type::second_type;
    // using TestContainer = std::map<string, int>;
    // bool typeChk = std::same_as<std::remove_cvref_t<std::iter_value_t<TestContainer>>,
    //                             std::pair<typename TestContainer::iterator::value_type::first_type,
    //                                       typename TestContainer::iterator::value_type::second_type>>;

    // 다음 코드는 컴파일러가 Sum의 인자를 추론할 수 있느냐 없느냐에 따라서 컴파일이 성공할 수도 있고 실패할 수도 있다.
    // cout << InvokeWrapper(Sum, 100, 200, 300) << "\n";

    // 언어가 중첩 템플릿 함수 추론을 지원한다고 명시되어 있지 않기 때문에
    // 중첩 템플릿 함수의 인자 타입을 하나하나 지정해줘야 제대로 컴파일 할 수 있다.
    cout << InvokeWrapper(Sum<int, int, int>, 100, 200, 300) << "\n";

    // Sum 함수 자체는 전달한 인자에 변형을 가하지도 않고 매번 호출할 때마다 같은 결과를 반환하기 때문에
    // std::regular_invocable로 제약을 건 템플릿 함수를 사용해도 괜찮다.
    cout << RegularInvokeWrapper(Sum<int, int, int>, 100, 200, 300) << "\n";

    // 다음 코드는 단일 인자를 받는 Callable을 통해 가변 인자 템플릿의 각 인자를 검증하기 위한 코드이다.
    cout << AllOf(IsEven, 10, 20, 30) << "\n";
    cout << AllOf([](int elem) { return elem % 2 == 0; }, 10, 20, 30, 15) << "\n";

    // 아래 코드는 IsOdd()가 string을 반환하는데 이는 bool 타입으로 암묵적인 변환이 불가능해서 컴파일 에러가 발생한다.
    // cout << OneOf(IsOdd, 10, 20, 30) << "\n";

    cout << OneOf([](int elem) { return elem % 2 == 1; }, 10, 20, 30) << "\n";

    // Functor 또한 Callable의 한 유형이기 때문에 
    struct OddFunctor
    {
        bool operator()(int elem)
        {
            return elem % 2 == 1;
        }
    };

    cout << OneOf(OddFunctor{ }, 10, 20, 30, 15) << "\n";

    //
    vector<string> strVec{ "Hello", "World", "Foo" };

    auto findIter = FindIf(strVec.begin(), strVec.end(), [ch = 'W'](string str) { return str[0] == ch; });

    cout << *findIter << "\n";

    return 0;
}
