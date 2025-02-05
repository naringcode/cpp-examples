// Update Date : 2025-02-05
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <concepts>
#include <type_traits>
#include <utility>

// https://en.cppreference.com/w/cpp/utility/functional/invoke

// std::invoke() 내에서 사용하는 것
// template <class _Callable, class _Ty1, class _Removed_cvref = _Remove_cvref_t<_Callable>,
//     bool _Is_pmf = is_member_function_pointer_v<_Removed_cvref>,
//     bool _Is_pmd = is_member_object_pointer_v<_Removed_cvref>>
// struct _Invoker1;
//
// 내부에서는 이 조건을 토대로 invoker의 유형을 지정한다.
// 여기서는 유형에 따라 달라지는 std::invoke()의 기능을 모방한다.

// TEMP
// template <typename Callable, typename Object, typename... Args>
// constexpr auto MyInvoke(Callable&& callable, Object&& obj, Args&&... args)
// {
//     return obj.*callable;
// }

template <typename T>
concept MemberFunc = std::is_member_function_pointer_v<T>;

template <typename T>
concept MemberObject = std::is_member_object_pointer_v<T>;

template <typename T>
concept NonMemberCallable = !MemberFunc<T> && !MemberObject<T>;

template <typename T>
concept SmartPointer = requires(T wrapped) {
    { wrapped.get() } -> std::same_as<typename T::element_type*>;
};

// 일반적인 Callable(일반 함수, 함수 객체, 람다) : _Invoker_strategy::_Functor
template <typename Callable, typename... Args>
    requires NonMemberCallable<Callable>
constexpr auto MyInvoke(Callable&& callable, Args&&... args)
{
    std::cout << "MyInvoke(Callable&& callable, Args&&... args) ";

    return callable(std::forward<Args>(args)...);
}

// 멤버 함수 호출 1 : _Invoker_strategy::_Pmf_object
template <typename MF, typename Object, typename... Args>
    requires MemberFunc<MF> && (!SmartPointer<Object>)
constexpr auto MyInvoke(MF mf, Object& obj, Args&&... args)
{
    std::cout << "MyInvoke(MF mf, Object& obj, Args&&... args) ";

    return (obj.*mf)(std::forward<Args>(args)...);
}

// 멤버 함수 호출 2 : _Invoker_strategy::_Pmf_pointer
template <typename MF, typename Object, typename... Args>
    requires MemberFunc<MF> && (!SmartPointer<Object>)
constexpr auto MyInvoke(MF mf, Object* obj, Args&&... args)
{
    std::cout << "MyInvoke(MF mf, Object* obj, Args&&... args) ";

    return (obj->*mf)(std::forward<Args>(args)...);
}

// 멤버 변수 접근 1 : _Invoker_strategy::_Pmd_object
template <typename MV, typename Object>
    requires MemberObject<MV> && (!SmartPointer<Object>)
constexpr auto MyInvoke(MV mv, Object& obj)
{
    std::cout << "MyInvoke(MV mv, Object& obj) ";

    return obj.*mv;
}

// 멤버 변수 접근 2 : _Invoker_strategy::_Pmd_pointer
template <typename MV, typename Object>
    requires MemberObject<MV> && (!SmartPointer<Object>)
constexpr auto MyInvoke(MV mv, Object* obj)
{
    std::cout << "MyInvoke(MV mv, Object* obj) ";

    return obj->*mv;
}

// 스마트 포인터 멤버 함수 호출 : _Invoker_strategy::_Pmf_refwrap
template <typename MF, typename Wrapped, typename... Args>
    requires MemberFunc<MF> && SmartPointer<Wrapped>
constexpr auto MyInvoke(MF mf, Wrapped&& wrapped, Args&&... args)
{
    std::cout << "MyInvoke(MF mf, Wrapped&& wrapped, Args&&... args) ";
    
    return (wrapped.get()->*mf)(std::forward<Args&&>(args)...);
}

// 스마트 포인터 멤버 변수 접근 : _Invoker_strategy::_Pmd_refwrap
template <typename MV, typename Wrapped, typename... Args>
    requires MemberObject<MV> && SmartPointer<Wrapped>
constexpr auto MyInvoke(MV mv, Wrapped&& wrapped)
{
    std::cout << "MyInvoke(MV mv, Wrapped&& wrapped) ";

    return wrapped.get()->*mv;
}

/*****************
*      Main      *
*****************/

// cppreference의 invoke 문서에 있는 예시를 참고한다.

struct FooObject
{
    FooObject(int num) : num{ num }
    { }

    void PrintAdd(int x) const
    {
        std::cout << num + x;
    }

    int num;
};

void PrintNum(int x)
{
    std::cout << x;
}

struct PrintFunctor
{
    void operator()(int x) const
    {
        std::cout << x;
    }
};

int main()
{
    std::cout << "invoke a free function : ";
    MyInvoke(PrintNum, -9);
    std::cout << '\n';

    std::cout << "invoke a lambda : ";
    MyInvoke([](int x) { PrintNum(42 + x); }, 100);
    std::cout << '\n';

    std::cout << "invoke a function object : ";
    std::invoke(PrintFunctor{ }, 18);
    std::cout << '\n';
    
    //
    const FooObject obj{ 314159 };

    std::cout << "invoke a member function : ";
    std::invoke(&FooObject::PrintAdd, obj, 1);
    std::cout << '\n';

    std::cout << "invoke (i.e., access) a data member num : ";
    std::cout << std::invoke(&FooObject::num, obj); // 멤버 변수도 invoke 가능함.
    std::cout << '\n';

    // 
    std::shared_ptr<FooObject> sptr = std::make_shared<FooObject>(200);
    
    std::cout << "invoke a member function in a shared_ptr : ";
    std::invoke(&FooObject::PrintAdd, sptr, 50);
    std::cout << '\n';

    std::cout << "invoke (i.e., access) a data member num in a shared_ptr : ";
    std::cout << std::invoke(&FooObject::num, sptr); // 멤버 변수도 invoke 가능함.
    std::cout << '\n';

    // TEMP 
    // std::shared_ptr<MyClass> ptr = std::make_shared<MyClass>();
    // int val = std::invoke(&MyClass::value, ptr); // 정상 작동 (C++20 이상)
    //
    // TEMP
    // MyClass myClass;
    // std::cout << MyInvoke([]() { std::cout << "Hello "; return 1000; }) << '\n';
    // std::cout << MyInvoke(&MyClass::Print, myClass, 10) << '\n';
    // std::cout << MyInvoke(&MyClass::Print, &myClass, 20) << '\n';
    // std::cout << MyInvoke(&MyClass::value, myClass) << '\n';
    // std::cout << MyInvoke(&MyClass::value, &myClass) << '\n';
    // std::cout << MyInvoke(&MyClass::Print, ptr, 30) << '\n';
    // std::cout << MyInvoke(&MyClass::value, ptr) << '\n';

    return 0;
}
