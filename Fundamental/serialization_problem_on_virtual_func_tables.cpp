#include <iostream>

using namespace std;

struct TestStruct
{
    uint32_t val1;
    uint32_t val2;
};

struct TestFooStruct
{
    uint32_t val1;
    uint32_t val2;
};

class TestClass : public TestStruct
{
public:
    TestClass()  { cout << "TestClass()\n"; }
    virtual ~TestClass() { cout << "~TestClass()\n"; }
};

int main()
{
    TestClass testClass;

    testClass.val1 = 0x12345678;
    testClass.val2 = 0xaabbccdd;

    // 이렇게 하면 제대로 나옴.
    TestStruct* testStruct = &testClass;

    cout << std::hex << testStruct->val1 << '\n';
    cout << std::hex << testStruct->val2 << '\n';

    // 당연하겠지만 가상함수 테이블의 주소가 나옴.
    TestFooStruct* testFoo = (TestFooStruct*)&testClass;

    cout << std::hex << testFoo->val1 << '\n'; // 64비트 환경에선 가상함수 테이블 1/2
    cout << std::hex << testFoo->val2 << '\n'; // 64비트 환경에선 가상함수 테이블 2/2

    // 자체적으로 직렬화하면 문제가 생김.
    int32_t* intArr = (int32_t*)&testClass;

    cout << std::hex << intArr[0] << '\n'; // 64비트 환경에선 가상함수 테이블 1/2
    cout << std::hex << intArr[1] << '\n'; // 64비트 환경에선 가상함수 테이블 2/2
    cout << std::hex << intArr[2] << '\n';
    cout << std::hex << intArr[3] << '\n';

    return 0;
}
