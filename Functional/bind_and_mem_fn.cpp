// Update Date : 2025-02-21
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <algorithm>
#include <ranges>
#include <functional>
#include <memory>
#include <vector>

// https://en.cppreference.com/w/cpp/utility/functional/bind
// https://en.cppreference.com/w/cpp/utility/functional/placeholders
// https://en.cppreference.com/w/cpp/utility/functional/mem_fn

#define BEGIN_NS(name) namespace name {
#define END_NS }

void GlobalPrintOne(std::string strA)
{
    std::cout << strA << '\n';
}

void GlobalPrintTwo(std::string strA, std::string strB)
{
    std::cout << strA << ' ' << strB << '\n';
}

void GlobalPrintThree(std::string strA, std::string strB, std::string strC)
{
    std::cout << strA << ' ' << strB << ' ' << strC << '\n';
}

class FooObject
{
public:
    void PrintOne(std::string strA) const
    {
        std::cout << strA << '\n';
    }

    void PrintTwo(std::string strA, std::string strB) const
    {
        std::cout << strA << ' ' << strB << '\n';
    }

    void PrintThree(std::string strA, std::string strB, std::string strC) const
    {
        std::cout << strA << ' ' << strB << ' ' << strC << '\n';
    }
};

class CopyTest
{
public:
    CopyTest()
    {
        std::cout << "Default Constructor\n";
    }

    CopyTest(const CopyTest& other)
    {
        std::cout << "Copy Constructor\n";
    }

    CopyTest(CopyTest&& other) noexcept
    {
        std::cout << "Move Constructor\n";
    }

public:
    void Print()
    {
        std::cout << "Print() : CopyTest\n";
    }
};

BEGIN_NS(Case01)

// std::bind()를 쓰면 함수와 인자를 묶어서 함수 객체처럼 사용할 수 있다.
// 또한 특정 객체와 멤버 함수를 묶어서 사용하는 것도 가능하다.

void Run()
{
    // 전역 함수와 인자 하나 묶기
    {
        auto bindA = std::bind(&GlobalPrintOne, "Hello One");
        std::function<void()> bindB = std::bind(&GlobalPrintOne, "Hello One");

        bindA();
        bindB();
    }

    std::cout << '\n';

    // 객체와 멤버 함수를 묶고 인자까지 묶기
    // 이건 멤버 함수를 함수 객체처럼 사용하는 방법이다.
    {
        // 코드 단계에서는 눈으로 보이지 않지만 코드가 컴파일될 때 멤버 함수 호출의 첫 번째 인자는 this로 받는다.
        // 이런 원리에 따라 std::function의 대상이 멤버 함수라면 첫 번째 인자는 객체로 넣어야 한다.
        // FooObject inst;
        // FooObject kInst;
        // 
        // std::function<void(FooObject&, std::string)> funcA = &FooObject::PrintOne;
        // std::function<void(const FooObject&, std::string)> funcB = &FooObject::PrintOne;
        // 
        // std::function<void(FooObject*, std::string, std::string)> funcC = &FooObject::PrintTwo;
        // std::function<void(const FooObject*, std::string, std::string)> funcD = &FooObject::PrintTwo;
        // 
        // funcA(inst, "Hello One");
        // funcB(kInst, "Hello One"); // const
        // 
        // // 참조형이 아닌 포인터로 받는 것도 가능하다.
        // funcC(&inst, "One", "Two");
        // funcD(&kInst, "One", "Two"); // const

        // std::bind()에서도 동일한 원리에 따라 코드를 작성하면 된다.
        FooObject inst;
        
        auto bindA = std::bind(&FooObject::PrintOne, &inst, "Hello One");
        std::function<void()> bindB = std::bind(&FooObject::PrintOne, &inst, "Hello One");
        
        auto bindC = std::bind(&FooObject::PrintTwo, &inst, "One", "Two");
        std::function<void()> bindD = std::bind(&FooObject::PrintTwo, &inst, "One", "Two");
        
        bindA();
        bindB();
        
        bindC();
        bindD();
    }

    std::cout << '\n';

    // 람다식(Functor)을 대상으로 std::bind()를 쓰는 것도 가능하다.
    {
        auto lambda = [](int x, int y) { std::cout << "Lambda : " << (x + y) << '\n'; };
    
        std::function<void(int, int)> func = lambda;
    
        // std::bind() 진행
        auto bindA = std::bind(lambda, 100, 200);
        std::function<void()> bindB = std::bind(lambda, 100, 200);
    
        auto bindC = std::bind(func, 1000, 2000);
        std::function<void()> bindD = std::bind(func, 1000, 2000);
    
        bindA();
        bindB();
    
        bindC();
        bindD();
    }
    
    std::cout << '\n';
    
    // std::placeholders를 사용하면 사용자가 전달할 인자의 개수와 순서(?)를 지정할 수 있다.
    {
        // (주의) placeholders는 바인드한 인자를 나타내는 것이 아닌 사용할 때 인자로 넣은 걸 몇 번째 인자로 할 것인지를 나타낸다.
        // std::bind()를 진행할 때 함수의 비어있는 파라미터의 개수만큼 placeholders가 지정되어야 한다.
        using namespace std::placeholders;
    
        // auto bindA = std::bind(&GlobalPrintThree, "One", std::placeholders::_1, std::placeholders::_2);
        std::function<void(std::string, std::string)> bindA = std::bind(&GlobalPrintThree, "One", std::placeholders::_1, std::placeholders::_2);
    
        bindA("Two", "Three");

        // 멤버 함수도 마찬가지
        FooObject inst;

        std::function<void(std::string, std::string)> bindB = 
            std::bind(&FooObject::PrintThree, inst, "One", /*std::placeholders::*/_1, /*std::placeholders::*/_2);

        bindB("Two", "Three");

        // 넣은 인자를 셔플링하는 것도 가능하다.
        std::function<void(std::string, std::string, std::string)> bindC =  
            std::bind(&GlobalPrintThree, _3, _2, _1);

        bindC("One", "Two", "Three");

        // 여러 차례에 걸쳐서 결합하는 것도 가능하다.
        std::function<void(int, int, int)> lambda = [](int x, int y, int z) { std::cout << (x + y + z) << '\n'; };

        std::function<void(int, int)> bindComplexA = std::bind(lambda, 100, _1, _2);
        bindComplexA(60, 9);

        std::function<void(int)> bindComplexB = std::bind(bindComplexA, 40, _1);
        bindComplexB(7);

        std::function<void()> bindComplexC = std::bind(bindComplexB, 3);
        bindComplexC();

        // placeholders가 꼭 뒤에 나와야 한다는 법은 없다.
        std::function<void(std::string, std::string)> bindD =
            std::bind(&GlobalPrintThree, _1, "#Two#", _2);

        std::function<void(std::string, std::string)> bindE =
            std::bind(&GlobalPrintThree, _1, _2, "#Three#");

        bindD("One", "Three");
        bindE("One", "Two");
    }

    std::cout << '\n';

    // std::bind()는 기본적으로 인자를 값으로 복사하여 전달하기 때문에
    // 참조 형식으로 전달하려면 std::ref()를 써야 한다.
    {
        CopyTest copyTest;

        std::cout << "--------------------------------------------------\n";

        auto bindA = std::bind([](CopyTest& copyTest) { copyTest.Print(); }, copyTest);
        bindA();

        std::cout << "--------------------------------------------------\n";

        auto bindB = std::bind([](CopyTest& copyTest) { copyTest.Print(); }, std::ref(copyTest));
        bindB();

        std::cout << "--------------------------------------------------\n";

        // 이건 move로 받는다.
        auto bindC = std::bind([](CopyTest& copyTest) { copyTest.Print(); }, CopyTest{ });
        bindC();

        std::cout << "--------------------------------------------------\n";

        // 명시적으로 std::move()를 통해 전달하는 것도 가능하다.
        auto bindD = std::bind([](CopyTest& copyTest) { copyTest.Print(); }, std::move(copyTest));
        bindD();
    }
}

END_NS

BEGIN_NS(Case02)

// std::bind()와 스마트 포인터를 연계해서 사용하는 것도 가능하다.
// 이 경우 생명 주기도 알아서 관리된다.

void Run()
{
    std::shared_ptr<FooObject> inst = std::make_shared<FooObject>();

    std::cout << "Count : " << inst.use_count() << '\n';

    // std::function을 활용한 정석적인 사용 방법
    {
        std::function<void(std::string)> func = [self = inst](std::string msg) {
                self->PrintTwo("Hello", msg);
            };

        func("World");

        std::cout << "Count : " << inst.use_count() << '\n';
    }

    std::cout << "Count : " << inst.use_count() << '\n';

    // 스마트 포인터와 std::bind() 연계
    {
        auto bindA = std::bind(&FooObject::PrintTwo, inst, "Hello", std::placeholders::_1);
        bindA("World");

        std::cout << "Count : " << inst.use_count() << '\n';

        std::function<void(std::string)> bindB = std::bind(&FooObject::PrintTwo, inst, "Hello", std::placeholders::_1);
        bindB("Alice");

        std::cout << "Count : " << inst.use_count() << '\n';

        auto bindC = std::bind(bindB, "World");
        bindC();

        std::cout << "Count : " << inst.use_count() << '\n';
    }

    std::cout << "Count : " << inst.use_count() << '\n';
}

END_NS

BEGIN_NS(Case03)

// https://en.cppreference.com/w/cpp/utility/functional#Operator_function_objects
// https://en.wikipedia.org/wiki/Partial_function

// std::bind()는 operator function objects와 연계해서 사용하기 좋다.
// C++ 알고리즘이 unary predicate(단항 predicate) 기반으로 동작할 경우 binary predicate(이항 predicate)을 단항 식으로 변환해서 사용할 수 있다.
//
// std::bind()는 인자를 특정 값으로 고정한 operator function objects를 통해 subset을 제한하는 방식으로 partial function을 구현하기에 용이하다.
// !! 대부분의 경우에는 람다식을 쓰는 것이 가독성이 더 좋긴 함. !!

void Run()
{
    // std::plus는 두 값을 더하기 위해 사용하는 함수 객체
    auto add100 = std::bind(std::plus<int>{ }, 100, std::placeholders::_1);

    std::cout << add100(10) << '\n';
    std::cout << add100(20) << '\n';

    std::cout << '\n';

    // std::multiplies는 두 수를 곱하기 위해 사용하는 함수 객체
    auto threeTimes = std::bind(std::multiplies<int>{ }, 3, std::placeholders::_1);

    std::cout << threeTimes(10) << '\n';
    std::cout << threeTimes(100) << '\n';

    std::cout << '\n';

    // Comparisons를 통한 비교 작업도 할 수 있다.
    std::vector<int> nums{ 5, 10, 15, 20, 25 };

    auto greaterThan10 = std::bind(std::greater<int>{ }, std::placeholders::_1, 10);

    for (int elem : nums | std::views::filter(greaterThan10))
    {
        std::cout << elem << ' ';
    }

    std::cout << '\n';

    std::cout << std::ranges::count_if(nums, greaterThan10) << '\n';

    std::cout << '\n';

    // 간단한 기능이라면 람다식 대신 사용해서 자료를 변형하는 것도 가능하다.
    std::vector<int> results;

    std::ranges::transform(nums, std::back_inserter(results),
                           std::bind(std::multiplies<int>(), 10, std::placeholders::_1));

    for (int elem : results)
    {
        std::cout << elem << ' ';
    }
    
    std::cout << '\n';
}

END_NS

BEGIN_NS(Case04)

// std::functiton이나 std::bind() 대신 std::mem_fn()을 쓰면 손쉽게 멤버 함수를 함수 객체로 지정할 수 있다.
// std::mem_fn()을 쓰면 객체 인스턴스를 수동으로 묶어주지 않아도 된다.
//
// (주의) 멤버 함수가 static으로 되어 있으면 사용할 수 없다.

void Run()
{
    FooObject inst;

    // 수동으로 받는다면 이렇게 써야 한다.
    // std::function<void(FooObject&, std::string, std::string)> memFn = std::mem_fn(&FooObject::PrintTwo);
    
    auto memFn = std::mem_fn(&FooObject::PrintTwo);
    memFn(inst, "Hello", "World");

    // std::mem_fn()은 특정 인자를 고정할 수 없기 때문에 이를 연계하려면 std::bind()나 람다를 섞어야 한다.
    // std::function<void(std::string)> bind = std::bind(memFn, inst, "Hello", std::placeholders::_1);
    auto bind = std::bind(memFn, inst, "Hello", std::placeholders::_1);
    bind("Alice");
}

END_NS

int main()
{
    // Case01::Run();
    // Case02::Run();
    Case03::Run();
    // Case04::Run();

    return 0;
}
