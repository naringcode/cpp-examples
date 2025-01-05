// Update Date : 2024-12-26
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <memory>

#include <type_traits>
#include <utility> // index_sequence

using namespace std;

// !! 자세한 내용은 shared_ptr_details.cpp에 적었다. !!
// !! 아래 코드는 간소화하여 생성한 메모리에 대해 Deleter를 적용하고, shared_ptr가 관리하는 메모리를 할당자로 생성하는 방법을 기술하였다. !!

// 순서대로 볼 것
// 
// # shared_ptr을 사용할 경우 알아야 할 기본적인 내용
// 1. shared_ptr_with_deleter.cpp
// 2. shared_ptr_with_allocator.cpp
// 3. shared_ptr_with_deleter_and_allocator.cpp <-----
// 4. (중요) shared_ptr_details.cpp (SFINAE 내용 포함)
// 
// # weak_ptr의 유효성 검증 로직에 대한 내용
// 5. weak_ptr.cpp
// 6. weak_ptr_details.cpp
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

public:
    virtual void Print() const
    {
        cout << "TestObject::Print()\n\n";
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

public:
    void Print() const override
    {
        cout << "TestObjectEx::Print()\n\n";
    }

private:
    double _valB = 0.0;
};

class FooObject
{
public:
    FooObject()
    {
        cout << "FooObject()\n";

        cout << std::hex << "\tPtr : 0x" << this << "\n\n";
        cout << std::dec;
    }

    ~FooObject()
    {
        cout << "~FooObject()\n";

        cout << std::hex << "\tPtr : 0x" << this << "\n\n";
        cout << std::dec;
    }

public:
    void Print() const
    {
        cout << "FooObject::Print()\n\n";
    }
};

/********************
*      Printer      *
********************/

template <typename... Args, size_t... Is>
void PrintArgs(std::index_sequence<Is...>, Args&&... args)
{
    // 폴드 표현식으로 인자 출력
    ((cout << "\t\tArg[" << Is << "] Type : " << typeid(args).name() << ", Value : " << args << '\n'), ...);
    cout << '\n';
}

/**********************
*      Allocator      *
**********************/

uint64_t g_IdCounter = 0;

// STL은 멤버 변수의 형태로 할당자를 보관하고, shared_ptr은 컨트롤 블록에 할당자를 묶어서 관리한다.
// !! 이렇게 하면 객체의 수명이 유지되는 동안 필요할 때마다 메모리를 할당 및 해제하는 것을 보장할 수 있음. !!
//
// 아래 할당자는 Visual Studio 2022 기준이며 버전이 달라지거나 사용하는 컴파일러가 바뀌면 요구사항이 달라질 수 있다.
template <class Type>
class MyAllocator
{
public:
    using value_type = Type;

public:
    MyAllocator() noexcept
    {
        _id = g_IdCounter++;

        cout << typeid(*this).name() << "\n";
        cout << "\t" << "Default Constructor [ Id : " << _id << " ]\n\n";
    }

    // 소멸자는 없어도 되지만 추적하기 편하게 추가함.
    ~MyAllocator() noexcept
    {
        cout << typeid(*this).name() << "\n";
        cout << "\t" << "Destructor [ Id : " << _id << " ]\n\n";
    }

    MyAllocator(const MyAllocator& rhs) noexcept
    {
        _id = g_IdCounter++;

        cout << typeid(*this).name() << "\n";
        cout << "\t" << "Copy Constructor [ Id : " << _id << " from " << rhs._id << " ]\n\n";
    }

    // 필수 인터페이스
    // 1. allocate_shared<Type>()를 쓸 경우
    // : _Alblock _Rebound(_Al); <<----- [ _Ref_count_obj_alloc3 ] 여기를 거치면서 호출됨.
    // 2. shared_ptr의 생성자의 유형이 Ptr, Deleter, Allocator를 받는 방식일 경우
    // _Alref_alloc _Alref(_Ax); <<---- [ _Ref_count_resource_alloc ] 여기를 거치면서 호출됨.
    // 
    // 왜 이런 인터페이스 필요한가 싶긴 하지만 Allocator 간 뭔가 연동하는 작업이 생길 걸 대비해서 마련한 장치라 생각함.
    // 특별한 이유가 없다면 해당 변환(?) 생성자의 몸체는 비어 있을 것임(컴파일을 위한 인터페이스만 제공하겠다는 의미)
    template <class Other>
    MyAllocator(const MyAllocator<Other>& rhs) noexcept
    {
        _id = g_IdCounter++;

        cout << typeid(*this).name() << "\n";
        cout << "\t" << "Converting Constructor [ Id : " << _id << " from " << rhs._id << " ]\n\n";
    }

public:
    Type* allocate(const size_t count)
    {
        const int32_t sizeBytes = (int32_t)(count * sizeof(Type));
        void* ptr = ::malloc(sizeBytes);

        cout << typeid(*this).name() << "\n";
        cout << "\t" << "Allocating... [ Id : " << _id << " ]\n";
        cout << std::hex << "\t\tPtr : 0x" << ptr;
        cout << std::dec << ", num of elements : " << count << ", size of bytes : " << sizeBytes << "\n\n";

        return static_cast<Type*>(ptr);
    }

    void deallocate(Type* const ptr, const size_t count)
    {
        const int32_t sizeBytes = (int32_t)(count * sizeof(Type));

        cout << typeid(*this).name() << "\n";
        cout << "\t" << "Deallocating... [ Id : " << _id << " ]\n";
        cout << std::hex << "\t\tPtr : 0x" << ptr;
        cout << std::dec << ", num of elements : " << count << ", size of bytes : " << sizeBytes << "\n\n";

        ::free(ptr);
    }

public: // 의도한 것
    uint64_t _id; // 어떤 할당자를 사용하는지 파악하기 위한 id
};

/*********************
*      Deleters      *
*********************/

template <typename Type>
void MyDeleter(Type* ptr)
{
    cout << "MyDeleter<Type>()\n" << "\tType: " << typeid(Type).name() << '\n';

    cout << std::hex << "\t\tPtr : 0x" << ptr << "\n\n";
    cout << std::dec;

    delete ptr;

    // 관리 객체의 메모리를 순수하게 날것 그대로 할당해서 관리하는 방식을 사용할 때는 소멸자를 직접 호출해야 한다.
    // !! 메모리 풀을 구현할 경우 다음 코드가 필요할 수 있음. !!
    // if constexpr (std::is_class_v<Type>)
    // {
    //     ptr->~Type();
    // }
    // 
    // ::free(ptr);
}

template <typename Type>
void MyDeleterWithAllocator(Type* ptr)
{
    cout << "MyDeleterWithAllocator<Type>()\n" << "\tType: " << typeid(Type).name() << '\n';

    cout << std::hex << "\t\tPtr : 0x" << ptr << "\n\n";
    cout << std::dec;

    // 여기서 사용하는 Allocator는 컨트롤 블록에 전달했던 것과는 다르게 봐야 한다.
    // !! 관리 객체를 직접 생성했을 때 사용한 Allocator로 반환한다고 봐야 함(혼동하지 말 것). !!
    MyAllocator<Type> allocator;
    allocator.deallocate(ptr, 1);
}

/*****************************************
*      Custom Make Shared Functions      *
*****************************************/

template <typename Type, typename... Args>
shared_ptr<Type> MyMakeSharedWithDeleterAllocator_MisleadingCaseA(Args&&... args)
{
    cout << "MyMakeSharedWithDeleterAllocator_MisleadingCaseA()\n" << "\tType : " << typeid(Type).name() << '\n';

    // 인자 출력
    PrintArgs(std::make_index_sequence<sizeof...(args)>(), args...);

    // 다음은 shared_ptr_with_allocator.cpp의 MyMakeSharedWithAllocator()에 달린 주석의 일부이다.
    // !! 생성한 객체 Ptr를 넘기면서, 직접 Deleter를 지정하며, 할당자를 제공하는 방식도 있지만 처음 보면 이해하기 어렵고 다소 난해함. !!
    // 
    // Ptr, Deleter, Allocator를 받는 방식을 쓰면 (Ptr, Deleter)와 Allocator를 따로 떼서 봐야 한다.
    // 
    // 여기서 Deleter는 관리 객체 Ptr를 삭제하는 방식이 기술된 Callable이며,
    // Allocator는 shared_ptr의 컨트롤 블록을 할당 및 해제할 때 사용하기 위한 할당자일 뿐이다(Allocator 자체는 Ptr나 Deleter와의 연관성이 없음).
    // !! (중요) 이 부분이 이해가 안 된다면 곰곰이 생각하고 이해한 다음 넘어갈 것. !!
    // 
    // Allocator의 allocate()로 Ptr를 할당하고, Deleter에서 Allocator의 deallocate()를 호출하여 Ptr을 반환해도 된다(안 되는 건 아님).
    // 
    // 틀린 사용 방법은 아니지만 원리를 모르거나 배경지식 없이 그런 코드를 마주하면 딱 오해하기 쉽다.
    // 특히 Deleter를 람다식으로 구성할 때 캡처 블록에 Allocator를 함께 묶어서 전달하면 혼란만 가중될 수 있다.
    // 
    // Ptr, Deleter, Allocator로 shared_ptr을 생성하는 유형에서 Ptr은 굳이 3번째 인자로 전달한 Allocator로 생성하지 않아도 된다.
    //
    // Ptr이 가리키는 관리 객체의 메모리는 new로 생성해도 되고, 메모리 풀로 확보해도 되고,
    // 3번째 할당자(MyAllocator)와는 다른 아예 별개 유형의 Allocator(AwesomeAllocator라든지)로 할당해서 제공해도 된다.
    // 관리 객체의 메모리 자체는 다양한 방식으로 확보할 수 있다.
    //
    // Deleter는 Ptr가 가리키는 메모리가 어떤 방식으로 확보되었는지 알 수 없으니 이걸 올바로 해제하기 위해 필요한 Callable일 뿐이다.
    //
    // !! (중요) Allocator는 순전히 shared_ptr에서 사용할 컨트롤 블록을 관리(할당 및 해제)하기 위해 필요한 것임. !!
    // !! (중요) Deleter는 Ptr가 생성된 방식에 맞춰서 해제 방식을 기술한 Callable일 뿐임. !!
    // !! (중요) Deleter는 전달한 메모리의 해제에 대한 책임을 사용자에게 위함하는 기능임. !!
    
    MyAllocator<Type> allocator;

    // Allocator는 순수하게 날것의 메모리만 할당하기 때문에 placement new를 통해 직접 생성자를 호출해야 함.
    Type* ptr = allocator.allocate(1);
    new (ptr) Type(std::forward<Args>(args)...); // placement new로 객체의 생성자 호출

    shared_ptr<Type> sptr{ ptr, 
                          // allocator 자체는 스택 메모리에 있기 때문에 참조로 받는 건 권장되지 않는 방식이다.
                          // allocator 클래스에 함수만 있고 멤버 변수가 없다면 참조로 받아도 되지만 여긴 스택 메모리다.
                          // !! 지킬 건 지키고 참조로 받아야 하는 상황이라면 명확한 이유를 반드시 기술해야 함. !!
                          [allocator](Type* ptrParam) mutable
                          {
                              // (중오) 람다식으로 구성한 Deleter와 shared_ptr에서 생성한 컨트롤 블록은 별개로 봐야 한다(연관성도 없으며 관계된 것도 없음).
                              // Deleter는 컨트롤 블록의 해제 여부를 담당하지 않는다.
                              // Deleter는 순전히 전달한 ptr이 가리키고 있는 관리 객체의 메모리를 해제하는 것만 담당한다.
                              cout << "# Begin - Lambda Deleter in MisleadingCaseA\n\n";

                              ptrParam->~Type();

                              allocator.deallocate(ptrParam, 1);

                              cout << "# End - Lambda Deleter in MisleadingCaseA\n\n";

                              // (중요) 전달한 ptr을 해제하는 것이 Deleter가 해야할 일의 전부이다.
                              // Deleter는 shared_ptr 차원에서 할당한 컨트롤 블록의 해제에는 일절 관여하지 않는다.
                              //
                              // 즉, 람다식을 통해 관리 객체의 메모리를 해제했다고 해도 아직 shared_ptr 차원에서 할당한 메모리의 해제 단계가 남아 있다.
                              // shared_ptr 차원에서 할당한 메모리를 해제할 때는 마찬가지로 3번째 인자로 넘긴 allocator의 deallocate()를 기반으로 한다.
                          },
                          // 3번째 인자로 전달되는 allocator는 내부에서 shared_ptr의 컨트롤 블록을 할당하기 위한 다른 형식의 allocator로 변환된다.
                          // 무슨 말이냐면 allocator가 관리 객체를 할당하는 형태로 기술되었다고 해도, 내부에서는 template을 통해 타입을 인지해 컨트롤 블록의 할당자로 변환 후 사용한다는 뜻이다.
                          // 이 부분은 디버깅해서 추적해도 어려운 내용이다(template의 사용법을 어느 정도 숙지해야 이해하기 수월함).
                          //
                          // allocator가 컨트롤 블록을 생성하기 위해 다른 유형의 allocator로 변환되어 사용된다고 해도,
                          // 전달하는 allocator의 template 유형은 ptr의 타입으로 맞춰야 한다.
                           allocator };

    // !! 람다 캡처 블록에 있는 allocator와 함수 안에 있는 allocator는 별개의 allocator이다. !!
    // !! 애초에 람다식은 컴파일되면 Functor(함수 객체)로 변환되며, 캡처 블록 안에 있는 allocator는 복사되어 Functor 안에 들어간 멤버 변수일 뿐이다. !!
    
    // 최대한 주석을 달긴 했지만 이것만 봐서는 굉장히 혼란스럽다.
    //
    // 이렇게 코드를 작성해도 안 되는 건 아니지만...
    // 이 코드는 "Allocator를 통해서 Ptr 생성하고, 람다식에 해당 Allocator를 전달해서 deallocate()를 호출해야 하고,
    // 마지막으로 shared_ptr에서 인지할 수 있게 사용한 Allocator를 넘겨야 하는구나!" 하고 오해하기 쉽다.
    // 
    // (중요) 혼동하기 쉬워서 계속 언급하지만 3번째 인자로 전달하는 Allocator는 shared_ptr에서 사용할 메모리(컨트롤 블록)을 할당하기 위한 할당자다.
    // (중요) 3번째 인자로 전달하는 할당자는 Ptr나 Deleter와는 별개로 생각해야 한다.
    // 
    // 다음 코드는 ptr에서 사용한 할당자 따로, Lambda 식 내에서 할당자 따로, 3번째 인자로 넘길 할당자를 따로 생성하고 있다.
    // 코드의 구성 방식은 다르지만 위에 있는 코드와 아래 있는 코드는 기능적으로 동일하다.
    // 
    // shared_ptr<Type> sptr{ ptr, 
    //                       [](Type* ptrParam) mutable
    //                       {
    //                           cout << "# Begin - Lambda Deleter in MisleadingCaseA\n\n";
    // 
    //                           ptrParam->~Type();
    // 
    //                           MyAllocator<Type> lambdaAllocator;
    //                           lambdaAllocator.deallocate(ptrParam, 1);
    // 
    //                           cout << "# End - Lambda Deleter in MisleadingCaseA\n\n";
    //                       },
    //                        MyAllocator<Type>{ } };

    return sptr;
}

template <typename Type, typename... Args>
shared_ptr<Type> MyMakeSharedWithDeleterAllocator_MisleadingCaseB(Args&&... args)
{
    cout << "MyMakeSharedWithDeleterAllocator_MisleadingCaseB()\n" << "\tType : " << typeid(Type).name() << '\n';

    // 인자 출력
    PrintArgs(std::make_index_sequence<sizeof...(args)>(), args...);

    // 다음은 MisleadingCaseA의 또 다른 형태의 코드이다.
    // 
    // 이전에는 Deleter를 람다로 구성하여 캡처 블록을 통해 Allocator를 전달했다면,
    // 이번에는 함수 포인터를 지정하여 함수 내에서 Allocator를 생성하는 방식을 취하고 있다.
    // 
    // 마찬가지로 안 되는 코드는 아니지만 처음 보면 오해하기 쉬운 코드이다.
    
    MyAllocator<Type> allocator;

    // Allocator는 순수하게 날것의 메모리만 할당하기 때문에 placement new를 통해 직접 생성자를 호출해야 함.
    Type* ptr = allocator.allocate(1);
    new (ptr) Type(std::forward<Args>(args)...); // placement new로 객체의 생성자 호출

    // 함수 포인터를 전달하는 방식
    shared_ptr<Type> sptr{ ptr, &MyDeleterWithAllocator<Type>, allocator };

    return sptr;
}

template <typename Type, typename... Args>
shared_ptr<Type> MyMakeSharedWithDeleterAllocator_FormalCaseA(Args&&... args)
{
    cout << "MyMakeSharedWithDeleterAllocator_FormalCaseA()\n" << "\tType : " << typeid(Type).name() << '\n';

    // 인자 출력
    PrintArgs(std::make_index_sequence<sizeof...(args)>(), args...);

    // 다음 코드는 Ptr, Deleter, Allocator를 전달하는 유형의 일반적인 사용 방법이다.
    // Type* ptr = new Type(std::forward<Args>(args)...);

    Type* ptr = static_cast<Type*>(::malloc(sizeof(Type)));

    cout << "Allocating by new...\n" << "\tType : " << typeid(Type).name() << '\n';
    cout << std::hex << "\t\tPtr : 0x" << ptr << "\n\n";
    cout << std::dec;

    new (ptr) Type(std::forward<Args>(args)...); // placement new로 객체의 생성자 호출

    //
    auto deleter = [](Type* ptrParam) {
        cout << "# Begin - Lambda Deleter in FormalCaseA\n\n";
        
        delete ptrParam;

        cout << "# End - Lambda Deleter in FormalCaseA\n\n";
    };

    return shared_ptr<Type>{ ptr, deleter, MyAllocator<Type>{ } };

    // !! 이게 가장 정석적인 사용 방법이자 해당 유형의 생성자를 쓸 경우에 대한 모범 코드이다. !!
    //
    // Allocator는 순전히 shared_ptr에서 사용할 메모리(컨트롤 블록)를 관리하기 위한 할당자일 뿐이다.
    // Ptr과 Deleter는 서로 연관성이 있지만, Allocator는 이 둘과 연관성이 없기에 이를 구분해서 볼 수 있어야 한다.
    // 
    // (중요) Deleter를 보면 Allocator를 사용하고 있지 않다.
    // !! Deleter는 전달한 메모리의 해제에 대한 책임을 사용자에게 위함하는 기능이라는 사실을 반드시 기억해야 함. !!
    //
    // 이를 통해서 맞춤형으로 메모리를 생성하여 전달하고, 맞춤형으로 메모리를 해제할 수 있다.
    // 전용 메모리 풀을 만들어 고성능 프로그램을 만들거나, 메모리 할당 사항이나 어떤 객체가 해제되지 않았는지를 모니터링하고자 할 때 필요한 기능이 모두 들어있는 유형이다.
    // 즉, 이해하기 가장 어려운 유형이어도 고급 메모리 관리 기능을 적용하고자 할 때 가장 적합한 방식이다.
    //
    // 정석적인 이 코드를 보고 MisleadingCaseA와 MisleadingCaseB를 보면 헷갈림의 원인이 되는 부분이 어디인지 쉽게 파악할 수 있다.
    // 이 코드를 통해 동작 방식을 이해했으면 MisleadingCaseA에 작성한 상세한 주석도 이해가 될 것이다.
    //
    // (중요) 절대 Allocator를 Ptr나 Deleter와 연관짓지 말자(아예 별개로 볼 것).
}

template <typename Type, typename... Args>
shared_ptr<Type> MyMakeSharedWithDeleterAllocator_FormalCaseB(Args&&... args)
{
    cout << "MyMakeSharedWithDeleterAllocator_FormalCaseB()\n" << "\tType : " << typeid(Type).name() << '\n';

    // 인자 출력
    PrintArgs(std::make_index_sequence<sizeof...(args)>(), args...);

    // 정석적인 사용 방법을 쓸 때 Callable에 람다식 대신 함수 포인터를 적용하는 것도 가능하다.
    // Type* ptr = new Type(std::forward<Args>(args)...);

    Type* ptr = static_cast<Type*>(::malloc(sizeof(Type)));

    cout << "Allocating by new...\n" << "\tType : " << typeid(Type).name() << '\n';
    cout << std::hex << "\t\tPtr : 0x" << ptr << "\n\n";
    cout << std::dec;

    new (ptr) Type(std::forward<Args>(args)...); // placement new로 객체의 생성자 호출

    return shared_ptr<Type>{ ptr, &MyDeleter<Type>, MyAllocator<Type>{ } };

    // !! 예시 코드에선 이 방식으로 shared_ptr을 생성할 것임. !!
}
template <typename Type, typename... Args>
shared_ptr<Type> MyMakeSharedWithDeleterAllocator_DontUse(Args&&... args)
{
    cout << "MyMakeSharedWithDeleterAllocator_DontUse()\n" << "\tType : " << typeid(Type).name() << '\n';

    // 인자 출력
    PrintArgs(std::make_index_sequence<sizeof...(args)>(), args...);

    // !! 이런 방식으로는 쓰지 말 것을 당부함. !!
    // !! 스마트 포인터 차원에서 런타임 에러를 뱉지도 않으니 나중에 소멸 단계에서 문제가 발생할 수 있는 구조의 코드임. !!

    // (nullptr, Deleter, Allocator)를 인자로 넘겨서 생성하는 것도 가능하다.
    //
    // 관리 객체가 nullptr이라도 컨트롤 블록은 할당된다.
    // 즉, 관리 객체가 nullptr이라도 컨트롤 블록을 통해서 소멸 과정을 거치기 때문에 이를 검증하는 로직이 필요하다.
    return shared_ptr<Type>{ nullptr, 
                            [](Type* ptrParam) mutable
                            {
                                cout << "# Begin - Lambda Deleter in FormalCaseA\n\n";

                                if (nullptr == ptrParam)
                                {
                                    cout << "ptrParam is nullptr!\n\n";
                                }
                                else
                                {
                                    cout << "ptrParam is not nullptr!\n\n";
                                }

                                cout << "# End - Lambda Deleter in FormalCaseA\n\n";
                            }, MyAllocator<Type>{ } };
}

/*****************
*      Main      *
*****************/

int main()
{
    // !! Visual Studio 2022 기준이며 아래 분석 내용은 컴파일러 버전에 따라 달라질 수 있음. !!

    cout << "-------------------------#01#-------------------------\n\n";

    // Ptr, Deleter, Allocator를 생성자의 인자로 넘겨 스마트 포인터를 생성하는 방식
    // MisleadingCaseA
    {
        // RVO에 의해 대상 메모리 공간에 직접적으로 Ptr, Deleter, Allocator를 받는 생성자가 적용됨.
        shared_ptr<TestObjectEx> objEx = MyMakeSharedWithDeleterAllocator_MisleadingCaseA<TestObjectEx>(100, 3.141592);

        cout << "## End Of Block ##\n\n";

        // End 이후 shared_ptr의 소멸자를 거쳐 관리 객체의 소멸자가 호출되어야 하며,
        // 최종적으로는 shared_ptr의 컨트롤 블록의 메모리가 반환되어야 함.
    }

    g_IdCounter = 0;

    cout << "-------------------------#02#-------------------------\n\n";

    // Ptr, Deleter, Allocator를 생성자의 인자로 넘겨 스마트 포인터를 생성하는 방식
    // MisleadingCaseB
    {
        // RVO에 의해 대상 메모리 공간에 직접적으로 Ptr, Deleter, Allocator를 받는 생성자가 적용됨.
        shared_ptr<TestObjectEx> objEx = MyMakeSharedWithDeleterAllocator_MisleadingCaseB<TestObjectEx>(100, 3.141592);

        cout << "## End Of Block ##\n\n";

        // End 이후 shared_ptr의 소멸자를 거쳐 관리 객체의 소멸자가 호출되어야 하며,
        // 최종적으로는 shared_ptr의 컨트롤 블록의 메모리가 반환되어야 함.
    }

    g_IdCounter = 0;

    cout << "-------------------------#03#-------------------------\n\n";

    // Ptr, Deleter, Allocator를 생성자의 인자로 넘겨 스마트 포인터를 생성하는 방식
    // FormalCaseA
    {
        // RVO에 의해 대상 메모리 공간에 직접적으로 Ptr, Deleter, Allocator를 받는 생성자가 적용됨.
        shared_ptr<TestObjectEx> objEx = MyMakeSharedWithDeleterAllocator_FormalCaseA<TestObjectEx>(100, 3.141592);

        cout << "## End Of Block ##\n\n";

        // End 이후 shared_ptr의 소멸자를 거쳐 관리 객체의 소멸자가 호출되어야 하며,
        // 최종적으로는 shared_ptr의 컨트롤 블록의 메모리가 반환되어야 함.
    }

    g_IdCounter = 0;

    cout << "-------------------------#04#-------------------------\n\n";

    // Ptr, Deleter, Allocator를 생성자의 인자로 넘겨 스마트 포인터를 생성하는 방식
    // FormalCaseB
    {
        // RVO에 의해 대상 메모리 공간에 직접적으로 Ptr, Deleter, Allocator를 받는 생성자가 적용됨.
        shared_ptr<TestObjectEx> objEx = MyMakeSharedWithDeleterAllocator_FormalCaseB<TestObjectEx>(100, 3.141592);

        cout << "## End Of Block ##\n\n";

        // End 이후 shared_ptr의 소멸자를 거쳐 관리 객체의 소멸자가 호출되어야 하며,
        // 최종적으로는 shared_ptr의 컨트롤 블록의 메모리가 반환되어야 함.
    }

    g_IdCounter = 0;

    cout << "-------------------------#05#-------------------------\n\n";

    // nullptr(!!), Deleter, Allocator를 생성자의 인자로 넘겨 스마트 포인터를 생성하는 방식
    // DontUse
    {
        // RVO에 의해 대상 메모리 공간에 직접적으로 Ptr, Deleter, Allocator를 받는 생성자가 적용됨.
        shared_ptr<TestObjectEx> objEx = MyMakeSharedWithDeleterAllocator_DontUse<TestObjectEx>(100, 3.141592);

        cout << "## End Of Block ##\n\n";

        // End 이후 shared_ptr의 소멸자를 거쳐 관리 객체의 소멸자가 호출되어야 하며,
        // 최종적으로는 shared_ptr의 컨트롤 블록의 메모리가 반환되어야 함.
    }

    g_IdCounter = 0;

    cout << "-------------------------#06#-------------------------\n\n";

    // 사용 도중에 nullptr을 대입했을 경우, nullptr을 가리키는 스마트 포인터가 Deleter나 deallocate()를 호출하는지 확인하기 위한 코드
    {
        // RVO에 의해 대상 메모리 공간에 직접적으로 Ptr, Deleter, Allocator를 받는 생성자가 적용됨.
        shared_ptr<TestObjectEx> objEx1 = MyMakeSharedWithDeleterAllocator_FormalCaseB<TestObjectEx>(100, 3.141592);

        {
            // 복사 생성자
            shared_ptr<TestObjectEx> objEx2 = objEx1;

            // shared_ptr(nullptr_t) 생성자로 임시 객체 생성 후, 이를 objEx1에 반영하기 위한 이동 대입 연산자가 호출됨.
            objEx1 = nullptr; // nullptr은 암시적으로 shared_ptr{ nullptr }로 변환됨.

            cout << "assign nullptr...\n\n";

            // 해당 블록을 빠져나오면 shared_ptr의 소멸자를 거치며 관리 객체의 소멸자를 호출함.
        }

        cout << "## End Of Block ##\n\n";

        // End Of Block 이후 전달한 Deleter나 할당자의 deallocate()가 호출되진 않음.
        // 사용 도중 스마트 포인터에 nullptr을 대입해도 문제 없음.
    }

    g_IdCounter = 0;

    cout << "-------------------------#07#-------------------------\n\n";

    // 업 캐스팅하여 스마트 포인터를 사용할 경우 어떤 소멸자가 호출되는지 관찰하기 위한 코드
    {
        shared_ptr<TestObject> obj;

        {
            // RVO에 의해 대상 메모리 공간에 직접적으로 Ptr, Deleter, Allocator를 받는 생성자가 적용됨.
            shared_ptr<TestObjectEx> objEx = MyMakeSharedWithDeleterAllocator_FormalCaseB<TestObjectEx>(300UL, 2.718281f);

            // 인자로 넘긴 objEx의 관리 객체의 주소를 캐스팅하여 shared_ptr<TestObject>를 생성하고
            // 이를 obj에 반영하기 위한 이동 대입 연산자가 호출됨.
            obj = static_pointer_cast<TestObject>(objEx);
            
            // 부모로 업 캐스팅하는 것이기에 다음 코드도 가능함.
            // obj = objEx;
        }

        cout << "after static_pointer_cast...\n\n";

        obj->Print();

        cout << "## End Of Block ##\n\n";
    }
    
    cout << "-------------------------#08#-------------------------\n\n";
    
    // static_pointer_cast<T1, T2>()를 이용한 다운 캐스팅
    {
        // 업 캐스팅으로 받는다.
        shared_ptr<TestObject> sptr1 = MyMakeSharedWithDeleterAllocator_FormalCaseB<TestObjectEx>(123, 3.14);

        sptr1->Print();

        // TEST
        // sptr1 = nullptr;

        // static_pointer_cast<T1, T2>()를 활용한 다운 캐스팅 진행
        shared_ptr<TestObjectEx> sptr2 = static_pointer_cast<TestObjectEx>(sptr1);

        cout << "after static_pointer_cast...\n\n";

        // sptr1이 빈 스마트 포인터였다면 sptr2도 빈 스마트 포인터가 된다.
        if (nullptr != sptr2)
        {
            sptr2->Print();
        }
        else
        {
            cout << "sptr2 is empty...\n\n";
        }

        // 생성자나 대입 연산자로 다운 캐스팅을 하는 것은 불가능하다.
        // 항상 적절한 캐스팅 함수를 먼저 적용한 다음에 결과를 다른 스마트 포인터에 반영해야 한다.
        // 이 부분은 static_pointer_cast<T1, T2>()든 dynamic_pointer_cast<T1, T2>()든 동일한 내용이다.
        // shared_ptr<TestObjectEx> sptr3 = sptr1;

        // 다음 코드는 컴파일 단계에서 에러가 발생한다.
        // shared_ptr<FooObject> sptr4 = static_pointer_cast<FooObject>(sptr1);

        cout << "## End Of Block ##\n\n";
    }

    cout << "-------------------------#09#-------------------------\n\n";

    // dynamic_pointer_cast<T1, T2>()를 이용한 다운 캐스팅
    {
        // 업 캐스팅으로 받는다.
        shared_ptr<TestObject> sptr1 = MyMakeSharedWithDeleterAllocator_FormalCaseB<TestObjectEx>(123, 3.14);

        sptr1->Print();

        // TEST
        // sptr1 = nullptr;
        
        // dynamic_pointer_cast<T1, T2>()를 활용한 다운 캐스팅 진행
        shared_ptr<TestObjectEx> sptr2 = dynamic_pointer_cast<TestObjectEx>(sptr1);

        // sptr1이 빈 스마트 포인터였다면 sptr2도 빈 스마트 포인터가 된다.
        if (nullptr != sptr2)
        {
            sptr2->Print();
        }
        else
        {
            cout << "sptr2 is empty...\n\n";
        }

        // 정의되지 않은 상속 관계로 캐스팅을 시도
        shared_ptr<FooObject> sptr3 = dynamic_pointer_cast<FooObject>(sptr1);
        
        if (nullptr != sptr3)
        {
            sptr3->Print();
        }
        else
        {
            cout << "sptr3 is empty...\n\n";
        }

        cout << "## End Of Block ##\n\n";
    }

    cout << "--------------------------------------------------\n";

    return 0;
}
