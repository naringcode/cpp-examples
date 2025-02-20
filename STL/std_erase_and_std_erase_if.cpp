// Update Date : 2025-02-21
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <algorithm>
#include <format>
#include <array>
#include <vector>
#include <list>
#include <forward_list>
#include <deque>
#include <set>
#include <unordered_set>

#define BEGIN_NS(name) namespace name {
#define END_NS }

template <typename Range>
void Print(const Range& range, std::string_view msg = "")
{
    std::cout << std::format("{:<20} [ ", msg);

    for (auto& elem : range)
    {
        std::cout << elem << ' ';
    }

    std::cout << "]\n";
}

BEGIN_NS(Case01)

// https://en.cppreference.com/w/cpp/algorithm/ranges/remove
// https://en.cppreference.com/w/cpp/container/vector/erase
// https://en.cppreference.com/w/cpp/container/list/remove
// https://en.cppreference.com/w/cpp/container/forward_list/remove
// https://en.cppreference.com/w/cpp/container/deque/erase
// https://en.cppreference.com/w/cpp/container/set/erase
// https://en.cppreference.com/w/cpp/container/multiset/erase
// https://en.cppreference.com/w/cpp/container/unordered_set/erase
// https://en.cppreference.com/w/cpp/container/unordered_multiset/erase

// 기존 컨테이너에서 특정 값을 제거하는 건 상당히 일관적이지 않다(inconsistent).

void Run()
{
    std::array sourceArr{ 1, 2, 3, 2, 2, 4, 2, 7, 2, 9 };
    Print(sourceArr, "source array");

    std::cout << '\n';

    // vector
    {
        std::vector<int> vec{ std::begin(sourceArr), std::end(sourceArr) };
        Print(vec, "vector");

        // vector 기반의 erase()는 iterator 기반으로 동작하기 때문에 remove()를 작업이 선행되어야 한다.
        auto rmvRange = std::ranges::remove(vec, 2); // ranges 버전의 remove()는 subrange를 반환함.

        Print(vec, "after remove()");
        Print(rmvRange, "returned by remove()");

        // vector 기반에서 특정 값을 제거하고자 할 때는 혼란스럽긴 해도 이렇게 진행해야 한다.
        vec.erase(rmvRange.begin(), rmvRange.end());
        
        Print(vec, "after erase()");
    }

    std::cout << '\n';

    // list
    {
        std::list<int> list{ std::begin(sourceArr), std::end(sourceArr) };
        Print(list, "list");

        // list는 remove()를 통해 특정 값을 제거할 수 있는 기능을 제공한다.
        list.remove(2);

        Print(list, "after remove()");
    }

    std::cout << '\n';

    // forward_list
    {
        std::forward_list<int> forwardList{ std::begin(sourceArr), std::end(sourceArr) };
        Print(forwardList, "forward_list");

        // forward_list도 list와 동일하게 remove()를 통해 특정 값을 제거할 수 있다.
        forwardList.remove(2);

        Print(forwardList, "after remove()");
    }

    std::cout << '\n';

    // deque
    {
        std::deque<int> dequeue{ std::begin(sourceArr), std::end(sourceArr) };
        Print(dequeue, "dequeue");

        // dequeue도 vector처럼 remove() 한 다음 erase()를 호출해야 한다.
        auto rmvRange = std::ranges::remove(dequeue, 2); // ranges 버전의 remove()는 subrange를 반환함.

        Print(dequeue, "after remove()");
        Print(rmvRange, "returned by remove()");

        // 실질적인 제거 구문
        dequeue.erase(rmvRange.begin(), rmvRange.end());

        Print(dequeue, "after erase()");
    }

    std::cout << '\n';

    // set
    {
        std::set<int> set{ std::begin(sourceArr), std::end(sourceArr) };
        Print(set, "set");

        // set 계열의 자료형은 vector와는 달리 erase()에서 key 값을 받는다.
        set.erase(2);
        Print(set, "after erase()");
    }

    std::cout << '\n';

    // multiset
    {
        std::multiset<int> multiSet{ std::begin(sourceArr), std::end(sourceArr) };
        Print(multiSet, "multiset");

        // set 계열의 자료형은 vector와는 달리 erase()에서 key 값을 받는다.
        multiSet.erase(2);
        Print(multiSet, "after erase()");
    }

    std::cout << '\n';

    // unordered_set
    {
        std::unordered_set<int> unorderedSet{ std::begin(sourceArr), std::end(sourceArr) };
        Print(unorderedSet, "unordered_set");

        // set 계열의 자료형은 vector와는 달리 erase()에서 key 값을 받는다.
        unorderedSet.erase(2);
        Print(unorderedSet, "after erase()");
    }

    std::cout << '\n';

    // unordered_multiset
    {
        std::unordered_multiset<int> unorderedMultiSet{ std::begin(sourceArr), std::end(sourceArr) };
        Print(unorderedMultiSet, "unordered_multiset");

        // set 계열의 자료형은 vector와는 달리 erase()에서 key 값을 받는다.
        unorderedMultiSet.erase(2);
        Print(unorderedMultiSet, "after erase()");
    }
}

END_NS

BEGIN_NS(Case02)

// https://en.cppreference.com/w/cpp/container/vector/erase2
// https://en.cppreference.com/w/cpp/container/list/erase2
// https://en.cppreference.com/w/cpp/container/forward_list/erase2
// https://en.cppreference.com/w/cpp/container/deque/erase2
// https://en.cppreference.com/w/cpp/container/set/erase_if
// https://en.cppreference.com/w/cpp/container/multiset/erase_if
// https://en.cppreference.com/w/cpp/container/unordered_set/erase_if
// https://en.cppreference.com/w/cpp/container/unordered_multiset/erase_if

// C++20에는 전역 erase()를 제공하는데 이를 통해 일관된 인터페이스대로 코드를 작성하여 특정 값을 제거하는 것이 가능하다.
// 전역 erase()를 사용하면 이따금 사용되는 remove() -> erase() 작업을 거치지 않아도 된다.
//
// set 계열의 자료형은 자체적인 erase()를 가져서 그런지 전역 erase()를 지원하지 않는다.
// 하지만 전역 erase_if()는 사용할 수 있기 때문에 이를 통해 인터페이스를 맞추면 된다.

void Run()
{
    std::array sourceArr{ 1, 2, 3, 2, 2, 4, 2, 7, 2, 9 };
    Print(sourceArr, "source array");

    std::cout << '\n';

    // vector
    {
        std::vector<int> vec{ std::begin(sourceArr), std::end(sourceArr) };
        Print(vec, "vector");

        // // vector 기반의 erase()는 iterator 기반으로 동작하기 때문에 remove()를 작업이 선행되어야 한다.
        // auto rmvRange = std::ranges::remove(vec, 2); // ranges 버전의 remove()는 subrange를 반환함.
        // 
        // Print(vec, "after remove()");
        // Print(rmvRange, "returned by remove()");
        // 
        // // vector 기반에서 특정 값을 제거하고자 할 때는 혼란스럽긴 해도 이렇게 진행해야 한다.
        // vec.erase(rmvRange.begin(), rmvRange.end());

        std::erase(vec, 2); // 제거한 값의 개수를 반환함.
        
        Print(vec, "after erase()");
    }

    std::cout << '\n';

    // list
    {
        std::list<int> list{ std::begin(sourceArr), std::end(sourceArr) };
        Print(list, "list");

        // // list는 remove()를 통해 특정 값을 제거할 수 있는 기능을 제공한다.
        // list.remove(2);
        // 
        // Print(list, "after remove()");

        std::erase(list, 2); // 제거한 값의 개수를 반환함.

        Print(list, "after erase()");
    }

    std::cout << '\n';

    // forward_list
    {
        std::forward_list<int> forwardList{ std::begin(sourceArr), std::end(sourceArr) };
        Print(forwardList, "forward_list");

        // // forward_list도 list와 동일하게 remove()를 통해 특정 값을 제거할 수 있다.
        // forwardList.remove(2);
        // 
        // Print(forwardList, "after remove()");

        std::erase(forwardList, 2); // 제거한 값의 개수를 반환함.

        Print(forwardList, "after erase()");
    }

    std::cout << '\n';

    // deque
    {
        std::deque<int> dequeue{ std::begin(sourceArr), std::end(sourceArr) };
        Print(dequeue, "dequeue");

        // // dequeue도 vector처럼 remove() 한 다음 erase()를 호출해야 한다.
        // auto rmvRange = std::ranges::remove(dequeue, 2); // ranges 버전의 remove()는 subrange를 반환함.
        // 
        // Print(dequeue, "after remove()");
        // Print(rmvRange, "returned by remove()");
        // 
        // // 실질적인 제거 구문
        // dequeue.erase(rmvRange.begin(), rmvRange.end());

        std::erase(dequeue, 2); // 제거한 값의 개수를 반환함.

        Print(dequeue, "after erase()");
    }

    std::cout << '\n';

    // set
    {
        std::set<int> set{ std::begin(sourceArr), std::end(sourceArr) };
        Print(set, "set");

        // // set 계열의 자료형은 vector와는 달리 erase()에서 key 값을 받는다.
        // set.erase(2);
        // Print(set, "after erase()");

        // set 계열의 자료형은 전역 erase()는 사용할 수 없지만 전역 erase_if()는 사용할 수 있다.
        std::erase_if(set, [](int elem) { return elem == 2; });

        Print(set, "after erase_if()");
    }

    std::cout << '\n';

    // multiset
    {
        std::multiset<int> multiSet{ std::begin(sourceArr), std::end(sourceArr) };
        Print(multiSet, "multiset");

        // // set 계열의 자료형은 vector와는 달리 erase()에서 key 값을 받는다.
        // multiSet.erase(2);
        // Print(multiSet, "after erase()");

        // set 계열의 자료형은 전역 erase()는 사용할 수 없지만 전역 erase_if()는 사용할 수 있다.
        std::erase_if(multiSet, [](int elem) { return elem == 2; });

        Print(multiSet, "after erase_if()");
    }

    std::cout << '\n';

    // unordered_set
    {
        std::unordered_set<int> unorderedSet{ std::begin(sourceArr), std::end(sourceArr) };
        Print(unorderedSet, "unordered_set");

        // // set 계열의 자료형은 vector와는 달리 erase()에서 key 값을 받는다.
        // unorderedSet.erase(2);
        // Print(unorderedSet, "after erase()");

        // set 계열의 자료형은 전역 erase()는 사용할 수 없지만 전역 erase_if()는 사용할 수 있다.
        std::erase_if(unorderedSet, [](int elem) { return elem == 2; });

        Print(unorderedSet, "after erase_if()");
    }

    std::cout << '\n';

    // unordered_multiset
    {
        std::unordered_multiset<int> unorderedMultiSet{ std::begin(sourceArr), std::end(sourceArr) };
        Print(unorderedMultiSet, "unordered_multiset");

        // // set 계열의 자료형은 vector와는 달리 erase()에서 key 값을 받는다.
        // unorderedMultiSet.erase(2);
        // Print(unorderedMultiSet, "after erase()");

        // set 계열의 자료형은 전역 erase()는 사용할 수 없지만 전역 erase_if()는 사용할 수 있다.
        std::erase_if(unorderedMultiSet, [](int elem) { return elem == 2; });

        Print(unorderedMultiSet, "after erase_if()");
    }
}

END_NS

int main()
{
    // Case01::Run();
    Case02::Run();

    return 0;
}
