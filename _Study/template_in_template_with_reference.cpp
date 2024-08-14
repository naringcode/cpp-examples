#include <iostream>

#include <vector>
#include <stack>

// 템플릿 내 템플릿을 적용해서 std::forward()를 사용하면 타입 추론 과정에서 문제가 생길 수 있다.
// 자료구조를 직접 구현하고자 한다면 완벽한 전달을 해야 하는 경우가 생기니 다음 내용을 숙지하도록 한다.
// 
// LValue의 경우
// - 명시적으로 std::move()를 사용했을 경우 -> Move Semantics 사용
// - std::move()로 전달하지 않았을 경우 -> Copy Semantics 사용
// 
// RValue의 경우
// - Move Semantics 사용

// vector의 push_back()은 함수 오버로딩을 통해 2가지 방식으로 구현되어 있다.
// - push_back(T& val) : 왼쪽값 참조(LValue Reference)로 받는 것.
// - push_back(T&& val) : 오른값 참조(RValue Reference)로 받는 것.
//   - 이건 template 클래스의 함수이지, tempalte 함수가 아님에 주의할 것(보편 참조가 아니라는 뜻).
//
// 위와 같은 방식으로 구현해도 된다.
// 이렇게 하면 왼쪽값 참조와 오른값 참조 함수를 구분하는 것이 가능하다.

// Effective Modern C++ 5장에 나오는 내용 참고하기.

class TestObject
{
public:
    TestObject()
    {
        std::cout << "Constructor\n";
    }

public:
    TestObject(const TestObject& rhs) noexcept
    {
        std::cout << "Copy\n";
    }
    
    TestObject(TestObject&& rhs) noexcept
    {
        std::cout << "Move\n";
    }
};

template <typename T>
class UnivRefStack
{
public:
    explicit UnivRefStack() = default;

public:
    // NO COPY
    UnivRefStack(const UnivRefStack&) = delete;
    UnivRefStack& operator=(const UnivRefStack&) = delete;

public:
    template <typename U>
    void DirectPush(U&& data)
    {
        std::cout << "UnivRefStack::DirectPush(U&& data)\n";

        // T와 U 중 어떤 것이 옳은지 추론할 수 없기 때문에 std::forward()에 U를 명시해야 한다.
        // TestObject obj = std::forward(data);    // Error
        // TestObject obj = std::forward<U>(data); // OK
        _stack.push(std::forward<U>(data));
    }

    template <typename U>
    void WrapPush(U&& data)
    {
        this->WrapPushReal(std::forward<U>(data));
    }

    void WrapPushReal(const T& data)
    {
        std::cout << "UnivRefStack::WrapPushReal(const T& data)\n";

        // LValue Reference를 대상으로 std::move()를 적용하면?
        // 이 경우에는 식별자의 타입에 따라서 LValue Reference로 해석될 수도 있고, RValue Reference로 해석될 수도 있다.
        // 
        // 일단 Move Semantics는 기본적으로 대상 객체의 내부 상태를 변경할 수 있어야 한다.
        // 따라서 상수 참조를 대상으로 이동 연산자를 사용하는 것은 제약에 걸린다.
        // 상수 참조를 대상으로 이동 연산자를 적용하는 것은 허용되지 않기에 이런 상황이 발생하면 Copy Semantics가 대신 적용된다.
        // 
        // 상수 참조를 대상으로 std::move()를 쓰면 아무런 효과도 없고, 오히려 오해를 불러일으킬 수 있다.
        
        // std::move(data);
        // - 식별자 타입에 const가 붙으면 제약에 따라 LValue Reference로 해석된다.
        // - 식별자 타입에 const가 없으면 RValue Reference로 해석된다.
        // _stack.push(std::move(data)); // push(value_type&& _Val)

        _stack.push(data);
    }

    void WrapPushReal(T&& data)
    {
        std::cout << "UnivRefStack::WrapPushReal(T&& data)\n";

        // data가 RValue Reference Type이라고 해도 함수의 환경에 저장된 이상 식별자로 사용할 때는 LValue로 취급한다.
        // 따라서 의도한대로 코드가 동작하게 하려면 std::move()를 꼭 써야 한다.
        // 이에 대한 자세한 사항은 rvalue_and_move_semantics.cpp를 확인할 것.

        // _stack.push(data); // push(const value_type& _Val)
        _stack.push(std::move(data)); // push(value_type&& _Val)
    }

private:
    std::stack<T> _stack;
};

template <typename T>
class OverloadingStack
{
public:
    explicit OverloadingStack() = default;

public:
    // NO COPY
    OverloadingStack(const OverloadingStack&) = delete;
    OverloadingStack& operator=(const OverloadingStack&) = delete;

public:
    void Push(const T& data)
    {
        std::cout << "OverloadingStack::Push(const T& data)\n";

        _stack.push(data);
    }

    // 이건 보편 참조가 아닌 RValue Reference임(!!! 혼동하지 말 것 !!!)
    // 아래 함수는 템플릿 함수가 아닌 "템플릿 클래스"의 함수임.
    void Push(T&& data)
    {
        std::cout << "OverloadingStack::Push(T&& data)\n";

        _stack.push(std::move(data));
    }

private:
    std::stack<T> _stack;
};

int main()
{
    using namespace std;

    // Test Code
    {
        std::cout << "1) ---------------------------\n";

        std::vector<TestObject> vec;

        // reserve(10)을 주석처리하고 관찰하면 Rellocation에서 어떤 일이 일어나는지까지 볼 수 있음.
        vec.reserve(10);

        TestObject obj;
        cout << "----\n";

        vec.emplace_back(obj);
        cout << "----\n";

        vec.emplace_back(std::ref(obj));
        cout << "----\n";

        vec.emplace_back(std::move(obj));
        cout << "----\n";

        vec.push_back(obj);
        cout << "----\n";

        vec.push_back(std::ref(obj));
        cout << "----\n";

        vec.push_back(std::move(obj));
    }

    std::cout << "------------------------------\n";

    // Test Code(중단점 걸고서 테스트할 것)
    {
        std::cout << "2) ---------------------------\n";

        int testVal = 100;

        UnivRefStack<int> testStackUniv;

        testStackUniv.DirectPush(10);
        testStackUniv.DirectPush(testVal);
        
        testStackUniv.WrapPush(10);
        testStackUniv.WrapPush(testVal);

        cout << "----\n";

        OverloadingStack<int> testStackOver;

        testStackOver.Push(10);
        testStackOver.Push(testVal);
    }

    std::cout << "------------------------------\n";

    // Test Code(중단점 걸고서 테스트할 것)
    {
        std::cout << "3) ---------------------------\n";

        UnivRefStack<TestObject> testStackUniv;

        TestObject testObj;

        cout << "----\n";

        testStackUniv.DirectPush(testObj);
        testStackUniv.DirectPush(std::ref(testObj));
        testStackUniv.DirectPush(std::move(testObj));

        testStackUniv.WrapPush(testObj);
        testStackUniv.WrapPush(std::ref(testObj));
        testStackUniv.WrapPush(std::move(testObj));

        cout << "----\n";

        OverloadingStack<TestObject> testStackOver;

        testStackOver.Push(testObj);
        testStackOver.Push(std::ref(testObj));
        testStackOver.Push(std::move(testObj));
    }

    std::cout << "------------------------------\n";

    return 0;
}
