#include <iostream>
#include <memory>

#include <utility> // index_sequence

// !! 자세한 내용은 _Study/_Primary/shared_ptr_details.cpp에 적었다. !!
// !! 아래 코드는 간소화하여 사용자 정의 Deleter를 적용하는 방법을 기술하였다. !!

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
        : TestObject{ valA }, _valB { valB }
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
    return shared_ptr<T>{ ptr, &MyDeleter<T> };
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

    // MyMakeShared<TestObjectEx>()로 shared_ptr 생성
    {
        shared_ptr<TestObjectEx> objEx = MyMakeShared<TestObjectEx>(100, 3.141592);
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

        // 최종적으로 shared_ptr<TestObject>의 소멸자를 거치게 해도 호출되는 객체의 소멸자는 ~TestObjectEx()이다.
        //
        // 이건 deleter로 등록된 함수가 MyDeleter<TestObjectEx>이기 때문이 그런 것이다.
        //
    }

    cout << "--------------------------------------------------\n";

    return 0;
}
