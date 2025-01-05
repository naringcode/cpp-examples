// Update Date : 2024-12-26
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <memory>

// 순서대로 볼 것
// 
// # shared_ptr을 사용할 경우 알아야 할 기본적인 내용
// 1. shared_ptr_with_deleter.cpp
// 2. shared_ptr_with_allocator.cpp
// 3. shared_ptr_with_deleter_and_allocator.cpp
// 4. (중요) shared_ptr_details.cpp (SFINAE 내용 포함)
// 
// # weak_ptr의 유효성 검증 로직에 대한 내용
// 5. weak_ptr.cpp <-----
// 6. weak_ptr_details.cpp
//
// # shared_ptr의 관리 객체에서 자신을 반환할 때 필요한 내용
// 7. enable_shared_from_this.cpp
// 8. enable_shared_from_this_details.cpp
// 9. enable_shared_from_this_examples.cpp
//
// # shared_ptr을 멀티스레딩 환경에서 사용할 때 발생할 수 있는 문제점을 기술한 내용
// 10. allocation_aligned_byte_boundaries.cpp(사전지식)
// 11. volatile_atomic_cache_coherence_and_memory_order.cpp(사전지식)
// 12. (중요) shared_ptr_multi_threading_issues.cpp

// https://en.cppreference.com/w/cpp/memory/weak_ptr
// https://learn.microsoft.com/ko-kr/cpp/standard-library/weak-ptr-class?view=msvc-170

// weak_ptr는 shared_ptr의 관리 객체의 참조 카운팅에 영향을 주지 않는 포인터이며,
// 내부에서 컨트롤 블록의 약한 참조(weak refs | _Weaks)를 통해서 관리된다.
//
// weak_ptr은 shared_ptr이나 다른 weak_ptr을 통해서 생성된다.
// weak_ptr은 shared_ptr 사이에서 발생하는 순환 참조 문제를 해결할 때 유용하다.
// 또한 관리 객체가 파괴되었는지 조회할 때도 유용하게 사용할 수 있다(관리 객체의 파괴가 컨트롤 블록의 파괴를 의미하는 것은 아니니까 주의할 것).
//
// weak_ptr은 자원에 접근할 때 lock()이 선행되어야 한다.
// 이때 자원이 유효하지 않다면 lock()이 반환하는 shared_ptr은 빈 스마트 포인터가 된다.

int main()
{
    using namespace std;

    shared_ptr<int> sptr = std::make_shared<int>(10);
    weak_ptr<int>   wptr;// = sptr;

    // shared_ptr은 다양한 방식으로 생성할 수 있지만 weak_ptr은 이미 생성된 스마트 포인터를 받는 방식으로 동작한다.
    // 아래 둘 중 어떤 방식으로 받든 관리 객체의 포인터와 컨트롤 블록의 포인터를 복사한다.
    // 1. shared_ptr을 받는 방식
    // 2. 다른 weak_ptr로부터 받는 방식
    //
    // weak_ptr은 이미 존재하고 있는 스마트 포인터를 기반으로 생성하기 때문에 생성자가 단순한 편이다.
    // shared_ptr든 weak_ptr든 두 스마트 포인터는 _Ptr_base를 상속받아 구현된 형태이며,
    // weak_ptr의 생성은 이 부모 클래스(_Ptr_base<T>)를 기반으로 이루어진다.

    // !! 멀티스레딩 환경 주의 !!
    // 
    // 멀티스레드 환경일 경우 expired()로 체크한 이후 lock()을 건다면 그 사이에 레이스 컨디션이 발생해서 자원이 소멸할 수 있다.
    // 다만 lock()을 거는 것 자체는 스레드 안전하기 때문에 lock()을 건 이후 nullptr 체크로 자원의 유효성을 검증하는 과정이 필요하다.

    // shared_ptr이 애초에 강한 참조를 들고 있는 형태이기 때문에 스레드 안전하다.
    // !! shared_ptr<T>&나 shared_ptr<T>*의 형태로 다른 스레드와 스마트 포인터를 공유하고 있다면? !!
    // !! 이런 경우라면 스레드 안전하지 않지만 이건 애초에 코딩을 잘못한 것이니 논외로 함. !!
    wptr = sptr;

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
        // 따라서 사용하는 측은 이게 유효한 스마트 포인터인지 검증하는 작업이 필요하다.
        if (nullptr != ptr)
        {
            *ptr = 200;
        }
    }

    // expired()로 빠르게 유효성만 검증하고 lock()을 통해 객체에 접근하는 방법(스레드 안전함)
    if (false == wptr.expired())
    {
        auto ptr = wptr.lock();
        
        if (nullptr != ptr)
        {
            *ptr = 300;
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

    // expired()로 빠르게 유효성만 검증하고 lock()을 통해 객체에 접근하는 방법
    if (false == wptr.expired())
    {
        auto ptr = wptr.lock();
        
        if (nullptr != ptr)
        {
            *ptr = 300;
        }
    }

    return 0;
}
