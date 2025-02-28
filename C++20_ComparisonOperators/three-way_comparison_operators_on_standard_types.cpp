// Update Date : 2025-02-25
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <string>
#include <vector>
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
// 3. three-way_comparison_operators_on_standard_types.cpp <-----
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
// 15. lexicographical_compare_three_way_on_static_array.cpp
// 
// # 응용하기
// 16. implementation_of_case_insensitive_string.cpp
//

// https://en.wikipedia.org/wiki/Three-way_comparison
// https://en.cppreference.com/w/cpp/language/operator_comparison#Three-way_comparison
// https://en.cppreference.com/w/cpp/utility#Three-way_comparison

// C++20부터는 Three-way comparison operator를 사용할 수 있다.
// 이를 통해서 손쉽게 관계 연산자와 동등 연산자를 적용하는 것이 가능하다.

// C++에서 제공하는 표준 타입은 특별한 이유가 없다면 Three-way 비교 연산자를 사용할 수 있다.

#define BEGIN_NS(name) namespace name {
#define END_NS }

BEGIN_NS(Case01)

// 비교 연산자 대신 operator<=>를 통한 비교

void Run()
{
    int x = 10;
    int y = 20;

    std::cout << "x > y  : " << ((x <=> y) > 0) << '\n';  // 0
    std::cout << "x >= y : " << ((x <=> y) >= 0) << '\n'; // 0
    std::cout << "x < y  : " << ((x <=> y) < 0) << '\n';  // 1
    std::cout << "x <= y : " << ((x <=> y) <= 0) << '\n'; // 1
    std::cout << "x == y : " << ((x <=> y) == 0) << '\n'; // 0
    std::cout << "x != y : " << ((x <=> y) != 0) << '\n'; // 1

    // operator<=>가 반환하는 값은 리터럴 0으로만 비교할 수 있다.
    // 다음과 같이 0 값을 가진 변수를 대상으로 비교 연산을 진행하려고 하면 에러가 발생한다.
    int  zeroVal = 0;
    auto threeWayRet = x <=> y;

    // error C7595: 'std::_Literal_zero::_Literal_zero': 직접 함수에 대한 호출이 상수 식이 아닙니다.
    // std::cout << "x > y : " << ((x <=> y) > zeroVal);
}

END_NS

BEGIN_NS(Case02)

// 표준 컨테이너의 경우 operator<=>가 정의되어 있어 <=> 연산자를 사용할 수 있다.
// 이 경우에는 사전 순서(lexicographical)에 따라서 데이터를 비교한다.
// 사전 순으로 보았을 때 앞서는 값이 작은 거고, 나중에 오는 값이 큰 것이다.
// 
// (중요) C++20 이후 표준 컨테이너를 대상으로 관계 연산자(Relational operators)를 사용하면 컴파일러는 <=> 형식에 맞게 코드를 재배치한다(정해진 코드 재배치 규칙이 존재함).
// 디스어셈블러로 조회하면 관계 연산자 대신 <=> 연산자가 적용된 것을 볼 수 있다.
// 
// (주의) 관계 연산자(<, >, <=, >=)는 operator<=> 기반으로 코드를 컴파일하지만 아닌 동등 연산자(==, !=)의 경우에는 operator==를 따르니 주의해야 한다.

void Run()
{
    std::string str1 = "Hello";
    std::string str2 = "World";

    std::vector<int> vec1{ 1, 2, 3, 4, 5 };
    std::vector<int> vec2{ 1, 2, 0 };
    std::vector<int> vec3{ 1, 2, 9 };

    std::set<int> set1{ 6, 5, 4 }; // 정렬되어 4, 5, 6 순으로 값이 들어감.
    std::set<int> set2{ 0 };
    std::set<int> set3{ 9 };

    // 사전 순서로 보면 H가 W보다 사전 순서로 보았을 때 앞선다.
    // 따라서 Hello보다 World가 더 크다.
    // 
    // 디스어셈블러로 관계 연산자가 적용된 것을 조회하면 operator<=>가 적용된 것을 확인할 수 있다.
    // 각 줄에 표현된 2개의 관계 구문은 컴파일되면 같은 방식으로 동작한다.
    std::cout << "str1 > str2  : " << (str1 > str2)  << ", " << ((str1 <=> str2) > 0) << '\n';  // 0  
    std::cout << "str1 >= str2 : " << (str1 >= str2) << ", " << ((str1 <=> str2) >= 0) << '\n'; // 0
    std::cout << "str1 < str2  : " << (str1 < str2)  << ", " << ((str1 <=> str2) < 0) << '\n';  // 1
    std::cout << "str1 <= str2 : " << (str1 <= str2) << ", " << ((str1 <=> str2) <= 0) << '\n'; // 1

    // 동등 작업의 경우에는 operator==을 기반으로 동작하며, 수동으로 operator<=>를 사용하는 것도 가능하다.
    // 각 줄에 표현된 2개의 관계 구문은 다른 방식으로 동작한다.
    std::cout << "str1 == str2 : " << (str1 == str2) << ", " << ((str1 <=> str2) == 0) << '\n'; // 0
    std::cout << "str1 != str2 : " << (str1 != str2) << ", " << ((str1 <=> str2) != 0) << '\n'; // 1

    std::cout << "-------------------------\n";

    // 3번째 인덱스를 보면 0은 3보다 앞선다(사전 순으로 보았을 때 앞선 게 작은 값).
    std::cout << "vec1 > vec2  : " << (vec1 > vec2)  << ", " << ((vec1 <=> vec2) > 0) << '\n';  // 1
    std::cout << "vec1 >= vec2 : " << (vec1 >= vec2) << ", " << ((vec1 <=> vec2) >= 0) << '\n'; // 1
    std::cout << "vec1 < vec2  : " << (vec1 < vec2)  << ", " << ((vec1 <=> vec2) < 0) << '\n';  // 0
    std::cout << "vec1 <= vec2 : " << (vec1 <= vec2) << ", " << ((vec1 <=> vec2) <= 0) << '\n'; // 0

    std::cout << "vec1 == vec2 : " << (vec1 == vec2) << ", " << ((vec1 <=> vec2) == 0) << '\n'; // 0
    std::cout << "vec1 != vec2 : " << (vec1 != vec2) << ", " << ((vec1 <=> vec2) != 0) << '\n'; // 1

    std::cout << '\n';

    // 3번째 인덱스를 보면 3은 9보다 앞선다(사전 순으로 보았을 때 앞선 게 작은 값).
    std::cout << "vec1 > vec3  : " << (vec1 > vec3)  << ", " << ((vec1 <=> vec3) > 0) << '\n';  // 0
    std::cout << "vec1 >= vec3 : " << (vec1 >= vec3) << ", " << ((vec1 <=> vec3) >= 0) << '\n'; // 0
    std::cout << "vec1 < vec3  : " << (vec1 < vec3)  << ", " << ((vec1 <=> vec3) < 0) << '\n';  // 1
    std::cout << "vec1 <= vec3 : " << (vec1 <= vec3) << ", " << ((vec1 <=> vec3) <= 0) << '\n'; // 1

    std::cout << "vec1 == vec3 : " << (vec1 == vec3) << ", " << ((vec1 <=> vec3) == 0) << '\n'; // 0
    std::cout << "vec1 != vec3 : " << (vec1 != vec3) << ", " << ((vec1 <=> vec3) != 0) << '\n'; // 1

    std::cout << "-------------------------\n";

    // set은 데이터를 정렬하여 저장하기에 첫 요소부터 비교를 진행해야 한다.
    // 첫 요소인 4와 0을 비교하면 4가 더 크며, 이는 0이 4보다 사전 순으로 봤을 때 앞선다는 걸 의미한다(사전 순으로 앞선 게 작은 값).
    std::cout << "set1 > set2  : " << (set1 > set2)  << ", " << ((set1 <=> set2) > 0) << '\n';  // 1
    std::cout << "set1 >= set2 : " << (set1 >= set2) << ", " << ((set1 <=> set2) >= 0) << '\n'; // 1
    std::cout << "set1 < set2  : " << (set1 < set2)  << ", " << ((set1 <=> set2) < 0) << '\n';  // 0
    std::cout << "set1 <= set2 : " << (set1 <= set2) << ", " << ((set1 <=> set2) <= 0) << '\n'; // 0

    std::cout << "set1 == set2 : " << (set1 == set2) << ", " << ((set1 <=> set2) == 0) << '\n'; // 0
    std::cout << "set1 != set2 : " << (set1 != set2) << ", " << ((set1 <=> set2) != 0) << '\n'; // 1

    std::cout << '\n';

    // set은 데이터를 정렬하여 저장하기에 첫 요소부터 비교를 진행해야 한다.
    // 첫 요소인 4와 9를 비교하면 9가 더 크며, 이는 4가 9보다 사준 선으로 봤을 때 앞선다는 걸 의미한다(사전 순으로 앞선 게 작은 값).
    std::cout << "set1 > set3  : " << (set1 > set3)  << ", " << ((set1 <=> set3) > 0) << '\n';  // 0
    std::cout << "set1 >= set3 : " << (set1 >= set3) << ", " << ((set1 <=> set3) >= 0) << '\n'; // 0
    std::cout << "set1 < set3  : " << (set1 < set3)  << ", " << ((set1 <=> set3) < 0) << '\n';  // 1
    std::cout << "set1 <= set3 : " << (set1 <= set3) << ", " << ((set1 <=> set3) <= 0) << '\n'; // 1

    std::cout << "set1 == set3 : " << (set1 == set3) << ", " << ((set1 <=> set3) == 0) << '\n'; // 0
    std::cout << "set1 != set3 : " << (set1 != set3) << ", " << ((set1 <=> set3) != 0) << '\n'; // 1

    // 반드시 디스어셈블러로 관계 연산자와 동등 연산자의 동작 방식을 확인할 것!
}

END_NS

BEGIN_NS(Case03)

// 일반 배열에 직접적으로 <=>를 적용하는 건 불가능하다.
// (주의) 객체에 적용하는 default operator<=>의 경우 member-wise 비교를 진행하는데 이 경우에는 일반 배열의 비교를 허용하니 혼동하면 안 된다.

void Run()
{
    int arr1[]{ 1, 2, 3, 4, 5 };
    int arr2[]{ 1, 2, 3, 4, 9 };

    // 컴파일러 에러 발생
    // std::cout << "arr1 > arr2  : " << ((arr1 <=> arr2) > 0) << '\n';
}

END_NS

int main()
{
    // Case01::Run();
    Case02::Run();
    // Case03::Run();

    return 0;
}
