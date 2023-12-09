#include <iostream>
#include <sstream>

#include <stack>
#include <queue>

#include <thread>
#include <mutex>
#include <condition_variable>

// Queue는 Stack과 거의 유사하기 때문에 만들지 않았음.

class TestObject
{
public:
    TestObject(int value = 0) : _value(value)
    { 
        std::cout << "TestObject()\n"; 
    }

public:
    TestObject(const TestObject& rhs)
        : _value(rhs._value)
    { 
        std::cout << "Copy Constructor\n"; 
    }

    TestObject& operator=(const TestObject& rhs)
    {
        std::cout << "Copy Assignment\n";

        _value = rhs._value;
    
        return *this;
    }

    TestObject(TestObject&& rhs) noexcept
    {
        std::cout << "Move Constructor\n"; 

        if (this != &rhs)
        {
            _value = rhs._value;
        }
    }

    TestObject& operator=(TestObject&& rhs) noexcept
    {
        std::cout << "Move Assignment\n";

        if (this != &rhs)
        {
            _value = rhs._value;
        }

        return *this;
    }

public:
    void Print()
    {
        std::stringstream ss;

        ss << "Value : " << _value << '\n';

        std::cout << ss.str();
    }

private:
    int _value = 0;
};

template <typename T>
class LockStack
{
public:
    explicit LockStack() = default;

public:
    // NO COPY
    LockStack(const LockStack&) = delete;
    LockStack& operator=(const LockStack&) = delete;

public: // std::vector에서 사용하는 push_back() 응용
    void Push(const T& data)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        _stack.push(data);

        _event.notify_one();
    }

    void Push(T&& data)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        _stack.push(std::move(data));

        _event.notify_one();
    }

public:
    bool TryPop(T* outData)
    {
        if (nullptr == outData)
            return false;

        std::lock_guard<std::mutex> lock(_mutex);

        if (true == _stack.empty())
            return false;

        *outData = std::move(_stack.top());
        _stack.pop();

        return true;
    }

    // 무한정 기다리는 버전
    bool WaitPop(T* outData)
    {
        if (nullptr == outData)
            return false;

        std::unique_lock<std::mutex> lock(_mutex);

        _event.wait(lock, [this]() { return false == _stack.empty(); });

        *outData = std::move(_stack.top());
        _stack.pop();

        return true;
    }

    // 제한 시간을 둬서 기다리는 버전
    template <class Rep, class Period>
    bool WaitPop(const std::chrono::duration<Rep, Period>& waitTime, T* outData)
    {
        if (nullptr == outData)
            return false;

        std::unique_lock<std::mutex> lock(_mutex);

        bool res = _event.wait_for(lock, waitTime, [this]() { return false == _stack.empty(); });
        if (false == res)
            return false;

        *outData = std::move(_stack.top());
        _stack.pop();

        return true;
    }

private:
    std::mutex _mutex;
    std::condition_variable _event;

private:
    std::stack<T> _stack;
};

int main()
{
    // Test Code
    // {
    //     std::vector<TestObject> vec;
    // 
    //     TestObject obj;
    // 
    //     // vec.emplace_back(obj);
    //     // vec.push_back(obj);
    //     // vec.push_back(std::ref(obj));
    //     // vec.push_back(std::move(obj));
    // }

    // Test Code
    // {
    //     LockStack<TestObject> stack;
    // 
    //     TestObject obj;
    // 
    //     stack.Push(obj);
    //     stack.Push(std::ref(obj));
    //     stack.Push(std::move(obj));
    // }

    // WaitPop() 대기 버전
    // {
    //     LockStack<TestObject> stack;
    // 
    //     std::thread th1 = std::thread([&]() {
    //         // th2가 먼저 실행될 때까지 대기
    //         std::this_thread::sleep_for(std::chrono::seconds(1));
    // 
    //         stack.Push(TestObject{ 10 });
    //     });
    // 
    //     std::thread th2 = std::thread([&]() {
    // 
    //         TestObject obj;
    // 
    //         while (false == stack.WaitPop(std::chrono::milliseconds(100), &obj))
    //         {
    //             std::cout << "Time Out\n";
    //         }
    // 
    //         obj.Print();
    //     });
    // 
    //     th1.join();
    //     th2.join();
    // }

    // WaitPop() 일반 버전
    // {
    //     LockStack<TestObject> stack;
    // 
    //     std::thread th1 = std::thread([&]() {
    //         // th2가 먼저 실행될 때까지 대기
    //         std::this_thread::sleep_for(std::chrono::seconds(1));
    // 
    //         stack.Push(TestObject{ 10 });
    //     });
    // 
    //     std::thread th2 = std::thread([&]() {
    // 
    //         TestObject obj;
    // 
    //         while (false == stack.WaitPop(&obj))
    //         {
    //             std::cout << "No Print\n";
    //         }
    // 
    //         obj.Print();
    //     });
    // 
    //     th1.join();
    //     th2.join();
    // }

    // 무한정 Push()하고 꺼내는 버전
    {
        LockStack<int>   stack;
        std::atomic<int> popCnt;

        // 경합 유도
        std::thread popThreads[10];
        std::thread pushThread = std::thread([&] {
            
            for (int i = 0; i < 100'000; i++)
            {
                stack.Push(i);

                // std::this_thread::yield();
            }
        });

        for (int i = 0; i < 10; i++)
        {
            popThreads[i] = std::thread([&] {

                int data;

                while (100'000 != popCnt.load())
                {
                    if (true == stack.TryPop(&data))
                    {
                        std::stringstream ss;
                        {
                            // ss << data << '\n';
                        }

                        popCnt.fetch_add(1);

                        // std::cout << ss.str();
                    }
                }
            });
        }

        pushThread.join();

        for (int i = 0; i < 10; i++)
        {
            popThreads[i].join();
        }
    }

    return 0;
}
