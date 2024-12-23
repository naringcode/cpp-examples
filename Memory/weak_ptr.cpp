// Update Date : 2024-12-23
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <memory>

// https://en.cppreference.com/w/cpp/memory/weak_ptr
// https://learn.microsoft.com/ko-kr/cpp/standard-library/weak-ptr-class?view=msvc-170

// weak_ptr는 shared_ptr의 관리 객체의 참조 카운팅에 영향을 주지 않는 포인터이며,
// 내부에서 컨트롤 블록의 약한 참조(weak ptr | _Weaks)를 통해서 관리된다.
//
// weak_ptr은 shared_ptr이나 다른 weak_ptr을 통해서 생성된다.
// weak_ptr은 shared_ptr 사이에서 발생하는 순환 참조 문제를 해결할 때 유용하다.
// 또한 관리 객체가 파괴되었는지 조회할 때도 유용하게 사용할 수 있다(관리 객체의 파괴가 컨트롤 블록의 파괴를 의미하는 것은 아니니까 주의할 것).
//
// weak_ptr은 자원에 접근할 때 lock()이 선행되어야 한다.
// 이때 자원이 유효하지 않다면 lock()이 반환하는 shared_ptr은 빈 스마트 포인터가 된다.
//

int main()
{
    using namespace std;

    shared_ptr<int> sptr = std::make_shared<int>(10);
    weak_ptr<int>   wptr;// = sptr;

    wptr = sptr;

    // !! 멀티스레딩 환경 주의 !!
    // 
    // 멀티스레드 환경일 경우 expired()로 체크한 이후 lock()을 건다면 그 사이에 레이스 컨디션이 발생해서 자원이 소멸할 수 있다.
    // 다만 lock()을 거는 것 자체는 스레드 안전하기 때문에 lock()을 건 이후 nullptr 체크로 자원의 유효성을 검증하는 과정이 필요하다.
    //

    // expired()로 체크하는 방법(스레드 안전하지 않음)
    if (false == wptr.expired())
    {
        auto ptr = wptr.lock();

        // expired()로 체크했다고 해도 lock()을 거는 사이에 자원이 유효하지 않게 되면?
        // 이 상황에서는 lock()이 빈 스마트 포인터를 반환한다.
        *ptr = 100;
    }

    // lock()로 체크하는 방법(스레드 안전함)
    {
        auto ptr = wptr.lock();

        // lock()은 스레드 안전하게 자원의 유효성을 판별한다.
        // 따라서 사용하는 측은 이게 빈 스마트 포인터인지 검증하는 작업이 필요하다.
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
