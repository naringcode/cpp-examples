// Update Date : 2025-02-27
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20, C++23
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <string>
#include <string_view>
#include <compare>
#include <algorithm>
#include <ranges>

// 대소문자를 무시하고 문자의 특성으로만 비교할 수 있는 방법 자체는 다양하다.

#define BEGIN_NS(name) namespace name {
#define END_NS }

BEGIN_NS(Case01)

// 가장 기본이 되는 방법은 모든 요소를 사용자가 순회하는 방식으로 코드를 작성하는 것이다.

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

        return std::weak_ordering::equivalent;
    }

    // operator<=>을 직접 정의했다면 operator==는 자동으로 생성되지 않는다.
    bool operator==(const CaseInsensitiveString& rhs) const
    {
        return this->weakCompare(rhs) == 0;
    }

private:
    int weakCompare(const CaseInsensitiveString& rhs) const
    {
        size_t size = std::min(_str.size(), rhs._str.size());
        
        // 일반적으로 많이 사용하는 방법(사용자 쪽에서 최대한 구현하기)
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

END_NS

BEGIN_NS(Case02)

// https://en.cppreference.com/w/cpp/algorithm/lexicographical_compare_three_way
// std::lexicographical_compare_three_way()를 사용하는 방법

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

        return std::weak_ordering::equivalent;
    }

    // operator<=>을 직접 정의했다면 operator==는 자동으로 생성되지 않는다.
    bool operator==(const CaseInsensitiveString& rhs) const
    {
        return this->weakCompare(rhs) == 0;
    }

private:
    int weakCompare(const CaseInsensitiveString& rhs) const
    {
        auto lhsLowerView = this->_str | std::views::transform(tolower);
        auto rhsLowerView = rhs._str   | std::views::transform(tolower);
        
        // 본래 일반 문자열 배열(char[])를 대상으로 사전 순서 비교를 하여 std::strong_ordering을 받기 위한 함수이다.
        // 정적 배열을 대상으로 사용할 때는 std::begin()과 std::end()를 쓰도록 한다.
        auto threeWayCmp = std::lexicographical_compare_three_way(lhsLowerView.begin(), lhsLowerView.end(),
                                                                  rhsLowerView.begin(), rhsLowerView.end());
    
        int cmp{ };
    
        if (threeWayCmp == std::strong_ordering::equal ||
            threeWayCmp == std::strong_ordering::equivalent)
        {
            cmp = 0;
        }
        else if (threeWayCmp == std::strong_ordering::less)
        {
            cmp = -1;
        }
        else if (threeWayCmp == std::strong_ordering::greater)
        {
            cmp = 1;
        }
    
        return cmp;
    }
    
private:
    std::string _str{ }; // std::string 자체는 std::strong_ordering를 기반으로 동작함.
};

END_NS

BEGIN_NS(Case03)

// https://en.cppreference.com/w/cpp/ranges/zip_view
// std::ranges::zip_view를 통해 한 문자씩 비교하는 방법(C++23)

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

        return std::weak_ordering::equivalent;
    }

    // operator<=>을 직접 정의했다면 operator==는 자동으로 생성되지 않는다.
    bool operator==(const CaseInsensitiveString& rhs) const
    {
        return this->weakCompare(rhs) == 0;
    }

private:
    int weakCompare(const CaseInsensitiveString& rhs) const
    {
        auto lhsLowerView = this->_str | std::views::transform(tolower);
        auto rhsLowerView = rhs._str   | std::views::transform(tolower);
    
        int cmp{ };
    
        // 사전순으로 비교하는 작업을 먼저 진행(lexicographical comparison)
        // std::views::zip()을 사용하면 두 뷰를 묶어서 이터레이팅하는 것이 가능하다.
        // 단순 조회가 아닌 묶음으로 순회하여 변형한 값을 조회할 목적이면 views::zip_transform()을 사용하는 방법도 있다.
        for (auto [ch1, ch2] : std::views::zip(lhsLowerView, rhsLowerView))
        {
            if (ch1 != ch2)
                return ch1 < ch2 ? -1 : +1;
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

END_NS

BEGIN_NS(Case04)

// https://en.cppreference.com/w/cpp/algorithm/ranges/mismatch
// std::ranges::mismatch()를 통해 비교하는 방법

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

        return std::weak_ordering::equivalent;
    }

    // operator<=>을 직접 정의했다면 operator==는 자동으로 생성되지 않는다.
    bool operator==(const CaseInsensitiveString& rhs) const
    {
        return this->weakCompare(rhs) == 0;
    }

private:
    int weakCompare(const CaseInsensitiveString& rhs) const
    {
        auto lhsLowerView = this->_str | std::views::transform(tolower);
        auto rhsLowerView = rhs._str   | std::views::transform(tolower);
    
        // iterator를 통한 직접 비교를 진행하면서 일치하지 않는 부분을 찾기 위해 사용하면 좋은 함수이다.
        // (중요) 문제가 있다면 end에 도달한 iterator를 직접적으로 사용하려고 할 때 프로그램이 터질 수 있다.
        auto [lhsIter, rhsIter] = std::ranges::mismatch(lhsLowerView, rhsLowerView);
    
        size_t lhsDist = std::distance(lhsIter, lhsLowerView.end());
        size_t rhsDist = std::distance(rhsIter, rhsLowerView.end());
    
        if (lhsDist != 0 && rhsDist != 0)
            return *lhsIter < *rhsIter ? -1 : +1;
    
        if (lhsDist == 0 && rhsDist != 0) // less -> 사전의 앞에 위치
            return -1;
    
        if (lhsDist != 0 && rhsDist == 0) // greater -> 사전의 뒤에 위치
            return +1;
    
        return 0;
    }
    
private:
    std::string _str{ }; // std::string 자체는 std::strong_ordering를 기반으로 동작함.
};

END_NS

BEGIN_NS(Case05)

// view의 iterator 기반으로 비교하는 방법

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

        return std::weak_ordering::equivalent;
    }

    // operator<=>을 직접 정의했다면 operator==는 자동으로 생성되지 않는다.
    bool operator==(const CaseInsensitiveString& rhs) const
    {
        return this->weakCompare(rhs) == 0;
    }

private:
    int weakCompare(const CaseInsensitiveString& rhs) const
    {
        auto size = std::min(_str.size(), rhs._str.size());
    
        auto lhsLowerView = this->_str | std::views::transform(tolower);
        auto rhsLowerView = rhs._str   | std::views::transform(tolower);
    
        auto lhsIter = lhsLowerView.begin();
        auto rhsIter = rhsLowerView.begin();
    
        // views를 통해 소문자만 조회하고 이터레이팅은 직접 하는 방식
        while (0 < size--)
        {
            if (*lhsIter != *rhsIter)
                return *lhsIter < *rhsIter ? -1 : +1;
    
            lhsIter++;
            rhsIter++;
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

END_NS

int main()
{
    // namespace Case = Case01;
    // namespace Case = Case02;
    // namespace Case = Case03;
    // namespace Case = Case04;
    namespace Case = Case05;
    
    while (true)
    {
        std::string inStr1;
        std::string inStr2;

        std::getline(std::cin, inStr1);
        std::getline(std::cin, inStr2);

        std::cout << '\n';

        Case::CaseInsensitiveString str1{ inStr1 };
        Case::CaseInsensitiveString str2{ inStr2 };

        std::cout << "str1 : " << inStr1 << '\n';
        std::cout << "str2 : " << inStr2 << '\n';

        std::cout << '\n';

        std::cout << "str1 < str2  : " << (str1 < str2)  << '\n';
        std::cout << "str1 > str2  : " << (str1 > str2)  << '\n';
        std::cout << "str1 <= str2 : " << (str1 <= str2) << '\n';
        std::cout << "str1 >= str2 : " << (str1 >= str2) << '\n';
        std::cout << "str1 == str2 : " << (str1 == str2) << '\n';
        std::cout << "str1 != str2 : " << (str1 != str2) << '\n';

        std::cout << '\n';
    }

    return 0;
}
