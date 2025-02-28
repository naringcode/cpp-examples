// Update Date : 2025-02-26
// OS : Windows 10 64bit
// Program : Visual Studio 2022, vscode(gcc-14.2.0)
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
// 4. default_three-way_comparison_operators.cpp <-----
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

// https://en.cppreference.com/w/cpp/language/default_comparisons#Three-way_comparisons
// https://en.wikipedia.org/wiki/Rewriting

#define BEGIN_NS(name) namespace name {
#define END_NS }

BEGIN_NS(Case01)

// C++20부터는 operator<=>와 operator==에는 표현식 재작성 규칙(혹은 합성 비교)이 적용되어 표현식을 재구성하여 코드를 컴파일한다.
// 관계 연산자의 경우에는 operator<=>를 기반으로, 동등 연산자의 경우에는 operator==를 기반으로 코드를 재구성한다.
//
// 관계 연산 "x @ y"를 operator<=>로 재구성했을 경우 해당 작업은 ordering 타입을 반환하는데
// 컴파일러는 이 값을 리터럴 0과 비교할 수 있게 "x <=> y @ 0"과 같은 형태로 코드를 컴파일한다.
//
// 표현식 재작성 규칙은 다음 규칙을 따른다.
// 
// 1. x == y : y == x
// 2. x != y : !(x == y), !(y == x)
// 3. x < y  : ((x <=> y) < 0), ((y <=> x) > 0)
// 4. x > y  : ((x <=> y) > 0), ((y <=> x) < 0)
// 5. x <= y : ((x <=> y) <= 0), ((y <=> x) >= 0)
// 6. x >= y : ((x <=> y) >= 0), ((y <=> x) <= 0)
//

struct Number
{
    int num = 0;

    constexpr Number() = default;
    constexpr Number(int num)
        : num{ num }
    { }

    // 두 코드 다 테스트해보는 것을 추천한다.
    // friend auto operator<=>(const Number& lhs, const Number& rhs) = default;
    auto operator<=>(const Number& rhs) const = default;

    // operator<=>를 default로 정의하면 constexpr과 noexcept를 구성하지 않아도 해당 두 옵션이 알아서 적용된다.
};

void Run()
{
    Number x{ 10 };
    Number y{ 20 };

    // (중요) 최적화 옵션을 끄고 디스어셈블러를 통해 조회할 것!

    // 아래 관계 연산자를 디스어셈블러를 통해 조회해보면 opeator<=>를 쓰는 것을 볼 수 있다.

    // 관계 연산자(Relational Operators) : 디스어셈블러를 통해 조회하면 operator<=> 이후 사용한 관계 심볼에 따른 std::operator@를 호출한다.
    // operator<=>는 리터럴 0과 비교 가능한 값을 반환하고 이걸 std::operator@로 비교하는 것이다.
    auto relComp1 = (x < y);  // call Number::operator<=> -> call std::operator<
    auto relComp2 = (x > y);  // call Number::operator<=> -> call std::operator>
    auto relComp3 = (x <= y); // call Number::operator<=> -> call std::operator<=
    auto relComp4 = (x >= y); // call Number::operator<=> -> call std::operator>=


    // 동등 연산자(Equality Operators) : 디스어셈블러를 통해 조회해보면 operator==를 기반으로 하는 것을 볼 수 있다.
    // operator<=>를 default로 정의하면 컴파일러 차원에서 operator==도 함께 정의해준다.
    // 사용자가 정의하지 않은 operator를 정의해주기 때문에 이를 Synthesized expressions이라 한다.
    auto eqComp1 = (x == y); // call Number::operator==
    auto eqComp2 = (x != y); // call Number::operator==

    // auto eqComp1 = (x == y); : call Number::operator== -> 결과를 0 혹은 1로 저장한 다음...
    // call        Case01::Number::operator==
    // mov         byte ptr [eqComp1],al <----- 계산한 결과를 그대로 반영

    // auto eqComp2 = (x != y); : call Number::operator== -> 결과를 0 혹은 1로 저장한 다음...
    // call        Case01::Number::operator==
    // movzx       eax,al  
    // test        eax,eax  
    // jne         __$EncStackInitStart+1D2h
    // mov         dword ptr [rbp+514h],1  
    // jmp         __$EncStackInitStart+1DCh
    // mov         dword ptr [rbp+514h],0  
    // movzx       eax,byte ptr [rbp+514h]  
    // mov         byte ptr [eqComp2],al <----- 계산한 결과를 분기에 따라 가공한 다음 저장

    // operator<=>를 default로 정의하면 constexpr과 noexcept를 구성하지 않아도 해당 두 옵션이 알아서 적용된다.
    constexpr Number num1{ 10 };
    constexpr Number num2{ 20 };

    // 아래 비교 연산은 컴파일 에러가 발생하지 않는다(constexpr 연산 가능).
    constexpr auto ls = num1 < num2;
    constexpr auto eq = num1 == num2;
    constexpr auto gt = num1 > num2;

    // operator<=>를 noexcept로 만들지 않았지만 noexcept 방식을 사용하는 것도 가능하다.
    // static_assert(noexcept(operator<=>(x, y)) == true);
    // static_assert(noexcept(operator<=>(num1, num2)) == true);
    static_assert(noexcept(x.operator<=>(y)) == true);
    static_assert(noexcept(num1.operator<=>(num2)) == true);

    // MSVC와 GCC에서 constexpr과 noexcept가 적용되는지 테스트해봤을 때 둘 다 잘 컴파일된다.
}

END_NS

BEGIN_NS(Case02)

// 디스어셈블러를 통해 컴파일된 코드를 확인하면 표현식 재작성 규칙에 따라 명령어가 생성된 것을 볼 수 있다.
// 이를 토대로 암묵적인 형변환을 받는 경우도 확인해 보자.
//
// 어떠한 클래스를 기반으로 하는 obj 변수가 있고 해당 클래스의 생성자가 받을 수 있는 값 x가 있다고 해보자.
// 이 경우 코드는 다음과 같이 컴파일된다.
// 
// 1. x == obj : obj == x
// 2. x != obj : !(obj == x)
// 3. x < obj : ((obj <=> x) > 0)
// 4. x > obj : ((obj <=> x) < 0)
// 5. x <= obj : ((obj <=> x) >= 0)
// 6. x >= obj : ((obj <=> x) <= 0)
//

struct Number
{
    int num = 0;

    constexpr Number() = default;
    constexpr Number(int num)
        : num{ num }
    { }

    auto operator<=>(const Number& rhs) const = default;
};

void Run()
{
    Number x{ 10 };
    Number y{ 20 };

    // 코드가 컴파일되면 주석에 적힌 내용을 토대로 명령을 수행한다.
    std::cout << "x < y  : " << (x < y) << '\n';  // (x <=> y) < 0 : 1 
    std::cout << "x > y  : " << (x > y) << '\n';  // (x <=> y) > 0 : 0
    std::cout << "x <= y : " << (x <= y) << '\n'; // (x <=> y) <= 0 : 1
    std::cout << "x >= y : " << (x >= y) << '\n'; // (x <=> y) >= 0 : 0
    std::cout << "x == y : " << (x == y) << '\n'; // (x == y) : 0
    std::cout << "x != y : " << (x != y) << '\n'; // !(x == y) : 1

    std::cout << "-------------------------\n";

    // x와 정수 5를 비교하면 비교 연산자를 num1.operator<=>(5)로 받고 반환된 결과를 리터럴 0과 비교한다.
    // 이 과정에서 operator<=>의 인자에 해당하는 5는 Number로 받을 수 있기 때문에 암묵적으로 5를 Number로 변환한다.
    std::cout << "x < 5  : " << (x < 5) << '\n';  // (x <=> 5) < 0 : 0
    std::cout << "x > 5  : " << (x > 5) << '\n';  // (x <=> 5) > 0 : 1
    std::cout << "x <= 5 : " << (x <= 5) << '\n'; // (x <=> 5) <= 0 : 0
    std::cout << "x >= 5 : " << (x >= 5) << '\n'; // (x <=> 5) >= 0 : 1
    std::cout << "x == 5 : " << (x == 5) << '\n'; // (x == 5) : 0
    std::cout << "x != 5 : " << (x != 5) << '\n'; // !(x == 5) : 1

    std::cout << "-------------------------\n";

    // 15.operator<=>(num2)를 사용하는 것은 불가능하다.
    // 이 경우 컴파일러는 해당 표현식을 재작성 규칙에 따라 num2.operator<=>(15)로 변경한다.
    std::cout << "15 < y  : " << (15 < y) << '\n';  // (y <=> 15) > 0 : 1
    std::cout << "15 > y  : " << (15 > y) << '\n';  // (y <=> 15) < 0 : 0
    std::cout << "15 <= y : " << (15 <= y) << '\n'; // (y <=> 15) >= 0 : 1
    std::cout << "15 >= y : " << (15 >= y) << '\n'; // (y <=> 15) <= 0 : 0
    std::cout << "15 == y : " << (15 == y) << '\n'; // (y == 15) : 0
    std::cout << "15 != y : " << (15 != y) << '\n'; // !(y == 15) : 1

    // C++20 이전에는 비교 연산자를 클래스의 멤버 함수로 작성했을 경우 후자의 코드는 컴파일 에러가 발생했다.
    // 하지만 C++20부터는 연산자 재작성 규칙 덕분에 이런 방식으로 코드를 작성하는 것이 가능하다.
}

END_NS

BEGIN_NS(Case03)

// Three-way 비교 연산자를 default로 정의하면 첫 번째 멤버부터 마지막 멤버까지 차례차례 비교를 진행하며,
// 각 비교의 대소 관계는 사전 순서(lexicographical)를 따른다.
//
// - 일반 값 : valueA @ valueB
// - 배열   : arrayA[n] @ arrayB[n] (lexicographically)
// - 포인터 : ptrA @ ptrB (주소 간 비교, 포인터 대상이 배열로 할당되어도 마찬가지)
// - 상속된 멤버 변수의 경우 : baseA @ baseB (위에 나온 항목을 토대로 비교)
// - 멤버 변수의 경우 : subA @ subB (위에 나온 항목을 토대로 비교)
//

struct Type
{
    int  value{ };
    char array[8]{ };
    int* ptr{ };

    auto operator<=>(const Type& rhs) const = default;
};

void Run()
{
    Type a{ .value = 100, .array{ "C++" },  .ptr = reinterpret_cast<int*>(0x0100) };
    Type b{ .value = 100, .array{ "C++" },  .ptr = reinterpret_cast<int*>(0x0100) };
    Type c{ .value = 100, .array{ "Java" }, .ptr = reinterpret_cast<int*>(0x0100) };
    Type d{ .value = 100, .array{ "C++" },  .ptr = reinterpret_cast<int*>(0x0050) };

    std::cout << "{ 100, \"C++\", 0x0100 } @ { 100, \"C++\", 0x0100 }\n";

    std::cout << "(a < b)  " << (a < b)  << '\n'; // 0
    std::cout << "(a > b)  " << (a > b)  << '\n'; // 0
    std::cout << "(a <= b) " << (a <= b) << '\n'; // 1
    std::cout << "(a >= b) " << (a >= b) << '\n'; // 1

    std::cout << "(a == b) " << (a == b) << '\n'; // 1
    std::cout << "(a != b) " << (a != b) << '\n'; // 0

    std::cout << '\n';
    
    std::cout << "{ 100, \"C++\", 0x0100 } @ { 100, \"Java\", 0x0100 }\n";

    std::cout << "(a < c)  " << (a < c)  << '\n'; // 1
    std::cout << "(a > c)  " << (a > c)  << '\n'; // 0
    std::cout << "(a <= c) " << (a <= c) << '\n'; // 1
    std::cout << "(a >= c) " << (a >= c) << '\n'; // 0

    std::cout << "(a == c) " << (a == c) << '\n'; // 0
    std::cout << "(a != c) " << (a != c) << '\n'; // 1

    std::cout << '\n';

    std::cout << "{ 100, \"C++\", 0x0100 } @ { 100, \"C++\", 0x0050 }\n";

    std::cout << "(a < d)  " << (a < d)  << '\n'; // 0
    std::cout << "(a > d)  " << (a > d)  << '\n'; // 1
    std::cout << "(a <= d) " << (a <= d) << '\n'; // 0
    std::cout << "(a >= d) " << (a >= d) << '\n'; // 1

    std::cout << "(a == d) " << (a == d) << '\n'; // 0
    std::cout << "(a != d) " << (a != d) << '\n'; // 1
}

END_NS

BEGIN_NS(Case04)

// 상속하여 구현한 객체에도 operator<=>가 정의되어 있고 부모 클래스에도 operator<=>가 정의되어 있으며
// 두 operator<=>가 모두 default로 적용된 상태라면 비교의 우선권은 부모 쪽에 있다.

struct Base
{
    int  value{ };
    char array[8]{ };
    int* ptr{ };

    // 부모 쪽에 operator<=>를 default로 정의
    auto operator<=>(const Base& rhs) const = default;
};

struct Derived : Base
{
    float num{ };

    // 상속한 쪽의 operator<=>도 default로 정의
    auto operator<=>(const Derived& rhs) const = default;
};

void Run()
{
    Derived a{ 100, "C++",  reinterpret_cast<int*>(0x100), 3.14f };
    Derived b{ 100, "Java", reinterpret_cast<int*>(0x100), 6.28f };
    Derived c{ 100, "Java", reinterpret_cast<int*>(0x100), 1.57f };
    Derived d{ 100, "C++",  reinterpret_cast<int*>(0x100), 6.28f };
    Derived e{ 100, "C++",  reinterpret_cast<int*>(0x100), 1.57f };
    
    std::cout << "{ 100, \"C++\", 0x0100, 3.14f } @ { 100, \"Java\", 0x0100, 6.28f }\n";

    std::cout << "(a < b)  " << (a < b)  << '\n'; // 1
    std::cout << "(a > b)  " << (a > b)  << '\n'; // 0
    std::cout << "(a <= b) " << (a <= b) << '\n'; // 1
    std::cout << "(a >= b) " << (a >= b) << '\n'; // 0

    std::cout << "(a == b) " << (a == b) << '\n'; // 0
    std::cout << "(a != b) " << (a != b) << '\n'; // 1

    std::cout << '\n';
    
    std::cout << "{ 100, \"C++\", 0x0100, 3.14f } @ { 100, \"Java\", 0x0100, 1.57f }\n";

    std::cout << "(a < c)  " << (a < c)  << '\n'; // 1
    std::cout << "(a > c)  " << (a > c)  << '\n'; // 0
    std::cout << "(a <= c) " << (a <= c) << '\n'; // 1
    std::cout << "(a >= c) " << (a >= c) << '\n'; // 0

    std::cout << "(a == c) " << (a == c) << '\n'; // 0
    std::cout << "(a != c) " << (a != c) << '\n'; // 1

    std::cout << '\n';

    std::cout << "{ 100, \"C++\", 0x0100, 3.14f } @ { 100, \"C++\", 0x0100, 6.28f }\n";

    std::cout << "(a < d)  " << (a < d)  << '\n'; // 1
    std::cout << "(a > d)  " << (a > d)  << '\n'; // 0
    std::cout << "(a <= d) " << (a <= d) << '\n'; // 1
    std::cout << "(a >= d) " << (a >= d) << '\n'; // 0

    std::cout << "(a == d) " << (a == d) << '\n'; // 0
    std::cout << "(a != d) " << (a != d) << '\n'; // 1

    std::cout << '\n';

    std::cout << "{ 100, \"C++\", 0x0100, 3.14f } @ { 100, \"C++\", 0x0100, 1.57f }\n";

    std::cout << "(a < e)  " << (a < e)  << '\n'; // 0
    std::cout << "(a > e)  " << (a > e)  << '\n'; // 1
    std::cout << "(a <= e) " << (a <= e) << '\n'; // 0
    std::cout << "(a >= e) " << (a >= e) << '\n'; // 1

    std::cout << "(a == e) " << (a == e) << '\n'; // 0
    std::cout << "(a != e) " << (a != e) << '\n'; // 1
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
