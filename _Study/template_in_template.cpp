#include <iostream>

#include <vector>
#include <stack>

// 템플릿 내 템플릿을 적용해서 std::forward()를 사용하면 어떤 타입인지 추론할 수 없는 경우가 생길 수 있다.
// 자료구조를 만드는 과정에서 완벽한 전달을 해야 하는 상황이 발생할 수도 있으니 이 부분을 알고 가자.
// 명시적으로 std::move()를 사용했을 때 -> Move Constructor 호출
// std::move()로 전달하지 않았을 경우 -> Copy Constructor 호출

// vector를 보면 push_back()은 2종류가 있다.
// - & : 왼쪽값 참조로 받는 것
// - && : 오른값 참조로 받는 것
//
// 이 방식을 써서 구현해도 된다.
// 이렇게 하면 왼쪽값 참조와 오른값 참조 함수를 구분하는 것이 가능하다(template으로 묶여있지 않으니까).
// 
// 현재 방식
// template <typename U>
// Push(Some&& data) -> std::forward<U>()
//
// 개선할 수 있는 방식
// Push(Some& data) -> 그냥 전달 -> realPush(Some&& data) -> std::forward()
// Push(Some&& data) -> std::move() -> realPush(Some&& data) -> std::forward()
//
// Effective Modern C++ 5장에 나오는 내용 참고하기.

class TestObject
{
public:
    TestObject() { std::cout << "TestObject()\n"; }

    TestObject(const TestObject& rhs) noexcept { std::cout << "Copy\n"; }
    TestObject(const TestObject&& rhs) noexcept { std::cout << "Move\n"; }
};

template <typename T>
class Stack
{
public:
    explicit Stack() = default;

public:
    // NO COPY
    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;

public:
    template <typename U>
    void Push(U&& data)
    {
        _stack.push(std::forward<U>(data));

        // T와 U 중 어떤 것이 옳은지 추론할 수 없기 때문에 std::forward에 U를 명시해야 한다.
        // TestObject obj = std::forward(data);    // Error
        // TestObject obj = std::forward<U>(data); // OK
    }

private:
    std::stack<T> _stack;
};

int main()
{
    // Test Code
    {
        std::vector<TestObject> vec;

        // TestObject obj;

        // vec.emplace_back(obj);
        // vec.push_back(obj);
        // vec.push_back(std::ref(obj));
        // vec.push_back(std::move(obj));
    }

    // Test Code
    {
        Stack<TestObject> stack;

        TestObject obj;

        stack.Push(obj);
        stack.Push(std::ref(obj));
        stack.Push(std::move(obj));
    }

    return 0;
}
