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

// 순서대로 볼 것
// 
// # shared_ptr을 사용할 경우 알아야 할 기본적인 내용
// 1. shared_ptr_with_deleter.cpp
// 2. shared_ptr_with_allocator.cpp
// 3. shared_ptr_with_deleter_and_allocator.cpp
// 4. (중요) shared_ptr_details.cpp (SFINAE 내용 포함) <-----
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
        cout << "\tDefault Constructor [ Id : " << _id << " ]\n\n";
    }

    // 소멸자는 없어도 되지만 추적하기 편하게 추가함.
    ~MyAllocator() noexcept
    {
        cout << typeid(*this).name() << "\n";
        cout << "\tDestructor [ Id : " << _id << " ]\n\n";
    }

    MyAllocator(const MyAllocator& rhs) noexcept
    {
        _id = g_IdCounter++;

        cout << typeid(*this).name() << "\n";
        cout << "\tCopy Constructor [ Id : " << _id << " from " << rhs._id << " ]\n\n";
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
        cout << "\tConverting Constructor [ Id : " << _id << " from " << rhs._id << " ]\n\n";
    }

public:
    // 할당 함수와 해제 함수는 생성자나 소멸자를 호출하지 않는다.
    // allocate()와 deallocate()은 순수하게 메모리만 관리하기 위한 함수이다.
    // 생성자와 소멸자를 호출하는 건 할당자를 활용하는 쪽이다.
    Type* allocate(const size_t count)
    {
        const int32_t sizeBytes = (int32_t)(count * sizeof(Type));
        void* ptr = ::malloc(sizeBytes);

        cout << typeid(*this).name() << "\n";
        cout << "\tAllocating... [ Id : " << _id << " ]\n";
        cout << "\t\tType : " << typeid(Type).name() << '\n';
        cout << std::hex << "\t\t\tPtr : 0x" << ptr;
        cout << std::dec << ", num of elements : " << count << ", size of bytes : " << sizeBytes << "\n\n";

        return static_cast<Type*>(ptr);
    }

    void deallocate(Type* const ptr, const size_t count)
    {
        const int32_t sizeBytes = (int32_t)(count * sizeof(Type));

        cout << typeid(*this).name() << "\n";
        cout << "\tDeallocating... [ Id : " << _id << " ]\n";
        cout << "\t\tType : " << typeid(Type).name() << '\n';
        cout << std::hex << "\t\t\tPtr : 0x" << ptr;
        cout << std::dec << ", num of elements : " << count << ", size of bytes : " << sizeBytes << "\n\n";

        ::free(ptr);
    }

public: // 의도한 것
    uint64_t _id; // 어떤 할당자를 사용하는지 파악하기 위한 id
};

template <class Type>
class SimpleAllocator
{
public:
    using value_type = Type;

public:
    SimpleAllocator() noexcept { }

    // 필수 인터페이스
    template <class Other>
    SimpleAllocator(const SimpleAllocator<Other>& rhs) noexcept { }

public:
    Type* allocate(const size_t count)
    {
        const int32_t sizeBytes = (int32_t)(count * sizeof(Type));
        void* ptr = ::malloc(sizeBytes);

        cout << typeid(*this).name() << "\n";
        cout << "\tAllocating...\n";
        cout << "\t\tType : " << typeid(Type).name() << '\n';
        cout << std::hex << "\t\t\tPtr : 0x" << ptr;
        cout << std::dec << ", num of elements : " << count << ", size of bytes : " << sizeBytes << "\n\n";

        return static_cast<Type*>(ptr);
    }

    void deallocate(Type* const ptr, const size_t count)
    {
        const int32_t sizeBytes = (int32_t)(count * sizeof(Type));

        cout << typeid(*this).name() << "\n";
        cout << "\tDeallocating...\n";
        cout << "\t\tType : " << typeid(Type).name() << '\n';
        cout << std::hex << "\t\t\tPtr : 0x" << ptr;
        cout << std::dec << ", num of elements : " << count << ", size of bytes : " << sizeBytes << "\n\n";

        ::free(ptr);
    }
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

/*****************************************
*      Custom Make Shared Functions      *
*****************************************/

template <typename Type, typename... Args>
shared_ptr<Type> MyMakeSharedWithDeleter(Args&&... args)
{
    cout << "MyMakeSharedWithDeleter()\n" << "\tType : " << typeid(Type).name() << '\n';

    // 폴드 표현식으로 인자 출력(인자의 번호까지는 출력할 수 없음(내가 방법을 모르는 것일 수도))
    // ((cout << "\tArg Type : " << typeid(args).name() << ", Value : " << args << '\n'), ...);

    // sizeof...()은 가변인자 템플릿의 인자 개수를 구하기 위해서 사용한 것임(인자 팩을 확장하는 것이 아니니까 혼동하지 말 것).
    // 완벽한 전달의 의미를 생각하면 인자는 복사 방식으로 전달하는 것이 좋음(std::forward()로 전달하면 new Type{ std::forward<Args>(args)... }에서 문제가 생길 수 있음).
    PrintArgs(std::make_index_sequence<sizeof...(args)>(), args...); // 전달을 우회할 순 있으나 너무 설명적인 코드가 되니 생략함(쉽게 쉽게 가자).

    // --------------------------------------------------

    // 이런저런 코드를 테스트해봤을 때 std::forward() 말고 std::forward<Args>()를 써야 하는 경우가 있었음(경험적인 부분).

    // 생성자에 인자를 넣을 때 중괄호 "{}" 안에 넣는 것이 내 스타일의 코드지만 템플릿의 가변 타입까지 범위를 확대하면 에러가 났을 때 찾기가 매우 힘들었음.
    // !! 생성자의 인자를 중괄호 안에 넣어서 처리하면 암시적 형변환을 허용하지 않음(중괄호 안에 넣긴 했으나 이건 리스트 초기화가 아님). !!
    // Type* ptr = new Type{ std::forward<Args>(args)... };

    // 템플릿의 가변 타입을 생성자의 인자로 사용할 때는 암시적인 형변환을 허용하는 방식을 쓰도록 함.
    // Type* ptr = new Type(std::forward<Args>(args)...);
    
    // 어떤 Deleter를 호출할 것인지 미리 지정하고 shared_ptr을 생성한다.
    // return shared_ptr<Type>{ ptr, &MyDeleter<Type> };

    // 관리 객체의 메모리를 순수하게 날것 그대로 할당해서 관리하는 방식을 사용할 때는 placement new로 생성자를 직접 호출해야 한다.
    Type* ptr = static_cast<Type*>(::malloc(sizeof(Type)));
    
    cout << "Allocating by new...\n" << "\tType : " << typeid(Type).name() << '\n';
    cout << std::hex << "\t\tPtr : 0x" << ptr << "\n\n";
    cout << std::dec;

    new (ptr) Type(std::forward<Args>(args)...); // placement new로 객체의 생성자 호출
    
    return shared_ptr<Type>{ ptr, &MyDeleter<Type> };

    // 다음과 같이 람다식을 구성해서 Deleter를 전달하는 것도 가능하다.
    // 
    // 다만 람다식은 컴파일 과정에서 Functor로 변환되고, 이를 생성하는 비용이 있기 때문에
    // 람다식을 통한 Deleter 전달은 단순 함수 포인터를 전달하는 방식보다 약간 오버헤드가 발생할 수 있다(그 차이는 매우 미미하긴 함).
    // 
    // !! 추가적인 오버헤드는 매우 미미하고, 요즘은 컴파일러가 알아서 최적화를 잘 해주기 때문에 큰 문제가 되는 부분은 아님. !!
    // return shared_ptr<Type>{ ptr, 
    //                         [](Type* ptrParam)
    //                         {
    //                             cout << "# Begin - Lambda Deleter\n\n";
    // 
    //                             delete ptrParam;
    // 
    //                             cout << "# End - Lambda Deleter\n\n";
    //                         } };
}

template <typename Type, typename... Args>
shared_ptr<Type> MyMakeSharedWithAllocator(Args&&... args)
{
    cout << "MyMakeSharedWithAllocator()\n" << "\tType : " << typeid(Type).name() << '\n';

    // 인자 출력
    PrintArgs(std::make_index_sequence<sizeof...(args)>(), args...);

    // Allocator를 넘기는 방식 1
    // return std::allocate_shared<Type>(MyAllocator<Type>{ }, std::forward<Args>(args)...);

    // Allocator를 넘기는 방식 2
    MyAllocator<Type> allocator; // 할당자 자체는 다른 곳에서 생성해도 됨(TLS라든지 메모리 풀이라든지).

    // Deleter와 Allocator만 받는 유형은 찾지 못 했음(Allocator는 컨트롤 블록의 생성과 해제를 담당함).
    // 애초에 Deleter는 유저가 전달한 관리 객체의 메모리를 해제하는 방식과 관련되어 있음(Deleter는 컨트롤 블록과 연관성이 없음).
    // 
    // 찾아보니 할당 함수를 넘기는 방식은 없고 이렇게 생성한 할당자를 직접 지정해야 함.
    return std::allocate_shared<Type>(allocator, std::forward<Args>(args)...);

    // !! 생성한 객체 Ptr를 넘기면서, 직접 Deleter를 지정하며, 할당자를 제공하는 방식도 있지만 처음 보면 이해하기 어렵고 다소 난해함. !!
    // !! 용어 정리에도 적었지만 해당 유형의 생성자를 이해하려면 넘긴 관리 객체의 메모리와 shared_ptr에서 관리하는 컨트롤 블록을 구분해서 볼 수 있어야 함. !!
    // !! 실제로는 별거 없지만 배경지식 없이 인터페이스만 보고 기능을 유추하려 하면 혼란스러울 수 있음(보통은 이렇게까지 깊게 들어가지 않음). !!
    //
    // !! Ptr과 Deleter를 지정하지 않았고, 할당자만 넘기는 방식을 취했으면? !!
    // !! 이는 컨트롤 블록의 메모리 뿐만 아니라, 스마트 포인터 차원에서 할당자를 통해 관리 객체의 메모리를 생성 및 해제하여 관리하겠다는 뜻이 됨. !!
    //
    // !! 관리 객체의 메모리를 넘기면서 Deleter를 지정하는 방식은 넘긴 관리 객체의 소멸 여부를 직접 제어하겠다는 의미로 봐야 함. !!
    // !! 이를 할당자와 연관지으면 안 됨. !!
    //
    // !! Ptr, Deleter, Allocator를 모두 받을 경우 할당자는 shared_ptr의 컨트롤 블록의 메모리를 직접 관리하기 위해서 사용하는 것임. !!
    // !! Allocator의 allocate()와 Ptr은 연관성이 없으며, 마찬가지로 Allocator의 deallocate()와 Deleter도 연관성이 없음(혼동하기 쉬움). !!
    // !! 이 유형에서 Ptr와 Deleter는 서로 연관성이 있지만, Allocator는 Ptr나 Deleter와 연관성이 없음(혼동하기 쉬워서 2번 강조). !!
    //
    // Deleter를 사용하지 않고 넘긴 할당자를 통해 관리 객체를 생성하고자 할 때, Deleter를 통해 넣으려고 했던 기능이 있다면 할당자의 deallocate() 쪽에 추가해야 한다.
    // 이 기능은 객체의 소멸자 호출 여부를 말하는 것이 아닌 프로파일링 기능이나 메모리 풀 최적화와 같은 그런 부가적인 기능을 말하는 것이다.
    //
    // 물론 Deleter와 할당자의 deallocate() 양쪽 모든 곳에 적용하려고 했던 기능을 제공해도 된다.
}

template <typename Type, typename... Args>
shared_ptr<Type> MyMakeSharedWithDeleterAllocator(Args&&... args)
{
    cout << "MyMakeSharedWithDeleterAllocator()\n" << "\tType : " << typeid(Type).name() << '\n';

    // 인자 출력
    PrintArgs(std::make_index_sequence<sizeof...(args)>(), args...);

    // !! Deleter와 Allocator를 함께 사용하는 유형에 대한 상세한 설명은 shared_ptr_with_deleter_and_allocator.cpp에 기술함. !!
    // !! Deleter와 Allocator를 함께 사용하는 유형에 대한 상세한 설명은 shared_ptr_with_deleter_and_allocator.cpp에 기술함. !!
    // !! Deleter와 Allocator를 함께 사용하는 유형에 대한 상세한 설명은 shared_ptr_with_deleter_and_allocator.cpp에 기술함. !!
    // !! Deleter와 Allocator를 함께 사용하는 유형에 대한 상세한 설명은 shared_ptr_with_deleter_and_allocator.cpp에 기술함. !!
    
    // 여기선 이해하기 쉬운 가장 정석적인 방법을 사용할 것이다.
    //
    // (중요) Allocator는 Ptr나 Deleter와 연관되어 있는 부분이 없지만, Ptr과 Deleter는 서로 연관성이 있다.
    // (중요) Allocator는 shared_ptr 내에서 사용할 메모리(컨트롤 블록)를 관리하기 위한 할당자일 뿐이며, Ptr와는 관련이 없다고 봐야 한다.
    // 
    // Ptr이 가리키는 관리 객체의 메모리는 new로 생성해도 되고, 메모리 풀로 확보해도 되고,
    // 3번째 할당자와는 다른 아예 별개 유형의 Allocator(AwesomeAllocator라든지)로 할당해서 제공해도 된다.
    // 관리 객체의 메모리 자체는 다양한 방식으로 확보할 수 있다.
    // 
    // Deleter는 Ptr가 가리키는 메모리가 어떤 방식으로 확보되었는지 알 수 없으니 이걸 올바로 해제하기 위해 필요한 Callable일 뿐이다.
    //
    // !! (중요) Allocator는 순전히 shared_ptr에서 사용할 컨트롤 블록을 관리(할당 및 해제)하기 위해 필요한 것임. !!
    // !! (중요) Deleter는 Ptr가 생성된 방식에 맞춰서 해제 방식을 기술한 Callable일 뿐임. !!

    // Ptr은 Allocator로 생성하지 않아도 된다.
    // Type* ptr = new Type(std::forward<Args>(args)...);

    Type* ptr = static_cast<Type*>(::malloc(sizeof(Type)));

    cout << "Allocating by new...\n" << "\tType : " << typeid(Type).name() << '\n';
    cout << std::hex << "\t\tPtr : 0x" << ptr << "\n\n";
    cout << std::dec;

    new (ptr) Type(std::forward<Args>(args)...); // placement new로 객체의 생성자 호출

    // 3번째 인자로 전달되는 allocator는 내부에서 shared_ptr의 컨트롤 블록을 할당하기 위한 다른 형식의 allocator로 변환된다.
    // 무슨 말이냐면 allocator가 관리 객체를 할당하는 형태로 기술되었다고 해도, 내부에서는 template을 통해 타입을 인지해 컨트롤 블록의 할당자로 변환 후 사용한다는 뜻이다.
    // 이 부분은 디버깅해서 추적해도 어려운 내용이다(template의 사용법을 어느 정도 숙지해야 이해하기 수월함).
    //
    // allocator가 컨트롤 블록을 생성하기 위해 다른 유형의 allocator로 변환되어 사용된다고 해도,
    // 전달하는 allocator의 template 유형은 ptr의 타입으로 맞춰야 한다.
    return shared_ptr<Type>{ ptr, &MyDeleter<Type>, MyAllocator<Type>{ } };

    // (Ptr, Deleter, Allocator)를 모두 받는 유형은 이해하기 가장 어려운 방식이다.
    // 하지만 이를 통해서 맞춤형으로 메모리를 생성하여 전달하고, 맞춤형으로 메모리를 해제할 수 있다.
    // 전용 메모리 풀을 만들어 고성능 프로그램을 만들거나, 메모리 할당 사항이나 어떤 객체가 해제되지 않았는지를 모니터링하고자 할 때 필요한 기능이 모두 들어있는 유형이다.
    // 즉, 이해하기 가장 어려운 유형이어도 고급 메모리 관리 기능을 적용하고자 할 때 가장 적합한 방식이다.
}

template <typename Type, typename... Args>
shared_ptr<Type> MyMakeSharedWithDeleterAllocator_Simple(Args&&... args)
{
    cout << "MyMakeSharedWithDeleterAllocator_Simple()\n" << "\tType : " << typeid(Type).name() << '\n';

    // 인자 출력
    PrintArgs(std::make_index_sequence<sizeof...(args)>(), args...);

    // ptr 생성
    Type* ptr = static_cast<Type*>(::malloc(sizeof(Type)));

    cout << "Allocating by new...\n" << "\tType : " << typeid(Type).name() << '\n';
    cout << std::hex << "\t\tPtr : 0x" << ptr << "\n\n";
    cout << std::dec;

    new (ptr) Type(std::forward<Args>(args)...); // placement new로 객체의 생성자 호출

    // 분석하기 편하게 부가적인 기능은 다 뺀 심플한 Allocator를 사용함.
    return shared_ptr<Type>{ ptr, &MyDeleter<Type>, SimpleAllocator<Type>{ } };
}

/*****************
*      Main      *
*****************/

int main() 
{
    // !! Visual Studio 2022 + x64 빌드 기준이며 아래 분석 내용은 컴파일러 환경에 따라 달라질 수 있음. !!

    // !! 사전 지식(매우 중요 매우 중요 매우 중요 | 꼭 읽고 넘어가기) !!
    // 
    // shared_ptr<T>와 weak_ptr<T>는 _Ptr_base<T>를 이용하는데 해당 자료형은 다음과 같은 형태로 구성되어 있다(unique_ptr은 해당하지 않음).
    // 
    // template <class _Ty>
    // class _Ptr_base { // base class for shared_ptr and weak_ptr
    // public:
    //     using element_type = remove_extent_t<_Ty>;
    //     ...
    // 
    // private:
    //     element_type* _Ptr{ nullptr };
    //     _Ref_count_base* _Rep{ nullptr };
    //     ...
    // };
    // 
    // shared_ptr -> _Ptr_base<T> : [ element_type* _Ptr ][ _Ref_count_base* _Rep ]
    // weak_ptr -> _Ptr_base<T> : [ element_type* _Ptr ][ _Ref_count_base* _Rep ]
    // - _Ptr : 접근해서 사용하고자 하는 관리 객체의 메모리를 지정한 포인터
    // - _Rep : 컨트롤 블록(shared_ptr가 실질적으로 사용하는 메모리 블록)
    // 
    // 
    // 스마트 포인터 구현의 핵심이 되는 건 _Rep(컨트롤 블록)이며 shared_ptr와 weak_ptr의 기능 대부분은 이걸 기반으로 동작한다.
    // 
    // !! __declspec(novtable)이 적용되었어도 _Ref_count_base에는 가상 함수 테이블의 크기가 포함됨. !!
    // class __declspec(novtable) _Ref_count_base { // common code for reference counting
    // private:
    //     virtual void _Destroy() noexcept     = 0; // destroy managed resource
    //     virtual void _Delete_this() noexcept = 0; // destroy self
    // 
    //     _Atomic_counter_t _Uses  = 1; // 4바이트, strong ref(s)
    //     _Atomic_counter_t _Weaks = 1; // 4바이트, weak ref(s)
    //     ...
    // };
    // 
    // _Ref_count_base는 2개의 레퍼런스 카운터 _Uses와 _Weaks를 멤버 변수로 가지고 있다.
    // 또한 _Uses가 0으로 떨어졌을 때 호출될 _Destroy()와 _Weaks가 0으로 떨어졌을 때 호출될 _Delete_this()도 들고 있다.
    // - _Destroy() : 관리 객체의 수명이 다 했을 때 호출되는 함수(_Uses가 0으로 떨어졌을 때 호출)
    // - _Delete_this() : 컨트롤 블록의 수명이 다 했을 때 호출되는 함수(_Uses와 _Weaks가 0으로 떨어졌을 때 호출)
    // 
    // _Destroy()는 관리 객체의 소멸 과정을 유도하고, _Delete_this()는 컨트롤 블록의 소멸 과정을 유도한다.
    // 직접적으로 해제한다고 하지 않고 소멸 과정을 유도한다고 쓴 이유가 있다.
    // !! _Destroy()와 _Delete_this()는 순수 가상 함수이기 때문에 기능이 파생 클래스에 의존적임. !!
    // 
    // _Destroy()는 대상 Type의 소멸자만 호출할 수도 있고, Deleter를 경유해서 삭제하게 할 수도 있고, 실질적으로 전달한 메모리를 해제할 수도 있다.
    // _Delete_this() 또한 스마트 포인터 차원에서 할당한 컨트롤 블록의 메모리를 직접 해제할 수도 있고 재사용하는 방식을 취할 수도 있다.
    // 
    // (중요) _Ref_count_base는 순수 가상 함수가 적용된 클래스이기 때문에, 이걸 상속하여 구현한 파생 클래스 기반으로 동작한다.
    // (중요) _Ref_count_base는 파생 클래스를 위한 인터페이스를 제공하기 위한 클래스라고 봐야 한다.
    // 
    // 파생 클래스의 종류는 다양하며, 어떤 파생 클래스를 사용할 것인지에 따라 스마트 포인터가 관리하는 컨트롤 블록의 특성이 달라진다.
    // 
    // !! 여기서 말하는 컨트롤 블록의 특성은 shared_ptr나 weak_ptr의 동작 특성을 말하는 것이 아니다. !!
    // !! make_shared<T>()를 통해서 생성했는지, 직접 할당한 메모리를 인자로 넘겨서 생성했는지, !!
    // !! Deleter를 직접 구성하여 넘기는 방식으로 생성했는지, 직접 작성한 할당자를 적용한 방식인지를 말하는 것이다. !!
    // 
    // --------------------------------------------------
    // 
    // _Ref_count_base를 상속하여 구현한 파생 클래스의 종류는 굉장히 다양하다.
    // Visual Studio 2022 기준 Microsoft에서 제공하는 memory 헤더만 봐도 다음 파생 클래스들이 있다(대괄호 안에 있는 건 상속 구역에 있는 멤버 변수).
    //
    // # _Ref_count<_Ty>[ _Ty* _Ptr ]
    // : 사용자가 생성한 관리 객체의 메모리를 가리킨 _Ptr을 전달하여 스마트 포인터를 생성할 때 사용함.
    // : new나 malloc()으로 할당한 관리 객체의 생명주기를 스마트 포인터에서 관리함(사용자 쪽에서 delete를 사용하지 않아도 됨).
    // : 컨트롤 블록은 사용자가 넘긴 관리 객체의 포인터를 관리함.
    // : _Ref_count 컨트롤 블록은 new로 할당됨.
    // 
    // # _Ref_count_resource<_Resource, _Dx>[ _Compressed_pair<_Dx, _Resource> _Mypair ]
    // : 사용자가 관리 객체(_Ptr)를 생성하여 전달하고, 이를 해제하기 위한 Deleter를 지정할 때 사용함.
    // : 관리 객체의 수명이 다 한 시점에 Deleter를 호출하는 방식임.
    // : 컨트롤 블록은 사용자가 넘긴 관리 객체의 포인터를 관리함.
    // : _Ref_count_resource 컨트롤 블록은 new로 할당됨.
    // 
    // # _Ref_count_resource_alloc<_Resource, _Dx, _Alloc>[ _Compressed_pair<_Dx, _Compressed_pair<_Myalty, _Resource>> _Mypair ]
    // : 생성한 _Ptr을 전달하고, 이를 해제할 방식이 기술된 Deleter도 전달하며, 스마트 포인터에서 사용할 컨트롤 블록을 생성하기 위한 할당자까지 전달하는 방식일 경우 사용함.
    // : 마찬가지로 관리 객체의 수명이 다 한 시점에 Deleter를 호출하는 방식임.
    // : 컨트롤 블록은 사용자가 넘긴 관리 객체의 포인터를 관리함.
    // : !! _Ref_count_resource_alloc 컨트롤 블록은 할당자를 통해서 생성함. !!
    // 
    // # _Ref_count_obj2<_Ty>[ _Wrap<remove_cv_t<_Ty>> _Storage ]
    // : make_shared<T>() 기반으로 스마트 포인터를 생성할 경우 사용되는 컨트롤 블록임.
    // : _Ref_count_obj2 컨트롤 블록은 new로 할당됨.
    // : !! 컨트롤 블록을 할당할 때 관리 객체의 값에 해당하는 메모리(_Storage)도 묶어서 함께 생성하기에 동적할당은 한 번만 이루어짐. !!
    // : !! 관리 객체는 컨트롤 블록(_Rep)의 상속 구역으로부터 가져와 사용함. !!
    // 
    // # _Ref_count_obj_alloc3<_Ty, _Alloc>[ _Ebco_base(다중 상속), _Wrap<_Ty> _Storage ]
    // : 할당자를 지정하여 스마트 포인터를 생성하는 방식일 경우 사용함.
    // : !! _Ref_count_obj_alloc3 컨트롤 블록은 할당자를 통해서 생성함. !!
    // : !! 컨트롤 블록을 할당할 때 관리 객체의 값에 해당하는 메모리(_Storage)도 묶어서 함께 생성하기에 동적할당은 한 번만 이루어짐. !!
    // : !! 관리 객체는 컨트롤 블록(_Rep)의 상속 구역으로부터 가져와 사용함. !!
    // 
    // # _Ref_count_unbounded_array<_Ty, bool = is_trivially_destructible_v<remove_extent_t<_Ty>>>[ _Wrap<remove_cv_t<_Element_type>> _Storage ]
    // # _Ref_count_unbounded_array<_Ty, false>[ size_t _Size, _Wrap<remove_cv_t<_Element_type>> _Storage ]
    // # _Ref_count_bounded_array<_Ty>[ _Wrap<remove_cv_t<_Ty>> _Storage ]
    // # _Ref_count_unbounded_array_alloc<_Ty, _Alloc>[ _Ebco_base(다중 상속), size_t _Size, _Wrap<_Element_type> _Storage ]
    // # _Ref_count_bounded_array_alloc<_Ty, _Alloc> : [ _Ebco_base(다중 상속), _Wrap<_Ty> _Storage ]
    // 
    // --------------------------------------------------
    // 
    // shared_ptr<T> sptr = make_shared<T>()로 sptr을 만들고 메모리에서 "sptr._Rep"를 조회하면 다음과 같은 컨트롤 블록이 생성된다는 것을 확인할 수 있다.
    // [ 가상 함수 테이블(8바이트) ][ _Uses(4바이트) ][ _Weaks(4바이트) ][ 상속 구역(파생 클래스의 형태에 따라 다르게 구성되는 블록) ]
    // 
    // 쉽게 말해서 _Rep의 주소를 알면 상속 구역에 접근해서 관리 객체의 영역에 접근할 수 있다는 뜻이다.
    // 관리 객체는 Type 값 그 자체가 되었든 Type*처럼 포인터로 받든 일단 _Rep의 상속 구역에 묶여 있는 형태가 된다.
    // !! make_shared<T>()로 할당하면 관리 객체는 값(T)으로 할당되지만, Ptr을 직접 넘기는 유형이라면 포인터(T*)로 할당됨. !!
    // 
    // 컨트롤 블록에 관리 객체의 값까지 묶어서 한 번에 할당하면,
    // 스마트 포인터는 사용할 관리 객체의 포인터를 _Rep의 상속 구역으로부터 가져올 수 있다.
    // !! 외부에서 생성 후 넘기는 방식을 쓰면 스마트 포인터 차원에서 관리하기 위한 컨트롤 블록의 할당까지 합해서 총 2번 동적할당이 일어남. !!
    // 
    // 이런 유형으로 동작하는 _Ref_count_base의 파생 클래스는 동적할당 한 번으로 컨트롤 블록과 관리 객체까지 한 번에 할당할 수 있으니 효율적이다.
    // 이는 컨트롤 블록의 유형에 따라 갈리는 부분이며, 관리 객체를 따로 생성해서 넘기고 컨트롤 블록을 따로 생성해서 관리하는 유형도 있음.
    // !! 동작하는 방식을 알면 동적할당을 한 번만 진행하는 유형이 더 효율적이라는 것을 파악할 수 있음. !!
    // 
    // 값 자체가 되었든 포인터가 되었든 컨트롤 블록의 상속 구역에는 관리 객체를 포함하여, 컨트롤 블록이 이를 쉽게 알 수 있게 하는 것이 좋다.
    // 파생 컨트롤 블록 차원에서 직접 관리 객체를 인지할 수 있으면 관리 객체의 소멸 과정을 편하게 유도할 수 있다.
    // !! Visual Studio의 C++에서 제공하는 컨트롤 블록을 분석하면 CPU 시간을 줄이기 위해 메모리를 약간 희생하는 구조로 되어 있음. !!
    // 
    // --------------------------------------------------
    // 
    // !! _Ref_count_base 기반 컨트롤 블록의 형태 !!
    // [ 가상 함수 테이블(8바이트) ][ _Uses(4바이트) ][ _Weaks(4바이트) ][ 상속 구역(파생 클래스의 형태에 따라 다르게 구성되는 블록) ]
    // 
    // _Rep가 가리키는 대상은 위와 같이 메모리를 구성한다고 했다. 
    // 실제로도 그런지 확인해 봐야 한다.
    // 
    // struct Test
    // {
    //     uint64_t valA = 0x1111'1111'1111'1111;
    //     uint64_t valB = 0x2222'2222'2222'2222;
    // };
    //
    // int main()
    // {
    //     shared_ptr<Test> sptr = make_shared<Test>();
    //     auto temp = sptr; // strong ref 1 증가
    //
    //     return 0;
    // }
    //
    // "return 0;" 라인에 중단점을 걸고 메모리에서 "sptr._Rep"를 조회하면 레퍼런스 카운팅 블록과 Test의 멤버 변수 값이 들어간 것을 확인할 수 있다.
    //
    // 0x000002B799433860  [10 bd f5 8d f6 7f 00 00]    // 가상 함수 테이블(std::_Ref_count_obj2<Test>::`vftable')
    // 0x000002B799433868  [02 00 00 00] [01 00 00 00]  // strong refs : 2, weak ref : 1
    // 0x000002B799433870  [11 11 11 11 11 11 11 11]    // Test의 valA | 상속 구역
    // 0x000002B799433878  [22 22 22 22 22 22 22 22]    // Test의 valB | 상속 구역
    // 
    // !! _Rep의 구성 방식을 알면 상속 구역에 접근하여 관리 객체의 영역에 접근할 수 있다. !!
    // !! make_shared<T>()는 관리 객체와 컨트롤 블록을 한 덩어리로 묶어서 생성하기 때문에 그냥 값 자체를 들여다 볼 수 있다. !!
    // 
    // _Rep에 _Storage가 묶인 형태의 컨트롤 블록이라면 이렇게 직접적으로 값을 들여다 보는 것이 가능하다.
    // _Rep에 묶인 것이 저장소가 아닌 단순 포인터 값이라면 메모리를 본 후 한 번 더 타고 들어가서 조회해야 관리 객체의 값을 볼 수 있다.
    // 
    // 조사식을 통해서 "&(sptr._Rep->__vfptr)"를 조회하면 "sptr._Rep"와 포인터 값이 같은 것을 확인할 수 있다.
    // 
    // 또한 조사식으로 "&(sptr._Rep->__vfptr) + 2"와 "&(sptr._Rep->__vfptr) + 3"을 조회하면
    // 각 포인터가 가리키고 있는 대상의 값이 0x1111111111111111과 0x2222222222222222라는 것도 확인할 수 있다.
    // 
    // !! 조사식에서 "&(sptr._Rep->__vfptr) + 2"과 "sptr._Ptr"를 조회하면 포인터 주소 또한 같은 것을 파악할 수 있다. !!
    // !! (중요) 스마트 포인터에서 관리하는 _Ptr은 컨트롤 블록 _Rep에 묶여 있다는 뜻이다. !!
    // 
    // --------------------------------------------------
    // 
    // (중요) _Ref_count_base가 컨트롤 블록의 기반 클래스인 만큼 한 번 더 정리한다.
    // 
    // _Uses는 사용하고 있는 관리 객체의 유효성을 검증하기 위한 카운터이고,
    // _Weaks는 스마트 포인터 차원에서 할당한 컨트롤 블록(레피런스 카운팅 블록)의 유효성을 검증하기 위한 카운터이다.
    // 
    // _Uses가 0으로 떨어지면 관리 객체는 더 이상 유효하지 않은 상태가 되니 _Destroy()를 통해 관리 객체의 소멸 과정에 들어간다.
    // !! 소멸 과정이라는 애매한 표현을 쓴 이유는 관리 객체의 소멸자만 호출할 수도 있고, Deleter를 호출하여 관리 객체를 전달할 수도 있고, !!
    // !! 실질적으로 관리 객체의 메모리까지 해제할 수도 있기 때문임(스마트 포인터가 생성되었을 당시의 유형에 따라 갈리는 부분). !!
    // 
    // (매우 중요) _Uses가 0으로 떨어져 호출되는 _Destroy()는 관리 객체에만 영향을 미칠 뿐, 이 행위가 스마트 포인터의 컨트롤 블록을 해제하겠다는 것을 의미하는 건 아니다.
    // 
    // _Weaks가 0으로 떨어져야만 _Delete_this()를 통해 컨트롤 블록을 해제한다.
    // _Uses가 유효하다면 _Weaks는 항상 유효하기 때문에 _Delete_this()가 _Destroy()보다 먼저 호출될 일은 없다.
    // 즉, _Uses와 _Weaks가 둘 다 0으로 떨어져야만 shared_ptr 차원에서 할당한 메모리 블록을 해제한다는 뜻이다.
    // !! 컨트롤 블록의 유효성을 관리 객체의 유효성보다 우선시 한다는 뜻임. !!
    // 
    // 최초 shared_ptr의 컨트롤 블록을 할당하면 _Uses와 _Weaks의 값은 1로 초기화된다.
    // 이후 shared_ptr 변수 자체를 shared_ptr에 복사하면 _Uses의 값이 1 증가하고, weak_ptr에 복사하면 _Weaks의 값이 1 증가한다.
    // 이 과정에서 컨트롤 블록의 포인터와 관리 객체의 포인터가 복사 대상이 되는 스마트 포인터에 복사된다.
    // 
    // 관리 객체가 더 이상 유효하지 않은 상태가 되면 _Weaks의 값을 감소시켜 되면 컨트롤 블록의 유효성도 검증한다.
    // 
    // 1. 소멸자 호출
    // shared_ptr<T>::~shared_ptr() noexcept { // release resource
    //     this->_Decref(); // 관리 객체의 유효성 검증
    // }
    // 
    // 2. _Ptr_base<T>의 _Decref() 호출
    // void _Ptr_base<T>::_Decref() noexcept { // decrement reference count
    //     // 연동된 메모리 블록의 것으로 호출
    //     if (_Rep) {
    //         _Rep->_Decref(); // _Ptr_base<T>의 _Decref()를 경유해서 _Ref_count_base의 _Decref()를 호출
    //     }
    // }
    // 
    // 3. _Ref_count_base의 _Decref() 호출
    // void _Ref_count_base::_Decref() noexcept { // decrement use count
    //     if (_MT_DECR(_Uses) == 0) { // 원자적인 감소(_InterlockedDecrement())
    //         _Destroy(); // 순수 가상 함수 | 관리 객체의 소멸 과정 유도
    //         _Decwref(); // 관리 객체가 유효하지 않게 되면 _Weaks의 값을 줄여서 컨트롤 블록의 유효성을 검증
    //     }
    // }
    // 
    // 4. _Ref_count_base의 _Decwref() 호출
    // void _Ref_count_base::_Decwref() noexcept { // decrement weak reference count
    //     if (_MT_DECR(_Weaks) == 0) { // 원자적인 감소(_InterlockedDecrement())
    //         _Delete_this(); // 순수 가상 함수 | 컨트롤 블록의 소멸 과정 유도(어디서 weak_ptr로 컨트롤 블록을 참조하고 있는 게 아니라면 컨트롤 블록도 해제)
    //     }
    // }
    // 
    // !! _Destroy()와 _Delete_this()는 순수 가상 함수이기에 파생 클래스에서 구현한 것을 호출함. !!
    // 
    // _Uses가 0으로 떨어지면 _Destroy()를 호출하고, _Weaks가 0으로 떨어지면 _Delete_this()를 호출한다.
    // 이 두 함수는 컨트롤 블록 베이스(_Ref_count_base)의 순수 가상 함수인 만큼 파생 클래스의 구현에 의존적이다.
    // 
    // _Uses가 0으로 떨어져야 _Weaks를 감소시키기에, _Uses가 유효하다면(관리 객체가 유효하다면) _Weaks는 항상 유효한 것이다.
    // !! (중요) 컨트롤 블록의 수명은 관리 객체의 수명보다 길어야 함. !!
    // 
    // --------------------------------------------------
    // 
    // shared_ptr의 소멸자가 호출되면 다음 로직을 수행한다.
    // 
    // 1. _Uses를 1 감소시킨다.
    // 2. _Uses 값이 0으로 떨어졌는가?
    // -> 3. 그렇다면 _Destroy()를 호출하여 관리 객체의 소멸 과정을 유도한다(컨트롤 블록의 해제는 아직 아님).
    // -> 4. 이어서 _Weaks의 값을 1 감소시킨다.
    // -> 5. _Weaks의 값이 0으로 떨어졌는가?
    //    -> 6. 그렇다면 컨트롤 블록을 해제한다(스마트 포인터에서 관리하는 컨트롤 블록의 해제는 여기서 일어남).
    //
    // 만약 어디에서 weak_ptr로 스마트 포인터를 참조하고 있는 상태라면 _Weaks는 0으로 떨어지지 않는다.
    // 이 경우 관리 객체가 소멸되었다고 해도 shared_ptr를 구성했을 때 할당한 컨트롤 블록은 계속 유효할 것이다.
    // 
    // --------------------------------------------------
    // 
    // weak_ptr의 소멸자가 호출되면 다음 로직을 수행한다.
    // 
    // 1. _Weaks를 1 감소시킨다.
    // 2. _Weaks 값이 0으로 떨어졌는가?
    // -> 3. 그렇다면 컨트롤 블록을 해제한다.
    // 
    // 관리 객체의 수명이 다 했다고 해도 weak_ptr로 스마트 포인터를 참조하고 있는 상태라면 컨트롤 블록은 계속 유효한 상태이다.
    // 
    // --------------------------------------------------
    // 
    // !! (중요) 메모리 블록의 레퍼런스 카운트 _Uses가 0으로 떨어지기 전까지 _Weaks의 값은 항상 유효하다는 사실을 기억해야 한다. !!
    // 
    // shared_ptr나 weak_ptr는 레퍼런스 카운팅을 위한 메커니즘을 감싼 껍데기(?)일 뿐이다.
    // 실질적으로 레퍼런스 카운트를 조작하는 건 스마트 포인터 안에 있는 컨트롤 블록(레퍼런스 카운팅 블록)인 _Rep이다.
    // !! 컨트롤 블록은 _Ref_count_base를 상속하여 구현한 파생 클래스임. !!
    // 
    // 중요한 개념이기에 정리해서 한 번 더 보면...
    // 
    // shared_ptr -> _Ptr_base<T> : [ element_type* _Ptr ][ _Ref_count_base* _Rep ]
    // weak_ptr -> _Ptr_base<T> : [ element_type* _Ptr ][ _Ref_count_base* _Rep ]
    // - _Ptr : 접근해서 사용하고자 하는 관리 객체의 메모리를 지정한 포인터
    // - _Rep : 컨트롤 블록(스마트 포인터가 실질적으로 사용 및 관리하는 메모리 블록)
    // 
    // template <class _Ty>
    // class _Ptr_base { // base class for shared_ptr and weak_ptr
    // public:
    //     using element_type = remove_extent_t<_Ty>;
    //     ...
    // 
    // private:
    //     element_type* _Ptr{ nullptr };
    //     _Ref_count_base* _Rep{ nullptr };
    //     ...
    // };
    // 
    // 1. 소멸자 호출
    // shared_ptr<T>::~shared_ptr() noexcept { // release resource
    //     this->_Decref(); // 관리 객체의 유효성 검증
    // }
    // 
    // 2. _Ptr_base<T>의 _Decref() 호출
    // void _Ptr_base<T>::_Decref() noexcept { // decrement reference count
    //     // 연동된 메모리 블록의 것으로 호출
    //     if (_Rep) {
    //         _Rep->_Decref(); // _Ptr_base<T>의 _Decref()를 경유해서 _Ref_count_base의 _Decref()를 호출
    //     }
    // }
    // 
    // 3. _Ref_count_base의 _Decref() 호출
    // void _Ref_count_base::_Decref() noexcept { // decrement use count
    //     if (_MT_DECR(_Uses) == 0) { // 원자적인 감소(_InterlockedDecrement())
    //         _Destroy(); // 순수 가상 함수 | 관리 객체의 소멸 과정 유도
    //         _Decwref(); // 관리 객체의 유효하지 않게 되면 _Weaks의 값을 줄여서 컨트롤 블록의 유효성을 검증
    //     }
    // }
    // 
    // 4. _Ref_count_base의 _Decwref() 호출
    // void _Ref_count_base::_Decwref() noexcept { // decrement weak reference count
    //     if (_MT_DECR(_Weaks) == 0) { // 원자적인 감소(_InterlockedDecrement())
    //         _Delete_this(); // 순수 가상 함수 | 컨트롤 블록의 소멸 과정 유도(어디서 weak_ptr로 컨트롤 블록을 참조하고 있는 게 아니라면 컨트롤 블록도 해제)
    //     }
    // }
    // 
    // !! _Destroy()와 _Delete_this()는 순수 가상 함수이기에 파생 클래스에서 구현한 것을 호출함. !!
    // 
    // _Uses가 0으로 떨어지면 _Destroy()를 호출하고, _Weaks가 0으로 떨어지면 _Delete_this()를 호출한다.
    // 이 두 함수는 컨트롤 블록 베이스(_Ref_count_base)의 순수 가상 함수인 만큼 파생 클래스의 구현에 의존적이다.
    // 
    // !! Visual Studio 2022에서의 C++이 제공하는 스마트 포인터 구현의 핵심은 _Ref_count_base를 상속하여 구현한 파생 클래스에 있다. !!
    // !! 어떤 파생 클래스를 적용하냐에 따라서 _Destroy()와 _Delete_this()를 호출하기 위해 사용할 가상 함수 테이블이 달라진다. !!
    // 
    // (중요) 파생 클래스에서 관리 객체의 소멸 과정을 담당하는 _Destroy()는 제각기 구현되어 있기에 분석하고자 한다면 이 부분을 유심히 봐야 한다.
    // (중요) 컨트롤 블록의 형태에 따라 관리 객체의 소멸자만 호출할 수도 있고, Deleter를 경유할 수도 있고, 실질적인 메모리 해제까지 담당해서 처리할 수도 있다.
    // 
    // _Destroy()의 역할과 _Delete_this()의 역할은 다르니 기능을 반드시 구분해서 볼 수 있어야 한다.
    // 
    // --------------------------------------------------
    // 
    // 컨트롤 블록과 레퍼런스 카운팅 블록을 찾아보면 이를 동일한 개념으로 보는 경우가 많지만,
    // 엄밀히 말하자면 컨트롤 블록이 레퍼런스 카운팅을 포함하는 상위 개념이다.
    // !! 이에 대한 자세한 최상단 용어 설명에 적어 두었음. !!
    //
    // Visual Studio의 Inspector에서 _Ref_count_base를 조회하면 [control block]이라고 되어 있다.
    // 클래스 이름은 레퍼런스 카운팅을 할 것처럼 생겼지만 조회해서 보면 컨트롤 블록인 것이다.
    // !! _Ref_count_base의 파생 클래스 유형을 보면 단순 레퍼런스 카운팅만 하는 것이 아닌 Deleter와 Allocator를 받는 유형도 있으니 컨트롤 블록이 더 적합한 명칭이긴 함. !!
    //
    // --------------------------------------------------
    // 
    // (주의) _Uses와 _Weaks의 레퍼런스 카운팅 자체는 원자적으로 진행되지만 스마트 포인터의 생성 및 갱신은 스레드 안전하지 않다.
    //
    // (Return) ConstructorOrAssignment(SmartPointer& _Other)
    // {
    //     // 원자적으로 레퍼런스 카운팅이 이루어진다고 해도
    //     _Other._Incref();
    // 
    //     // 생성 및 갱신 자체는 스레드 안전하지 않음. 
    //     _Ptr = _Other._Ptr;
    //     // <----- 여기서 _Other의 _Ptr과 _Rep가 갱신되었으면?
    //     _Rep = _Other._Rep;
    // }
    //
    // 스마트 포인터의 생성 및 갱신 과정에서 스레드 경합이 일어난다면 문제가 발생할 수 있다.
    // 이에 대한 자세한 내용은 smart_pointer_multi_threading_issues.cpp를 보도록 한다.
    //
    // _Uses와 _Weaks가 0으로 떨어지는 시점은 단 한 순간이기 때문에 이 부분은 스레드 안전하다고 봐도 된다.
    // 스레드 안전하지 않은 건 레퍼런스 카운팅 쪽이 아니라 _Ptr과 _Rep의 갱신이 원자적으로 이루어지지 않는 부분에서 온다.
    //

    cout << "-------------------------#01#-------------------------\n\n";

    /***********************************************
    *      Basic Usage(std::make_shared<T>())      *
    ***********************************************/

    // std::make_shared<T>()로 스마트 포인터를 생성하는 방식
    {
        shared_ptr<TestObjectEx> objEx = make_shared<TestObjectEx>(123, 1.2345678);
        
        cout << "## End Of Block ##\n\n";

        // std::make_shared<T>() 기반이면 컨트롤 블록 _Ref_count_obj2<T>를 사용한다.
        //
        // template <class _Ty>
        // class _Ref_count_obj2 : public _Ref_count_base { // handle reference counting for object in control block, no allocator
        //     ...
        //     union {
        //         _Wrap<remove_cv_t<_Ty>> _Storage; // 관리 객체가 저장소의 형태로 컨트롤 블록에 묶이는 개념
        //     };
        // };
        // 
        // template <class _Ty>
        // struct _Wrap {
        //     _Ty _Value; // workaround for VSO-586813 "T^ is not allowed in a union"
        // };
        // 
        // [ 가상 함수 테이블(8바이트) ][ _Uses(4바이트) ][ _Weaks(4바이트) ][ _Storage(sizeof(T) 바이트) ]
        // 
        // _Ref_count_obj2<T>는 _Ref_count_base의 파생 클래스이며,
        // 컨트롤 블록 차원에서 관리 객체의 메모리(저장소)까지 한 번에 묶어서 관리한다.
        // 
        // 스마트 포인터에 지정되는 _Ptr은 _Ref_count_obj2<T>의 저장소(_Storage)이며,
        // 스마트 포인터가 사용하는 컨트롤 블록 _Rep는 _Ref_count_obj2<T>가 된다.
        // 
        // !! 관리 객체는 따로 동적할당되는 것이 아닌 컨트롤 블록의 상속 구역에 있는 저장소의 주소를 가져와 지정되는 방식임. !!
        // 
        // !! (중요) _Rep는 직접적으로 레퍼런스 카운팅을 수행하는 대상임(컨트롤 블록은 레퍼런스 카운팅의 기능을 겸함). !!
        //
        // 스마트 포인터의 생성자 안에서 컨트롤 블록을 할당하는 것이 아닌,
        // 컨트롤 블록을 먼저 할당한 다음 스마트 포인터의 생성자의 인자로 _Ptr과 _Rep를 넘겨서 지정하는 방식이다.
        // 
        // --------------------------------------------------
        //
        // 1. make_shared<TestObjectEx>() 호출
        // 
        // template <class _Ty, class... _Types>
        // enable_if_t<!is_array_v<_Ty>, shared_ptr<_Ty>> make_shared(_Types&&... _Args)
        // {
        //     // 컨트롤 블록 생성(관리 객체의 저장소까지 묶어서 한 번에 할당)
        //     const auto _Rx = new _Ref_count_obj2<_Ty>(_STD forward<_Types>(_Args)...);
        //
        //     // 일단 빈 스마트 포인터로 생성
        //     shared_ptr<_Ty> _Ret;
        // 
        //     // 컨트롤 블록 안에 있는 저장소의 주소를 관리 객체로 지정하며, 생성한 컨트롤 블록을 넘겨서 스마트 포인터를 초기화
        //     _Ret._Set_ptr_rep_and_enable_shared(_STD addressof(_Rx->_Storage._Value), _Rx);
        //
        //     // 반환
        //     return _Ret;
        // }
        // 
        // - enable_if_t<!is_array_v<_Ty>, shared_ptr<_Ty>> : 배열 타입이 아닐 경우 shared_ptr<_Ty>를 사용함.
        //
        // --------------------------------------------------
        // 
        // 2. _Ref_count_obj2<TestObjectEx> 생성자 호출(컨트롤 블록 생성)
        // 
        // template <class _Ty>
        // class _Ref_count_obj2 : public _Ref_count_base { // handle reference counting for object in control block, no allocator
        // public:
        //     template <class... _Types>
        //     explicit _Ref_count_obj2(_Types&&... _Args) : _Ref_count_base()
        //     {
        //         // !! 컴파일 타임에서 해당 부분은 실행되지 않게 걸러짐. !!
        //         if constexpr (sizeof...(_Types) == 1 && (is_same_v<_For_overwrite_tag, remove_cvref_t<_Types>> && ...))
        //         {
        //             _STD _Default_construct_in_place(_Storage._Value);
        //             ((void)_Args, ...);
        //         }
        //         else
        //         {
        //             // !! 이쪽 로직이 실행됨. !! //
        //             // 저장소의 메모리는 확보되었으나 생성자는 호출되지 않은 상태
        //             // 확보한 메모리에 적용되는 생성자를 직접적으로 호출
        //             _STD _Construct_in_place(_Storage._Value, _STD forward<_Types>(_Args)...);
        //         }
        //     }
        //     ...
        //     union {
        //         // _Storage는 _Ref_count_obj2<_Ty>에 묶여 있는 형태임(여기서는 TestObjectEx).
        //         // union으로 묶인 객체는 기본적으로 생성자를 호출하지 않음(테스트 해본 내용).
        //         // 즉, 생성자는 호출하지 않고 _Ty에 해당하는 타입의 메모리만 확보한 것임.
        //         _Wrap<remove_cv_t<_Ty>> _Storage;
        //     };
        // };
        // 
        // --------------------------------------------------
        // 
        // 3. _Construct_in_place() 호출
        // 
        // _Ref_count_obj2<T>의 생성자는 관리 객체의 저장소를 _Construct_in_place()로 초기화하며, 
        // 해당 함수는 내부에서 placement new를 사용한다.
        // 
        // template <class _Ty, class... _Types>
        // _CONSTEXPR20 void _Construct_in_place(_Ty& _Obj, _Types&&... _Args) noexcept(
        //     is_nothrow_constructible_v<_Ty, _Types...>)
        // {
        //     // !! is_constant_evaluated()는 constexpr 함수라서 최적화가 적용되면 해당 부분은 실행되지 않게 걸러짐. !!
        //     if (_STD is_constant_evaluated())
        //     {
        //         _STD construct_at(_STD addressof(_Obj), _STD forward<_Types>(_Args)...);
        //     }
        //     else
        //     {
        //         // !! 이쪽 로직이 실행됨. !! //
        //         // 템플릿으로 _Storage._Value의 타입을 인지해서 placement new로 생성자를 호출하여 초기화
        //         //
        //         // _Obj는 _Ty&의 형태로 전달되기에 저장소의 레퍼런스라고 보면 된다(_Storage._Value).
        //         // _Args는 완벽한 전달로 받은 가변 인자이다.
        //         //
        //         // 아래 코드를 쉽게 풀어쓰면 이런 느낌이다.
        //         // new (storage address) _Ty(arguments...)
        //         ::new (static_cast<void*>(_STD addressof(_Obj))) _Ty(_STD forward<_Types>(_Args)...);
        //     }
        // }
        // 
        // new _Ref_count_obj2<_Ty>(...)로 컨트롤 블록을 할당하면 관리 객체 _Ty는 저장소의 형태로 union에 묶인다.
        // !! 관리 객체는 새롭게 동적할당하는 것이 아닌 _Ref_count_obj2<_Ty>를 생성할 때 묶여서 딸려 오는 것임. !!
        // 
        // union으로 묶이면 객체의 생성자는 호출되지 않는다.
        // 따라서 직접 placement new를 통해 저장소의 메모리를 대상으로 생성자를 직접 호출하고 있는 형태로 되어 있는 것이다.
        // 
        // --------------------------------------------------
        // 
        // make_shared<TestObjectEx>()의 몸체에 도달할 때까지 콜 스택을 빠져 나오고...
        // 
        // 4. _Ret._Set_ptr_rep_and_enable_shared() 호출
        // 
        // shared_ptr<TestObjectEx> _Ret;
        // _Ret._Set_ptr_rep_and_enable_shared(_STD addressof(_Rx->_Storage._Value), _Rx);
        // 
        // template <class _Ux>
        // void shared_ptr<_Ty>::_Set_ptr_rep_and_enable_shared(_Ux* const _Px, _Ref_count_base* const _Rx) noexcept // take ownership of _Px
        // {
        //     this->_Ptr = _Px; // 관리 객체(_Ref_count_obj2<T>의 저장소 주소)
        //     this->_Rep = _Rx; // 컨트롤 블록(_Ref_count_obj2<T>(저장소가 묶인 형태))
        // 
        //     // !! 컴파일 타임에서 해당 부분은 실행되지 않게 걸러짐. !!
        //     if constexpr (conjunction_v<negation<is_array<_Ty>>, negation<is_volatile<_Ux>>, _Can_enable_shared<_Ux>>) {
        //         if (_Px && _Px->_Wptr.expired()) {
        //             _Px->_Wptr = shared_ptr<remove_cv_t<_Ux>>(*this, const_cast<remove_cv_t<_Ux>*>(_Px));
        //         }
        //     }
        // }
        //
        // --------------------------------------------------
        // 
        // make_shared<TestObjectEx>()의 몸체에 도달할 때까지 콜 스택을 빠져 나오고...
        // 
        // 5. 스마트 포인터를 반환(RVO에 의해 복사나 이동 연산자는 적용되지 않음)
        // 
        // template <class _Ty, class... _Types>
        // enable_if_t<!is_array_v<_Ty>, shared_ptr<_Ty>> make_shared(_Types&&... _Args)
        // {
        //     // 컨트롤 블록 생성(관리 객체의 저장소까지 묶어서 한 번에 할당)
        //     const auto _Rx = new _Ref_count_obj2<_Ty>(_STD forward<_Types>(_Args)...);
        //     shared_ptr<_Ty> _Ret;
        // 
        //     // 컨트롤 블록 안에 있는 저장소의 주소를 관리 객체로 지정하며, 생성한 컨트롤 블록을 넘겨서 스마트 포인터를 초기화
        //     _Ret._Set_ptr_rep_and_enable_shared(_STD addressof(_Rx->_Storage._Value), _Rx);
        //     return _Ret;
        // }
        //

        // 소멸자는 다음과 같이 동작한다.
        // 
        // --------------------------------------------------
        // 
        // 1. _Ref_count_base의 형태(사전 지식에서 다룬 내용)
        // 
        // _Ref_count_base는 _Uses와 _Weaks를 가지는 레퍼런스 카운팅 블록이며 두 개의 순수 가상 함수를 가진다.
        // 
        // class __declspec(novtable) _Ref_count_base {
        // private:
        //     virtual void _Destroy() noexcept     = 0; // destroy managed resource
        //     virtual void _Delete_this() noexcept = 0; // destroy self
        // 
        //     _Atomic_counter_t _Uses  = 1;
        //     _Atomic_counter_t _Weaks = 1;
        //     ...
        // };
        // 
        // - _Destroy() : 관리 객체의 수명이 다 했을 때 호출되는 함수(_Uses가 0으로 떨어졌을 때 호출)
        // - _Delete_this() : 컨트롤 블록의 수명이 다 했을 때 호출되는 함수(_Uses와 _Weaks가 0으로 떨어졌을 때 호출)
        // 
        // --------------------------------------------------
        // 
        // 2. _Ref_count_obj2<T>는 _Ref_count_base의 파생 클래스이기 때문에 2개의 순수 가상 함수를 오버라이딩하여 구체화해야 함.
        // 
        // template <class _Ty>
        // class _Ref_count_obj2 : public _Ref_count_base {
        //     ...
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
        // 3. shared_ptr<TestObjectEx>의 소멸자 호출
        // 
        // shared_ptr<_Ty>::~shared_ptr() noexcept { // release resource
        //     this->_Decref(); // 관리 객체의 유효성 검증
        // }
        // 
        // --------------------------------------------------
        // 
        // 4. _Decref()는 shared_ptr<T>에 있는 것이 아닌 기반 클래스 _Ptr_base<T>에 있는 함수
        // 
        // _Ptr_base<T>에는 관리 객체 _Ptr과 컨트롤 블록 _Rep가 있다.
        // 
        // template <class _Ty>
        // class _Ptr_base { // base class for shared_ptr and weak_ptr
        // public:
        //     using element_type = remove_extent_t<_Ty>;
        //     ...
        // 
        // private:
        //     element_type* _Ptr{ nullptr };
        //     _Ref_count_base* _Rep{ nullptr };
        //     ...
        // };
        // 
        // shared_ptr -> _Ptr_base<T> : [ element_type* _Ptr ][ _Ref_count_base* _Rep ]
        // weak_ptr -> _Ptr_base<T> : [ element_type* _Ptr ][ _Ref_count_base* _Rep ]
        // - _Ptr : 접근해서 사용하고자 하는 관리 객체의 메모리를 지정한 포인터
        // - _Rep : 컨트롤 블록(shared_ptr가 실질적으로 사용하는 메모리 블록)
        // 
        // !! _Ptr_base<T>의 _Decref() 호출 !!
        // void _Ptr_base<_Ty>::_Decref() noexcept { // decrement reference count
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // (중요) 컨트롤 블록 _Rep는 _Ref_count_base이긴 하나 실질적으로 적용된 건 _Ref_count_obj2<T>이다.
        // 
        // --------------------------------------------------
        // 
        // 5. 컨트롤 블록(_Ref_count_base)의 _Decref() 호출(_Ptr_base<T>를 경유해서 호출하는 형태)
        // 
        // void _Ref_count_base::_Decref() noexcept { // decrement use count
        //     // 원자적인 감소
        //     if (_MT_DECR(_Uses) == 0) 
        //     {
        //         // _Uses가 0으로 떨어지기에 해당 블록에 들어옴.
        //         _Destroy(); // 순수 가상 함수 | 관리 객체의 소멸 과정 유도
        //         _Decwref();
        //     }
        // }
        // 
        // _Uses가 0으로 떨어졌으면 관리 객체의 수명이 다 한 것이니 객체의 소멸 과정이 유도되어야 한다.
        // 
        // _Rep의 _Destroy()는 순수 가상 함수이기 때문에 가상 함수 테이블을 타고 들어가 파생 클래스의 _Destroy()를 호출한다.
        // !! 여기서는 _Ref_count_obj2<T>의 _Destroy()가 호출됨. !!
        // 
        // (중요) 가상 함수 테이블을 타고 들어가 파생 클래스의 _Destroy()를 호출하는 방식이기에 소멸 과정 유도 자체는 어떤 파생 컨트롤 블록을 사용하든 동일하게 적용된다.
        // 
        // --------------------------------------------------
        // 
        // 6. 파생 컨트롤 블록(_Ref_count_obj2<T>)의 _Destroy() 호출
        // 
        // void _Ref_count_obj2<_Ty>::_Destroy() noexcept override { // destroy managed resource
        //     _Destroy_in_place(_Storage._Value);
        // }
        // 
        // template <class _Ty>
        // _CONSTEXPR20 void _Destroy_in_place(_Ty& _Obj) noexcept 
        // {
        //     // !! 컴파일 타임에서 해당 부분은 실행되지 않게 걸러짐. !!
        //     if constexpr (is_array_v<_Ty>) 
        //     {
        //         _Destroy_range(_Obj, _Obj + extent_v<_Ty>);
        //     } 
        //     else 
        //     {
        //         // !! 이쪽 로직이 실행됨. !! //
        //         _Obj.~_Ty();
        //     }
        // }
        // 
        // 위 코드를 보면 저장소(_Storage._Value)를 _Ty&로 받아서 관리 객체의 소멸자를 직접적으로 호출하고 있는 것을 볼 수 있다.
        // !! 컨트롤 블록에 묶여 있는 관리 객체의 메모리를 대상으로 직접적으로 소멸자를 호출하는 것(메모리를 해제하는 건 아님). !!
        // 
        // --------------------------------------------------
        // 
        // 7. 관리 객체의 소멸자를 직접 호출
        // 
        // ~TestObjectEx() -> ~TestObject() 순으로 호출된다.
        // 
        // --------------------------------------------------
        // 
        // _Ref_count_base::_Decref()의 몸체에 도달할 때까지 콜 스택을 빠져 나오고...
        // 
        // void _Ref_count_base::_Decref() noexcept { // decrement use count
        //     // 원자적인 감소
        //     if (_MT_DECR(_Uses) == 0) {
        //         _Destroy(); // 순수 가상 함수 | 관리 객체의 소멸 과정 유도
        //         _Decwref();
        //     }
        // }
        // 
        // 8. 컨트롤 블록(_Ref_count_base)의 _Decwref()를 호출
        // 
        // void _Ref_count_base::_Decwref() noexcept { // decrement weak reference count
        //     // 원자적인 감소
        //     if (_MT_DECR(_Weaks) == 0) 
        //     {
        //         // _Weaks가 0으로 떨어지기에 해당 블록에 들어옴.
        //         _Delete_this(); // 순수 가상 함수 | 컨트롤 블록의 소멸 과정 유도
        //     }
        // }
        // 
        // _Weaks가 0으로 떨어졌으면 컨트롤 블록의 수명이 다 한 것이니 컨트롤 블록의 소멸 과정이 유도되어야 한다.
        // 
        // (중요) 마찬가지로 가상 함수 테이블을 타고 들어가 파생 클래스의 _Delete_this()를 호출하는 방식이며, 이러한 소멸 과정 유도 자체는 어떤 파생 컨트롤 블록을 사용하든 동일하게 적용된다.
        // 
        // --------------------------------------------------
        // 
        // 9. 파생 컨트롤 블록(_Ref_count_obj2<T>)의 _Delete_this() 호출
        // 
        // void _Ref_count_obj2<_Ty>::_Delete_this() noexcept override { // destroy self
        //     delete this;
        // }
        // 
        // _Ref_count_obj2<_Ty>::~_Ref_count_obj2() noexcept override { // TRANSITION, should be non-virtual
        //     // nothing to do, _Storage._Value was already destroyed in _Destroy
        // 
        //     // N4950 [class.dtor]/7:
        //     // "A defaulted destructor for a class X is defined as deleted if:
        //     // X is a union-like class that has a variant member with a non-trivial destructor"
        // }
        // 
        // --------------------------------------------------
        // 
        // 관리 객체의 소멸 유도와 컨트롤 블록의 소멸 유도 과정 자체는 어떤 파생 컨트롤 블록을 사용하든 동일하게 이루어진다.
        // !! 유도되는 과정 자체만 동일한 뿐이지 세부적인 소멸 과정은 파생 컨트롤 블록의 _Destroy()와 _Delete_this()의 구현에 의존적임. !!
        // 
        // --------------------------------------------------
        // 
        // (중요) 관리 객체의 소멸 유도와 컨트롤 블록의 소멸 유도가 별개로 이루어진다는 사실을 기억해야 한다.
        // - 관리 객체의 소멸 유도는 _Uses가 0으로 떨어졌을 때 진행됨.
        // - 컨트롤 블록의 소멸 유도는 _Weaks가 0으로 떨어졌을 때 진행됨.
        // 
        // _Uses가 0으로 떨어진 이후에 _Weaks가 0으로 떨어지는 구조이기 때문에
        // 관리 객체의 소멸 과정이 진행되었다고 해서 컨트롤 블록의 소멸 과정까지 이루어지는 것은 아니다.
        // 
        // 관리 객체와 컨트롤 블록은 별개의 대상으로 봐야 하며, 스마트 포인터는 이 둘을 처리하기 위한 관리 클래스라고 봐야 한다.
        // 
        // (중요) 컨트롤 블록이 저장소가 되었든 포인터가 되었든 관리 객체를 알고 있는 형태라면 
        // (중요) _Uses가 0으로 떨어졌을 때 관리 객체의 소멸 과정을 편하게 유도할 수 있다.
        // !! 여기선 관리 객체를 저장소로 보관하지만 외부에서 관리 객체를 만들어 전달하는 방식을 쓰면 포인터로 보관할 수도 있음. !!
        // 
        // Visual Studio의 C++에서 제공하는 스마트 포인터의 컨트롤 블록은 관리 객체를 알고 있는 형태로 구성하고 있지만,
        // 관리 객체와 컨트롤 블록은 별개의 대상으로 봐야 한다.
        // 
        // 컨트롤 블록이 관리 객체를 알고 있는 건 메모리를 약간(?) 희생해서 CPU 시간을 줄이기 위함이다.
        // 컨트롤 블록이 관리 객체를 모르게 하고 스마트 포인터에서 관리 객체의 소멸 과정을 유도해도 되지만 그럼 CPU 시간이 더 소모된다.
        // 
    }

    cout << "-------------------------#02#-------------------------\n\n";

    /**********************************
    *      Basic Usage(Self Ptr)      *
    **********************************/

    // 동적할당하여 생성한 Ptr를 생성자의 인자로 넘겨 스마트 포인터를 생성하는 방식
    {
        TestObjectEx* ptr = new TestObjectEx{ 456, 2.3456789 };
        
        shared_ptr<TestObjectEx> objEx{ ptr };
        
        cout << "## End Of Block ##\n\n";

        // 사용자가 직접 Ptr을 지정하는 방식이면 컨트롤 블록 _Ref_count<T>를 사용한다.
        // 
        // template <class _Ty>
        // class _Ref_count : public _Ref_count_base { // handle reference counting for pointer without deleter
        //     ...
        //     _Ty* _Ptr; // 사용자가 직접 전달한 객체의 포인터 Ptr
        // };
        // 
        // [ 가상 함수 테이블(8바이트) ][ _Uses(4바이트) ][ _Weaks(4바이트) ][ _Ptr(8바이트) ] 
        // 
        // !! 스마트 포인터에도 _Ptr가 있고 컨트롤 블록에도 _Ptr가 있음(여기서 말하는 _Ptr는 컨트롤 블록에 있는 _Ptr). !!
        // 
        // 스마트 포인터의 _Ptr는 스마트 포인터를 통해 접근해서 사용하자고 하는 관리 객체의 메모리를 말하는 것이다.
        // 컨트롤 블록 _Ref_count<T>에도 _Ptr이 있긴 하지만 이건 스마트 포인터의 _Ptr와는 의미가 묘하게 다르다.
        // 
        // (중요) 스마트 포인터의 _Ptr과 컨트롤 블록의 _Ptr이 가리키는 대상은 동일하지만 둘은 별개의 변수라는 점을 기억하고 분석해야 한다.
        // 
        // 컨트롤 블록 안에 관리 객체의 포인터(?)를 묶어서 저장한 이유는 "사전 지식"과 "Basic Usage(std::make_shared<T>())"의 마지막에 충분히 설명하였다.
        // 
        // --------------------------------------------------
        // 
        // 1. Ptr를 받는 shared_ptr<T>의 생성자 호출
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
        //     // !! 컴파일 타임에서 해당 부분은 실행되지 않게 걸러짐. !!
        //     if constexpr (is_array_v<_Ty>)
        //     {
        //         _Setpd(_Px, default_delete<_Ux[]>{});
        //     }
        //     else
        //     {
        //         // !! 이쪽 로직이 실행됨. !! //
        //         _Temporary_owner<_Ux> _Owner(_Px); // 임시 객체 생성
        // 
        //         _Set_ptr_rep_and_enable_shared( // shared_ptr을 실질적으로 초기화하는 함수
        //             // !! 사용자가 전달한 Ptr !!
        //             _Owner._Ptr,
        //             // !! 컨트롤 블록 생성 !!
        //             new _Ref_count<_Ux>(_Owner._Ptr));
        // 
        //         _Owner._Ptr = nullptr; // 임시 객체의 _Ptr을 성공적으로 전달했으니 nullptr로 밀어버림.
        //     }
        // }
        // 
        // --------------------------------------------------
        /*
        // !! 템플릿에 대한 설명(한 번 쯤은 분석해 볼 가치가 있다고 판단해서 작성함) !!
        // 
        // https://en.cppreference.com/w/cpp/language/sfinae
        // https://en.cppreference.com/w/cpp/types/enable_if
        // https://en.cppreference.com/w/cpp/types/conjunction
        // https://en.cppreference.com/w/cpp/types/conditional
        // https://en.cppreference.com/w/cpp/types/is_convertible
        // https://learn.microsoft.com/ko-kr/cpp/extensions/compiler-support-for-type-traits-cpp-component-extensions?view=msvc-170
        // https://en.cppreference.com/w/cpp/utility/declval
        // https://en.cppreference.com/w/cpp/types/void_t
        //
        // 템플릿 프로그래밍과 템플릿 메타 프로그래밍은 다른 개념이다.
        // 템플릿을 단순 일반화 타입으로 사용해서 프로그래밍하면 이건 일반화(Generic) 프로그래밍이라고 한다.
        // !! 메타 프로그래밍 : 컴파일 타임에 생성되는 코드를 이용해 프로그래밍을 하는 것. !!
        // 
        // 아래 서술하는 템플릿 프로그래밍 기법은 SFINAE(Substitution Failure Is Not An Error)와 관련이 있는 내용이다.
        // !! SFINAE : 템플릿 인자를 추론하는데 실패해도 이를 컴파일 에러로 간주하지 않고, 이를 적용하고자 했던 템플릿을 오버로딩 목록에서 제외시키는 상황을 말하는 것임. !!
        // 
        // 인자를 추론하는데 실패하면 이를 대체할 만한 적절한 템플릿을 다른 후보군에서 찾는데,
        // 이런 방식을 이용하여 프로그래밍하는 것이 SFINAE 프로그래밍이다.
        // !! (중요) SFINAE를 이용한 프로그래밍을 해도 대체할 만한 적절한 템플릿을 찾지 못 하면 컴파일 에러가 발생함. !!
        //
        // SFINAE를 이용하면 컴파일 타임에 조건부 선택을 적용하여 코드를 생성할 수 있다.
        // - 기본 케이스 : 템플릿 인자 추론에 실패했을 경우 적용할 템플릿
        // - 성공 케이스 : 템플릿 인자 추론에 성공했을 경우 적용하여 특수화할 템플릿
        // !! 추론에 실패하면 성공 케이스를 템플릿 오버로딩하지 않고 기본 케이스(실패 케이스)를 대체 템플릿으로 사용하여 적용함. !!
        //
        // template <class _Ux,
        //     enable_if_t<
        //         conjunction_v<
        //             conditional_t<is_array_v<_Ty>, _Can_array_delete<_Ux>, _Can_scalar_delete<_Ux>>,
        //             _SP_convertible<_Ux, _Ty>
        //         >,
        //         int
        //     > = 0>
        // - enable_if_t<..., int> : 조건이 참일 경우 int 타입으로 간주함.
        //   - conjunction_v<...> : 여러 조건이 true일 경우에만 true를 반환하는 템플릿(논리 AND 연산)
        //     - conditional_t<is_array_v<_Ty>, _Can_array_delete<_Ux>, _Can_scalar_delete<_Ux>>
        //      : is_array_v<_Ty>가 true일 경우 _Can_array_delete<_Ux>를 사용하고, false일 경우 _Can_scalar_delete<_Ux>를 사용함.
        //     - _SP_convertible<_Ux, _Ty>
        //      : _Ux에서 _Ty로 암시적인 변환이 가능한지 확인하기 위한 템플릿으로 가능하다면 value에 true가 담김.
        // 
        // 조건부 메타 프로그래밍(컴파일 타임에 조건에 따라서 타입이나 값을 선택적으로 결정하는 메타 프로그래밍)
        // - template <class _Ux, enable_if_t<..., int> = 0>
        //  : 조건을 만족했다면 사용할 템플릿 인자의 타입을 int로 가정하고 int의 기본값을 0으로 대입하는 문법이다.
        //    -> 조건을 충족하면 이런 느낌으로 코드가 생성됨(template <class _Ux, int = 0>).
        //  :
        //  : 조건을 만족하지 않으면 추론에 실패한 것이며, 이 경우 int든 뭐든 생성되는 타입 자체가 없으니 에러가 발생한다(대체할 만한 템플릿까지 없다면).
        //  : 조건을 충족하지 않아서 발생하는 에러 자체는 "= 0"과는 관련 없으니 분석할 때 주의하도록 한다.
        //  :
        //  : (중요) 템플릿 매개변수의 이름을 생략해도 기본값을 지정하면 템플릿 인자는 활성화된다.
        //  : 템플릿 인자가 템플릿 내에서 활용되진 않지만, 템플릿의 유형을 선택하기 위해서 인자 자체는 필요한 상황이라면 이 방식을 쓸 수 있다.
        //  : 이렇게 템플릿 매개변수의 이름을 생략하고 기본값만 넣는 방식은 조건부 메타 프로그래밍에서 자주 사용되는 방식이다.
        //  :
        //  : (중요) 이렇게 템플릿 인자의 이름을 생략하면 기본값(여기선 0)을 대입하는 코드가 반드시 작성되어야 한다.
        //    -> 예를 들어 다음 코드와 같이 타입의 이름도 없고 기본값도 없으면 문법 에러가 발생함(template <class _Ux, int>).
        // 
        // !! 템플릿 내부에서 인자는 쓰지 않되 유형 자체는 선택해야 하는 상황이라 이렇게 한 것임. !!
        // !! 템플릿 인자의 이름 자체는 생략하지 않고 명시해도 상관 없으니 분석할 때 혼동하면 안 됨. !!
        //
        // 
        // // 탈출 조건(재귀 조건 도중에 실패해면 type은 false가 될 것이고, 끝까지 도달하고 테스트까지 성공하면 type은 true가 될 것임)
        // template <bool _First_value, class _First, class... _Rest>
        // struct _Conjunction { // handle false trait or last trait
        //     using type = _First;
        // };
        // 
        // // 재귀 조건
        // template <class _True, class _Next, class... _Rest>
        // struct _Conjunction<true, _True, _Next, _Rest...> { // the first trait is true, try the next one
        //     using type = typename _Conjunction<_Next::value, _Next, _Rest...>::type;
        // };
        // 
        // // 헬퍼 템플릿
        // template <class... _Traits>
        // constexpr bool conjunction_v = conjunction<_Traits...>::value;
        // 
        // // 탈출 조건
        // template <class... _Traits>
        // struct conjunction : true_type {}; // If _Traits is empty, true_type
        // 
        // // 재귀 조건
        // template <class _First, class... _Rest>
        // struct conjunction<_First, _Rest...> : _Conjunction<_First::value, _First, _Rest...>::type {
        //     // the first false trait in _Traits, or the last trait if none are false
        // };
        // 
        // !! conjunction은 재귀 템플릿으로 구현되어 있음(템플릿 메타 프로그래밍). !!
        // 
        // 
        // // _SP_convertible는 어떠한 자료형 type을 상속받아 구현된 것(type은 값이 아니라 재정의한 타입임)
        // template <class _Yty, class _Ty>
        // struct _SP_convertible : is_convertible<_Yty*, _Ty*>::type {};
        // 
        // template <class _From, class _To>
        // struct is_convertible : bool_constant<__is_convertible_to(_From, _To)> {
        //     // determine whether _From is convertible to _To
        // };
        // 
        // template <bool _Val>
        // using bool_constant = integral_constant<bool, _Val>;
        // 
        // template <class _Ty, _Ty _Val>
        // struct integral_constant {
        //     static constexpr _Ty value = _Val; // bool_constant<_Val>이 대상이기에 true나 false임.
        // 
        //     using value_type = _Ty;
        //     using type = integral_constant; // 이걸 상속하여 _SP_convertible를 구현한 것임.
        //     // using 자체는 단순하게 타입을 재정의하는 문법임(여기서는 컴파일 타임에 타입이 결정되었다는 특성이 가미됨).
        //     // bool_constant<_Val>이 대상이기 때문에 integral_constant<bool, true>나 integral_constant<bool, false> 중 하나여야 함.
        // 
        //     constexpr operator value_type() const noexcept {
        //         return value;
        //     }
        // 
        //     constexpr value_type operator()() const noexcept {
        //         return value;
        //     }
        // };
        // 
        // 조금 복잡하긴 해도 상속 대상은 integral_constant<class _Ty, _Ty _Val>라는 것을 명심해야 한다.
        // _SP_convertible<_Ux, _Ty>::value는 integral_constant<class _Ty, _Ty _Val>에서의 _Val이다.
        // !! static constexpr 변수는 컴파일 타임에 상수로 평가되기 때문에 템플릿의 값으로 사용할 수 있음. !!
        // 
        // bool_constant<_Val>로 컴파일될 수 있는 건 integral_constant<bool, true>나 integral_constant<bool, false> 뿐이다.
        // 컴파일 타임에서, 타입이 결정된 템플릿 인자를 가진 구조체를 상속하여, 결정된 템플릿 인자의 값을 value로 쓰겠다는 뜻이다.
        // 
        // 
        // // SFINAE에 의해 템플릿 인자 추론에 실패하면 해당 템플릿은 템플릿 오버로딩 목록에서 걸러지는데,
        // // void_t<...>는 인자를 목록으로 받아 이를 간편하게 처리하게 하기 위한 목적의 템플릿임.
        // // void_t<...>는 조건부 메타 프로그래밍의 핵심 기능 중 하나라 자세하게 설명함.
        // template <class... _Types>
        // using void_t = void;
        // 
        // // 추론 실패 케이스 : std::declval()에서 사용됨.
        // template <class _Ty, class = void> // 기본 타입이 void라는 뜻이지 void 타입을 받는 특수화를 의미하는 것이 아님(혼동하지 말 것)
        // struct _Add_reference { // add reference (non-referenceable type)
        //     using _Lvalue = _Ty;
        //     using _Rvalue = _Ty;
        // };
        // 
        // // 추론 성공 케이스 : std::declval()에서 사용됨.
        // template <class _Ty>
        // struct _Add_reference<_Ty, void_t<_Ty&>> { // (referenceable type)
        //     using _Lvalue = _Ty&;
        //     using _Rvalue = _Ty&&;
        // };
        // 
        // // std::declval()에서 사용됨.
        // template <class _Ty>
        // using add_rvalue_reference_t = typename _Add_reference<_Ty>::_Rvalue;
        // 
        // // 컴파일 타임에 해당 타입의 객체가 있을 때 사용할 수 있는 기능을 조회하기 위한 함수
        // template <class _Ty>
        // add_rvalue_reference_t<_Ty> declval() noexcept {
        //     static_assert(_Always_false<_Ty>, "Calling declval is ill-formed, see N4950 [declval]/2.");
        // }
        // 
        // // 전달 인자가 void인지 확인
        // template <class _Ty>
        // constexpr bool is_void_v = is_same_v<remove_cv_t<_Ty>, void>;
        // 
        // // 실질적인 코드 분석 시작
        // using true_type  = bool_constant<true>;
        // using false_type = bool_constant<false>;
        //
        // template <class _Yty, class = void>
        // struct _Can_scalar_delete : false_type {}; // 추론 실패 케이스로 사용할 것(기본 템플릿)
        // template <class _Yty>
        // struct _Can_scalar_delete<_Yty, void_t<decltype(delete _STD declval<_Yty*>())>> : bool_constant<!is_void_v<_Yty>> {}; // 추론 성공 케이스를 템플릿 특수화
        // 
        // template <class _Yty, class = void>
        // struct _Can_array_delete : false_type {}; // 추론 실패 케이스로 사용할 것(기본 템플릿)
        // template <class _Yty>
        // struct _Can_array_delete<_Yty, void_t<decltype(delete[] _STD declval<_Yty*>())>> : true_type{}; // 추론 성공 케이스를 템플릿 특수화
        // 
        // std::declval<T>()는 객체가 없는 상태에서 해당 타입의 객체가 있을 때 사용할 수 있는 것을 조회하기 위한 템플릿이다.
        // !! decltype()은 실제 객체나 표현식을 대상으로 동작하는 문법이기 때문에 객체 없는 상태에서 기능을 조회하고자 한다면 std::declval<T>()를 써야 함. !!
        // 
        // std::declval<T>()를 쓰면 컴파일 타임에 "해당 타입이 있을 때 어떤 멤버나 표현식을 사용할 수 있을 것이다"를 가정할 수 있기 때문에
        // 실제 객체나 표현식을 대상으로 동작하는 decltype()과 연계되어 쓰이는 경우가 대부분이다(보통 표현식을 검증하는 용도로 연계함). 
        // ex) decltype(std::declval<T>().SomeFunc()) : T 타입에 SomeFunc()가 있는지 확인함.
        // 
        // (중요) std::void_t<...>는 조건부 메타 프로그래밍의 핵심이 되는 도구 중 하나이다.
        // 
        // https://en.cppreference.com/w/cpp/language/sfinae
        // https://en.cppreference.com/w/cpp/language/partial_specialization
        // 위 문서에서 "SFINAE in partial specializations"를 보면 다음 내용을 확인할 수 있다.
        // 
        // A substitution failure is not treated as a hard-error during such determination, 
        // but makes the corresponding partial specialization declaration ignored instead, 
        // as if in the overload resolution involving function templates.
        //
        // 링크의 예시 코드를 보면 부분적으로 특수화하여 적용한 std::void_t<...>가 추론에 실패하여,
        // Substitution Failure가 발생했을 때 이러한 실패 케이스를 받는 방법이 기술되어 있다.
        //
        // 템플릿 인자로 void_t<...>를 사용했을 때, void_t<...>에 전달한 각 인자들을 컴파일 타임에 계산했는데,
        // 추론에 실패하여 Substitution Failure가 발생하면 void_t<...>를 적용한 템플릿은 오버로딩되지 않는다.
        // !! 문법적으로 올바르지 않은 인자가 있으면 추론에 실패함(함수가 없다든지, 변수가 없다든지 등). !!
        // 
        // 전달한 모든 인자가 유효하여 추론에 성공하면 void_t<...>는 void 타입으로 치환된다.
        // void_t<...>이 void 인자로 치환되면 부분적으로 특수화된 템플릿으로 오버로딩한다.
        //
        // void_t<...>를 적용한 템플릿 쪽에서 인자 추론에 실패하면 이걸 적용한 템플릿을 무시하고 다른 대체 템플릿을 찾는다.
        // 이때 사용되는 것이 바로 실패 케이스(기본 템플릿)이다(대체 템플릿까지 찾을 수 없다면 컴파일 에러가 뜸).
        // !! 추론에 실패했을 때 받을 템플릿을 구성하는 것이 SFINAE 프로그래밍의 핵심임. !!
        // 
        // 정확히 말하자면 템플릿 인자 추론에 실패했을 때 템플릿 오버로딩 목록에서 제외하되,
        // 컴파일 에러 없이 다른 대체 템플릿을 선택하는 것이 SFINAE 기법의 핵심이다.
        // 
        // (중요) 이런 이유로 void_t<...>를 인자로 사용하는 쪽은 기본적으로 받을 기본 템플릿을 준비해두고(추론에 실패했을 경우 사용할 것),
        // (중요) 추론 성공 케이스를 특수화하여 적용하는 방식을 채택하는 것이 일반적이다.
        // 
        // 
        // template <class _Yty, class = void>
        // struct _Can_scalar_delete : false_type {}; // 추론 실패 케이스로 사용할 것(기본 템플릿)
        // template <class _Yty>
        // struct _Can_scalar_delete<_Yty, void_t<decltype(delete _STD declval<_Yty*>())>> : bool_constant<!is_void_v<_Yty>> {}; // 추론 성공 케이스를 템플릿 특수화
        // 
        // template <class _Yty, class = void>
        // struct _Can_array_delete : false_type {}; // 추론 실패 케이스로 사용할 것(기본 템플릿)
        // template <class _Yty>
        // struct _Can_array_delete<_Yty, void_t<decltype(delete[] _STD declval<_Yty*>())>> : true_type{}; // 추론 성공 케이스를 템플릿 특수화
        // 
        // 다시 이 코드를 보도록 하자.
        // 
        // _Can_array_delete<_Ux>와 _Can_scalar_delete<_Ux>> 양쪽 모두 전달된 인자를 추론하여 사용할 타입을 정의하고 있다.
        // !! 추론에 성공하면 true_type을, 실패하면 false_type을 적용함. !!
        // 
        // _Can_scalar_delete와 _Can_array_delete 양쪽 모두 기본 템플릿을 먼저 정의하고 추론에 성공했을 때 사용할 템플릿을 특수화하고 있다.
        // 
        // 기본 템플릿은 "template <class _Yty, class = void>"를 적용하고 있는데 "= void"에서 void 대신 다른 타입을 적용하면 안 된다.
        // 이건 void_t<...>의 특수성 때문에 그런 것인데, 특수화를 시도하는 쪽은 void_t<...>에 인자를 전달하여 추론을 유도하는 방식을 택하고 있다.
        // 추론에 성공하면 void_t<...>는 void 타입으로 치환되며, 이건 "template <class _Yty, class = void>" 자체를 특수화하여 정의한 상황이라고 볼 수 있다.
        // !! "class = void"로 기본 템플릿을 정의해야 void_t<...>로 특수화할 수 있음. !!
        //
        // void_t<decltype(delete _STD declval<_Yty*>())>>는 delete 연산이 가능하지 확인하기 위한 코드이다.
        // using CheckType = decltype(delete declval<int*>()); // CheckType은 void임.
        // !! Delete 연산을 할 수 없는 유형이라면 추론에 실패함. !!
        // 
        // 
        // 위에 나열된 내용을 이해한 상태에서 shared_ptr에 적용된 템플릿 코드를 보도록 하자.
        // 
        // https://en.cppreference.com/w/cpp/language/constraints
        // C++20의 concept과 requires를 쓰면 이런저런 복잡한 템플릿 특수화 과정을 단순화할 수 있다는데 이건 따로 학습해 봐야 한다.
        // 이걸 쓰면 조건을 만족하지 않는 타입을 전달했을 때 에러 메시지가 직관적으로 뜬다는데 이게 참 마음에 든다.
        */
        // --------------------------------------------------
        // 
        // 2. _Ref_count<TestObjectEx> 생성자 호출(컨트롤 블록 생성)
        // 
        // template <class _Ty>
        // class _Ref_count : public _Ref_count_base { // handle reference counting for pointer without deleter
        // public:
        //     explicit _Ref_count(_Ty* _Px) : _Ref_count_base(), _Ptr(_Px) {}
        //     ...
        // };
        // 
        // !! 컨트롤 블록 _Ref_count는 전달한 관리 객체의 포인터를 알고 있는 형태로 되어 있음. !!
        // 
        // --------------------------------------------------
        // 
        // 3. _Set_ptr_rep_and_enable_shared() 호출
        // 
        // template <class _Ux>
        // void shared_ptr<_Ty>::_Set_ptr_rep_and_enable_shared(_Ux* const _Px, _Ref_count_base* const _Rx) noexcept // take ownership of _Px
        // {
        //     this->_Ptr = _Px; // 관리 객체(사용자가 전달한 Ptr)
        //     this->_Rep = _Rx; // 컨트롤 블록(_Ref_count<T>)
        // 
        //     // !! 컴파일 타임에서 해당 부분은 실행되지 않게 걸러짐. !!
        //     if constexpr (conjunction_v<negation<is_array<_Ty>>, negation<is_volatile<_Ux>>, _Can_enable_shared<_Ux>>) {
        //         if (_Px && _Px->_Wptr.expired()) {
        //             _Px->_Wptr = shared_ptr<remove_cv_t<_Ux>>(*this, const_cast<remove_cv_t<_Ux>*>(_Px));
        //         }
        //     }
        // }
        // 
        // (중요) shared_ptr의 _Ptr과 _Ref_count<T>의 _Ptr은 다른 변수이다.
        // !! 가리키고 있는 대상만 같을 뿐임. !!
        // 
         
        // 스마트 포인터가 소멸자에서 소멸 과정을 유도하는 것 자체는 "Basic Usage(std::make_shared<T>())"에서 설명한 것과 동일하다.
        // 따라서 세부적인 동작 방식이 기술된 _Destroy()와 _Delete_this()만 보도록 할 것이다.
        // 
        // --------------------------------------------------
        // 
        // 1. _Ref_count<T>에서 오버라이딩한 순수 가상 함수
        // 
        // template <class _Ty>
        // class _Ref_count : public _Ref_count_base { // handle reference counting for pointer without deleter
        //     ...
        // private:
        //     void _Destroy() noexcept override { // destroy managed resource
        //         delete _Ptr; // 전달한 관리 객체를 컨트롤 블록에서 직접 삭제함.
        //     }
        // 
        //     void _Delete_this() noexcept override { // destroy self
        //         delete this; // 자기 자신을 삭제(컨트롤 블록 삭제)
        //     }
        // 
        //     _Ty* _Ptr;
        // };
        // 
        // 무언가 거쳐가는 작업 없이 바로 사용자가 전달한 관리 객체를 바로 삭제하는 것이 특징이다.
        // 
        // shared_ptr의 _Ptr과 _Ref_count<T>의 _Ptr은 다른 변수이지만 같은 대상을 가리키고 있다.
        // 스마트 포인터는 관리 객체를 사용하고자 하는 쪽에 제공만 할 뿐 관리 객체의 소멸 유도 자체는 컨트롤 블록에서 진행한다.
        //
    }

    cout << "-------------------------#03#-------------------------\n\n";

    /**************************************
    *      Basic Usage(Ptr, Deleter)      *
    **************************************/

    // Ptr, Deleter를 생성자의 인자로 넘겨 스마트 포인터를 생성하는 방식
    {
        // RVO에 의해 대상 메모리 공간에 직접적으로 Ptr, Deleter를 받는 생성자가 적용됨.
        shared_ptr<TestObjectEx> objEx = MyMakeSharedWithDeleter<TestObjectEx>(100, 3.141592);
        
        cout << "## End Of Block ##\n\n";

        // MyMakeSharedWithDeleter<T>()는 스마트 포인터를 생성할 때 Deleter를 등록하는 방식을 사용한다.
        // 
        // shared_ptr<T>{ ptr, &MyDeleter<T> };
        // 
        // --------------------------------------------------
        // 
        // Ptr과 Deleter를 받는 유형으로 스마트 포인터를 생성하면 컨트롤 블록 _Ref_count_resource<_Res, _Dx>를 사용한다.
        // 
        // template <class _Resource, class _Dx>
        // class _Ref_count_resource : public _Ref_count_base { // handle reference counting for object with deleter
        //     ...
        //     _Compressed_pair<_Dx, _Resource> _Mypair; // Deleter(_Dx)가 무엇으로 전달되느냐에 따라 다르게 적용됨.
        // };
        // 
        // template <class _Ty1, class _Ty2, bool = is_empty_v<_Ty1> && !is_final_v<_Ty1>> // !! 기본 템플릿 !!
        // class _Compressed_pair final : private _Ty1  { // store a pair of values, deriving from empty first
        // public:
        //     _Ty2 _Myval2;
        //     ...
        // };
        // 
        // template <class _Ty1, class _Ty2> // !! 템플릿 특수화 !!
        // class _Compressed_pair<_Ty1, _Ty2, false> final { // store a pair of values, not deriving from first
        // public:
        //     _Ty1 _Myval1;
        //     _Ty2 _Myval2;
        //     ...
        // };
        // 
        // [ 가상 함수 테이블(8바이트) ][ _Uses(4바이트) ][ _Weaks(4바이트) ][ _Mypair(16바이트 | 전달한 Ptr과 Deleter가 묶인 형태) ]
        // 
        // _Mypair의 자료형인 _Compressed_pair는 어떤 Deleter를 전달하냐에 따라서 유형이 달라진다.
        // !! 함수 포인터를 전달하는 형태는 특수화된 템플릿을 사용하며 그 크기는 16바이트(함수 포인터 8바이트 + 관리 객체의 포인터 8바이트)로 구성됨. !!
        // 
        // 위 코드는 추론을 실패하여 오버로딩에서 제외하는 그런 유형의 템플릿이 아니기 때문이 SFINAE가 아니다.
        // 이건 단순한 템플릿 특수화 코드이다.
        // !! 기본 템플릿의 인자를 보면 단순히 _Ty1의 Deleter가 함수 포인터이기 때문에 3번째 템플릿 인자를 false로 추론한 것임. !!
        // !! 그래서 특수화된 템플릿 _Compressed_pair<_Ty1, _Ty2, false>가 이를 받아서 처리한 것임. !!
        // 
        // (중요) Deleter를 함수 포인터가 아닌 Lambda로 지정하면 내부에선 Functor로 처리된다.
        // 
        // Functor는 객체이기 때문에 상속하는 것이 가능하며, 캡처 블록을 빈 상태로 두면 기본 템플릿이 적용된다.
        // 만약 캡처 블록이 무언가를 캡처하면 이건 일종의 멤버 변수이기 때문에 is_empty_v<_Ty1> 조건은 false가 되어 특수화된 템플릿이 적용된다.
        // 
        // --------------------------------------------------
        // 
        // 1. MyMakeSharedWithDeleter<T>(...)를 거쳐 메모리를 생성하고 shared_ptr<Type>{ ptr, &MyDeleter<Type> } 적용
        // 
        // template <typename Type, typename... Args>
        // shared_ptr<Type> MyMakeSharedWithDeleter(Args&&... args)
        // {
        //     ...
        //     Type* ptr = static_cast<Type*>(::malloc(sizeof(Type)));
        // 
        //     new (ptr) Type(std::forward<Args>(args)...); // placement new로 객체의 생성자 호출
        // 
        //     return shared_ptr<Type>{ ptr, &MyDeleter<Type> };
        // }
        // 
        // --------------------------------------------------
        // 
        // 2. Ptr과 Deleter를 받는 shared_ptr<T>의 생성자 호출
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
        // --------------------------------------------------
        // 
        // 2. _Setpd() 호출
        // 
        // template <class _UxptrOrNullptr, class _Dx>
        // void shared_ptr<T>::_Setpd(const _UxptrOrNullptr _Px, _Dx _Dt) // take ownership of _Px, deleter _Dt
        // {
        //     // !! Px는 외부에서 생성한 ptr, _Dt는 유저가 전달한 MyDeleter<Type> !!
        //     _Temporary_owner_del<_UxptrOrNullptr, _Dx> _Owner(_Px, _Dt); // 전달 용도의 임시 객체
        // 
        //     // shared_ptr을 실질적으로 초기화하기 위한 함수
        //     _Set_ptr_rep_and_enable_shared(
        //         _Owner._Ptr, 
        //         // !! 컨트롤 블록 생성 !!
        //         new _Ref_count_resource<_UxptrOrNullptr, _Dx>(_Owner._Ptr, _STD move(_Dt)));
        // 
        //     _Owner._Call_deleter = false;
        // }
        // 
        // --------------------------------------------------
        // 
        // 3. _Ref_count_resource<_Res, _Dx> 생성자 호출(컨트롤 블록 생성)
        // 
        // template <class _Resource, class _Dx>
        // class _Ref_count_resource : public _Ref_count_base // handle reference counting for object with deleter
        // {
        // public:
        //     _Ref_count_resource(_Resource _Px, _Dx _Dt)
        //         : _Ref_count_base(), _Mypair(_One_then_variadic_args_t{}, _STD move(_Dt), _Px) {}
        //     ...
        // private:
        //     void _Destroy() noexcept override { // destroy managed resource
        //         _Mypair._Get_first()(_Mypair._Myval2);
        //     }
        // 
        //     void _Delete_this() noexcept override { // destroy self
        //         delete this;
        //     }
        // 
        //     // _Dx가 Deleter이고, _Resource는 Deleter가 적용될 포인터임.
        //     _Compressed_pair<_Dx, _Resource> _Mypair;
        // };
        // 
        // --------------------------------------------------
        // 
        // 4. _Set_ptr_rep_and_enable_shared() 호출
        // 
        // template <class _Ux>
        // void shared_ptr<T>::_Set_ptr_rep_and_enable_shared(_Ux* const _Px, _Ref_count_base* const _Rx) noexcept // take ownership of _Px
        // {
        //     this->_Ptr = _Px; // 관리 객체(사용자가 전달한 Ptr)
        //     this->_Rep = _Rx; // 컨트롤 블록(_Ref_count_resource<T>)
        // 
        //     // !! 컴파일 타임에서 해당 부분은 실행되지 않게 걸러짐. !!
        //     if constexpr (conjunction_v<negation<is_array<_Ty>>, negation<is_volatile<_Ux>>, _Can_enable_shared<_Ux>>) {
        //         if (_Px && _Px->_Wptr.expired()) {
        //             _Px->_Wptr = shared_ptr<remove_cv_t<_Ux>>(*this, const_cast<remove_cv_t<_Ux>*>(_Px));
        //         }
        //     }
        // }
        // 

        // 스마트 포인터의 소멸자에서 소멸 과정을 유도하는 것 자체는 동일하기 때문에 _Destroy()와 _Delete_this()만 보도록 할 것이다.
        //
        // --------------------------------------------------
        // 
        // 1. _Ref_count_resource<T>에서 오버라이딩한 순수 가상 함수
        // 
        // template <class _Resource, class _Dx>
        // class _Ref_count_resource : public _Ref_count_base // handle reference counting for object with deleter
        // {
        //     ...
        // private:
        //     void _Destroy() noexcept override { // destroy managed resource
        //         // _Myval1(first)에는 Deleter가, _Myval2(second)에는 관리 객체의 주소가 저장되어 있음.
        //         _Mypair._Get_first()(_Mypair._Myval2);
        //     }
        // 
        //     void _Delete_this() noexcept override { // destroy self
        //         // 자기 자신을 삭제(컨트롤 블록 삭제)
        //         delete this;
        //     }
        // 
        //     // _Dx가 Deleter이고, _Resource는 Deleter가 적용될 포인터임.
        //     _Compressed_pair<_Dx, _Resource> _Mypair;
        // };
        // 
        // _Ref_count_resource 유형은 지정한 Deleter에 관리 객체의 포인터를 넣어서 객체의 소멸 과정을 유도하고 있다.
        // 
        // --------------------------------------------------
        // 
        // 2. _Myval2에 담긴 Deleter는 MyDeleter<T>
        // 
        // template <typename Type>
        // void MyDeleter(Type* ptr)
        // {
        //     ...
        //     delete ptr;
        // }
        //
        // 지정된 Deleter에 따라 객체를 바로 삭제하거나 메모리 풀 등을 사용해서 반환하는 것도 가능하다.
        //
    }
    
    cout << "-------------------------#04#-------------------------\n\n";

    /***********************************
    *      Basic Usage(Allocator)      *
    ***********************************/

    // Allocator를 생성자의 인자로 넘겨 스마트 포인터를 생성하는 방식
    {
        // RVO에 의해 대상 메모리 공간에 직접적으로 Allocator를 받는 생성자가 적용됨.
        shared_ptr<TestObjectEx> objEx = MyMakeSharedWithAllocator<TestObjectEx>(248, 6.283184);
        
        cout << "## End Of Block ##\n\n";

        // MyMakeSharedWithAllocator<T>()는 스마트 포인터를 생성할 때 Allocator를 등록하는 방식을 사용한다.
        // 
        // MyAllocator<Type> allocator;
        // 
        // return std::allocate_shared<Type>(allocator, std::forward<Args>(args)...);
        // 
        // --------------------------------------------------
        // 
        // Allocator를 받는 유형으로 생성하면 컨트롤 블록 _Ref_count_obj_alloc3<_Ty, _Alloc>을 사용한다.
        // !! _Ref_count_obj_alloc3는 다중 상속으로 구현되어 있음. !!
        // 
        // template <class _Ty, class _Alloc>
        // class _Ref_count_obj_alloc3 : public _Ebco_base<_Rebind_alloc_t<_Alloc, _Ty>>, public _Ref_count_base {
        //     ...
        //     union {
        //         _Wrap<_Ty> _Storage;
        //     };
        //     ...
        // };
        // 
        // [ 가상 함수 테이블(8바이트) ][ _Uses(4바이트) ][ _Weaks(4바이트) ][ _Myval(Allocator의 크기 바이트) ][ _Storage(sizeof(T) 바이트) ]
        // !! _Uses와 _Weaks는 _Ref_count_base 영역, _Myval은 _Ebco_base 영역, _Storage는 _Ref_count_obj_alloc3의 영역임. !!
        // 
        // --------------------------------------------------
        // 
        // 1. 할당자를 생성하고 allocate_shared<TestObjectEx>() 호출
        // 
        // template <typename Type, typename... Args>
        // shared_ptr<Type> MyMakeSharedWithAllocator(Args&&... args)
        // {
        //     ...
        //     MyAllocator<Type> allocator;
        // 
        //     return std::allocate_shared<Type>(allocator, std::forward<Args>(args)...);
        // }
        // 
        // template <class _Ty, class _Alloc, class... _Types>
        // enable_if_t<!is_array_v<_Ty>, shared_ptr<_Ty>> allocate_shared(const _Alloc& _Al, _Types&&... _Args) // make a shared_ptr to non-array object
        // {
        //     // Note: As of 2019-05-28, this implements the proposed resolution of LWG-3210 (which controls whether
        //     // allocator::construct sees T or const T when _Ty is const qualified)
        //     using _Refoa   = _Ref_count_obj_alloc3<remove_cv_t<_Ty>, _Alloc>;
        //     using _Alblock = _Rebind_alloc_t<_Alloc, _Refoa>;
        //     
        //     // 전달한 할당자를 변환(?) 생성자를 적용한 새로운 할당자로 받음.
        //     _Alblock _Rebound(_Al);
        //     
        //     // 할당자가 할당한 메모리를 받기 위한 임시 객체 생성
        //     _Alloc_construct_ptr<_Alblock> _Constructor{_Rebound};
        //     
        //     // 할당자로 메모리를 확보하고 _Construct_in_place() 내에서 placement new로 생성자 호출
        //     _Constructor._Allocate();
        //     _Construct_in_place(*_Constructor._Ptr, _Al, _STD forward<_Types>(_Args)...);
        //     
        //     // 빈 스마트 포인터 생성
        //     shared_ptr<_Ty> _Ret;
        //     
        //     // 임시 객체 내 할당자로 할당한 메모리로부터 관리 객체의 주소 추출
        //     const auto _Ptr = reinterpret_cast<_Ty*>(_STD addressof(_Constructor._Ptr->_Storage._Value));
        //     
        //     // 스마트 포인터가 사용할 관리 객체와 컨트롤 블록 지정
        //     _Ret._Set_ptr_rep_and_enable_shared(_Ptr, _Unfancy(_Constructor._Release()));
        //     
        //     // 반환
        //     return _Ret;
        // }
        // 
        // --------------------------------------------------
        // 
        // 2. 메모리를 확보하고 placement new로 생성자 호출
        // 
        // template <class _Alloc>
        // struct _Alloc_construct_ptr // pointer used to help construct 1 _Alloc::value_type without EH
        // {
        //     using pointer = _Alloc_ptr_t<_Alloc>;
        //     _Alloc& _Al;
        //     pointer _Ptr;
        //     ...
        //     _CONSTEXPR20 void _Allocate() { // disengage *this, then allocate a new memory block
        //         _Ptr = nullptr; // if allocate throws, prevents double-free
        //
        //         // 메모리 확보
        //         _Ptr = _Al.allocate(1);
        //     }
        //     ...
        // };
        // 
        // template <class _Ty, class... _Types>
        // _CONSTEXPR20 void _Construct_in_place(_Ty& _Obj, _Types&&... _Args) noexcept(
        //     is_nothrow_constructible_v<_Ty, _Types...>) 
        // {
        //     // !! 컴파일 타임에서 해당 부분은 실행되지 않게 걸러짐. !!
        //     if (_STD is_constant_evaluated())
        //     {
        //         _STD construct_at(_STD addressof(_Obj), _STD forward<_Types>(_Args)...);
        //     }
        //     else
        //     {
        //         // placement new로 생성자 호출
        //         ::new (static_cast<void*>(_STD addressof(_Obj))) _Ty(_STD forward<_Types>(_Args)...);
        //     }
        // }
        // 
        // --------------------------------------------------
        // 
        // 3. 스마트 포인터가 사용할 관리 객체와 컨트롤 블록 지정
        // 
        // template <class _Ux>
        // void shared_ptr<_Ty>::_Set_ptr_rep_and_enable_shared(_Ux* const _Px, _Ref_count_base* const _Rx) noexcept // take ownership of _Px
        // {
        //     this->_Ptr = _Px; // 관리 객체(_Ref_count_obj_alloc3<T>의 저장소 주소)
        //     this->_Rep = _Rx; // 컨트롤 블록(_Ref_count_obj_alloc3<T>(저장소가 묶인 형태))
        // 
        //     // !! 컴파일 타임에서 해당 부분은 실행되지 않게 걸러짐. !!
        //     if constexpr (conjunction_v<negation<is_array<_Ty>>, negation<is_volatile<_Ux>>, _Can_enable_shared<_Ux>>) {
        //         if (_Px && _Px->_Wptr.expired()) {
        //             _Px->_Wptr = shared_ptr<remove_cv_t<_Ux>>(*this, const_cast<remove_cv_t<_Ux>*>(_Px));
        //         }
        //     }
        // }
        // 
        // --------------------------------------------------
        // 
        // 4. 스마트 포인터를 반환(RVO에 의해 복사나 이동 연산자는 적용되지 않음)
        // 
        // template <class _Ty, class _Alloc, class... _Types>
        // enable_if_t<!is_array_v<_Ty>, shared_ptr<_Ty>> allocate_shared(const _Alloc& _Al, _Types&&... _Args) // make a shared_ptr to non-array object
        // {
        //     ...
        //     shared_ptr<_Ty> _Ret;
        //     const auto _Ptr = reinterpret_cast<_Ty*>(_STD addressof(_Constructor._Ptr->_Storage._Value));
        //     
        //     _Ret._Set_ptr_rep_and_enable_shared(_Ptr, _Unfancy(_Constructor._Release()));
        //     
        //     // 반환
        //     return _Ret;
        // }
        //

        // 스마트 포인터의 소멸자에서 소멸 과정을 유도하는 것 자체는 동일하기 때문에 _Destroy()와 _Delete_this()만 보도록 할 것이다.
        //
        // --------------------------------------------------
        //
        // 1. _Ref_count_resource<T>에서 오버라이딩한 순수 가상 함수
        // 
        // template <class _Ty, class _Alloc>
        // class _Ref_count_obj_alloc3 : public _Ebco_base<_Rebind_alloc_t<_Alloc, _Ty>>, public _Ref_count_base 
        // {
        //     ...
        // private:
        //     ...
        //     void _Destroy() noexcept override { // destroy managed resource
        //         allocator_traits<_Rebound>::destroy(this->_Get_val(), _STD addressof(_Storage._Value));
        //     }
        // 
        //     void _Delete_this() noexcept override { // destroy self
        //         _Rebind_alloc_t<_Alloc, _Ref_count_obj_alloc3> _Al(this->_Get_val());
        //         this->~_Ref_count_obj_alloc3();
        //         _Deallocate_plain(_Al, this);
        //     }
        // };
        // 
        // --------------------------------------------------
        // 
        // 2. _Destroy() 관련 함수 호출 과정
        // 
        // template <class _Alloc>
        // struct _Normal_allocator_traits // defines traits for allocators
        // {
        //     ...
        //     template <class _Ty>
        //     static _CONSTEXPR20 void destroy(_Alloc& _Al, _Ty* _Ptr) 
        //     {
        //         if constexpr (_Uses_default_destroy<_Alloc, _Ty*>::value)\
        //         {
        //             // 이쪽 로직이 실행됨.
        //             _STD destroy_at(_Ptr);
        //         }
        //         else
        //         {
        //             _Al.destroy(_Ptr);
        //         }
        //     }
        //     ...
        // };
        // 
        // _EXPORT_STD template <class _Ty>
        // _CONSTEXPR20 void destroy_at(_Ty* const _Location) noexcept /* strengthened */
        // {
        //     // !! 컴파일 타임에서 해당 부분은 실행되지 않게 걸러짐. !!
        //     if constexpr (is_array_v<_Ty>)
        //     {
        //         _Destroy_range(_STD begin(*_Location), _STD end(*_Location));
        //     }
        //     else
        //     {
        //         // 이쪽 로직이 실행됨.
        //         // 관리 객체의 소멸자 호출을 유도함.
        //         _Location->~_Ty();
        //     }
        // }
        // 
        // --------------------------------------------------
        // 
        // 3. _Delete_this() 관련 함수 호출 과정
        // 
        // template <class _Ty, class _Alloc>
        // class _Ref_count_obj_alloc3 : public _Ebco_base<_Rebind_alloc_t<_Alloc, _Ty>>, public _Ref_count_base 
        // {
        //     ...
        //     void _Delete_this() noexcept override // destroy self
        //     {
        //         // _Get_val()은 할당자를 반환하며 해당 할당자를 변환(?) 생성자로 받음.
        //         _Rebind_alloc_t<_Alloc, _Ref_count_obj_alloc3> _Al(this->_Get_val());
        // 
        //         // 컨트롤 블록의 소멸자 호출
        //         this->~_Ref_count_obj_alloc3();
        // 
        //         // 메모리를 반환하기 위한 함수(할당자로 메모리를 할당했으니)
        //         _Deallocate_plain(_Al, this);
        //     }
        // };
        // 
        // template <class _Alloc>
        // _CONSTEXPR20 void _Deallocate_plain(_Alloc& _Al, typename _Alloc::value_type* const _Ptr) noexcept 
        // {
        //     // deallocate a plain pointer using an allocator
        //     using _Alloc_traits = allocator_traits<_Alloc>;
        // 
        //     if constexpr (is_same_v<_Alloc_ptr_t<_Alloc>, typename _Alloc::value_type*>)
        //     {
        //         // 해당 로직이 실행됨.
        //         _Alloc_traits::deallocate(_Al, _Ptr, 1);
        //     }
        //     else
        //     {
        //         using _Ptr_traits = pointer_traits<_Alloc_ptr_t<_Alloc>>;
        //         _Alloc_traits::deallocate(_Al, _Ptr_traits::pointer_to(*_Ptr), 1);
        //     }
        // }
        // 
        // template <class _Alloc>
        // struct _Normal_allocator_traits // defines traits for allocators
        // {
        //     ...
        //     static _CONSTEXPR20 void deallocate(_Alloc& _Al, pointer _Ptr, size_type _Count)
        //     {
        //         // 할당자의 deallocate() 호출하여 메모리를 반환함.
        //         _Al.deallocate(_Ptr, _Count);
        //     }
        // };
        // 
    }
    
    cout << "-------------------------#05#-------------------------\n\n";

    /*************************************************
    *      Basic Usage(Ptr, Deleter, Allocator)      *
    *************************************************/

    // Ptr, Deleter, Allocator를 생성자의 인자로 넘겨 스마트 포인터를 생성하는 방식
    {
        // RVO에 의해 대상 메모리 공간에 직접적으로 Ptr, Deleter, Allocator를 받는 생성자가 적용됨.
        shared_ptr<TestObjectEx> objEx = MyMakeSharedWithDeleterAllocator<TestObjectEx>(777, 1.6180339);
        
        cout << "## End Of Block ##\n\n";

        // MyMakeSharedWithDeleterAllocator<T>()는 스마트 포인터를 생성할 때 Ptr, Deleter, Allocator을 받는 방식을 사용한다.
        // !! Deleter와 Allocator를 둘 다 받는 유형에 대한 설명은 shared_ptr_with_deleter_and_allocator.cpp에 기술되어 있음. !!
        //
        // shared_ptr<Type>{ ptr, &MyDeleter<Type>, MyAllocator<Type>{ } };
        // 
        // --------------------------------------------------
        // 
        // Ptr, Deleter, Allocator를 받는 유형으로 스마트 포인터를 생성하면 _Ref_count_resource_alloc<_Res, _Dx, _Alloc>을 사용한다.
        // 
        // template <class _Resource, class _Dx, class _Alloc>
        // class _Ref_count_resource_alloc : public _Ref_count_base {
        //     ...
        //     _Compressed_pair<_Dx, _Compressed_pair<_Myalty, _Resource>> _Mypair;
        // };
        // 
        // template <class _Ty1, class _Ty2, bool = is_empty_v<_Ty1> && !is_final_v<_Ty1>> // !! 기본 템플릿 !!
        // class _Compressed_pair final : private _Ty1  { // store a pair of values, deriving from empty first
        // public:
        //     _Ty2 _Myval2;
        //     ...
        // };
        // 
        // template <class _Ty1, class _Ty2> // !! 템플릿 특수화 !!
        // class _Compressed_pair<_Ty1, _Ty2, false> final { // store a pair of values, not deriving from first
        // public:
        //     _Ty1 _Myval1;
        //     _Ty2 _Myval2;
        //     ...
        // };
        // 
        // [ 가상 함수 테이블(8바이트) ][ _Uses(4바이트) ][ _Weaks(4바이트) ][ Pair<Dx, Pair<Alloc, Px>(24바이트 | 전달한 Ptr, Deleter, Allocator가 묶인 형태) ]
        // 
        // _Ref_count_resource_alloc의 _Mypair는 또 다른 Pair를 포함하고 있는 형태이다.
        // !! _Compressed_pair에 대한 설명은 "Basic Usage(Ptr, Deleter)" 쪽에 있음. !!
        // 
        // --------------------------------------------------
        // 
        // 1. MyMakeSharedWithDeleterAllocator<T>(...)를 거쳐 할당자를 통해 메모리를 생성하고 shared_ptr<Type>{ ptr, &MyDeleter<Type>, MyAllocator<Type>{ } } 적용
        // 
        // template <typename Type, typename... Args>
        // shared_ptr<Type> MyMakeSharedWithDeleterAllocator(Args&&... args)
        // {
        //     ...
        //     Type* ptr = static_cast<Type*>(::malloc(sizeof(Type)));
        // 
        //     new (ptr) Type(std::forward<Args>(args)...); // placement new로 객체의 생성자 호출
        // 
        //     return shared_ptr<Type>{ ptr, &MyDeleter<Type>, MyAllocator<Type>{ } };
        // }
        // 
        // 여기서 전달하는 Allocator는 Ptr와 Deleter와는 연관성이 없다.
        // 
        // Allocator는 내부의 컨트롤 블록을 할당 및 해제하기 위한 목적으로 사용하며,
        // Ptr는 독립적으로 생성되어 전달되고 Deleter는 이러한 Ptr를 해제하기 위한 목적으로 사용된다.
        // 
        // --------------------------------------------------
        // 
        // 2. Ptr, Deleter, Allocator를 받는 shared_ptr<T>의 생성자 호출
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
        // --------------------------------------------------
        // 
        // 3. _Setpda() 호출
        // 
        // template <class _UxptrOrNullptr, class _Dx, class _Alloc>
        // void shared_ptr<T>::_Setpda(const _UxptrOrNullptr _Px, _Dx _Dt, _Alloc _Ax) // take ownership of _Px, deleter _Dt, allocator _Ax
        // {
        //     using _Alref_alloc = _Rebind_alloc_t<_Alloc, _Ref_count_resource_alloc<_UxptrOrNullptr, _Dx, _Alloc>>;
        // 
        //     // 전달한 인자를 묶은 임시 객체
        //     _Temporary_owner_del<_UxptrOrNullptr, _Dx> _Owner(_Px, _Dt);
        // 
        //     // 전달한 할당자를 변환(?) 생성자를 적용한 새로운 할당자로 받음.
        //     _Alref_alloc _Alref(_Ax);
        // 
        //     // 할당자가 할당한 메모리를 받기 위한 임시 객체 생성
        //     _Alloc_construct_ptr<_Alref_alloc> _Constructor(_Alref);
        // 
        //     // 할당자로 메모리를 확보하고 _Construct_in_place() 내에서 placement new로 생성자 호출
        //     _Constructor._Allocate();
        //     _Construct_in_place(*_Constructor._Ptr, _Owner._Ptr, _STD move(_Dt), _Ax);
        // 
        //     // 스마트 포인터가 사용할 관리 객체와 컨트롤 블록 지정
        //     _Set_ptr_rep_and_enable_shared(_Owner._Ptr, _Unfancy(_Constructor._Ptr));
        // 
        //     // 정리
        //     _Constructor._Ptr    = nullptr;
        //     _Owner._Call_deleter = false;
        // }
        // 
        // 여기서 진행되는 과정 자체는 "Basic Usage(Allocator)"에서 한 것과 유사하다.
        // !! 따라서 메모리 할당 과정과 관리 객체 + 컨트롤 블록을 지정하는 방식은 생략하도록 함. !!

        // 스마트 포인터의 소멸자에서 소멸 과정을 유도하는 것 자체는 동일하기 때문에 _Destroy()와 _Delete_this()만 보도록 할 것이다.
        //
        // --------------------------------------------------
        // 
        // 1. _Ref_count_resource_alloc<T>에서 오버라이딩한 순수 가상 함수
        // 
        // template <class _Resource, class _Dx, class _Alloc>
        // class _Ref_count_resource_alloc : public _Ref_count_base
        // {
        //     ...
        // private:
        //     ...
        //     void _Destroy() noexcept override { // destroy managed resource
        //         _Mypair._Get_first()(_Mypair._Myval2._Myval2);
        //     }
        // 
        //     void _Delete_this() noexcept override { // destroy self
        //         _Myalty _Al = _Mypair._Myval2._Get_first();
        //         this->~_Ref_count_resource_alloc();
        //         _Deallocate_plain(_Al, this);
        //     }
        // 
        //     _Compressed_pair<_Dx, _Compressed_pair<_Myalty, _Resource>> _Mypair;
        // };
        // 
        // --------------------------------------------------
        // 
        // 2. _Destroy() 함수 설명
        // 
        // template <class _Resource, class _Dx, class _Alloc>
        // class _Ref_count_resource_alloc : public _Ref_count_base
        // {
        //     ...
        //     void _Destroy() noexcept override { // destroy managed resource
        //         _Mypair._Get_first()(_Mypair._Myval2._Myval2);
        //     }
        //     ...
        // };
        // 
        // _Mypair._Get_first()에 의해 반환되는 건 인자로 전달했던 MyDeleter,
        // _Mypair._Myval2._Myval2는 인자로 전달했던 Ptr이다.
        // !! _Compressed_pair<_Dx, _Compressed_pair<_Myalty, _Resource>> _Mypair; !!
        // 
        // Ptr에 대한 메모리 반환 책임은 Deleter에 있다.
        // !! 컨트롤 블록의 메모리 할당과 해제에 대한 책임은 할당자에게 있음(다른 개념). !!
        // 
        // --------------------------------------------------
        // 
        // 2. _Delete_this() 함수 설명
        // 
        // template <class _Resource, class _Dx, class _Alloc>
        // class _Ref_count_resource_alloc : public _Ref_count_base
        // {
        //     ...
        //     void _Delete_this() noexcept override // destroy self
        //     {
        //         // 할당자 복사
        //         _Myalty _Al = _Mypair._Myval2._Get_first();
        // 
        //         // 컨트롤 블록의 소멸자 호출
        //         this->~_Ref_count_resource_alloc();
        // 
        //         // 메모리를 반환하기 위한 함수(할당자로 메모리를 할당했으니)
        //         _Deallocate_plain(_Al, this);
        //     }
        //     ...
        // };
        // 
        // template <class _Alloc>
        // struct _Normal_allocator_traits // defines traits for allocators
        // {
        //     ...
        //     static _CONSTEXPR20 void deallocate(_Alloc& _Al, pointer _Ptr, size_type _Count)
        //     {
        //         // 할당자의 deallocate() 호출하여 메모리를 반환함.
        //         _Al.deallocate(_Ptr, _Count);
        //     }
        // };
        //
    }

    cout << "-------------------------#06#-------------------------\n\n";

    /*****************************
    *      Copy Constructor      *
    *****************************/

    // 복사, 이동 관련 작업을 분석할 때는 (Ptr, Deleter, Allocator)를 받는 유형으로 스마트 포인터를 생성할 것이다.
    // 이 유형으로 분석해야 관리 객체의 포인터 주소와 컨트롤 블록의 주소를 추적하기 쉽다.
    //
    // 스마트 포인터를 어떤 유형으로 생성하든 복사 연산과 이동 연산 자체는 동일한 방식으로 수행된다.
    // 복사 연산과 이동 연산은 스마트 포인터의 유형에 영향을 받는 것이 아니다.
    // 이 두 연산은 관리 객체의 주소(_Ptr)와 컨트롤 블록의 베이스 _Ref_count_base(_Rep)로 이루어진다.
    //
    // 쉽게 말해서 make_shared<T>()로 스마트 포인터를 생성해도 복사와 이동 연산 자체는 동일한 방식으로 진행된다.
    // !! 그냥 추적하기 편하게 (Ptr, Deleter, Allocator)를 받는 유형으로 생성한 것임. !!

    // 복사 생성자(shared_ptr(const shared_ptr& _Other))로 shared_ptr을 생성했을 경우
    {
        // 빈 스마트 포인터 생성
        shared_ptr<TestObject> obj1;
        
        // A(Ptr, Deleter, Allocator를 받는 유형의 생성자를 거친 후 RVO에 의해 반환된 스마트 포인터를 이동 대입 연산자로 받음)
        obj1 = MyMakeSharedWithDeleterAllocator_Simple<TestObject>(111);
        
        cout << "# Before - Copy Constructor\n";
        
        // B(복사 생성자)
        shared_ptr<TestObject> obj2 = obj1; // obj2{ obj1 };
        
        cout << "\n# After - Copy Constructor\n";
        
        cout << "\n## End Of Block ##\n\n";

        // 관리 객체(_Ptr)와 컨트롤 블록(_Rep)가 어떻게 관리되는지 확인하기 위한 코드이다.
        // 정확히 말해서 shared_ptr의 소멸자가 소멸 과정을 어떻게 유도하는지 파악하기 위한 부분이다.
        // 
        // B 과정을 거쳤을 때 스마트 포인터가 어떻게 소멸 과정을 유도하는지 확인하면 된다.
        //
        // --------------------------------------------------
        // 
        // 1. "shared_ptr<TestObject> obj1"을 생성하되 내용을 채우지 않으면 _Ptr과 _Rep는 nullptr로 채워진다.
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
        // ***** A 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 2. "obj1 = MyMakeSharedWithDeleterAllocator_Simple<TestObject>(111)"에서 함수가 반환한 스마트 포인터를 이동 대입 연산자로 받는다.
        // 
        // template <typename Type, typename... Args>
        // shared_ptr<Type> MyMakeSharedWithDeleterAllocator_Simple(Args&&... args)
        // {
        //     ...
        //     return shared_ptr<Type>{ ptr, &MyDeleter<Type>, SimpleAllocator<Type>{ } };
        // }
        // 
        // !! obj1의 _Ptr과 _Rep는 nullptr인 상황 !!
        // !! 이동 대입 연산자 호출 !!
        // shared_ptr& operator=(shared_ptr&& _Right) noexcept // take resource from _Right
        // {
        //     // 전달한 _Right의 소유권을 이전 받은 임시 객체를 생성하고 swap()을 진행함.
        //     shared_ptr(_STD move(_Right)).swap(*this);
        //     return *this;
        // }
        // 
        // !! 임시 객체에 소유권을 이전하기 위해 move(_Right)를 적용하였기에 이동 생성자 호출 !!
        // shared_ptr(shared_ptr&& _Right) noexcept // construct shared_ptr object that takes resource from _Right
        // {
        //     this->_Move_construct_from(_STD move(_Right));
        // }
        // 
        // !! _Move_construct_from(_STD move(_Right)) 호출 !!
        // template <class _Ty2>
        // void _Ptr_base<T>::_Move_construct_from(_Ptr_base<_Ty2>&& _Right) noexcept
        // {
        //     // implement shared_ptr's (converting) move ctor and weak_ptr's move ctor
        //     // !! 스마트 포인터의 소유권(관리 객체와 컨트롤 블록)을 임시 객체에 넘김. !!
        //     _Ptr = _Right._Ptr;
        //     _Rep = _Right._Rep;
        // 
        //     // !! 전달 대상이 된 기존 스마트 포인터의 소유권을 날림. !!
        //     _Right._Ptr = nullptr;
        //     _Right._Rep = nullptr;
        // }
        // 
        // --------------------------------------------------
        // 
        // 3. 임시 객체에 소유권을 전달한 다음 swap()을 호출
        // 
        // void shared_ptr<T>::swap(shared_ptr& _Other) noexcept
        // {
        //     // this는 스마트 포인터의 소유권을 양도 받은 대상임(이동 대입 연산자에 있는 this와 여기 있는 this는 다른 대상이니 주의할 것).
        //     // _Other는 "shared_ptr<TestObject> obj1"임(_Swap()을 호출하고 있는 this는 임시 객체임).
        //     this->_Swap(_Other);
        // }
        // 
        // void _Ptr_base<T>::_Swap(_Ptr_base& _Right) noexcept // swap pointers
        // {
        //     // "obj1 = MyMakeSharedWithDeleterAllocator_Simple<TestObject>(111)"에서 함수가 반환한 스마트 포인터를 obj1에 넘기는 단계임.
        //     // _Right는 위의 _Other이자 "shared_ptr<TestObject> obj1"임('=' 대입 연산자의 왼쪽에 있던 것을 _Right로 받은 것).
        //     _STD swap(_Ptr, _Right._Ptr);
        //     _STD swap(_Rep, _Right._Rep);
        // }
        // 
        // --------------------------------------------------
        // 
        // 4. 콜 스택을 빠져나오고 임시 객체의 소멸자 호출
        // 
        // shared_ptr& operator=(shared_ptr&& _Right) noexcept // take resource from _Right
        // {
        //     // _Right의 _Ptr과 _Rep는 소유권을 임시 객체에 양도하면서 nullptr로 밀리고,
        //     // 임시 객체의 _Ptr과 _Rep 또한 "shared_ptr<TestObject> obj1"에 소유권을 양도하면서 교환 과정을 통해 nullptr로 교체됨.
        //     shared_ptr(_STD move(_Right)).swap(*this);
        //     return *this;
        // }
        // 
        // _Right는 소유권을 양도하는 이동 대상이라 소멸자가 호출되지 않는다(애초에 참조 형태임).
        // 소멸자가 호출되는 건 _Right의 소유권을 이전 받은 임시 객체 쪽이다.
        // 
        // !! 임시 객체 shared_ptr(_STD move(_Right))의 소멸자를 호출 !!
        // ~shared_ptr() noexcept // release resource
        // {
        //     this->_Decref();
        // }
        // 
        // void _Ptr_base<T>::_Decref() noexcept // decrement reference count
        // {
        //     // obj1의 _Rep가 nullptr이었으며 이걸 임시 객체가 교환 받았기 때문에 _Decref()를 실행하지 않음.
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // RVO로 반환되었다고 해도 객체의 소멸자는 호출된다.
        // 
        // !! "obj1 = MyMakeSharedWithDeleterAllocator_Simple<TestObject>(111)"에서 함수가 반환한 스마트 포인터가 대상임. !!
        // ~shared_ptr() noexcept // release resource
        // {
        //     this->_Decref();
        // }
        // 
        // void _Ptr_base<T>::_Decref() noexcept // decrement reference count
        // {
        //     // 임시 객체를 만드는 도중 "MyMakeSharedWithDeleterAllocator_Simple<TestObject>(111)"가 반환한 자원의 소유권은 이전된 상태임.
        //     // 따라서 _Rep는 nullptr임.
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // --------------------------------------------------
        // 
        // ***** B 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 5. "shared_ptr<TestObject> obj2 = obj1" 과정에서 복사 생성자 호출
        // 
        // shared_ptr(const shared_ptr& _Other) noexcept // construct shared_ptr object that owns same resource as _Other
        // {
        //     this->_Copy_construct_from(_Other);
        // }
        // 
        // --------------------------------------------------
        // 
        // 6. _Copy_construct_from() 호출
        // 
        // template <class _Ty2>
        // void _Ptr_base<T>::_Copy_construct_from(const shared_ptr<_Ty2>& _Other) noexcept
        // {
        //     // implement shared_ptr's (converting) copy ctor
        //     // !! _Other는 obj1이고 이걸 대상으로 _Incref() 호출하여 _Uses를 1 증가시킴. !!
        //     _Other._Incref();
        // 
        //     // 레퍼런스 카운트 _Uses를 1 증가시킨 후 obj2가 obj1과 같은 관리 객체와 컨트롤 블록을 가지게 설정함.
        //     _Ptr = _Other._Ptr;
        //     _Rep = _Other._Rep;
        // }
        // 
        // void _Ptr_base<T>::_Incref() const noexcept 
        // {
        //     if (_Rep) {
        //         _Rep->_Incref();
        //     }
        // }
        // 
        // void _Ref_count_base::_Incref() noexcept // increment use count
        // {
        //     _MT_INCR(_Uses);
        // }
        // 
        // --------------------------------------------------
        // 
        // ***** 블록을 빠져나오는 과정에서 소멸자 호출 진행 과정 *****
        // 
        // --------------------------------------------------
        // 
        // 7. obj2와 obj1의 소멸자를 차례대로 호출
        // 
        // !! obj2의 소멸자 호출 !!
        // ~shared_ptr() noexcept // release resource
        // {
        //     this->_Decref();
        // }
        // 
        // void _Ptr_base<T>::_Decref() noexcept // decrement reference count
        // {
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // void _Ref_count_base::_Decref() noexcept // decrement use count
        // {
        //     // _Uses의 값이 2이기 때문에 1을 감소시켜도 조건에 충족하지 않음.
        //     if (_MT_DECR(_Uses) == 0) {
        //         _Destroy();
        //         _Decwref();
        //     }
        // }
        // 
        // !! obj1의 소멸자 호출 !!
        // ~shared_ptr() noexcept // release resource
        // {
        //     this->_Decref();
        // }
        // 
        // void _Ptr_base<T>::_Decref() noexcept // decrement reference count
        // {
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // void _Ref_count_base::_Decref() noexcept // decrement use count
        // {
        //     // _Uses의 값이 1이기 때문에 1을 감소시키면 0에 도달하여 조건에 충족함.
        //     // 따라서 _Destroy()와 _Decwref()가 호출됨.
        //     if (_MT_DECR(_Uses) == 0) {
        //         // !! 컨트롤 블록에 재정의된 _Destroy()와 _Delete_this() 호출 !!
        //         _Destroy();
        //         _Decwref(); // _Weaks의 값을 1 줄이고 _Delete_this() 호출
        //     }
        // }
        // 
    }

    cout << "-------------------------#07#-------------------------\n\n";

    /*****************************
    *      Move Constructor      *
    *****************************/

    // 이동 생성자(shared_ptr(shared_ptr&& _Right))로 shared_ptr을 생성했을 경우
    {
        // A(RVO에 의해 Ptr, Deleter, Allocator를 받는 유형의 생성자가 적용됨)
        shared_ptr<TestObject> obj1 = MyMakeSharedWithDeleterAllocator_Simple<TestObject>(222);
        
        cout << "# Before - Move Constructor\n";
        
        // B(이동 생성자)
        shared_ptr<TestObject> obj2 = std::move(obj1);
        
        cout << "\n# After - Move Constructor\n";
        
        cout << "\n## End Of Block ##\n\n";

        // B 과정을 거쳤을 때 스마트 포인터가 어떻게 소멸 과정을 유도하는지 확인하면 된다.
        //
        // --------------------------------------------------
        // 
        // ***** A 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 1. shared_ptr<TestObject> obj1 = MyMakeSharedWithDeleterAllocator_Simple<TestObject>(222);
        // 
        // RVO에 의해 MyMakeSharedWithDeleterAllocator_Simple<T>()가 반환한 스마트 포인터가 바로 obj1에 적용된다.
        // 
        // template <typename Type, typename... Args>
        // shared_ptr<Type> MyMakeSharedWithDeleterAllocator_Simple(Args&&... args)
        // {
        //     ...
        //     return shared_ptr<Type>{ ptr, &MyDeleter<Type>, SimpleAllocator<Type>{ } };
        // }
        // 
        // --------------------------------------------------
        // 
        // ***** B 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 2. "shared_ptr<TestObject> obj2 = std::move(obj1)" 과정에서 이동 생성자 호출
        // 
        // shared_ptr(shared_ptr&& _Right) noexcept // construct shared_ptr object that takes resource from _Right
        // {
        //     this->_Move_construct_from(_STD move(_Right));
        // }
        // 
        // !! _Move_construct_from(_STD move(_Right)) 호출 !!
        // template <class _Ty2>
        // void _Ptr_base<T>::_Move_construct_from(_Ptr_base<_Ty2>&& _Right) noexcept
        // {
        //     // implement shared_ptr's (converting) move ctor and weak_ptr's move ctor
        //     // !! 스마트 포인터의 소유권(관리 객체와 컨트롤 블록)을 임시 객체에 넘김. !!
        //     _Ptr = _Right._Ptr;
        //     _Rep = _Right._Rep;
        // 
        //     // !! 전달 대상이 된 기존 스마트 포인터 obj1의 소유권을 날림. !!
        //     _Right._Ptr = nullptr;
        //     _Right._Rep = nullptr;
        // }
        // 
        // --------------------------------------------------
        // 
        // ***** 블록을 빠져나오는 과정에서 소멸자 호출 진행 과정 *****
        // 
        // --------------------------------------------------
        // 
        // 3. obj2와 obj1의 소멸자를 차례대로 호출
        // 
        // !! obj2의 소멸자 호출(std::move()로 소유권을 이전 받은 스마트 포인터) !!
        // ~shared_ptr() noexcept // release resource
        // {
        //     this->_Decref();
        // }
        // 
        // void _Ptr_base<T>::_Decref() noexcept // decrement reference count
        // {
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // void _Ref_count_base::_Decref() noexcept // decrement use count
        // {
        //     // _Uses의 값이 1이기 때문에 1을 감소시키면 0이 돼서 조건을 충족함.
        //     if (_MT_DECR(_Uses) == 0) {
        //         // !! 컨트롤 블록에 재정의된 _Destroy()와 _Delete_this() 호출 !!
        //         _Destroy();
        //         _Decwref(); // _Weaks의 값을 1 줄이고 _Delete_this() 호출
        //     }
        // }
        // 
        // !! obj1의 소멸자 호출(std::move()에 의해 소유권을 상실한 스마트 포인터) !!
        // ~shared_ptr() noexcept // release resource
        // {
        //     this->_Decref();
        // }
        // 
        // void _Ptr_base<T>::_Decref() noexcept // decrement reference count
        // {
        //     // "shared_ptr<TestObject> obj2 = std::move(obj1)" 과정에서 소유권을 이전했기 때문에 obj1의 _Rep는 nullptr임.
        //     // 따라서 조건을 만족하지 못 하고 빠져 나옴.
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
    }

    cout << "-------------------------#08#-------------------------\n\n";

    /*************************************
    *      Copy Assignment Operator      *
    *************************************/

    // 복사 대입 연산자(operator=(const shared_ptr& _Right))로 shared_ptr의 변수 자체를 갱신할 경우
    {
        // A(RVO에 의해 Ptr, Deleter, Allocator를 받는 유형의 생성자가 적용됨)
        shared_ptr<TestObject> obj1 = MyMakeSharedWithDeleterAllocator_Simple<TestObject>(333);
        
        cout << "# Separator #\n\n";
        
        // B(RVO에 의해 Ptr, Deleter, Allocator를 받는 유형의 생성자가 적용됨)
        shared_ptr<TestObject> obj2 = MyMakeSharedWithDeleterAllocator_Simple<TestObject>(444);
        
        cout << "# Before - Copy Assignment Operator\n\n";
        
        // C(복사 대입 연산자)
        obj1 = obj2;
        
        cout << "# After - Copy Assignment Operator\n";
        
        cout << "\n## End Of Block ##\n\n";

        // C 과정을 거쳤을 때 스마트 포인터가 어떻게 소멸 과정을 유도하는지 확인하면 된다.
        //
        // --------------------------------------------------
        // 
        // ***** A 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 1. shared_ptr<TestObject> obj1 = MyMakeSharedWithDeleterAllocator_Simple<TestObject>(333);
        // 
        // RVO에 의해 MyMakeSharedWithDeleterAllocator_Simple<T>()가 반환한 스마트 포인터가 바로 obj1에 적용된다.
        // 
        // template <typename Type, typename... Args>
        // shared_ptr<Type> MyMakeSharedWithDeleterAllocator_Simple(Args&&... args)
        // {
        //     ...
        //     return shared_ptr<Type>{ ptr, &MyDeleter<Type>, SimpleAllocator<Type>{ } };
        // }
        // 
        // --------------------------------------------------
        // 
        // ***** B 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 2. 대상만 다를 뿐 A에서 진행했던 것과 똑같다.
        // 
        // --------------------------------------------------
        // 
        // ***** C 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 3. "obj1 = obj2" 과정에서 복사 대입 연산자 호출
        // 
        // shared_ptr& operator=(const shared_ptr& _Right) noexcept
        // {
        //     // this는 obj1, _Right는 obj2임.
        //     // !! shared_ptr(_Right)로 되어 있음(shared_ptr(_STD move(_Right))로 전달되는 것이 아님)). !!
        //     shared_ptr(_Right).swap(*this);
        //     return *this;
        // }
        // 
        // !! 임시 객체 shared_ptr(_Right)를 생성하기 위한 복사 생성자 호출 !!
        // shared_ptr(const shared_ptr& _Other) noexcept // construct shared_ptr object that owns same resource as _Other
        // {
        //     // _Other는 obj2임(위에서 전달된 매개변수 _Right).
        //     this->_Copy_construct_from(_Other);
        // }
        // 
        // !! _Copy_construct_from() 호출 !!
        // template <class _Ty2>
        // void _Ptr_base<T>::_Copy_construct_from(const shared_ptr<_Ty2>& _Other) noexcept
        // {
        //     // implement shared_ptr's (converting) copy ctor
        //     // !! _Other는 obj2고 이걸 대상으로 _Incref() 호출하여 _Uses를 1 증가시킴. !!
        //     _Other._Incref();
        // 
        //     // 레퍼런스 카운트 _Uses를 1 증가시킨 후 임시 객체 shared_ptr(_Right)가 obj2와 같은 관리 객체와 컨트롤 블록을 가지게 설정함.
        //     _Ptr = _Other._Ptr;
        //     _Rep = _Other._Rep;
        // }
        // 
        // !! _Other._Incref() 호출 !!
        // void _Ptr_base<T>::_Incref() const noexcept
        // {
        //     // obj1은 make_shared<TestObject>(100)의 소유권을 이전 받은 상태이기에 _Rep는 nullptr이 아님.
        // 
        //     // obj2(_Other)는 MyMakeSharedWithDeleterAllocator_Simple<TestObject>(444)가 반환한 스마트 포인터를 RVO로 받은 상태이다.
        //     // _Rep는 nullptr이 아니기 때문에 _Incref()를 호출한다.
        //     if (_Rep) 
        //     {
        //         _Rep->_Incref();
        //     }
        // }
        // 
        // void _Ref_count_base::_Incref() noexcept // increment use count
        // {
        //     _MT_INCR(_Uses);
        // }
        // 
        // --------------------------------------------------
        // 
        // 4. 콜 스택을 빠져나오고 swap() 호출
        // 
        // shared_ptr& operator=(const shared_ptr& _Right) noexcept
        // {
        //     // 임시 객체는 obj2(_Right)를 복사한 상태이며, this는 obj1임.
        //     shared_ptr(_Right).swap(*this);
        //     return *this;
        // }
        // 
        // void shared_ptr<T>::swap(shared_ptr& _Other) noexcept
        // {
        //     // 여기서의 this는 obj2를 복사한 임시 객체이고, _Other는 obj1임.
        //     this->_Swap(_Other);
        // }
        // 
        // !! 여기서 this는 obj2를 복사한 임시 객체이기 때문에 _Uses(strong ptr)은 하나 증가한 상태임. !!
        // void _Ptr_base<T>::_Swap(_Ptr_base& _Right) noexcept // swap pointers
        // {
        //     // 교환 전 상태 : this{ _Ptr : 100, strong ptr : 2 }, _Right{ _Ptr : 200, strong ptr : 1 }
        //     // 교환 후 상태 : this{ _Ptr : 200, strong ptr : 1 }, _Right{ _Ptr : 100, strong ptr : 2 }
        //     _STD swap(_Ptr, _Right._Ptr);
        //     _STD swap(_Rep, _Right._Rep);
        // 
        //     // obj1 = obj2;
        //     // 
        //     // 여기서 this는 obj2를 복사한 임시 객체라는 사실을 잊으면 안 됨.
        //     // _Right는 위 대입 코드에서 왼쪽에 있는 obj1임.
        //     // 
        //     // 위 swap() 과정을 통해 obj1의 모든 내용을 임시 객체의 것으로 교환하여 갱신한 상태임.
        //     // 조금 이상하긴 해도 obj1은 obj2와 동일한 _Ptr과 _Rep를 가리키고 있는 형태임.
        //     // 
        //     // 교환 대상이 임시 객체와 obj1이기 때문에 임시 객체는 obj1이 가지고 있던 _Ptr과 _Rep를 가리키고 있는 형태가 됨.
        // }
        // 
        // --------------------------------------------------
        // 
        // 5. 콜 스택을 빠져나오고 임시 객체의 소멸자 호출
        // 
        // shared_ptr& operator=(const shared_ptr& _Right) noexcept
        // {
        //     // obj1과 obj2는 같은 _Ptr과 _Rep를 가리키고 있는 상태임.
        //     // 임시 객체는 obj1이 가지고 있던 _Ptr과 _Rep를 가리키고 있음(위의 교환 후 상태 참고).
        //     shared_ptr(_Right).swap(*this);
        //     return *this;
        // }
        // 
        // !! 임시 객체 shared_ptr(_Right)의 소멸자를 호출 !!
        // ~shared_ptr() noexcept // release resource
        // {
        //     this->_Decref();
        // }
        // 
        // void _Ptr_base<T>::_Decref() noexcept // decrement reference count
        // {
        //     // 현재 임시 객체가 반영하고 있는 건 교체하기 이전의 obj1의 _Ptr과 _Rep임.
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // void _Ref_count_base::_Decref() noexcept // decrement use count
        // {
        //     // _Uses의 카운트가 1이니 감소시키면 0에 도달하여 _Destroy()와 _Decwref()를 이어서 호출함.
        //     if (_MT_DECR(_Uses) == 0) {
        //         _Destroy();
        //         _Decwref();
        //     }
        // }
        // 
        // --------------------------------------------------
        // 
        // ***** 블록을 빠져나오는 과정에서 소멸자 호출 진행 과정 *****
        // 
        // --------------------------------------------------
        // 
        // 6. obj2와 obj1의 소멸자를 차례대로 호출
        // 
        // !! obj2의 소멸자 호출 !!
        // ~shared_ptr() noexcept // release resource
        // {
        //     this->_Decref();
        // }
        // 
        // void _Ptr_base<T>::_Decref() noexcept // decrement reference count
        // {
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // void _Ref_count_base::_Decref() noexcept // decrement use count
        // {
        //     // _Uses의 값이 2이기 때문에 1을 감소시켜도 조건에 충족하지 않음.
        //     if (_MT_DECR(_Uses) == 0) {
        //         _Destroy();
        //         _Decwref();
        //     }
        // }
        // 
        // !! obj1의 소멸자 호출 !!
        // ~shared_ptr() noexcept // release resource
        // {
        //     this->_Decref();
        // }
        // 
        // void _Ptr_base<T>_Decref() noexcept // decrement reference count
        // {
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // void _Ref_count_base::_Decref() noexcept // decrement use count
        // {
        //     // _Uses의 값이 1이기 때문에 1을 감소시키면 0에 도달하여 조건에 충족함.
        //     // 따라서 _Destroy()와 _Decwref()가 호출됨.
        //     if (_MT_DECR(_Uses) == 0) {
        //         // !! 컨트롤 블록에 재정의된 _Destroy()와 _Delete_this() 호출 !!
        //         _Destroy();
        //         _Decwref(); // _Weaks의 값을 1 줄이고 _Delete_this() 호출
        //     }
        // }
        // 
    }

    cout << "-------------------------#09#-------------------------\n\n";

    /*************************************
    *      Move Assignment Operator      *
    *************************************/

    // 이동 대입 연산자(operator=(shared_ptr&& _Right))로 shared_ptr의 변수 자체를 갱신할 경우
    {
        // A(RVO에 의해 Ptr, Deleter, Allocator를 받는 유형의 생성자가 적용됨)
        shared_ptr<TestObject> obj1 = MyMakeSharedWithDeleterAllocator_Simple<TestObject>(555);
        
        cout << "# Separator #\n\n";
        
        // B(RVO에 의해 Ptr, Deleter, Allocator를 받는 유형의 생성자가 적용됨)
        shared_ptr<TestObject> obj2 = MyMakeSharedWithDeleterAllocator_Simple<TestObject>(666);
        
        cout << "# Before - Move Assignment Operator\n\n";
        
        // C(이동 대입 연산자)
        obj1 = std::move(obj2);
        
        cout << "# After - Move Assignment Operator\n";
        
        cout << "\n## End Of Block ##\n\n";

        // C의 이동 대입 과정을 거쳤을 때 스마트 포인터가 어떻게 소멸 과정을 유도하는지 확인하면 된다.
        // 
        // --------------------------------------------------
        // 
        // ***** A 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 1. shared_ptr<TestObject> obj1 = MyMakeSharedWithDeleterAllocator_Simple<TestObject>(555);
        // 
        // RVO에 의해 MyMakeSharedWithDeleterAllocator_Simple<T>()가 반환한 스마트 포인터가 바로 obj1에 적용된다.
        // 
        // template <typename Type, typename... Args>
        // shared_ptr<Type> MyMakeSharedWithDeleterAllocator_Simple(Args&&... args)
        // {
        //     ...
        //     return shared_ptr<Type>{ ptr, &MyDeleter<Type>, SimpleAllocator<Type>{ } };
        // }
        // 
        // --------------------------------------------------
        // 
        // ***** B 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 2. 대상만 다를 뿐 A에서 진행했던 것과 똑같다.
        // 
        // --------------------------------------------------
        // 
        // ***** C 부분 *****
        // 
        // --------------------------------------------------
        // 
        // 3. "obj1 = std::move(obj2)" 과정에서 이동 대입 연산자 호출
        // 
        // shared_ptr& operator=(shared_ptr&& _Right) noexcept // take resource from _Right
        // {
        //     shared_ptr(_STD move(_Right)).swap(*this);
        //     return *this;
        // }
        // 
        // !! 임시 객체에 소유권을 이전하기 위해 move(_Right)를 적용하였기에 이동 생성자 호출 !!
        // shared_ptr(shared_ptr&& _Right) noexcept // construct shared_ptr object that takes resource from _Right
        // {
        //     this->_Move_construct_from(_STD move(_Right));
        // }
        // 
        // !! _Move_construct_from(_STD move(_Right)) 호출 !!
        // template <class _Ty2>
        // void _Ptr_base<T>::_Move_construct_from(_Ptr_base<_Ty2>&& _Right) noexcept
        // {
        //     // implement shared_ptr's (converting) move ctor and weak_ptr's move ctor
        //     // !! 스마트 포인터의 소유권(관리 객체와 컨트롤 블록)을 임시 객체에 넘김. !!
        //     _Ptr = _Right._Ptr;
        //     _Rep = _Right._Rep;
        // 
        //     // !! 전달 대상이 된 기존 스마트 포인터의 소유권을 날림. !!
        //     _Right._Ptr = nullptr;
        //     _Right._Rep = nullptr;
        // }
        // 
        // --------------------------------------------------
        // 
        // 4. 임시 객체에 소유권을 전달한 다음 swap()을 호출
        // 
        // void shared_ptr<T>::swap(shared_ptr& _Other) noexcept
        // {
        //     // this는 스마트 포인터의 소유권을 양도 받은 대상임(이동 대입 연산자에 있는 this와 여기 있는 this는 다른 대상이니 주의할 것).
        //     // _Other는 "obj1 = std::move(obj2)" 중 obj1에 해당하며, _Swap()을 호출하고 있는 this는 임시 객체임.
        //     this->_Swap(_Other);
        // }
        // 
        // void _Ptr_base<T>::_Swap(_Ptr_base& _Right) noexcept // swap pointers
        // {
        //     // obj2가 임시 객체에 넘긴 소유권을 obj1에 반영하는 단계임.
        //     _STD swap(_Ptr, _Right._Ptr);
        //     _STD swap(_Rep, _Right._Rep);
        // 
        //     // 위 코드가 실행되었으면 임시 객체는 obj1의 _Ptr과 _Rep를 가리키게 됨.
        //     //
        //     // swap()으로 교환하기 이전
        //     // : obj2{ _Ptr : nullptr }, 임시 객체{ _Ptr : 200, strong ptr : 1 }, obj1{ _Ptr : 100, strong ptr : 1 }
        //     //
        //     // swap()으로 교환한 이후
        //     // : obj2{ _Ptr : nullptr }, 임시 객체{ _Ptr : 100, strong ptr : 1 }, obj1{ _Ptr : 200, strong ptr : 1 }
        //     // 
        //     // obj2는 임시 객체에 소유권을 넘기면서 _Ptr과 _Rep를 nullptr로 밀었음.
        // }
        // 
        // --------------------------------------------------
        // 
        // 5. 콜 스택을 빠져나오고 임시 객체의 소멸자 호출
        // 
        // shared_ptr& operator=(shared_ptr&& _Right) noexcept // take resource from _Right
        // {
        //     // _Right의 _Ptr과 _Rep는 소유권을 임시 객체에 양도하면서 nullptr로 밀리고,
        //     // 임시 객체의 _Ptr과 _Rep는 obj2의 것으로 교환된 상태임.
        //     shared_ptr(_STD move(_Right)).swap(*this);
        //     return *this;
        // }
        // 
        // _Right는 소유권을 양도하는 이동 대상이라 소멸자가 호출되지 않는다(애초에 참조 형태임).
        // 소멸자가 호출되는 건 _Right의 소유권을 이전 받은 임시 객체 쪽이다.
        // 
        // !! 임시 객체 shared_ptr(_STD move(_Right))의 소멸자를 호출 !!
        // ~shared_ptr() noexcept // release resource
        // {
        //     this->_Decref();
        // }
        // 
        // void _Ptr_base<T>::_Decref() noexcept // decrement reference count
        // {
        //     // 임시 객체는 obj2의 소유권을 이전 받은 다음 obj1과 소유권을 교환한 상태임.
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // void _Ref_count_base::_Decref() noexcept // decrement use count
        // {
        //     // _Uses의 값이 1이기 때문에 1을 감소시키면 0에 도달하여 조건에 충족함.
        //     // 따라서 _Destroy()와 _Decwref()가 호출됨.
        //     if (_MT_DECR(_Uses) == 0) {
        //         // !! 컨트롤 블록에 재정의된 _Destroy()와 _Delete_this() 호출 !!
        //         _Destroy();
        //         _Decwref(); // _Weaks의 값을 1 줄이고 _Delete_this() 호출
        //     }
        // }
        // 
        // --------------------------------------------------
        // 
        // ***** 블록을 빠져나오는 과정에서 소멸자 호출 진행 과정 *****
        // 
        // --------------------------------------------------
        // 
        // 6. obj2와 obj1의 소멸자를 차례대로 호출
        // 
        // !! obj2의 소멸자 호출(std::move()에 의해 소유권을 상실한 스마트 포인터) !!
        // ~shared_ptr() noexcept // release resource
        // {
        //     this->_Decref();
        // }
        // 
        // void _Ptr_base<T>::_Decref() noexcept // decrement reference count
        // {
        //     // "obj1 = std::move(obj2)" 과정에서 소유권을 이전했기 때문에 obj2의 _Rep는 nullptr임.
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // !! obj1의 소멸자 호출(std::move()로 obj2의 소유권을 이전 받은 스마트 포인터) !!
        // 
        // ~shared_ptr() noexcept // release resource
        // {
        //     this->_Decref();
        // }
        // 
        // void _Ptr_base<T>::_Decref() noexcept // decrement reference count
        // {
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // void _Ref_count_base::_Decref() noexcept // decrement use count
        // {
        //     // _Uses의 값이 1이기 때문에 1을 감소시키면 0이 돼서 조건을 충족함.
        //     if (_MT_DECR(_Uses) == 0) {
        //         // !! 컨트롤 블록에 재정의된 _Destroy()와 _Delete_this() 호출 !!
        //         _Destroy();
        //         _Decwref(); // _Weaks의 값을 1 줄이고 _Delete_this() 호출
        //     }
        // }
        // 
    }

    cout << "-------------------------#10#-------------------------\n\n";

    /*********************************************
    *      Allocator Copy by Copy Operation      *
    *********************************************/

    // 스마트 포인터를 복사할 때, 생성 당시 만든 Allocator까지 복사해서 새로 만드는지 확인하기 위한 코드
    // Copy Constructor와 Copy Assignment Operator의 연장선
    {
        // RVO에 의해 대상 메모리 공간에 직접적으로 Allocator를 받는 생성자가 적용됨.
        shared_ptr<TestObjectEx> objEx1 = MyMakeSharedWithAllocator<TestObjectEx>(100, 3.141592);
        
        cout << "# Separator #\n\n";
        
        // 복사 대입 연산자
        {
            shared_ptr<TestObjectEx> objEx2; // 빈 스마트 포인터
        
            objEx2 = objEx1;
        
            cout << "Copy : objEx2 = objEx1;\n\n";
        }
        
        // 복사 생성자
        {
            shared_ptr<TestObjectEx> objEx3 = objEx1;
        
            cout << "Copy : shared_ptr<TestObjectEx> objEx3 = objEx1;\n\n";
        }
        
        cout << "## End Of Block ##\n\n";

        // 스마트 포인터를 복사한다고 해서 최초 스마트 포인터를 생성했을 때 만든 Allocator를 새로 만들지는 않는다.
        // 스마트 포인터를 복사할 때 레퍼런스 카운팅이 이루어지긴 하지만 기본적으로는 관리 객체와 컨트롤 블록의 포인터를 복사하는 형태이다.
        // !! 이 부분에 대한 내용은 "Copy Constructor"와 "Copy Assignment Operator"에서 이루어지는 과정을 통해 보도록 함. !!
    }

    cout << "-------------------------#11#-------------------------\n\n";

    /*******************************
    *      Deleter on nullptr      *
    *******************************/

    // 관리 객체의 소멸 과정(_Destroy())이나 컨트롤 블록이 소멸되는 방식(_Delete_this())은
    // 가상 함수 인터페이스를 구현한 방식에 따라 다르다.
    // 
    // (중요) 하지만 _Ptr_base와 _Ref_count_base를 보면 스마트 포인터 차원에서 조건을 걸고 가상 함수를 호출하는 방식은 동일하다.
    // !! 관리 객체가 소멸되어야 하는 조건에 맞으면 _Destroy()를 호출, 컨트롤 블록이 소멸되어야 하는 조건에 맞으면 _Delete_this()를 호출하는 식임. !!
    // 
    // 컨트롤 블록 _Rep가 nullptr인 것과 스마트 포인터에 nullptr을 대입하는 건 느낌이 조금 다르다.
    // _Rep가 nullptr이라는 건 컨트롤 블록이 비어 있다는 뜻이자 빈 스마트 포인터임을 나타낸다.
    // !! 반면에 스마트 포인터에 nullptr을 대입하는 건 관리 객체를 nullptr로 지정하겠다는 의미이다. !!
    //
    // 가상 함수를 호출하는 방식 자체는 동일하다.
    // 하지만 파생 클래스에서 구체화된 가상 함수에서 다루는 내용이 다르기 때문에 여러 방면으로 분석하는 것이다.
    // !! 가상 함수에서 구체화된 내용만 다를 뿐 호출되기 이전의 과정 자체는 어떤 컨트롤 블록을 쓰든 동일함. !!
    
    // 일반적인 경우, 관리 객체가 없는 상태라면 컨트롤 블록은 비어 있을 것이다(대부분의 경우가 그럼).
    // 여기서는 다루지 않지만 관리 객체를 nullptr로 하고, 컨트롤 블록을 할당하는 방식의 스마트 포인터도 생성할 수 있다.
    //
    // 예를 들어 (nullptr, Deleter, Allocator) 방식으로 스마트 포인터를 생성하면?
    // 신기하게도 관리 객체는 nullptr인데 컨트롤 블록은 할당된 스마트 포인터가 생성된다.
    // !! 좀 특이한 케이스인데 이 경우에는 Deleter에서 nullptr 유무를 체크하고 관리 객체를 반환해야 함. !!
    //
    // 관리 객체를 nullptr로 지정하고 컨트롤 블록을 할당하는 유형은 앞으로도 평생 안 쓸 것 같지만
    // 혹시 몰라서 shared_ptr_with_deleter_and_allocator.cpp에 예시 코드는 적어 두었다.
    // !! 그냥 이런 것도 되긴 하는구나 하고 넘어가자. !!

    // 사용 도중에 nullptr을 대입했을 경우, nullptr을 가리키는 스마트 포인터가 Deleter를 호출하는지 확인하기 위한 코드
    {
        // RVO에 의해 대상 메모리 공간에 직접적으로 Ptr, Deleter를 받는 생성자가 적용됨.
        shared_ptr<TestObjectEx> objEx1 = MyMakeSharedWithDeleter<TestObjectEx>(100, 3.141592);
        
        cout << "# Separator #\n\n";
        
        {
            // 복사 생성자
            shared_ptr<TestObjectEx> objEx2 = objEx1;
        
            // shared_ptr(nullptr_t) 생성자로 임시 객체 생성 후, 이를 objEx1에 반영하기 위한 이동 대입 연산자가 호출됨.
            objEx1 = nullptr; // nullptr은 암시적으로 shared_ptr{ nullptr }로 변환됨.
        
            cout << "assign nullptr...\n\n";
        
            // 해당 블록을 빠져나오면 shared_ptr의 소멸자를 거치며 컨트롤 블록을 통해 관리 객체의 소멸자를 호출함.
        }
        
        cout << "## End Of Block ##\n\n";

        // End Of Block 이후 Deleter가 호출되진 않음.
        // 사용 도중 objEx1에 nullptr을 대입해도 문제 없음.

        // "objEx1 = nullptr"에서 nullptr은 암시적으로 shared_ptr{ nullptr }로 변환된다.
        // 
        // --------------------------------------------------
        // 
        // 1. 암시적으로 코드가 변환되면 수행되는 코드는 다음과 같다.
        // 
        // objEx1 = shared_ptr{ nullptr };
        // 
        // !! nullptr을 받는 shared_ptr의 생성자 !!
        // constexpr shared_ptr(nullptr_t) noexcept {} // construct empty shared_ptr
        // 
        // --------------------------------------------------
        // 
        // 2. "objEx1 = shared_ptr{ nullptr }" 과정에서 이동 대입 연산자 호출
        // 
        // shared_ptr& operator=(shared_ptr&& _Right) noexcept // take resource from _Right
        // {
        //     shared_ptr(_STD move(_Right)).swap(*this);
        //     return *this;
        // }
        // 
        // !! 임시 객체에 소유권을 이전하기 위해 move(_Right)를 적용하였기에 이동 생성자 호출 !!
        // shared_ptr(shared_ptr&& _Right) noexcept // construct shared_ptr object that takes resource from _Right
        // {
        //     // _Right는 shared_ptr{ nullptr }임.
        //     this->_Move_construct_from(_STD move(_Right));
        // }
        // 
        // !! _Move_construct_from(_STD move(_Right)) 호출 !!
        // template <class _Ty2>
        // void _Ptr_base<T>::_Move_construct_from(_Ptr_base<_Ty2>&& _Right) noexcept
        // {
        //     // implement shared_ptr's (converting) move ctor and weak_ptr
        //     // !! 스마트 포인터의 소유권(관리 객체와 컨트롤 블록)을 임시 객체에 넘김. !!
        //     _Ptr = _Right._Ptr;
        //     _Rep = _Right._Rep;
        // 
        //     // !! 전달 대상이 된 기존 스마트 포인터의 소유권을 날림. !!
        //     _Right._Ptr = nullptr;
        //     _Right._Rep = nullptr;
        // 
        //     // this의 _Ptr과 _Rep, _Right의 _Ptr과 _Rep는 nullptr임.
        // }
        // 
        // --------------------------------------------------
        // 
        // 3. shared_ptr{ nullptr }의 소유권을 임시 객체에 전달한 다음 swap()을 호출
        // 
        // void shared_ptr<T>::swap(shared_ptr& _Other) noexcept
        // {
        //     // _Other는 "objEx1 = shared_ptr{ nullptr }" 중 objEx1에 해당하며, _Swap()을 호출하고 있는 this는 임시 객체임.
        //     this->_Swap(_Other);
        // }
        // 
        // void _Ptr_base<T>::_Swap(_Ptr_base& _Right) noexcept // swap pointers
        // {
        //     // 교환 전 상태 : this{ _Ptr : nullptr }, _Right{ _Ptr : 100, strong ptr : 2 }
        //     // 교환 후 상태 : this{ _Ptr : 100, strong ptr : 2 }, _Right{ _Ptr : nullptr }
        //     _STD swap(_Ptr, _Right._Ptr);
        //     _STD swap(_Rep, _Right._Rep);
        // 
        //     // "shared_ptr<TestObjectEx> objEx2 = objEx1"로 복사 생성자를 호출했기 때문에 레퍼런스 카운팅이 증가한 상태임.
        // }
        // 
        // --------------------------------------------------
        // 
        // 4. 콜 스택을 빠져나오고 임시 객체의 소멸자 호출
        // 
        // ~shared_ptr() noexcept // release resource
        // {
        //     this->_Decref();
        // }
        // 
        // void _Ptr_base<T>::_Decref() noexcept // decrement reference count
        // {
        //     // objEx1과 임시 객체는 서로 내용을 교환한 생태임.
        //     // objEx1에는 shared_ptr{ nullptr }이 반영되었지만, 임시 객체는 objEx1이 가지고 있던 것을 들고 있음.
        //     // 따라서 교환 작업을 수행한 임시 객체는 _Rep가 유효함.
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // void _Ref_count_base::_Decref() noexcept // decrement use count
        // {    
        //     // _Uses의 값이 2이기 때문에 1을 감소시켜도 조건에 충족하지 않음.
        //     if (_MT_DECR(_Uses) == 0) {
        //         _Destroy();
        //         _Decwref();
        //     }
        // }
        // 
        // --------------------------------------------------
        // 
        // 5. 내부 스코프를 빠져나오면서 objEx2의 소멸자 호출
        // 
        // ~shared_ptr() noexcept // release resource
        // {
        //     this->_Decref();
        // }
        // 
        // void _Ptr_base<T>::_Decref() noexcept // decrement reference count
        // {
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // void _Ref_count_base::_Decref() noexcept // decrement use count
        // {
        //     // _Uses의 값이 1이기 때문에 1을 감소시키면 0에 도달하여 조건에 충족함.
        //     // 따라서 _Destroy()와 _Decwref()가 호출됨.
        //     if (_MT_DECR(_Uses) == 0) {
        //         // !! 컨트롤 블록에 재정의된 _Destroy()와 _Delete_this() 호출 !!
        //         _Destroy();
        //         _Decwref();
        //     }
        // }
        // 
        // --------------------------------------------------
        // 
        // ***** 블록을 빠져나오는 과정에서 소멸자 호출 진행 과정 *****
        // 
        // --------------------------------------------------
        // 
        // 6. objEx1의 소멸자 호출
        // 
        // ~shared_ptr() noexcept // release resource
        // {
        //     this->_Decref();
        // }
        // 
        // void _Ptr_base<T>::_Decref() noexcept // decrement reference count
        // {
        //     // "objEx1 = shared_ptr{ nullptr }" 과정에서 빈 스마트 포인터를 objEx1에 반영했다.
        //     // _Rep는 nullptr이기 때문에 _Rep->_Decref()는 호출되지 않음.
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
    }

    cout << "-------------------------#12#-------------------------\n\n";
    
    /************************************
    *      deallocate() on nullptr      *
    ************************************/

    // 사용 도중에 nullptr을 대입했을 경우, nullptr을 가리키는 스마트 포인터가 deallocate()를 호출하는지 확인하기 위한 코드
    {
        // RVO에 의해 대상 메모리 공간에 직접적으로 Allocator를 받는 생성자가 적용됨.
        shared_ptr<TestObjectEx> objEx1 = MyMakeSharedWithAllocator<TestObjectEx>(100, 3.141592);
        
        cout << "# Separator #\n\n";
        
        {
            // 복사 생성자
            shared_ptr<TestObjectEx> objEx2 = objEx1;
        
            // shared_ptr(nullptr_t) 생성자로 임시 객체 생성 후, 이를 objEx1에 반영하기 위한 이동 대입 연산자가 호출됨.
            objEx1 = nullptr; // nullptr은 암시적으로 shared_ptr{ nullptr }로 변환됨.
        
            cout << "assign nullptr...\n\n";
            
            // 해당 블록을 빠져나오면 shared_ptr의 소멸자를 거치며 컨트롤 블록을 통해 관리 객체의 소멸자를 호출함.
        }
        
        cout << "## End Of Block ##\n\n";

        // End Of Block 이후 할당자의 deallocate()가 호출되진 않음.
        // 사용 도중 objEx1에 nullptr을 대입해도 문제 없음.

        // "objEx1 = nullptr"을 수행하면 objEx1은 빈 스마트 포인터가 된다.
        // 
        // 스마트 포인터가 할당자의 deallocator()를 호출하려면 먼저 _Delete_this()가 호출되어야 한다.
        // 
        // template <class _Ty, class _Alloc>
        // class _Ref_count_obj_alloc3 : public _Ebco_base<_Rebind_alloc_t<_Alloc, _Ty>>, public _Ref_count_base 
        // {
        //     ...
        //     void _Delete_this() noexcept override // destroy self
        //     {
        //         _Rebind_alloc_t<_Alloc, _Ref_count_obj_alloc3> _Al(this->_Get_val());
        //         this->~_Ref_count_obj_alloc3();
        // 
        //         // 할당자의 deallocate()는 _Deallocate_plain()을 수행하는 도중 호출됨.
        //         _Deallocate_plain(_Al, this);
        //     }
        // };
        // 
        // 하지만 objEx1은 빈 스마트 포인터이기 때문에 소멸자 호출 과정에서 _Decref()를 호출하지 않는다.
        // 
        // ~shared_ptr() noexcept // release resource
        // {
        //     this->_Decref();
        // }
        // 
        // void _Ptr_base<T>::_Decref() noexcept // decrement reference count
        // {
        //     // "objEx1 = shared_ptr{ nullptr }" 과정에서 빈 스마트 포인터를 objEx1에 반영했다.
        //     // _Rep는 nullptr이기 때문에 _Rep->_Decref()는 호출되지 않음.
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // !! 상단의 조건을 만족하지 않기 때문에 _Decref()는 호출되지 않음. !!
        // !! 즉, 컨트롤 블록에 재정의된 _Destroy()와 _Delete_this()는 호출되지 않음. !!
        // void _Ref_count_base::_Decref() noexcept // decrement use count
        // {
        //     if (_MT_DECR(_Uses) == 0) {
        //         _Destroy();
        //         _Decwref();
        //     }
        // }
        // 
        // 이런 이유로 objEx1의 소멸자는 할당자의 deallocator()를 호출하지 않는다.
        // 
    }

    cout << "-------------------------#13#-------------------------\n\n";

    /**********************************************
    *      Deleter & deallocate() on nullptr      *
    **********************************************/

    // 사용 도중에 nullptr을 대입했을 경우, nullptr을 가리키는 스마트 포인터가 Deleter나 deallocate()를 호출하는지 확인하기 위한 코드
    {
        // RVO에 의해 대상 메모리 공간에 직접적으로 Ptr, Deleter, Allocator를 받는 생성자가 적용됨.
        shared_ptr<TestObjectEx> objEx1 = MyMakeSharedWithDeleterAllocator_Simple<TestObjectEx>(100, 3.141592);
        
        cout << "# Separator #\n\n";
        
        {
            // 복사 생성자
            shared_ptr<TestObjectEx> objEx2 = objEx1;
        
            // shared_ptr(nullptr_t) 생성자로 임시 객체 생성 후, 이를 objEx1에 반영하기 위한 이동 대입 연산자가 호출됨.
            objEx1 = nullptr; // nullptr은 암시적으로 shared_ptr{ nullptr }로 변환됨.
        
            cout << "assign nullptr...\n\n";
        
            // 해당 블록을 빠져나오면 shared_ptr의 소멸자를 거치며 컨트롤 블록을 통해 관리 객체의 소멸자를 호출함.
        }
        
        cout << "## End Of Block ##\n\n";

        // End Of Block 이후 전달한 Deleter나 할당자의 deallocate()가 호출되진 않음.
        // 사용 도중 스마트 포인터에 nullptr을 대입해도 문제 없음.

        // !! 동일한 내용이 "deallocate() on nullptr"에도 적혀 있음. !!
    }

    cout << "-------------------------#14#-------------------------\n\n";

    /**************************************************************************
    *      Up Casting by static_pointer_cast<TestObject, TestObjectEx>()      *
    **************************************************************************/

    // 순수 가상 함수라는 인터페이스를 타고 들어가는 로직 자체는 동일하다.
    // 따라서 어떤 유형으로 스마트 포인터를 생성하든 상관 없다.

    // 업 캐스팅하여 스마트 포인터를 사용할 경우 소멸 과정이 어떻게 유도되는지 관찰하기 위한 코드
    {
        shared_ptr<TestObject> obj;
        
        {
            // 간단하게 로직만 분석할 것이기 때문에 make_shared<T>()를 사용함.
            shared_ptr<TestObjectEx> objEx = make_shared<TestObjectEx>(123, 3.14);
        
            // 인자로 넘긴 objEx의 관리 객체의 주소를 부모로 업 캐스팅하여 shared_ptr<TestObject>를 생성하고
            // 이를 obj에 반영하기 위한 이동 대입 연산자가 호출됨.
            obj = static_pointer_cast<TestObject>(objEx);
        }
        
        cout << "after static_pointer_cast...\n\n";
        
        obj->Print();

        // 블록을 빠져나올 때 관리 객체의 소멸자는 shared_ptr<TestObject>의 소멸자를 통해서 호출된다.
        // shared_ptr<TestObjectEx>를 통해서 넘겼다고 해도 최종적으로 호출되는 건 shared_ptr<TestObject>의 소멸자이다.
        // 스마트 포인터의 소멸자가 동작하는 방식은 컨트롤 블록에 의존적이다.
        // 즉, shared_ptr<TestObject>의 소멸자를 거쳤다고 해도 이게 ~TestObject()의 호출과 연결되는 것은 아니다.
        // 
        // "shared_ptr<TestObject> obj"가 전달받은 관리 객체는 shared_ptr<TestObjectEx>를 기반으로 하고 있다.
        // !! shared_ptr<TestObjectEx>가 make_shared<T>()로 받은 스마트 포인터의 관리 객체는 TestObjectEx임. !!
        // 
        // (중요) shared_ptr<TestObject>와 TestObject는 아예 다른 별개의 객체이다.
        // 
        // "shared_ptr<TestObject> obj"는 관리 객체를 static_pointer_cast<T1, T2>()로 캐스팅하여 받았다.
        // 캐스팅 되는 대상은 shared_ptr<TestObjectEx>이며 관리 객체와 컨트롤 블록은 TestObjectEx를 기반으로 하고 있다.
        // 
        // 쉽게 생각해서 shared_ptr<TestObject>의 소멸자를 거친다고 해도 컨트롤 블록을 거치며 호출되는 관리 객체의 소멸자는 ~TestObjectEx()이다.
        // 
        // 가상 함수 테이블을 타지 않고 컨트롤 블록 차원에서 직접적으로 생성한 객체의 소멸자를 호출한다.
        // 부모로 업 캐스팅 했다고 해도 파생 클래스의 소멸자를 직접 호출하는 방식이기 때문에 스마트 포인터로 관리되는 객체의 소멸자에는 virtual이 안 붙어도 된다.
        // !! 가상 함수 타이블을 타고 들어가는 방식으로 소멸자를 호출하는 방식이 아님. !!
        // 
        // --------------------------------------------------
        // 
        // obj = static_pointer_cast<TestObject>(objEx);
        // 
        // 위 코드로 objEx를 캐스팅하여 obj에 적용하는 과정과 블록을 빠져나오면서 호출되는 obj의 소멸자를 관찰할 것이다.
        // 
        // --------------------------------------------------
        // 
        // 1. static_pointer_cast<T1, T2>()를 호출
        // 
        // _EXPORT_STD template <class _Ty1, class _Ty2>
        // _NODISCARD shared_ptr<_Ty1> static_pointer_cast(const shared_ptr<_Ty2>& _Other) noexcept
        // {
        //     // static_cast for shared_ptr that properly respects the reference count control block
        //     // _Other.get()으로 관리 객체의 포인터를 받고 static_cast<T1>()으로 캐스팅함.
        //     const auto _Ptr = static_cast<typename shared_ptr<_Ty1>::element_type*>(_Other.get());
        // 
        //     // _Other는 캐스팅 되는 대상을 담은 스마트 포인터(shared_ptr<T2>),
        //     // _Ptr은 캐스팅한 관리 객체의 포인터를 담은 변수(T1*)임.
        //     // 
        //     // _Other는 shared_ptr이기 때문에 _Ptr(관리 객체의 포인터)과 _Rep(컨트롤 블록)을 가짐(_Other의 _Ptr과 함수 상단에 있는 _Ptr은 다름).
        //     // 함수 상단에 있는 _Ptr은 관리 객체의 포인터를 캐스팅한 것일 뿐이지 이것 자체는 컨트롤 블록을 가지고 있지 않음.
        //     //
        //     // 캐스팅한 포인터와 컨트롤 블록이 분리되어 있기 때문에 이를 묶는 작업이 필요함.
        //     return shared_ptr<_Ty1>(_Other, _Ptr); // _Other과 _Ptr을 묶어서 shared_ptr를 생성
        // }
        // 
        // 여기서는 일반적인 참조 형태를 받는 static_pointer_cast<T1, T2>()를 사용하지만 
        // 보편 참조를 받는 static_pointer_cast(shared_ptr<_Ty2>&& _Other) 유형도 존재한다.
        // 
        // --------------------------------------------------
        // 
        // 2. shared_ptr<T1>의 생성자 호출
        // 
        // template <class _Ty2>
        // shared_ptr(const shared_ptr<_Ty2>& _Right, element_type* _Px) noexcept
        // {
        //     // construct shared_ptr object that aliases _Right
        //     // _Right는 컨트롤 블록이 있는 스마트 포인터, _Px는 관리 객체를 캐스팅한 포인터임.
        //     this->_Alias_construct_from(_Right, _Px);
        // }
        // 
        // --------------------------------------------------
        // 
        // 3._Alias_construct_from() 호출
        // 
        // template <class _Ty2>
        // void _Ptr_base<T1>::_Alias_construct_from(const shared_ptr<_Ty2>& _Other, element_type* _Px) noexcept
        // {
        //     // implement shared_ptr's aliasing ctor
        //     // 컨트롤 블록의 _Uses 값을 1 증가시킴.
        //     _Other._Incref();
        // 
        //     _Ptr = _Px;         // 캐스팅한 관리 객체의 포인터를 적용
        //     _Rep = _Other._Rep; // 최초 shared_ptr를 만들었을 때 생성한 컨트롤 블록을 적용(스마트 포인터를 복사하지 않고 컨트롤 블록만 가지고 옮)
        // }
        // 
        // --------------------------------------------------
        // 
        // 4. 이동 대입 연산자로 obj에 캐스팅 결과 반영
        // 
        // shared_ptr& operator=(shared_ptr&& _Right) noexcept // take resource from _Right
        // {
        //     shared_ptr(_STD move(_Right)).swap(*this);
        //     return *this;
        // }
        // 

        // _Uses가 0으로 떨어지면 최초 스마트 포인터를 만들었을 때 생성한 컨트롤 블록을 타고 관리 객체의 소멸자를 호출한다.
        // 
        // (중요) 관리 객체의 소멸자 호출은 캐스팅 타입에 의존적인 것이 아닌 컨트롤 블록에 의존적이다.
        // 
        // --------------------------------------------------
        // 
        // ***** 블록을 빠져나오는 과정에서 소멸자 호출 진행 과정 *****
        // 
        // --------------------------------------------------
        // 
        // 1. obj의 소멸자 호출
        // 
        // ~shared_ptr() noexcept // release resource
        // {
        //     this->_Decref();
        // }
        // 
        // void _Ptr_base<T>::_Decref() noexcept // decrement reference count
        // {
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
        // void _Ref_count_base::_Decref() noexcept // decrement use count
        // {
        //     // _Uses의 값이 1이기 때문에 1을 감소시키면 0에 도달하여 조건에 충족함.
        //     if (_MT_DECR(_Uses) == 0) {
        //         // !! 컨트롤 블록에 재정의된 _Destroy() 호출 !!
        //         _Destroy();
        //         _Decwref();
        //     }
        // }
        // 
        // void _Ref_count_base::_Decwref() noexcept // decrement weak reference count
        // {
        //     // _Weaks의 값이 1이기 때문에 1을 감소시키면 0에 도달하여 조건에 충족함.
        //     if (_MT_DECR(_Weaks) == 0) {
        //         // !! 컨트롤 블록에 재정의된 _Delete_this() 호출 !!
        //         _Delete_this();
        //     }
        // }
        // 
        // --------------------------------------------------
        // 
        // 2. make_shared<T>()로 스마트 포인터를 생성했기 때문에 컨트롤 블록은 _Ref_count_obj2임.
        // 
        // template <class _Ty>
        // class _Ref_count_obj2 : public _Ref_count_base
        // {
        //     ...
        //     void _Destroy() noexcept override // destroy managed resource
        //     {
        //         // shared_ptr의 타입에 의존해서 소멸자를 호출하는 것이 아닌
        //         // 최초 컨트롤 블록이 생성되었을 때 지정한 관리 객체의 타입에 맞게 소멸자를 호출함.
        //         _Destroy_in_place(_Storage._Value);
        //     }
        // 
        //     void _Delete_this() noexcept override // destroy self
        //     {
        //         delete this;
        //     }
        // };
        // 
        // template <class _Ty>
        // _CONSTEXPR20 void _Destroy_in_place(_Ty& _Obj) noexcept 
        // {
        //     // !! 컴파일 타임에서 해당 부분은 실행되지 않게 걸러짐. !!
        //     if constexpr (is_array_v<_Ty>) 
        //     {
        //         _Destroy_range(_Obj, _Obj + extent_v<_Ty>);
        //     } 
        //     else 
        //     {
        //         // !! 이쪽 로직이 실행됨. !! //
        //         _Obj.~_Ty();
        //     }
        // }
        // 
        // _Ref_count_obj2<T>의 T는 최초 shared_ptr을 생성했을 때 지정한 타입이다.
        // 따라서 _Destroy()에서 _Destroy_in_place()에 전달하는 _Storage._Value는 이 지정된 타입을 기반으로 한다.
        // 
        // 이런 이유로 shared_ptr<TestObject>의 소멸자가 호출된다고 해도
        // 최초 지정한 타입 T가 TestObjectEx이기 때문에 컨트롤 블록에 의해 TestObjectEx의 소멸자가 호출되는 것이다.
        // 
        // 이런 느낌으로 호출된다고 보면 된다.
        // 
        // void _Destroy_in_place(TestObjectEx& _Obj) noexcept
        // {
        //     _Obj.~TestObjectEx();
        // }
        // 

        cout << "## End Of Block ##\n\n";
    }

    // 관리 객체의 소멸 과정은 스마트 포인터가 사용하는 관리 객체의 타입이 아닌 컨트롤 블록에 영향을 받는다.
    // 쉽게 말해서 컨트롤 블록에는 생성했을 당시의 관리 객체 타입이 묶여 있고, 이걸 기반으로 관리 객체의 소멸 과정을 유도하는 것이다.
    // 
    // (중요) 하위 객체의 타입이 컨트롤 블록에 묶여 있으면, 상위 타입으로 캐스팅해서 사용해도 하위 객체의 타입을 기반으로 소멸 되정이 유도된다.
    // 
    // 여기서는 make_shared<T>()로 생성한 것은 캐스팅하였지만 다른 유형으로 생성한 스마트 포인터를 캐스팅해도 원리 자체는 동일하다.

    cout << "-------------------------#15#-------------------------\n\n";

    /*****************************************
    *      Up Casting by Copy Operation      *
    *****************************************/

    // 업 캐스팅을 복사 연산자로 진행할 경우 어떤 과정을 거치는지 확인하기 위한 코드
    {
        shared_ptr<TestObjectEx> objEx1 = make_shared<TestObjectEx>(123, 3.14);
        
        cout << "# Separator #\n\n";
        
        // A) 복사 기반 변환 생성자
        {
            shared_ptr<TestObject> objEx2 = objEx1; // objEx2{ objEx1 };
        
            cout << "Copy : shared_ptr<TestObject> objEx2 = objEx1;\n\n";
        
            cout << "objEx1.use_count() : " << objEx1.use_count() << "\n\n";
        
            objEx2->Print();
        }
        
        cout << "# Separator #\n\n";
        
        // B) 복사 기반 변환 대입 연산자
        {
            shared_ptr<TestObject> objEx3; // 빈 스마트 포인터
        
            objEx3 = objEx1;
        
            cout << "Copy : objEx3 = objEx1;\n\n";
        
            cout << "objEx1.use_count() : " << objEx1.use_count() << "\n\n";
        
            objEx3->Print();
        }
        
        cout << "# Separator #\n\n";
        
        cout << "objEx1.use_count() : " << objEx1.use_count() << "\n\n";
        
        cout << "## End Of Block ##\n\n";

        // A와 B 과정에서 복사 연산이 어떻게 이루어지는지 확인할 것이다.
        //
        // --------------------------------------------------
        // 
        // ***** A 부분 *****
        //
        // --------------------------------------------------
        //
        // 1. shared_ptr<TestObject> objEx2 = objEx1; // objEx2{ objEx1 };
        // 
        // 위 과정을 거치며 다른 타입의 스마트 포인터를 받는 "복사 기반 변환 생성자"를 호출한다.
        // 
        // template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        // shared_ptr(const shared_ptr<_Ty2>& _Other) noexcept 
        // {
        //     // construct shared_ptr object that owns same resource as _Other
        //     this->_Copy_construct_from(_Other);
        // }
        // 
        // !! 컴파일 타임에서 템플릿 인자를 추론할 수 있게 _SP_pointer_compatible<T1, T2> 사용 !!
        // template <class _Yty, class _Ty>
        // struct _SP_pointer_compatible : is_convertible<_Yty*, _Ty*>::type 
        // {
        //     // N4950 [util.smartptr.shared.general]/6 "a pointer type Y* is said to be compatible
        //     // with a pointer type T* when either Y* is convertible to T* ..."
        // };
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
        // struct integral_constant {
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
        // https://learn.microsoft.com/ko-kr/cpp/extensions/compiler-support-for-type-traits-cpp-component-extensions?view=msvc-170
        // __is_convertible_to(from, to)는 MSVC에서 제공하는 기능이다.
        // 
        // --------------------------------------------------
        // 
        // 2. _Copy_construct_from() 호출
        // 
        // template <class _Ty2>
        // void _Ptr_base<T>::_Copy_construct_from(const shared_ptr<_Ty2>& _Other) noexcept
        // {
        //     // implement shared_ptr's (converting) copy ctor
        //     // !! _Other는 objEx1이고 이걸 대상으로 _Incref() 호출하여 _Uses를 1 증가시킴. !!
        //     _Other._Incref();
        // 
        //     // 레퍼런스 카운트 _Uses를 1 증가시킨 후 objEx2가 objEx1과 같은 관리 객체와 컨트롤 블록을 가지게 설정함.
        //     _Ptr = _Other._Ptr;
        //     _Rep = _Other._Rep;
        // }
        // 
        // --------------------------------------------------
        // 
        // ***** B 부분 *****
        //
        // --------------------------------------------------
        // 
        // 3. objEx3 = objEx1;
        // 
        // 위 과정을 거치며 다른 타입의 스마트 포인터를 받는 "복사 기반 변환 대입 연산자"를 호출한다.
        // 
        // template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        // shared_ptr& operator=(const shared_ptr<_Ty2>& _Right) noexcept 
        // {
        //     shared_ptr(_Right).swap(*this);
        //     return *this;
        // }
        // 
        // 템플릿 인자를 추론하는 과정은 A에 있는 내용과 동일하며,
        // 임시 객체를 생성하고 swap()을 호출하는 건 "Copy Assignment Operator"에 있는 내용과 동일하니 생략한다.
        // 
    }

    cout << "-------------------------#16#-------------------------\n\n";

    /*****************************************
    *      Up Casting by Move Operation      *
    *****************************************/

    // 업 캐스팅을 이동 연산자로 진행할 경우 어떤 과정을 거치는지 확인하기 위한 코드
    {
        shared_ptr<TestObjectEx> objEx1 = make_shared<TestObjectEx>(123, 3.14);
        
        cout << "# Separator #\n\n";
        
        // A) 이동 기반 변환 생성자
        {
            shared_ptr<TestObject> objEx2 = std::move(objEx1); // objEx2{ std::move(objEx1) };
        
            cout << "Move : shared_ptr<TestObject> objEx2 = std::move(objEx1);\n\n";
        
            cout << "objEx1.use_count() : " << objEx1.use_count() << " & ";
            cout << "objEx2.use_count() : " << objEx2.use_count() << "\n\n";
        
            objEx2->Print();
        }
        
        cout << "# Separator #\n\n";
        
        objEx1 = make_shared<TestObjectEx>(456, 6.28);
        
        cout << "# Separator #\n\n";
        
        // B) 이동 기반 변환 대입 연산자
        {
            shared_ptr<TestObject> objEx3; // 빈 스마트 포인터
        
            objEx3 = std::move(objEx1);
        
            cout << "Move : objEx3 = objEx1;\n\n";
        
            cout << "objEx1.use_count() : " << objEx1.use_count() << " & ";
            cout << "objEx3.use_count() : " << objEx3.use_count() << "\n\n";
        
            objEx3->Print();
        }
        
        cout << "## End Of Block ##\n\n";

        // A와 B 과정에서 이동 연산이 어떻게 이루어지는지 확인할 것이다.
        //
        // --------------------------------------------------
        // 
        // ***** A 부분 *****
        //
        // --------------------------------------------------
        //
        // 1. shared_ptr<TestObject> objEx2 = std::move(objEx1); // objEx2{ std::move(objEx1) };
        // 
        // 위 과정을 거치며 다른 타입의 스마트 포인터를 받는 "이동 기반 변환 생성자"를 호출한다.
        // 
        // template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        // shared_ptr(shared_ptr<_Ty2>&& _Right) noexcept // construct shared_ptr object that takes resource from _Right
        // {
        //     this->_Move_construct_from(_STD move(_Right));
        // }
        // 
        // 타입을 추론하는 과정은 "Up Casting by Copy Operation"에 있다.
        // 
        // --------------------------------------------------
        // 
        // 2. _Move_construct_from() 호출
        // template <class _Ty2>
        // void _Ptr_base<T>::_Move_construct_from(_Ptr_base<_Ty2>&& _Right) noexcept
        // {
        //     // implement shared_ptr's (converting) move ctor and weak_ptr's move ctor
        //     // !! 스마트 포인터의 소유권(관리 객체와 컨트롤 블록)을 objEx2에 넘김. !!
        //     _Ptr = _Right._Ptr;
        //     _Rep = _Right._Rep;
        // 
        //     // !! 전달 대상이 된 기존 스마트 포인터 objEx1의 소유권을 날림. !!
        //     _Right._Ptr = nullptr;
        //     _Right._Rep = nullptr;
        // }
        // 
        // --------------------------------------------------
        // 
        // ***** B 부분 *****
        //
        // --------------------------------------------------
        // 
        // 3. objEx3 = std::move(objEx1);
        // 
        // 위 과정을 거치며 다른 타입의 스마트 포인터를 받는 "이동 기반 변환 대입 연산자"를 호출한다.
        // 
        // template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        // shared_ptr& operator=(shared_ptr<_Ty2>&& _Right) noexcept // take resource from _Right
        // {
        //     shared_ptr(_STD move(_Right)).swap(*this);
        //     return *this;
        // }
        // 
        // 타입을 추론하는 과정은 "Up Casting by Copy Operation"에 있다.
        // 
        // !! 임시 객체에 소유권을 이전하기 위해 move(_Right)를 적용하였기에 이동 기반 변환 생성자 호출 !!
        // template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        // shared_ptr(shared_ptr<_Ty2>&& _Right) noexcept // construct shared_ptr object that takes resource from _Right
        // {
        //     this->_Move_construct_from(_STD move(_Right));
        // }
        // 
        // !! _Move_construct_from(_STD move(_Right)) 호출 !!
        // template <class _Ty2>
        // void _Ptr_base<T>::_Move_construct_from(_Ptr_base<_Ty2>&& _Right) noexcept
        // {
        //     // implement shared_ptr's (converting) move ctor and weak_ptr's move ctor
        //     // !! 스마트 포인터의 소유권(관리 객체와 컨트롤 블록)을 임시 객체에 넘김. !!
        //     _Ptr = _Right._Ptr;
        //     _Rep = _Right._Rep;
        // 
        //     // !! 전달 대상이 된 기존 스마트 포인터 objEx1의 소유권을 날림. !!
        //     _Right._Ptr = nullptr;
        //     _Right._Rep = nullptr;
        // }
        // 
        // --------------------------------------------------
        // 
        // 4. 임시 객체에 소유권을 전달한 다음 swap()을 호출
        // 
        // void shared_ptr<T>::swap(shared_ptr& _Other) noexcept
        // {
        //     // this는 스마트 포인터의 소유권을 양도 받은 대상임(이동 대입 연산자에 있는 this와 여기 있는 this는 다른 대상이니 주의할 것).
        //     // _Other는 "objEx3 = std::move(objEx1)" 중 objEx3에 해당하며, _Swap()을 호출하고 있는 this는 임시 객체임.
        //     this->_Swap(_Other);
        // }
        // 
        // void _Ptr_base<T>::_Swap(_Ptr_base& _Right) noexcept // swap pointers
        // {
        //     // objEx1이 임시 객체에 넘긴 소유권을 objEx3에 반영하는 단계임.
        //     _STD swap(_Ptr, _Right._Ptr);
        //     _STD swap(_Rep, _Right._Rep);
        // 
        //     // 위 코드가 실행되었으면 임시 객체는 obj1의 _Ptr과 _Rep를 가리키게 됨.
        //     //
        //     // swap()으로 교환하기 이전
        //     // : objEx1{ _Ptr : nullptr }, 임시 객체{ _Ptr : 100, strong ptr : 1 }, objEx3{ _Ptr : nullptr }
        //     //
        //     // swap()으로 교환한 이후
        //     // : objEx1{ _Ptr : nullptr }, 임시 객체{ _Ptr : nullptr }, objEx3{ _Ptr : 100, strong ptr : 1 }
        //     // 
        //     // objEx1은 임시 객체에 소유권을 넘기면서 _Ptr과 _Rep를 nullptr로 밀었음.
        // }
        // 
        // 
        // --------------------------------------------------
        // 
        // 5. 콜 스택을 빠져나오고 임시 객체의 소멸자 호출
        // 
        // template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        // shared_ptr& operator=(shared_ptr<_Ty2>&& _Right) noexcept // take resource from _Right
        // {
        //     // _Right의 _Ptr과 _Rep는 소유권을 임시 객체에 양도하면서 nullptr로 밀리고,
        //     // 임시 객체의 _Ptr과 _Rep는 objEx3의 것으로 교환된 상태임.
        //     shared_ptr(_STD move(_Right)).swap(*this);
        //     return *this;
        // }
        // 
        // _Right는 소유권을 양도하는 이동 대상이라 소멸자가 호출되지 않는다(애초에 참조 형태임).
        // 소멸자가 호출되는 건 _Right(objEx1)의 소유권을 이전 받은 임시 객체 쪽이다.
        // 
        // !! 임시 객체 shared_ptr(_STD move(_Right))의 소멸자를 호출 !!
        // ~shared_ptr() noexcept // release resource
        // {
        //     this->_Decref();
        // }
        // 
        // void _Ptr_base<T>::_Decref() noexcept // decrement reference count
        // {
        //     // 임시 객체는 objEx1의 소유권을 이전 받은 다음 objEx3와 소유권을 교환한 상태임.
        //     // objEx3는 빈 스마트 포인터였기 때문에 _Rep는 nullptr이라 아래 if 문 안에 있는 로직은 실행되지 않음.
        //     if (_Rep) {
        //         _Rep->_Decref();
        //     }
        // }
        // 
    }

    cout << "-------------------------#17#-------------------------\n\n";

    /*****************************************************************************
    *      Down Casting by static_pointer_cast<TestObject, TestObjectEx>()      *
    *****************************************************************************/

    // static_pointer_cast<T1, T2>()를 이용한 다운 캐스팅을 진행할 경우 어떤 과정을 거치는지 확인하기 위한 코드
    {
        // 업 캐스팅으로 받는다.
        // shared_ptr(shared_ptr<_Ty2>&& _Right)가 호출되기 때문에 이 부분은 "Up Casting by Move Operation"를 참고하도록 한다.
        shared_ptr<TestObject> sptr1 = make_shared<TestObjectEx>(123, 3.14);
        
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

        // !! 캐스팅 과정 자체는 "Up Casting by static_pointer_cast<TestObject, TestObjectEx>()"에 나온 내용과 동일함. !!

        cout << "## End Of Block ##\n\n";
    }

    cout << "-------------------------#18#-------------------------\n\n";

    /*****************************************************************************
    *      Down Casting by dynamic_pointer_cast<TestObject, TestObjectEx>()      *
    *****************************************************************************/

    // dynamic_pointer_cast<T1, T2>()를 이용한 다운 캐스팅을 진행할 경우 어떤 과정을 거치는지 확인하기 위한 코드
    {
        // 업 캐스팅으로 받는다.
        // shared_ptr(shared_ptr<_Ty2>&& _Right)가 호출되기 때문에 이 부분은 "Up Casting by Move Operation"를 참고하도록 한다.
        shared_ptr<TestObject> sptr1 = make_shared<TestObjectEx>(123, 3.14);

        sptr1->Print();

        // TEST
        // sptr1 = nullptr;

        // A) dynamic_pointer_cast<T1, T2>()를 활용한 다운 캐스팅 진행
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

        // B) 정의되지 않은 상속 관계로 캐스팅을 시도하면 빈 스마트 포인터를 반환한다.
        // static_pointer_cast<T1, T2>()와 달리 컴파일 타임에 에러가 발생하지 않는다.
        // !! dynamic_pointer_cast<T1, T2>()는 dynamic_cast<T1>(T2)와 유사하게 동작함. !!
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

        // 유효한 다운 캐스팅을 진행했을 때와 유효하지 않은 다운 캐스팅을 진행했을 때
        // 연산이 어떻게 이루어지는지 확인할 것이다.
        //
        // --------------------------------------------------
        // 
        // 1. dynamic_pointer_cast<TestObjectEx>(sptr1) 호출
        // 
        // _EXPORT_STD template <class _Ty1, class _Ty2>
        // _NODISCARD shared_ptr<_Ty1> dynamic_pointer_cast(const shared_ptr<_Ty2>& _Other) noexcept 
        // {
        //     // dynamic_cast for shared_ptr that properly respects the reference count control block
        //     // 대상 스마트 포인터에서 관리 객체의 포인터를 가져와 dynamic_cast<T1>()을 진행함.
        //     const auto _Ptr = dynamic_cast<typename shared_ptr<_Ty1>::element_type*>(_Other.get());
        // 
        //     // A) 부분
        //     // 포인터가 유효하다면 dynamic_cast가 적용된 포인터를 묶어서 새로운 shared_ptr을 구성하고 반환함.
        //     if (_Ptr) {
        //         return shared_ptr<_Ty1>(_Other, _Ptr);
        //     }
        // 
        //     // B) 부분
        //     // 포인터가 유효하지 않다면 빈 스마트 포인터를 반환함.
        //     return {};
        // }
        // 
        // 여기서는 일반적인 참조 형태를 받는 dynamic_pointer_cast<T1, T2>()를 사용하지만 
        // 보편 참조를 받는 dynamic_pointer_cast(shared_ptr<_Ty2>&& _Other) 유형도 존재한다.
        // 
        // --------------------------------------------------
        // 
        // 2. shared_ptr<TestObjectEx> sptr2 = dynamic_pointer_cast<TestObjectEx>(sptr1);
        // 
        // RVO에 의해 dynamic_pointer_cast<T1, T2>()가 반환한 결과가 sptr2에 반영된다.
        // 
        // 만약 다음과 같이 RVO가 적용되지 않는 코드를 작성한다면 이동 대입 연산자가 호출된다.
        // 
        // shared_ptr<TestObjectEx> sptr2;
        // sptr2 = dynamic_pointer_cast<TestObjectEx>(sptr1);
        // 
        // shared_ptr& operator=(shared_ptr&& _Right) noexcept // take resource from _Right
        // {
        //     shared_ptr(_STD move(_Right)).swap(*this);
        //     return *this;
        // }
        // 
        // --------------------------------------------------
        // 
        // 3. shared_ptr<FooObject> sptr3 = dynamic_pointer_cast<FooObject>(sptr1);
        // 
        // 위 과정은 1번, 2번과 유사하다.
        // 다만 유효하지 않은 상속 관계이기 때문에 빈 스마트 포인터가 반환된다.
        // 
    }

    // static_pointer_cast<T1, T2>(), dynamic_pointer_cast<T1, T2>() 외에
    // const_pointer_cast<T1, T2>()와 reinterpret_pointer_cast<T1, T2>()도 존재한다.
    // 
    // C++에서 지원하는 기본적인 캐스팅 관련 연산은 스마트 포인터에도 비슷하게 정의되어 있다.

    cout << "------------------------------------------------------\n\n";

    return 0;
}
