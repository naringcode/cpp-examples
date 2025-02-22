// Update Date : 2025-02-12
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++23(experimental)
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <format>
#include <algorithm>
#include <ranges>
#include <string>
#include <array>
#include <vector>
#include <deque>
#include <forward_list>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>
// #include <flat_set> // C++23(Visual Studio 2022 기준 사용 불가)
// #include <flat_map> // C++23(Visual Studio 2022 기준 사용 불가)
#include <span>
#include <mdspan> // C++23

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
// 3. range_concepts_and_iterator_concepts.cpp <-----
// 4. legacy_algorithms_and_range_algorithms.cpp
// 5. return_types_by_range_algorithms.cpp
// 
// # Projections
// 6. implementation_of_invoke.cpp(사전지식)
// 7. range_algorithms_with_projections.cpp
// 8. implementation_of_projections.cpp
// 
// # Views
// 9. views_by_constructors.cpp
// 10. views_by_range_adaptors.cpp
// 11. algorithms_with_and_without_views.cpp
// 12. view_composition.cpp
// 13. lazy_evaluation_of_views.cpp
// 14. view_working_with_range_algorithms.cpp
// 
// # Range Factories
// 15. range_factories.cpp
//
// # 응용하기
// 16. Functional/bind_and_mem_fn.cpp(Case03)
// 17. Functional/bind_front_and_bind_back.cpp(Case03)
// 18. STL/span.cpp(Case04)
//

// https://en.cppreference.com/w/cpp/ranges#Range_concepts
// https://en.cppreference.com/w/cpp/iterator#Iterator_concepts
// https://learn.microsoft.com/ko-kr/cpp/standard-library/range-concepts?view=msvc-170
// https://learn.microsoft.com/en-us/cpp/standard-library/iterator-concepts?view=msvc-170
// https://en.cppreference.com/w/cpp/container
// https://en.cppreference.com/w/cpp/string/basic_string

// std::basic_string은 컨테이너가 아니지만 컨테이너와 유사하게 사용할 수 있다.

#define BEGIN_NS(name) namespace name {
#define END_NS };

#define FORMAT(...) std::format("    {1:<{0}} : {2:}\n", __VA_ARGS__)

BEGIN_NS(Case01)

// Ranges 라이브러리는 range라는 concept을 기반으로 동작한다.
//
// template <class T>
// concept range = requires( T& t ) { 
//     ranges::begin(t); // equality-preserving for forward iterators 
//     ranges::end(t); 
// }; 
// 
// Ranges는 쉽게 생각해서 std::ranges::begin()과 std::ranges::end()로 시작과 끝을 조회할 수 있는 컨테이너를 말한다.
//

void Range()
{
    // https://en.cppreference.com/w/cpp/ranges/range
    //
    // template <class T>
    // concept range = requires( T& t ) {
    //     ranges::begin(t); // equality-preserving for forward iterators
    //     ranges::end(t);
    // };
    //
    std::cout << "########## std::ranges::range<> ##########\n";

    // Basic types
    std::cout << "  ========== Basic Types ==========\n";

    int arr[] = { 1, 2, 3, 4, 5 };
    
    std::cout << FORMAT(14, "int",    std::ranges::range<int>);
    std::cout << FORMAT(14, "T[]",    std::ranges::range<decltype(arr)>);
    std::cout << FORMAT(14, "string", std::ranges::range<std::string>);

    // Sequence containers
    std::cout << "  ========== Sequence Containers ==========\n";

    std::cout << FORMAT(14, "array<T>",        std::ranges::range<std::array<int, 10>>);
    std::cout << FORMAT(14, "vector<T>",       std::ranges::range<std::vector<int>>);
    std::cout << FORMAT(14, "deque<T>",        std::ranges::range<std::deque<int>>);
    std::cout << FORMAT(14, "forward_list<T>", std::ranges::range<std::forward_list<int>>);
    std::cout << FORMAT(14, "list<T>",         std::ranges::range<std::list<int>>);

    // Associative containers
    std::cout << "  ========== Associative Containers ==========\n";

    std::cout << FORMAT(14, "set<T>",         std::ranges::range<std::set<int>>);
    std::cout << FORMAT(14, "map<K, V>",      std::ranges::range<std::map<int, int>>);
    std::cout << FORMAT(14, "multiset<T>",    std::ranges::range<std::multiset<int>>);
    std::cout << FORMAT(14, "multimap<K, V>", std::ranges::range<std::multimap<int, int>>);

    // Unordered Associative containers
    std::cout << "  ========== Unordered Associative Containers ==========\n";

    std::cout << FORMAT(14, "unordered_set<T>",         std::ranges::range<std::unordered_set<int>>);
    std::cout << FORMAT(14, "unordered_map<K, V>",      std::ranges::range<std::unordered_map<int, int>>);
    std::cout << FORMAT(14, "unordered_multiset<T>",    std::ranges::range<std::unordered_multiset<int>>);
    std::cout << FORMAT(14, "unordered_multimap<K, V>", std::ranges::range<std::unordered_multimap<int, int>>);

    // Container adaptors
    std::cout << "  ========== Container Adaptors ==========\n";

    std::cout << FORMAT(14, "stack<T>",          std::ranges::range<std::stack<int>>);
    std::cout << FORMAT(14, "queue<T>",          std::ranges::range<std::queue<int>>);
    std::cout << FORMAT(14, "priority_queue<T>", std::ranges::range<std::priority_queue<int>>);

    // Views(Ranges 라이브러리에서 다루는 Views와는 별개의 개념임)
    std::cout << "  ========== Views ==========\n";

    std::vector vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    auto mdspan = std::mdspan(vec.data(), 2, 6);

    std::cout << FORMAT(14, "span<T>",      std::ranges::range<std::span<int>>);
    std::cout << FORMAT(14, "mdspan<T, E>", std::ranges::range<decltype(mdspan)>);
}

void Run()
{
    Range();
}

END_NS

BEGIN_NS(Case02) 

// Ranges 라이브러리는 제약을 걸 때 내부적으로 iterator 제약을 사용한다.
// iterator 제약은 알고리즘의 특성과 컨테이너의 특성을 제한하기 위해 사용된다.

void InputOrOutputIterator()
{
    // https://en.cppreference.com/w/cpp/iterator/input_or_output_iterator
    //
    // template <class I>
    // concept input_or_output_iterator =
    //     requires(I i) {
    //         { *i } -> /*can-reference*/;
    //     } &&
    //     std::weakly_incrementable<I>;
    //
    std::cout << "########## std::input_or_output_iterator<> ##########\n";

    // Basic types
    std::cout << "  ========== Basic Types ==========\n";

    // int arr[] = { 1, 2, 3, 4, 5 };
    // 
    // std::cout << FORMAT(14, "int", std::input_or_output_iterator<int>);
    // std::cout << FORMAT(14, "T[]", std::input_or_output_iterator<decltype(arr)>);
    std::cout << FORMAT(24, "string::iterator", std::input_or_output_iterator<std::string::iterator>);

    // Sequence containers
    std::cout << "  ========== Sequence Containers ==========\n";

    std::cout << FORMAT(24, "array<T>::iterator",        std::input_or_output_iterator<std::array<int, 10>::iterator>);
    std::cout << FORMAT(24, "vector<T>::iterator",       std::input_or_output_iterator<std::vector<int>::iterator>);
    std::cout << FORMAT(24, "deque<T>::iterator",        std::input_or_output_iterator<std::deque<int>::iterator>);
    std::cout << FORMAT(24, "forward_list<T>::iterator", std::input_or_output_iterator<std::forward_list<int>::iterator>);
    std::cout << FORMAT(24, "list<T>::iterator",         std::input_or_output_iterator<std::list<int>::iterator>);

    // Associative containers
    std::cout << "  ========== Associative Containers ==========\n";

    std::cout << FORMAT(24, "set<T>::iterator",         std::input_or_output_iterator<std::set<int>::iterator>);
    std::cout << FORMAT(24, "map<K, V>::iterator",      std::input_or_output_iterator<std::map<int, int>::iterator>);
    std::cout << FORMAT(24, "multiset<T>::iterator",    std::input_or_output_iterator<std::multiset<int>::iterator>);
    std::cout << FORMAT(24, "multimap<K, V>::iterator", std::input_or_output_iterator<std::multimap<int, int>::iterator>);

    // Unordered Associative containers
    std::cout << "  ========== Unordered Associative Containers ==========\n";

    std::cout << FORMAT(24, "unordered_set<T>::iterator",         std::input_or_output_iterator<std::unordered_set<int>::iterator>);
    std::cout << FORMAT(24, "unordered_map<K, V>::iterator",      std::input_or_output_iterator<std::unordered_map<int, int>::iterator>);
    std::cout << FORMAT(24, "unordered_multiset<T>::iterator",    std::input_or_output_iterator<std::unordered_multiset<int>::iterator>);
    std::cout << FORMAT(24, "unordered_multimap<K, V>::iterator", std::input_or_output_iterator<std::unordered_multimap<int, int>::iterator>);

    // Container adaptors
    // std::cout << "  ========== Container Adaptors ==========\n";
    //
    // std::cout << FORMAT(14, "stack<T>",          std::input_or_output_iterator<std::stack<int>>);
    // std::cout << FORMAT(14, "queue<T>",          std::input_or_output_iterator<std::queue<int>>);
    // std::cout << FORMAT(14, "priority_queue<T>", std::input_or_output_iterator<std::priority_queue<int>>);

    // Views(Ranges 라이브러리에서 다루는 Views와는 별개의 개념임)
    std::cout << "  ========== Views ==========\n";

    // std::vector vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    // auto mdspan = std::mdspan(vec.data(), 2, 6);

    std::cout << FORMAT(24, "span<T>::iterator", std::input_or_output_iterator<std::span<int>::iterator>);
    // std::cout << FORMAT(14, "mdspan<T, E>", std::input_or_output_iterator<decltype(mdspan)>);
}

void OutputIterator()
{
    // https://en.cppreference.com/w/cpp/iterator/output_iterator
    // 
    // template <class I, class T>
    // concept output_iterator =
    //     std::input_or_output_iterator<I> &&
    //     std::indirectly_writable<I, T> &&
    //     requires(I i, T&& t) {
    //         *i++ = std::forward<T>(t); /* not required to be equality-preserving */
    //     };
    //
    std::cout << "########## std::output_iterator<> ##########\n";

    // Basic types
    // Basic types
    std::cout << "  ========== Basic Types ==========\n";

    // int arr[] = { 1, 2, 3, 4, 5 };
    // 
    // std::cout << FORMAT(14, "int", std::output_iterator<int>);
    // std::cout << FORMAT(14, "T[]", std::output_iterator<decltype(arr)>);
    std::cout << FORMAT(24, "string::iterator", std::output_iterator<std::string::iterator, std::string::value_type>);

    // Sequence containers
    std::cout << "  ========== Sequence Containers ==========\n";
    
    std::cout << FORMAT(24, "array<T>::iterator",        std::output_iterator<std::array<int, 10>::iterator, std::array<int, 10>::value_type>);
    std::cout << FORMAT(24, "vector<T>::iterator",       std::output_iterator<std::vector<int>::iterator, std::vector<int>::value_type>);
    std::cout << FORMAT(24, "deque<T>::iterator",        std::output_iterator<std::deque<int>::iterator, std::deque<int>::value_type>);
    std::cout << FORMAT(24, "forward_list<T>::iterator", std::output_iterator<std::forward_list<int>::iterator, std::forward_list<int>::value_type>);
    std::cout << FORMAT(24, "list<T>::iterator",         std::output_iterator<std::list<int>::iterator, std::list<int>::value_type>);
    
    // Associative containers
    std::cout << "  ========== Associative Containers ==========\n";
    
    std::cout << FORMAT(24, "set<T>::iterator",         std::output_iterator<std::set<int>::iterator, std::set<int>::value_type>);
    std::cout << FORMAT(24, "map<K, V>::iterator",      std::output_iterator<std::map<int, int>::iterator, std::map<int, int>::value_type>);
    std::cout << FORMAT(24, "multiset<T>::iterator",    std::output_iterator<std::multiset<int>::iterator, std::multiset<int>::value_type>);
    std::cout << FORMAT(24, "multimap<K, V>::iterator", std::output_iterator<std::multimap<int, int>::iterator, std::multimap<int, int>::value_type>);
    
    // Unordered Associative containers
    std::cout << "  ========== Unordered Associative Containers ==========\n";
    
    std::cout << FORMAT(24, "unordered_set<T>::iterator",         std::output_iterator<std::unordered_set<int>::iterator, std::unordered_set<int>::value_type>);
    std::cout << FORMAT(24, "unordered_map<K, V>::iterator",      std::output_iterator<std::unordered_map<int, int>::iterator, std::unordered_map<int, int>::value_type>);
    std::cout << FORMAT(24, "unordered_multiset<T>::iterator",    std::output_iterator<std::unordered_multiset<int>::iterator, std::unordered_multiset<int>::value_type>);
    std::cout << FORMAT(24, "unordered_multimap<K, V>::iterator", std::output_iterator<std::unordered_multimap<int, int>::iterator, std::unordered_multimap<int, int>::value_type>);
    
    // Container adaptors
    // std::cout << "  ========== Container Adaptors ==========\n";
    //
    // std::cout << FORMAT(14, "stack<T>",          std::output_iterator<std::stack<int>>);
    // std::cout << FORMAT(14, "queue<T>",          std::output_iterator<std::queue<int>>);
    // std::cout << FORMAT(14, "priority_queue<T>", std::output_iterator<std::priority_queue<int>>);
    
    // Views(Ranges 라이브러리에서 다루는 Views와는 별개의 개념임)
    std::cout << "  ========== Views ==========\n";
    
    // std::vector vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    // auto mdspan = std::mdspan(vec.data(), 2, 6);
    
    std::cout << FORMAT(24, "span<T>::iterator", std::output_iterator<std::span<int>::iterator, std::span<int>::value_type>);
    // std::cout << FORMAT(14, "mdspan<T, E>", std::output_iterator<decltype(mdspan)>);
}

void InputIterator()
{
    // https://en.cppreference.com/w/cpp/iterator/input_iterator
    //
    // template <class I>
    // concept input_iterator =
    //     std::input_or_output_iterator<I> &&
    //     std::indirectly_readable<I> &&
    //     requires { typename /*ITER_CONCEPT*/<I>; } &&
    //     std::derived_from</*ITER_CONCEPT*/<I>, std::input_iterator_tag>;
    //
    std::cout << "########## std::input_iterator<> ##########\n";

    // Basic types
    std::cout << "  ========== Basic Types ==========\n";

    // int arr[] = { 1, 2, 3, 4, 5 };
    // 
    // std::cout << FORMAT(14, "int", std::input_iterator<int>);
    // std::cout << FORMAT(14, "T[]", std::input_iterator<decltype(arr)>);
    std::cout << FORMAT(24, "string::iterator", std::input_iterator<std::string::iterator>);

    // Sequence containers
    std::cout << "  ========== Sequence Containers ==========\n";

    std::cout << FORMAT(24, "array<T>::iterator",        std::input_iterator<std::array<int, 10>::iterator>);
    std::cout << FORMAT(24, "vector<T>::iterator",       std::input_iterator<std::vector<int>::iterator>);
    std::cout << FORMAT(24, "deque<T>::iterator",        std::input_iterator<std::deque<int>::iterator>);
    std::cout << FORMAT(24, "forward_list<T>::iterator", std::input_iterator<std::forward_list<int>::iterator>);
    std::cout << FORMAT(24, "list<T>::iterator",         std::input_iterator<std::list<int>::iterator>);

    // Associative containers
    std::cout << "  ========== Associative Containers ==========\n";

    std::cout << FORMAT(24, "set<T>::iterator",         std::input_iterator<std::set<int>::iterator>);
    std::cout << FORMAT(24, "map<K, V>::iterator",      std::input_iterator<std::map<int, int>::iterator>);
    std::cout << FORMAT(24, "multiset<T>::iterator",    std::input_iterator<std::multiset<int>::iterator>);
    std::cout << FORMAT(24, "multimap<K, V>::iterator", std::input_iterator<std::multimap<int, int>::iterator>);

    // Unordered Associative containers
    std::cout << "  ========== Unordered Associative Containers ==========\n";

    std::cout << FORMAT(24, "unordered_set<T>::iterator",         std::input_iterator<std::unordered_set<int>::iterator>);
    std::cout << FORMAT(24, "unordered_map<K, V>::iterator",      std::input_iterator<std::unordered_map<int, int>::iterator>);
    std::cout << FORMAT(24, "unordered_multiset<T>::iterator",    std::input_iterator<std::unordered_multiset<int>::iterator>);
    std::cout << FORMAT(24, "unordered_multimap<K, V>::iterator", std::input_iterator<std::unordered_multimap<int, int>::iterator>);

    // Container adaptors
    // std::cout << "  ========== Container Adaptors ==========\n";
    //
    // std::cout << FORMAT(14, "stack<T>",          std::input_iterator<std::stack<int>>);
    // std::cout << FORMAT(14, "queue<T>",          std::input_iterator<std::queue<int>>);
    // std::cout << FORMAT(14, "priority_queue<T>", std::input_iterator<std::priority_queue<int>>);

    // Views(Ranges 라이브러리에서 다루는 Views와는 별개의 개념임)
    std::cout << "  ========== Views ==========\n";

    // std::vector vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    // auto mdspan = std::mdspan(vec.data(), 2, 6);

    std::cout << FORMAT(24, "span<T>::iterator", std::input_iterator<std::span<int>::iterator>);
    // std::cout << FORMAT(14, "mdspan<T, E>", std::input_iterator<decltype(mdspan)>);
}

void ForwardIterator()
{
    // https://en.cppreference.com/w/cpp/iterator/forward_iterator
    //
    // template <class I>
    // concept forward_iterator =
    //     std::input_iterator<I> &&
    //     std::derived_from</*ITER_CONCEPT*/<I>, std::forward_iterator_tag> &&
    //     std::incrementable<I> &&
    //     std::sentinel_for<I, I>;
    //
    std::cout << "########## std::forward_iterator<> ##########\n";

    // Basic types
    std::cout << "  ========== Basic Types ==========\n";

    // int arr[] = { 1, 2, 3, 4, 5 };
    // 
    // std::cout << FORMAT(14, "int", std::forward_iterator<int>);
    // std::cout << FORMAT(14, "T[]", std::forward_iterator<decltype(arr)>);
    std::cout << FORMAT(24, "string::iterator", std::forward_iterator<std::string::iterator>);

    // Sequence containers
    std::cout << "  ========== Sequence Containers ==========\n";

    std::cout << FORMAT(24, "array<T>::iterator",        std::forward_iterator<std::array<int, 10>::iterator>);
    std::cout << FORMAT(24, "vector<T>::iterator",       std::forward_iterator<std::vector<int>::iterator>);
    std::cout << FORMAT(24, "deque<T>::iterator",        std::forward_iterator<std::deque<int>::iterator>);
    std::cout << FORMAT(24, "forward_list<T>::iterator", std::forward_iterator<std::forward_list<int>::iterator>);
    std::cout << FORMAT(24, "list<T>::iterator",         std::forward_iterator<std::list<int>::iterator>);

    // Associative containers
    std::cout << "  ========== Associative Containers ==========\n";

    std::cout << FORMAT(24, "set<T>::iterator",         std::forward_iterator<std::set<int>::iterator>);
    std::cout << FORMAT(24, "map<K, V>::iterator",      std::forward_iterator<std::map<int, int>::iterator>);
    std::cout << FORMAT(24, "multiset<T>::iterator",    std::forward_iterator<std::multiset<int>::iterator>);
    std::cout << FORMAT(24, "multimap<K, V>::iterator", std::forward_iterator<std::multimap<int, int>::iterator>);

    // Unordered Associative containers
    std::cout << "  ========== Unordered Associative Containers ==========\n";

    std::cout << FORMAT(24, "unordered_set<T>::iterator",         std::forward_iterator<std::unordered_set<int>::iterator>);
    std::cout << FORMAT(24, "unordered_map<K, V>::iterator",      std::forward_iterator<std::unordered_map<int, int>::iterator>);
    std::cout << FORMAT(24, "unordered_multiset<T>::iterator",    std::forward_iterator<std::unordered_multiset<int>::iterator>);
    std::cout << FORMAT(24, "unordered_multimap<K, V>::iterator", std::forward_iterator<std::unordered_multimap<int, int>::iterator>);

    // Container adaptors
    // std::cout << "  ========== Container Adaptors ==========\n";
    //
    // std::cout << FORMAT(14, "stack<T>",          std::forward_iterator<std::stack<int>>);
    // std::cout << FORMAT(14, "queue<T>",          std::forward_iterator<std::queue<int>>);
    // std::cout << FORMAT(14, "priority_queue<T>", std::forward_iterator<std::priority_queue<int>>);

    // Views(Ranges 라이브러리에서 다루는 Views와는 별개의 개념임)
    std::cout << "  ========== Views ==========\n";

    // std::vector vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    // auto mdspan = std::mdspan(vec.data(), 2, 6);

    std::cout << FORMAT(24, "span<T>::iterator", std::forward_iterator<std::span<int>::iterator>);
    // std::cout << FORMAT(14, "mdspan<T, E>", std::forward_iterator<decltype(mdspan)>);
}

void BidirectionalIterator()
{
    // https://en.cppreference.com/w/cpp/iterator/bidirectional_iterator
    //
    // template <class I>
    // concept bidirectional_iterator =
    //     std::forward_iterator<I> &&
    //     std::derived_from</*ITER_CONCEPT*/<I>, std::bidirectional_iterator_tag> &&
    //     requires(I i) {
    //         { --i } -> std::same_as<I&>;
    //         { i-- } -> std::same_as<I>;
    //     };
    //
    std::cout << "########## std::bidirectional_iterator<> ##########\n";

    // Basic types
    std::cout << "  ========== Basic Types ==========\n";

    // int arr[] = { 1, 2, 3, 4, 5 };
    // 
    // std::cout << FORMAT(14, "int", std::bidirectional_iterator<int>);
    // std::cout << FORMAT(14, "T[]", std::bidirectional_iterator<decltype(arr)>);
    std::cout << FORMAT(24, "string::iterator", std::bidirectional_iterator<std::string::iterator>);

    // Sequence containers
    std::cout << "  ========== Sequence Containers ==========\n";

    std::cout << FORMAT(24, "array<T>::iterator",        std::bidirectional_iterator<std::array<int, 10>::iterator>);
    std::cout << FORMAT(24, "vector<T>::iterator",       std::bidirectional_iterator<std::vector<int>::iterator>);
    std::cout << FORMAT(24, "deque<T>::iterator",        std::bidirectional_iterator<std::deque<int>::iterator>);
    std::cout << FORMAT(24, "forward_list<T>::iterator", std::bidirectional_iterator<std::forward_list<int>::iterator>);
    std::cout << FORMAT(24, "list<T>::iterator",         std::bidirectional_iterator<std::list<int>::iterator>);

    // Associative containers
    std::cout << "  ========== Associative Containers ==========\n";

    std::cout << FORMAT(24, "set<T>::iterator",         std::bidirectional_iterator<std::set<int>::iterator>);
    std::cout << FORMAT(24, "map<K, V>::iterator",      std::bidirectional_iterator<std::map<int, int>::iterator>);
    std::cout << FORMAT(24, "multiset<T>::iterator",    std::bidirectional_iterator<std::multiset<int>::iterator>);
    std::cout << FORMAT(24, "multimap<K, V>::iterator", std::bidirectional_iterator<std::multimap<int, int>::iterator>);

    // Unordered Associative containers
    std::cout << "  ========== Unordered Associative Containers ==========\n";

    std::cout << FORMAT(24, "unordered_set<T>::iterator",         std::bidirectional_iterator<std::unordered_set<int>::iterator>);
    std::cout << FORMAT(24, "unordered_map<K, V>::iterator",      std::bidirectional_iterator<std::unordered_map<int, int>::iterator>);
    std::cout << FORMAT(24, "unordered_multiset<T>::iterator",    std::bidirectional_iterator<std::unordered_multiset<int>::iterator>);
    std::cout << FORMAT(24, "unordered_multimap<K, V>::iterator", std::bidirectional_iterator<std::unordered_multimap<int, int>::iterator>);

    // Container adaptors
    // std::cout << "  ========== Container Adaptors ==========\n";
    //
    // std::cout << FORMAT(14, "stack<T>",          std::bidirectional_iterator<std::stack<int>>);
    // std::cout << FORMAT(14, "queue<T>",          std::bidirectional_iterator<std::queue<int>>);
    // std::cout << FORMAT(14, "priority_queue<T>", std::bidirectional_iterator<std::priority_queue<int>>);

    // Views(Ranges 라이브러리에서 다루는 Views와는 별개의 개념임)
    std::cout << "  ========== Views ==========\n";

    // std::vector vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    // auto mdspan = std::mdspan(vec.data(), 2, 6);

    std::cout << FORMAT(24, "span<T>::iterator", std::bidirectional_iterator<std::span<int>::iterator>);
    // std::cout << FORMAT(14, "mdspan<T, E>", std::bidirectional_iterator<decltype(mdspan)>);
}

void RandomAccessIterator()
{
    // https://en.cppreference.com/w/cpp/iterator/random_access_iterator
    //
    // template <class I>
    // concept random_access_iterator =
    //     std::bidirectional_iterator<I> &&
    //     std::derived_from</*ITER_CONCEPT*/<I>, std::random_access_iterator_tag> &&
    //     std::totally_ordered<I> &&
    //     std::sized_sentinel_for<I, I> &&
    //     requires(I i, const I j, const std::iter_difference_t<I> n) {
    //         { i += n } -> std::same_as<I&>;
    //         { j +  n } -> std::same_as<I>;
    //         { n +  j } -> std::same_as<I>;
    //         { i -= n } -> std::same_as<I&>;
    //         { j -  n } -> std::same_as<I>;
    //         {  j[n]  } -> std::same_as<std::iter_reference_t<I>>;
    //     };
    //
    std::cout << "########## std::random_access_iterator<> ##########\n";

    // Basic types
    std::cout << "  ========== Basic Types ==========\n";

    // int arr[] = { 1, 2, 3, 4, 5 };
    // 
    // std::cout << FORMAT(14, "int", std::random_access_iterator<int>);
    // std::cout << FORMAT(14, "T[]", std::random_access_iterator<decltype(arr)>);
    std::cout << FORMAT(24, "string::iterator", std::random_access_iterator<std::string::iterator>);

    // Sequence containers
    std::cout << "  ========== Sequence Containers ==========\n";

    std::cout << FORMAT(24, "array<T>::iterator",        std::random_access_iterator<std::array<int, 10>::iterator>);
    std::cout << FORMAT(24, "vector<T>::iterator",       std::random_access_iterator<std::vector<int>::iterator>);
    std::cout << FORMAT(24, "deque<T>::iterator",        std::random_access_iterator<std::deque<int>::iterator>);
    std::cout << FORMAT(24, "forward_list<T>::iterator", std::random_access_iterator<std::forward_list<int>::iterator>);
    std::cout << FORMAT(24, "list<T>::iterator",         std::random_access_iterator<std::list<int>::iterator>);

    // Associative containers
    std::cout << "  ========== Associative Containers ==========\n";

    std::cout << FORMAT(24, "set<T>::iterator",         std::random_access_iterator<std::set<int>::iterator>);
    std::cout << FORMAT(24, "map<K, V>::iterator",      std::random_access_iterator<std::map<int, int>::iterator>);
    std::cout << FORMAT(24, "multiset<T>::iterator",    std::random_access_iterator<std::multiset<int>::iterator>);
    std::cout << FORMAT(24, "multimap<K, V>::iterator", std::random_access_iterator<std::multimap<int, int>::iterator>);

    // Unordered Associative containers
    std::cout << "  ========== Unordered Associative Containers ==========\n";

    std::cout << FORMAT(24, "unordered_set<T>::iterator",         std::random_access_iterator<std::unordered_set<int>::iterator>);
    std::cout << FORMAT(24, "unordered_map<K, V>::iterator",      std::random_access_iterator<std::unordered_map<int, int>::iterator>);
    std::cout << FORMAT(24, "unordered_multiset<T>::iterator",    std::random_access_iterator<std::unordered_multiset<int>::iterator>);
    std::cout << FORMAT(24, "unordered_multimap<K, V>::iterator", std::random_access_iterator<std::unordered_multimap<int, int>::iterator>);

    // Container adaptors
    // std::cout << "  ========== Container Adaptors ==========\n";
    //
    // std::cout << FORMAT(14, "stack<T>",          std::random_access_iterator<std::stack<int>>);
    // std::cout << FORMAT(14, "queue<T>",          std::random_access_iterator<std::queue<int>>);
    // std::cout << FORMAT(14, "priority_queue<T>", std::random_access_iterator<std::priority_queue<int>>);

    // Views(Ranges 라이브러리에서 다루는 Views와는 별개의 개념임)
    std::cout << "  ========== Views ==========\n";

    // std::vector vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    // auto mdspan = std::mdspan(vec.data(), 2, 6);

    std::cout << FORMAT(24, "span<T>::iterator", std::random_access_iterator<std::span<int>::iterator>);
    // std::cout << FORMAT(14, "mdspan<T, E>", std::random_access_iterator<decltype(mdspan)>);
}

void ContiguousIterator()
{
    // https://en.cppreference.com/w/cpp/iterator/contiguous_iterator
    //
    // template <class I>
    // concept contiguous_iterator =
    //     std::random_access_iterator<I> &&
    //     std::derived_from</*ITER_CONCEPT*/<I>, std::contiguous_iterator_tag> &&
    //     std::is_lvalue_reference_v<std::iter_reference_t<I>> &&
    //     std::same_as<
    //         std::iter_value_t<I>, std::remove_cvref_t<std::iter_reference_t<I>>
    //     > &&
    //     requires(const I& i) {
    //         { std::to_address(i) } ->
    //           std::same_as<std::add_pointer_t<std::iter_reference_t<I>>>;
    // 
    //     };
    //
    std::cout << "########## std::contiguous_iterator<> ##########\n";

    // Basic types
    std::cout << "  ========== Basic Types ==========\n";

    // int arr[] = { 1, 2, 3, 4, 5 };
    // 
    // std::cout << FORMAT(14, "int", std::contiguous_iterator<int>);
    // std::cout << FORMAT(14, "T[]", std::contiguous_iterator<decltype(arr)>);
    std::cout << FORMAT(24, "string::iterator", std::contiguous_iterator<std::string::iterator>);

    // Sequence containers
    std::cout << "  ========== Sequence Containers ==========\n";

    std::cout << FORMAT(24, "array<T>::iterator",        std::contiguous_iterator<std::array<int, 10>::iterator>);
    std::cout << FORMAT(24, "vector<T>::iterator",       std::contiguous_iterator<std::vector<int>::iterator>);
    std::cout << FORMAT(24, "deque<T>::iterator",        std::contiguous_iterator<std::deque<int>::iterator>);
    std::cout << FORMAT(24, "forward_list<T>::iterator", std::contiguous_iterator<std::forward_list<int>::iterator>);
    std::cout << FORMAT(24, "list<T>::iterator",         std::contiguous_iterator<std::list<int>::iterator>);

    // Associative containers
    std::cout << "  ========== Associative Containers ==========\n";

    std::cout << FORMAT(24, "set<T>::iterator",         std::contiguous_iterator<std::set<int>::iterator>);
    std::cout << FORMAT(24, "map<K, V>::iterator",      std::contiguous_iterator<std::map<int, int>::iterator>);
    std::cout << FORMAT(24, "multiset<T>::iterator",    std::contiguous_iterator<std::multiset<int>::iterator>);
    std::cout << FORMAT(24, "multimap<K, V>::iterator", std::contiguous_iterator<std::multimap<int, int>::iterator>);

    // Unordered Associative containers
    std::cout << "  ========== Unordered Associative Containers ==========\n";

    std::cout << FORMAT(24, "unordered_set<T>::iterator",         std::contiguous_iterator<std::unordered_set<int>::iterator>);
    std::cout << FORMAT(24, "unordered_map<K, V>::iterator",      std::contiguous_iterator<std::unordered_map<int, int>::iterator>);
    std::cout << FORMAT(24, "unordered_multiset<T>::iterator",    std::contiguous_iterator<std::unordered_multiset<int>::iterator>);
    std::cout << FORMAT(24, "unordered_multimap<K, V>::iterator", std::contiguous_iterator<std::unordered_multimap<int, int>::iterator>);

    // Container adaptors
    // std::cout << "  ========== Container Adaptors ==========\n";
    //
    // std::cout << FORMAT(14, "stack<T>",          std::contiguous_iterator<std::stack<int>>);
    // std::cout << FORMAT(14, "queue<T>",          std::contiguous_iterator<std::queue<int>>);
    // std::cout << FORMAT(14, "priority_queue<T>", std::contiguous_iterator<std::priority_queue<int>>);

    // Views(Ranges 라이브러리에서 다루는 Views와는 별개의 개념임)
    std::cout << "  ========== Views ==========\n";

    // std::vector vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    // auto mdspan = std::mdspan(vec.data(), 2, 6);

    std::cout << FORMAT(24, "span<T>::iterator", std::contiguous_iterator<std::span<int>::iterator>);
    // std::cout << FORMAT(14, "mdspan<T, E>", std::contiguous_iterator<decltype(mdspan)>);
}

void Run()
{
    InputOrOutputIterator(); std::cout << '\n';
    OutputIterator();        std::cout << '\n';
    InputIterator();         std::cout << '\n';
    ForwardIterator();       std::cout << '\n';
    BidirectionalIterator(); std::cout << '\n';
    RandomAccessIterator();  std::cout << '\n';
    ContiguousIterator();    std::cout << '\n';
}

END_NS

BEGIN_NS(Case03)

// range 제약은 단독으로 쓰이는 것이 아닌 iterator 제약과 함께 사용되는 경우가 많다.
// 마찬가지로 iterator 제약은 알고리즘의 특성과 컨테이너의 특성을 제한하기 위해 사용된다.

void OutputRange()
{
    // https://en.cppreference.com/w/cpp/ranges/output_range
    // 
    // template <class R, class T>
    // concept output_range =
    //     ranges::range<R> && std::output_iterator<ranges::iterator_t<R>, T>;
    //
    std::cout << "########## std::ranges::output_range<> ##########\n";

    // Basic types
    std::cout << "  ========== Basic Types ==========\n";

    int arr[] = { 1, 2, 3, 4, 5 };
    
    std::cout << FORMAT(14, "int",    std::ranges::output_range<int, int>);
    std::cout << FORMAT(14, "T[]",    std::ranges::output_range<decltype(arr), int>);
    std::cout << FORMAT(14, "string", std::ranges::output_range<std::string, std::string::value_type>);

    // Sequence containers
    std::cout << "  ========== Sequence Containers ==========\n";

    std::cout << FORMAT(14, "array<T>",        std::ranges::output_range<std::array<int, 10>, std::array<int, 10>::value_type>);
    std::cout << FORMAT(14, "vector<T>",       std::ranges::output_range<std::vector<int>, std::vector<int>::value_type>);
    std::cout << FORMAT(14, "deque<T>",        std::ranges::output_range<std::deque<int>, std::deque<int>::value_type>);
    std::cout << FORMAT(14, "forward_list<T>", std::ranges::output_range<std::forward_list<int>, std::forward_list<int>::value_type>);
    std::cout << FORMAT(14, "list<T>",         std::ranges::output_range<std::list<int>, std::list<int>::value_type>);

    // Associative containers
    std::cout << "  ========== Associative Containers ==========\n";

    std::cout << FORMAT(14, "set<T>",         std::ranges::output_range<std::set<int>, std::set<int>::value_type>);
    std::cout << FORMAT(14, "map<K, V>",      std::ranges::output_range<std::map<int, int>, std::map<int, int>::value_type>);
    std::cout << FORMAT(14, "multiset<T>",    std::ranges::output_range<std::multiset<int>, std::multiset<int>::value_type>);
    std::cout << FORMAT(14, "multimap<K, V>", std::ranges::output_range<std::multimap<int, int>, std::multimap<int, int>::value_type>);

    // Unordered Associative containers
    std::cout << "  ========== Unordered Associative Containers ==========\n";

    std::cout << FORMAT(14, "unordered_set<T>",         std::ranges::output_range<std::unordered_set<int>, std::unordered_set<int>::value_type>);
    std::cout << FORMAT(14, "unordered_map<K, V>",      std::ranges::output_range<std::unordered_map<int, int>, std::unordered_map<int, int>::value_type>);
    std::cout << FORMAT(14, "unordered_multiset<T>",    std::ranges::output_range<std::unordered_multiset<int>, std::unordered_multiset<int>::value_type>);
    std::cout << FORMAT(14, "unordered_multimap<K, V>", std::ranges::output_range<std::unordered_multimap<int, int>, std::unordered_multimap<int, int>::value_type>);

    // Container adaptors
    std::cout << "  ========== Container Adaptors ==========\n";
    
    std::cout << FORMAT(14, "stack<T>",          std::ranges::output_range<std::stack<int>, std::stack<int>::value_type>);
    std::cout << FORMAT(14, "queue<T>",          std::ranges::output_range<std::queue<int>, std::queue<int>::value_type>);
    std::cout << FORMAT(14, "priority_queue<T>", std::ranges::output_range<std::priority_queue<int>, std::priority_queue<int>::value_type>);

    // Views(Ranges 라이브러리에서 다루는 Views와는 별개의 개념임)
    std::cout << "  ========== Views ==========\n";

    std::vector vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    auto mdspan = std::mdspan(vec.data(), 2, 6);

    std::cout << FORMAT(14, "span<T>",      std::ranges::output_range<std::span<int>, std::span<int>::value_type>);
    std::cout << FORMAT(14, "mdspan<T, E>", std::ranges::output_range<decltype(mdspan), decltype(mdspan)::value_type>);
}

void InputRange()
{
    // https://en.cppreference.com/w/cpp/ranges/input_range
    // 
    // template <class T>
    // concept input_range =
    //     ranges::range<T> && std::input_iterator<ranges::iterator_t<T>>;
    //
    std::cout << "########## std::ranges::input_range<> ##########\n";

    // Basic types
    std::cout << "  ========== Basic Types ==========\n";

    int arr[] = { 1, 2, 3, 4, 5 };
    
    std::cout << FORMAT(14, "int",    std::ranges::input_range<int>);
    std::cout << FORMAT(14, "T[]",    std::ranges::input_range<decltype(arr)>);
    std::cout << FORMAT(14, "string", std::ranges::input_range<std::string>);

    // Sequence containers
    std::cout << "  ========== Sequence Containers ==========\n";

    std::cout << FORMAT(14, "array<T>",        std::ranges::input_range<std::array<int, 10>>);
    std::cout << FORMAT(14, "vector<T>",       std::ranges::input_range<std::vector<int>>);
    std::cout << FORMAT(14, "deque<T>",        std::ranges::input_range<std::deque<int>>);
    std::cout << FORMAT(14, "forward_list<T>", std::ranges::input_range<std::forward_list<int>>);
    std::cout << FORMAT(14, "list<T>",         std::ranges::input_range<std::list<int>>);

    // Associative containers
    std::cout << "  ========== Associative Containers ==========\n";

    std::cout << FORMAT(14, "set<T>",         std::ranges::input_range<std::set<int>>);
    std::cout << FORMAT(14, "map<K, V>",      std::ranges::input_range<std::map<int, int>>);
    std::cout << FORMAT(14, "multiset<T>",    std::ranges::input_range<std::multiset<int>>);
    std::cout << FORMAT(14, "multimap<K, V>", std::ranges::input_range<std::multimap<int, int>>);

    // Unordered Associative containers
    std::cout << "  ========== Unordered Associative Containers ==========\n";

    std::cout << FORMAT(14, "unordered_set<T>",         std::ranges::input_range<std::unordered_set<int>>);
    std::cout << FORMAT(14, "unordered_map<K, V>",      std::ranges::input_range<std::unordered_map<int, int>>);
    std::cout << FORMAT(14, "unordered_multiset<T>",    std::ranges::input_range<std::unordered_multiset<int>>);
    std::cout << FORMAT(14, "unordered_multimap<K, V>", std::ranges::input_range<std::unordered_multimap<int, int>>);

    // Container adaptors
    std::cout << "  ========== Container Adaptors ==========\n";
    
    std::cout << FORMAT(14, "stack<T>",          std::ranges::input_range<std::stack<int>>);
    std::cout << FORMAT(14, "queue<T>",          std::ranges::input_range<std::queue<int>>);
    std::cout << FORMAT(14, "priority_queue<T>", std::ranges::input_range<std::priority_queue<int>>);

    // Views(Ranges 라이브러리에서 다루는 Views와는 별개의 개념임)
    std::cout << "  ========== Views ==========\n";

    std::vector vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    auto mdspan = std::mdspan(vec.data(), 2, 6);

    std::cout << FORMAT(14, "span<T>",      std::ranges::input_range<std::span<int>>);
    std::cout << FORMAT(14, "mdspan<T, E>", std::ranges::input_range<decltype(mdspan)>);
}

void ForwardRange()
{
    // https://en.cppreference.com/w/cpp/ranges/forward_range
    // 
    // template <class T>
    // concept forward_range =
    //     ranges::input_range<T> && std::forward_iterator<ranges::iterator_t<T>>;
    //
    std::cout << "########## std::ranges::forward_range<> ##########\n";

    // Basic types
    std::cout << "  ========== Basic Types ==========\n";

    int arr[] = { 1, 2, 3, 4, 5 };
    
    std::cout << FORMAT(14, "int",    std::ranges::forward_range<int>);
    std::cout << FORMAT(14, "T[]",    std::ranges::forward_range<decltype(arr)>);
    std::cout << FORMAT(14, "string", std::ranges::forward_range<std::string>);

    // Sequence containers
    std::cout << "  ========== Sequence Containers ==========\n";

    std::cout << FORMAT(14, "array<T>",        std::ranges::forward_range<std::array<int, 10>>);
    std::cout << FORMAT(14, "vector<T>",       std::ranges::forward_range<std::vector<int>>);
    std::cout << FORMAT(14, "deque<T>",        std::ranges::forward_range<std::deque<int>>);
    std::cout << FORMAT(14, "forward_list<T>", std::ranges::forward_range<std::forward_list<int>>);
    std::cout << FORMAT(14, "list<T>",         std::ranges::forward_range<std::list<int>>);

    // Associative containers
    std::cout << "  ========== Associative Containers ==========\n";

    std::cout << FORMAT(14, "set<T>",         std::ranges::forward_range<std::set<int>>);
    std::cout << FORMAT(14, "map<K, V>",      std::ranges::forward_range<std::map<int, int>>);
    std::cout << FORMAT(14, "multiset<T>",    std::ranges::forward_range<std::multiset<int>>);
    std::cout << FORMAT(14, "multimap<K, V>", std::ranges::forward_range<std::multimap<int, int>>);

    // Unordered Associative containers
    std::cout << "  ========== Unordered Associative Containers ==========\n";

    std::cout << FORMAT(14, "unordered_set<T>",         std::ranges::forward_range<std::unordered_set<int>>);
    std::cout << FORMAT(14, "unordered_map<K, V>",      std::ranges::forward_range<std::unordered_map<int, int>>);
    std::cout << FORMAT(14, "unordered_multiset<T>",    std::ranges::forward_range<std::unordered_multiset<int>>);
    std::cout << FORMAT(14, "unordered_multimap<K, V>", std::ranges::forward_range<std::unordered_multimap<int, int>>);

    // Container adaptors
    std::cout << "  ========== Container Adaptors ==========\n";
    
    std::cout << FORMAT(14, "stack<T>",          std::ranges::forward_range<std::stack<int>>);
    std::cout << FORMAT(14, "queue<T>",          std::ranges::forward_range<std::queue<int>>);
    std::cout << FORMAT(14, "priority_queue<T>", std::ranges::forward_range<std::priority_queue<int>>);

    // Views(Ranges 라이브러리에서 다루는 Views와는 별개의 개념임)
    std::cout << "  ========== Views ==========\n";

    std::vector vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    auto mdspan = std::mdspan(vec.data(), 2, 6);

    std::cout << FORMAT(14, "span<T>",      std::ranges::forward_range<std::span<int>>);
    std::cout << FORMAT(14, "mdspan<T, E>", std::ranges::forward_range<decltype(mdspan)>);
}

void BidirectionalRange()
{
    // https://en.cppreference.com/w/cpp/ranges/bidirectional_range
    // 
    // template <class T>
    // concept bidirectional_range =
    //     ranges::forward_range<T> && std::bidirectional_iterator<ranges::iterator_t<T>>;
    //
    std::cout << "########## std::ranges::bidirectional_range<> ##########\n";

    // Basic types
    std::cout << "  ========== Basic Types ==========\n";

    int arr[] = { 1, 2, 3, 4, 5 };
    
    std::cout << FORMAT(14, "int",    std::ranges::bidirectional_range<int>);
    std::cout << FORMAT(14, "T[]",    std::ranges::bidirectional_range<decltype(arr)>);
    std::cout << FORMAT(14, "string", std::ranges::bidirectional_range<std::string>);

    // Sequence containers
    std::cout << "  ========== Sequence Containers ==========\n";

    std::cout << FORMAT(14, "array<T>",        std::ranges::bidirectional_range<std::array<int, 10>>);
    std::cout << FORMAT(14, "vector<T>",       std::ranges::bidirectional_range<std::vector<int>>);
    std::cout << FORMAT(14, "deque<T>",        std::ranges::bidirectional_range<std::deque<int>>);
    std::cout << FORMAT(14, "forward_list<T>", std::ranges::bidirectional_range<std::forward_list<int>>);
    std::cout << FORMAT(14, "list<T>",         std::ranges::bidirectional_range<std::list<int>>);

    // Associative containers
    std::cout << "  ========== Associative Containers ==========\n";

    std::cout << FORMAT(14, "set<T>",         std::ranges::bidirectional_range<std::set<int>>);
    std::cout << FORMAT(14, "map<K, V>",      std::ranges::bidirectional_range<std::map<int, int>>);
    std::cout << FORMAT(14, "multiset<T>",    std::ranges::bidirectional_range<std::multiset<int>>);
    std::cout << FORMAT(14, "multimap<K, V>", std::ranges::bidirectional_range<std::multimap<int, int>>);

    // Unordered Associative containers
    std::cout << "  ========== Unordered Associative Containers ==========\n";

    std::cout << FORMAT(14, "unordered_set<T>",         std::ranges::bidirectional_range<std::unordered_set<int>>);
    std::cout << FORMAT(14, "unordered_map<K, V>",      std::ranges::bidirectional_range<std::unordered_map<int, int>>);
    std::cout << FORMAT(14, "unordered_multiset<T>",    std::ranges::bidirectional_range<std::unordered_multiset<int>>);
    std::cout << FORMAT(14, "unordered_multimap<K, V>", std::ranges::bidirectional_range<std::unordered_multimap<int, int>>);

    // Container adaptors
    std::cout << "  ========== Container Adaptors ==========\n";
    
    std::cout << FORMAT(14, "stack<T>",          std::ranges::bidirectional_range<std::stack<int>>);
    std::cout << FORMAT(14, "queue<T>",          std::ranges::bidirectional_range<std::queue<int>>);
    std::cout << FORMAT(14, "priority_queue<T>", std::ranges::bidirectional_range<std::priority_queue<int>>);

    // Views(Ranges 라이브러리에서 다루는 Views와는 별개의 개념임)
    std::cout << "  ========== Views ==========\n";

    std::vector vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    auto mdspan = std::mdspan(vec.data(), 2, 6);

    std::cout << FORMAT(14, "span<T>",      std::ranges::bidirectional_range<std::span<int>>);
    std::cout << FORMAT(14, "mdspan<T, E>", std::ranges::bidirectional_range<decltype(mdspan)>);
}

void RandomAccessRange()
{
    // https://en.cppreference.com/w/cpp/ranges/random_access_range
    // 
    // template <class T>
    // concept random_access_range =
    //     ranges::bidirectional_range<T> && std::random_access_iterator<ranges::iterator_t<T>>;
    //
    std::cout << "########## std::ranges::random_access_range<> ##########\n";

    // Basic types
    std::cout << "  ========== Basic Types ==========\n";

    int arr[] = { 1, 2, 3, 4, 5 };
    
    std::cout << FORMAT(14, "int",    std::ranges::random_access_range<int>);
    std::cout << FORMAT(14, "T[]",    std::ranges::random_access_range<decltype(arr)>);
    std::cout << FORMAT(14, "string", std::ranges::random_access_range<std::string>);

    // Sequence containers
    std::cout << "  ========== Sequence Containers ==========\n";

    std::cout << FORMAT(14, "array<T>",        std::ranges::random_access_range<std::array<int, 10>>);
    std::cout << FORMAT(14, "vector<T>",       std::ranges::random_access_range<std::vector<int>>);
    std::cout << FORMAT(14, "deque<T>",        std::ranges::random_access_range<std::deque<int>>);
    std::cout << FORMAT(14, "forward_list<T>", std::ranges::random_access_range<std::forward_list<int>>);
    std::cout << FORMAT(14, "list<T>",         std::ranges::random_access_range<std::list<int>>);

    // Associative containers
    std::cout << "  ========== Associative Containers ==========\n";

    std::cout << FORMAT(14, "set<T>",         std::ranges::random_access_range<std::set<int>>);
    std::cout << FORMAT(14, "map<K, V>",      std::ranges::random_access_range<std::map<int, int>>);
    std::cout << FORMAT(14, "multiset<T>",    std::ranges::random_access_range<std::multiset<int>>);
    std::cout << FORMAT(14, "multimap<K, V>", std::ranges::random_access_range<std::multimap<int, int>>);

    // Unordered Associative containers
    std::cout << "  ========== Unordered Associative Containers ==========\n";

    std::cout << FORMAT(14, "unordered_set<T>",         std::ranges::random_access_range<std::unordered_set<int>>);
    std::cout << FORMAT(14, "unordered_map<K, V>",      std::ranges::random_access_range<std::unordered_map<int, int>>);
    std::cout << FORMAT(14, "unordered_multiset<T>",    std::ranges::random_access_range<std::unordered_multiset<int>>);
    std::cout << FORMAT(14, "unordered_multimap<K, V>", std::ranges::random_access_range<std::unordered_multimap<int, int>>);

    // Container adaptors
    std::cout << "  ========== Container Adaptors ==========\n";
    
    std::cout << FORMAT(14, "stack<T>",          std::ranges::random_access_range<std::stack<int>>);
    std::cout << FORMAT(14, "queue<T>",          std::ranges::random_access_range<std::queue<int>>);
    std::cout << FORMAT(14, "priority_queue<T>", std::ranges::random_access_range<std::priority_queue<int>>);

    // Views(Ranges 라이브러리에서 다루는 Views와는 별개의 개념임)
    std::cout << "  ========== Views ==========\n";

    std::vector vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    auto mdspan = std::mdspan(vec.data(), 2, 6);

    std::cout << FORMAT(14, "span<T>",      std::ranges::random_access_range<std::span<int>>);
    std::cout << FORMAT(14, "mdspan<T, E>", std::ranges::random_access_range<decltype(mdspan)>);
}

void ContiguousRange()
{
    // https://en.cppreference.com/w/cpp/ranges/contiguous_range
    // 
    // template< class T >
    // concept contiguous_range =
    //     ranges::random_access_range<T> &&
    //     std::contiguous_iterator<ranges::iterator_t<T>> &&
    //     requires(T& t) {
    //         { ranges::data(t) } ->
    //             std::same_as<std::add_pointer_t<ranges::range_reference_t<T>>>;
    //     };
    //
    std::cout << "########## std::ranges::contiguous_range<> ##########\n";

    // Basic types
    std::cout << "  ========== Basic Types ==========\n";

    int arr[] = { 1, 2, 3, 4, 5 };
    
    std::cout << FORMAT(14, "int",    std::ranges::contiguous_range<int>);
    std::cout << FORMAT(14, "T[]",    std::ranges::contiguous_range<decltype(arr)>);
    std::cout << FORMAT(14, "string", std::ranges::contiguous_range<std::string>);

    // Sequence containers
    std::cout << "  ========== Sequence Containers ==========\n";

    std::cout << FORMAT(14, "array<T>",        std::ranges::contiguous_range<std::array<int, 10>>);
    std::cout << FORMAT(14, "vector<T>",       std::ranges::contiguous_range<std::vector<int>>);
    std::cout << FORMAT(14, "deque<T>",        std::ranges::contiguous_range<std::deque<int>>);
    std::cout << FORMAT(14, "forward_list<T>", std::ranges::contiguous_range<std::forward_list<int>>);
    std::cout << FORMAT(14, "list<T>",         std::ranges::contiguous_range<std::list<int>>);

    // Associative containers
    std::cout << "  ========== Associative Containers ==========\n";

    std::cout << FORMAT(14, "set<T>",         std::ranges::contiguous_range<std::set<int>>);
    std::cout << FORMAT(14, "map<K, V>",      std::ranges::contiguous_range<std::map<int, int>>);
    std::cout << FORMAT(14, "multiset<T>",    std::ranges::contiguous_range<std::multiset<int>>);
    std::cout << FORMAT(14, "multimap<K, V>", std::ranges::contiguous_range<std::multimap<int, int>>);

    // Unordered Associative containers
    std::cout << "  ========== Unordered Associative Containers ==========\n";

    std::cout << FORMAT(14, "unordered_set<T>",         std::ranges::contiguous_range<std::unordered_set<int>>);
    std::cout << FORMAT(14, "unordered_map<K, V>",      std::ranges::contiguous_range<std::unordered_map<int, int>>);
    std::cout << FORMAT(14, "unordered_multiset<T>",    std::ranges::contiguous_range<std::unordered_multiset<int>>);
    std::cout << FORMAT(14, "unordered_multimap<K, V>", std::ranges::contiguous_range<std::unordered_multimap<int, int>>);

    // Container adaptors
    std::cout << "  ========== Container Adaptors ==========\n";
    
    std::cout << FORMAT(14, "stack<T>",          std::ranges::contiguous_range<std::stack<int>>);
    std::cout << FORMAT(14, "queue<T>",          std::ranges::contiguous_range<std::queue<int>>);
    std::cout << FORMAT(14, "priority_queue<T>", std::ranges::contiguous_range<std::priority_queue<int>>);

    // Views(Ranges 라이브러리에서 다루는 Views와는 별개의 개념임)
    std::cout << "  ========== Views ==========\n";

    std::vector vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    auto mdspan = std::mdspan(vec.data(), 2, 6);

    std::cout << FORMAT(14, "span<T>",      std::ranges::contiguous_range<std::span<int>>);
    std::cout << FORMAT(14, "mdspan<T, E>", std::ranges::contiguous_range<decltype(mdspan)>);
}

void SizedRange()
{
    // https://en.cppreference.com/w/cpp/ranges/sized_range
    // 
    // template< class T >
    // concept sized_range = ranges::range<T> &&
    //     requires(T& t) {
    //         ranges::size(t);
    //     };
    //
    std::cout << "########## std::ranges::sized_range<> ##########\n";

    // Basic types
    std::cout << "  ========== Basic Types ==========\n";

    int arr[] = { 1, 2, 3, 4, 5 };
    
    std::cout << FORMAT(14, "int",    std::ranges::sized_range<int>);
    std::cout << FORMAT(14, "T[]",    std::ranges::sized_range<decltype(arr)>);
    std::cout << FORMAT(14, "string", std::ranges::sized_range<std::string>);

    // Sequence containers
    std::cout << "  ========== Sequence Containers ==========\n";

    std::cout << FORMAT(14, "array<T>",        std::ranges::sized_range<std::array<int, 10>>);
    std::cout << FORMAT(14, "vector<T>",       std::ranges::sized_range<std::vector<int>>);
    std::cout << FORMAT(14, "deque<T>",        std::ranges::sized_range<std::deque<int>>);
    std::cout << FORMAT(14, "forward_list<T>", std::ranges::sized_range<std::forward_list<int>>);
    std::cout << FORMAT(14, "list<T>",         std::ranges::sized_range<std::list<int>>);

    // Associative containers
    std::cout << "  ========== Associative Containers ==========\n";

    std::cout << FORMAT(14, "set<T>",         std::ranges::sized_range<std::set<int>>);
    std::cout << FORMAT(14, "map<K, V>",      std::ranges::sized_range<std::map<int, int>>);
    std::cout << FORMAT(14, "multiset<T>",    std::ranges::sized_range<std::multiset<int>>);
    std::cout << FORMAT(14, "multimap<K, V>", std::ranges::sized_range<std::multimap<int, int>>);

    // Unordered Associative containers
    std::cout << "  ========== Unordered Associative Containers ==========\n";

    std::cout << FORMAT(14, "unordered_set<T>",         std::ranges::sized_range<std::unordered_set<int>>);
    std::cout << FORMAT(14, "unordered_map<K, V>",      std::ranges::sized_range<std::unordered_map<int, int>>);
    std::cout << FORMAT(14, "unordered_multiset<T>",    std::ranges::sized_range<std::unordered_multiset<int>>);
    std::cout << FORMAT(14, "unordered_multimap<K, V>", std::ranges::sized_range<std::unordered_multimap<int, int>>);

    // Container adaptors
    std::cout << "  ========== Container Adaptors ==========\n";
    
    std::cout << FORMAT(14, "stack<T>",          std::ranges::sized_range<std::stack<int>>);
    std::cout << FORMAT(14, "queue<T>",          std::ranges::sized_range<std::queue<int>>);
    std::cout << FORMAT(14, "priority_queue<T>", std::ranges::sized_range<std::priority_queue<int>>);

    // Views(Ranges 라이브러리에서 다루는 Views와는 별개의 개념임)
    std::cout << "  ========== Views ==========\n";

    std::vector vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    auto mdspan = std::mdspan(vec.data(), 2, 6);

    std::cout << FORMAT(14, "span<T>",      std::ranges::sized_range<std::span<int>>);
    std::cout << FORMAT(14, "mdspan<T, E>", std::ranges::sized_range<decltype(mdspan)>);
}

void Run()
{
    OutputRange();        std::cout << '\n';
    InputRange();         std::cout << '\n';
    ForwardRange();       std::cout << '\n';
    BidirectionalRange(); std::cout << '\n';
    RandomAccessRange();  std::cout << '\n';
    ContiguousRange();    std::cout << '\n';
    SizedRange();         std::cout << '\n';
}

END_NS

int main()
{
    // Case01::Run();
    // Case02::Run();
    Case03::Run();

    return 0;
}
