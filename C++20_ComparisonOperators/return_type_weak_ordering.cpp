// Update Date : 2025-02-27
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <string_view>
#include <compare>
#include <algorithm>

// https://en.cppreference.com/w/cpp/utility/compare/weak_ordering
// https://en.wikipedia.org/wiki/Weak_ordering
// https://en.wikipedia.org/wiki/Equivalence_relation
// https://en.wikipedia.org/wiki/Equivalence_class

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
// 9. return_type_weak_ordering.cpp <-----
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

// std::weak_ordering에 대한 수학적인 개념은 "c++20_comparison_operators_intro.txt"의 "@ std::weak_ordering" 쪽을 참고하도록 한다.

#define BEGIN_NS(name) namespace name {
#define END_NS }

BEGIN_NS(Case01)

// C++의 기본 타입 중에는 std::weak_ordering 방식으로 동작하는 것이 없다(C++20 기준).
// 따라서 이러한 유형의 ordering 타입을 테스트하려면 직접 타입을 정의해야 한다.
//
// 대소문자를 무시하는 문자열의 경우 동일성(equality)은 만족하지 않지만 동등성(equivalence)은 만족한다고 볼 수 있다.

class CaseInsensitiveString
{
public:
    CaseInsensitiveString(std::string_view str = "")
        : _str{ str }
    { }

public:
    std::weak_ordering operator<=>(const CaseInsensitiveString& rhs) const
    {
        int cmp = this->weakCompare(rhs);

        if (cmp < 0)
            return std::weak_ordering::less; // result < 0 : 사전 앞에 위치

        if (cmp > 0)
            return std::weak_ordering::greater; // result > 0 : 사전 뒤에 위치

        // 대소문자를 구분하지 않고 비교하는 방식을 택했기 때문에 equal 방식은 사용할 수 없다.
        // 무엇보다 std::weak_ordering 방식은 Weak Order 기반으로 동작하기 때문에 완전히 같음을 의미하는 equal을 지원하지 않는다.
        //
        // std::strong_ordering::equal이나 std::weak_ordering::equivalent는 둘 다 0 값이라 리터럴 0과 비교하면 true를 반환한다.
        // 개발할 때 equal과 equivalent는 단순히 사용자 측면에서의 의미론적인 구분일 뿐이다.
        return std::weak_ordering::equivalent;
    }

    // operator<=>을 직접 정의했다면 operator==는 자동으로 생성되지 않는다.
    bool operator==(const CaseInsensitiveString& rhs) const
    {
        return this->weakCompare(rhs) == 0;
    }

private:
    // 해당 함수는 다양한 방식으로 구현할 수 있는데 이에 대한 건 "implementation_of_case_insensitive_string.cpp"를 참고하도록 한다.
    int weakCompare(const CaseInsensitiveString& rhs) const
    {
        size_t size = std::min(_str.size(), rhs._str.size());
        
        for (size_t idx = 0; idx < size; idx++)
        {
            auto lhsCh = std::tolower(_str[idx]);
            auto rhsCh = std::tolower(rhs._str[idx]);

            if (lhsCh != rhsCh)
                return lhsCh < rhsCh ? -1 : +1;
        }
        
        // "Hello"와 "Hel"을 비교한 경우라면?
        // 사전순으로 비교하는 것을 통과해도 아직 모든 비교가 끝난 것은 아니다.
        if (this->_str.size() < rhs._str.size())
            return -1;
        
        if (this->_str.size() > rhs._str.size())
            return +1;
        
        return 0;
    }

private:
    std::string _str{ }; // std::string 자체는 std::strong_ordering를 기반으로 동작함.
};

void Run()
{
    CaseInsensitiveString str1{ "Hello" };
    CaseInsensitiveString str2{ "HELLO" };
    CaseInsensitiveString str3{ "XELLO" };
    CaseInsensitiveString str4{ "aello" };

    std::cout << "ordering : " << typeid(str1 <=> str2).name() << '\n'; // std::weak_ordering

    std::cout << '\n';

    std::cout << "str1 < str2  : " << (str1 < str2)  << '\n'; // 0
    std::cout << "str1 > str2  : " << (str1 > str2)  << '\n'; // 0
    std::cout << "str1 <= str2 : " << (str1 <= str2) << '\n'; // 1
    std::cout << "str1 >= str2 : " << (str1 >= str2) << '\n'; // 1
    std::cout << "str1 == str2 : " << (str1 == str2) << '\n'; // 1
    std::cout << "str1 != str2 : " << (str1 != str2) << '\n'; // 0

    std::cout << '\n';

    std::cout << "str1 < str3  : " << (str1 < str3)  << '\n'; // 1
    std::cout << "str1 > str3  : " << (str1 > str3)  << '\n'; // 0
    std::cout << "str1 <= str3 : " << (str1 <= str3) << '\n'; // 1
    std::cout << "str1 >= str3 : " << (str1 >= str3) << '\n'; // 0
    std::cout << "str1 == str3 : " << (str1 == str3) << '\n'; // 0
    std::cout << "str1 != str3 : " << (str1 != str3) << '\n'; // 1

    std::cout << '\n';

    auto ordering = str1 <=> str4;

    // std::weak_ordering은 다음 3가지 타입 중 하나와 비교할 수 있다.
    if (ordering == std::weak_ordering::less)
    {
        std::cout << "less : str1 < str4\n";
    }

    if (ordering == std::weak_ordering::greater)
    {
        std::cout << "greater : str1 > str4\n"; // <-----
    }

    if (ordering == std::weak_ordering::equivalent)
    {
        std::cout << "equivalent : str1 == str4\n";
    }

    // ordering 타입은 리터럴 0과 비교할 수 있다.
    if (ordering < 0)
    {
        std::cout << "ordering < 0 : str1 < str4\n";
    }

    if (ordering > 0)
    {
        std::cout << "ordering > 0 : str1 > str4\n"; // <-----
    }

    if (ordering == 0)
    {
        std::cout << "ordering == 0 : str1 == str4\n";
    }
}

END_NS

BEGIN_NS(Case02)

// 대소문자를 무시하고 문자 단위로 비교하는 것도 가능하지만 순수하게 길이로 비교하는 것도 std::weak_ordering으로 표현할 수 있다.
// 이 경우에는 모든 문자를 같은 그룹으로 묶어서 취급한다고 보면 된다.

class SizeComparableString
{
public:
    SizeComparableString(std::string_view str = "")
        : _str{ str }
    { }
    
public:
    std::weak_ordering operator<=>(const SizeComparableString& rhs) const
    {
        size_t lhsLen = _str.length();
        size_t rhsLen = rhs._str.length();

        if (lhsLen < rhsLen)
        {
            return std::weak_ordering::less;
        }
        else if (lhsLen > rhsLen)
        {
            return std::weak_ordering::greater;
        }
        else // if (lhsLen == rhsLen)
        {
            return std::weak_ordering::equivalent;
        }
    }

    // operator<=>을 직접 정의했다면 operator==는 자동으로 생성되지 않는다.
    bool operator==(const SizeComparableString& rhs) const
    {
        return _str.length() == rhs._str.length();
    }

private:
    std::string _str{ }; // std::string 자체는 std::strong_ordering를 기반으로 동작함.
};

void Run()
{
    SizeComparableString str1{ "Hello" };
    SizeComparableString str2{ "HELLO" };
    SizeComparableString str3{ "XELL" };
    SizeComparableString str4{ "aellow" };

    std::cout << "ordering : " << typeid(str1 <=> str2).name() << '\n'; // std::weak_ordering

    std::cout << '\n';

    std::cout << "str1 < str2  : " << (str1 < str2)  << '\n'; // 0
    std::cout << "str1 > str2  : " << (str1 > str2)  << '\n'; // 0
    std::cout << "str1 <= str2 : " << (str1 <= str2) << '\n'; // 1
    std::cout << "str1 >= str2 : " << (str1 >= str2) << '\n'; // 1
    std::cout << "str1 == str2 : " << (str1 == str2) << '\n'; // 1
    std::cout << "str1 != str2 : " << (str1 != str2) << '\n'; // 0

    std::cout << '\n';

    std::cout << "str1 < str3  : " << (str1 < str3)  << '\n'; // 0
    std::cout << "str1 > str3  : " << (str1 > str3)  << '\n'; // 1
    std::cout << "str1 <= str3 : " << (str1 <= str3) << '\n'; // 0
    std::cout << "str1 >= str3 : " << (str1 >= str3) << '\n'; // 1
    std::cout << "str1 == str3 : " << (str1 == str3) << '\n'; // 0
    std::cout << "str1 != str3 : " << (str1 != str3) << '\n'; // 1

    std::cout << '\n';

    auto ordering = str1 <=> str4;

    // std::weak_ordering은 다음 3가지 타입 중 하나와 비교할 수 있다.
    if (ordering == std::weak_ordering::less)
    {
        std::cout << "less : str1 < str4\n"; // <-----
    }

    if (ordering == std::weak_ordering::greater)
    {
        std::cout << "greater : str1 > str4\n";
    }

    if (ordering == std::weak_ordering::equivalent)
    {
        std::cout << "equivalent : str1 == str4\n";
    }

    // ordering 타입은 리터럴 0과 비교할 수 있다.
    if (ordering < 0)
    {
        std::cout << "ordering < 0 : str1 < str4\n"; // <-----
    }

    if (ordering > 0)
    {
        std::cout << "ordering > 0 : str1 > str4\n";
    }

    if (ordering == 0)
    {
        std::cout << "ordering == 0 : str1 == str4\n";
    }
}

END_NS

int main()
{
    // Case01::Run();
    Case02::Run();

    return 0;
}
