// Update Date : 2025-02-14
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <format>
#include <random>
#include <algorithm>
#include <ranges>
#include <vector>

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
// 5. return_types_by_range_algorithms.cpp <-----
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

// https://en.cppreference.com/w/cpp/algorithm/ranges#Return_types
// https://en.cppreference.com/w/cpp/algorithm/ranges/return_types/in_fun_result
// https://en.cppreference.com/w/cpp/algorithm/ranges/return_types/in_out_result

#define BEGIN_NS(name) namespace name {
#define END_NS };

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

struct TwiceAndBackupOddNums
{
    std::vector<int> oddNums;

    void operator()(int& elem)
    {
        if (elem % 2 != 0)
        {
            oddNums.push_back(elem);
        }

        elem *= 2;
    }
};

BEGIN_NS(Case01)

// 기존 버전의 std::for_each()는 Fn을 반환한다.

void Run()
{
    std::vector<int> data(10);
    std::ranges::generate(data, []() { return std::random_device{ }() % 10; });

    std::ranges::sort(data);
    Print(data, "Data");

    // twice all numbers and backup odd numbers(use legacy for_each())
    auto backupOddNums = std::for_each(data.begin(), data.end(), TwiceAndBackupOddNums{ });

    Print(data, "Twice");
    Print(backupOddNums.oddNums, "Backup OddNums");
}

END_NS

BEGIN_NS(Case02)

// Ranges 버전의 std::ranges::for_each()는 Fn와 iterator를 묶어서 반환한다.
// - in_fun_result, for_each_result

void Run()
{
    std::vector<int> data(10);
    std::ranges::generate(data, []() { return std::random_device{ }() % 10; });
    
    std::ranges::sort(data);
    Print(data, "Data");

    // twice all numbers(use ranges version for_each())

    // in_fun_result로 받는 방식(기본)
    // std::ranges::in_fun_result<std::vector<int>::iterator, TwiceAndBackupOddNums> result
    //     = std::ranges::for_each(data, TwiceAndBackupOddNums{ });

    // in_fun_result로 받는 방식(템플릿 인자 추론)
    // std::ranges::in_fun_result result = std::ranges::for_each(data, TwiceAndBackupOddNums{ });

    // in_fun_result로 받는 방식(auto를 통한 타입 추론)
    // auto result = std::ranges::for_each(data, TwiceAndBackupOddNums{ });

    // in_fun_result를 ranges::for_each()의 반환에 대응되는 별칭으로 받는 방식
    // for_each_result<I, F>의 템플릿 인자는 컴파일 타임에 추론된다.
    // std::ranges::for_each_result result = std::ranges::for_each(data, TwiceAndBackupOddNums{ });
    
    // structured binding을 통한 직접적인 값 추출
    auto [iter, fnObj] = std::ranges::for_each(data, TwiceAndBackupOddNums{ });

    Print(data, "Twice");

    // Print(result.fun.oddNums, "Backup OddNums");
    Print(fnObj.oddNums, "Backup OddNums"); // fnObj는 사용한 Functor이며 마찬가지로 이동 생성자로 받아옴.

    // for_each()는 모든 요소를 순회하기 때문에 iter는 end를 가리키고 있어야 한다.
    // if (result.in == data.end())
    if (iter == data.end())
    {
        std::cout << "iter reached to the end\n";
    }
    else
    {
        std::cout << "iter did not reach to the end\n";
    }
}

END_NS

BEGIN_NS(Case03)

// 기존 버전의 std::for_each()는 마지막으로 copy한 element의 iterator를 반환한다.

void Run()
{
    std::vector<int> data(10);
    std::ranges::generate(data, []() { return std::random_device{ }() % 10; });

    std::ranges::sort(data);
    Print(data, "Data");

    // 실제 개발할 때는 크기를 지정해서 컨테이너를 할당하는 것보다 std::back_inserter()를 쓰는 것이 안전하다.
    std::vector<int> copies(10); // capacity가 아닌 size임.

    copies.push_back(100);
    copies.push_back(200);
    copies.push_back(300);

    Print(copies, "Before copy");
    std::cout << '\n';

    // 홀수 값만 복사(copies.begin() 대신 std::back_inserter(copies)를 쓰는 방법도 있음)
    auto iter = std::copy_if(data.begin(), data.end(), copies.begin(),
                             [](const int& elem) {
                                 return elem % 2 != 0;
                             });

    // 마지막으로 copy한 element의 다음 iterator를 가리킨다.
    // 따라서 실제로 작업할 때는 유효할 수도 있고 유효하지 않을 수도 있다.
    if (iter != copies.end())
    {
        std::cout << "Distance from begin : " << std::distance(copies.begin(), iter) << '\n';

        std::cout << "Current iter value  : " << *iter << '\n';
        std::cout << "Previous iter value : " << *(iter - 1) << '\n';

        std::cout << '\n';
    }

    // copies의 앞 부분부터 덮어쓰며 값을 갱신한 것을 볼 수 있다.
    Print(copies, "After copy");
}

END_NS

BEGIN_NS(Case04)

// Ranges 버전의 std::ranges::for_each()는 Fn와 iterator를 묶어서 반환한다.
// - in_out_result, copy_if_result

void Run()
{
    std::vector<int> data(10);
    std::ranges::generate(data, []() { return std::random_device{ }() % 10; });

    std::ranges::sort(data);
    Print(data, "Data");

    // 실제 개발할 때는 크기를 지정해서 컨테이너를 할당하는 것보다 std::back_inserter()를 쓰는 것이 안전하다.
    std::vector<int> copies(10); // capacity가 아닌 size임.

    copies.push_back(100);
    copies.push_back(200);
    copies.push_back(300);

    Print(copies, "Before copy");
    std::cout << '\n';

    // 홀수 값만 복사(copies.begin() 대신 std::back_inserter(copies)를 쓰는 방법도 있음)
    // in  : 복사 원본이 되는 컨테이너의 반복자로 끝을 가리킴(여기선 data).
    // out : 복사 대상이 되는 컨테이너의 반복자로 마지막으로 copy한 element의 다음 iterator를 가리킴(여기선 copies).
    auto [in, out] = std::ranges::copy_if(data, copies.begin(),
                                          [](const int& elem) {
                                              return elem % 2 != 0;
                                          });

    // in은 원본 컨테이너의 끝을 가리키는 반복자이다.
    if (in == data.end())
    {
        std::cout << "inIter reached to the end\n";
        std::cout << '\n';
    }

    // out은 마지막으로 copy한 element의 다음 iterator를 가리킨다.
    // 따라서 실제로 작업할 때는 유효할 수도 있고 유효하지 않을 수도 있다.
    if (out != copies.end())
    {
        std::cout << "Distance from begin : " << std::distance(copies.begin(), out) << '\n';

        std::cout << "Current outIter value  : " << *out << '\n';
        std::cout << "Previous outIter value : " << *(out - 1) << '\n';

        std::cout << '\n';
    }

    // copies의 앞 부분부터 덮어쓰며 값을 갱신한 것을 볼 수 있다.
    Print(copies, "After copy");
}

END_NS

int main()
{
    // Case01::Run();
    // Case02::Run();
    // Case03::Run();
    Case04::Run();

    return 0;
}
