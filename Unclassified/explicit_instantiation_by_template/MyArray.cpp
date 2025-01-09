#include "MyArray.h"

#include <iostream>

template <typename T>
void MyArray<T>::Print()
{
    for (size_t idx = 0; idx < 10; idx++)
    {
        std::cout << _data[idx] << ' ';
    }

    std::cout << '\n';
}

// 명시적 인스턴스화
// https://learn.microsoft.com/ko-kr/cpp/cpp/explicit-instantiation?view=msvc-170

// explicit instantiation
// 멤버 함수만 명시적으로 인스턴스화하는 방법
// int 타입과 char 타입에 대한 Print()가 있다고 알림
// template void MyArray<int>::Print();
// template void MyArray<char>::Print();

// 클래스 자체를 명시적으로 인스턴스화하는 방법(위의 방법은 멤버 함수마다 하나씩 만들어줘야 함)
template class MyArray<int>;
template class MyArray<char>;
