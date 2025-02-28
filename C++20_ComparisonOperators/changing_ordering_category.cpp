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
// 11. changing_ordering_category.cpp <-----
// 
// # default로 정의한 Three-way 비교 연산자를 기반으로 동작하는 객체의 부모 클래스와 멤버 객체 간 관계 연산
// 12. default_three-way_rel_ops_on_inheritance_and_member_objects.cpp
// 
// # Furthermore
// 13. sorting_by_user_defined_three-way_comparison_operators.cpp
// 14. compare_fallback_funcs_to_synthesize_three-way_comp.cpp
// 15. lexicographical_compare_three_way_on_static_array.cpp
// 
// # 응용하기
// 16. implementation_of_case_insensitive_string.cpp
//

// https://en.cppreference.com/w/cpp/utility/compare/strong_order
// https://en.cppreference.com/w/cpp/utility/compare/weak_order
// https://en.cppreference.com/w/cpp/utility/compare/partial_ordering

// operator<=>가 반환하는 3가지 타입을 암묵적으로 사용할 때 하위 타입에서 상위 타입으로 거슬러 올라가는 건 불가능하다.
// - std::strong_ordering -> std::weak_ordering -> std::partial_ordering (역방향으로의 암묵적인 형변환은 허용하지 않음)
// 
// 하지만 경우에 따라선 알고리즘이나 컨테이너가 요구하는 사항에 맞춰 특정 ordering 타입을 반환해야 할 때가 있다.
// 이러한 상황은 자주 등장하기 때문에 C++20은 카테고리 변환을 위한 함수 객체를 제공한다(함수 아님).
// - std::strong_order
// - std::weak_order
// - std::partial_order
//

struct Item
{
    int   integral{ };
    float floatingPoint{ };

    // 다음 함수는 타입을 제대로 추론할 수 없어 에러가 발생한다.
    // E2546 : 추론된 반환 형식 "std::partial_ordering"이(가) 이전에 추론된 형식 "std::strong_ordering"과(와) 충돌합니다.
    // auto operator<=>(const Item& rhs) const
    // {
    //     auto intOrder = integral <=> rhs.integral;
    //     if (intOrder != 0)
    //         return intOrder;
    // 
    //     return floatingPoint <=> rhs.floatingPoint;
    // }

    // 반환형을 auto 대신 전환 가능한 하위 타입인 std::partial_ordering으로 명시하면 통과하긴 한다.
    // std::partial_ordering operator<=>(const Item& rhs) const
    // {
    //     auto intOrder = integral <=> rhs.integral;
    //     if (intOrder != 0)
    //         return intOrder;
    // 
    //     return floatingPoint <=> rhs.floatingPoint;
    // }

    // 하지만 역으로 거슬러 올라가려면 적절한 변환 함수 객체를 사용하는 것이 좋다.
    std::strong_ordering operator<=>(const Item& rhs) const
    {
        auto intOrder = integral <=> rhs.integral;
        if (intOrder != 0)
            return intOrder;
    
        // 이렇게 수동으로 대응되는 ordering의 값에 따라 반환하는 것도 가능하다.
        // 하지만 이 방식은 unordered에 대한 깔끔한 처리를 할 수 없다.
        // auto floatOrder = floatingPoint <=> rhs.floatingPoint;
        // 
        // if (floatOrder < 0)
        //     return std::strong_ordering::less;
        // 
        // if (floatOrder > 0)
        //     return std::strong_ordering::greater;
        // 
        // return std::strong_ordering::equal; // floatOrder == equal
    
        // 하지만 다음과 같이 사용하는 것이 가독성도 좋고 깔끔하다.
        return std::strong_order(floatingPoint, rhs.floatingPoint);
    }
};

int main()
{
    std::set<Item> items{ };

    items.insert({ 50,  3.14f });
    items.insert({ 100, 3.14f });
    items.insert({ 200, 3.14f });
    items.insert({ 50,  1.57f });
    items.insert({ 200, 1.57f });
    items.insert({ 100, 1.57f });
    items.insert({ 200, 6.28f });
    items.insert({ 100, 6.28f });
    items.insert({ 50,  6.28f });

    for (auto& [x, y] : items)
    {
        std::cout << std::format("{:<5} {}\n", x, y);
    }

    return 0;
}
