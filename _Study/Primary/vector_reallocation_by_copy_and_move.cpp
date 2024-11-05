// Update Date : 2024-10-07
// Program : Visual Studio 2022
// Version : C++14
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <vector>
#include <memory>

// https://www.youtube.com/watch?v=g65_5ep-kZw
// 7분 50초 부근

// https://stackoverflow.com/questions/72237427/move-elements-while-reallocation-elements-in-vector
// noexcept, reserve()

/**
 * 이동 연산(Move Semantics)에 noexcept를 적용하는 것은 권장 사항이지만 가능하면 붙여주는 것이 좋다.
 * 이건 함수가 예외를 던지지 않는다는 일종의 키워드로 컴파일러에게 알려주는 역할을 한다.
 * 
 * C++의 STL에서 많은 컨테이너 자료구조는 예외 안정성을 보장한 경우에만 Move Semantics를 적용한다.
 * 이 말은 객체의 Move Semantics에 예외를 던지지 않는 noexcept가 적용되어 있어야 한다는 의미이다.
 * 
 * 예를 들어 std::vector에 저장된 객체의 Move Semantics에 noexcept가 적용되어 있지 않다면
 * Reallocation과 같은 원소 재배치 과정이 일어났을 때 "COPY" 연산을 사용한다.
 */
// Reallocation 테스트 진행해본 결과
// Case 1)
//     복사 연산 제거
//     이동 연산 추가
//     -> 이동 연산이 수행됨
//
// Case 2)
//     복사 연산 추가
//     이동 연산 제거
//     -> 복사 연산이 수행됨
//
// Case 3)
//     복사 연산 추가(with noexcept)
//     이동 연산 추가(without noexcept)
//     -> 복사 연산이 수행됨
// 
// Case 4) ★
//     복사 연산 추가(without noexcept)
//     이동 연산 추가(with noexcept)
//     -> 이동 연산이 수행됨
// 
// Case 5) ★
//     복사 연산 추가(with noexcept)
//     이동 연산 추가(with noexcept)
//     -> 이동 연산이 수행됨
//
// ## 결론 ##
// 1. Copy Semantics를 만들지 않고 Move Semantics만 정의했으면 Move 연산이 수행됨.
// 2. Move Semantics에 noexcept를 적용하면 Reallocation 과정에서 Copy Semantics가 아닌 Move 연산을 수행함.
//
// - 사라질 객체, 임시 객체가 속하는 RValue 유형은 소유권을 위임하는 Move Semantics을 적용하도록 할 것.
// - 보편 참조를 적용하여 완벽한 전달(Perfert Forwarding) 방식을 적용하여 Move Semantics를 구현해도 됨.
//
class MyClass
{
public:
    MyClass(int data)
        : _data(data)
    {
        std::cout << "MyClass() : " << _data << '\n';
    }

public:
    // NO COPY
    // MyClass(const MyClass& rhs) = delete;
    // MyClass& operator=(const MyClass& rhs) = delete;

    MyClass(const MyClass& rhs) noexcept
        : _data(rhs._data + 100)
    {
        std::cout << "MyClass(const MyClass& rhs) : " << rhs._data << "->" << _data << '\n';
    }

    MyClass& operator=(const MyClass& rhs) noexcept
    {
        _data = rhs._data + 100;

        std::cout << "MyClass& operator=(const MyClass& rhs) : " << rhs._data << "->" << _data << '\n';
    }


public:
    MyClass(const MyClass&& rhs) noexcept
        : _data(rhs._data + 100)
    {
        std::cout << "MyClass(const MyClass&& rhs) : " << rhs._data << "->" << _data << '\n';
    }
    
    MyClass& operator=(const MyClass&& rhs) noexcept
    {
        _data = rhs._data + 100;
    
        std::cout << "MyClass& operator=(const MyClass&& rhs) : " << rhs._data << "->" << _data << '\n';
    }

private:
    int _data;
};

int main()
{
    using namespace std;

    vector<MyClass> vec;

    // 재할당 연산이 한 번만 일어나게 설정
    vec.reserve(8);

    for (int i = 0; i < 10; i++)
    {
        // 둘 다 && 연산이 일어남(r-value)
        // vec.push_back(std::move(MyClass{ i }));
        vec.push_back(MyClass{ i });
    }

    return 0;
}
