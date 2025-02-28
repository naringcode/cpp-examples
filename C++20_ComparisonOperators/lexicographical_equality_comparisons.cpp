// Update Date : 2025-02-27
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>

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
// 6. lexicographical_equality_comparisons.cpp <-----
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

#define BEGIN_NS(name) namespace name {
#define END_NS }

BEGIN_NS(Case01)

// operator<=>나 operator==를 default로 정의하면 멤버의 배열이나 컨테이너 요소를 사전 순서대로 처리한다.
// 이 과정에서 비교 가능한 한도 내에 모든 요소를 비교한다.
// - 배열 : arrayA[n] @ arrayB[n] (lexicographically)

struct Type
{
    char arr[20]{ };

    auto operator<=>(const Type& rhs) const = default;
};

void Run()
{
    Type a{ "Hello World" };
    Type b{ "Hello World" };

    // operator<=>를 default로 정의하면 컴파일러 차원에서 사전 순서대로 처리하는 operator==를 제공한다.
    std::cout << "(a == b) " << (a == b) << '\n'; // 1
    std::cout << "(a != b) " << (a != b) << '\n'; // 0

    // (a == b) : 최적화 모드를 끄고 디스어셈블러를 통해 확인하면 다음 과정을 거친다.
    // call        Type::operator==
    // jmp         Type::operator==
    //
    // Type::operator==(const Type &):
    // mov         eax,1  
    // imul        rax,rax,0  
    // mov         rcx,qword ptr [this]  
    // movsx       eax,byte ptr [rcx+rax]  
    // mov         ecx,1  
    // imul        rcx,rcx,0  
    // mov         rdx,qword ptr [__that]  
    // movsx       ecx,byte ptr [rdx+rcx]  
    // cmp         eax,ecx  // 비교
    // jne         Type::operator==+3DDh
    // ----- 구분선 -----
    // mov         eax,1  
    // imul        rax,rax,1  
    // mov         rcx,qword ptr [this]  
    // movsx       eax,byte ptr [rcx+rax]  
    // mov         ecx,1  
    // imul        rcx,rcx,1  
    // mov         rdx,qword ptr [__that]  
    // movsx       ecx,byte ptr [rdx+rcx]  
    // cmp         eax,ecx  // 비교
    // jne         Type::operator==+3DDh
    // ----- 구분선 -----
    // mov         eax,1  
    // imul        rax,rax,2  
    // mov         rcx,qword ptr [this]  
    // movsx       eax,byte ptr [rcx+rax]  
    // mov         ecx,1  
    // imul        rcx,rcx,2  
    // mov         rdx,qword ptr [__that]  
    // movsx       ecx,byte ptr [rdx+rcx]  
    // cmp         eax,ecx  // 비교
    // jne         Type::operator==+3DDh
    // ..... 배열의 길이 만큼 반복 .....
}

END_NS

BEGIN_NS(Case02)

// 배열의 길이가 다르다면 이러한 동작 방식은 굉장히 비효율적일 수 있다.
// 이 경우에는 모든 요소를 대상으로 비교하는 작업을 거치지 않아도 된다.
// 따라서 operator== 방식을 직접 정의하여 다음과 같이 개선하는 것이 가능하다.

struct Type
{
    char arr[20]{ };

    auto operator<=>(const Type& rhs) const = default;
    
    // operator==만 따로 정의하기
    auto operator==(const Type& rhs) const
    {
        auto lhsLen = strlen(arr);
        auto rhsLen = strlen(rhs.arr);

        // 길이가 같을 경우에만 lexicographical한 비교 진행
        if (lhsLen == rhsLen && !strcmp(arr, rhs.arr))
            return true;

        return false;
    }
};

void Run()
{
    Type a{ "Hello World" };
    Type b{ "Hello World" };
    Type c{ "Hello World!!" };

    std::cout << "(a == b) " << (a == b) << '\n'; // 1
    std::cout << "(a != b) " << (a != b) << '\n'; // 0

    std::cout << '\n';

    std::cout << "(a == c) " << (a == c) << '\n'; // 0
    std::cout << "(a != c) " << (a != c) << '\n'; // 1
}

END_NS

BEGIN_NS(Case03)

// 배열의 길이를 나타내는 변수를 하나 둔다면 operator== 작업을 특수화(?)하여 구현하지 않아도 된다.
// 이처럼 메모리를 희생해서 성능을 개선하는 것도 가능하다.

struct Type
{
    size_t len;
    char   arr[20]{ };

    auto operator<=>(const Type& rhs) const = default;
};

void Run()
{
    Type a{ 11, "Hello World" };
    Type b{ 11, "Hello World" };
    Type c{ 13, "Hello World!!" };

    std::cout << "(a == b) " << (a == b) << '\n'; // 1
    std::cout << "(a != b) " << (a != b) << '\n'; // 0

    std::cout << '\n';

    std::cout << "(a == c) " << (a == c) << '\n'; // 0
    std::cout << "(a != c) " << (a != c) << '\n'; // 1
}

END_NS

int main()
{
    // Case01::Run();
    // Case02::Run();
    Case03::Run();

    return 0;
}
