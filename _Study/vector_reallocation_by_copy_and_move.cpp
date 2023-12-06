#include <iostream>
#include <vector>
#include <memory>

// https://www.youtube.com/watch?v=g65_5ep-kZw
// 7분 50초 부근

// https://stackoverflow.com/questions/72237427/move-elements-while-reallocation-elements-in-vector
// noexcept, reserve()

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
// Case 4)
//     복사 연산 추가(without noexcept)
//     이동 연산 추가(with noexcept)
//     -> 이동 연산이 수행됨
// 
// Case 5)
//     복사 연산 추가(with noexcept)
//     이동 연산 추가(with noexcept)
//     -> 이동 연산이 수행됨
//
// -결론-
// 1. 복사 연산을 만들지 않고 이동 연산만 정의하면 이동 연산이 수행됨
// 2. 이동 연산에 noexcept를 걸면 복사 연산을 정의했어도 이동 연산이 수행됨
//
// - 사라질 객체는 소유권을 위임하는 방식으로 가도 충분함
// - 보편 참조가 일어날 때는 Perfect Forwarding 방식을 적용해서 이동 연산이 일어나게 유도

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
