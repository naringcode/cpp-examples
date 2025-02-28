// Update Date : 2025-03-01
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <format>
#include <compare>
#include <set>

// 단계별 학습을 위해서라도 다음 순서대로 보도록 하자.
// 
// # C++20 비교 연산자 개요(Three-way 비교 연산자를 위주로 다룸)
// 1. c++20_comparison_operators_intro.txt
// 
// # C++20 이전의 비교 연산
// 2. user_defined_comparison_operators_before_c++20.cpp
// 
// # C++20 이후의 비교 연산
// 3. three-way_comparison_operators_on_standard_types.cpp
// 4. default_three-way_comparison_operators.cpp
// 5. equality_operators_after_c++20.cpp
// 6. lexicographical_equality_comparisons.cpp
// 7. precautions_on_three-way_comparison_operators.cpp
// 
// # operator<=> 반환 타입
// 8. return_type_strong_ordering.cpp
// 9. return_type_weak_ordering.cpp
// 10. return_type_partial_ordering.cpp
// 11. changing_ordering_category.cpp
// 
// # default로 정의한 Three-way 비교 연산자를 기반으로 동작하는 객체의 부모 클래스와 멤버 객체 간 관계 연산
// 12. default_three-way_rel_ops_on_inheritance_and_member_objects.cpp
// 
// # Furthermore
// 13. sorting_by_user_defined_three-way_comparison_operators.cpp
// 14. compare_fallback_funcs_to_synthesize_three-way_comp.cpp
// 15. lexicographical_compare_three_way_on_static_array.cpp <-----
// 
// # 응용하기
// 16. implementation_of_case_insensitive_string.cpp
//

// https://en.cppreference.com/w/cpp/algorithm/lexicographical_compare_three_way

// 해당 함수는 std::string이 아닌 일반 문자 배열(char*, char[])을 대상으로 사전 순서 비교를 진행하고 싶을 경우 사용하면 된다.
// 
// 해당 함수가 반환하는 결과는 다음과 동등(equivalent)하다고 되어 있다.
// 
// return std::lexicographical_compare_three_way(
//     first1, last1, first2, last2, std::compare_three_way());
// 
// 여기서 std::compare_three_way는 operator<=>를 사용하는 함수 객체이다.
// 즉, ordering 타입을 반환하는 람다식이나 함수 객체를 직접 구성할 수 있다면 
// std::lexicographical_compare_three_way()의 동작 방식을 사용자가 제어할 수 있다는 뜻이다.

struct Person
{
    char name[16];
    int  age{ };
    char gender{ };

    // gender -> name -> age 순으로 비교한다.
    auto operator<=>(const Person& rhs) const
    {
        auto genOrder = gender <=> rhs.gender;
        if (genOrder != 0)
            return genOrder;

        // 정적 배열을 대상으로 사용할 때는 std::begin()과 std::end()를 쓰도록 한다.
        // std::lexicographical_compare_three_way()는 std::strong_ordering를 반환한다.
        auto nameOrder = std::lexicographical_compare_three_way(std::begin(name), std::end(name),
                                                                std::begin(rhs.name), std::end(rhs.name));
        if (nameOrder != 0)
            return nameOrder;

        return age <=> rhs.age;
    }
};

int main()
{
    std::set<Person> people{ };

    people.insert({ "Daniel",  23, 'M' });
    people.insert({ "Chloe",   16, 'F' });
    people.insert({ "Ethan",   23, 'M' });
    people.insert({ "Brian",   23, 'M' });
    people.insert({ "Alex",    16, 'M' });
    people.insert({ "Fiona",   30, 'F' });
    people.insert({ "Bella",   23, 'F' });
    people.insert({ "Diana",   16, 'F' });
    people.insert({ "Alice",   30, 'F' });
    people.insert({ "Frank",   28, 'M' });
    people.insert({ "Ella",    28, 'F' });
    people.insert({ "Charlie", 30, 'M' });

    for (auto& [name, age, gender] : people)
    {
        std::cout << std::format("{:<8} {:<3} {}\n", name, age, gender);
    }

    return 0;
}
