#include <iostream>
#include <thread>

using namespace std;

class TLSObject
{
public:
    TLSObject() { cout << "TLSObject()\n"; }
    ~TLSObject() { cout << "~TLSObject()\n"; }
};

class TestObject
{
public:
    TestObject() { cout << "TestObject()\n"; }
    ~TestObject() { cout << "~TestObject()\n"; }

public:
    static thread_local TLSObject tls_Object;
};

thread_local TLSObject TestObject::tls_Object;

void ThreadMain()
{
    cout << "1\n";

    TestObject testObject;

    cout << "2\n";

    this_thread::sleep_for(1s);
}

#define ONOFF 0

int main()
{
    /* TLS 객체는 매 스레드가 생성될 때마다 생성자를 호출하고, 소멸하는 순간 소멸자를 호출함. */
    /* TLS가 적용되면 사용 여부와 관계 없이 일단 초기화가 진행되니 꼭 필요한 것만 적용하도록 한다. */

    // ONOFF를 비활성화시켜도 TLSObject의 생성자가 초기화된다.
    // 스레드가 생기면 TLS 자료를 생성하거나 사용하는 코드가 없어도 초기화 작업이 진행된다는 사실을 꼭 기억하자.

#if ONOFF == 1
    cout << "before thread\n";

    std::thread th1{ ThreadMain };
    std::thread th2{ ThreadMain };

    cout << "after thread\n";

    this_thread::sleep_for(2s);

    cout << "before join()\n";

    th1.join();
    th2.join();

    cout << "after join()\n";
#endif

    return 0;
}
