// Update Date : 2024-12-26
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <memory>
#include <thread>

using namespace std;

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
// 6. weak_ptr_details.cpp <-----
//
// # shared_ptr의 관리 객체에서 자기 자신을 반환할 때 필요한 내용
// 7. enable_shared_from_this.cpp
// 8. enable_shared_from_this_details.cpp
// 9. enable_shared_from_this_examples.cpp
//
// # shared_ptr을 멀티스레딩 환경에서 사용할 때 발생할 수 있는 문제점을 기술한 내용
// 10. allocation_aligned_byte_boundaries.cpp(사전지식)
// 11. volatile_atomic_cache_coherence_and_memory_order.cpp(사전지식)
// 12. (중요) shared_ptr_multi_threading_issues.cpp

// 용어 정리
// 
// 관리 객체(Managed Object) : 스마트 포인터를 통해서 사용 및 관리하고자 하는 대상 객체(이것의 메모리를 "관리 객체의 메모리"라고 칭할 것임)
// 레퍼런스 카운팅 블록 : 참조 카운트를 관리하기 위한 블록
// 컨트롤 블록(다음 내용을 포함하는 개념)
// : 레퍼런스 카운팅 관리
// : 객체의 삭제 방식이나 메모리 관리 방식을 지정하기 위한 Deleter나 Allocator를 가질 수 있음.
// : 디버깅이나 메모리 관리 및 프로파일링을 위한 메타 데이터를 가질 수 있음.
// : 컨트롤 블록 안에 관리 객체의 메모리를 할당하여 최적화하는 경우도 있음(둘은 다른 개념).
//
// 컨트롤 블록은 레퍼런스 카운팅 블록을 포함하는 상위 개념이지만 찾아보면 두 개념을 동일하게 보는 경우가 많다.
// 컨트롤 블록 안에 관리 객체의 메모리를 함께 할당하는 경우도 있지만 이건 최적화를 위해서 그렇게 한 것일 뿐, 관리 객체와 컨트롤 블록 두 개념은 별개로 봐야 한다.
//
// Deleter(삭제자) : 관리 객체의 수명이 다했을 때 이를 해제할 방식이 기술되어 있는 Callable
// Allocator(할당자) : 컨트롤 블록을 할당하고 해제하기 위해 사용할 클래스
//
// Deleter는 관리 객체와 연관되어 있고, Allocator는 컨트롤 블록과 연관되어 있다.
// 할당자의 deallocate()와 Deleter는 개념상 서로 연관성이 없지만 인터페이스만 보고 기능을 유추하려 하면 혼란스러울 수 있다.
// !! 할당자의 deallocate()와 Deleter는 서로 연관성이 없으며 아예 독립된 별개의 기능을 수행한다. !!

/***************************
*      Object Classes      *
***************************/

class TestObject
{
public:
    TestObject(int valA) : _valA{ valA }
    {
        cout << "TestObject()\n";

        cout << std::hex << "\tPtr : 0x" << this << "\n\n";
        cout << std::dec;
    }

    /* virtual */ ~TestObject()
    {
        cout << "~TestObject()\n";

        cout << std::hex << "\tPtr : 0x" << this << "\n\n";
        cout << std::dec;
    }

private:
    int _valA = 0;
};

class TestObjectEx : public TestObject
{
public:
    TestObjectEx(int valA, double valB)
        : TestObject{ valA }, _valB{ valB }
    {
        cout << "TestObjectEx()\n";

        cout << std::hex << "\tPtr : 0x" << this << "\n\n";
        cout << std::dec;
    }

    /* virtual */ ~TestObjectEx()
    {
        cout << "~TestObjectEx()\n";

        cout << std::hex << "\tPtr : 0x" << this << "\n\n";
        cout << std::dec;
    }

private:
    double _valB = 0.0;
};

/*****************
*      Main      *
*****************/

int main()
{
    // weak_ptr는 shared_ptr의 참조 카운팅에 영향을 받지 않는 스마트 포인터이다.
    // 내부에서 컨트롤 블록의 약한 참조(weak refs | _Weaks)를 통해서 관리된다.
    // 
    // weak_ptr은 강한 참조를 사용하지 않기 때문에 shared_ptr 간 참조에서 발생하는 순환 참조 문제를 해결할 때 유용하다.
    // 관리 객체의 파괴 여부를 조회할 때도 사용할 수 있긴 하지만 이게 컨트롤 블록의 해제 여부를 나타내는 것은 아니니 주의해야 한다.
    // 
    // 이러한 상황이 메모리 누수인 것은 아니지만 관리 객체의 소멸자가 호출되었는데
    // 컨트롤 블록을 해제하지 않은 weak_ptr이 다수 존재한다면 메모리가 누수된 것처럼 동작할 수 있다.
    // 따라서 적절하게 weak_ptr을 nullptr(빈 스마트 포인터)로 밀거나 reset()을 호출하여 컨트롤 블록이 해제될 수 있게 유도하는 과정이 필요하다.

    cout << "-------------------------#01#-------------------------\n\n";

    /****************************************************
    *      Copy Operation : shared_ptr -> weak_ptr      *
    ****************************************************/

    // shared_ptr를 weak_ptr에 복사
    {
        shared_ptr<TestObject> sptr = make_shared<TestObject>(100);
        
        // A(복사 기반 변환 생성자)
        weak_ptr<TestObject> wptr1 = sptr; // wptr1{ sptr };
        
        // B(복사 기반 변환 대입 연산자)
        weak_ptr<TestObject> wptr2;
        wptr2 = sptr;
        
        cout << "# Separator #\n\n";
        
        cout << "sptr.use_count() : " << sptr.use_count() << "\n\n";
        
        cout << "# Separator #\n\n";
        
        // 소멸 과정에서 weak_ptr을 거치며 컨트롤 블록이 해제될 수 있도록 함.
        sptr = nullptr;
        
        cout << "sptr = nullptr;\n\n";
        
        cout << "sptr.use_count() : " << sptr.use_count() << "\n\n";

        // --------------------------------------------------
        // 
        // ***** A 부분 *****
        // 
        // --------------------------------------------------
        //
        // 1. weak_ptr<TestObject> wptr1 = sptr;
        // 
        // shared_ptr<TestObject>를 받는 "복사 기반 변환 생성자" 호출
        // 
        // template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        // weak_ptr(const shared_ptr<_Ty2>& _Other) noexcept
        // {
        //     this->_Weakly_construct_from(_Other); // shared_ptr keeps resource alive during conversion
        // }
        // 
        // !! _Weakly_construct_from() 호출 !!
        // template <class _Ty2>
        // void _Ptr_base<T>::_Weakly_construct_from(const _Ptr_base<_Ty2>& _Other) noexcept // implement weak_ptr's ctors
        // {
        //     // _Rep가 존재하기 때문에 아래 if 문에 들어가 실행됨.
        //     if (_Other._Rep)
        //     {
        //         _Ptr = _Other._Ptr;
        //         _Rep = _Other._Rep;
        // 
        //         // 다음 코드는 원자적으로 수행됨.
        //         _Rep->_Incwref();
        //     }
        //     else
        //     {
        //         _STL_INTERNAL_CHECK(!_Ptr && !_Rep);
        //     }
        // }
        // 
        // --------------------------------------------------
        // 
        // 2. _Rep->_Incwref() 호출
        // 
        // void _Ref_count_base::_Incwref() noexcept // increment weak reference count
        // {
        //     // 원자적인 증가 연산을 수행
        //     _MT_INCR(_Weaks);
        // }
        //
        // --------------------------------------------------
        // 
        // ***** B 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 3. wptr2 = sptr;
        // 
        // 위 코드를 거치며 "복사 기반 변환 대입 연산자" 호출
        // 
        // template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        // weak_ptr& operator=(const shared_ptr<_Ty2>& _Right) noexcept
        // {
        //     weak_ptr(_Right).swap(*this);
        //     return *this;
        // }
        // 
        // 타입을 추론하는 과정은 shared_ptr_details.cpp에 적었으니 이 부분은 생략한다.
        // 
        // !! 임시 객체 weak_ptr(_Right)를 생성하기 위한 "복사 기반 변환 생성자" 호출 !!
        // template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        // weak_ptr(const shared_ptr<_Ty2>& _Other) noexcept
        // {
        //     this->_Weakly_construct_from(_Other); // shared_ptr keeps resource alive during conversion
        // }
        // 
        // !! _Weakly_construct_from() 호출 !!
        // template <class _Ty2>
        // void _Ptr_base<T>::_Weakly_construct_from(const _Ptr_base<_Ty2>& _Other) noexcept // implement weak_ptr's ctors
        // {
        //     if (_Other._Rep)
        //     {
        //         _Ptr = _Other._Ptr;
        //         _Rep = _Other._Rep;
        //         _Rep->_Incwref();
        //     }
        //     else
        //     {
        //         _STL_INTERNAL_CHECK(!_Ptr && !_Rep);
        //     }
        // }
        // 
        // 위 과정은 A에서 진행한 것과 똑같다.
        //

        // weak_ptr의 소멸자 호출 과정은 다음과 같다.
        //
        // --------------------------------------------------
        // 
        // 1. weak_ptr의 소멸자 호출
        // 
        // ~weak_ptr() noexcept
        // {
        //     this->_Decwref();
        // }
        // 
        // --------------------------------------------------
        // 
        // 2. _Decwref() 호출(스마트 포인터 관리 클래스)
        // 
        // void _Ptr_base<T>::_Decwref() noexcept // decrement weak reference count
        // {
        //     // 컨트롤 블록이 유효하다면 아래 if 문의 로직을 수행함.
        //     if (_Rep)
        //     {
        //         // weak_ptr도 컨트롤 블록을 감싼 일종의 래퍼 클래스임.
        //         // 이런 이유로 컨트롤 블록 차원에서 _Weaks를 감소시켜 컨트롤 블록의 소멸 과정을 유도함.
        //         // !! _Ptr_base<T>는 weak_ptr<T>와 shared_ptr<T>의 부모 클래스이지 컨트롤 블록의 최상위 부모가 아님. !!
        //         // !! 컨트롤 블록의 최상위 부모 클래스는 _Ref_count_base임. !!
        //         _Rep->_Decwref();
        //     }
        // }
        // 
        // --------------------------------------------------
        // 
        // 3. _Decwref() 호출(컨트롤 블록의 최상위 클래스)
        // 
        // void _Ref_count_base::_Decwref() noexcept // decrement weak reference count
        // {
        //     // 원자적인 감소를 수행했을 때 _Weaks가 0에 도달한다면 _Delete_this()를 호출함.
        //     if (_MT_DECR(_Weaks) == 0)
        //     {
        //         // 순수 가상 함수(컨트롤 블록의 소멸 과정 유도)
        //         _Delete_this();
        //     }
        // }
        // 
        // 컨트롤 블록에 있는 _Decwref()는 shared_ptr의 _Decref()에서 _Uses가 0에 도달할 때 호출되기도 한다.
        // 
        // --------------------------------------------------
        // 
        // 4. _Delete_this() 호출
        // 
        // void _Ref_count_obj2<T>::_Delete_this() noexcept override // destroy self
        // {
        //     delete this;
        // }
        // 
        // 소멸자가 호출된 이후 weak_ptr을 다시 사용할 일은 없기 때문에 _Rep를 nullptr로 갱신하는 작업은 하지 않는다.
        //

        cout << "## End Of Block ##\n\n";
    }

    cout << "-------------------------#02#-------------------------\n\n";

    /****************************************************
    *      Move Operation : shared_ptr -> weak_ptr      *
    ****************************************************/

    // shared_ptr를 weak_ptr에 이동
    {
        shared_ptr<TestObject> sptr1 = make_shared<TestObject>(200);
        shared_ptr<TestObject> sptr2 = make_shared<TestObject>(300);
        
        // A(이동 기반 변환 생성자가 아닌 "복사 기반 변환 생성자"가 호출됨)
        weak_ptr<TestObject> wptr1 = std::move(sptr1); // wptr1{ std::move(sptr1) };
        
        // B(이동 기반 변환 대입 연산자가 아닌 "복사 기반 변환 대입 연산자"가 호출됨)
        weak_ptr<TestObject> wptr2;
        wptr2 = std::move(sptr2);
        
        cout << "# Separator #\n\n";
        
        // 실제로는 복사 과정이 이루어졌기 때문에 sptr1.use_count()는 0을 반환하지 않는다.
        cout << "sptr1.use_count() : " << sptr1.use_count() << "\n\n";
        cout << "sptr2.use_count() : " << sptr2.use_count() << "\n\n";

        // --------------------------------------------------
        // 
        // ***** A 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 1. weak_ptr<TestObject> wptr1 = std::move(sptr1); // wptr1{ std::move(sptr1) };
        // 
        // shared_ptr<TestObject>를 받는 "복사 기반 변환 생성자" 호출
        // !! std::move()로 전달했지만 이동 기반의 무언가가 호출되는 것이 아니니까 주의해야 함. !!
        // 
        // template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        // weak_ptr(const shared_ptr<_Ty2>& _Other) noexcept
        // {
        //     this->_Weakly_construct_from(_Other); // shared_ptr keeps resource alive during conversion
        // }
        //
        // --------------------------------------------------
        // 
        // ***** B 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 2. wptr2 = std::move(sptr2);
        // 
        // 위 코드를 거치며 "복사 기반 변환 대입 연산자" 호출
        // !! std::move()로 전달했지만 이동 기반의 무언가가 호출되는 것이 아니니까 주의해야 함. !!
        // 
        // template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        // weak_ptr& operator=(const shared_ptr<_Ty2>& _Right) noexcept
        // {
        //     weak_ptr(_Right).swap(*this);
        //     return *this;
        // }
        // 
        // 코드 수행 과정은 "Copy Operation : shared_ptr -> weak_ptr"의 B 부분과 동일하다.
        //

        cout << "## End Of Block ##\n\n";
    }

    cout << "-------------------------#03#-------------------------\n\n";

    /***********************************************************
    *      Copy & Move Operation : weak_ptr -> shared_ptr      *
    ***********************************************************/

    // weak_ptr를 shared_ptr에 복사 혹은 이동
    {
        shared_ptr<TestObject> sptr1 = make_shared<TestObject>(100);
        weak_ptr<TestObject>   wptr1 = sptr1; // wptr1{ sptr1 };

        // A(복사 기반 변환 생성자)
        shared_ptr<TestObject> sptr2{ wptr1 };

        // B(이동 기반 변환 생성자가 아닌 "복사 기반 변환 생성자"가 호출됨)
        shared_ptr<TestObject> sptr3{ std::move(wptr1) };

        cout << "# Separator #\n\n";

        // 모든 과정이 복사 기반으로 이루어졌기 때문에 모든 use_count()는 3을 반환해야 한다.
        cout << "sptr1.use_count() : " << sptr1.use_count() << "\n\n";
        cout << "sptr2.use_count() : " << sptr2.use_count() << "\n\n";
        cout << "sptr3.use_count() : " << sptr3.use_count() << "\n\n";

        // --------------------------------------------------
        // 
        // ***** A 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 1. shared_ptr<TestObject> sptr2{ wptr1 };
        // 
        // 위 코드를 거치며 "복사 기반 변환 생성자" 호출
        // 
        // template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        // explicit shared_ptr(const weak_ptr<_Ty2>& _Other) // construct shared_ptr object that owns resource *_Other
        // {
        //     if (!this->_Construct_from_weak(_Other))
        //     {
        //         // weak_ptr을 shared_ptr로 변환할 수 없는 상태라면 예외를 던짐.
        //         _Throw_bad_weak_ptr();
        //     }
        // }
        // 
        // [[noreturn]] inline void _Throw_bad_weak_ptr() {
        //     _THROW(bad_weak_ptr{});
        // }
        // 
        // 생성자가 explicit으로 되어 있기에 암시적 형변환을 허용하지 않는다(명시적으로 생성자의 인자를 넘겨야 함).
        // 
        // --------------------------------------------------
        // 
        // 2. _Construct_from_weak() 호출
        // 
        // template <class _Ty2>
        // bool _Ptr_base<T>::_Construct_from_weak(const weak_ptr<_Ty2>& _Other) noexcept
        // {
        //     // implement shared_ptr's ctor from weak_ptr, and weak_ptr::lock()
        //     // _Incref_nz()는 원자성을 보장하는 함수임.
        //     if (_Other._Rep && _Other._Rep->_Incref_nz())
        //     {
        //         _Ptr = _Other._Ptr;
        //         _Rep = _Other._Rep;
        // 
        //         return true;
        //     }
        // 
        //     // _Incref_nz()가 false를 반환했으면 _Ptr과 _Rep를 갱신하지 않음.
        //     // 대상이 빈 스마트 포인터이기 때문에 이 경우 _Ptr과 _Rep에는 nullptr로 되어 있어야 함.
        //     return false;
        // }
        // 
        // _Incref_nz()에 대한 설명은 "weak_ptr<T>::lock()" 쪽에 있다.
        //
        // --------------------------------------------------
        // 
        // ***** B 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 3. shared_ptr<TestObject> sptr3{ std::move(wptr1) };
        // 
        // 위 코드를 거치며 "복사 기반 변환 대입 연산자" 호출
        // !! std::move()로 전달했지만 이동 기반의 무언가가 호출되는 것이 아니니까 주의해야 함. !!
        // 
        // template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        // explicit shared_ptr(const weak_ptr<_Ty2>& _Other) // construct shared_ptr object that owns resource *_Other
        // {
        //     if (!this->_Construct_from_weak(_Other))
        //     {
        //         // weak_ptr을 shared_ptr로 변환할 수 없는 상태라면 예외를 던짐.
        //         _Throw_bad_weak_ptr();
        //     }
        // }
        // 
        // 진행 과정은 A 부분과 동일하다.
        // 

        cout << "## End Of Block ##\n\n";
    }

    cout << "-------------------------#04#-------------------------\n\n";

    /*********************************************************
    *      Copy & Move Operation : weak_ptr -> weak_ptr      *
    *********************************************************/

    // weak_ptr 간 복사 혹은 이동
    {
        shared_ptr<TestObject> sptr = make_shared<TestObject>(400);
        weak_ptr<TestObject> wptr1 = sptr; // wptr1{ sptr };
        
        // A(복사 생성자)
        weak_ptr<TestObject> wptr2 = wptr1; // wptr2{ wptr1 };
        
        // B(복사 대입 연산자)
        weak_ptr<TestObject> wptr3;
        wptr3 = wptr1;
        
        // C(이동 생성자)
        weak_ptr<TestObject> wptr4 = std::move(wptr1); // wptr3{ std::move(wptr1) };
        
        cout << "# Separator #\n\n";
        
        wptr1 = sptr;
        
        cout << "# Separator #\n\n";
        
        // D(이동 대입 연산자)
        weak_ptr<TestObject> wptr5;
        wptr5 = std::move(wptr1);

        // --------------------------------------------------
        // 
        // ***** A 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 1. weak_ptr<TestObject> wptr2 = wptr1; // wptr2{ wptr1 };
        // 
        // 위 코드를 거치며 복사 생성자 호출
        // 
        // weak_ptr(const weak_ptr& _Other) noexcept
        // {
        //     this->_Weakly_construct_from(_Other); // same type, no conversion
        // }
        // 
        // !! _Weakly_construct_from() 호출 !!
        // template <class _Ty2>
        // void _Ptr_base<T>::_Weakly_construct_from(const _Ptr_base<_Ty2>& _Other) noexcept // implement weak_ptr's ctors
        // {
        //     if (_Other._Rep)
        //     {
        //         _Ptr = _Other._Ptr;
        //         _Rep = _Other._Rep;
        //         _Rep->_Incwref();
        //     }
        //     else
        //     {
        //         _STL_INTERNAL_CHECK(!_Ptr && !_Rep);
        //     }
        // }
        // 
        // --------------------------------------------------
        // 
        // ***** B 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 2. wptr3 = wptr1;
        // 
        // 위 과정을 거치며 복사 대입 연산자 호출
        // 
        // weak_ptr& operator=(const weak_ptr& _Right) noexcept
        // {
        //     weak_ptr(_Right).swap(*this);
        //     return *this;
        // }
        // 
        // !! 임시 객체 weak_ptr(_Right)를 생성하기 위한 복사 생성자 호출 !!
        // weak_ptr(const weak_ptr& _Other) noexcept
        // {
        //     this->_Weakly_construct_from(_Other); // same type, no conversion
        // }
        // 
        // !! _Weakly_construct_from() 호출 !!
        // template <class _Ty2>
        // void _Ptr_base<T>::_Weakly_construct_from(const _Ptr_base<_Ty2>& _Other) noexcept // implement weak_ptr's ctors
        // {
        //     if (_Other._Rep)
        //     {
        //         _Ptr = _Other._Ptr;
        //         _Rep = _Other._Rep;
        //         _Rep->_Incwref();
        //     }
        //     else
        //     {
        //         _STL_INTERNAL_CHECK(!_Ptr && !_Rep);
        //     }
        // }
        // 
        // --------------------------------------------------
        // 
        // 3. weak_ptr(_Right).swap(*this);
        // 
        // 임시 객체 생성 후 swap() 호출
        // 
        // void weak_ptr<T>::swap(weak_ptr& _Other) noexcept
        // {
        //     // this는 wptr1을 복사한 임시 객체이고, _Other는 wptr3임.
        //     this->_Swap(_Other);
        // }
        // 
        // !! _Swap() 호출 !!
        // !! 코드를 보면 컨트롤 블록 _Rep가 가진 _Weaks의 값은 4로 되어 있어야 함(임시 객체에서 카운팅한 것 포함) !!
        // void _Swap(_Ptr_base& _Right) noexcept // swap pointers
        // {
        //     // 임시 객체에 있는 내용와 wptr3에 있는 내용 교환
        //     _STD swap(_Ptr, _Right._Ptr);
        //     _STD swap(_Rep, _Right._Rep);
        // }
        // 
        // --------------------------------------------------
        // 
        // 4. 콜 스택을 빠져나오고 임시 객체의 소멸자 호출
        // 
        // weak_ptr& operator=(const weak_ptr& _Right) noexcept
        // {
        //     // 교환 과정을 마치면 wptr3와 wptr1는 같은 _Ptr과 _Rep를 가지고 있어야 함.
        //     // 임시 객체 weak_ptr(_Right)는 wptr3가 가지고 있던 _Ptr과 _Rep를 가리키고 있음.
        //     weak_ptr(_Right).swap(*this);
        //     return *this;
        // }
        // 
        // !! 임시 객체의 소멸자 호출 !!
        // ~weak_ptr() noexcept
        // {
        //     this->_Decwref();
        // }
        // 
        // !! _Decwref() 호출 !!
        // void _Ptr_base<T>::_Decwref() noexcept // decrement weak reference count
        // {
        //     // wptr3는 빈 스마트 포인터였기 때문에 _Rep는 nullptr임.
        //     // 따라서 아래 if 문의 코드는 실행되지 않음.
        //     if (_Rep)
        //     {
        //         _Rep->_Decwref();
        //     }
        // }
        // 
        // --------------------------------------------------
        // 
        // ***** C 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 5. weak_ptr<TestObject> wptr4 = std::move(wptr1); // wptr3{ std::move(wptr1) };
        // 
        // 위 코드를 거치며 이동 생성자 호출
        // 
        // weak_ptr(weak_ptr&& _Other) noexcept
        // {
        //     this->_Move_construct_from(_STD move(_Other));
        // }
        // 
        // !! _Move_construct_from() 호출 !!
        // template <class _Ty2>
        // void _Ptr_base<T>::_Move_construct_from(_Ptr_base<_Ty2>&& _Right) noexcept
        // {
        //     // implement shared_ptr's (converting) move ctor and weak_ptr's move ctor
        //     _Ptr = _Right._Ptr;
        //     _Rep = _Right._Rep;
        // 
        //     _Right._Ptr = nullptr;
        //     _Right._Rep = nullptr;
        // }
        // 
        // 소유권을 이전하는 개념이기 때문에 _Weaks를 증가시키는 코드는 없다.
        // 
        // --------------------------------------------------
        // 
        // ***** D 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 6. wptr5 = std::move(wptr1);
        // 
        // 위 코드를 거치며 이동 대입 연산자 호출
        // 
        // weak_ptr& operator=(weak_ptr&& _Right) noexcept
        // {
        //     weak_ptr(_STD move(_Right)).swap(*this);
        //     return *this;
        // }
        // 
        // !! 임시 객체 weak_ptr(_Right)를 생성하기 위한 "복사 기반 변환 생성자" 호출 !!
        // weak_ptr(weak_ptr&& _Other) noexcept
        // {
        //     this->_Move_construct_from(_STD move(_Other));
        // }
        // 
        // !! _Move_construct_from() 호출 !!
        // template <class _Ty2>
        // void _Ptr_base<T>::_Move_construct_from(_Ptr_base<_Ty2>&& _Right) noexcept
        // {
        //     // implement shared_ptr's (converting) move ctor and weak_ptr's move ctor
        //     _Ptr = _Right._Ptr;
        //     _Rep = _Right._Rep;
        // 
        //     _Right._Ptr = nullptr;
        //     _Right._Rep = nullptr;
        // }
        // 
        // --------------------------------------------------
        // 
        // 7. 콜 스택을 빠져나오고 임시 객체의 소멸자 호출
        // 
        // !! 임시 객체의 소멸자 호출 !!
        // ~weak_ptr() noexcept
        // {
        //     this->_Decwref();
        // }
        // 
        // void _Ptr_base<T>::_Decwref() noexcept // decrement weak reference count
        // {
        //     if (_Rep)
        // {
        //         _Rep->_Decwref();
        //     }
        // }
        // 

        cout << "## End Of Block ##\n\n";
    }

    cout << "-------------------------#05#-------------------------\n\n";

    /***********************************
    *      weak_ptr<T>::expired()      *
    ***********************************/

    // expired()로 객체의 유효성을 검증하는 로직
    {
        shared_ptr<TestObject> sptr = make_shared<TestObject>(500);
        
        weak_ptr<TestObject> wptr = sptr; // wptr{ sptr };
        
        if (false == wptr.expired())
        {
            cout << "sptr is not expired...\n\n";
        }
        else // if (true == wptr.expired())
        {
            cout << "sptr is expired...\n\n";
        }
        
        sptr = nullptr;
        
        if (false == wptr.expired())
        {
            cout << "sptr is not expired...\n\n";
        }
        else // if (true == wptr.expired())
        {
            cout << "sptr is expired...\n\n";
        }

        // weak_ptr의 expired()는 다음 과정을 거친다.
        //
        // --------------------------------------------------
        // 
        // 1. expired() 호출
        // 
        // _NODISCARD bool weak_ptr<T>::expired() const noexcept
        // {
        //     return this->use_count() == 0;
        // }
        // 
        // !! use_count() 호출 !!
        // _NODISCARD long _Ptr_base<T>::use_count() const noexcept
        // {
        //     // 스마트 포인터는 일종의 래퍼 클래스이기 때문에 컨트롤 블록이 가진 _Uses의 값을 가져와야 함. !!
        //     return _Rep ? _Rep->_Use_count() : 0;
        // }
        // 
        // !! 컨트롤 블록의 _Use_count() 호출 !!
        // long _Ref_count_base::_Use_count() const noexcept
        // {
        //     return static_cast<long>(_Uses);
        // }
        // 
        // weak_ptr의 expired()는 컨트롤 블록이 유효하고 _Uses 값이 0이 아니면 true를 반환하며 그 외의 경우에는 false를 반환한다.
        // _Uses는 shared_ptr의 상태에 의존적인 값이라는 것을 명심하자.
        // 
    }

    cout << "-------------------------#06#-------------------------\n\n";

    /********************************
    *      weak_ptr<T>::lock()      *
    ********************************/

    // lock()으로 객체에 접근하는 로직
    {
        shared_ptr<TestObject> sptr = make_shared<TestObject>(600);
        
        weak_ptr<TestObject> wptr = sptr; // wptr{ sptr };
        
        if (shared_ptr<TestObject> tempPtr = wptr.lock())
        {
            cout << "wptr.lock() returned a valid shared_ptr...\n\n";
        }
        else // if (nullptr == tempPtr)
        {
            cout << "wptr.lock() returned nullptr...\n\n";
        }
        
        sptr = nullptr;
        
        if (shared_ptr<TestObject> tempPtr = wptr.lock())
        {
            cout << "wptr.lock() returned a valid shared_ptr...\n\n";
        }
        else // if (nullptr == tempPtr)
        {
            cout << "wptr.lock() returned nullptr...\n\n";
        }

        // weak_ptr의 lock()은 다음 과정을 거친다.
        // 
        // --------------------------------------------------
        // 
        // 1. weak_ptr의 lock() 호출
        // 
        // _NODISCARD shared_ptr<_Ty> weak_ptr<_Ty>::lock() const noexcept // convert to shared_ptr
        // {
        //     // 빈 스마트 포인터 생성
        //     shared_ptr<_Ty> _Ret;
        // 
        //     // this를 활용해 shared_ptr의 내용을 구성
        //     (void) _Ret._Construct_from_weak(*this);
        // 
        //     // 반환
        //     return _Ret;
        // }
        // 
        // --------------------------------------------------
        // 
        // 2. _Construct_from_weak() 호출
        // 
        // template <class _Ty2>
        // bool _Ptr_base<T>::_Construct_from_weak(const weak_ptr<_Ty2>& _Other) noexcept
        // {
        //     // implement shared_ptr's ctor from weak_ptr, and weak_ptr::lock()
        //     // _Incref_nz()는 원자성을 보장하는 함수임.
        //     if (_Other._Rep && _Other._Rep->_Incref_nz())
        //     {
        //         _Ptr = _Other._Ptr;
        //         _Rep = _Other._Rep;
        // 
        //         return true;
        //     }
        // 
        //     // _Incref_nz()가 false를 반환했으면 _Ptr과 _Rep를 갱신하지 않음.
        //     // 대상이 빈 스마트 포인터이기 때문에 이 경우 _Ptr과 _Rep에는 nullptr로 되어 있어야 함.
        //     return false;
        // }
        // 
        // --------------------------------------------------
        // 
        // 3. 컨트롤 블록이 유효하다면 _Incref_nz() 호출
        // 
        // !! 이 코드는 원자적으로 수행됨. !!
        // bool _Ref_count_base::_Incref_nz() noexcept // increment use count if not zero, return true if successful
        // {
        //     // 컴파일러가 해당 _Uses 변수를 최적화하지 않게 강제함(_Volatile_uses는 _Uses를 가리키는 참조 변수일 뿐임).
        //     auto& _Volatile_uses = reinterpret_cast<volatile long&>(_Uses);
        // 
        //     // https://learn.microsoft.com/ko-kr/cpp/intrinsics/arm64-intrinsics?view=msvc-170#IsoVolatileLoadStore
        //     // MSVC 전용 내장 함수로 하드웨어 장치의 메모리를 읽을 때 사용함.
        //     long _Count = __iso_volatile_load32(reinterpret_cast<volatile int*>(&_Volatile_uses));
        //
        //     // !! 주의 !!
        //     // C++에서의 volatile은 최적화만 막기 때문에 가시성과는 무관함.
        //     // 즉, volatile을 적용했다고 해도 CPU의 명령어 재배치 문제는 해결되지 않음.
        // 
        //     // _Count 값이 0이면 _Uses가 0이거나 혹은 경합 도중 0에 도달했다는 뜻임.
        //     // 이 경우에는 false를 반환해야 함.
        //     while (_Count != 0)
        //     {
        //         // CAS 연산
        //         // MS의 Interlocked 계열의 함수는 기본적으로 메모리 배리어를 포함하고 있기에 원자성과 가시성을 모두 보장함.
        //         const long _Old_value = _INTRIN_RELAXED(_InterlockedCompareExchange)(&_Volatile_uses, _Count + 1, _Count);
        // 
        //         // 해당 스레드에서 CAS를 성공적으로 수행했다면 _Old_value와 _Count는 같은 값을 가짐.
        //         if (_Old_value == _Count)
        //         {
        //             return true;
        //         }
        // 
        //         // 경합 과정에서 다른 스레드가 먼저 _Uses의 값을 변경했다면
        //         // 해당 스레드에서 다시 변경 작업을 수행할 수 있게 만들어 줘야 함.
        //         _Count = _Old_value;
        //     }
        // 
        //     return false;
        // }
        // 

        // !! 주의 !!
        // weak_ptr의 lock() 자체는 스레드 안전을 보장한다.
        // 하지만 멀티스레딩 환경에서 스마트 포인터(shared_ptr이나 weak_ptr)를 생성 및 갱신하는 과정이 경합되면 문제가 발생할 수 있다.
        // 
        // --------------------------------------------------
        // 
        // template <class _Ty2>
        // void _Ptr_base<T>::_Weakly_construct_from(const _Ptr_base<_Ty2>& _Other) noexcept // implement weak_ptr's ctors
        // {
        //     // _Rep가 존재하기 때문에 아래 if 문에 들어가 실행됨.
        //     if (_Other._Rep)
        //     {
        //         _Ptr = _Other._Ptr;
        //         // <----- 여기서 _Other의 _Ptr과 _Rep가 갱신되었으면?
        //         _Rep = _Other._Rep;
        // 
        //         // 다음 코드는 원자적으로 수행됨.
        //         _Rep->_Incwref();
        //     }
        //     else
        //     {
        //         _STL_INTERNAL_CHECK(!_Ptr && !_Rep);
        //     }
        // }
        // 
        // --------------------------------------------------
        // 
        // template <class _Ty2>
        // void _Ptr_base<T>::_Move_construct_from(_Ptr_base<_Ty2>&& _Right) noexcept
        // {
        //     // implement shared_ptr's (converting) move ctor and weak_ptr's move ctor
        //     _Ptr = _Right._Ptr;
        //     // <----- 여기서 _Other의 _Ptr과 _Rep가 갱신되었으면?
        //     _Rep = _Right._Rep;
        // 
        //     _Right._Ptr = nullptr;
        //     _Right._Rep = nullptr;
        // }
        // 
        // --------------------------------------------------
        // 
        // 성공적으로 weak_ptr을 생성 혹은 갱신한 상태에서 lock()을 호출하는 건 문제가 되지 않지만
        // 스마트 포인터(여기선 weak_ptr)를 생성 혹은 갱신하고 있는 도중 적용할 대상 스마트 포인터와의 갱신 작업이 경합된다면 문제가 발생할 여지가 생긴다.
        // 
        // weak_ptr을 생성 혹은 갱신할 때는 mutex 등을 활용해 적용 대상이 되는 스마트 포인터에 대한 접근을 막아야 한다.
        // 
        // 이건 lock() 뿐만 아니라 expired()에도 해당하는 문제이다.
        // 이에 대한 자세한 내용은 smart_pointer_multi_threading_issues.cpp를 보도록 한다.
        // 
        // 컨트롤 블록의 해제는 _Weaks가 0으로 떨어지는 단 한 번의 순간에 진행되기 때문에 이 부분은 적어도 스레드 안전하다고 봐도 된다.
        // 스레드 안전하지 않은 건 레퍼런스 카운팅 쪽이 아니라 _Ptr과 _Rep의 갱신이 원자적으로 이루어지지 않는 부분에서 온다.
        //
    }

    cout << "-------------------------#07#-------------------------\n\n";

    /**********************************************************
    *      weak_ptr<T>::expired() -> weak_ptr<T>::lock()      *
    **********************************************************/

    // expired()로 객체의 유효성을 검증하고 이후 lock()으로 객체에 접근하는 로직
    {
        shared_ptr<TestObject> sptr = make_shared<TestObject>(700);
        
        weak_ptr<TestObject> wptr = sptr; // wptr{ sptr };
        
        if (false == wptr.expired())
        {
            cout << "sptr is not expired...\n\n";
        
            if (shared_ptr<TestObject> tempPtr = wptr.lock())
            {
                cout << "wptr.lock() returned a valid shared_ptr...\n\n";
            }
            else // if (nullptr == tempPtr)
            {
                cout << "wptr.lock() returned nullptr...\n\n";
            }
        }
        else // if (true == wptr.expired())
        {
            cout << "sptr is expired...\n\n";
        }
        
        // 1초 뒤 sptr에 nullptr 대입
        thread th = thread([&] {
            this_thread::sleep_for(1s);
        
            sptr = nullptr;
        });
        
        if (false == wptr.expired())
        {
            cout << "sptr is not expired...\n\n";
        
            // 멀티스레딩 환경에서 expired() 체크 이후 lock()을 호출하기도 전에 sptr이 유효하지 않게 되었을 때
            // 어떤 일이 발생하는지 확인하기 위한 코드
            // !! 여기서 바로 nullptr을 대입하는 건 싱글스레딩 환경 같아서 번거롭지만 이렇게 진행함. !!
            th.join();
        
            if (shared_ptr<TestObject> tempPtr = wptr.lock())
            {
                cout << "wptr.lock() returned a valid shared_ptr...\n\n";
            }
            else // if (nullptr == tempPtr)
            {
                cout << "wptr.lock() returned nullptr...\n\n";
            }
        }
        else // if (true == wptr.expired())
        {
            cout << "sptr is expired...\n\n";
        }

        // weak_ptr을 성공적으로 생성 및 갱신했다면 lock() 자체는 스레드 안전을 보장한다.
        // (중요) 하지만 멀티스레딩 환경이라면 expired() 이후 lock()을 거는 사이에 자원이 유효하지 않게 될 가능성이 있다.
        //
        // expired() 이후 lock()을 호출하여 shared_ptr을 받았으면?
        // 1. 싱글스레딩 환경이라면 shared_ptr을 검증하지 않고 바로 사용해도 된다.
        // 2. 멀티스레딩 환경이라면 shared_ptr이 빈 스마트 포인터인지 검증하고 사용해야 한다.
        //
        // 멀티스레딩 환경일 경우 항상 레이스 컨디션을 고려해야 한다.
        //
        // 여기서 말하는 자원이 유효하지 않게 된다는 건 _Uses가 0으로 떨어져 관리 객체가 유효하지 않게 되었단 뜻이지
        // 컨트롤 블록이 유효하지 않게 되었다는 뜻이 아니다(_Weaks가 0으로 떨어지지 않는 이상 컨트롤 블록은 항상 유효함. !!
        // !! 컨트롤 블록이 유효하지 않게 되는 건 _Weaks가 0으로 떨어진 시점인데 이 컨디션에 도달하게 되는 스레드는 오직 하나임. !!
        // !! _Weaks가 0으로 떨어지려면 먼저 _Uses가 0으로 떨어져야 하며, _Uses가 0으로 떨어지는 컨디션에 도달하게 되는 스레드도 오직 하나임. !!
        // 
        // 스레드 안전하지 않은 건 레퍼런스 카운팅 쪽이 아니라 _Ptr과 _Rep의 갱신이 원자적으로 이루어지지 않는 부분에서 온다.
        // 이에 대한 내용은 "weak_ptr<T>::lock()"을 설명한 코드 쪽 설명을 보도록 한다.
        // 
    }

    cout << "-------------------------#08#-------------------------\n\n";

    /**************************************
    *      Up Casting & Down Casting      *
    **************************************/

    // 업 캐스팅과 다운 캐스팅
    {
        shared_ptr<TestObjectEx> sptr1 = make_shared<TestObjectEx>(800, 3.14);
        weak_ptr<TestObjectEx>   wptr1 = sptr1;

        // A) weak_ptr을 업 캐스팅해서 weak_ptr가 받는 것은 가능하다.
        weak_ptr<TestObject> wptr2 = wptr1; // wptr2{ wptr1 };

        // B) shared_ptr을 업 캐스팅해서 weak_ptr가 받는 것도 가능하다.
        weak_ptr<TestObject> wptr3 = sptr1; // wptr3{ sptr1 };

        // weak_ptr의 lock()이 반환하는 건 weak_ptr의 관리 객체 타입을 기반으로 한다.
        auto sptr2 = wptr3.lock();

        cout << "wptr1.lock() returned " << typeid(sptr2).name() << "...\n\n";

        // C) weak_ptr 간 다운 캐스팅을 지원하는 기능은 없기 때문에 lock()을 통해 shared_ptr을 가지고 온 후 적절한 캐스팅 함수를 써야 한다.
        weak_ptr<TestObjectEx> wptr4 = static_pointer_cast<TestObjectEx>(wptr1.lock());

        // --------------------------------------------------
        // 
        // ***** A 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 1. weak_ptr<TestObject> wptr2 = wptr1; // wptr2{ wptr1 };
        // 
        // weak_ptr<TestObjectEx>를 받는 "복사 기반 변환 생성자" 호출
        // 
        // template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        // weak_ptr(const weak_ptr<_Ty2>& _Other) noexcept
        // {
        //     // 타입 추론을 통한 결과를 가져옴.
        //     constexpr bool _Avoid_expired_conversions = _Must_avoid_expired_conversions_from<_Ty2>;
        // 
        //     // !! 컴파일 타임에서 해당 부분은 실행되지 않게 걸러짐. !!
        //     if constexpr (_Avoid_expired_conversions)
        //     {
        //         this->_Weakly_convert_lvalue_avoiding_expired_conversions(_Other);
        //     }
        //     else
        //     {
        //         // !! 이쪽 로직이 실행됨. !! //
        //         this->_Weakly_construct_from(_Other);
        //     }
        // }
        // 
        // // Primary template, the value is used when the substitution fails.
        // template <class _Ty2, class = const _Ty2*>
        // static constexpr bool _Must_avoid_expired_conversions_from = true;
        // 
        // // Template specialization, the value is used when the substitution succeeds.
        // template <class _Ty2>
        // static constexpr bool _Must_avoid_expired_conversions_from<_Ty2, decltype(static_cast<const _Ty2*>(static_cast<_Ty*>(nullptr)))> = false;
        // 
        // !! _Must_avoid_expired_conversions_from<T>는 기본 템플릿과 특수화 템플릿을 적용하여 동작하는 SFINAE를 기반으로 함. !!
        // !! 타입 변환이 가능하거나 상속 관계가 유효하다면 false, 그 외의 경우에는 true를 반환함. !!
        // 
        // weak_ptr의 생성자 쪽 enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0에 의해서
        // 타입 변환이 불가능하거나 상속 관계가 유효하지 않다면 애초에 컴파일이 되지 않는데 왜 이렇게 했는지는 파악하지 못 했다.
        // 
        // --------------------------------------------------
        // 
        // 2. _Weakly_construct_from() 호출
        // 
        // template <class _Ty2>
        // void _Ptr_base<T>::_Weakly_construct_from(const _Ptr_base<_Ty2>& _Other) noexcept // implement weak_ptr's ctors
        // {
        //     // "wptr2 = wptr1"
        //     // wptr1에 있는 내용을 wptr2에 복사하고 _Weaks의 값을 1 증가
        //     if (_Other._Rep)
        //     {
        //         _Ptr = _Other._Ptr; // _Ptr의 타입이 달라도 상속 관계에 따라 업 캐스팅으로 받는 것이 가능
        //         _Rep = _Other._Rep;
        //         _Rep->_Incwref();
        //     }
        //     else
        //     {
        //         _STL_INTERNAL_CHECK(!_Ptr && !_Rep);
        //     }
        // }
        // 
        // --------------------------------------------------
        // 
        // ***** B 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 3. weak_ptr<TestObject> wptr3 = sptr1; // wptr3{ sptr1 };
        // 
        // shared_ptr<TestObjectEx>를 받는 "복사 기반 변환 생성자" 호출
        // 
        // template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        // weak_ptr(const shared_ptr<_Ty2>& _Other) noexcept
        // {
        //     this->_Weakly_construct_from(_Other); // shared_ptr keeps resource alive during conversion
        // }
        // 
        // _Weakly_construct_from()을 호출하는 건 A에 있는 내용과 동일하다.
        // 
        // --------------------------------------------------
        // 
        // ***** C 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 4. weak_ptr<TestObjectEx> wptr4 = static_pointer_cast<TestObjectEx>(wptr1.lock());
        // 
        // !! lock()을 호출해서 shared_ptr로 가져옴. !!
        // _NODISCARD shared_ptr<_Ty> weak_ptr<_Ty>::lock() const noexcept // convert to shared_ptr
        // {
        //     shared_ptr<_Ty> _Ret;
        //     (void) _Ret._Construct_from_weak(*this);
        // 
        //     // 자기 자신(weak_ptr<_Ty>)으로부터 적절한 shared_ptr을 구성하고 반환
        //     return _Ret;
        // }
        // 
        // !! static_pointer_cast<T1, T2>()을 호출하여 캐스팅 진행 !!
        // _EXPORT_STD template <class _Ty1, class _Ty2>
        // _NODISCARD shared_ptr<_Ty1> static_pointer_cast(shared_ptr<_Ty2>&& _Other) noexcept
        // {
        //     // static_cast for shared_ptr that properly respects the reference count control block
        //     // _Other.get()으로 관리 객체의 포인터를 받고 static_cast<T1>()으로 캐스팅함.
        //     const auto _Ptr = static_cast<typename shared_ptr<_Ty1>::element_type*>(_Other.get());
        // 
        //     // 캐스팅한 포인터와 컨트롤 블록이 분리되어 있기 때문에 이를 묶어서 shared_ptr로 반환함.
        //     return shared_ptr<_Ty1>(_STD move(_Other), _Ptr);
        // }
        // 
        // !! 반환된 shared_ptr을 받기 위해 weak_ptr에서 변환 생성자 호출 !!
        // template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        // weak_ptr(const shared_ptr<_Ty2>& _Other) noexcept
        // {
        //     this->_Weakly_construct_from(_Other); // shared_ptr keeps resource alive during conversion
        // }
        // 
        // !! _Weakly_construct_from() 호출 !!
        // template <class _Ty2>
        // void _Ptr_base<T>::_Weakly_construct_from(const _Ptr_base<_Ty2>& _Other) noexcept // implement weak_ptr's ctors
        // {
        //     if (_Other._Rep)
        //     {
        //         _Ptr = _Other._Ptr;
        //         _Rep = _Other._Rep;
        //         _Rep->_Incwref();
        //     }
        //     else
        //     {
        //         _STL_INTERNAL_CHECK(!_Ptr && !_Rep);
        //     }
        // }
        // 
        // weak_ptr<TestObjectEx> wptr4 = static_pointer_cast<TestObjectEx>(wptr1.lock());
        // 
        // 위 코드는 받는 타입과 반환되는 타입이 다르기 때문에 반환 결과가 wptr4에 바로 반영되지 않고 따로 생성자를 호출한다.
        // !! RVO가 적용되는 것이 아니라는 의미임. !!
        //
    }

    cout << "------------------------------------------------------\n\n";

    return 0;
}
