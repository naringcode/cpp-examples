// Update Date : 2024-12-26
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
// 7. enable_shared_from_this.cpp <-----
// 8. enable_shared_from_this_details.cpp
// 9. enable_shared_from_this_examples.cpp
//
// # shared_ptr을 멀티스레딩 환경에서 사용할 때 발생할 수 있는 문제점을 기술한 내용
// 10. allocation_aligned_byte_boundaries.cpp(사전지식)
// 11. volatile_atomic_cache_coherence_and_memory_order.cpp(사전지식)
// 12. (중요) shared_ptr_multi_threading_issues.cpp

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

int main()
{
    // enable_shared_from_this<T>를 사용
    shared_ptr<FooObjectA> fooASharedPtr = make_shared<FooObjectA>();

    shared_ptr<FooObjectA> fooASharedFromThisPtr = fooASharedPtr->GetSharedPtr();
    weak_ptr<FooObjectA>   fooAWeakFromThis      = fooASharedPtr->GetWeakPtr();

    // --------------------------------------------------

    // enable_shared_from_this<T>를 미사용
    shared_ptr<FooObjectB> fooBSharedPtr = make_shared<FooObjectB>();
    
    shared_ptr<FooObjectB> fooBSharedPtrPlain = fooBSharedPtr;
    weak_ptr<FooObjectB>   fooBWeakPtrPlain   = fooBSharedPtr;

    // 클래스 내부에서 자기 자신에 대한 스마트 포인터를 반환해야 하는 상황은 자주 발생한다.
    // - 어떠한 이유로 자기 자신을 받는 로직이 필요한 경우(자기 참조)
    // - 스레드 생성 시 자신의 인스턴스를 스마트 포인터로 전달해야 하는 경우
    // - 비동기 작업이나 콜백 등을 통해 객체가 다른 구성 요소와 협력해야 하는 경우
    // - 멀티스레딩 로직을 구성했을 시 자기 자신을 다른 구성 요소에 전달하는 경우
    // - 옵저버 패턴 같이 무언가 등록해야 하는 과정을 거치는 도중 자신을 전달할 필요가 있을 경우

    return 0;
}
