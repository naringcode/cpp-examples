// Update Date : 2024-11-05
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64

#include <iostream>
#include <memory>

#include <utility> // index_sequence

using namespace std;

class TestObject
{
public:
    TestObject(int valA) : _valA{ valA }
    {
        cout << "TestObject()\n";
    }

    /* virtual */ ~TestObject()
    {
        cout << "~TestObject()\n";
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
    }

    /* virtual */ ~TestObjectEx()
    {
        cout << "~TestObjectEx()\n";
    }

private:
    double _valB = 0.0;
};

template <typename... Args, size_t... Is>
void PrintArgs(std::index_sequence<Is...>, Args&&... args)
{
    // 폴드 표현식으로 인자 출력
    ((cout << "\tArg[" << Is << "] Type : " << typeid(args).name() << ", Value : " << args << '\n'), ...);
}

template <typename T>
void MyDeleter(T* ptr);

template <typename T, typename... Args>
shared_ptr<T> MyMakeShared(Args&&... args)
{
    cout << "MyMakeShared()\n" << "\tType : " << typeid(T).name() << '\n';

    // 폴드 표현식으로 인자 출력
    // ((cout << "\tArg Type : " << typeid(args).name() << ", Value : " << args << '\n'), ...);

    // sizeof...()은 가변인자 템플릿의 인자 개수를 구하기 위해서 사용함.
    // 인자 팩을 확장하는 건 아니니까 혼동하지 말 것.
    PrintArgs(std::make_index_sequence<sizeof...(args)>(), args...); // std::forward()로 전달하면 아래 std::forward()에 문제가 생길 수 있음(MyMakeShared()를 한 번 더 래핑하면 해결할 수 있긴 함).

    // 이런저런 코드를 테스트해봤을 때 std::forward() 말고 std::forward<Args>()를 써야 하는 경우가 있었음.

    // 이쪽이 내 스타일 코드지만 MyMakeShared<TestObjectEx>(300UL, 2.718281f)을 사용할 때 문제가 생김.
    // !! 리스트 초기화는 암시적 형변환을 허용하지 않음. !!
    // T* ptr = new T{ std::forward<Args>(args)... };

    // 어쩔 수 없이 암시적인 형변환을 허용하도록 함.
    T* ptr = new T(std::forward<Args>(args)...);

    // 어떤 Deleter를 호출할 것인지 미리 지정하고 shared_ptr을 생성한다.
    return shared_ptr<T>{ ptr, & MyDeleter<T> };
}

template <typename T>
void MyDeleter(T* ptr)
{
    cout << "MyDeleter<T>()\n" << "\tType: " << typeid(T).name() << '\n';

    delete ptr;
}

int main()
{
    // !! Visual Studio 2022 기준이며 아래 분석 내용은 컴파일러 버전에 따라 달라질 수 있음. !!

    // !! 사전 지식(매우 중요 매우 중요 매우 중요 | 꼭 읽고 넘어가기) !!
    // 
    // shared_ptr과 weak_ptr은 _Ptr_base을 이용하는데 이건 다음과 같은 형태로 구성되어 있다(unique_ptr은 해당하지 않음).
    // 
    // [ element_type* _Ptr ][ _Ref_count_base* _Rep ]
    // 
    // 여기서 중요한 것은 _Rep이며 shared_ptr과 weak_ptr에서 수행되는 대부분의 작업은 여기서 이루어진다.
    // 
    // _Ref_count_base는 2개의 레퍼런스 카운터 _Uses와 _Weaks를 가지고 있으며,
    // _Uses와 _Weaks가 0으로 떨어질 때 호출될 2개의 순수 가상 함수 _Destroy()와 _Delete_this()도 들고 있다.
    // 
    // class __declspec(novtable) _Ref_count_base { // common code for reference counting
    // private:
    //     virtual void _Destroy() noexcept     = 0; // destroy managed resource
    //     virtual void _Delete_this() noexcept = 0; // destroy self
    // 
    //     _Atomic_counter_t _Uses  = 1;
    //     _Atomic_counter_t _Weaks = 1;
    // 
    //     ...
    // };
    // 
    // 순수 가상 함수가 적용되어 있기 때문에 _Ref_count_base를 그대로 사용하지 않고 이를 상속하여 확장한 클래스를 사용할 것이다.
    // 어떤 파생 클래스를 사용할 것인지는 생성한 스마트 포인터의 특성에 따라 달라진다.
    // 
    // !! 이 스마트 포인터의 특성은 shared_ptr과 weak_ptr의 타입을 말하는 것이 아니다. !!
    // !! make_shared<T>()를 통해서 생성했는지, 유저가 할당한 메모리를 넘겼는지, 할당자를 사용할 것인지, Custom Deleter는 적용한 것인지를 말하는 것이다. !!
    // 
    // --------------------------------------------------
    // 
    // _Uses는 사용하고 있는 객체의 유효성을 검증하기 위한 카운터이고, 
    // _Weaks는 스마트 포인터 차원에서 할당한 메모리 블록(레퍼런스 카운팅 블록)의 유효성을 검증하기 위한 카운터이다.
    // 
    // _Uses가 0으로 떨어지면 객체는 더 이상 유효해선 안 되니 _Destroy()를 통해 객체의 소멸자를 호출한다.
    // 다만 이게 메모리 블록의 해제를 의미하지는 않는다(매우 중요).
    // 
    // _Weaks가 0으로 떨어져야만 _Delete_this()를 통해 메모리 블록을 해제한다.
    // 참고로 _Uses가 유효하다면 _Weaks는 항상 유효하다.
    // 즉, _Uses와 _Weaks가 둘 다 0으로 떨어져야만 shared_ptr 차원에서 할당한 메모리 블록을 해제한다는 뜻이다.
    //
    // 최초 shared_ptr을 생성하면 _Uses와 _Weaks의 값은 1로 초기화된다.
    // 이후 shared_ptr에 복사하면 _Uses의 값이 1 증가하고, weak_ptr에 복사하면 _Weaks의 값이 1 증가한다.
    // 
    // --------------------------------------------------
    // 
    // shared_ptr의 소멸자가 호출되면 다음 로직을 수행한다.
    // 
    // 1. _Uses를 1 감소시킨다.
    // 2. _Uses 값이 0으로 떨어졌는가?
    // -> 3. 그렇다면 _Destroy()를 호출하여 객체의 소멸자를 호출한다(메모리 블록의 해제는 아직 아님).
    // -> 4. 이어서 _Weaks의 값을 1 감소시킨다.
    // -> 5. _Weaks의 값이 0으로 떨어졌는가?
    //    -> 6. 그렇다면 메모리 블록을 해제한다(메모리 블록 해제는 여기서 일어남).
    // 
    // 만약 어디에서 weak_ptr을 하나 이상 사용하고 있으면 _Weaks는 0으로 떨어지지 않는다.
    // 이 경우 객체의 소멸자가 호출되어도 shared_ptr을 생성할 때 할당한 메모리 블록은 계속 유효한 상태가 된다.
    // 
    // --------------------------------------------------
    // 
    // weak_ptr의 소멸자 또한 비슷한 과정을 거친다.
    // 차이가 있다면 객체의 유효성과 관련한 내용은 없고 메모리 블록의 상태에만 관련이 있다는 것이다.
    // 
    // 1. _Weaks의 값을 1 감소시킨다.
    // 2. _Weaks의 값이 0으로 떨어졌는가?
    // -> 3. 그렇다면 메모리 블록을 해제한다.
    // 
    // --------------------------------------------------
    // 
    // !! shared_ptr을 통해 _Uses가 0으로 떨어지기 전까지 _Weaks의 값은 항상 유효하다는 사실을 기억해야 한다. !!
    // 
    // 실질적으로 레퍼런스 카운팅을 조작하는 건 shared_ptr이나 weak_ptr이 아니다.
    // 레퍼런스 카운팅을 수행하는 건 스마트 포인터에 연동된 메모리 블록이다.
    // 
    // 1. 소멸자 호출
    // ~shared_ptr() noexcept { // release resource
    //     this->_Decref();
    // }
    // 
    // 2. _Ptr_base<T>의 _Decref() 호출
    // void _Ptr_base<T>::_Decref() noexcept { // decrement reference count
    //     // 연동된 메모리 블록의 것으로 호출
    //     if (_Rep) {
    //         _Rep->_Decref();
    //     }
    // }
    // 
    // 3. _Ref_count_base의 _Decref() 호출
    // void _Ref_count_base::_Decref() noexcept { // decrement use count
    //     if (_MT_DECR(_Uses) == 0) {
    //         _Destroy(); // 순수 가상 함수
    //         _Decwref();
    //     }
    // }
    // 
    // 4. _Ref_count_base의 _Decwref() 호출
    // void _Ref_count_base::_Decwref() noexcept { // decrement weak reference count
    //     if (_MT_DECR(_Weaks) == 0) {
    //         _Delete_this(); // 순수 가상 함수
    //     }
    // }
    // 
    // _Uses가 0으로 떨어지면 _Destroy()를 호출하고, _Weaks가 0으로 떨어지면 _Delete_this()를 호출한다.
    // 이 두 함수는 메모리 블록(혹은 레퍼런스 카운팅 블록)의 순수 가상 함수인 만큼 파생 클래스에 의존적이다.
    // 
    // !! C++에서 사용하는 shared_ptr의 핵심은 _Ref_count_base를 상속한 클래스들 중 어떤 파생 클래스를 사용하여, !!
    // !! 어떤 _Destroy()와 _Delete_this()를 호출하느냐에 있다. !!
    // !! 특히 _Destroy()를 유심히 봐야 한다. !!
    // 
    // 메모리 블록, 레퍼런스 카운팅 블록, 컨트롤 블록은 같은 의미니까 다르게 보면 안 된다.
    //
    // _Ref_count_base는 위 3가지 의미를 전부 내포하니까 명칭은 좋을대로 쓰면 된다.
    // Visual Studio의 Inspector에서는 이걸 [control block]라고 칭한다.
    // 

    // MyMakeShared<TestObjectEx>()로 shared_ptr 생성
    {
        shared_ptr<TestObjectEx> objEx = MyMakeShared<TestObjectEx>(100, 3.141592);

        // MyMakeShared<T>()는 shared_ptr을 생성할 때 deleter를 등록하는 방식을 사용한다.
        // 
        // shared_ptr<T>{ ptr, &MyDeleter<T> };
        //
        // --------------------------------------------------
        // 
        // 커스텀 Deleter를 적용할 경우 레퍼런스 카운팅 블록은 _Ref_count_resource<_Res, _Dx>를 사용한다.
        // 
        // 1. 생성자 호출
        // template <class _Ux, class _Dx,
        //     enable_if_t<conjunction_v<is_move_constructible<_Dx>, _Can_call_function_object<_Dx&, _Ux*&>,
        //                     _SP_convertible<_Ux, _Ty>>,
        //         int> = 0>
        // shared_ptr(_Ux* _Px, _Dx _Dt) { // construct with _Px, deleter
        //     _Setpd(_Px, _STD move(_Dt));
        // }
        // 
        // 
        // 2. _Setpd() 호출
        // template <class _UxptrOrNullptr, class _Dx>
        // void shared_ptr<T>::_Setpd(const _UxptrOrNullptr _Px, _Dx _Dt) { // take ownership of _Px, deleter _Dt
        //     // !! ----- _Px는 외부에서 생성한 객체, _Dt는 유저가 설정한 Deleter ----- !!
        //     _Temporary_owner_del<_UxptrOrNullptr, _Dx> _Owner(_Px, _Dt); // 전달 용도의 임시 객체
        // 
        //     _Set_ptr_rep_and_enable_shared(
        //         _Owner._Ptr, 
        //         // !! ----- 레퍼런스 카운팅 블록 생성 ----- !!
        //         new _Ref_count_resource<_UxptrOrNullptr, _Dx>(_Owner._Ptr, _STD move(_Dt)));
        // 
        //     // _Ref_count_resource<_Px, _Dx>를 생성 후 _Set_ptr_rep_and_enable_shared()를 호출하여 인자로 전달함.
        // 
        //     _Owner._Call_deleter = false;
        // }
        // 
        // 3. _Ref_count_resource<_Res, _Dx>는 _Ref_count_base의 파생 클래스이자 적용할 레퍼런스 카운팅 블록
        // template <class _Resource, class _Dx>
        // class _Ref_count_resource : public _Ref_count_base {
        // public:
        // _Ref_count_resource(_Resource _Px, _Dx _Dt)
        //     : _Ref_count_base(), _Mypair(_One_then_variadic_args_t{}, _STD move(_Dt), _Px) {}
        // 
        //     ...
        // 
        // private:
        //     void _Destroy() noexcept override { // destroy managed resource
        //         _Mypair._Get_first()(_Mypair._Myval2);
        //     }
        // 
        //     void _Delete_this() noexcept override { // destroy self
        //         delete this;
        //     }
        // 
        //     // _Dx가 Deletor이고 _Resource는 Deletor에 전달할 포인터이다.
        //     _Compressed_pair<_Dx, _Resource> _Mypair;
        // };
        // 
        // 4. _Set_ptr_rep_and_enable_shared() 호출
        // template <class _Ux>
        // void shared_ptr<T>::_Set_ptr_rep_and_enable_shared(_Ux* const _Px, _Ref_count_base* const _Rx) noexcept { // take ownership of _Px
        //     this->_Ptr = _Px; // 유저가 생성하고 전달한 객체의 포인터
        //     this->_Rep = _Rx; // 레퍼런스 카운팅 블록(Deleter가 묶인 형태)
        //     if constexpr (conjunction_v<negation<is_array<_Ty>>, negation<is_volatile<_Ux>>, _Can_enable_shared<_Ux>>) {
        //         if (_Px && _Px->_Wptr.expired()) {
        //             _Px->_Wptr = shared_ptr<remove_cv_t<_Ux>>(*this, const_cast<remove_cv_t<_Ux>*>(_Px));
        //         }
        //     }
        // }
        // 
        // --------------------------------------------------
        // 
        // 레퍼런스 카운팅 블록의 형태를 잘 봐야 한다.
        // 
        // 기반 클래스인 _Ref_count_base에는 기본이 되는 _Uses와 _Weaks가 들어있고,
        // 파생 클래스인 _Ref_count_resource<_Res, _Dx>에는 전달한 deleter(_Dx)와 메모리(_Resource)를 묶어서 Pair의 형태로 저장한다.
        // 
        // _Uses가 0으로 떨어지면 _Ref_count_resource<_Res, _Dx>의 _Destroy()를 호출하는데,
        // 해당 함수는 Pair를 활용하여 유저가 전달한 deleter를 추출하고 넘겼던 메모리를 인자로 전달한다.
        // 
        // 여기서 알 수 있는 건 메모리 해제는 유저의 Custom Deleter에 의존적이라는 것이다.
        // shared_ptr + _Ref_count_resource<_Res, _Dx> 조합은 사용자가 전달한 메모리를 해제하지 않는다.
        // 
        // !! _Uses와 _Weaks가 0으로 떨어지면 _Delete_this()를 호출하여 자기 자신을 삭제하긴 하지만 그렇다고 전달한 메모리까지 삭제하는 건 아니다. !!
        // !! 혼동할 수 있는 사항이니 반드시 주의하도록 한다. !!
        //
    }

    cout << "--------------------------------------------------\n";

    // MyMakeShared<TestObjectEx>()로 생성 후 static_pointer_cast<TestObject>()로 다운캐스팅했을 때 어떤 소멸자가 호출되는지 관찰
    {
        shared_ptr<TestObject> obj;

        {
            shared_ptr<TestObjectEx> objEx = MyMakeShared<TestObjectEx>(300UL, 2.718281f);

            obj = static_pointer_cast<TestObject>(objEx);
        }

        cout << "after static_pointer_cast...\n";

        // !! 해제는 shared_ptr<TestObject>의 소멸자를 통해서 이루어짐. !!
        // !! 하지만 이게 ~TestObject()를 호출하겠다는 뜻은 아님. !!
        // !! shared_ptr<TestObject>와 TestObject는 아예 별개의 객체임. !!

        // static_pointer_cast<T1, T2>()를 통해 shared_ptr<TestObject>를 받아와 사용하고,
        // 최종적으로 shared_ptr<TestObject>의 소멸자를 거치게 해도 호출되는 객체의 소멸자는 ~TestObjectEx()이다.
        //
        // 이건 deleter로 등록된 함수가 MyDeleter<TestObjectEx>이기 때문이 그런 것이다.
        // 
        // --------------------------------------------------
        //
        // static_pointer_cast<T1, T2>()는 다음 과정을 거치며 shared_ptr을 생성한다.
        // 
        // 1. static_pointer_cast<T1, T2>()를 호출
        // _EXPORT_STD template <class _Ty1, class _Ty2>
        // _NODISCARD shared_ptr<_Ty1> static_pointer_cast(const shared_ptr<_Ty2>& _Other) noexcept {
        //     // static_cast for shared_ptr that properly respects the reference count control block
        //     const auto _Ptr = static_cast<typename shared_ptr<_Ty1>::element_type*>(_Other.get());
        // 
        //     // _Other는 캐스팅 되는 대상을 담은 스마트 포인터(shared_ptr<T2>), _Ptr은 캐스팅한 포인터를 담은 변수(T1*)이다.
        //     //
        //     // _Other는 shared_ptr이기에 _Ptr(사용 대상이 되는 포인터)과 _Rep(레퍼런스 카운팅 블록)을 가진다.
        //     // _Ptr은 포인터를 캐스팅한 것일 뿐이지 레퍼런스 카운팅 블록을 가지고 있는 건 아니다.
        //     //
        //     // 따라서 shared_ptr의 생성자를 통해 캐스팅한 포인터와 함께 레퍼런스 카운팅 블록을 묶어야 한다.
        //     return shared_ptr<_Ty1>(_Other, _Ptr); // 이걸 이용해서 shared_ptr을 생성함.
        // }
        // 
        // 2. shared_ptr<T1>의 생성자 호출
        // template <class _Ty2>
        // shared_ptr(const shared_ptr<_Ty2>& _Right, element_type* _Px) noexcept {
        //     // construct shared_ptr object that aliases _Right
        //     this->_Alias_construct_from(_Right, _Px);
        // }
        // 
        // 3._Alias_construct_from() 호출
        // template <class _Ty2>
        // void _Ptr_base::_Alias_construct_from(const shared_ptr<_Ty2>& _Other, element_type* _Px) noexcept {
        //     // implement shared_ptr's aliasing ctor
        //     _Other._Incref(); // shared_ptr에는 레퍼런스 카운팅 블록이 있다(_Uses의 값을 1 증가).
        // 
        //     _Ptr = _Px;         // 캐스팅한 포인터를 사용
        //     _Rep = _Other._Rep; // 최초 shared_ptr을 만들 때 생성한 레퍼런스 카운팅 블록(_Ref_count_resource<_Res, _Dx>)을 사용
        // }
        // 
        // !! _Ref_count_resource<_Res, _Dx>에는 사용자가 지정한 deleter와 메모리가 묶여 있는데 그걸 그대로 전달한다. !!
        // !! _Rep는 _Ref_count_base의 포인터지만 _Ref_count_resource<_Res, _Dx>는 _Ref_count_base의 파생 클래스이기 때문에 그냥 넘겨서 사용할 수 있다. !!
        // 
        // --------------------------------------------------
        // 
        // _Uses가 0으로 떨어지면 _Ref_count_resource<_Res, _Dx>의 _Destroy()가 호출된다.
        // 이 함수는 유저가 넘긴 Custom Deleter를 호출하고, 최초 shared_ptr을 생성했을 때 넘긴 객체의 포인터를 인자로 넣는다.
        // 
        // 유저가 넘긴 Custom Deleter는 MyDeleter<TestObjectEx>이고,
        // 최초 shared_ptr을 생성했을 때 넘긴 포인터는 할당한 TestObjectEx 객체의 메모리 주소이다.
        // 
        // shared_ptr<TestObject>의 소멸자를 거쳐 _Uses가 0으로 떨어진다고 해도 유저가 정의한 Custom Deleter가 호출된다.
        // 즉, MyDeleter<TestObjectEx>()가 호출된다는 소리이다.
        // 
        // template <typename T>
        // void MyDeleter(T* ptr)
        // {
        //     cout << "MyDeleter<T>()\n" << "\tType: " << typeid(T).name() << '\n';
        // 
        //     delete ptr;
        // }
        // 
        // T는 TestObjectEx이기 때문에 delete되는 대상은 TestObjectEx의 포인터이다.
        // 
        // !! TestObjectEx를 바로 삭제하기 때문에 TestObject의 소멸자를 가상화하지 않아도 된다. !!
        // !! 하지만 shared_ptr을 사용하지 않고 일반적인 동적 할당(?)을 사용할 수도 있고, !!
        // !! 어차피 인터페이스는 가상 함수를 통해 구성하는 경우가 많아 어차피 가상함수 테이블은 생성된다. !!
        // 
        // 특정 API의 요구 조건을 맞추는 것이 아니라면 위 동작 방식을 악로 있다고 해도 소멸자는 가상화하여 정의하도록 하자. !!
        //
    }

    cout << "--------------------------------------------------\n";

    // C++이 제공하는 make_shared<TestObjectEx>() 형태로 shared_ptr 생성
    {
        shared_ptr<TestObjectEx> objEx = make_shared<TestObjectEx>(500, 1.414141);

        // !! 생성 관련 설명과 해제 관련 설명을 구분해서 진행할 것이다. !!

        // make_shared<T>()를 이용한 생성 관련 설명
        // 
        // --------------------------------------------------
        // 
        // make_shared<T>()를 보면 _Ref_count_obj2<T>와 shared_ptr을 따로 생성한 다음 연동하는 작업을 거친다.
        // 
        // 가장 먼저 하는 일은 _Ref_count_obj2<T>를 생성하는 것인데 이건 _Ref_count_base의 파생 클래스이다.
        // _Ref_count_obj2<T>는 레퍼런스 카운터 블록임과 동시에 저장소를 가지는 형태로 구성되어 있다.
        // 
        // !! T를 동적 할당해서 사용하는 것이 아닌 _Ref_count_obj2<T>를 생성해 저장소를 가져와 쓰는 것임. !!
        // 
        // _Ref_count_obj2<T>를 생성했으면 shared_ptr의 _Ptr과 _Rep를 연동하는 작업을 거쳐야 한다.
        // _Ptr은 _Ref_count_obj2<T>의 저장소이고, _Rep는 _Ref_count_obj2<T> 그 자체이다(그 자체로 레퍼런스 카운팅 블록이니까).
        //
        // 1. make_shared<T> 호출
        // _NODISCARD_SMART_PTR_ALLOC shared_ptr<_Ty> make_shared(_Types&&... _Args) { // make a shared_ptr to non-array object
        //     // ---------- 구분하기 쉽게 ----------
        //     const auto _Rx = new _Ref_count_obj2<_Ty>(_STD forward<_Types>(_Args)...); // 객체 생성
        //     // ---------- 구분하기 쉽게 ----------
        // 
        //     shared_ptr<_Ty> _Ret; // shared_ptr을 별도로 생성
        // 
        //     // 객체(저장소)와 shared_ptr을 따로 생성하고 연동하는 개념임.
        //     _Ret._Set_ptr_rep_and_enable_shared(_STD addressof(_Rx->_Storage._Value), _Rx);
        // 
        //     return _Ret;
        // } 
        //
        // 2. _Ref_count_obj2<T>는 _Ref_count_base의 파생 클래스이자 적용할 레퍼런스 카운팅 블록
        // template <class _Ty>
        // class _Ref_count_obj2 : public _Ref_count_base {
        // public:
        //     template <class... _Types>
        //     explicit _Ref_count_obj2(_Types&&... _Args) : _Ref_count_base() {
        // #if _HAS_CXX20
        //         if constexpr (sizeof...(_Types) == 1 && (is_same_v<_For_overwrite_tag, remove_cvref_t<_Types>> && ...)) {
        //             _STD _Default_construct_in_place(_Storage._Value);
        //             ((void) _Args, ...);
        //         } else
        // #endif // _HAS_CXX20
        //         {
        //             // !! ---------- 이걸 사용함. ---------- !!
        //             _STD _Construct_in_place(_Storage._Value, _STD forward<_Types>(_Args)...);
        //         }
        //     }
        // 
        //     ...
        // 
        //     union {
        //         // _Storage는 _Ref_count_obj2<T>에 묶여있는 형태임.
        //         // union으로 묶여있는 객체의 생성자는 기본적으로 호출되지 않음.
        //         // 즉, 생성자는 호출하지 않고 _Ty에 해당하는 메모리만 확보한 것임.
        //         _Wrap<remove_cv_t<_Ty>> _Storage;
        //     };
        // 
        //     ...
        // };
        // 
        // template <class _Ty>
        // struct _Wrap {
        //     _Ty _Value; // workaround for VSO-586813 "T^ is not allowed in a union"
        // };
        // 
        // --------------------------------------------------
        // 
        // _Ref_count_obj2<T>의 생성자는 _Construct_in_place()를 호출하는데 이건 내부에서 placement new를 사용한다.
        // 
        // 3._Construct_in_place() 호출
        // template <class _Ty, class... _Types>
        // _CONSTEXPR20 void _Construct_in_place(_Ty& _Obj, _Types&&... _Args) noexcept(
        //     is_nothrow_constructible_v<_Ty, _Types...>) {
        // #if _HAS_CXX20
        //     if (_STD is_constant_evaluated()) {
        //         _STD construct_at(_STD addressof(_Obj), _STD forward<_Types>(_Args)...);
        //     } else
        // #endif // _HAS_CXX20
        //     {
        //         // ---------- 찾기 쉽게 ----------
        //         // placement new를 통해서 저장소를 초기화한다.
        //         // _Obj는 _Ty&로 전달되기에 저장소의 레퍼런스라고 보면 된다(_Storage._Value).
        //         // _Args는 완벽한 전달로 받은 가변인자이다.
        //         //
        //         // 조금 어려울 수 있는데 쉽게 풀어서 보면 이런 형태이다.
        //         // new (storage address) _Ty(arguments...)
        //         ::new (static_cast<void*>(_STD addressof(_Obj))) _Ty(_STD forward<_Types>(_Args)...);
        //     }
        // }
        // 
        // new _Ref_count_obj2<_Ty>(...)로 오브젝트를 할당하면 대상 객체 _Ty가 저장소의 형태로 union에 묶인다.
        // 즉, 저장소는 새롭게 동적 할당하는 것이 아닌 _Ref_count_obj2를 생성할 때 딸려오는 것이다.
        // 
        // union으로 묶인 객체인 만큼 생성자가 호출되지 않으니 직접 placement new를 활용하여
        // 저장소의 메모리를 대상으로 생성자를 직접 호출하고 있는 형태로 되어 있다.
        // 
        // --------------------------------------------------
        //
        // _Ref_count_obj2<T>를 생성하고 이를 shared_ptr에 전달하는 과정 중 _Set_ptr_rep_and_enable_shared()를 사용한다.
        // 
        // shared_ptr<_Ty> _Ret;
        // _Ret._Set_ptr_rep_and_enable_shared(_STD addressof(_Rx->_Storage._Value), _Rx);
        //
        // 4. _Set_ptr_rep_and_enable_shared() 호출
        // template <class _Ux>
        // void shared_ptr<T>::_Set_ptr_rep_and_enable_shared(_Ux* const _Px, _Ref_count_base* const _Rx) noexcept { // take ownership of _Px
        //     this->_Ptr = _Px; // _Ref_count_obj2<T>의 저장소 주소
        //     this->_Rep = _Rx; // 레퍼런스 카운팅 블록(저장소가 묶인 형태)
        //     if constexpr (conjunction_v<negation<is_array<_Ty>>, negation<is_volatile<_Ux>>, _Can_enable_shared<_Ux>>) {
        //         if (_Px && _Px->_Wptr.expired()) {
        //             _Px->_Wptr = shared_ptr<remove_cv_t<_Ux>>(*this, const_cast<remove_cv_t<_Ux>*>(_Px));
        //         }
        //     }
        // }
        //

        // 소멸자가 호출되는 과정
        // 
        // --------------------------------------------------
        // 
        // _Ref_count_base는 _Uses와 _Weaks를 가지는 레퍼런스 카운팅 블록이며 두 개의 순수 가상 함수를 가진다.
        // 
        // 1. _Ref_count_base의 형태(사전 지식에서 다룬 내용)
        // class __declspec(novtable) _Ref_count_base {
        // private:
        //     virtual void _Destroy() noexcept     = 0; // destroy managed resource
        //     virtual void _Delete_this() noexcept = 0; // destroy self
        // 
        //     _Atomic_counter_t _Uses  = 1;
        //     _Atomic_counter_t _Weaks = 1;
        // 
        //     ...
        // };
        // 
        // 2. _Ref_count_obj2<T>는 _Ref_count_base의 파생 클래스이기에 2개의 가상 함수를 오버라이딩하여 구체화해야 함.
        // template <class _Ty>
        // class _Ref_count_obj2 : public _Ref_count_base {
        //     ...
        // 
        // private:
        //     void _Destroy() noexcept override { // destroy managed resource
        //         _STD _Destroy_in_place(_Storage._Value);
        //     }
        // 
        //     void _Delete_this() noexcept override { // destroy self
        //         delete this;
        //     }
        // };
        //
        // --------------------------------------------------
        //
        // 3. _Uses가 0으로 떨어져 객체의 소멸 조건을 만족하면 _Destroy()를 통해서 _Destroy_in_place()를 호출
        // template <class _Ty>
        // _CONSTEXPR20 void _Ref_count_obj2<T>::_Destroy_in_place(_Ty& _Obj) noexcept {
        //     if constexpr (is_array_v<_Ty>) {
        //         _STD _Destroy_range(_Obj, _Obj + extent_v<_Ty>);
        //     } else {
        //         _Obj.~_Ty();
        //     }
        // }
        //
        // 위 코드를 보면 ~_Ty()를 통해 관리 대상의 소멸자를 직접적으로 호출하고 있는 것을 볼 수 있다.
        // 여기서 말하는 관리 대상은 _Ref_count_obj2의 저장소이다.
        //
    }

    cout << "--------------------------------------------------\n";

    // C++이 제공하는 make_shared<TestObjectEx>()로 shared_ptr을 생성한 다음에
    // static_pointer_cast<TestObject>()로 다운캐스팅을 하면 어떤 소멸자가 호출되는지 관찰
    {
        shared_ptr<TestObject> obj;

        {
            shared_ptr<TestObjectEx> objEx = make_shared<TestObjectEx>(700, 1.6180339);

            obj = static_pointer_cast<TestObject>(objEx);
        }

        cout << "after static_pointer_cast...\n";

        // 위에서 다룬 static_pointer_cast<T1, T2>와 마찬가지로 해제는 shared_ptr<TestObject>를 통해서 이루어진다.
        // 하지만 동일한 이유로 이게 ~TestObject()를 호출하겠다는 것을 의미하진 않는다.
        //
        // static_pointer_cast<T1, T2>() 또한 make_shared<T>()를 통해서 생성한 레퍼런스 카운팅 블록을 사용한다.
        // 즉, 최초 shared_ptr을 생성했을 때 만든 _Ref_count_obj2<T>을 기반으로 소멸 과정을 진행한다는 뜻이다.
        // 
        // template <class _Ty>
        // class _Ref_count_obj2 : public _Ref_count_base {
        //     ...
        // 
        // private:
        //     void _Destroy() noexcept override { // destroy managed resource
        //         _STD _Destroy_in_place(_Storage._Value);
        //     }
        // 
        //     void _Delete_this() noexcept override { // destroy self
        //         delete this;
        //     }
        // };
        // 
        // --------------------------------------------------
        // 
        // 1. static_pointer_cast<T1, T2>()를 호출
        // _EXPORT_STD template <class _Ty1, class _Ty2>
        // _NODISCARD shared_ptr<_Ty1> static_pointer_cast(const shared_ptr<_Ty2>& _Other) noexcept {
        //     // static_cast for shared_ptr that properly respects the reference count control block
        //     const auto _Ptr = static_cast<typename shared_ptr<_Ty1>::element_type*>(_Other.get());
        // 
        //     // _Other는 캐스팅 되는 대상을 담은 스마트 포인터(shared_ptr<T2>),
        //     // _Ptr은 캐스팅한 포인터를 담은 변수(T1*)인데 이건 _Ref_count_obj2<T2> 저장소의 포인터이다.
        //     //
        //     // _Other는 shared_ptr이기에 _Ptr(사용 대상이 되는 포인터)과 _Rep(레퍼런스 카운팅 블록)을 가진다.
        //     // _Ptr은 포인터를 캐스팅한 것일 뿐이지 레퍼런스 카운팅 블록을 가지고 있는 건 아니다.
        //     //
        //     // 따라서 shared_ptr의 생성자를 통해 캐스팅한 포인터와 함께 레퍼런스 카운팅 블록을 묶어야 한다.
        //     return shared_ptr<_Ty1>(_Other, _Ptr); // 이걸 이용해서 shared_ptr을 생성함.
        // }
        // 
        // 2. shared_ptr<T1>의 생성자 호출
        // template <class _Ty2>
        // shared_ptr(const shared_ptr<_Ty2>& _Right, element_type* _Px) noexcept {
        //     // construct shared_ptr object that aliases _Right
        //     this->_Alias_construct_from(_Right, _Px);
        // }
        // 
        // 3._Alias_construct_from() 호출
        // template <class _Ty2>
        // void _Ptr_base::_Alias_construct_from(const shared_ptr<_Ty2>& _Other, element_type* _Px) noexcept {
        //     // implement shared_ptr's aliasing ctor
        //     _Other._Incref(); // shared_ptr에는 레퍼런스 카운팅 블록이 있다(_Uses의 값을 1 증가).
        // 
        //     _Ptr = _Px;         // 캐스팅한 포인터를 사용
        //     _Rep = _Other._Rep; // 최초 shared_ptr을 만들 때 생성한 레퍼런스 카운팅 블록(_Ref_count_obj2<T2>)을 사용
        // }
        // 
        // !! _Rep로 전달되는 레퍼런스 카운팅 블록은 _Ref_count_obj2<T1>이 아닌 _Ref_count_obj2<T2>이다. !!
        // !! _Ref_count_obj2<T2>는 최초로 shared_ptr을 구성했을 때 make_shared<T2>()로 생성한 레퍼런스 카운팅 블록임을 명심하자. !!
        //
        // !! _Rep는 _Ref_count_base의 포인터지만 _Ref_count_obj2<T2>는 _Ref_count_base의 파생 클래스이기 때문에 그냥 넘겨서 사용할 수 있다. !!
        // 
        // --------------------------------------------------
        // 
        // _Uses가 0으로 떨어지면 _Ref_count_obj2<T>의 _Destroy()를 통해 _Destroy_in_place<T>()를 호출한다.
        // 
        // 1. _Ref_count_obj2<T>의 _Destroy() 호출
        // void _Ref_count_obj2<T>::_Destroy() noexcept override { // destroy managed resource
        //     _STD _Destroy_in_place(_Storage._Value);
        // }
        // 
        // 2. _Destroy_in_place<T>() 호출
        // template <class _Ty>
        // _CONSTEXPR20 void _Destroy_in_place(_Ty& _Obj) noexcept {
        //     if constexpr (is_array_v<_Ty>) {
        //         _STD _Destroy_range(_Obj, _Obj + extent_v<_Ty>);
        //     }
        //     else {
        //         _Obj.~_Ty();
        //     }
        // }
        // 
        // _Ref_count_obj2<T>의 T는 최초 shared_ptr을 생성했을 때 지정한 타입이다.
        // 또한 _Storage._Value는 이 지정한 타입을 의미한다.
        // 
        // 따라서 shared_ptr<TestObject>의 소멸자가 호출된다고 해도 최초 T가 TestObjectEx이기 때문에 TestObjectEx의 소멸자가 호출되는 것이다.
        //
        // void _Destroy_in_place(TestObjectEx& _Obj) noexcept {
        //     _Obj.~TestObjectEx();
        // }
        // 
    }

    cout << "--------------------------------------------------\n";

    // deleter를 지정하지 않고 직접 생성한 메모리를 전달하여 shared_ptr 생성
    {
        TestObjectEx* objEx = new TestObjectEx{ 1111, 1.2345678 };

        shared_ptr<TestObjectEx> sptr{ objEx };

        // deleter를 지정하지 않고 직접 생성한 메모리를 전달하여 shared_ptr을 생성하는 방식은
        // 위에서 언급된 방식들과 비교하면 매우 심플하다.
        // 
        // 레퍼런스 카운팅 블록으로 _Ref_count<T>를 사용하는데 여기서 직접 생성하여 전달한 메모리를 저장한다.
        // 
        // !! shared_ptr의 _Ptr과 _Ref_count<T>의 _Ptr이 가리키는 대상은 동일하지만 별개의 변수이다. !!
        //
        // --------------------------------------------------
        // 
        // 1. shared_ptr<T> 생성자 호출
        // template <class _Ux,
        //     enable_if_t<conjunction_v<conditional_t<is_array_v<_Ty>, _Can_array_delete<_Ux>, _Can_scalar_delete<_Ux>>,
        //                     _SP_convertible<_Ux, _Ty>>,
        //         int> = 0>
        // explicit shared_ptr(_Ux* _Px) { // construct shared_ptr object that owns _Px
        //     if constexpr (is_array_v<_Ty>) {
        //         _Setpd(_Px, default_delete<_Ux[]>{});
        //     }
        //     else {
        //         _Temporary_owner<_Ux> _Owner(_Px); // 임시 객체 생성
        //         _Set_ptr_rep_and_enable_shared(
        //             // !! ----- 직접 생성하여 전달한 객체의 포인터 ----- !!
        //             _Owner._Ptr,
        //             // !! ----- 레퍼런스 카운팅 블록 생성 ----- !!
        //             new _Ref_count<_Ux>(_Owner._Ptr));
        //         _Owner._Ptr = nullptr;
        // 
        //         // _Ref_count<_Ux>를 생성 후 _Set_ptr_rep_and_enable_shared()를 호출하여 인자로 전달함.
        //     }
        // }
        // 
        // 2. _Ref_count<T>는 _Ref_count_base의 파생 클래스이자 적용할 레퍼런스 카운팅 블록
        // template <class _Ty>
        // class _Ref_count : public _Ref_count_base { // handle reference counting for pointer without deleter
        // public:
        //     explicit _Ref_count(_Ty* _Px) : _Ref_count_base(), _Ptr(_Px) {}
        // 
        // private:
        //     void _Destroy() noexcept override { // destroy managed resource
        //         delete _Ptr;
        //     }
        // 
        //     void _Delete_this() noexcept override { // destroy self
        //         delete this;
        //     }
        // 
        //     _Ty* _Ptr; // 직접 생성하여 전달한 객체의 포인터
        // };
        // 
        // 3. _Set_ptr_rep_and_enable_shared() 호출
        // template <class _Ux>
        // void shared_ptr<T>::_Set_ptr_rep_and_enable_shared(_Ux* const _Px, _Ref_count_base* const _Rx) noexcept { // take ownership of _Px
        //     this->_Ptr = _Px; // 유저가 생성하고 전달한 객체의 포인터
        //     this->_Rep = _Rx; // 레퍼런스 카운팅 블록
        //     if constexpr (conjunction_v<negation<is_array<_Ty>>, negation<is_volatile<_Ux>>, _Can_enable_shared<_Ux>>) {
        //         if (_Px && _Px->_Wptr.expired()) {
        //             _Px->_Wptr = shared_ptr<remove_cv_t<_Ux>>(*this, const_cast<remove_cv_t<_Ux>*>(_Px));
        //         }
        //     }
        // }
        // 
        // !! 다시 한 번 말하지만 shared_ptr의 _Ptr과 _Ref_count<T>의 _Ptr은 다른 변수이다. !!
        // !! 가리키고 있는 대상만 같을 뿐이다. !!
        // 
        // --------------------------------------------------
        // 
        // _Uses가 0으로 떨어지면 _Ref_count<TestObjectEx>의 _Destroy()를 통해 직접 delete를 호출한다.
        // 이전의 다른 방식들은 뭔가를 거쳐가는 작업이 있었는데 이전 그냥 delete 오퍼레이터를 호출하는 것이 끝이다.
        // 
        // static_pointer_cast를 통해 다운캐스팅을 적용한 상태에서 _Uses가 0으로 떨어졌을 때 
        // 레퍼런스 카운팅 블록 _Rep가 어떻게 동작하는지 2번이나 설명했기 때문에 이에 대한 설명은 생략한다.
        // 
        // 전체적인 과정이 헷갈린다면 main() 최상단의 사전 지식부터 읽고 차례대로 디버깅해서 분석하도록 하자.
        // 
    }

    cout << "--------------------------------------------------\n";

    // operator=(shared_ptr&&)로 shared_ptr 자체를 갱신할 경우
    {
        // A
        shared_ptr<TestObject> obj;

        // B
        obj = make_shared<TestObject>(100);

        // C
        obj = make_shared<TestObject>(200);

        // _Ptr과 _Rep가 어떻게 갱신되며 shared_ptr의 소멸자가 어떤 방식으로 호출되는지 파악하기 위한 부분이다.
        //
        // --------------------------------------------------
        // 
        // 1. shared_ptr<TestObject> obj를 생성하되 내용을 채우지 않으면 _Ptr과 _Rep는 nullptr로 채워짐.
        // 
        // template <class _Ty>
        // class _Ptr_base { // base class for shared_ptr and weak_ptr
        //     ...
        // 
        // private:
        //     element_type* _Ptr{ nullptr };
        //     _Ref_count_base* _Rep{ nullptr };
        // 
        //     ...
        // };
        // 
        // --------------------------------------------------
        // 
        // 2. obj = make_shared<TestObject>(100)로 인해 operator=(shared_ptr&&)가 호출되었으면?
        // 
        // !! obj의 _Ptr과 _Rep는 nullptr인 상황 !!
        // shared_ptr& operator=(shared_ptr&& _Right) noexcept { // take resource from _Right
        //     // 임시 객체를 생성하고 swap()을 진행해야 함.
        //     shared_ptr(_STD move(_Right)).swap(*this);
        //     return *this;
        // }
        // 
        // !! 임시 객체에 소유권을 이전하기 위해 move(_Right)를 적용하였기에 이동 생성자 호출 !!
        // shared_ptr(shared_ptr&& _Right) noexcept { // construct shared_ptr object that takes resource from _Right
        //     this->_Move_construct_from(_STD move(_Right));
        // }
        // 
        // !! _Move_construct_from(_STD move(_Right)) 호출 !!
        // template <class _Ty2>
        // void _Move_construct_from(_Ptr_base<_Ty2>&& _Right) noexcept {
        //     // implement shared_ptr's (converting) move ctor and weak_ptr's move ctor
        //     // !! ----- make_shared<TestObject>(100)로 생성한 값을 임시 객체에 넘김. ----- !!
        //     _Ptr = _Right._Ptr;
        //     _Rep = _Right._Rep;
        // 
        //     // !! ----- make_shared<TestObject>(100)로 생성한 값을 nullptr로 밀어 버림. ----- !!
        //     _Right._Ptr = nullptr;
        //     _Right._Rep = nullptr;
        // 
        // }
        // 
        // 콜 스택을 빠져 나오고...
        // 
        // shared_ptr& operator=(shared_ptr&& _Right) noexcept { // take resource from _Right
        //     // _Right의 _Ptr과 _Rep는 nullptr로 밀리고, 임시 객체 shared_ptr에 소유권을 양도한 상태
        //     shared_ptr(_STD move(_Right)).swap(*this);
        //     return *this;
        // }
        // 
        // !! swap() 함수 호출 !!
        // void swap(shared_ptr& _Other) noexcept {
        //     // this는 make_shared<TestObject>(100)의 값을 양도 받은 대상임(위에 있는 this랑 여기 있는 this는 다름).
        //     // _Other는 shared_ptr<TestObject> obj임(위에 있는 this는 obj, 여기 있는 this는 임시 객체).
        //     this->_Swap(_Other);
        // }
        // 
        // void _Swap(_Ptr_base& _Right) noexcept { // swap p
        //     // 헷갈릴 수도 있지만 _Right는 shared_ptr<TestObject> obj임('='의 왼쪽에 있던 것인데 _Right로 받은 것)
        //     // "obj = make_shared<TestObject>(100)" 두 값을 실질적으로 교환하는 코드
        //     _STD swap(_Ptr, _Right._Ptr);
        //     _STD swap(_Rep, _Right._Rep);
        // }
        // 
        // 콜 스택을 빠져 나오고...
        // 
        // shared_ptr& operator=(shared_ptr&& _Right) noexcept { // take resource from _Right
        //     shared_ptr(_STD move(_Right)).swap(*this);
        //     return *this;
        // }
        // 
        // !! 임시 객체 shared_ptr(_STD move(_Right))의 소멸자를 호출 !!
        // ~shared_ptr() noexcept { // release resource
        //     this->_Decref();
        // }
        // 
        // void _Decref() noexcept { // decrement reference count
        //     // obj의 _Rep가 nullptr이었기 때문에 _Decref()를 실행하지 않음.
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // 콜 스택을 빠져나와 main 함수까지 도달한 후...
        // 
        // obj = make_shared<TestObject>(100);
        // 
        // !! make_shared<TestObject>(100)의 소멸자 호출 !!
        // ~shared_ptr() noexcept { // release resource
        //     this->_Decref();
        // }
        // 
        // void _Decref() noexcept { // decrement reference count
        //     // 임시 객체를 만드는 도중 make_shared<TestObject>(100)의 자원의 소유권은 이전되었기에 _Rep는 nullptr임.
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // --------------------------------------------------
        // 
        // 3. 값이 채워진 obj를 대상으로 "obj = make_shared<TestObject>(200)"를 진행하면?
        // 
        // shared_ptr& operator=(shared_ptr&& _Right) noexcept { // take resource from _Right
        //     shared_ptr(_STD move(_Right)).swap(*this);
        //     return *this;
        // }
        // 
        // !! 임시 객체를 채우는 과정은 똑같으니 swap()부터 진행 !!
        // void swap(shared_ptr& _Other) noexcept {
        //     // _Other는 obj라는 사실을 잊으면 안 됨.
        //     this->_Swap(_Other);
        // }
        // 
        // void _Swap(_Ptr_base& _Right) noexcept { // swap pointers
        //     // 교환 전 상태 : this{ _Ptr : 200 }, _Right{ _Ptr : 100 }
        //     // 교환 후 상태 : this{ _Ptr : 100 }, _Right{ _Ptr : 200 }
        //     _STD swap(_Ptr, _Right._Ptr);
        //     _STD swap(_Rep, _Right._Rep);
        // 
        //    // !! ----- obj에는 이미 값이 채워진 상태이기 때문에 임시 객체의 _Rep는 nullptr이 아님. ----- !!
        // }
        // 
        // 콜 스택을 빠져 나오고...
        // 
        // shared_ptr& operator=(shared_ptr&& _Right) noexcept { // take resource from _Right
        //     shared_ptr(_STD move(_Right)).swap(*this);
        //     return *this;
        // }
        // 
        // !! 임시 객체 shared_ptr(_STD move(_Right))의 소멸자를 호출 !!
        // ~shared_ptr() noexcept { // release resource
        //     this->_Decref();
        // }
        // 
        // void _Decref() noexcept { // decrement reference count
        //     // 교환의 대상이 된 obj의 _Rep는 nullptr 아니었기에 _Decref()를 호출
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // void _Decref() noexcept { // decrement use count
        //     // _Uses는 현재 1이기에 윈자적 Decrement 연산을 수행하면 0이 됨.
        //     // 그럼 _Destroy()와 _Decwref()를 이어서 실행하는데 이에 대한 설명은 앞선 예시에서 설명함.
        //     if (_MT_DECR(_Uses) == 0) {
        //         _Destroy();
        //         _Decwref();
        //     }
        // }
        // 
        // 콜 스택을 빠져나와 main 함수까지 도달한 후...
        // 
        // obj = make_shared<TestObject>(200);
        // 
        // !! make_shared<TestObject>(200)의 소멸자 호출 !!
        // ~shared_ptr() noexcept { // release resource
        //     this->_Decref();
        // }
        // 
        // void _Decref() noexcept { // decrement reference count
        //     // 임시 객체를 만드는 도중 make_shared<TestObject>(200)의 자원은 소유권은 이전되었기에 _Rep는 nullptr임.
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // --------------------------------------------------
        // 
        // 4. 스코프를 빠져나오며 obj의 소멸자 호출
        // 
        // ~shared_ptr() noexcept { // release resource
        //     this->_Decref();
        // }
        // 
        // void _Decref() noexcept { // decrement reference count
        //     // make_shared<TestObject>(200)로부터 이전 받은 _Rep가 있기 때문에 _Decref() 호출
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // void _Decref() noexcept { // decrement use count
        //     // 값이 1인 _Uses를 원자적으로 감소시키며 _Destroy()와 _Decwref()를 이어서 호출
        //     if (_MT_DECR(_Uses) == 0) {
        //         _Destroy();
        //         _Decwref();
        //     }
        // }
        //
    }

    cout << "--------------------------------------------------\n";

    // operator=(const shared_ptr&)로 shared_ptr 자체를 갱신할 경우
    {
        shared_ptr<TestObject> obj1 = make_shared<TestObject>(100);
        shared_ptr<TestObject> obj2 = make_shared<TestObject>(200);

        // 이 부분만 볼 것임.
        obj2 = obj1;

        // 전체적인 내용은 operator=(shared_ptr&&)로 갱신한 것과 유사함.
        // 
        // obj1과 obj2에 서로 값이 있는 상태에서 대입했을 때 소멸자가 어떻게 호출되는지 관찰하는 것이 핵심임.
        // 
        // --------------------------------------------------
        // 
        // 1. "obj2 = obj1"로 인한 operator=(const shared_ptr&) 호출
        // 
        // shared_ptr& operator=(const shared_ptr& _Right) noexcept {
        //     // this는 obj2, _Right는 obj1임.
        //     // !! shared_ptr(_Right)로 되어 있다. shared_ptr(_STD move(_Right))로 전달되는 것이 아니다. !!
        //     shared_ptr(_Right).swap(*this);
        //     return *this;
        // }
        // 
        // !! 임시 객체를 생성하기 위한 복사 생성자 호출 !!
        // shared_ptr(const shared_ptr& _Other) noexcept { // construct shared_ptr object that owns same resource as _Other
        //     // _Other는 obj1임.
        //     this->_Copy_construct_from(_Other);
        // }
        // 
        // template <class _Ty2>
        // void _Copy_construct_from(const shared_ptr<_Ty2>& _Other) noexcept {
        //     // implement shared_ptr's (converting) copy ctor
        //     // _Other는 obj1이고 이걸 대상으로 _Incref() 호출
        //     _Other._Incref();
        // 
        //     // 레퍼런스 카운팅을 증가시킨 후 임시 객체가 같은 객체 메모리와 레퍼런스 카운팅 블록을 가지게 설정함.
        //     _Ptr = _Other._Ptr;
        //     _Rep = _Other._Rep;
        // }
        // 
        // void _Incref() const noexcept {
        //     // obj1은 make_shared<TestObject>(100)의 소유권을 이전 받은 상태이기에 _Rep는 nullptr이 아님.
        //     if (_Rep) {
        //         _Rep->_Incref();
        //     }
        // }
        // 
        // !! 단순 레퍼런스 카운팅 !!
        // void _Incref() noexcept { // increment use count
        //     _MT_INCR(_Uses);
        // }
        // 
        // 콜 스택을 빠져 나오고...
        // 
        // shared_ptr& operator=(const shared_ptr& _Right) noexcept {
        //     // _Right(obj1)와 임시 객체 shared_ptr의 _Ptr과 _Rep는 같은 대상을 가리키고 있는 상태(레퍼런스 카운팅은 1 증가함)
        //     // this는 obj2임.
        //     shared_ptr(_Right).swap(*this);
        //     return *this;
        // }
        // 
        // !! swap() 함수 호출 !!
        // void swap(shared_ptr& _Other) noexcept {
        //     // this는 obj1과 연동된(?) 임시 객체, _Other는 obj2임.
        //     this->_Swap(_Other);
        // }
        // 
        // void _Swap(_Ptr_base& _Right) noexcept { // swap pointers
        //     // 교환 전 상태 : this{ _Ptr : 100, strong ptr : 2 }, _Right{ _Ptr : 200, strong ptr : 1 }
        //     // 교환 후 상태 : this{ _Ptr : 200, strong ptr : 1 }, _Right{ _Ptr : 100, strong ptr : 2 }
        //     _STD swap(_Ptr, _Right._Ptr);
        //     _STD swap(_Rep, _Right._Rep);
        // 
        //     // this는 obj1을 반영한 임시 객체라는 사실을 잊으면 안 됨.
        //     // _Right는 우리가 실질적으로 반영할 "obj2 = obj1"에서의 obj2임.
        // 
        //     // obj2의 모든 내용을 임시 객체의 것으로 갱신한 상태(obj1과 동일한 _Ptr과 _Rep를 가리키게 함)
        //     // 임시 객체의 내용은 갱신되기 이전의 obj2의 것으로 교체함.
        // }
        // 
        // 콜 스택을 빠져 나오고...
        // 
        // shared_ptr& operator=(const shared_ptr& _Right) noexcept {
        //     shared_ptr(_Right).swap(*this);
        //     return *this;
        // }
        // 
        // !! 임시 객체 shared_ptr(_Right)의 소멸자를 호출 !!
        // ~shared_ptr() noexcept { // release resource
        //     this->_Decref();
        // }
        // 
        // void _Decref() noexcept { // decrement reference count
        //     // 현재 임시 객체가 반영하고 있는 건 교체하기 이전의 obj2의 _Ptr과 _Rep임.
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // void _Decref() noexcept { // decrement use count
        //     // _Uses의 현재 카운트가 1이니 감소시키면 _Destroy()와 _Decwref()를 이어서 호출함.
        //     if (_MT_DECR(_Uses) == 0) {
        //         _Destroy();
        //         _Decwref();
        //     }
        // }
        // 
        // 콜 스택을 빠져나와 main 함수까지 도달한 후...
        // 
        // !! 이전 obj2의 내용물을 임시 객체와 교환해서 임시 객체의 소멸자 호출을 통해 레퍼런스 카운팅을 감소시킴. !!
        // !! obj1과 obj2는 서로 연동된 상태이며 레퍼런스 카운트도 1 증가시킴. !!
        // obj2 = obj1;
        // 
        // --------------------------------------------------
        // 
        // 2. obj2과 obj1의 소멸자를 차례대로 호출
        // 
        // !! obj2의 소멸자 호출 !!
        // ~shared_ptr() noexcept { // release resource
        //     this->_Decref();
        // }
        // 
        // void _Decref() noexcept { // decrement reference count
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // void _Decref() noexcept { // decrement use count
        //     // _Uses의 값이 2이기 때문에 1을 감소시켜도 조건에 충족하지 않음.
        //     if (_MT_DECR(_Uses) == 0) {
        //         _Destroy();
        //         _Decwref();
        //     }
        // }
        // 
        // !! obj1의 소멸자 호출 !!
        // ~shared_ptr() noexcept { // release resource
        //     this->_Decref();
        // }
        // 
        // void _Decref() noexcept { // decrement reference count
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // void _Decref() noexcept { // decrement use count
        //     // _Uses의 값이 1이기 때문에 1을 감소시키면 조건에 충족하고 이어서 _Destroy()와 _Decwref()를 호출함.
        //     if (_MT_DECR(_Uses) == 0) {
        //         _Destroy();
        //         _Decwref();
        //     }
        // }
        //
    }

    cout << "--------------------------------------------------\n";

    // shared_ptr을 복사 생성자로 받았을 경우
    {

        shared_ptr<TestObject> obj1 = make_shared<TestObject>(100);

        // 이 부분만 보도록 할 것임.
        shared_ptr<TestObject> obj2 = obj1;

        // 복사 생성자는 _Copy_construct_from()을 사용하는데 이건 operator=(const shared_ptr&)를 설명하는 과정에서 언급함.
        // 
        // --------------------------------------------------
        // 
        // shared_ptr(const shared_ptr& _Other) noexcept { // construct shared_ptr object that owns same resource as _Other
        //     this->_Copy_construct_from(_Other);
        // }
        // 
        // template <class _Ty2>
        // void _Copy_construct_from(const shared_ptr<_Ty2>& _Other) noexcept {
        //     // implement shared_ptr's (converting) copy ctor
        //     _Other._Incref();
        // 
        //     _Ptr = _Other._Ptr;
        //     _Rep = _Other._Rep;
        // }
        // 
        // void _Incref() const noexcept {
        //     if (_Rep) {
        //         _Rep->_Incref();
        //     }
        // }
        // 
        // void _Incref() noexcept { // increment use count
        //     _MT_INCR(_Uses);
        // }
        // 
        // !! 이게 끝이다. !!
        //
        // --------------------------------------------------
        //
        // 소멸자 호출 과정은 operator=(const shared_ptr&)에서 설명한 과정과 완전 동일하니 생략함.
        //
    }

    cout << "--------------------------------------------------\n";

    return 0;
}
