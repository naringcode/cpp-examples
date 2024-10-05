#include <iostream>
#include <memory>

// unique_ptr : 데이터의 소유권이 한 곳에 속할 경우에 사용하는 스마트 포인터

int main()
{
    using namespace std;

    std::unique_ptr<int> ptrA = std::make_unique<int>(5);
    std::unique_ptr<int> ptrB;

    // Copy X, Move O

    // unique_ptr은 복사하는 것이 불가능하다.
    // ptrB = ptrA;

    // unique_ptr은 소유권을 이전하는 방식으로 동작한다.
    ptrB = std::move(ptrA);

    if (nullptr != ptrA)
    {
        std::cout << "ptrA : " << *ptrA << '\n';
    }
    
    if (nullptr != ptrB)
    {
        std::cout << "ptrB : " << *ptrB << '\n';
    }

    return 0;
}
