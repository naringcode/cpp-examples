#include <iostream>
#include <memory>

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

class TestClassB // : public enable_shared_from_this<TestClassA>
{
public:
    TestClassB() { cout << "TestClassB()\n"; }
    ~TestClassB() { cout << "~TestClassB()\n"; }

public:
    // shared_ptr<TestClassA> GetSharedPtr()
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

    weak_ptr<TestClassB>   testWeak2 = testShared3;

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
    return 0;
}
