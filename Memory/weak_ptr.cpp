#include <iostream>
#include <memory>

// https://en.cppreference.com/w/cpp/memory/weak_ptr
// https://learn.microsoft.com/ko-kr/cpp/standard-library/weak-ptr-class?view=msvc-170

// weak_ptr : shared_ptr의 참조 카운트에 영향을 받지 않는 스마트 포인터
//
// weak_ptr은 shared_ptr이나 다른 weak_ptr을 통해서 자원을 할당 받는다.
// weak_ptr은 순환 참조 문제를 해결할 때 유용하다.
// 또는 객체가 파괴되었는지 조회할 때도 유용하다(외부에서 nullptr로 설정해주지 않아도 됨).
//
// weak_ptr은 자원에 접근할 때 lock()이 선행되어야 한다.

int main()
{
    using namespace std;

    shared_ptr<int> sptr = std::make_shared<int>(10);
    weak_ptr<int>   wptr;// = sptr;

    wptr = sptr;

    // expired()로 체크하는 방법
    if (false == wptr.expired())
    {
        auto ptr = wptr.lock();

        *ptr = 100;
    }

    // lock()로 체크하는 방법
    {
        auto ptr = wptr.lock();
        if (nullptr != ptr)
        {
            *ptr = 200;
        }
    }

    cout << "sptr use_count() : " << sptr.use_count() << " / value " << *sptr << '\n';
    cout << "wptr use_count() : " << wptr.use_count() << '\n';

    // 소유권 박탈
    sptr = nullptr;
    // sptr.reset();

    cout << "sptr use_count() : " << sptr.use_count() << '\n';
    cout << "wptr use_count() : " << wptr.use_count() << '\n';

    // expired()로 체크하는 방법
    if (false == wptr.expired())
    {
        auto ptr = wptr.lock();

        *ptr = 100;
    }

    // lock()로 체크하는 방법
    {
        auto ptr = wptr.lock();
        if (nullptr != ptr)
        {
            *ptr = 200;
        }
    }

    return 0;
}
