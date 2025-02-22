// Update Date : 2025-02-23
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <format>
#include <string>
#include <string_view>
#include <functional>
#include <random>
#include <algorithm>
#include <ranges>
#include <numeric>
#include <vector>
#include <array>
#include <span>

// https://en.cppreference.com/w/cpp/container#Views
// https://en.cppreference.com/w/cpp/container/span
// https://en.cppreference.com/w/cpp/container/span/span
// https://hackingcpp.com/cpp/std/span.html

// std::span은 C++20 이후부터 제공하는 자료형으로 경량 참조 타입의 컨테이너 뷰(view)를 말한다.
// 분류상 Containers library에 속하지만 데이터를 소유하지 않기 때문에 엄밀히 말해선 컨테이너는 아니다(a non-owning view).
// 
// std::span은 연속된 메모리 시퀀스의 요소(a contiguous sequence of elements)의 범위만 가리킬 뿐이다.
// 데이터를 소유하지 않는다는 것은 생성 단계에서 참조 대상을 지정해줘야 한다는 뜻이다.
 
// template <class _Ty, size_t _Extent>
// struct _Span_extent_type
// {
//     using pointer = _Ty*;
//     ...
//     pointer _Mydata{nullptr};
//     static constexpr size_t _Mysize = _Extent;
//     ...
// };
// 
// template <class _Ty, size_t _Extent = dynamic_extent>
// class span : private _Span_extent_type<_Ty, _Extent>
// {
//     ...
// };
// 
// sizeof(span<T, N>) == sizeof(T*) + sizeof(size_t) : 보통은 16바이트
//
// std::span은 원본 데이터의 포인터와 크기 두 변수로만 구성되어 있기 때문에 복사 비용이 매우 저렴하다.
// 포인터를 기반으로 동작하기 때문에 std::span이 받는 건 contiguous_range 조건을 충족해야 한다.
// !! std::span<T, N>& 이렇게 참조형으로 전달해도 되지만 가능하면 그냥 값 복사 형식으로 전달하는 게 좋음. !!
//
// std::vector, std::array처럼 연속된 메모리 시퀀스를 가지는 컨테이너 자료형을 받을 수 있지만
// 크기를 알고 있는 정적 원시 배열도 std::span으로 받는 것이 가능하다(높은 안정성).
//
// std::span은 데이터를 복사하는 개념이 아니기 때문에 원본 데이터의 생애주기에 영향을 미치지 않는다.
// (주의) 다만 원본을 참조하는 개념으로 동작하기 때문에 생성되었을 때 사용한 원본이 소멸하면 실행 도중 문제가 발생할 수 있다.

// std::span 자체는 컨테이너가 아니지만 Containers library에 속하는 만큼
// begin(), end()와 같은 이터레이터 함수는 물론 front(), back(), operator[]와 같은 원소 접근 함수도 사용할 수 있다.
// size()와 empty()처럼 상태를 조회하는 함수도 사용할 수 있다.
// 
// 기본적인 컨테이너 함수들은 대부분 지원하지만 cbegin(), cend()처럼 C++23 이후부터 지원하는 기능도 있으니
// 자세한 것은 공식 문서를 참고해서 보도록 하자.

// (중요) std::span은 컴파일 타임에 크기를 결정할 수 있는 static extent와 런타임에 크기를 결정할 수 있는 dynamic extent로 구분할 수 있다.
// 원문 : A span can either have a static extent, in which case the number of elements in the sequence is known at compile-time and encoded in the type, or a dynamic extent.

// 원본을 참조한다는 점에서는 Ranges의 View 기능과 유사하지만 세부적으로 들어가면 둘은 개념적으로 다르다.
// - std::span은 단순히 메모리 뷰일 뿐이지만, views는 네임스페이스로 되어 있는 기능 모음이다.
// - std::span은 연속된 메모리를 가진 대상만 적용할 수 있지만, views는 연속된 메모리 뿐만 아니라 비연속 메모리를 가진 컨테이너를 대상으로도 사용할 수 있다.
// - std::span은 데이터를 가져올 때 원본을 가져오지만, views는 원본을 가공한 데이터를 가져올 수 있다.
// - std::span은 그 즉시 평가되지만, views는 접근할 때 평가되는 lazy evaluation 방식으로 동작한다.
// 
// 무엇보다 std::span은 Containers libray에 속하고, views는 Ranges library에 속한다.

#define BEGIN_NS(name) namespace name {
#define END_NS }

BEGIN_NS(Case01)

// std::span은 정적 크기 방식과 가변 크기 방식이 있다(템플릿 인자가 다르기 때문에 둘은 다른 자료형으로 취급해야 함).
//
// 정적 크기 방식(static extent or fixed extent)
// - std::span<T, N>
// - 배열의 크기를 템플릿 인자로 명시하는 방식(런타임에 크기 변경 불가)
// - 컴파일 타임에 컴파일러가 크기를 알 수 있기 때문에 최적화에 용이
//
// 가변 크기 방식(dynamic extent)
// - std:span<T>
// - 템플릿 인자에 크기를 명시하지 않는 방식으로 크기가 정해지지 않음(std::dynamic_extent가 사용됨).
// - 런타임에 사용할 크기 결정 가능
// - 크기가 런타임에 결정되기 때문에 정적 방식에 비해선 컴파일러 최적화가 잘 이루어지지 않을 수 있음.
//
// 특별한 경우가 아닌 이상 대부분의 경우 가변 크기 방식이 효율적이다.

void SpanWithStaticExtent()
{
    int  staticArr[]{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    int* dynamicArr = new int[] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    std::array  arr{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    std::vector vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    std::string str{ "Hello World!" };

    // std::span은 2개의 템플릿 인자를 받는데 마지막 인자가 특정 값으로 지정되면 static extent가 적용된 span이라 한다.
    // - elements 타입
    // - extent 사이즈
    //
    // (중요) static extent 방식을 사용할 때는 반드시 템플릿 인자로 넣은 크기와 생성자의 인자로 넣은 크기가 일치해야 한다.
    // (중요) 템플릿 인자를 하나라도 기입한 상태에서 두 번째 인자를 명시하지 않는다면 두 번째 인자의 값은 std::dynamic_extent가 적용된다.
    
    /**
     * IDE에서 지원하는 마우스 인스팩터를 통해 타입을 확인해서 볼 것
     */

     // 콜 스택에 생성된 원시 배열은 그냥 인자로 넣기만 해도 타입을 추론할 수 있다.
    {
        std::span sp1{ staticArr }; // std::span<int, 10>

        // std::span sp2{ staticArr, 10 }; // std::span<int, std::dynamic_extent>
        // std::span<int> sp3{ staticArr }; // std::span<int, std::dynamic_extent>

        // std::span<int, 100> sp4{ staticArr, 10 }; // RUNTIME ERROR(count != extent)
        // std::span<int, 5> sp5{ staticArr, 10 }; // RUNTIME ERROR(count != extent)

        std::span<int, 10> sp6{ staticArr, 10 }; // std::span<int, 10>
        std::span<int, 5> sp7{ staticArr, 5 }; // std::span<int, 5>
    }

    // 동적할당한 배열을 가리키는 span에 static extent를 적용하려면 템플릿 인자에 크기를 명시해야 한다.
    {
        // std::span sp1{ dynamicArr }; // COMPILE ERROR
        // std::span<int> sp2{ dynamicArr }; // COMPILE ERROR
        
        // std::span<int> sp3{ dynamicArr, 10 }; // std::span<int, std::dynamic_extent>
         
        // std::span<int, 100> sp4{ dynamicArr, 10 }; // RUNTIME ERROR(count != extent)
        // std::span<int, 5> sp5{ dynamicArr, 10 }; // RUNTIME ERROR(count != extent)

        std::span<int, 10> sp6{ dynamicArr, 10 }; // std::span<int, 10>
        std::span<int, 5> sp7{ dynamicArr, 5 }; // std::span<int, 5>
    }
    
    // std::array는 크기가 그 자체로 이미 결정된 배열이기 때문에 타입 추론 가능
    {
        std::span sp1{ arr }; // std::span<int, 10>

        // std::span sp2{ arr, 10 }; // COMPILE ERROR
        // std::span sp3{ arr.data(), 10 }; // std::span<int, std::dynamic_extent>
        // std::span<int> sp4{ arr }; // std::span<int, std::dynamic_extent>

        // std::span<int, 100> sp5{ arr.data(), 10 }; // RUNTIME ERROR(count != extent)
        // std::span<int, 5> sp6{ arr.data(), 10 }; // RUNTIME ERROR(count != extent)

        std::span<int, 10> sp7{ arr.data(), 10}; // std::span<int, 10>
        std::span<int, 5> sp8{ arr.data(), 5}; // std::span<int, 5>
    }

    // std::vector는 동적 배열로 동작하기 때문에 크기를 명시해야 static extent로 사용할 수 있다.
    {
        // std::span sp1{ vec }; // std::span<int, std::dynamic_extent>

        // std::span sp2{ vec, 10 }; // COMPILE ERROR
        // std::span sp3{ vec.data(), 10 }; // std::span<int, std::dynamic_extent>
        // std::span<int> sp4{ vec }; // std::span<int, std::dynamic_extent>

        // std::span<int, 100> sp5{ vec.data(), 10 }; // RUNTIME ERROR(count != extent)
        // std::span<int, 5> sp6{ vec.data(), 10 }; // RUNTIME ERROR(count != extent)

        std::span<int, 10> sp7{ vec.data(), 10 }; // std::span<int, 10>
        std::span<int, 5> sp8{ vec.data(), 5 }; // std::span<int, 5>
    }

    // std::string도 문자열이 인접한 메모리에 저장되기 때문에 std::span을 적용할 수 있다.
    {
        // std::span sp1{ str }; // std::span<char, std::dynamic_extent>
        // std::cout << sp1.extent << ' ' << sp1.size() << '\n'; // 18446744073709551615 12

        // std::span sp2{ str, str.length() }; // COMPILE ERROR

        // std::span<char, 100> sp3{ str }; // RUNTIME ERROR(std::ranges::size(r) != extent)
        // std::span<char, 5> sp4{ str }; // RUNTIME ERROR(std::ranges::size(r) != extent)

        std::span<char, 12> sp5{ str }; // std::span<char, 12>, 널 문자 길이는 제외해서 봐야 함.

        // std::span<char, 12> sp6{ str, 12 }; // COMPILE ERROR
        std::span<char, 12> sp7{ str.data(), 12 }; // std::span<char, 12>
    }

    // 반복자 기반으로 생성하는 것도 가능하다.
    {
        // std::span sp1{ std::begin(staticArr), std::end(staticArr) }; // std::span<int, std::dynamic_extent>
        // std::span sp2{ arr.begin(), arr.end() }; // std::span<int, std::dynamic_extent>
        // std::span sp3{ std::begin(arr), std::end(arr) }; // std::span<int, std::dynamic_extent>
        // std::span sp4{ vec.begin(), vec.end() }; // std::span<int, std::dynamic_extent>
        // std::span sp5{ std::begin(vec), std::end(vec) }; // std::span<int, std::dynamic_extent>
        // std::span sp6{ str.begin(), str.end() }; // std::span<char, std::dynamic_extent>
        // std::span sp7{ std::begin(str), std::end(str) }; // std::span<char, std::dynamic_extent>
        
        // 반복자 기반으로 적용할 때 begin()과 end()는 런타임에 실행되는 함수이기 때문에
        // static extent 방식을 적용하려면 직접 크기를 지정해야 한다.
        std::span<int, 10> sp8{ vec.begin(), vec.end() }; // std::span<int, 10>
        std::span<int, 10> sp9{ std::begin(vec), std::end(vec) }; // std::span<int, 10>

        // std::span<int, 5> sp10{ vec.begin(), vec.end() }; // RUNTIME ERROR(last - first != extent)
        std::span<int, 5> sp11{ vec.begin(), vec.begin() + 5 }; // std::span<int, 5>
        std::span<int, 5> sp12{ vec.begin(), vec.end() - 5 }; // std::span<int, 5>

        // https://en.cppreference.com/w/cpp/iterator#Range_access
        // std::begin()과 std::end()는 스택 영역에 있는 배열을 대상으로도 사용할 수 있다.
        std::span<int, 10> sp13{ std::begin(staticArr), std::end(staticArr) }; // std::span<int, 10>

        // 동적할당한 배열은 스택이 아닌 힙 영역에 있기 때문에 두 함수를 사용할 수 없다.
        // std::span<int, 10> sp13{ std::begin(dynamicArr), std::end(dynamicArr) }; // COMPILE ERROR

        // 동적할당한 배열은 기존의 포인터를 사용하는 것처럼 코드를 작성해야 한다.
        // std::span<int, 10> sp14{ dynamicArr, dynamicArr + 5 }; // RUNTIME ERROR(last - first != extent)
        std::span<int, 10> sp15{ dynamicArr, dynamicArr + 10 }; // std::span<int, 10>
    }

    // 반복자 외 생성자의 인자를 2개를 받는 경우 생성자의 첫 번째 인자는 포인터이다(컨테이너 아님).
    {
        // std::span sp1{ arr, 10 }; // COMPILE ERROR
        // std::span sp2{ vec, 10 }; // COMPILE ERROR

        // std::span<int, 10> sp3{ arr, 10 }; // COMPILE ERROR
        // std::span<int, 10> sp4{ vec, 10 }; // COMPILE ERROR

        std::span<int, 10> sp5{ arr.data(), 10 }; // std::span<int, 10>
        std::span<int, 10> sp6{ vec.data(), 10 }; // std::span<int, 10>

        std::span<int, 10> sp7{ staticArr, 10 }; // std::span<int, 10>
        std::span<int, 10> sp8{ dynamicArr, 10 }; // std::span<int, 10>

        // https://en.cppreference.com/w/cpp/iterator/data
        // std::data()를 쓰면 정적 배열이나 컨테이너에 상관 없이 데이터의 시작 주소를 받아 범용성 있는 코드를 작성할 수 있다(C++17).
        // 템플릿 코드를 작성할 때는 멤버 함수를 직접 호출하는 것보다 이렇게 Range access 기반으로 작업하는 것이 좋은 선택이다.
        // 일관성을 보장하는 템플릿 코드를 작성할 때 사용하면 좋은 함수이다.
        std::span<int, 10> sp9{ std::data(staticArr), 10 }; // std::span<int, 10>
        std::span<int, 10> sp10{ std::data(arr), 10 }; // std::span<int, 10>
        std::span<int, 10> sp11{ std::data(vec), 10 }; // std::span<int, 10>
    }

    // !! 정리 !!
    // std::span을 static extent 형식으로 쓸 때는 템플릿 인자 2개를 명시하는 것이 좋다.
    // 
    // - std::span sp{ staticArr }; // std::span<int, 10>
    // - std::span sp{ vec };       // std::span<int, std::dynamic_extent>
    // 
    // - std::span<int, 10> sp{ staticArr };                // std::span<int, 10>
    // - std::span<int, 10> sp{ std::data(staticArr), 10 }; // std::span<int, 10>
    // - std::span<int, 10> sp{ std::data(vec), 10 };       // std::span<int, 10>
    // 
    // std::span 인자만 명시하고 나머지는 생략하면 전달되는 자료형에 따라 dynamic_extent가 적용되는 경우도 있다.

    // Clean-up
    delete[] dynamicArr;
}

void SpanWithDynamicExtent()
{
    int  staticArr[]{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    int* dynamicArr = new int[] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    std::array  arr{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    std::vector vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    std::string str{ "Hello World!" };

    /**
     * IDE에서 지원하는 마우스 인스팩터를 통해 타입을 확인해서 볼 것
     */

    // 첫 번째 인자에 타입을 명시하고 두 번째 인자를 생략하면 dynamic extent로 동작한다.
    {
        // 아래 변수들의 타입에 적용되는 두 번째 템플릿 인자는 std::dynamic_extent이다.
        std::span<int> sp1{ staticArr }; // 스택 영역에 있는 배열은 크기 추론 가능
        std::span<int> sp2{ dynamicArr, 10 }; // 동적할당한 배열은 생성자에 크기를 명시해야 함(힙 영역에 있는 건 크기 추론 불가).
        std::span<int> sp3{ arr };
        std::span<int> sp4{ vec };
        std::span<char> sp5{ str };
    }

    // 모든 템플릿 인자를 생략하는 경우 유형에 따라 dynamic extent로 동작하기도 하고 static extent로 동작하기도 한다.
    {
        // std::span sp1{ staticArr }; // std::span<int, 10>
        std::span sp2{ dynamicArr, 10 }; // std::span<int, std::dynamic_extent>
        // std::span sp3{ arr }; // std::span<int, 10>
        std::span sp4{ vec }; // std::span<int, std::dynamic_extent>
        std::span sp5{ str }; // std::span<char, std::dynamic_extent>
    }

    // 모든 템플릿 인자를 생략해도 생성자의 첫 번째 인자가 포인터인 경우 dynamic extent로 동작한다.
    {
        // 포인터를 첫 번째 인자로 넣는 경우 두 번째 인자를 생략해선 안 된다.
        // std::span sp1{ std::data(staticArr) }; // COMPILE ERROR
        // std::span sp2{ dynamicArr };     // COMPILE ERROR
        // std::span sp3{ std::data(arr) }; // COMPILE ERROR
        // std::span sp4{ std::data(vec) }; // COMPILE ERROR
        // std::span sp5{ std::data(str) }; // COMPILE ERROR

        std::span sp1{ std::data(staticArr), 1 }; // std::span<int, std::dynamic_extent>
        // std::span sp2{ std::data(dynamicArr), 2 }; // COMPILE ERROR(동적할당한 배열은 std::data() 적용 불가)
        std::span sp3{ dynamicArr, 2 };     // std::span<int, std::dynamic_extent>
        std::span sp4{ std::data(arr), 3 }; // std::span<int, std::dynamic_extent>
        std::span sp5{ std::data(vec), 4 }; // std::span<int, std::dynamic_extent>
        std::span sp6{ std::data(str), 5 }; // std::span<char, std::dynamic_extent>
    }

    // static extent 방식에 반복자를 적용하면 무조건 (last - first)의 값을 extent와 일치시켜야 했다.
    // dynamic extent 방식에 반복자를 적용하면 굳이 크기를 일치시키지 않아도 된다.
    {
        std::span<int> sp1{ std::begin(staticArr), std::end(staticArr) };
        std::span<int> sp2{ dynamicArr + 2, dynamicArr + 5 };
        std::span<int> sp3{ std::begin(arr), std::begin(arr) + 5 };
        std::span<int> sp4{ std::begin(vec) + 3, std::end(vec) - 3 };
        std::span<char> sp5{ std::begin(str), std::end(str) };
    }

    // Clean-up
    delete[] dynamicArr;
}

void ReassignTest()
{
    int  staticArr[]{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    int* dynamicArr = new int[] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    
    std::array  arr{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    std::vector vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    std::string str{ "Hello World!" };
    
    // spans with static extent
    std::span<int, 10> staticSp1{ std::data(staticArr), 10 };
    std::span<int, 10> staticSp2{ dynamicArr, 10 };
    
    std::span<int, 5> staticSp3{ std::data(arr), 5 };
    std::span<int, 5> staticSp4{ std::data(vec), 5 };

    std::span<char, 12> staticSp5{ str.data(), 12 };
    
    // spans with dynamic extent
    std::span<int> dynamicSp1{ staticArr };
    std::span<int> dynamicSp2{ dynamicArr, 10 };
    
    std::span<int> dynamicSp3{ arr };
    std::span<int> dynamicSp4{ vec };

    // static extent는 자료형이 서로 일치해야만 reassign을 허용한다.
    {
        staticSp1 = staticSp2;
        staticSp3 = staticSp4;

        // staticSp1 = staticSp3; // COMPILE ERROR
        // staticSp4 = staticSp2; // COMPILE ERROR

        // staticSp1 = dynamicSp1; // COMPILE ERROR
        // staticSp1 = dynamicSp2; // COMPILE ERROR
    }

    // static extent가 컨테이너와 호환되면 reassign을 허용한다.
    {
        staticSp1 = staticArr;
        // staticSp1 = dynamicArr; // COMPILE ERROR

        staticSp1 = arr;
        // staticSp1 = vec; // COMPILE ERROR
    }

    // dynamic extent는 타입이 일치하기만 하면 ressign을 허용한다.
    {
        dynamicSp1 = dynamicSp2;
        dynamicSp1 = dynamicSp3;
        dynamicSp1 = dynamicSp4;

        dynamicSp1 = staticSp1;
        dynamicSp1 = staticSp2;
        dynamicSp1 = staticSp3;
        dynamicSp1 = staticSp4;

        // dynamicSp1 = staticSp5; // COMPILE ERROR(타입 불일치)
    }

    // dynamic extent는 크기를 알 수 있는 연속된 메모리를 가진 컨테이너라면 reassign을 허용한다.
    {
        dynamicSp1 = staticArr;
        // dynamicSp1 = dynamicArr; // 힙 영역에 있는 건 크기를 파악할 수 없음.

        dynamicSp1 = arr;
        dynamicSp1 = vec;

        // dynamicSp1 = str; // COMPILE ERROR(타입 불일치)
    }

    // Clean-up
    delete[] dynamicArr;
}

void SpanFromSpan()
{
    std::vector vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    std::span<int, 10> staticSp{ vec };
    std::span<int>     dynamicSp{ vec };

    // std::span도 그 자체로 contiguous_range 조건을 충족하기 때문에 다른 std::span을 생성하는 것이 가능하다.
    std::span<int, 10> otherSp1 = staticSp;
    std::span<int, 7>  otherSp2{ staticSp.data(), 7};
    std::span<int, 5>  otherSp3{ std::data(staticSp), 5};

    // std::span<int, 10> otherSp4 = dynamicSp.data(); // COMPILE ERROR
    std::span<int, 10> otherSp5{ dynamicSp.data(), 10 };
    std::span<int, 5>  otherSp6{ dynamicSp.data(), 5 };

    // dynamic extent 유형은 어떤 크기든 원소의 타입이 일치하면 다 받을 수 있다.
    std::span<int> otherSp7  = staticSp;
    std::span<int> otherSp8  = dynamicSp;
    std::span<int> otherSp9  = otherSp2;
}

void ReferenceTest()
{
    std::vector vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    std::array  arr{ 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };

    std::span<int> sp1{ vec };
    std::span<int> sp2{ vec };

    // std::span은 원본을 가리키는 일종의 레퍼런스 자료형이기 때문에 값을 변경하면 원본에 반영된다.
    // std::span도 begin()과 end()를 멤버 함수로 가지기에 range-based for loop를 사용할 수 있다.
    for (auto& elem : sp1)
    {
        elem *= 2;
    }

    // 원본 조회
    std::cout << "vec : [ ";
    for (auto elem : vec)
    {
        std::cout << elem << ' ';
    }
    std::cout << "]\n";

    // 같은 대상을 참조하는 std::span 조회
    std::cout << "sp2 : [ ";
    for (auto elem : sp2)
    {
        std::cout << elem << ' ';
    }
    std::cout << "]\n";

    // reassign을 통해 참조 대상을 변경하고 조회하면 당연히 바꾼 참조 대상의 값을 출력한다.
    sp1 = arr;

    std::cout << "sp1 : [ ";
    for (auto elem : sp1)
    {
        std::cout << elem << ' ';
    }
    std::cout << "]\n";
}

void WarningCases()
{
    std::vector<int> vec(10);
    std::iota(vec.begin(), vec.end(), 1);

    // std::span은 원본을 참조하는 뷰(non-owning view)이기 때문에 원본이 모종의 이유로 손상(?)되면 문제가 생긴다.
    // std::span이 원본을 참조하고 있는 동안은 원본은 반드시 사용할 수 있는 상태여야 한다.
    std::span<int> sp1 = vec;
    vec.push_back(100); // 벡터가 꽉 찬 상태에서 push_back()을 진행하면 재할당을 진행하며 이 과정에서 std::span이 가리키고 있는 원본은 해제됨.
    
    // 아래 코드는 실행하면 터질 수 있으니 주의해야 한다.
    // std::cout << sp1[0] << '\n'; // 더 이상 유효하지 않은 버퍼를 대상으로 접근하는 상황 발생!

    // (주의)std::span 자체를 const로 만든다고 해도 원본에 접근해서 데이터를 수정할 수 있다.
    // const std::span의 operator[]는 const로 작성되어 있지만 반환 대상에는 cosnt가 붙어 있지 않다.
    const std::span<int> sp2 = vec;

    const auto& whatIWantToPrevent = sp2[0];
    // whatIWantToPrevent = 100; // 이 작업을 막고자 하는데...

    auto& butItWorksLikeThis = sp2[0]; // 반환 타입에 const가 붙지 않아서 auto는 그냥 int로 조회됨.
    butItWorksLikeThis = 100; // 따라서 이렇게 그냥 수정할 수 있음.

    sp2[0] *= 2; // 우리가 원하는 건 이 작업을 막는 것임.

    std::cout << "vec[0] : " << vec[0] << '\n'; // 수정된 값이 조회됨.

    // 좀 희한하지만 const가 붙어야 하는 건 std::span이 아닌 std::span의 원소 타입이다.
    std::span<const int> sp3 = vec;
    // sp3[0] = 1000; // COMPILE ERROR

    // const std::span이 값의 수정을 허용하는 건 C++ 버전이 올라가면서 deprecated될 가능성이 있으니 일단 참고만 하자.

    // 이 내용은 std::span을 함수의 매개변수로 사용할 때도 적용되는 내용이니 기억하도록 하자.
    // 인자로 던진 컨테이너를 실수로 수정하여 버그가 발생하는 경우도 있을 수 있으니 말이다.
}

void Run()
{
    SpanWithStaticExtent();
    SpanWithDynamicExtent();
    ReassignTest();
    SpanFromSpan();
    ReferenceTest();
    WarningCases();
}

END_NS

BEGIN_NS(Case02)

// std::span을 로컬 영역에서 생성해서 사용하는 경우는 거의 없다.
// std::span은 함수의 매개변수로 사용되었을 때 그 진가가 발휘된다.

// std::span은 복사 비용이 저렴하기에 레퍼런스 형태로 전달하지 않아도 된다(보통 16바이트).
// sizeof(std::span<T, N>) == sizeof(T*) + sizeof(size_t)

// 아래 형식은 사용하는 과정에서 타입을 명시해야 한다.
// template <typename T, std::size_t Size>
// void PrintStatic(std::span<T, Size> span)
// {
//     ...
// }
// 
// int        staticArr[]{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
// std::array arr{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
//
// PrintStatic(staticArr); // ERROR : no matching function for call to 'PrintStatic(int [10])'
// PrintStatic(arr);       // ERROR : no matching function for call to 'PrintStatic(std::array<int, 10>&)'
//
// PrintStatic(std::span<int, 10>{ staticArr }); // PASS
// PrintStatic(std::span<int, 10>{ arr });       // PASS
// 
// 템플릿 함수 내 템플릿 매개변수를 이중 추론을 통해 암묵적인 형변환을 적용하는 건 불가능한 것 같다.
// 템플릿 타입 추론은 함수 따로 매개변수 따로 독립적으로 이루어진다고 봐야할 듯?
// 

// 좀 복잡하긴 해도 사용 단계에서 그냥 타입만 넘겨서 사용하려면 우회해야 한다.

template <typename T, std::size_t Size>
    requires (Size != std::dynamic_extent)
void PrintStaticImpl(std::span<T, Size> span, std::string_view msg)
{
    std::cout << std::format("static extent - {:<10} : ", msg);

    if constexpr (Size == 0) // 크기가 컴파일 타임에 결정되기에 사용 가능한 방법
    {
        std::cout << "Empty\n";
    }
    else
    {
        std::cout << std::format("Extent[{}], First[{}], Last[{}] [ ", span.extent, span.front(), span.back());

        for (auto elem : span)
        {
            std::cout << elem << ' ';
        }

        std::cout << "]\n";
    }
}

template <std::ranges::range Range>
    requires requires (Range range)
    {
        PrintStaticImpl(std::span{ range }, "");
    }
void PrintStatic(Range&& range, std::string_view msg)
{
    // PrintStaticImpl(range); // 이거 쓰면 에러 발생(템플릿 함수 내 템플릿 매개변수를 이중 추론을 통해 암묵적인 형변환을 적용하는 건 불가능)
    PrintStaticImpl(std::span{ range }, msg); // 이렇게 해야 통과

    // 본래 하려고 했던 작업
    //
    // using RemoveRefType = std::remove_reference_t<Range>;
    // using Span = std::span<std::ranges::range_value_t<RemoveRefType>, std::extent_v<RemoveRefType>>;
    // 
    // if constexpr (Span::extent != std::dynamic_extent) // 컴파일 타임에 조건을 맞아야만 사용할 수 있게
    // {
    //     PrintStaticImpl(Span{ range });
    // }
    // 
    // 
    // 정적 배열은 std::extent_v<T>를 통해 컴파일 타임에 상수를 가져와 std::span의 두 번째 템플릿 인자로 넣을 수 있지만
    // 컨테이너 타입은 함수 호출을 통해서만 길이를 가지고 올 수 있어서 std::span의 두 번째 인자에 적용하는 것이 불가능했다.
    // 
    // Span span{ range };
    // 
    // if (span.extent != std::dynamic_extent) // 이건 컴파일 타임이 아닌 런타임에 걸러주는 것
    // {
    //     PrintStaticImpl(span);
    // }
    // 
    // 이런 식으로 사용하면 될 것 같긴 한데 컴파일 타임에 결정하는 것이 아니기 때문에 이는 내가 의도한 코드가 아니다.
    // 고민 끝에 함수 호출이 되었다고 가정하여 우회하는 방법을 찾다가 채택한 코드가 이거다.
    // 아직까지는 더 나은 방법을 떠올리지 못했다.

    // 아래 코드는 컴파일 타임에 dynamic extent를 걸러서 에러를 발생시키지 못 한다.
    // assert()나 static_assert()를 통한 검증은 내가 의도한 바가 아니다(이 방법은 사용하는 쪽에서 원인을 파악하지 못 함).
    // using RemoveRefType = std::remove_reference_t<Range>;
    // 
    // if constexpr (std::is_array_v<RemoveRefType>) // 정적 배열만 허용(std::array는 통과 안 함)
    // {
    //     using Span = std::span<std::ranges::range_value_t<RemoveRefType>, std::extent_v<RemoveRefType>>;
    // 
    //     PrintStaticImpl(Span{ range });
    // }
    // else
    // {
    //     PrintStaticImpl(std::span{ range });
    // }
}

// dynamic extent도 static extent와 유사하게 코드를 작성하면 된다.

template <typename T>
void PrintDynamicImpl(std::span<T> span, std::string_view msg)
{
    std::cout << std::format("dynamic extent - {:<10} : ", msg);

    if (span.size() == 0) // size가 동적으로 결정되기에 constexpr 적용 불가(적용하면 에러 발생)
    {
        std::cout << "Empty\n";
    }
    else
    {
        std::cout << std::format("Size[{}], First[{}], Last[{}] [ ", span.size(), span.front(), span.back());

        for (auto elem : span)
        {
            std::cout << elem << ' ';
        }

        std::cout << "]\n";
    }
}

template <std::ranges::range Range>
    requires requires (Range range)
    {
        // 많이 복잡해도 이렇게 해야 가능하다.
        PrintDynamicImpl(std::span<std::ranges::range_value_t<Range>>{ range }, "");
    }
void PrintDynamic(Range&& range, std::string msg)
{
    using RemoveRefType = std::remove_reference_t<Range>;
    using SpanElemType  = std::ranges::range_value_t<RemoveRefType>;

    // PrintDynamicImpl(range); // 이거 쓰면 에러 발생(템플릿 함수 내 템플릿 매개변수를 이중 추론을 통해 암묵적인 형변환을 적용하는 건 불가능)

    // 이 방식도 사용할 수 없다.
    // PrintDynamicImpl()가 받는 건 "std::span<T, std::dynamic_extent> span"이기 때문에 static extent 계열이 섞이면 에러가 발생한다.
    // PrintStaticImpl(std::span{ range }, msg);

    PrintDynamicImpl(std::span<SpanElemType>{ range }, msg); // 이렇게 타입만 딱 지정해야 통과
}

// 컴파일러의 최적화를 노리는 경우라면 이렇게 사용할 수 있지만 범용성이 떨어진다.
void PrintSpanIntTen(std::span<int, 10> span, std::string_view msg)
{
    std::cout << std::format("std::span<int, 10> {:<10} : ", msg);

    // span의 크기는 무조건 10이 나온다(elements의 개수가 무조건 10개라는 뜻)
    std::cout << std::format("Extent[{}], First[{}], Last[{}] [ ", span.extent, span.front(), span.back());

    for (auto elem : span)
    {
        std::cout << elem << ' ';
    }

    std::cout << "]\n";
}

// 보통은 이렇게 받을 타입만 딱 지정해서 받는다.
void PrintSpanIntAny(std::span<int> span, std::string_view msg)
{
    std::cout << std::format("std::span<int> {:<10} : ", msg);

    if (span.size() == 0) // size가 동적으로 결정되기에 constexpr 적용 불가(적용하면 에러 발생)
    {
        std::cout << "Empty\n";
    }
    else
    {
        std::cout << std::format("Size[{}], First[{}], Last[{}] [ ", span.size(), span.front(), span.back());

        for (auto elem : span)
        {
            std::cout << elem << ' ';
        }

        std::cout << "]\n";
    }
}

void Run()
{
    int  staticArr[]{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    int* dynamicArr = new int[] { 2, 4, 6, 8, 10, 12, 14, 16, 18, 20 };

    std::array  arr{ 3, 6, 9, 12, 15, 18, 21, 24, 27, 30 };
    std::vector vec{ 4, 8, 12, 16, 20, 24, 28, 32, 36, 40 };

    std::array<int, 0> emptyArr{ };
    std::vector<int>   emptyVec{ };

    std::string str{ "Hello World!" };

    // Static
    {
        std::cout << "##### static extent #####\n";

        PrintStatic(staticArr, "staticArr");
        // PrintStatic(dynamicArr, "dynamicArr"); // COMPILE ERROR
        PrintStatic(arr, "arr");
        // PrintStatic(vec, "vec"); // COMPILE ERROR
        PrintStatic(emptyArr, "emptyArr");
        // PrintStatic(emptyVec, "emptyVec"); // COMPILE ERROR
        // PrintStatic(str, "str"); // COMPILE ERROR

        std::cout << "--------------------------------------------------\n";

        // static extent를 추론할 수 없는 경우 수동으로 std::span을 생성해서 넣어주면 된다(비추천).
        PrintStatic(std::span<int, 5>{ staticArr, 5 }, "staticArr"); // 이렇게 제한해서 가져오는 것도 가능함.
        PrintStatic(std::span<int, 10>{ dynamicArr, 10}, "dynamicArr");
        PrintStatic(std::span<int, 5>{ std::begin(arr) + 2, std::begin(arr) + 7 }, "arr"); // 반복자 기반으로 넣는 것도 extent 크기만 맞으면 허용함.
        PrintStatic(std::span<int, 10>{ std::data(vec), 10 }, "vec");     // 이건 포인터로 받는 방식임.
        PrintStatic(std::span<int, 0>{ emptyArr.data(), 0 }, "emptyArr"); // 마찬가지로 포인터로 받는 방식임.
        PrintStatic(std::span<int, 0>{ emptyVec.data(), 0 }, "emptyVec"); // 포인터
        PrintStatic(std::span<char, 12>{ str.data(), 12 }, "str"); // 문자열의 길이에 딱 맞게 전달해야 해서 실용성이 없음.
    }

    std::cout << '\n';

    // Dynamic
    {
        std::cout << "##### dynamic extent #####\n";

        PrintDynamic(staticArr, "staticArr");
        // PrintDynamic(dynamicArr, "dynamicArr"); // COMPILE ERROR(힙 영역에 있는 배열을 받는 건 단순 포인터임)
        PrintDynamic(arr, "arr");
        PrintDynamic(vec, "vec");
        PrintDynamic(emptyArr, "emptyArr");
        PrintDynamic(emptyVec, "emptyVec");
        PrintDynamic(str, "str");

        std::cout << "--------------------------------------------------\n";

        // 템플릿 인자를 하나만 기입하면 두 번째 인자는 std::dynamic_extent가 사용된다.
        PrintDynamic(std::span<int>{ staticArr, 8 }, "staticArr");
        PrintDynamic(std::span<int>{ dynamicArr, 10 }, "dynamicArr");
        PrintDynamic(std::span<int>{ std::begin(arr) + 2, std::begin(arr) + 7 }, "arr");
        PrintDynamic(std::span<int>{ std::data(vec), 7 }, "vec");
        PrintDynamic(std::span<int>{ emptyArr }, "emptyArr");
        PrintDynamic(std::span<int>{ emptyVec }, "emptyVec");
        PrintDynamic(std::span<char>{ str }, "str");
    }

    std::cout << '\n';

    // Unusual Cases
    {
        std::cout << "##### Unusual Cases #####\n";

        PrintSpanIntTen(staticArr, "staticArr");
        // PrintSpanIntTen(dynamicArr, "dynamicArr"); // COMPILE ERROR
        PrintSpanIntTen(arr, "arr");
        // PrintSpanIntTen(vec, "vec"); // COMPILE ERROR
        // PrintSpanIntTen(emptyArr, "emptyArr"); // COMPILE ERROR
        // PrintSpanIntTen(emptyVec, "emptyVec"); // COMPILE ERROR
        // PrintSpanIntTen(str, "str"); // COMPILE ERROR(이쪽은 아예 타입이 불일치함)
    }

    std::cout << '\n';

    // Usual Cases
    {
        std::cout << "##### Usual Cases #####\n";

        PrintSpanIntAny(staticArr, "staticArr");
        // PrintSpanIntAny(dynamicArr, "dynamicArr"); // COMPILE ERROR
        PrintSpanIntAny(arr, "arr");
        PrintSpanIntAny(vec, "vec");
        PrintSpanIntAny(emptyArr, "emptyArr");
        PrintSpanIntAny(emptyVec, "emptyVec");
        // PrintSpanIntAny(str, "str"); // COMPILE ERROR(이쪽은 아예 타입이 불일치함)
    }

    // Clean-up
    delete[] dynamicArr;
}

END_NS

BEGIN_NS(Case03)

// https://en.cppreference.com/w/cpp/container/span#Subviews
// https://en.cppreference.com/w/cpp/container/span/first
// https://en.cppreference.com/w/cpp/container/span/last
// https://en.cppreference.com/w/cpp/container/span/subspan

// std::span은 subspan을 만드는 것이 가능하다. 
// (중요) subspan은 static extent와 dynamic extent를 모두 반환할 수 있게 함수 오버로딩이 되어 있다.

// Case02에 있는 예제와는 달리 직접 std::span을 넘길 것이라 문제 없다.
template <typename T, std::size_t Size>
void Print(std::span<T, Size> span, std::string_view msg)
{
    std::cout << std::format("{:<16} : ", msg);

    if (span.size() == 0)
    {
        std::cout << "Empty\n";
    }
    else
    {
        auto type = Size == std::dynamic_extent ? "dynamic" : "static";

        std::cout << std::format("Type[{}], Size[{}], First[{}], Last[{}] [ ", type, span.size(), span.front(), span.back());

        for (auto elem : span)
        {
            std::cout << elem << ' ';
        }

        std::cout << "]\n";
    }
}

void Run()
{
    std::vector<int> vec(20);
    std::iota(vec.begin(), vec.end(), 1);

    std::span<int, 20> staticSp{ std::data(vec), 20 };
    std::span<int> dynamicSp{ vec };

    Print(std::span<int>{ vec }, "original");

    std::cout << '\n';

    // first(N) : 앞에서 N개의 요소만 받는 std::span을 반환(dynamic extent)
    // auto dontUseLikeThis = staticSp.first(100); // RUNTIME ERROR(Count out of range)
    // auto dontUseLikeThis = dynamicSp.first(100); // RUNTIME ERROR(Count out of range)
    auto spFirst1 = staticSp.first(4);
    auto spFirst2 = dynamicSp.first(8);

    Print(spFirst1, "first(4)");
    Print(spFirst2, "first(8)");

    std::cout << '\n';

    // first<N>() : 앞에서 N개의 요소만 받는 std::span을 반환(static extent)
    // auto dontUseLikeThis = staticSp.first<100>(); // COMPILE ERROR(Count out of range)
    // auto dontUseLikeThis = dynamicSp.first<100>(); // RUNTIME ERROR(Count out of range)
    auto spFirst3 = staticSp.first<4>();
    auto spFirst4 = dynamicSp.first<8>();

    Print(spFirst3, "first<4>()");
    Print(spFirst4, "first<8>()");

    std::cout << '\n';

    // last(N) : 뒤에서 N개의 요소만 받는 std::span을 반환(dynamic extent)
    // auto dontUseLikeThis = staticSp.last(100); // RUNTIME ERROR(Count out of range)
    // auto dontUseLikeThis = dynamicSp.last(100); // RUNTIME ERROR(Count out of range)
    auto spLast1 = staticSp.last(4);
    auto spLast2 = dynamicSp.last(8);

    Print(spLast1, "last(4)");
    Print(spLast2, "last(8)");

    std::cout << '\n';

    // last<N>() : 뒤에서 N개의 요소만 받는 std::span을 반환(static extent)
    // auto dontUseLikeThis = staticSp.last<100>(); // COMPILE ERROR(Count out of range)
    // auto dontUseLikeThis = dynamicSp.last<100>(); // COMPILE ERROR(Count out of range)
    auto spLast3 = staticSp.last<4>();
    auto spLast4 = dynamicSp.last<8>();

    Print(spLast3, "last<4>()");
    Print(spLast4, "last<8>()");

    std::cout << '\n';

    // subspan(Offset) : Offset 위치부터 마지막까지의 요소를 받는 std::span을 반환(dynamic extent)
    // auto dontUseLikeThis = staticSp.subspan(100); // RUNTIME ERROR(Offset out of range)
    // auto dontUseLikeThis = dynamicSp.subspan(100); // RUNTIME ERROR(Offset out of range)
    auto subspan1 = staticSp.subspan(5);
    auto subspan2 = dynamicSp.subspan(10);

    Print(subspan1, "subspan(5)");
    Print(subspan2, "subspan(10)");

    std::cout << '\n';

    // subspan(Offset, Count) : Offset 위치에서 Count개의 요소를 받는 std::span을 반환(dynamic extent)
    // auto dontUseLikeThis = staticSp.subspan(100, 10); // RUNTIME ERROR(Offset out of range)
    // auto dontUseLikeThis = staticSp.subspan(10, 100); // RUNTIME ERROR(Count out of range)
    // auto dontUseLikeThis = dynamicSp.subspan(100, 10); // RUNTIME ERROR(Offset out of range)
    // auto dontUseLikeThis = dynamicSp.subspan(10, 100); // RUNTIME ERROR(Count out of range)
    auto subspan3 = staticSp.subspan(5, 7);
    auto subspan4 = dynamicSp.subspan(10, 7);

    Print(subspan3, "subspan(5, 7)");
    Print(subspan4, "subspan(10, 7)");

    std::cout << '\n';

    // subspan<Offset>() : Offset 위치부터 마지막까지의 요소를 받는 std::span을 반환(!! 반환 타입 주의 !!)
    // auto dontUseLikeThis = staticSp.subspan<100>(); // COMPILE ERROR(Offset out of range)
    // auto dontUseLikeThis = dynamicSp.subspan<100>(); // RUNTIME ERROR(Offset out of range)
    auto subspan5 = staticSp.subspan<5>();   // std::span(static extent)인 경우 std::span(static extent)를 반환하고 원소의 개수가 extent 값임.
    auto subspan6 = dynamicSp.subspan<10>(); // std::span(dynamic extent)인 경우 std::span(dynamic extent) 반환

    Print(subspan5, "subspan<5>()");
    Print(subspan6, "subspan<10>()");

    std::cout << '\n';

    // subspan<Offset, Count>() : Offset 위치에서 Count개의 요소를 받는 std::span을 반환(static extent)
    // auto dontUseLikeThis = staticSp.subspan<100, 10>(); // COMPILE ERROR(Offset out of range)
    // auto dontUseLikeThis = staticSp.subspan<10, 100>(); // COMPILE ERROR(Count out of range)
    // auto dontUseLikeThis = dynamicSp.subspan<100, 10>(); // RUNTIME ERROR(Offset out of range)
    // auto dontUseLikeThis = dynamicSp.subspan<10, 100>(); // RUNTIME ERROR(Count out of range)
    auto subspan7 = staticSp.subspan<5, 7>();   // std::span(static extent)인 경우 std::span(static extent)를 반환하고 원소의 개수가 extent 값임.
    auto subspan8 = dynamicSp.subspan<10, 7>(); // std::span(dynamic extent)인 경우 std::span(dynamic extent) 반환

    Print(subspan7, "subspan<5, 7>()");
    Print(subspan8, "subspan<10, 7>()");

    std::cout << '\n';
}

END_NS

BEGIN_NS(Case04)

// std::span도 range 조건을 충족하기 때문에 Ranges 라이브러리에서 제공하는 기능을 사용할 수 있다.

void DoSort(std::span<int> span)
{
    // std::span 자체는 원본의 레퍼런스이기 때문에 작업한 내역이 원본에 영향을 미친다.
    std::ranges::sort(span);
}

void PrintOdd(std::span<const int> span)
{
    std::cout << "PrintOdd() : [ ";

    for (auto elem : span | std::views::filter([](int elem) { return elem % 2 != 0; }))
    {
        std::cout << elem << ' ';
    }

    std::cout << "]\n";
}

void PrintEven(std::span<const int> span)
{
    std::cout << "PrintEven() : [ ";

    for (auto elem : span | std::views::filter([](int elem) { return elem % 2 == 0; }))
    {
        std::cout << elem << ' ';
    }

    std::cout << "]\n";
}

void PrintMultiplied(std::span<const int> span, int times)
{
    auto multiplier = std::bind_front(std::multiplies<int>{  }, times);

    std::cout << "PrintMultiplied() " << times << " times [ ";

    for (auto elem : span | std::views::transform(multiplier))
    {
        std::cout << elem << ' ';
    }

    std::cout << "]\n";
}

void Run()
{
    // auto chk = std::ranges::range<std::span<int>>;
    // std::cout << chk << '\n';

    int staticArr[]{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    std::vector<int> vec(10);
    std::ranges::generate(vec, []() { return std::random_device{ }() % 100; });

    PrintOdd(staticArr);
    PrintEven(staticArr);
    PrintMultiplied(staticArr, 2);
    PrintMultiplied(staticArr, 3);

    std::cout << '\n';

    DoSort(vec);

    PrintOdd(vec);
    PrintEven(vec);
    PrintMultiplied(vec, 2);
    PrintMultiplied(vec, 3);
}

END_NS

int main()
{
    // Case01::Run();
    // Case02::Run();
    Case03::Run();
    // Case04::Run();

    return 0;
}
