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
// 13. sorting_by_user_defined_three-way_comparison_operators.cpp <-----
// 14. compare_fallback_funcs_to_synthesize_three-way_comp.cpp
// 15. lexicographical_compare_three_way_on_static_array.cpp
// 
// # 응용하기
// 16. implementation_of_case_insensitive_string.cpp
//

// operator<=>를 수동으로 작성할 때 멤버 변수를 비교하는 순서에 따라 정렬 순서를 조절할 수 있다.
struct Person
{
    std::string name{ };
    int  age{ };
    char gender{ };

    // default 유형으로 사용하면 Member-wise 방향에 따라 name -> age -> gender 순으로 비교한다.
    // auto operator<=>(const Person& rhs) const = default;

    // 직접 구성하는 방식을 사용하면 age -> gender -> name 순으로 비교하여 반환된 ordering을 통해 정렬 순서를 결정할 수 있다.
    auto operator<=>(const Person& rhs) const
    {
        // std::strong_ordering ageOrder{ };
        // if (age == rhs.age)
        // {
        //     ageOrder = std::strong_ordering::equal;
        // }
        // else if (age < rhs.age)
        // {
        //     ageOrder = std::strong_ordering::less;
        // }
        // else
        // {
        //     ageOrder = std::strong_ordering::greater;
        // }
        // 
        // if (ageOrder != 0)
        //     return ageOrder;

        // 위에 주석으로 작성된 코드와 같은 기능을 수행한다.
        auto ageOrder = age <=> rhs.age;
        if (ageOrder != 0)
            return ageOrder;

        auto genOrder = gender <=> rhs.gender;
        if (genOrder != 0)
            return genOrder;

        return name <=> rhs.name;
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
