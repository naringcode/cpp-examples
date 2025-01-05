// Update Date : 2024-12-26
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <memory>

#include <utility> // index_sequence

using namespace std;

// !! 자세한 내용은 shared_ptr_details.cpp에 적었다. !!
// !! 아래 코드는 간소화하여 할당자를 적용하는 방법을 기술하였다. !!

// 순서대로 볼 것
// 
// # shared_ptr을 사용할 경우 알아야 할 기본적인 내용
// 1. shared_ptr_with_deleter.cpp
// 2. shared_ptr_with_allocator.cpp <-----
// 3. shared_ptr_with_deleter_and_allocator.cpp
// 4. (중요) shared_ptr_details.cpp (SFINAE 내용 포함)volatile_atomic_cache_coherence_and_memory_order.cpp
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

/****************************************
*      Custom Make Shared Function      *
****************************************/

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

/*****************
*      Main      *
*****************/

int main()
{
    // !! Visual Studio 2022 기준이며 아래 분석 내용은 컴파일러 버전에 따라 달라질 수 있음. !!

    cout << "-------------------------#01#-------------------------\n\n";

    // Allocator를 생성자의 인자로 넘겨 스마트 포인터를 생성하는 방식
    {
        // RVO에 의해 대상 메모리 공간에 직접적으로 Allocator를 받는 생성자가 적용됨.
        shared_ptr<TestObjectEx> objEx = MyMakeSharedWithAllocator<TestObjectEx>(100, 3.141592);

        cout << "## End Of Block ##\n\n";
    }

    g_IdCounter = 0;

    cout << "-------------------------#02#-------------------------\n\n";

    // 스마트 포인터를 복사할 때, 생성 당시 만든 Allocator까지 복사해서 새로 만드는지 확인하기 위한 코드
    {
        // RVO에 의해 대상 메모리 공간에 직접적으로 Allocator를 받는 생성자가 적용됨.
        shared_ptr<TestObjectEx> objEx1 = MyMakeSharedWithAllocator<TestObjectEx>(100, 3.141592);

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

        // 스마트 포인터를 복사하는 과정을 거친다고 해도 Allocator를 따로 생성하진 않는다.
        // 스마트 포인터를 통해 관리되는 컨트롤 블록(_Ref_count_obj_alloc3)은 생성될 때 전달한 Allocator를 멤버 변수로 담는다.
        // Allocator는 컨트롤 블록에 묶여서 관리되는 형태이기 때문에 복사하는 과정에서 별도로 Allocator를 생성하지 않아도 된다.
        //
        // C++에서 스마트 포인터가 사용하는 컨트롤 블록의 유형은 다양한데,
        // 이건 해당 예시에 적용된 컨트롤 블록 _Ref_count_obj_alloc3의 특성이라고 보는 것이 맞다.
        //
        // template <class _Ty, class _Alloc>
        // class _Ref_count_obj_alloc3 : 
        //     public _Ebco_base<_Rebind_alloc_t<_Alloc, _Ty>>, // Allocator가 멤버로 있음. 
        //     public _Ref_count_base // _Uses와 _Weaks가 멤버로 있음(이게 _Rep의 원형).
        // {
        //     ...
        //     union {
        //         // 사용하고자 하는 관리 객체의 메모리 확보 용도(생성자 호출 안 함(메모리만 확보))
        //         // 컨트롤 블록을 할당할 때 관리 객체의 메모리가 함께 할당되는 형태임.
        //         _Wrap<_Ty> _Storage;
        //     };
        //     ...
        // };
        // 
        // template <class _Ty>
        // class _Ebco_base<_Ty, false> // std::_Ebco_base<MyAllocator<TestObjectEx>, 0>
        // {
        // private:
        //     _Ty _Myval; // Allocator(할당자는 상속을 통해 멤버 변수의 형태로 컨트롤 블록에 묶임)
        //     ...
        // };
        // 
        // 복사 과정을 보면...
        // 
        // !! 복사 생성자 호출 !!
        // shared_ptr(const shared_ptr& _Other) noexcept { // construct shared_ptr object that owns same resource as _Other
        //     this->_Copy_construct_from(_Other);
        // }
        // 
        // template <class _Ty2>
        // void _Copy_construct_from(const shared_ptr<_Ty2>& _Other) noexcept {
        //     // implement shared_ptr's (converting) copy ctor
        //     _Other._Incref(); // _Rep의 _Uses(strong refs)를 1 증가시킨다.
        // 
        //     _Ptr = _Other._Ptr; // 관리 객체의 메모리(_Storage._Value의 주소를 지정하여 사용함)
        //     _Rep = _Other._Rep; // 컨트롤 블록(컨트롤 블록에는 Allocator가 묶여 있음)
        // }
        // 
        // 단순 포인터 복사 방식으로 동작한다는 것을 알 수 있다.
        // 즉, Allocator가 묶인 컨트롤 블록을 공유하는 개념이기에 Allocator를 따로 복사해서 생성할 필요가 없다.
    }

    g_IdCounter = 0;

    cout << "-------------------------#03#-------------------------\n\n";

    // 사용 도중에 nullptr을 대입했을 경우, nullptr을 가리키는 스마트 포인터가 deallocate()를 호출하는지 확인하기 위한 코드
    {
        // RVO에 의해 대상 메모리 공간에 직접적으로 Allocator를 받는 생성자가 적용됨.
        shared_ptr<TestObjectEx> objEx1 = MyMakeSharedWithAllocator<TestObjectEx>(100, 3.141592);

        {
            // 복사 생성자
            shared_ptr<TestObjectEx> objEx2 = objEx1;

            // shared_ptr(nullptr_t) 생성자로 임시 객체 생성 후, 이를 objEx1에 반영하기 위한 이동 대입 연산자가 호출됨.
            objEx1 = nullptr; // nullptr은 암시적으로 shared_ptr{ nullptr }로 변환됨.

            cout << "assign nullptr...\n\n";
            
            // 해당 블록을 빠져나오면 shared_ptr의 소멸자를 거치며 관리 객체의 소멸자를 호출함.
        }

        cout << "## End Of Block ##\n\n";

        // End Of Block 이후 할당자의 deallocate()가 호출되진 않음.
        // 사용 도중 objEx1에 nullptr을 대입해도 문제 없음.
    }

    g_IdCounter = 0;

    cout << "-------------------------#04#-------------------------\n\n";

    // 업 캐스팅하여 스마트 포인터를 사용할 경우 어떤 소멸자가 호출되는지 관찰하기 위한 코드
    {
        shared_ptr<TestObject> obj;

        {
            // RVO에 의해 대상 메모리 공간에 직접적으로 Allocator를 받는 생성자가 적용됨.
            shared_ptr<TestObjectEx> objEx = MyMakeSharedWithAllocator<TestObjectEx>(300UL, 2.718281f);

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

    g_IdCounter = 0;
    
    cout << "-------------------------#05#-------------------------\n\n";

    // static_pointer_cast<T1, T2>()를 이용한 다운 캐스팅
    {
        // 업 캐스팅으로 받는다.
        shared_ptr<TestObject> sptr1 = MyMakeSharedWithAllocator<TestObjectEx>(123, 3.14);

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

    cout << "-------------------------#06#-------------------------\n\n";

    // dynamic_pointer_cast<T1, T2>()를 이용한 다운 캐스팅
    {
        // 업 캐스팅으로 받는다.
        shared_ptr<TestObject> sptr1 = MyMakeSharedWithAllocator<TestObjectEx>(123, 3.14);

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
