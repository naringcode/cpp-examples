// Update Date : 2025-01-10
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Release-x64

#include <iostream>
#include <thread>
#include <atomic>
#include <memory>

using namespace std;

#define BEGIN_NS(name) namespace name {
#define END_NS };

// 순서대로 볼 것
// 
// # shared_ptr을 사용할 경우 알아야 할 기본적인 내용
// 1. shared_ptr_with_deleter.cpp
// 2. shared_ptr_with_allocator.cpp
// 3. shared_ptr_with_deleter_and_allocator.cpp
// 4. (중요) shared_ptr_details.cpp (SFINAE 내용 포함)
// 
// # weak_ptr의 유효성 검증 로직에 대한 내용
// 5. weak_ptr.cpp
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
// 12. (중요) shared_ptr_multi_threading_issues.cpp <-----

// https://en.cppreference.com/w/cpp/memory/shared_ptr
// https://en.cppreference.com/w/cpp/memory/weak_ptr
// https://en.cppreference.com/w/cpp/memory/shared_ptr/atomic2
// https://en.cppreference.com/w/cpp/memory/weak_ptr/atomic2
// https://en.cppreference.com/w/cpp/experimental/atomic_shared_ptr
// https://en.cppreference.com/w/cpp/experimental/atomic_weak_ptr
// http://ndcreplay.nexon.com/NDC2017/sessions/NDC2017_0093.html
// https://isocpp.org/files/papers/N4162.pdf
// https://stackoverflow.com/questions/40223599/what-is-the-difference-between-stdshared-ptr-and-stdatomicstdshared-ptr
// https://openmynotepad.tistory.com/90
// https://en.wikipedia.org/wiki/Cache_coherency_protocols_(examples)
// https://hackmd.io/@vesuppi/Syvoiw1f8

// Visual Studio 2022에서 제공하는 C++의 shared_ptr과 weak_ptr을 _Ptr_base를 상속하여 구현하며 해당 클래스는 다음과 같이 구성되어 있다. 
// [ element_type* _Ptr ][ _Ref_count_base* _Rep ]
// 
// _Ptr : 사용 대상이 되는 관리 객체
// _Rep : 컨트롤 블록(레퍼런스 카운팅 및 관리 객체와 컨트롤 블록의 소멸 방식 담당)
// 
// 스마트 포인터의 레퍼런스 카운팅은 원자적으로 동작하기에 이 카운터가 0으로 떨어지는 타이밍 자체는 보장되어 있다.
// 즉, 관리 객체의 소멸 과정과 컨트롤 블록의 해제 자체는 단 한 번만 이루어진다.
// 
// !! (중요) 하지만 !!
// 소멸이나 해제가 한 번만 이루어진다는 것이 보장된다고 해도 스마트 포인터 자체는 해제된 메모리 혹은 갱신된 메모리에 대한 잘못된 접근을 차단하진 않는다.
// 레퍼런스 카운팅 자체는 스레드 안전(?)하게 원자적으로 이루어지지만 사용할 자원에 대한 접근은 스레드 안전하지 않다는 뜻이다.
// 
// 멀티스레딩 환경에서 shared_ptr가 동작하는 방식을 제대로 이해하지 않으면 해제된 혹은 일관성이 틀어진 메모리로의 접근으로 인한 "대형사고"가 발생할 수 있다.
// 자체적인 메모리 풀을 써서 스마트 포인터를 구현해서 사용한다면 예기치 못한 문제가 발생할 확률이 더욱 높아지니 특히 주의해야 한다.


// 멀티스레딩 환경이라고 해도 단일 shared_ptr 변수를 갱신하지 않고 read-only 방식으로 사용할 때는 스레드 안전하다.
// 여기서 말하는 shared_ptr 변수를 대상으로 하는 read-only란 shared_ptr가 담은 내용물(관리 객체, 컨트롤 블록)의 포인터를 다른 대상으로 변경하거나 대체하지 않겠다는 뜻이다.
// !! shared_ptr의 생명주기 중 소멸해야 하는 시점까지 가리키는 대상의 포인터가 바뀌지 않는 것만 보장된다면 관리 객체나 컨트롤 블록의 내용은 변경돼도 상관 없음. !!
// !! read-only가 스마트 포인터를 const로 사용하겠다는 것을 의미하는 것은 아니니까 혼동하면 안 됨. !!
//
// 하나의 shared_ptr가 멀티스레딩 환경에서 공유된다고 해도 "A = B"와 같은 구문을 써서 스마트 포인터 자체를 갱신하는 상황만 피한다면 스레드 안전하게 동작한다.
// !! A가 관리하고 있는 관리 객체와 컨트롤 블록이 있는데 이걸 B의 것으로 교체하는 상황을 의미하는 것임. !!
// 
// 달리 말하자면 공유되고 있는 shared_ptr(복사한 게 아닌 말 그대로 하나의 shared_ptr를 여러 스레드가 공유하고 있는 상황)를 사용할 때
// shared_ptr를 갱신(store)하는 과정과 shared_ptr에 접근해서 사용(load)하는 과정에서 레이스 컨디션이 발생하면 이는 스레드 안전하지 않은 상황이다.
// !! shared_ptr를 갱신하는 도중 다른 스레드에서 해당 shared_ptr를 읽고자 할 때 발생하는 데이터 레이스 문제는 "반드시" 해결해야 함. !!
// !! 복사하여 별개의 변수로 동작하는 shared_ptr을 대상으로 말하는 것이 아닌 공유 변수가 된 특정한 단일 shared_ptr을 대상으로 말하는 것이니 혼동하면 안 됨. !!
// 
// !! 매우 매우 중요한 사항 !!
// shared_ptr가 가리키는 대상을 새로운 것으로 변경하고자 할 때 갱신하는 동안 다른 스레드에서 해당 shared_ptr에 접근하는 것은 위험하다.
// 이러한 상황이 스레드 안전하지 않다는 것은 "반드시 반드시" 숙지해야 한다.


// _Ptr과 _Rep를 묶어서 Double CAS 등을 통해 원자적으로 검증하면 스마트 포인터가 가리키는 대상을 안전하게 교체할 수 있긴 하다.
// 하지만 이렇게 해도 하나의 스마트 포인터가 여러 곳에서 공유되는 상황에 따른 근본적인 문제는 발생하지 않는다.
// !! 애초에 Double CAS는 하드웨어나 운영체제에 의존적인 기능이며 아직까지는 표준 C++에 포함되어 있지 않음. !!
// 
// code)---------------------------------------------
// sptr->MustBeFirst();
// // <----- 이 사이에 sptr이 갱신되었으면?
// sptr->MustBeSecond();
// --------------------------------------------------
// 
// 이건 스마트 포인터의 소유권에 대한 책임을 분명히 하지 않아서 발생한 설계 오류라고 봐야 한다.
// 하나의 스마트 포인터 변수에 경합이 발생하는 상황은 적절하지 않은 설계이며 개선이 필요한 상황이다.
// 
// 여러 객체 혹은 여러 스레드가 하나의 스마트 포인터 변수를 공유하는 상황이면 소유권에 대한 책임이 분명하지 않기에 수명 관리가 복잡해진다.
// 또한 이러한 상황에서 스마트 포인터의 상태나 값을 변경하는 일이 생긴다면 그 과정에서 예기치 못한 동작이 초래될 수 있다.
// 
// 스마트 포인터를 사용할 때는 소유권에 대한 책임을 분명히 하는 것이 좋다.
// 스마트 포인터를 교체하거나 끊어야 하는 상황이 생기면 전역 큐로 처리하거나 스마트 포인터를 소유한 객체에 처리 큐를 따로 두는 것이 나은 설계이다.
// 
// 이 페이지에선 이런 문제의 해결 방법을 논하지 않고 어째서 단순 스마트 포인터의 경합이 안전하지 않은지 그것만 다룰 것이다.
//
// (참고) 스마트 포인터의 관리 객체가 불변임이 보장된다면 동기화 과정 없이도 안전하게 사용할 수 있다.


// 멀티스레딩 환경에서 단일 shared_ptr 변수를 대상으로 갱신(store)하고 읽는(load) 것이 경합되면 스레드 안전하지 않다고 했다.
// 
// shared_ptr에 기존의 값이 있든, 빈 스마트 포인터라 값이 없든 특정 스레드에서 갱신하고 다른 스레드에서 읽는 방식이 경합되면 스레드 안전하지 않다.
// !! 애초에 shared_ptr을 대상으로 읽고 쓰는 데이터 레이스가 발생하는 상황 자체가 발생해선 안 됨. !!
// 
// shared_ptr의 관리 객체만 신경쓰면 안 되고, shared_ptr에 묶인 컨트롤 블록의 레퍼런스 카운팅에 대한 무결성도 고려해야 한다.
// 컴파일러에 따라서, 스마트 포인터의 생성 유형에 따라서 shared_ptr의 관리 객체가 nullptr을 가리킨다고 해도 레퍼런스 카운팅이 이루어질 수도 있다.
// !! 여담이긴 하나 Visual Studio 2022의 C++이 제공하는 스마트 포인터의 컨트롤 블록 중 _Ref_count_resource_alloc을 사용하는 유형은 관리 객체에 nullptr을 대입해도 레퍼런스 카운팅이 이루어짐. !!
// !! 이렇게 지정하면 좀 특이하긴 해도 관리 객체의 소멸 과정에서 Deleter에 nullptr이 넘어옴. !!
// !! "shared_ptr_details.cpp"를 보면 이 유형에 대한 설명이 있으니 참고하도록 함. !!


// 멀티스레딩 환경이라고 해도 shared_ptr가 가리키고 있는 포인터의 대상이 바뀌지 않는다는 것이 보장된다면
// 여러 스레드에서 지지고 볶든, 여러 스레드에서 읽히거나 복사되어도 문제가 되지 않는다(이 경우 레퍼런스 카운트가 깨질 일이 없음).
// !! shared_ptr 변수 자체를 갱신(store)하는 도중에 접근(load)해서 사용하는 것이 문제임(너무 너무 중요하기에 여러 번 강조함). !!


// shared_ptr를 다른 스마트 포인터로 갱신하려고 하면 다음 과정에 직면하게 된다.
// !! 간략화해서 표현한 것이기 때문에 자세한 건 "shared_ptr_details.cpp"를 보도록 함. !!
// 
// sptrA = sptrB;
// 
// 1. sptrA의 레퍼런스 카운트를 감소시키고 해당 카운트가 0이 되면 스마트 포인터의 자원을 해제함.
// 2. sptrB의 레퍼런스 카운트를 증가시킴.
// 3. shared_ptr 교체 작업을 진행함.
//   3-1. sptrA의 관리 객체와 컨트롤 블록을 sptrB의 것으로 교체함.
// 
// 컴파일러 구성에 따라서 위 순서는 섞일 수 있다.
// 이런 이유로 멀티스레딩 환경에서 원자적으로 shared_ptr의 스레드 안전성을 보장하기 위해선 명시적인 동기화 작업이 필요하다.
// !! 특정 컴파일러에 의존하는 동기화 방식을 구성해선 안 됨. !!


// shared_ptr이 원자성을 보장하나는 부분은 컨트롤 블록 _Rep에서 진행하는 레퍼런스 카운팅이다.
// 
// class __declspec(novtable) _Ref_count_base // common code for reference counting
// {
//     ...
//     _Atomic_counter_t _Uses  = 1; // 관리 객체의 수명
//     _Atomic_counter_t _Weaks = 1; // 컨트롤 블록의 수명
//     ...
// };
// 
// 위 _Uses와 _Weaks가 0으로 떨어지는 것은 원자적이기 때문에 수명이 다 했을 때 소멸하는 로직은 단 한 번만 진행된다.
// 
// 컨트롤 블록 _Rep에서 레퍼런스 카운팅을 원자적으로 처리한다고 해도 관리 객체에 해당하는 _Ptr은 원자적으로 동작하지 않는다.
// !! 스마트 포인터의 관리 객체에 대한 접근 및 해당 관리 객체의 멤버에 대한 동시적인 접근은 여전히 데이터 레이스가 발생함. !!
// 
// shared_ptr을 통해 관리 객체 _Ptr에 접근하는 건 단순한 메모리 참조일 뿐이며 이것 자체는 레퍼런스 카운팅에 영향을 미치지 않는다.
// (중요) shared_ptr의 멀티스레딩 이슈는 여기서 기인하니까 잊으면 안 되며 이 사실을 명확하게 인지하고 있어야 한다.


// https://en.cppreference.com/w/cpp/memory/shared_ptr/atomic2
// 이러한 이유들로 인해 C++20부터는 멀티스레딩 환경에 안전한 atomic<shared_ptr<T>>를 템플릿 특수화하여 제공하고 있다.
// !! shared_ptr<T>를 받는 atomic<T>가 아닌 말 그대로 자료형 자체를 템플릿 특수화하여 별도로 구성함. !!
// !! 찾아보면 atomic_shared_ptr<T>도 있긴 한데 이건 deprecated 상태니까 무시해도 됨. !!
// 
// 아래 코드는 Visual Studio 2022 기준 MSVC에서 제공하는 atomic<shared_ptr<T>>에 대한 코드이다.
// !! 다른 컴파일러를 통해서 해당 내용을 분석하면 다르게 나옴(GCC 13.1.0으로 확인하면 atomic<shared_ptr<T>>를 상속하지 않고 구현함). !!
// 
// ## 바이트 경계(?) ##
// 16바이트 경계를 맞추는 이유를 모르겠어서 GCC에서 구현된 방식도 확인했는데 여긴 8바이트 경계를 사용한다.
// 혹시나 해서 공식 문서도 찾아봤는데 역시나 바이트 경계에 대해 명시된 부분이 없었다.
// Double CAS를 사용하나 싶어서 봤더니 이것도 아니었다.
// atomic<shared_ptr<_Ty>>를 실제로 사용할 때 크기를 조회하면 16바이트가 나온다.
// 추정이긴 하나 MSVC에선 Windows 플랫폼의 성능 최적화를 위해서 명시적으로 16바이트 경계를 맞춘 것 같다고 생각하자.
// 전체적으로 분석했을 때 8바이트 경계로 맞춰서 사용해도 큰 문제는 없다고 판단했지만 그게 아니면 설명이 되지 않는다.
// !! 메모리 정렬의 경계는 컴파일러, 운영체제, 플랫폼에 따라서 기본으로 지정되어 값이 다름. !!
// !! Windows 64비트 환경에서 Visual Studio 2022로 동적할당을 진행하면 8바이트가 아닌 16바이트 경계에 맞춰서 메모리를 할당함. !!
// 
// 바이트 경계에 대해선 너무 크게 신경쓰지 말고 분석하자.
// 
// !! atomic<shared_ptr<_Ty>>를 구현하기 위한 베이스 클래스 !!
// !! 64비트 환경 기준 명시적으로 16바이트 경계를 맞춤. !!
// template <class _Ty>
// class alignas(2 * sizeof(void*)) _Atomic_ptr_base // overalignment is to allow potential future use of cmpxchg16b
// {
// protected:
//     constexpr _Atomic_ptr_base() noexcept = default;
// 
//     _Atomic_ptr_base(remove_extent_t<_Ty>* const _Px, _Ref_count_base* const _Ref) noexcept
//         : _Ptr(_Px), _Repptr(_Ref) { }
//     ...
//     atomic<remove_extent_t<_Ty>*> _Ptr{nullptr};      // 8바이트
//     mutable _Locked_pointer<_Ref_count_base> _Repptr; // 8바이트(컨트롤 블록의 포인터를 동기화하기 위한 장치)
// };
// 
// !! atomic<shared_ptr<T>>는 _Atomic_ptr_base<T>를 상속하여 구현한 템플릿 특수화 클래스 !!
// !! 64비트 환경 기준 16바이트의 경계를 맞추는 특성 또한 계승됨. !!
// template <class _Ty>
// struct atomic<shared_ptr<_Ty>> : private _Atomic_ptr_base<_Ty>
// {
// private:
//     using _Base = _Atomic_ptr_base<_Ty>;
// 
// public:
//     ...
//     constexpr atomic() noexcept = default;
//     
//     constexpr atomic(nullptr_t) noexcept : atomic() { }
//     
//     // 일반적으로 사용될 유형
//     atomic(const shared_ptr<_Ty> _Value) noexcept : _Base(_Value._Ptr, _Value._Rep) {
//         // 컨트롤 블록을 통해 레퍼런스 카운팅을 진행하는 것을 잊으면 안 됨.
//         _Value._Incref();
//     }
//     
//     atomic(const atomic&)         = delete;
//     void operator=(const atomic&) = delete;
//     
//     // 일반적으로 사용될 유형
//     void operator=(shared_ptr<_Ty> _Value) noexcept {
//         store(_STD move(_Value));
//     }
//     
//     // 참조를 명시적으로 끊을 때 사용
//     void operator=(nullptr_t) noexcept {
//         store(nullptr);
//     }
// 
//     // (주의) 멀티스레딩 환경이라고 해도 소멸자 호출은 경합되지 않음.
//     ~atomic()
//     {
//         const auto _Rep = this->_Repptr._Unsafe_load_relaxed();
//         if (_Rep) {
//             _Rep->_Decref();
//         }
//     }
// };
// 
// atomic<shared_ptr<T>>는 shared_ptr<T>를 래핑하는 수준을 넘어 스레드 안전한 처리를 지원하기 위해 아예 새로 정의한 타입이다.
// !! atomic<shared_ptr<T>> 내부를 보면 shared_ptr<T>의 요소를 부모 클래스에 넘기고 있음(어디에서 shared_ptr<T> 변수를 저장하는 코드는 없음). !!
// 
// shared_ptr<T>와 호환되긴 하지만 _Ptr과 _Rep를 다소 특이한 형태의 자료형으로 래핑해서 사용하고 있다.
// 래핑된 형태의 자료형을 사용하는 만큼 세부적인 동작 방식도 상이하다.
// 
// ## _Ptr ##
// atomic<shared_ptr<T>>의 _Ptr은 원자성을 보장하지만, shared_ptr<T>의 _Ptr은 원자성을 보장하지 않는다.
// !! 단순 대입으로 _Ptr을 갱신한다고 해도 어셈블리 수준에서 이루어지는 연산을 보면 여러 단계를 걸치기에 원자성을 보장하지 않는 것임. !!
// - atomic<shared_ptr<T>> : atomic<remove_extent_t<_Ty>*> _Ptr{nullptr}; // 관리 객체의 포인터를 atomic으로 감싸서 원자적인 갱신을 보장함.
// - shared_ptr<T>         : element_type* _Ptr{nullptr}; // 관리 객체의 포인터를 갱신하는 과정에서 데이터 경합에 대한 원자성을 보장하지 않음.
//                              └ using element_type = remove_extent_t<_Ty>;
// ## _Rep ##
// atomic<shared_ptr<T>>의 _Repptr는 shared_ptr<T>의 컨트롤 블록 _Rep의 주소를 정수값으로 저장하기 위한 단순 저장소이다.
// _Repptr 자체는 레퍼런스 카운팅을 진행하지 않지만 shared_ptr<T>의 _Rep는 레퍼런스 카운팅을 한다는 것을 생각하면 처음 봤을 땐 개념적으로 혼동하기 쉬운 부분이다.
// !! _Repptr과 _Rep는 개념적으로 사용되는 방식이 다름. !!
// !! _Repptr는 주소 경계를 활용해서 사이사이의 빈 주소(?)를 마스킹하고 대기 조건이 들어간 조건부 스핀-락을 돌림. !!
// - atomic<shared_ptr<T>> : mutable _Locked_pointer<_Ref_count_base> _Repptr; // mutable로 선언된 대상은 const 함수 내에서도 값을 바꿀 수 있음.
// - shared_ptr<T>         : _Ref_count_base* _Rep{nullptr};
// 
// ## _Locked_pointer<T> ##
// !! _Locked_pointer<T>는 atomic<shared_ptr<T>>를 구현하기 위한 핵심 기능임. !!
// template <class _Ty>
// class _Locked_pointer // T는 _Ref_count_base(컨트롤 블록)임.
// {
// public:
//     // https://en.cppreference.com/w/cpp/language/alignof
//     // https://learn.microsoft.com/ko-kr/cpp/cpp/alignof-operator?view=msvc-170
//     // alignof()는 타입이 배치될 때 적용될 메모리의 정렬 위치 단위(크기??)를 반환하기 위한 함수
//     // 여기서는 포인터로 받으니 특별한 문제가 있는 게 아니면 아래 static_assert()가 실패하는 일은 없음.
//     static_assert(alignof(_Ty) >= (1 << 2), "2 low order bits are needed by _Locked_pointer");
//     static constexpr uintptr_t _Lock_mask                = 3;
//     static constexpr uintptr_t _Not_locked               = 0;
//     static constexpr uintptr_t _Locked_notify_not_needed = 1;
//     static constexpr uintptr_t _Locked_notify_needed     = 2;
//     static constexpr uintptr_t _Ptr_value_mask           = ~_Lock_mask; // 정수에서 포인터 값만 추출하기 위한 마스크
// 
//     constexpr _Locked_pointer() noexcept : _Storage{} { }
// 
//     // _Ptr은 관리 객체가 아닌 컨트롤 블록 _Ref_count_base이며 이것의 주소를 정수값으로 캐스팅하여 받음.
//     explicit _Locked_pointer(_Ty* const _Ptr) noexcept : _Storage{reinterpret_cast<uintptr_t>(_Ptr)} { }
// 
//     _Locked_pointer(const _Locked_pointer&)            = delete;
//     _Locked_pointer& operator=(const _Locked_pointer&) = delete;
// 
//     _NODISCARD _Ty* _Lock_and_load() noexcept
//     {
//         ...
//     }
// 
//     void _Store_and_unlock(_Ty* const _Value) noexcept
//     {
//         ...
//     }
// 
//     // _Ty는 관리 객체가 아니라 컨트롤 블록임.
//     // _Locked_pointer<T>의 다른 함수들도 reinterpret_cast<T>()를 통해 _Storage 값을 _Ref_count_base의 주소로 변환해서 사용함.
//     _NODISCARD _Ty* _Unsafe_load_relaxed() const noexcept
//     {
//         return reinterpret_cast<_Ty*>(_Storage.load(memory_order_relaxed));
//     }
// 
// private:
//     // typedef unsigned __int64  uintptr_t;
//     // _Storage는 포인터가 아닌 그냥 정수값임.
//     atomic<uintptr_t> _Storage; // shared_ptr<T>의 _Rep를 대신하는 변수로 주소를 값으로 사용함(마스킹 활용).
// };
// 
// _Locked_pointer<_Ref_count_base> 자체는 _Ref_count_base의 주소에 접근하지 않는다(레퍼런스 카운팅을 하지 않는다는 뜻).
// _Ref_count_base의 주소에 접근해서 레퍼런스 카운팅을 수행하는 건 atomic<shared_ptr<_Ty>> 쪽이다.
// 
// _Locked_pointer<_Ref_count_base>는 _Rep 주소를 정수값으로 보관하며 나중에 이를 복원할 수 있는 기능을 제공한다.
// 또한 내부에서 스핀-락과 대기 조건을 사용하여 컨트롤 블록에 대한 접근이 스레드 안전하게 동작할 수 있게 해준다.
// 
// ## _Lock_and_load() ##
// !! 해당 함수는 경합을 통해 Lock의 소유권을 획득하기 위한 함수이며 동시에 _Storage에 정수로 저장된 컨트롤 블록의 주소를 캐스팅하여 반환함. !!
// _NODISCARD _Ty* _Lock_and_load() noexcept
// {
//     // 값을 교체하기 전에 일단 읽음(상태 변화 감지 용도).
//     uintptr_t _Rep = _Storage.load(memory_order_relaxed);
// 
//     // https://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange
//     // atomic<T>::compare_exchange_weak(&expected, desired)의 기능은 한 번 보고 분석하는 것이 좋음.
//     // (주의) compare_exchange_weak()은 교환에 실패하면 expected의 값을 대상 변수의 값으로 갱신함.
// 
//     // _Storage와 _Rep가 같을 때 갱신하고 아니면 _Rep를 최신 _Storage의 값으로 갱신한 다음 다시 비교하는 과정을 거침.
//     // _Storage 값은 항상 원자적으로 갱신됨.
//     // (중요) 비교하는 과정은 원자적으로 일어나며 또 경합되는 부분임.
//     // (중요) _Storage와 _Rep가 다르면 _Rep를 _Storage의 값으로 갱신한 다음 다시 경합해서 비교하는 과정을 거치는 것이 핵심임.
// 
//     // 스핀-락과 대기 조건이 섞인 하이브리드 형태
//     for (;;)
//     {
//         // 마스킹을 통한 _Storage의 상태 확인
//         // !! 단일 스마트 포인터를 대상으로 한 멀티스레딩 환경에서의 경합이 발생했을 경우를 상정하고 보도록 할 것! !!
//         switch (_Rep & _Lock_mask) // ★ 소유권을 획득하지 못 하면 _Rep는 _Storage의 값으로 갱신됨(위에 있는 주의 참고).
//         {
//         // 누구도 Lock의 소유권을 획득하지 않은 상태에서 경합이 발생한 케이스
//         case _Not_locked: // Can try to lock now
//             if (_Storage.compare_exchange_weak(_Rep, _Rep | _Locked_notify_not_needed))
//             {
//                 // _Storage와 _Rep의 값을 교환했다면 소유권 획득 경쟁에서 이긴 거임.
//                 // 그럼 _Storage의 값은 (_Rep | _Locked_notify_not_needed)로 교체됨.
//                 // !! Lock 소유권 경쟁에서 이겼으면 _Rep의 값은 변하지 않음 -> 즉, 컨트롤 블록 그 자체의 주소가 정수값으로 보존된 상태임
//                 
//                 // _Rep가 _Not_locked 상태일 때 _Storage와 값이 같았기 때문에 _Rep는 컨트롤 블록의 주소가 변형되지 않은 상태의 정수값임.
//                 return reinterpret_cast<_Ty*>(_Rep);
//             }
// 
//             // https://modoocode.com/en/inst/pause
//             // https://learn.microsoft.com/ko-kr/windows/win32/api/winnt/nf-winnt-yieldprocessor
//             // !! _YIELD_PROCESSOR()와 C++의 스레드에서 사용하는 yield()는 다름. !!
//             // # _YIELD_PROCESSOR()
//             //  - 하드웨어 수준의 CPU 명령어
//             //  - 매우 짧게 대기하며 컨텍스트 스위칭이 발생하지 않음.
//             //  - 하이퍼 스레딩과 같은 기술이 적용됨.
//             //  - 짧은 시간 자원을 과도하게 사용하는 스핀-락 같은 곳에서 사용하면 좋음.
//             // # 스레드의 yield()
//             //  - 운영체제 수준의 스케줄링 관련 함수
//             //  - 비교적 긴 대기 시간을 가지며 컨텍스트 스위칭이 발생할 수 있음.
//             //  - 스레드의 실행 순서를 다른 스레드에게 양보함.
//             //  - 다른 스레드와 협력해야 하는 경우 사용하면 좋음.
//             _YIELD_PROCESSOR(); // <----- pause 명령어로 컴파일됨(컨텍스트 스위칭을 유발하는 명렁어가 아니니까 주의).
// 
//            break;
// 
//         // 누군가가 Lock의 소유권을 획득한 상황에서 비교하는 케이스
//         case _Locked_notify_not_needed: // Try to set "notify needed" and wait
//             if (!_Storage.compare_exchange_weak(_Rep, (_Rep & _Ptr_value_mask) | _Locked_notify_needed))
//             {
//                 // 1. _Storage와 _Rep의 값이 같다면?
//                 //   - 해당 if 문에 들어오지 않음(Lock의 소유권이 아직 반환되지 않은 상태).
//                 //   - _Rep에 마스킹을 취해 구한 순수한 컨트롤 블록의 포인터에 _Locked_notify_needed를 묶은 값은 _Storage 값으로 갱신함.
//                 //   - _Rep의 값에 _Locked_notify_needed 플래그를 엮서 _Rep의 값을 갱신하고 case _Locked_notify_needed 코드를 수행함.
//                 // 2. _Storage와 _Rep의 값이 다르다면?
//                 //   - 해당 if 문에 들어옴(Lock의 소유권이 반환되었거나 _Locked_notify_needed 플래그로 갱신된 상태).
//                 //   - break로 switch를 빠져나가고 처음부터 다시 비교함.
//                 //   - (중요) 교환에 실패하면 _Rep의 값은 최신 _Storage의 값으로 갱신됨.
//                 //
//                 // !! 이 로직이 이해가 안 간다면 천천히 손으로 그리면서 이해할 것. !!
//                 
//                 // Failed to set notify needed flag, try again
//                 _YIELD_PROCESSOR(); // <----- pause 명령어
// 
//                 break;
//             }
// 
//             // _Storage와 _Rep의 값이 같아서 교환에 성공했으면 Lock의 소유권이 반환되지 않은 상태임.
//             // 그럼 대기 조건을 걸어서 값이 갱신되었는지 확인하고 대기해야 한다면 notify 계열의 함수가 호출될 때까지 대기해야 함.
//             _Rep = (_Rep & _Ptr_value_mask) | _Locked_notify_needed;
// 
//             [[fallthrough]]; // 명시적으로 switch에서 break를 쓰지 않았음을 알리는 attribute
// 
//         // 플래그를 확인했을 당시 Lock의 소유권이 반환되지 않았을 때 들어오는 케이스
//         // 해당 케이스에 들어왔을 때 Lock의 소유권이 반환되었거나 _Storage의 플래그가 _Locked_notify_not_needed로 바뀌었을 가능성이 있음.
//         case _Locked_notify_needed: // "Notify needed" is already set, just wait
//             // https://en.cppreference.com/w/cpp/atomic/atomic/wait C++20(condition_variable 아님)
//             // # Compare the value representation of this->load(order) with that of old
//             // - If those are equal, then blocks until *this is notified by notify_one() or notify_all(), or the thread is unblocked spuriously.
//             // - Otherwise, returns.
//             //
//             // wait() 호출 시 _Storage 값이 _Rep 값과 같다면 대기 상태에 들어가고 그게 아니라면 즉시 함수를 빠져나옴.
//             // 대기 상태에 들어갔으면 notify_one()이나 notify_all()을 호출하기 전까지 깨어나지 않음.
//             // 갱신되어야 하는 대상은 당연히 _Storage이며, notify 계열의 함수를 호출했다고 해도 _Storage 값이 _Rep와 같다면 깨어나지 않음.
//             // wait()가 atomic<T>의 함수인 만큼 해당 함수는 원자적으로 수행됨.
//             //
//             // _Storage의 값은 갱신되었을 수 있지만 적어도 _Rep는 _Locked_notify_needed 플래그가 묶인 형태임.
//             _Storage.wait(_Rep, memory_order_relaxed);
// 
//             // notify_all()이 호출되었고 _Storage 값과 _Rep의 값이 다르다면 대기 상태에서 깨어남.
//             // 다시 비교 및 경합하는 과정을 거쳐야 하기 때문에 _Rep의 값을 최신 _Storage의 값으로 갱신함.
//             _Rep = _Storage.load(memory_order_relaxed);
// 
//             break;
// 
//         // 정의되지 않은 비트 플래그 패턴일 경우 프로그램 중단
//         default: // Unrecognized bit pattern
//             _CSTD abort();
//         }
//     }
// }
// 
// ## _Store_and_unlock(_Ty* const _Value) ##
// !! 해당 함수는 Lock의 소유권을 반환하는 함수이며 동시에 _Storage의 값을 복원함. !!
// void _Store_and_unlock(_Ty* const _Value) noexcept
// {
//     // _Value는 _Lock_and_load()로 반환한 순수한 컨트롤 블록의 주소값임.
//     // (중요) _Store_and_unlock() 내 _Storage.exchange()는 경합 대상이 아님.
//     const auto _Rep = _Storage.exchange(reinterpret_cast<uintptr_t>(_Value));
// 
//     // Lock의 소유권이 반환된 이후 코드는 경합될 수 있음.
//     // exchange()가 적용되면 경합 중인 스레드 중 하나는 반드시 case _Not_locked를 거침.
// 
//     // _Rep는 _Storage가 가지고 있던 값을 가지고 옴.
//     // 만약 교환 이전 _Storage에 _Locked_notify_needed 플래그가 적용된 상태였다면 wait()에 의해 대기하고 있던 스레드를 깨움.
//     // (혼동) _Value을 적용한 값이 아닌 교환 이전 _Storage 값을 기준으로 비교하는 것임.
//     if ((_Rep & _Lock_mask) == _Locked_notify_needed)
//     {
//         // As we don't count waiters, every waiter is notified, and then some may re-request notification
//         _Storage.notify_all();
//     }
// 
//     // 항상 exchange() 이후 nofity_all()을 호출하기 때문에 _Storage.wait()에서 무한 대기하는 경우는 없음(반드시 숙지할 것).
//     // _Storage 값 자체는 항상 원자적으로 갱신되며 _Storage에 _Locked_notify_needed 플래그가 없다면 애초에 wait()에 의한 대기 상태에 들어가지 않음.
// }
// 
// atomic<shared_ptr<T>>를 보면 직접적인 접근을 위한 연산자 오버로딩 operator*()나 operator->()가 없다.
// shared_ptr<T>를 받는 형태이긴 하나 참조 카운트를 원자적으로 안전하게 관리하기 위해 관리 객체에 직접 접근할 수 있는 기능은 지원하지 않는다.
// 
// atomic<shared_ptr<T>>로 관리 객체에 접근하려면 weak_ptr<T>의 lock()과 유사하게 load()를 통해 shared_ptr<T>를 가져와야 한다.
// atomic<shared_ptr<T>> 자체는 관리 객체의 멤버에 접근하거나 디레퍼런싱하는 기능을 제공하지 않는다.
// 
// !! load()의 과정을 보면 shared_ptr<T>를 반환하기 전에 레퍼런스 카운팅을 하나 증가시키는 전략을 취하고 있음. !!
// _NODISCARD shared_ptr<_Ty> atomic<shared_ptr<_Ty>>::load(const memory_order _Order = memory_order_seq_cst) const noexcept
// {
//     _Check_load_memory_order(_Order); // _Order의 값이 memory_order를 벗어났는지 확인하기 위한 함수
// 
//     shared_ptr<_Ty> _Result;
//     const auto _Rep = this->_Repptr._Lock_and_load(); // <----- Lock 소유권 획득 + 컨트롤 블록 획득
// 
//     _Result._Ptr    = this->_Ptr.load(memory_order_relaxed); // 관리 객체를 원자적으로 가져옴(_Ptr은 _Atomic_ptr_base<T>에 있음).
//                                                              // 원자적으로 가져오는 것이 아니라면 캐시 일관성이 깨질 수 있음.
//     _Result._Rep    = _Rep;
//     _Result._Incref(); // <----- _Rep를 거쳐 강한 참조(_Uses)를 하나 증가
// 
//     this->_Repptr._Store_and_unlock(_Rep); // <----- Lock 소유권 반환 + _Storage 복원
// 
//     return _Result; // <----- 새로 구성한 shared_ptr<T> 반환
// }
// 
// !! (주의) 멀티스레딩 환경이라고 해도 소멸자 호출은 경합되지 않으니 혼동해선 안 됨(단 한 번만 호출됨). !!
// atomic<shared_ptr<_Ty>>::~atomic()
// {
//     const auto _Rep = this->_Repptr._Unsafe_load_relaxed();
//     if (_Rep) {
//         _Rep->_Decref();
//     }
// }
// 
// !! 소멸 과정에 진입했으면 더 이상 _Storage가 경합될 일은 없음. !!
// !! _Storage에는 어떤 플래그 값도 설정되어 있지 않기에 순수한 컨트롤 블록의 주소를 얻을 수 있음. !!
// _NODISCARD _Ty* _Locked_pointer<T>::_Unsafe_load_relaxed() const noexcept
// {
//     return reinterpret_cast<_Ty*>(_Storage.load(memory_order_relaxed));
// }
// 
// !! atomic<shared_ptr<T>>에서 shared_ptr<T>를 다른 것으로 교체(갱신)하고 싶다면 store()를 호출해야 함. !!
// void atomic<shared_ptr<_Ty>>::operator=(shared_ptr<_Ty> _Value) noexcept
// {
//     store(_STD move(_Value));
// }
// 
// void atomic<shared_ptr<_Ty>>::operator=(nullptr_t) noexcept
// {
//     store(nullptr);
// }
// 
// void atomic<shared_ptr<_Ty>>::store(shared_ptr<_Ty> _Value, const memory_order _Order = memory_order_seq_cst) noexcept
// {
//     _Check_store_memory_order(_Order); // _Order의 값이 memory_order를 벗어났는지 확인하기 위한 함수
// 
//     // 기존 컨트롤 블록을 가져옴.
//     const auto _Rep = this->_Repptr._Lock_and_load();
// 
//     // 교체 대상인 _Value로부터 관리 객체를 가져옴.
//     remove_extent_t<_Ty>* const _Tmp = _Value._Ptr;
// 
//     // 기존 관리 객체를 교체 대상인 _Value의 관리 객체로 지정함.
//     _Value._Ptr = this->_Ptr.load(memory_order_relaxed);
// 
//     // 기존 관리 객체를 갱신함.
//     this->_Ptr.store(_Tmp, memory_order_relaxed);
// 
//     // 기존 컨트롤 블록을 갱신함.
//     this->_Repptr._Store_and_unlock(_Value._Rep);
// 
//     // 이전 컨트롤 블록을 교체 대한인 _Value의 컨트롤 블록으로 지정함.
//     _Value._Rep = _Rep;
// 
//     // !! 위 과정을 거치면 this와 _Value의 관리 객체의 포인터와 컨트롤 블록의 포인터가 교환됨. !!
//     // (중요) 이전 관리 객체와 컨트롤 블록은 _Value의 소멸자를 거치며 소멸 과정을 거칠 수 있다.
// }
// 
// (참고) atomic<shared_ptr<T>>을 쓰면 shared_ptr<T>을 load()로 가져오기 때문에 다음 상황은 발생하지 않는다.
// code)---------------------------------------------
// sptr->MustBeFirst();
// // <----- 이 사이에 sptr가 갱신될 일은 없음.
// sptr->MustBeSecond();
// --------------------------------------------------
// 

BEGIN_NS(Case01)

shared_ptr<int64_t> dataRacingSPtr;

// 문제가 있는데 정상 작동하는 것처럼 보이는 코드(매우 매우 문제가 많은 코드)
// 이런 코드는 차라리 Crash가 발생해서 프로그램이 죽어야 한다.
void Run()
{
    dataRacingSPtr = make_shared<int64_t>(100);

    thread th{ [] {
        // 스마트 포인터를 갱신하고 있는 도중 접근하는 상황이 발생하면?
        while (true)
        {
            // template <class _Ty2 = _Ty, enable_if_t<!disjunction_v<is_array<_Ty2>, is_void<_Ty2>>, int> = 0>
            // _NODISCARD _Ty2& shared_ptr<_Ty>::operator*() const noexcept
            // {
            //     // 레퍼런스 카운팅을 진행하지 않고 그냥 접근함.
            //     return *get();
            // }
            // 
            // _NODISCARD element_type* _Ptr_base<_Ty>::get() const noexcept
            // {
            //     return _Ptr;
            // }
            // 
            // shared_ptr<T>의 관리 객체에 접근하는 행위 자체는 레퍼런스 카운팅을 진행하지 않으며,
            // 그냥 관리 객체의 포인터를 반환하는 단순한 방식으로 처리한다.
            // 
            if (100 != *dataRacingSPtr)
            // mov         rax,qword ptr [Case01::dataRacingSPtr] <----- dataRacingSPtr에 대응되는 메모리 주소가 가진 메모리 값(포인터) 복사
            // <----- 여기서 dataRacingSPtr에 대응되는 메모리 주소가 가진 값이 변경(교체)되면? 동기화 이슈 및 메모리 일관성이 깨지는 문제 발생
            // cmp         qword ptr [rax],64h <----- dataRacingSPtr에 대응되는 메모리 주소가 가진 값이 바뀌면 rax와의 일관성이 깨짐.
            //                                        일관성이 깨졌으면 비교를 통과하고 "Error"를 출력해야 하는데 이 케이스에서는 "Error"를 출력하지 않음.
            //                                        "Error"가 출력되려면 rax에 담긴 포인터가 가리키는 메모리가 재사용된 상태에서 해당 메모리가 가진 값이 변경되어야 함.
            // je          `Case01::Run'::`2'::<lambda_1>::operator()+10h
            {
                cout << "Error!\n";
            }

            // 단순하게 관리 객체에 접근하는 방식이기 때문에 컨트롤 블록 관련 코드는 생성되지 않는다.
            // dataRacingSPtr에 대응되는 메모리의 위치 자체가 관리 객체의 포인터이며,
            // dataRacingSPtr + 8의 위치가 컨트롤 블록의 포인터이다.
            // !! 컴파일러는 코드를 컴파일할 때 멀티스레딩 환경을 고려하지 않음(싱글스레딩 환경 기반으로 컴파일하는 것이 일반적임). !!
            // !! 무엇보다 shared_ptr<T>::operator*()는 관리 객체에 단순 접근하는 방식으로 동작함. !!
        }
    } };

    // ## 어셈블리 명령어 앞에 붙는 LOCK 접두사 ##
    // https://hackmd.io/@vesuppi/Syvoiw1f8
    // https://modoocode.com/en/inst/lock
    // https://stackoverflow.com/questions/8891067/what-does-the-lock-instruction-mean-in-x86-assembly
    //
    // 명령어 앞에 LOCK 접두사가 붙으면 해당 명령어가 원자적으로 처리될 수 있게 힌트를 제공한다.
    // 이를 통해 데이터 레이스로 인해 발생하는 문제를 막아 스레드의 캐시 동기화 및 일관성을 유지할 수 있게 해준다.
    // 정확히 말하자면 LOCK 접두사가 붙은 명령어는 처리 대상이 되는 특정 프로세서의 캐시 메모리(캐시 라인 단위)를 잠가서 배타적인(exclusive) 처리와 원자적인 접근을 보장한다.
    // 캐시 일관성 프로토콜이 적용되기 때문에 연산이 이루어지는 코어의 레지스터만 갱신하는 것이 아닌 다른 코어의 캐시나 주기억장치에도 결과가 반영된다.
    //
    // 과거의 CPU는 LOCK은 Bus Lock을 통해서 동작했지만 현대 CPU는 성능상의 이슈로 프로세서의 캐시 라인을 락으로 보호하는 정책을 사용한다.
    while (true)
    {
        auto tempSPtr = make_shared<int64_t>(100);
        // mov         ecx,18h  
        // call        operator new // 이 과정을 거치면 rax에 동적할당한 메모리의 주소가 담김.
        // mov         rbx,rax      // rbx에도 동적할당한 메모리의 주소를 담음.
        // mov         qword ptr [rsp+30h],rax  
        // xorps       xmm0,xmm0  
        // movups      xmmword ptr [rax],xmm0  
        // mov         dword ptr [rax+8],1      // strong ref : 1
        // mov         dword ptr [rax+0Ch],1    // weak ref : 1
        // mov         qword ptr [rax],rsi      // std::_Ref_count_obj2<__int64>에 대한 vtable 설정(디스어셈블리와 메모리 디버깅을 통한 추정)
        // add         rax,10h                  // rax의 위치를 16바이트 만큼 이동
        // mov         qword ptr [rax],64h      // value : 100
        // 
        // |                     Base Control Block                      ||    int64_t     |
        // [ vtable(8 bytes) ][ strong ref(4 bytes) ][ weak ref(4 bytes) ][ value(8 bytes) ]
        //     └ 엄밀히 말하자면 컴파일되어 들어간 함수의 "주소" 목록이 저장된 테이블에서 std::_Ref_count_obj2<__int64>::_Destroy(void)를 가리키는 위치가 저장됨.
        // 
        // !! 이 부분은 직접 디스어셈블리 창과 메모리 창을 통해서 추적하여 확인한 내용임(vtable이 컴파일되어 동작하는 방식(?)이라고 봐야 함). !!
        // 0x00007FF79FAB3380  80 12 ab 9f f7 7f 00 00  이게 std::_Ref_count_obj2<__int64>::_Destroy(void)를 가리키는 위치라면
        // 0x00007FF79FAB3388  60 12 ab 9f f7 7f 00 00  해당 위치에서 +8을 더한 주소에 저장된 값은 std::_Ref_count_obj2<__int64>::_Delete_this()의 위치임.
        // 
        // !! call 명령어는 서브루틴(주로 함수) 호출을 위한 명령어임. !!
        // !! call 명령어가 실행되면 현재 위치를 스택에 저장(push)한 다음 서브루틴을 호출하고 작업이 끝나면 스택에 저장된 값을 토대로 원래 위치로 복원함. !!
        // !! 서브루틴의 끝은 ret 명령어로 지정되며 해당 명령어를 만나면 스택에서 복귀할 위치를 가지고(pop) 옴. !!
        // 
        // ## std::_Ref_count_obj2<__int64>::_Destroy(void) 위치에서 실행되는 코드를 예시로 들면? ##
        // ## 이 코드 자체가 std::_Ref_count_obj2<__int64>::_Destroy(void)에 해당하는 서브루틴임. ##
        // --- C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.38.33130\include\memory 
        //         _Destroy_in_place(_Storage._Value);
        //     }
        // ret         0 <----- rsi에 담긴 메모리 위치(0 말고 "ret 0"을 말하는 것)
        //                      관리 객체가 기본 자료형 int64_t이기 때문에 소멸자를 호출하지 않고 그냥 나옴.
        // 
        // 최적화가 적용된 상태라면 이런 느낌으로 메모리가 할당된다고 봐야 한다.
        // 이 메모리 구성은 컨트롤 블록 _Ref_count_obj2<T>의 메모리 레이아웃과 동일하다.
        // 
        // 최적화가 적용되지 않는 Debug 모드라면 rax에 직관적으로 tempSPtr가 담기는 것이 보이기 때문에 분석하기 쉽다.
        // Release 모드에선 최적화가 적용되는 만큼 직관적인 분석이 힘들지만 레지스터에 담긴 값을 유추하면
        // rax에 동적할당된 메모리의 주소가 담긴다는 것을 알 수 있고 또 해당 위치에 값이 어떻게 채워지는지도 확인할 수 있다.
        
        // (중요) 로컬에서 생성한 shared_ptr(tempSPtr)을 전역 shared_ptr(dataRacingSPtr)에 넣는 과정에서 문제가 생긴다.
        dataRacingSPtr = tempSPtr;
        // lock inc    dword ptr [rbx+8]        // strong refs : 2
        // mov         qword ptr [Case01::dataRacingSPtr],rax    // dataRacingSPtr의 _Ptr 교체(!! 문제 발생 지점 !!)
        // mov         rdi,qword ptr [Case01::dataRacingSPtr+8h] // dataRacingSPtr의 _Rep 포인터 값을 가져와 rdi에 저장
        // mov         qword ptr [Case01::dataRacingSPtr+8h],rbx // dataRacingSPtr의 _Rep 교체
        // 
        // dataRacingSPtr은 tempSPtr로 교체한 상황이고 rdi에는 이전 dataRacingSPtr의 컨트롤 블록 주소가 담겨 있다.
        // 아래 이어지는 코드는 이전 dataRacingSPtr의 컨트롤 블록을 대상으로 하는 레퍼런스 카운팅 및 소멸 과정에 대한 내용이다.
        // test        rdi,rdi              // rdi가 nullptr(0)인지 검사(컨트롤 블록의 포인터가 nullptr인지 체크) 
        // je          Case01::Run+161h  
        // mov         eax,0FFFFFFFFh           // -1
        // lock xadd   dword ptr [rdi+8],eax    // 이전 dataRacingSPtr의 _Rep의 멤버 _Uses(strong refs)에 접근한 다음 -1을 더하고, 더하기 이전 값을 eax에 저장
        // cmp         eax,1  
        // jne         Case01::Run+161h 
        // mov         rax,qword ptr [rdi]  // rdi가 가리키는 메모리 주소가 가진 값을 rax에 복사
        // mov         rcx,rdi              // rdi에 저장된 값을 그대로 rcx에 복사
        // call        qword ptr [rax]      // rax가 가리키는 메모리 주소 값을 위치로 하여 서브루틴 호출(std::_Ref_count_obj2<__int64>::_Destroy(void))
        // mov         eax,0FFFFFFFFh           // -1
        // lock xadd   dword ptr [rdi+0Ch],eax  // 이전 dataRacingSPtr의 _Rep의 멤버 _Weaks(weak refs)에 접근한 다음 -1을 더하고, 더하기 이전 값을 eax에 저장
        // cmp         eax,1  
        // jne         Case01::Run+161h  
        // mov         rax,qword ptr [rdi]  // rdi가 가리키는 메모리 주소가 가진 값을 rax에 복사(eax는 rax의 하위 32비트 값이라 eax에 0FFFFFFFFh 값을 넣으면 rax도 바뀜)
        // mov         rcx,rdi              // rdi에 저장된 값을 그대로 rcx에 복사
        // call        qword ptr [rax+8]    // rax + 8이 가리키는 메모리 주소 값을 위치로 하여 서브루틴 호출(std::_Ref_count_obj2<__int64>::_Delete_this())

        // 마지막 3개의 과정을 보면 rax를 다시 설정하는 코드가 있다.
        // mov         rax,qword ptr [rdi]
        // mov         rcx,rdi            
        // call        qword ptr [rax+8]
        // 
        // "lock xadd a, b"는 Exchange And Add를 수행하는 명령어인데
        // 단순히 a에 b의 값을 더하는 것 뿐만 아니라 더하기 이전 값을 b에 저장한다.
        // 
        // eax는 rax의 하위 32비트 값을 나타내는 레지스터라서 eax 값이 갱신되면 rax 값도 갱신된다.
        // 예를 들면 이렇다.
        // mov rax, 0x123456789ABCDEF0 // rax = 0x123456789ABCDEF0
        // mov eax, 0x87654321         // eax = 0x87654321, rax = 0x0000000087654321
        // 
        // 그런데 eax를 잘 보면 중간에 한 번 다시 -1로 값을 세팅하기도 하고 "lock xadd"를 통해 이전 값을 저장하기도 한다.
        // eax는 갱신되면 rax에도 영향을 미친다.
        // 
        // 이런 이유로 rax의 값을 다시 세팅하는 것이다.
    }

    th.join();
}

END_NS

BEGIN_NS(Case02)

shared_ptr<int64_t> dataRacingSPtr;

// Case01에서 스마트 포인터 갱신 여부를 인지하지 못하는 문제를 탐지하기 위해 약간 개선한 코드
// 배리어를 치는 작업이 필요하기 때문에 volatile_atomic_cache_coherence_and_memory_order.cpp에 나온 설명을 보면서 해당 코드를 읽도록 한다.
void Run()
{
    dataRacingSPtr = make_shared<int64_t>(100);
    
    thread th{ [] {
        while (true)
        {
            auto tempPtr = dataRacingSPtr.get();

            // if 안에 있는 "tempPtr != dataRacingSPtr.get()"에 대한 최적화를 방지하기 위한 코드(주석을 걸었다 제거해보면서 실행해볼 것)
            // 최적화가 적용되지 않는 Debug 모드로 돌리면 배리어를 치지 않아도 상관없다.
            // 하지만 최적화가 적용되는 Release 모드로 컴파일하면 배리어를 쳐야 코드가 제대로 돌아갈 확률이 높다.
            // !! 코드가 생성되는 방식은 컴파일러에 따라 다르고 버전에 따라 바뀔 수 있기 때문에 "확률이 높다"로 표현함. !!
            atomic_thread_fence(memory_order_seq_cst); // (중요) 주석을 칠 때와 안 칠 때 어떻게 돌아가는지 확인할 것

            // 배리어를 치지 않으면 "tempPtr != dataRacingSPtr.get()"는 최적화되어 어셈블리로 변환되지 않음(의도하지 않은 코드가 됨)
            // 배리어를 쳤을 때와 치지 않았을 때 어셈블리 코드가 어떻게 들어가는지 확인해서 차이를 이해할 것.

            // 배리어를 치지 않으면 "tempPtr != dataRacingSPtr.get()"는 최적화되어 어셈블리로 변환되지 않는다(의도하지 않은 코드가 됨).
            // 배리어를 쳤을 때와 치지 않았을 때 어셈블리 코드가 어떻게 들어가는지 직접 확인해서 차이를 이해해야 한다.
            // !! 컴파일러는 멀티스레딩 환경을 고려해서 컴파일하지 않고 싱글스레딩 환경 기준으로 컴파일하는 것이 보통임. !!
            // !! (매우 중요) 멀티스레딩 안정성을 제공하는 건 프로그래머의 책임임. !!
            if (100 != *dataRacingSPtr || tempPtr != dataRacingSPtr.get()) // 관리 객체의 포인터의 갱신 여부를 확인하는 로직 추가
            // mov         rax,qword ptr [dataRacingSPtr]  
            // cmp         qword ptr [rax],64h  
            // jne         `Case02::Run'::`2'::<lambda_1>::operator()+3Ah  
            // cmp         rcx,rax  <----- 배리어를 치지 않은 상태라면 해당 코드가 최적화되어 사라짐!
            // je          `Case02::Run'::`2'::<lambda_1>::operator()+20h  
            {
                // atomic_thread_fence(memory_order_seq_cst)로 배리어를 치면 "Error!"가 출력되어야 한다.
                cout << "Error!\n";
            }
        }
    } };

    while (true)
    {
        auto tempSPtr = make_shared<int64_t>(100);

        dataRacingSPtr = tempSPtr;
    }

    th.join();
}

END_NS

BEGIN_NS(Case03)

shared_ptr<int64_t> dataRacingSPtr;

// 잠재적인 문제를 내포하고 있는 코드 + 운영체제의 메모리 관리 방식에 따라서 Crash가 발생하는 코드
// 전역 shared_ptr을 로컬 영역에 복사한다고 해도 그 과정과 shared_ptr을 교체하는 과정이 경합되면 이것 또한 안전하지 않다(데이터 레이스 유형).
// (중요) 컴파일러는 멀티스레딩 환경을 고려하지 않고 코드를 생성하는 것이 기본이기 때문에 이로 인해 발생할 수 있는 문제는 숙지해야 한다.
void Run()
{
    // TEMP : while을 몇 번 순회해야, shared_ptr을 몇 번 생성해야 문제가 발생하는지 확인하기 위한 변수
    //      : 테스트를 해봤을 때 두 카운터 모두 적게 나왔을 때는 10만, 많게 나왔을 때는 100만이 나왔다.
    //      : 할당과 해제 빈도를 고려하면 페이지는 충분히 Committed 상태에서 반환된 다음 다시 재사용되어 Commited 상태로 변화하는 과정을 거치기 충분하다.
    atomic<uint64_t> loopCounter;
    atomic<uint64_t> creationCounter;

    // ## 분석할 때 주의할 점 ##
    // 멀티스레딩 환경에서 레지스터 단위, 어셈블리 단위로 코드를 분석하면 Memory 창에서 보여지는 값과 레지스터를 통해 가져오는 값이 일치하지 않을 수 있다.
    // 아래 내용은 추정이긴 하나 며칠을 분석해서 내린 나름대로의 결론이다.
    // - VirtualAlloc()으로 페이지를 새로 할당하면 메모리 값이 0으로 초기화되지만 그게 즉각적으로 Memory 창에 반영되는 건 아님.
    //   - 근거 : 명령어 "call operator new"가 실행되고 있는 와중에 프로그램이 중단되면 실제 레지스터에서 가져오는 값은 0인데 Memory에는 0이 아닌 다른 값이 있었음.
    // - 스레드를 실행하는 코어가 어떤 캐시에 접근해서 값을 수정하면 해당 캐시에 대응되는 메모리를 다른 코어에서 캐시로 참조하고 있다면 캐시 일관성 프로토콜에 의해 갱신될 수 있음.
    // - 캐시 일관성 프로토콜에 의해 캐시의 값이 갱신된다고 해도 주기억장치에 있는 내용은 이를 반영하지 않을 수 있음.
    //   - 근거 : "mov rax, qword ptr [rbx]"와 같은 명령어를 수행했는데 rax에 들어간 값과 rbx가 가리키는 값이 다른 경우가 자주 발생했음.
    // - 해당 명령어 예시에서 "qword ptr [rbx]"로 가져오는 값은 주기억장치를 기반으로 하는 것이 아닌 CPU 캐시를 기반으로 하며 rax에는 그런 캐시에 있는 값이 담김(조회되는 Memory 창에서 보여지는 값과 레지스터에 담기는 값이 다른 이유).
    
    dataRacingSPtr = make_shared<int64_t>(100);

    thread th{ [&] {
        while (true)
        {
            // TEMP
            loopCounter++;
            (void)creationCounter.load(); // Release 모드에서 중단점을 걸었을 때 조사식으로 creationCounter를 조회할 수 있게 하기 위한 임시 코드

            shared_ptr<int64_t> tempSPtr;

            // 전역 shared_ptr을 로컬 영역의 shared_ptr로 복사하는 과정은 잠재적인 문제를 내포하고 있다.
            tempSPtr = dataRacingSPtr;
            // A-01. mov         rax,qword ptr [Case03::dataRacingSPtr+8h] // 컨트롤 블록의 포인터를 rax에 단순 복사(★ 추정했을 때 Crash를 유발하는 명령어 중 하나)
            // A-02. test        rax,rax  // test는 두 피연산자에 AND 논리 연산을 적용하여 0이 나오면 ZF(ZeroFlag)의 값을 1로, 아니면 0으로 설정
            //                            // 따라서 rax가 nullptr(0)인지 검사하는 코드(컨트롤 블록이 nullptr인지 체크)
            // A-03. je          `Case03::Run'::`2'::<lambda_1>::operator()+38h // rax가 nullptr(0)이면 "lock inc" 명령어를 수행하지 않고 점프(mov rbx,qword ptr [Case03::dataRacingSPtr+8h]로 점프)
            //                                                          // rax에 유효한 주소가 저장되었으면 점프하지 않고 순차적으로 "lock inc dword ptr [rax+8]"을 실행함.
            // <----- A-01을 거쳐 rax에 dataRacingSPtr의 컨트롤 블록의 포인터를 저장한 상태임.
            // <----- 그런데 A-04를 수행하기 이전에 다른 스레드에서 dataRacingSPtr의 갱신 과정이 이루어졌고 이전 dataRacingSPtr의 컨트롤 블록까지 해제한 상태면?
            // A-04. lock inc    dword ptr [rax+8] // 컨트롤 블록의 레퍼런스 카운터 _Uses에 접근해서 값을 1 증가
            // <----- lock 접두사가 붙어서 A-04가 수행 단계에 있을 때 대상 메모리의 값을 원자적으로 증가시킨다고 해도 문제는 사라지지 않음.
            // <----- 프로세서 코어의 캐시 라인을 잠근다고 해도 dataRacingSPtr가 갱신된 상태라면 이 단계에서는 해당 상황을 인지할 방법이 없음.
            // <----- 이런 문제가 발생하면 차라리 Crash가 떠야 하는데 Crash는 메모리 해제 작업을 거쳤어도 페이지가 Committed 상태라면 발생하지 않음.
            // <----- 해제된 메모리가 속한 페이지가 Reserved 혹은 Free 상태일 때 접근해야 Crash가 발생하는 구조라는 것을 명심해야 함.
            // <----- 낮은 확률이긴 하나 메모리가 재사용되었고 페이지 또한 Committed 상태라면 누군가 사용하고 있는 메모리에 접근하여 값을 갱신하게 될 소지가 있음.
            // A-05. mov         rbx,qword ptr [Case03::dataRacingSPtr+8h] // 컨트롤 블록의 포인터를 rbx에 저장
            // A-06. mov         rax,qword ptr [Case03::dataRacingSPtr]    // 관리 객체의 포인터를 rax에 저장
            // A-07. mov         qword ptr [tempSPtr],rax
            // A-07. mov         qword ptr [tempSPtr],rax
            // A-08. mov         qword ptr [rsp+28h],rbx
            if (100 != *tempSPtr)
            {
                cout << "Error!\n";
            }
    
            // ## 컨트롤 블록의 포인터와 관리 객체의 포인터를 레지스터에 저장하는 로직 ##
            // A-05. mov         rbx,qword ptr [Case03::dataRacingSPtr+8h] // 컨트롤 블록의 포인터를 rbx에 저장
            // A-06. mov         rax,qword ptr [Case03::dataRacingSPtr]    // 관리 객체의 포인터를 rax에 저장
            //
            // dataRacingSPtr    [0x00007FF6CDA85810] -> 접근 f0 5c 4f c5 f3 01 00 00 // 이 값은 관리 객체의 포인터로 이걸 타고 들어가면 관리 객체의 값이 나옴(int64_t).
            // dataRacingSPtr+8h [0x00007FF6CDA85818] -> 접근 e0 5c 4f c5 f3 01 00 00 // 이 값은 컨트롤 블록의 포인터로 이걸 타고 들어가면 컨트롤 블록에 접근 가능함(std::_Ref_count_obj2<__int64>).
            //
            //                          ┌ [rax], [rax+8]처럼 포인터를 타고 들어가는 디레퍼런싱(Dereferencing)을 통해 접근하게 되는 메모리의 값
            // 0x000001F3C54F5CE0 -> [ f8 33 a8 cd f6 7f 00 00 ] // 컨트롤 블록
            // 0x000001F3C54F5CE8    [ 02 00 00 00 01 00 00 00 ] // 컨트롤 블록
            // 0x000001F3C54F5CF0 -> [ 64 00 00 00 00 00 00 00 ] // 관리 객체(+ 컨트롤 블록에 포함된 _Storage)
            //   └ 레지스터에 저장되는 포인터
            // 
        }
        // while 문 블록 끝에서 수행되는 코드(조건에 따른 tempSPtr의 소멸 과정을 담고 있음 + Crash가 발생하는 빈도가 높은 곳 중 하나)
        // B-01. test        rbx,rbx    // rbx가 nullptr(0)인지 검사(rbx에는 컨트롤 블록의 포인터가 저장됨)
        // B-02. je          `Case03::Run'::`2'::<lambda_1>::operator()+20h // rbx가 nullptr(0)이면 while 문 시작 위치로 점프
        // B-03. mov         eax,0FFFFFFFFh         // -1
        // B-04. lock xadd   dword ptr [rbx+8],eax  // 컨트롤 블록의 _Uses에 접근한 다음 -1을 더하고, 더하기 이전 값을 eax에 저장
        // B-05. cmp         eax,1      // eax의 값이 1이라는 것은 이전 _Uses의 값이 1이었다는 의미(_Uses를 0으로 떨어트린 쪽이 이쪽이라는 뜻이기도 함)
        // B-06. jne         `Case03::Run'::`2'::<lambda_1>::operator()+20h // _Uses를 0으로 만든 게 이쪽이 아니면 while 문 시작 위치로 점프
        // B-07. mov         rax,qword ptr [rbx]    // rbx가 가리키는 메모리 주소가 가진 값을 rax에 복사
        // B-08. mov         rcx,rbx 
        // B-09. call        qword ptr [rax]    // rax가 가리키는 메모리 주소 값을 위치로 하여 서브루틴 호출(std::_Ref_count_obj2<__int64>::_Destroy(void))
        //                                      // ★ B-09는 Crash가 발생하는 지점 중 하나
        // B-10. mov         eax,0FFFFFFFFh // -1
        // B-11. lock xadd   dword ptr [rbx+0Ch],eax // 컨트롤 블록의 _Weaks에 접근한 다음 -1을 더하고, 더하기 이전 값을 eax에 저장
        // B-12. cmp         eax,1      // eax의 값이 1이면 이쪽에서 _Weaks의 값을 0으로 떨어트렸다는 것을 의미
        // B-13. jne         `Case03::Run'::`2'::<lambda_1>::operator()+20h // _Weaks의 값을 0으로 만든 게 이쪽이 아니면 while 문 시작 위치로 점프
        // B-14. mov         rax,qword ptr [rbx]
        // B-15. mov         rcx,rbx  
        // B-16. call        qword ptr [rax+8]  // rax + 8이 가리키는 메모리 주소 값을 위치로 하여 서브루틴 호출(std::_Ref_count_obj2<__int64>::_Delete_this())
        //                                      // ★ B-16은 추정했을 때 Crash를 유발하는 명령어 중 하나
        // B-17. jmp         `Case03::Run'::`2'::<lambda_1>::operator()+20h // while 문 시작 위치로 가서 처음부터 코드를 다시 수행
    } };

    while (true)
    {
        // TEMP
        creationCounter++;

        // !! 아래 보여지는 어셈블리 코드 자체는 Case01에서 생성된 것과 동일함. !!
        auto tempSPtr = make_shared<int64_t>(100);
        // C-01. mov         ecx,18h  
        // C-02. call        operator new // 이 과정을 거치면 rax에 동적할당한 메모리의 주소가 담김(★ 추정했을 때 Crash를 유발하는 명령어 중 하나).
        // C-03. mov         rbx,rax      // rbx에도 동적할당한 메모리의 주소를 담음.
        // C-04. mov         qword ptr [rsp+30h],rax  
        // C-05. xorps       xmm0,xmm0  
        // C-06. movups      xmmword ptr [rax],xmm0  
        // C-07. mov         dword ptr [rax+8],1    // strong ref : 1
        // C-08. mov         dword ptr [rax+0Ch],1  // weak ref : 1
        // C-09. mov         qword ptr [rax],rsi    // std::_Ref_count_obj2<__int64>에 대한 vtable 설정(디스어셈블리와 메모리 디버깅을 통한 추정)
        // C-10. add         rax,10h                // rax의 위치를 16바이트 만큼 이동
        // C-11. mov         qword ptr [rax],64h    // value : 100
    
        // 전역 shared_ptr을 로컬 영역에서 생성한 shared_ptr로 교체하려는 과정에서 문제가 생긴다.
        dataRacingSPtr = tempSPtr;
        // D-01. lock inc    dword ptr [rbx+8]        // strong refs : 2
        // D-02. mov         qword ptr [Case03::dataRacingSPtr],rax    // dataRacingSPtr의 _Ptr 교체
        // D-03. mov         rdi,qword ptr [Case03::dataRacingSPtr+8h] // dataRacingSPtr의 _Rep 포인터 값을 가져와 rdi에 저장
        // D-04. mov         qword ptr [Case03::dataRacingSPtr+8h],rbx // dataRacingSPtr의 _Rep 교체
        // 
        // dataRacingSPtr은 tempSPtr로 교체한 상황이고 rdi에는 이전 dataRacingSPtr의 컨트롤 블록 주소가 담겨 있다.
        // 아래 이어지는 코드는 이전 dataRacingSPtr의 컨트롤 블록을 대상으로 하는 레퍼런스 카운팅 및 소멸 과정에 대한 내용이다.
        // D-05. test        rdi,rdi            // rdi가 nullptr(0)인지 검사(컨트롤 블록이 nullptr인지 체크) 
        // D-06. je          Case03::Run+161h  
        // D-07. mov         eax,0FFFFFFFFh     // -1
        // D-08. lock xadd   dword ptr [rdi+8],eax // 이전 dataRacingSPtr의 _Rep의 멤버 _Uses(strong refs)에 접근한 다음 -1을 더하고, 더하기 이전 값을 eax에 저장
        // D-09. cmp         eax,1              // _Uses를 0으로 떨어트린 곳이 이쪽인지 확인
        // D-10. jne         Case03::Run+161h 
        // D-11. mov         rax,qword ptr [rdi] // rdi가 가리키는 메모리 주소가 가진 값을 rax에 복사(vtable의 시작 위치)
        // D-12. mov         rcx,rdi             // rdi에 저장된 값을 그대로 rcx에 복사
        // D-13. call        qword ptr [rax]     // rax가 가리키는 메모리 주소 값을 위치로 하여 서브루틴 호출(std::_Ref_count_obj2<__int64>::_Destroy(void))
        //                                       // ★ D-13는 Crash가 발생하는 지점 중 하나
        // D-14. mov         eax,0FFFFFFFFh          // -1
        // D-15. lock xadd   dword ptr [rdi+0Ch],eax // 이전 dataRacingSPtr의 _Rep의 멤버 _Weaks(weak refs)에 접근한 다음 -1을 더하고, 더하기 이전 값을 eax에 저장
        // D-16. cmp         eax,1               // _Weaks를 0으로 떨어트린 곳이 이쪽인지 확인
        // D-17. jne         Case03::Run+161h  
        // D-18. mov         rax,qword ptr [rdi] // rdi가 가리키는 메모리 주소가 가진 값을 rax에 복사(vtable의 시작 위치, eax는 rax의 하위 32비트 값이라 eax에 0FFFFFFFFh 값을 넣으면 rax도 바뀜)
        // D-19. mov         rcx,rdi             // rdi에 저장된 값을 그대로 rcx에 복사
        // D-20. call        qword ptr [rax+8]   // rax + 8이 가리키는 메모리 주소 값을 위치로 하여 서브루틴 호출(std::_Ref_count_obj2<__int64>::_Delete_this())
        //                                       // ★ D-20은 추정했을 때 Crash를 유발하는 명령어 중 하나
        //
        // ## 이 코드 자체가 std::_Ref_count_obj2<__int64>::_Delete_this()에 해당하는 서브루틴임. ##
        // --- C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.38.33130\include\memory
        //     delete this;
        // test        rcx,rcx              // rcx가 nullptr(0)인지 검사(컨트롤 블록이 nullptr인지 체크) 
        // je          std::_Ref_count_obj2<__int64>::_Delete_this+11h // rcx가 nullptr(0)이면 마지막 ret 쪽으로 점프
        // mov         rax,qword ptr [rcx]  // rcx가 가리키는 메모리 주소가 가진 값을 rax에 복사(vtable의 시작 위치)
        // mov         edx,1                // edx에 1 저장
        // jmp         qword ptr [rax+10h]  // std::_Ref_count_obj2<__int64>::`scalar deleting destructor'(unsigned int) 위치로 이동
        // ret
        // 
        // My C++ Console Project1.exe!std::_Ref_count_obj2<__int64>::`scalar deleting destructor'(unsigned int):
        // push        rbx  
        // sub         rsp,20h  
        // lea         rax,[std::_Ref_count_obj2<__int64>::`vftable']  
        // mov         rbx,rcx  
        // mov         qword ptr [rcx],rax  
        // test        dl,1  
        // je          std::_Ref_count_obj2<__int64>::`scalar deleting destructor'+22h  
        // mov         edx,18h  
        // call        operator delete // 메모리 해제는 여기서 진행
        // mov         rax,rbx  
        // add         rsp,20h  
        // pop         rbx  
        // ret
        //
        // ## 이 코드는 operator delete에 대응함. !!
        // --- D:\a\_work\1\s\src\vctools\crt\vcstartup\src\heap\delete_scalar_size.cpp ---
        //     operator delete(block);
        // jmp         operator delete
        // 
        // ## jmp operator delete가 점프하는 부분 !!
        // --- D:\a\_work\1\s\src\vctools\crt\vcstartup\src\heap\delete_scalar.cpp --------
        //     #ifdef _DEBUG
        //     _free_dbg(block, _UNKNOWN_BLOCK);
        //     #else
        //     free(block);
        // jmp         free // 메모리 해제 진행
    }
    // 아래 로직은 임시 객체 tempSPtr의 소멸 과정을 담고 있다.
    // E-01. mov         eax,0FFFFFFFFh  
    // E-02. lock xadd   dword ptr [rbx+8],eax  
    // E-03. cmp         eax,1  
    // E-04. jne         Case03::Run+0E0h
    // E-05. mov         rax,qword ptr [rbx]  
    // E-06. mov         rcx,rbx  
    // E-07. call        qword ptr [rax]  
    // E-08. mov         eax,0FFFFFFFFh  
    // E-09. lock xadd   dword ptr [rbx+0Ch],eax  
    // E-10. cmp         eax,1  
    // E-11. jne         Case03::Run+0E0h
    // E-12. mov         rax,qword ptr [rbx]  
    // E-13. mov         rcx,rbx  
    // E-14. call        qword ptr [rax+8]  
    // E-15. jmp         Case03::Run+0E0h

    // Case03에서 Crash는 대부분 2곳에서 발생한다.
    // 
    // 발생 지점 첫 번째
    // B-07. mov         rax,qword ptr [rbx]
    // B-08. mov         rcx,rbx
    // B-09. call        qword ptr [rax] <----- Crash 발생
    // 
    // 발생 지점 두 번째
    // D-11. mov         rax,qword ptr [rdi]
    // D-12. mov         rcx,rdi
    // D-13. call        qword ptr [rax] <----- Crash 발생
    // 
    // 원인이 발생하는 지점을 분석하면 둘 다 rax에 nullptr(0x0000000000000000)이란 값이 들어갔고 해당 값을 서브루틴의 위치로 사용하려 했다고 Crash가 발생한다.
    // 쉽게 생각해서 B-07과 D-11에서 가져온 값이 nullptr이고 이를 rax에 넣었다는 뜻이다.
    // 
    // --------------------------------------------------
    // 
    // 어셈블리 수준으로 코드를 분석했지만 그 어디에도 메모리 값을 0으로 초기화하는 곳이 없다.
    // (중요) 하지만 운영체제 차원에서 메모리를 관리하는 방식까지 고려한다면 명시되어 있지는 않아도 메모리 값을 0으로 미는 구간이 한 군데 존재한다.
    // 
    // https://learn.microsoft.com/ko-kr/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc
    // Windows 환경에서는 동적할당을 진행하면 내부에서 힙 관리자가 VirtualAlloc()을 호출한다.
    // 해당 함수를 이용해서 새로운 메모리를 할당한다면 그 메모리의 값은 전부 0으로 초기화된다.
    // 
    // 동적할당을 할 때마다 매번 VirtualAlloc()을 호출하는 것도 아니며 호출한다고 해도 매번 새로운 메모리를 할당하는 것도 아니다.
    // 일단 메모리 관리자 차원에서 해당 함수를 호출하면 메모리 블록 덩어리를 할당해서 예약(Committed)한다고 봐야 하며,
    // 이후에는 내부의 힙 관리자가 적절하게 배분하는 방식을 취해서 동적할당하는 과정이 수행된다.
    // C-02. call        operator new
    // 
    // https://learn.microsoft.com/ko-kr/windows/win32/api/memoryapi/nf-memoryapi-virtualfree
    // 메모리를 해제하는 과정에서는 VirtualFree()를 호출한다.
    // 
    // 마찬가지로 매번 VirtualFree()를 호출하는 건 아니지만 무한 루프를 통해 new로 할당하고 delete로 해제하는 작업을 반복하면 해당 함수가 호출되는 빈도 또한 증가한다.
    // B-16. call        qword ptr [rax+8]
    // D-20. call        qword ptr [rax+8]
    // 
    // 위 두 명령어는 std::_Ref_count_obj2<__int64>::_Delete_this()를 서브루틴으로 호출하는 함수이다.
    // 해당 명령어를 거치면 내부에서 "delete this" -> "operator delete" -> "free"를 호출하는 과정을 거친다.
    // 
    // 동적할당과 해제를 진행한다고 해도 힙 관리자 차원에서 매번 VirtualAlloc()과 VirtualFree()를 호출하는 건 아니다.
    // 운영체제의 힙 관리자는 적절한 시점에서 두 함수를 호출하기에 언제 어떻게 해당 함수가 호출되는지는 운영체제의 메모리 관리 방식에 달려 있다.
    // 
    // 하지만 쉬지 않고 무한 루프로 할당과 해제를 반복하는 만큼 VirtualFree()로 반환하고,
    // 해당 메모리의 페이지를 다시 VirtualAlloc()으로 재할당하는 과정이 중첩(?)되어 문제가 발생할 여지가 생긴다.
    // !! VirtualAlloc()으로 메모리를 할당하면 해당 메모리(페이지)는 전부 0으로 채워짐. !!
    // 
    // 만약 이 과정에서 어떤 레지스터가 해제되기 전 메모리의 주소를 들고 있는데 그 사이 VirtualAlloc()으로 재할당되었다고 하면?
    // 당연히 메모리의 값이 0으로 밀리지 때문에 문제가 발생한다.
    // 
    // --------------------------------------------------
    // 
    // 멀티스레딩 환경에서 분석하는 만큼 100% 정확하다고 보장할 순 없지만 이런 이유로 문제가 발생할 수 있구나 하고 추정하는 건 가능하다.
    // 
    // ## 발생 지점 첫 번째의 경우 ##
    // 스레드 첫 번째 수행
    // C-01. mov         ecx,18h
    //  ~
    // D-04. mov         qword ptr [Case03::dataRacingSPtr+8h],rbx // std::_Ref_count_obj2<__int64>를 생성하고 이를 dataRacingSPtr에 반영함.
    // 
    // 스레드 두 번째 수행
    // A-01. mov         rax,qword ptr [Case03::dataRacingSPtr+8h] // dataRacingSPtr의 컨트롤 블록의 포인터를 가지고 옴.
    // 
    // 스레드 첫 번째 수행
    // D-05. test        rdi,rdi
    //  ~
    // D-20. call        qword ptr [rax+8] // 이전 dataRacingSPtr의 레퍼런스 카운트 차감
    // 
    // 스레드 첫 번째 수행
    // E-01. mov         eax,0FFFFFFFFh  
    //  ~
    // E-14. call        qword ptr [rax+8] // tempSPtr의 소멸 과정 수행(내부에서 VirtualFree()로 반환되었다고 가정)
    // 
    // 스레드 첫 번째 수행
    // E-15. jmp         Case03::Run+0E0h
    // C-01. mov         ecx,18h
    // C-02. call        operator new // 내부에서 VirtualAlloc()으로 메모리를 새롭게 구성하고 값을 0으로 초기화하고 있는 도중이라고 가정
    //                                // 새로 구성하는 메모리의 영역과 반환한 메모리의 영역과 겹친다고 가정
    // 
    // 스레드 두 번째 수행
    // A-02. test        rax,rax
    //  ~
    // A-05. mov         rbx,qword ptr [Case03::dataRacingSPtr+8h] // Case03::dataRacingSPtr+8h는 컨트롤 블록이 어디있는지 나타내는 메모리 포인터(이건 0으로 밀리는 대상이 아님)
    //  ~
    // B-07. mov         rax,qword ptr [rbx] // 컨트롤 블록의 포인터를 타고 들어가면 메모리 값이 0으로 밀리고 있는 도중이라 rax에는 nullptr(0)이 들어감.
    //  ~                                       멀티스레딩 환경에서 할당하고 있는 과정이기에 이 순간 rbx가 가리키는 메모리 값을 Memory 창으로 조회하면 값이 0으로 나오지 않을 수도 있음.
    //  ~
    // B-09. call        qword ptr [rax] // rax의 값은 nullptr(0)이기에 이걸 위치로 하여 서브루틴을 호출하면 에러가 발생함.
    // 
    // (주의) 컨트롤 블록의 포인터를 가지고 오는 것과 컨트롤 블록의 포인터를 타고 값을 가지고 오는 건 다른 개념이다.
    // 0으로 초기화되는 부분은 컨트롤 블록의 포인터를 타고 들어갔을 때 조회되는 메모리 값이지 컨트롤 블록의 포인터 자체는 0으로 밀리지 않는다.
    // 
    // 레퍼런스 카운팅을 고려하면서 위 순서대로 차례차례 분석하면 결국은 문제가 터진다는 것을 알 수 있다.
    // 
    // ## 발생 지점 두 번째의 경우 ##
    // 문제가 발생하는 과정 자체는 발생 지점 첫 번째와 유사하기에 생략한다.
    // 
    // 일단 Crash가 발생하는 원인은 이러한 이유 때문이라고 짐작하고 있다.
    // 위 방식은 정석적인 순서를 풀어쓴 것이며 멀티스레딩 환경을 고려하면 실제로 코드가 수행되는 순서는 차이가 있을 수밖에 없다.
    // 하지만 문제가 발생하는 핵심이 되는 지점 자체는 정해져 있기 때문에 운이 나쁘게(?) 문제가 되는 핵심 로직이 순차적으로 수행되면 Crash는 발생할 수밖에 없다.
    //

    th.join();
}

END_NS

BEGIN_NS(Case04)

// https://en.cppreference.com/w/cpp/memory/shared_ptr/atomic2
// C++20부터 사용 가능한 템플릿 특수화로 구현된 스마트 포인터
atomic<shared_ptr<int64_t>> dataRacingAtomSPtr;

// 비교하기 위한 기본 스마트 포인터
shared_ptr<int64_t> dataRacingSPtr;

// Case03 코드를 스레드 안전한 방식으로 구동되게 만든 코드
void Run()
{
    // <<---- 테스트 코드 블록 시작 ----->>
    // !! 동적할당의 바이트 경계를 확인하기 위한 코드(allocation_aligned_byte_boundaries.cpp에 있는 내용) !!
    // int cntA[0x10]{ 0 };
    // int cntB[0x10]{ 0 };
    // 
    // for (int i = 0; i < 100'000; i++)
    // {
    //     char* ch = new char{ 'C' };
    // 
    //     uint64_t ptr = reinterpret_cast<uint64_t>(ch);
    //     cntA[ptr & 0xf]++; // 8바이트 경계를 맞춘다면 cntA[0x8]을 대상으로도 카운팅이 되어야 함.
    // 
    //     ptr = ptr >> 4; // 16진수 자릿수 하나 내리기
    //     cntB[ptr & 0xf]++;
    // }
    // 
    // for (int i = 0; i < 0x10; i++)
    // {
    //     cout << cntA[i] << '\n';
    // }
    // 
    // cout << '\n';
    // 
    // for (int i = 0; i < 0x10; i++)
    // {
    //     cout << cntB[i] << '\n';
    // }
    // !! Windows 64비트 환경에서 Visual Studio 2022로 위 코드를 실행하면 배열을 8바이트가 아닌 16바이트 경계에 맞춰서 할당함. !!
    // <<---- 테스트 코드 블록 끝 ----->>

    atomic<int> counter = 0;

    dataRacingAtomSPtr = make_shared<int64_t>(counter);
    // dataRacingSPtr = make_shared<int64_t>(counter);

    thread countingTh1{ [&] {
        while (true)
        {
            counter.fetch_add(1);
    
            auto tempSPtr = make_shared<int64_t>(counter);
    
            dataRacingAtomSPtr = tempSPtr;
            // dataRacingSPtr = tempSPtr;
        }
    } };
    
    thread countingTh2{ [&] {
        while (true)
        {
            counter.fetch_add(1);
        
            auto tempSPtr = make_shared<int64_t>(counter);
        
            dataRacingAtomSPtr.store(tempSPtr);
            // dataRacingSPtr = tempSPtr;
        }
    } };

    while (true)
    {
        shared_ptr<int64_t> tempSPtr;
        
        // 다음 두 코드는 같은 작업을 수행함.
        // tempSPtr = dataRacingAtomSPtr.load();
        tempSPtr = dataRacingAtomSPtr;
        
        // tempSPtr = dataRacingSPtr;

        // Do Something(출력 자체는 매우 느린 작업이기 때문에 문제의 상황이 최대한 발생되게 유도하고자 한다면 출력 코드를 주석처리할 것)
        // if (*tempSPtr == counter.load())
        // {
        //     cout << "Match!\n";
        // }
        // else
        // {
        //     cout << "Does Not Match!\n";
        // }
    }

    // 다음 코드로 주 스레드의 while 문을 실행하면 문제가 생기지 않는다.
    // atomic<shared_ptr<int64_t>> dataRacingAtomSPtr;
    // 
    // 하지만 다음 코드로 주 스레드의 while 문을 실행하면 문제가 생긴다.
    // shared_ptr<int64_t> dataRacingSPtr;
    // 
    // 후자의 경우로 프로그램을 실행하면 문제가 생기는 이유는 Case03에 적어두었다.
    // 
    // --------------------------------------------------
    // 
    // 전자의 경우(atomic<shared_ptr<int64_t>>)로 코드를 실행하면 문제가 생기지 않는 이유를 봐야 한다.
    // 
    // atomic<shared_ptr<int64_t>> dataRacingAtomSPtr;
    // 
    // while (true)
    // {
    //     shared_ptr<int64_t> tempSPtr;
    // 
    //     tempSPtr = dataRacingAtomSPtr; // <----- 이 부분
    // 
    //     ...
    // }
    // 
    // shared_ptr<T>의 대입 연산자 유형 중 atomic<shared_ptr<T>>를 받는 건 없기 때문에 컴파일러는 적절한 유형을 찾아서 컴파일한다.
    // 
    // !! 해당 부분은 Release 모드가 아닌 Debug로 확인할 것! !!
    // 
    // --------------------------------------------------
    // 
    // 1. atomic<shared_ptr<T>>를 shared_ptr<T>로 사용하기 위한 변환 연산자 호출
    // 
    // std::atomic<std::shared_ptr<_Ty>>::operator shared_ptr<_Ty>() const noexcept
    // {
    //     return load();
    // }
    // 
    // !! 내부에서 load()를 이어서 호출 !!
    // _NODISCARD shared_ptr<_Ty> atomic<shared_ptr<_Ty>>::load(const memory_order _Order = memory_order_seq_cst) const noexcept
    // {
    //     _Check_load_memory_order(_Order); // _Order의 값이 memory_order를 벗어났는지 확인하기 위한 함수
    // 
    //     shared_ptr<_Ty> _Result;
    //     const auto _Rep = this->_Repptr._Lock_and_load(); // <----- Lock 소유권 획득 + 컨트롤 블록 획득
    // 
    //     _Result._Ptr    = this->_Ptr.load(memory_order_relaxed); // 관리 객체를 원자적으로 가져옴(_Ptr은 _Atomic_ptr_base<T>에 있음).
    //                                                              // 원자적으로 가져오는 것이 아니라면 캐시 일관성이 깨질 수 있음.
    //     _Result._Rep    = _Rep;
    //     _Result._Incref(); // <----- _Rep를 거쳐 강한 참조(_Uses)를 하나 증가
    // 
    //     this->_Repptr._Store_and_unlock(_Rep); // <----- Lock 소유권 반환 + _Storage 복원
    // 
    //     return _Result; // <----- 새로 구성한 shared_ptr<T> 반환
    // }
    // 
    // atomic<shared_ptr<T>>의 load()가 추가적으로 호출하는 함수 _Lock_and_load()는 Spin-Wait Lock을 사용해서 스레드 안전함을 보장한다.
    // _Lock_and_load()는 컨트롤 블록의 주소를 정수로 가지는 _Locked_pointer<T>에서 호출하는 함수이며 원자적인 연산을 사용한다.
    // !! 멀티스레딩 환경에서 공유되는 데이터에 대한 동시 접근 및 수정을 진행할 때는 원자적인 연산과 같은 작업을 통해 스레드 안전함을 보장해야 함. !!
    // 
    // --------------------------------------------------
    // 
    // 2. 반환된 shared_ptr<T>를 이동 기반 변환 대입 연산자로 받는다.
    // 
    // template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
    // shared_ptr& operator=(shared_ptr<_Ty2>&& _Right) noexcept // take resource from _Right
    // {
    //     shared_ptr(_STD move(_Right)).swap(*this);
    //     return *this;
    // }
    // 
    // --------------------------------------------------
    // 
    // atomic<shared_ptr<T>>에서 구성 요소를 shared_ptr<T>로 변환(?)해서 반환하는 과정을 보면
    // 내부에서 Lock을 건 다음 필요한 구성 요소를 원자적으로 가져와 shared_ptr<T>를 구성한 다음 반환한다.
    // 
    // 원자적인 연산은 atomic<T>를 통해 이루어지며 이는 데이터 레이싱이 발생해도 스레드 안전한 자료형이다.
    // 

    countingTh1.join();
    countingTh2.join();
}

END_NS

BEGIN_NS(Case05)

// https://en.cppreference.com/w/cpp/memory/shared_ptr/atomic2
// C++20부터 사용 가능한 템플릿 특수화로 구현된 스마트 포인터
atomic<shared_ptr<int64_t>> dataRacingAtomSPtr;

// atomic<shared_ptr<T>>를 대상으로 load(), store() 하는 건 스레드 안전하다.
// 하지만 atomic<shared_ptr<T>>을 사용한다고 해도 관리 객체에 접근하는 건 스레드 안전하지 않다.
void Run()
{
    dataRacingAtomSPtr = make_shared<int64_t>(100);

    int64_t localV1 = 0;
    int64_t localV2 = 0;

    thread th1{ [&] {

        while (true)
        {
            // load()를 통해서 가져오는 건 스레드 안전하다.
            shared_ptr<int64_t> tempSPtr = dataRacingAtomSPtr.load();

            // 하지만 관리 객체에 접근해서 발생하는 데이터 레이스 현상은 스레드 안전하지 않다.
            *tempSPtr = localV1++;
        }
    } };

    thread th2{ [&] {

        while (true)
        {
            // load()를 통해서 가져오는 건 스레드 안전하다.
            shared_ptr<int64_t> tempSPtr = dataRacingAtomSPtr.load();

            // 하지만 관리 객체에 접근해서 발생하는 데이터 레이스 현상은 스레드 안전하지 않다.
            *tempSPtr = localV2++;
        }
    } };

    // 지금 th1과 th2에서 가져오는 스마트 포인터의 관리 객체는 int64_t이다.
    // 하지만 관리 객체가 이처럼 단순하 방식으로 표현되는 것이 아닌 스마트 포인터를 멤버로 들고 있다면 Case01, Case02, Case03에서 생기는 문제가 똑같이 발생한다.
    
    int64_t mainV = 0;

    while (true)
    {
        shared_ptr<int64_t> tempSPtr = make_shared<int64_t>(mainV++);

        // store()를 통해서 새로운 스마트 포인터를 설정하는 건 스레드 안전하다.
        dataRacingAtomSPtr.store(tempSPtr);
    }

    // atomic<shared_ptr<T>>가 원자성을 보장해주는 건 shared_ptr<T> 그 자체이다.
    // atomic<shared_ptr<T>>는 shared_ptr<T>로 표현되는 관리 객체 T에 대한 스레드 안전함을 보장하지 않는다.
    // atomic<shared_ptr<T>>가 관리 객체 T까지 관리해주는 것은 아니니까 이 차이점을 숙지해야 한다.
    // 
    // (중요) atomic<shared_ptr<T>>에서 스레드 안전하게 보장하는 건 shared_ptr<T> 그 자체의 원자성이지 이것의 관리 객체까지 스레드 안전하게 처리하는 것은 아니다.
    // 따라서 관리 객체가 단순 기본 자료형이 아닌 클래스와 같이 복잡한 자료형이라면 뮤텍스나 조건 변수를 통한 동기화 과정이 필요하다.

    th1.join();
    th2.join();
}

END_NS

BEGIN_NS(Case06)

atomic<shared_ptr<int64_t>> nullAtomSPtr;

// atomic<shared_ptr<T>>의 load()를 통해 반환되는 shared_ptr<T>는 빈 스마트 포인터도 반환한다.
// 개발할 때 빈 스마트 포인터를 사용하지 않기로 약속한 것이 아니거나 널 객체 패턴을 쓰는 것이 아니라면 이 부분도 신경써서 작업해야 한다.
void Run()
{
    // 빈 스마트 포인터를 관리하게 설정
    nullAtomSPtr.store(nullptr); // nullAtomSPtr.store(shared_ptr<int64_t>{ nullptr });

    // atomic<shared_ptr<T>>에서 스마트 포인터를 가지고 오기.
    shared_ptr<int64_t> tempSPtr = nullAtomSPtr.load();

    if (nullptr == tempSPtr)
    {
        cout << "nullptr...\n";
    }
    else
    {
        cout << "not nullptr...\n";
    }
}

END_NS

/*****************
*      Main      *
*****************/

int main()
{
    // Case01::Run();

    // Case02::Run();

    // Case03::Run();

    // Case04::Run();

    // Case05::Run();
    
    Case06::Run();

    return true;
}
