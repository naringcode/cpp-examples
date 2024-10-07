#include <iostream>
#include <memory>

// 동일한 내용이 _Study/enable_shared_from_this_with_refCnt.cpp에 있음.
// 일종의 북마크 용도...

using namespace std;

class TestClassA : public enable_shared_from_this<TestClassA>
{
public:
    TestClassA()  { cout << "TestClassA()\n"; }
    ~TestClassA() { cout << "~TestClassA()\n"; }

public:
    shared_ptr<TestClassA> GetSharedPtr()
    {
        // this를 넣어야 하는 상황이 발생하면 enable_shared_from_this<T>를 상속하고,
        // shared_from_this()를 쓰는 방식을 택할 것!
        return this->shared_from_this();

        // shared_ptr을 적용한 객체라면 this를 가지고 새로운 shared_ptr를 생성하는 방식을 쓰면 안 된다.
        // shared_ptr<T>(this);
        // 
        // 이건 새로운 shared_ptr을 생성하는 것이라 자칫 잘못하면 동일 포인터를 대상으로 하는 shared_ptr이 2개 이상 생길 수 있다.
        // 1. 본래 shared_ptr
        // 2. 새로운 shared_ptr x n
        //
        // shared_from_this()를 쓰면 본래 shared_ptr의 오브젝트와 레퍼런스 카운팅 블록을 연계한 shared_ptr을 생성한다.
    }

public:
    int val = 100;
};

class TestClassB // : public enable_shared_from_this<TestClassB>
{
public:
    TestClassB() { cout << "TestClassB()\n"; }
    ~TestClassB() { cout << "~TestClassB()\n"; }

public:
    // shared_ptr<TestClassB> GetSharedPtr()
    // {
    //     // this를 넣어야 하는 상황이 발생하면 enable_shared_from_this<T>를 상속하고,
    //     // shared_from_this()를 쓰는 방식을 택할 것!
    //     return this->shared_from_this();
    // }

public:
    int val = 200;
};

int main()
{
    // 중단점을 걸고 레퍼런스 카운팅을 확인 및 분석을 진행할 것.

    // enable_shared_from_this<T>를 사용
    shared_ptr<TestClassA> testShared1 = make_shared<TestClassA>();
    weak_ptr<TestClassA>   testWeak1   = testShared1;

    shared_ptr<TestClassA> testShared2 = testShared1->GetSharedPtr();

    // --------------------------------------------------

    // 그냥 사용
    shared_ptr<TestClassB> testShared3 = make_shared<TestClassB>();
    shared_ptr<TestClassB> testShared4 = testShared3;

    weak_ptr<TestClassB> testWeak2 = testShared3;

    // --------------------------------------------------

    // !! Visual Studio 2022 기준 !!

    // shared_ptr과 weak_ptr은 _Ptr_base을 상속받는 형태로 구현되어 있으며 다음 두 변수를 가진다.
    // - element_type*    _Ptr{ nullptr }; // 할당된 오브젝트
    // - _Ref_count_base* _Rep{ nullptr }; // 레퍼런스 카운팅 블록
    // 
    // _Ref_count_base는 내부에 shared_ptr 카운터와 weak_ptr 카운터를 가지고 있다.
    // - _Atomic_counter_t _Uses  = 1; // shared_ptr 카운터
    // - _Atomic_counter_t _Weaks = 1; // weak_ptr 카운터
    // 
    // enable_shared_from_this<T>는 스마트 포인터가 아니다.
    // 단순히 내부에 "weak_ptr<_Ty> _Wptr;"을 가지는 클래스일 뿐이다.
    //
    // make_shared()를 호출하면 내부에서 _Set_ptr_rep_and_enable_shared()를 호출한다.
    // 
    // _NODISCARD_SMART_PTR_ALLOC shared_ptr<_Ty> make_shared(_Types&&... _Args) { // make a shared_ptr to non-array object
    //     const auto _Rx = new _Ref_count_obj2<_Ty>(_STD forward<_Types>(_Args)...);
    //     shared_ptr<_Ty> _Ret;
    //     _Ret._Set_ptr_rep_and_enable_shared(_STD addressof(_Rx->_Storage._Value), _Rx);
    //     return _Ret;
    // }
    // 
    // template <class _Ux>
    // void _Set_ptr_rep_and_enable_shared(_Ux* const _Px, _Ref_count_base* const _Rx) noexcept { // take ownership of _Px
    //     this->_Ptr = _Px;
    //     this->_Rep = _Rx;
    //     if constexpr (conjunction_v<negation<is_array<_Ty>>, negation<is_volatile<_Ux>>, _Can_enable_shared<_Ux>>) {
    //         if (_Px && _Px->_Wptr.expired()) {
    //             _Px->_Wptr = shared_ptr<remove_cv_t<_Ux>>(*this, const_cast<remove_cv_t<_Ux>*>(_Px));
    //         }
    //     }
    // }
    // 
    // 잘 보면 "if constexpr"를 통해 컴파일 시간에 _Wptr을 세팅하는 코드가 있는 것을 볼 수 있다.
    // !! enable_shared_from_this<T>를 상속했다면 해당 if 문에 들어오고 아니라면 들어오지 않는다. !!
    // 
    // _Px는 카운터 블록이 아닌 할당된 오브젝트를 말한다.
    // 따라서 "_Px->_Wptr = ..." 구문은 달리 봐야 한다.
    // 
    // 할당된 오브젝트라고 해도 enable_shared_from_this<T>는 내부에 _Wptr을 멤버로 보유하고 있다.
    // 따라서 make_shared()를 진행함과 동시에 _Wptr에 "레퍼런스 카운팅 블록"을 넘기는 것이 가능하다.
    // 
    // !! make_shared()를 통해 생성한 레퍼런스 카운팅 블록과 _Wptr의 레퍼런스 카운팅 블록은 동일하다. !!
    //

    // 혼동되는 사항이 생겨서 복습 차원에서 한 번 더 정리함.
    //
    // enable_shared_from_this<T>는 스마트 포인터가 아니라 내부에 weak_ptr<T>를 가지는 클래스임.
    // enable_shared_from_this<T>는 객체를 확장하는 개념이라기보단 특정 기능을 개방하는 느낌이라고 봐야 함.
    //
    // 일반적인 객체를 shared_ptr로 생성하면 레퍼런스 카운팅 블록은 [1 strong refs, 1 weak ref]이지만,
    // enable_shared_from_this<T>을 적용한 객체는 내부의 _Wptr에 레퍼런스 카운팅 블록을 복사하는 과정을 거쳐서 [1 strong refs, 2 weak ref]가 됨.
    //
    // 어째서 _Wptr에 데이터가 복사될 수 있는지는 _Set_ptr_rep_and_enable_shared() 코드의 동작 방식을 설명하는 위에 적혀 있음.
    //
    // --------------------------------------------------
    //
    // shared_ptr의 소멸자는 로직을 수행함.
    // 
    // 1. shared_ptr의 소멸자는 _Decref()를 호출함.
    //
    // ~shared_ptr() noexcept { // release resource
    //     this->_Decref(); // _Ptr_base에 있는 함수
    // }
    //
    // 2. _Ptr_base의 _Decref()는 _Ref의 _Decref()를 호출함(_Ref는 레퍼런스 카운팅 블록).
    //
    // void _Decref() noexcept { // decrement reference count
    //     if (_Rep) {
    //         _Rep->_Decref(); // _Ref_count_base에 있는 함수
    //     }
    // }
    //
    // 3. 레퍼런스 카운팅 블록의 _Decref()는 _Uses를 감소시키고 0에 도달하면 _Destroy()와 _Decwref()를 수행함(_Destroy()와 _Decwref()는 가상 함수임).
    //
    // void _Decref() noexcept { // decrement use count
    //     if (_MT_DECR(_Uses) == 0) {
    //         _Destroy();
    //         _Decwref();
    //     }
    // }
    //
    // *4*. _Destroy()를 호출하는 과정에서 객체의 소멸자를 호출함.
    //
    // void _Destroy() noexcept override { // destroy managed resource
    //     _STD _Destroy_in_place(_Storage._Value);
    // }
    //
    // template <class _Ty>
    // _CONSTEXPR20 void _Destroy_in_place(_Ty& _Obj) noexcept {
    //     if constexpr (is_array_v<_Ty>) {
    //         _STD _Destroy_range(_Obj, _Obj + extent_v<_Ty>);
    //     } else {
    //         _Obj.~_Ty();
    //     }
    // }
    //
    // 5. _Decwref()를 호출하면 _Weaks를 감소시키고 0에 도달하면 자기 자신(레퍼런스 카운팅 블록)을 소멸시키는 함수를 호출함.
    //
    // void _Decwref() noexcept { // decrement weak reference count
    //     if (_MT_DECR(_Weaks) == 0) {
    //         _Delete_this();
    //     }
    // }
    //
    // void _Delete_this() noexcept override { // destroy self
    //     delete this;
    // }
    //
    // --------------------------------------------------
    //
    // shared_ptr이 생성되는 순간 가지는 레퍼런스 카운팅은 기본적으로 [1 strong refs, 1 weak ref]이다.
    // 위 로직에 따르면 본래 shared_ptr의 소멸자를 거치면 두 카운터는 하나씩 감소한다.
    //
    // 하지만 this를 스마트 포인터로 전달하기 위해 enable_shared_from_this<T>를 사용하면
    // 최초 생성 시 레퍼런스 카운팅 블록은 _Wptr로 인한 복사로 인해 [1 strong refs, 2 weak ref]가 된다.
    //
    // enable_shared_from_this<T>를 적용한 shared_ptr의 소멸자 호출 과정을 보면 [0 strong refs, 1 weak ref]이 되어 누수가 날 것 같지만
    // _Uses(strong refs)가 0에 도달하여 객체를 소멸시키는 과정을 거치면 enable_shared_from_this<T>가 가진 weak_ptr의 소멸자도 호출된다.
    // 따라서 [0 strong refs, 0 weak ref]에 도달하여 정상적으로 객체와 레퍼런스 카운팅 블록을 해제할 수 있다.

    // !! enable_shared_from_this<T>를 적용해도 분석할 때 [1 strong refs, 1 weak ref]로 나올 수 있다. !!
    // !! 하지만 조사식을 통해 sptr._Rep->_Uses와 sptr._Rep->_Weaks를 관찰하면 각각 1과 2로 나오는 것을 볼 수 있다. !!

    return 0;
}
