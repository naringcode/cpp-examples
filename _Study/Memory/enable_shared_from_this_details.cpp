// Update Date : 2024-12-27
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <memory>

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
// 6. weak_ptr_details.cpp
//
// # shared_ptr의 관리 객체에서 자신을 반환할 때 필요한 내용
// 7. enable_shared_from_this.cpp
// 8. enable_shared_from_this_details.cpp <-----
// 9. enable_shared_from_this_examples.cpp
//
// # shared_ptr을 멀티스레딩 환경에서 사용할 때 발생할 수 있는 문제점을 기술한 내용
// 10. allocation_aligned_byte_boundaries.cpp(사전지식)
// 11. volatile_atomic_cache_coherence_and_memory_order.cpp(사전지식)
// 12. (중요) shared_ptr_multi_threading_issues.cpp

/***************************
*      Object Classes      *
***************************/

class FooObjectA : public enable_shared_from_this<FooObjectA>
{
public:
    FooObjectA()  { cout << "FooObjectA()\n"; }
    ~FooObjectA() { cout << "~FooObjectA()\n"; }

public:
    shared_ptr<FooObjectA> GetSharedPtr()
    {
        // shared_ptr가 적용된 관리 객체가 클래스 내부에서 자기 자신을 this 형식으로 어딘가에 반환해야 하는 상황이 발생한다면,
        // enable_shared_from_this<T>를 상속하고, shared_from_this()를 쓰는 방식을 택해야 한다.
        // !! 베이스 클래스가 있는 경우 다중 상속을 사용해야 함. !!
        return this->shared_from_this();

        // 이런 식으로 쓰면 안 된다.
        // return shared_ptr<FooObjectA>(this);

        // shared_ptr로 관리되고 있는 객체일 경우 this를 가지고 또 다른 shared_ptr를 생성해선 안 된다.
        // shared_ptr<T>(this); // 잘못된 코드
        //
        // 이건 새로운 shared_ptr을 생성하는 것이기 때문에 자칫 잘못하면 동일 포인터를 대상으로 하는 여러 개의 shared_ptr이 생길 수 있다.
        // 1. 본래의 shared_ptr이 있는데 이걸 shared_ptr<T>(this)로 새로운 스마트 포인터 생성함.
        // 2. 여러 개의 새로운 shared_ptr이 생성됨(동일 관리 객체, 다른 컨트롤 블록).
        // 3. shared_ptr<T>(this)를 한 횟수만큼 관리 객체의 소멸 과정이 추가적으로 유도됨.
        // 
        // (주의) this 기반으로 새로운 shared_ptr를 생성하게 되면 동일 관리 객체를 대상으로 다른 컨트롤 블록이 생성된다.
        // 
        // enable_shared_from_this<T>를 적용하고 shared_from_this()를 쓰면
        // 본래 shared_ptr의 관리 객체와 컨트롤 블록을 연계한 shared_ptr가 생성된다.
    }

    weak_ptr<FooObjectA> GetWeakPtr()
    {
        // weak_ptr도 마찬가지로 관리 객체 차원에서 this로 구성하여 넘겨야 하는 상황이 발생하면
        // this로 weak_ptr를 구성하는 것이 아닌 weak_from_this()로 반환해야 한다.
        // weak_ptr의 경우에는 weak_ptr<T>(this)를 받는 생성자 자체가 없다.
        return this->weak_from_this();
    }

public:
    int val = 0x1234;
};

class FooObjectB // : public enable_shared_from_this<FooObjectB>
{
public:
    FooObjectB() { cout << "FooObjectB()\n"; }
    ~FooObjectB() { cout << "~FooObjectB()\n"; }

public:
    // shared_ptr<FooObjectB> GetSharedPtr()
    // {
    //     return this->shared_from_this();
    // }

    // weak_ptr<FooObjectB> GetWeakPtr()
    // {
    //     return this->weak_from_this();
    // }

public:
    int val = 0x5678;
};

/*****************
*      Main      *
*****************/

int main()
{
    // !! 중단점을 걸고 레퍼런스 카운팅을 확인 및 분석을 진행할 것. !!

    // auto typeStr      = typeid(_Can_enable_shared<FooObjectA>::type).name();
    // auto valueStr     = typeid(_Can_enable_shared<FooObjectA>::value).name();
    // auto valueTypeStr = typeid(_Can_enable_shared<FooObjectA>::value_type).name();

    // enable_shared_from_this<T>를 사용
    shared_ptr<FooObjectA> fooASharedPtr = make_shared<FooObjectA>();

    shared_ptr<FooObjectA> fooASharedFromThisPtr = fooASharedPtr->GetSharedPtr();
    weak_ptr<FooObjectA>   fooAWeakFromThis      = fooASharedPtr->GetWeakPtr();
    
    // --------------------------------------------------
    
    // enable_shared_from_this<T>를 미사용
    shared_ptr<FooObjectB> fooBSharedPtr = make_shared<FooObjectB>();
    
    shared_ptr<FooObjectB> fooBSharedPtrPlain = fooBSharedPtr;
    weak_ptr<FooObjectB>   fooBWeakPtrPlain   = fooBSharedPtr;

    // --------------------------------------------------

    // !! Visual Studio 2022 기준 !!

    // 스마트 포인터(shared_ptr, weak_ptr)는 _Ptr_base를 상속받는 형태로 구현되어 있으며 _Ptr_base는 다음 두 변수를 가진다.
    // - element_type*    _Ptr{ nullptr }; // 관리 객체의 포인터
    // - _Ref_count_base* _Rep{ nullptr }; // 컨트롤 블록의 포인터
    // 
    // _Ref_count_base는 컨트롤 블록의 최상위 부모 클래스이며 레퍼런스 카운팅을 진행하기 위한 변수가 존재한다.
    // - _Atomic_counter_t _Uses  = 1; // 강한 참조(주로 shared_ptr에서 사용)
    // - _Atomic_counter_t _Weaks = 1; // 약한 참조(주로 weak_ptr에서 사용)
    // 
    // --------------------------------------------------
    // 
    // enable_shared_from_this<T>는 스마트 포인터가 아니다.
    // 해당 클래스는 스마트 포인터의 기능을 개방하여 자기참조 문제를 해결하기 위한 인터페이스에 가깝다.
    // 
    // _EXPORT_STD template <class _Ty>
    // class enable_shared_from_this // provide member functions that create shared_ptr to this
    // {
    //     ...
    //     mutable weak_ptr<_Ty> _Wptr;
    // };
    // 
    // 클래스 내부를 보면 "weak_ptr<_Ty> _Wptr"을 들고 있는 게 전부이다.
    //

    // shared_ptr을 최초로 생성하는 코드를 보면 _Set_ptr_rep_and_enable_shared()를 거치게 되어 있다.
    // 
    // --------------------------------------------------
    // 
    // A) make_shared<T>()의 경우
    // 
    // _NODISCARD_SMART_PTR_ALLOC shared_ptr<_Ty> make_shared(_Types&&... _Args) // make a shared_ptr to non-array object
    // {
    //     const auto _Rx = new _Ref_count_obj2<_Ty>(_STD forward<_Types>(_Args)...);
    //     shared_ptr<_Ty> _Ret;
    //     _Ret._Set_ptr_rep_and_enable_shared(_STD addressof(_Rx->_Storage._Value), _Rx); // <-----
    //     return _Ret;
    // }
    // 
    // --------------------------------------------------
    // 
    // B) Ptr를 받는 생성자의 경우
    // 
    // template <class _Ux,
    //     enable_if_t<
    //         conjunction_v<
    //             conditional_t<is_array_v<_Ty>, _Can_array_delete<_Ux>, _Can_scalar_delete<_Ux>>,
    //             _SP_convertible<_Ux, _Ty>
    //         >,
    //         int
    //     > = 0>
    // explicit shared_ptr(_Ux* _Px) // construct shared_ptr object that owns _Px
    // {
    //     if constexpr (is_array_v<_Ty>)
    //     {
    //         _Setpd(_Px, default_delete<_Ux[]>{});
    //     }
    //     else
    //     {
    //         _Temporary_owner<_Ux> _Owner(_Px);
    //         _Set_ptr_rep_and_enable_shared(_Owner._Ptr, new _Ref_count<_Ux>(_Owner._Ptr)); // <-----
    //         _Owner._Ptr = nullptr;
    //     }
    // }
    // 
    // --------------------------------------------------
    // 
    // C) Ptr와 Deleter 받는 생성자의 경우
    // 
    // template <class _Ux, class _Dx,
    //     enable_if_t<
    //         conjunction_v<
    //             is_move_constructible<_Dx>, _Can_call_function_object<_Dx&, _Ux*&>,
    //             _SP_convertible<_Ux, _Ty>
    //         >,
    //         int> = 0>
    // shared_ptr(_Ux* _Px, _Dx _Dt) // construct with _Px, deleter
    // {
    //     _Setpd(_Px, _STD move(_Dt));
    // }
    // 
    // template <class _UxptrOrNullptr, class _Dx>
    // void shared_ptr<T>::_Setpd(const _UxptrOrNullptr _Px, _Dx _Dt) // take ownership of _Px, deleter _Dt
    // {
    //     _Temporary_owner_del<_UxptrOrNullptr, _Dx> _Owner(_Px, _Dt);
    // 
    //     _Set_ptr_rep_and_enable_shared( // <-----
    //         _Owner._Ptr, 
    //         new _Ref_count_resource<_UxptrOrNullptr, _Dx>(_Owner._Ptr, _STD move(_Dt)));
    // 
    //     _Owner._Call_deleter = false;
    // }
    // 
    // --------------------------------------------------
    // 
    // D) Allocator를 받는 생성자의 경우
    // 
    // template <class _Ty, class _Alloc, class... _Types>
    // enable_if_t<!is_array_v<_Ty>, shared_ptr<_Ty>> allocate_shared(const _Alloc& _Al, _Types&&... _Args) // make a shared_ptr to non-array object
    // {
    //     // Note: As of 2019-05-28, this implements the proposed resolution of LWG-3210 (which controls whether
    //     // allocator::construct sees T or const T when _Ty is const qualified)
    //     using _Refoa   = _Ref_count_obj_alloc3<remove_cv_t<_Ty>, _Alloc>;
    //     using _Alblock = _Rebind_alloc_t<_Alloc, _Refoa>;
    //     
    //     _Alblock _Rebound(_Al);
    //     _Alloc_construct_ptr<_Alblock> _Constructor{_Rebound};
    //     
    //     _Constructor._Allocate();
    //     _Construct_in_place(*_Constructor._Ptr, _Al, _STD forward<_Types>(_Args)...);
    //     
    //     shared_ptr<_Ty> _Ret;
    //     const auto _Ptr = reinterpret_cast<_Ty*>(_STD addressof(_Constructor._Ptr->_Storage._Value));
    //     _Ret._Set_ptr_rep_and_enable_shared(_Ptr, _Unfancy(_Constructor._Release())); // <-----
    //     
    //     return _Ret;
    // }
    // 
    // --------------------------------------------------
    // 
    // E) Ptr, Deleter, Allocator를 받는 생성자의 경우
    // 
    // template <class _Ux, class _Dx, class _Alloc,
    //     enable_if_t<
    //         conjunction_v<is_move_constructible<_Dx>, _Can_call_function_object<_Dx&, _Ux*&>,
    //         _SP_convertible<_Ux, _Ty>>,
    //     int> = 0>
    // shared_ptr(_Ux* _Px, _Dx _Dt, _Alloc _Ax) // construct with _Px, deleter, allocator
    // {
    //     _Setpda(_Px, _STD move(_Dt), _Ax);
    // }
    // 
    // template <class _UxptrOrNullptr, class _Dx, class _Alloc>
    // void shared_ptr<T>::_Setpda(const _UxptrOrNullptr _Px, _Dx _Dt, _Alloc _Ax) // take ownership of _Px, deleter _Dt, allocator _Ax
    // {
    //     using _Alref_alloc = _Rebind_alloc_t<_Alloc, _Ref_count_resource_alloc<_UxptrOrNullptr, _Dx, _Alloc>>;
    // 
    //     _Temporary_owner_del<_UxptrOrNullptr, _Dx> _Owner(_Px, _Dt);
    // 
    //     _Alref_alloc _Alref(_Ax);
    //     _Alloc_construct_ptr<_Alref_alloc> _Constructor(_Alref);
    // 
    //     _Constructor._Allocate();
    //     _Construct_in_place(*_Constructor._Ptr, _Owner._Ptr, _STD move(_Dt), _Ax);
    // 
    //     _Set_ptr_rep_and_enable_shared(_Owner._Ptr, _Unfancy(_Constructor._Ptr)); // <-----
    // 
    //     _Constructor._Ptr    = nullptr;
    //     _Owner._Call_deleter = false;
    // }
    // 
    // --------------------------------------------------
    // 
    // * _Set_ptr_rep_and_enable_shared()는 다음과 같이 구성되어 있다.
    // 
    // template <class _Ux>
    // void shared_ptr<_Ty>::_Set_ptr_rep_and_enable_shared(_Ux* const _Px, _Ref_count_base* const _Rx) noexcept // take ownership of _Px
    // {
    //     this->_Ptr = _Px; // 관리 객체
    //     this->_Rep = _Rx; // 컨트롤 블록
    // 
    //     // enable_shared_from_this<T>를 상속받은 상태라면 아래 if 문에 들어가 관리 객체의 _Wptr을 설정할 것임.
    //     // !! conjunction_v<...>는 템플릿 구조체를 활용하여 구성되며 컴파일 타임에 추론됨. !!
    //     if constexpr (conjunction_v<negation<is_array<_Ty>>, negation<is_volatile<_Ux>>, _Can_enable_shared<_Ux>>)
    //     {
    //         if (_Px && _Px->_Wptr.expired())
    //         {
    //             // _Wptr은 enable_shared_from_this<T>의 멤버 변수
    //             // 관리 객체가 enable_shared_from_this<T>를 상속받은 형태이기 때문에 관리 객체는 _Wptr을 가지고 있음.
    //             _Px->_Wptr = shared_ptr<remove_cv_t<_Ux>>(*this, const_cast<remove_cv_t<_Ux>*>(_Px));
    //         }
    //     }
    // }
    // 
    // (중요) "if constexpr 문"으로 되어 있기 때문에 컴파일 타임에 계산했을 시 조건이 맞다면 코드를 생성하고 아닐 경우 코드를 생성하지 않는다.
    // 
    // _Px->_Wptr = shared_ptr<remove_cv_t<_Ux>>(*this, const_cast<remove_cv_t<_Ux>*>(_Px));
    // 
    // 위 과정을 거치면 _Weaks의 값이 1 증가하며 _Wptr는 생성된 shared_ptr와 동일한 관리 객체와 컨트롤 블록을 가지게 된다.
    // 좀 이상하긴 해도 자기 자신의 포인터를 관리 객체로 하여 weak_ptr로 들고 있는 형태가 된다.
    // !! 생성한 shared_ptr와 이후 설정한 _Wptr은 동일한 관리 객체와 컨트롤 블록을 가짐. !!
    // 
    // "if constexpr 문"의 조건으로 들어간 템플릿 요소를 보도록 하자.
    // - conjunction_v<Traits...> : conjunction<Traits...>::value로 되어 있으며 여러 조건이 true일 경우 true를 반환하는 논리 AND 연산임.
    // - negation<T> : bool_constant<!bool(T::value)>로 되어 있는 논리 부정 연산임.
    // - is_array<T> : T가 배열일 경우 true, 아닐 경우 false인 value를 반환하는 템플릿 구조체임.
    // - is_volatile<T> : T가 volatile 또는 const volatile로 되어 있을 경우 true, 그 외의 경우 false를 반환하는 템플릿 구조체임.
    // - _Can_enable_shared<T> : T가 enable_shared_from_this<T>를 상속받은 것인지 확인하기 위한 템플릿 구조체임.
    // 
    // // 기본 템플릿
    // template <class _Yty, class = void>
    // struct _Can_enable_shared : false_type { }; // detect unambiguous and accessible inheritance from enable_shared_from_this
    // 
    // // 특수화 템플릿
    // template <class _Yty>
    // struct _Can_enable_shared<_Yty, void_t<typename _Yty::_Esft_type>>
    //     : is_convertible<remove_cv_t<_Yty>*, typename _Yty::_Esft_type*>::type
    // {
    //     // is_convertible is necessary to verify unambiguous inheritance
    // };
    // 
    // https://en.cppreference.com/w/cpp/types/remove_cv
    // remove_cv_t<T>는 const와 volatile를 제거한 타입을 받기 위한 템플릿 구조체이다.
    // 
    // enable_shared_from_this<T>를 상속하면 _Esft_type이 내부에 정의된다.
    // 
    // _EXPORT_STD template <class _Ty>
    // class enable_shared_from_this { // provide member functions that create shared_ptr to this
    // public:
    //     using _Esft_type = enable_shared_from_this;
    //     ...
    // };
    // 
    // _Can_enable_shared<T>는 T가 _Esft_type을 가지고 있는지 판단하기 위한 템플릿 구조체이며,
    // _Esft_type의 존재 여부를 판단했으면 그 다음에는 is_convertible<From, To>를 추론한다.
    // 
    // !! is_convertible은 bool_constant를 상속받고, bool_constant는 integral_constant를 상속받음. !!
    // _EXPORT_STD template <class _From, class _To>
    // struct is_convertible : bool_constant<__is_convertible_to(_From, _To)> {
    //     // determine whether _From is convertible to _To
    // };
    // 
    // _EXPORT_STD template <bool _Val>
    // using bool_constant = integral_constant<bool, _Val>;
    // 
    // _EXPORT_STD template <class _Ty, _Ty _Val>
    // struct integral_constant
    // {
    //     static constexpr _Ty value = _Val;
    // 
    //     using value_type = _Ty;
    //     using type       = integral_constant;
    // 
    //     constexpr operator value_type() const noexcept {
    //         return value;
    //     }
    // 
    //     _NODISCARD constexpr value_type operator()() const noexcept {
    //         return value;
    //     }
    // };
    // 
    // is_convertible<From, To>이 상속하는 대상은 bool_constant<true>이거나 bool_constant<false>이다.
    // 따라서 is_convertible<From, To>의 type은 bool_constant<true> 혹은 bool_constant<false>이고,
    // is_convertible<From, To>의 value는 true나 false가 된다.
    // 
    // 그리고 이러한 is_convertible<From, To>를 _Can_enable_shared<T>가 상속하는 형태로 되어 있다.
    // 
    // if constexpr (conjunction_v<negation<is_array<_Ty>>, negation<is_volatile<_Ux>>, _Can_enable_shared<_Ux>>)
    // {
    //     ...
    // }
    //   
    // 그럼 생각을 정리하고 위 "if constexpr 문"을 다시 보도록 하자.
    //
    
    // template <class _Ux>
    // void shared_ptr<_Ty>::_Set_ptr_rep_and_enable_shared(_Ux* const _Px, _Ref_count_base* const _Rx) noexcept // take ownership of _Px
    // {
    //     this->_Ptr = _Px; // 관리 객체
    //     this->_Rep = _Rx; // 컨트롤 블록
    // 
    //     // enable_shared_from_this<T>를 상속받은 상태라면 아래 if 문에 들어가 관리 객체의 _Wptr을 설정할 것임.
    //     // !! conjunction_v<...>는 템플릿 구조체를 활용하여 구성되며 컴파일 타임에 추론됨. !!
    //     if constexpr (conjunction_v<negation<is_array<_Ty>>, negation<is_volatile<_Ux>>, _Can_enable_shared<_Ux>>)
    //     {
    //         if (_Px && _Px->_Wptr.expired())
    //         {
    //             // _Wptr은 enable_shared_from_this<T>의 멤버 변수
    //             // 관리 객체가 enable_shared_from_this<T>를 상속받은 형태이기 때문에 관리 객체는 _Wptr을 가지고 있음.
    //             _Px->_Wptr = shared_ptr<remove_cv_t<_Ux>>(*this, const_cast<remove_cv_t<_Ux>*>(_Px));
    //         }
    //     }
    // }
    // 
    // if constexpr가 컴파일 타임에 생성한 코드를 보도록 하자.
    // 
    // if (_Px && _Px->_Wptr.expired()) { ... }
    // 
    // if 문 내에 있는 코드가 수행되기 위해선 관리 객체가 존재해야 하고 _Wptr이 유효하지 않은(_Uses가 0) 상태여야 한다.
    // 
    // _Px->_Wptr = shared_ptr<remove_cv_t<_Ux>>(*this, const_cast<remove_cv_t<_Ux>*>(_Px));
    // 
    // 그럼 위 코드가 수행되는 과정을 보도록 하자.
    // 
    // --------------------------------------------------
    // 
    // 1. shared_ptr<T2>와 관리 객체를 받아 임시 객체를 생성하기 위한 shared_ptr<T1>의 생성자 호출
    // 
    // 여기서 T1과 T2는 같은 타입이다.
    // 
    // template <class _Ty2>
    // shared_ptr(const shared_ptr<_Ty2>& _Right, element_type* _Px) noexcept
    // {
    //     // construct shared_ptr object that aliases _Right
    //     this->_Alias_construct_from(_Right, _Px);
    // }
    // 
    // --------------------------------------------------
    // 
    // 2._Alias_construct_from() 호출
    // 
    // template <class _Ty2>
    // void _Ptr_base<T1>::_Alias_construct_from(const shared_ptr<_Ty2>& _Other, element_type* _Px) noexcept
    // {
    //     // implement shared_ptr's aliasing ctor
    //     _Other._Incref();
    // 
    //     _Ptr = _Px;         // 스마트 포인터로 관리할 관리 객체를 그대로 적용
    //     _Rep = _Other._Rep; // 스마트 포인터로 관리할 관리 객체를 그대로 적용(*this로 넘긴 대상임)
    // }
    // 
    // --------------------------------------------------
    // 
    // 3. _Px->_Wptr에 적용하기 위한 "복사 기반 변환 대입 연산자"를 호출
    // 
    // template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
    // weak_ptr& operator=(const shared_ptr<_Ty2>& _Right) noexcept
    // {
    //     weak_ptr(_Right).swap(*this);
    //     return *this;
    // }
    // 
    // !! 임시 객체 weak_ptr(_Right)를 생성하기 위한 변환 생성자 호출 !!
    // template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
    // weak_ptr(const shared_ptr<_Ty2>& _Other) noexcept
    // {
    //     this->_Weakly_construct_from(_Other); // shared_ptr keeps resource alive during conversion
    // }
    // 
    // !! _Weakly_construct_from() 호출 !!
    // template <class _Ty2>
    // void _Ptr_base<T1>::_Weakly_construct_from(const _Ptr_base<_Ty2>& _Other) noexcept // implement weak_ptr's ctors
    // {
    //     // 1번과 2번 과정에서 생성한 임시 객체 shared_ptr<remove_cv_t<_Ux>>(*this, const_cast<remove_cv_t<_Ux>*>(_Px))를
    //     // 대입 연산자에 있는 임시 객체 weak_ptr(_Right)에 반영
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
    // !! 임시 객체 weak_ptr(_Right)를 _Px->_Wptr에 반영하기 위한 swap() 호출 !!
    // void weak_ptr<T>::swap(weak_ptr& _Other) noexcept
    // {
    //     this->_Swap(_Other);
    // }
    // 
    // !! 최종적으로 _Px->_Wptr에 반영 !!
    // void _Ptr_base<T>::_Swap(_Ptr_base& _Right) noexcept // swap pointers
    // {
    //     _STD swap(_Ptr, _Right._Ptr);
    //     _STD swap(_Rep, _Right._Rep);
    // }
    //

    // 로직이 다소 혼란스럽기에 복습 차원에서 한 번 더 정리한다.
    // 
    // enable_shared_from_this<T>는 스마트 포인터가 아니라 내부에 weak_ptr<T>를 가지는 클래스이다.
    // enable_shared_from_this<T>는 객체를 확장하는 개념이라기보단 특정 기능을 개방하는 느낌에 가깝다.
    // 
    // 일반적으로 객체를 shared_ptr로 생성하면 컨트롤 블록의 레퍼런스 카운팅은 [1 strong ref, 1 weak ref]이지만,
    // enable_shared_from_this<T>를 적용한 객체는 내부에서 _Wptr을 대상으로 복사하는 과정을 거치기 때문에 해당 카운팅은 [1 strong ref, 2 weak refs]가 된다.
    // 
    // 이해가 되지 않는다면 _Set_ptr_rep_and_enable_shared()의 동작 방식을 설명한 코드를 다시 보도록 하자.
    // 
    // 소멸자 호출 과정 중 레퍼런스 카운팅이 제대로 수행되어 관리 객체의 소멸 과정 이후 컨트롤 블록의 해제까지 제대로 이루어지는지 확인해야 한다.
    // 
    // --------------------------------------------------
    // 
    // 1. shared_ptr의 소멸자 호출
    // 
    // ~shared_ptr() noexcept // release resource
    // {
    //     this->_Decref(); // _Ptr_base에 있는 함수
    // }
    // 
    // --------------------------------------------------
    // 
    // 2. _Ptr_base<T>의 _Decref()를 거쳐 컨트롤 블록의 _Decref() 호출
    // 
    // void _Ptr_base<T>::_Decref() noexcept // decrement reference count
    // {
    //     if (_Rep)
    //     {
    //         _Rep->_Decref();
    //     }
    // }
    // 
    // !! _Uses의 값이 0에 도달하여 _Destroy()와 _Decwref()를 차례대로 호출한다고 가정 !!
    // void _Ref_count_base::_Decref() noexcept // decrement use count
    // {
    //     if (_MT_DECR(_Uses) == 0)
    //     {
    //         _Destroy();
    //         _Decwref();
    //     }
    // }
    // 
    // --------------------------------------------------
    // 
    // 3. 컨트롤 블록의 _Destroy() 호출
    // 
    // _Destroy()는 최상위 컨트롤 블록의 순수 가상 함수이기 때문에 이를 상속하여 구현한 쪽에 의존적이다.
    // 
    // !! 여기서는 make_shared<T>()의 컨트롤 블록인 _Ref_count_obj2<T>를 기반으로 함. !!
    // void _Ref_count_obj2<T>::_Destroy() noexcept override // destroy managed resource
    // {
    //     _Destroy_in_place(_Storage._Value);
    // }
    // 
    // template <class _Ty>
    // _CONSTEXPR20 void _Destroy_in_place(_Ty& _Obj) noexcept
    // {
    //     if constexpr (is_array_v<_Ty>)
    //     {
    //         _Destroy_range(_Obj, _Obj + extent_v<_Ty>);
    //     }
    //     else
    //     {
    //         // 이쪽 로직이 수행됨.
    //         _Obj.~_Ty();
    //     }
    // }
    // 
    // (* 중요 *) _Destroy()를 호출하는 과정에서 관리 객체의 소멸자를 호출하는데 이때 _Wptr의 소멸자가 호출된다.
    // 
    // --------------------------------------------------
    // 
    // 4. enable_shared_from_this<T>의 멤버 변수 _Wptr의 소멸자 호출
    // 
    // ~weak_ptr() noexcept
    // {
    //     this->_Decwref();
    // }
    // 
    // void _Ptr_base<T>::_Decwref() noexcept // decrement weak reference count
    // {
    //     if (_Rep)
    //     {
    //         _Rep->_Decwref();
    //     }
    // }
    // 
    // void _Ref_count_base::_Decwref() noexcept // decrement weak reference count
    // {
    //     // _Weaks의 값이 하나 줄었으나 해당 값이 0에 도달한 것은 아니기 때문에 _Delete_this()를 호출하진 않음.
    //     if (_MT_DECR(_Weaks) == 0)
    //     {
    //         _Delete_this();
    //     }
    // }
    // 
    // --------------------------------------------------
    // 
    // 5. 컨트롤 블록의 _Decref()에 돌아와 _Decwref()를 이어서 호출
    // 
    // !! _Uses의 값이 0에 도달하여 _Destroy()와 _Decwref()를 차례대로 호출한다고 가정 !!
    // void _Ref_count_base::_Decref() noexcept // decrement use count
    // {
    //     if (_MT_DECR(_Uses) == 0)
    //     {
    //         _Destroy();
    //         _Decwref();
    //     }
    // }
    // 
    // void _Ref_count_base::_Decwref() noexcept // decrement weak reference count
    // {
    //     // _Weaks의 값을 하나 줄이면 비로소 해당 값이 0에 도달하기 때문에 _Delete_this()를 호출함.
    //     if (_MT_DECR(_Weaks) == 0)
    //     {
    //         // _Delete_this() 또한 최상위 컨트롤 블록의 순수 가상 함수이기 때문에 이를 상속하여 구현한 쪽에 의존적임.
    //         _Delete_this();
    //     }
    // }
    // 
    // !! 여기서는 make_shared<T>()의 컨트롤 블록인 _Ref_count_obj2<T>를 기반으로 함. !!
    // void _Ref_count_obj2<T>::_Delete_this() noexcept override // destroy self
    // {
    //     delete this;
    // }
    //

    // shared_ptr이 생성되는 순간 기본적으로 컨트롤 블록에 설정되는 레퍼런스 카운팅 값은 [1 strong ref, 1 weak ref]이다.
    // shared_ptr의 소멸자 로직에 따르면 강한 참조(storng refs) 값을 하나 줄이고 이 값이 0에 도달했을 때 약한 참조(weak refs) 값을 줄인다.
    // 
    // 관리 객체에 enable_shared_from_this<T>가 적용된 경우 최초 스마트 포인터를 생성했을 시 레퍼런스 카운팅 값은
    // _Wptr로 복사하는 과정에 의해 [1 strong ref, 2 weak refs]가 된다.
    // 
    // enable_shared_from_this<T>를 적용한 shared_ptr의 소멸자 호출 과정을 보면 [0 strong ref, 1 weak ref]가 되어 누수가 날 것 같지만
    // 강한 참조 값인 _Uses가 0에 도달하여 관리 객체의 소멸 과정을 거쳤을 때 enable_shared_from_this<T>의 멤버 변수인 weak_ptr의 소멸자도 같이 호출된다.
    // 
    // 이런 이유로 최종적으로 레퍼런스 카운팅이 [0 strong ref, 0 weak ref]에 도달하여 정상적으로 컨트롤 블록을 해제할 수 있는 것이다.
    //
    // --------------------------------------------------
    // 
    // enable_shared_from_this<T>를 적용한 스마트 포인터를 단독으로 사용하고 이를 Inspector로 관찰했을 때
    // [1 strong ref, 2 weak refs]가 아닌 [1 strong ref, 1 weak ref]로 나올 것이다.
    // 
    // 하지만 조사식을 통해 sptr._Rep->_Uses와 sptr._Rep->_Weaks를 관찰하면 각각 1과 2가 나오는 것을 볼 수 있다.
    // 
    // 값 뿐만 아닌 sptr._Rep를 메모리로 조회하면 "01 00 00 00 02 00 00 00"로 되어 있는 것도 확인할 수 있다.

    return 0;
}
