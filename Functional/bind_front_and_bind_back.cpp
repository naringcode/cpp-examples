// Update Date : 2025-02-22
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <algorithm>
#include <ranges>
#include <functional> // std::bind_front(), std::bind_back()

// https://en.cppreference.com/w/cpp/utility/functional/bind_front

// std::bind()를 다루는 "bind_and_mem_fn.cpp" 먼저 보고 오기

// 기존의 std::bind()를 통해 인자를 묶으면 채워지지 않은 부분은 std::placeholders로 기입해야 한다.
// 
// C++20은 이러한 단점을 보완한 std::bind_front()와 std::bind_back()이 추가되었다.
// 이 함수를 사용하면 함수 바인딩 시 std::placeholders를 기입하지 않아도 된다.
//
// std::bind()보다 위 두 함수를 사용하는 것이 훨씬 코드를 간결하게 작성할 수 있다.

// 사용하는 방법 자체는 std::bind()와 유사하기 때문에 예제는 "bind_and_mem_fn.cpp"와 유사하게 작성하였다.

#define BEGIN_NS(name) namespace name {
#define END_NS }

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

    void PrintTwo(std::string strA, std::string strB) const
    {
        std::cout << strA << ' ' << strB << '\n';
    }
};

BEGIN_NS(Case01)

// std::bind_front()는 앞의 인자를 고정할 때 사용하고, std::bind_back()은 뒤의 인자를 고정할 때 사용한다.
// std::placeholders를 기입하는 것을 제외하면 전체적인 사용 방법 자체는 std::bind()와 동일하다.

void Run()
{
    // std::bind_front()와 std::bind_back()을 사용하면 std::placeholders를 기입하지 않아도 된다.
    {
        // std::bind_front()
        // auto bindA1 = std::bind_front(&GlobalPrintThree, "One");
        std::function<void(std::string, std::string)> bindA1 = std::bind_front(&GlobalPrintThree, "One");

        bindA1("Two", "Three");

        // std::bind_back();
        // auto bindA2 = std::bind_back(&GlobalPrintThree, "Three");
        std::function<void(std::string, std::string)> bindA2 = std::bind_back(&GlobalPrintThree, "Three");

        bindA2("One", "Two");

        // std::bind_front()
        // auto bindB1 = std::bind_front(&GlobalPrintThree, "One", "Two");
        std::function<void(std::string)> bindB1 = std::bind_front(&GlobalPrintThree, "One", "Two");

        bindB1("Three");

        // std::bind_back();
        // auto bindB2 = std::bind_back(&GlobalPrintThree, "Two", "Three");
        std::function<void(std::string)> bindB2 = std::bind_back(&GlobalPrintThree, "Two", "Three");

        bindB2("One");

        std::cout << "\n";

        // 멤버 함수를 묶는 것도 가능하다.
        FooObject inst;

        // std::bind_front()
        // auto bindC1 = std::bind_front(&FooObject::PrintThree, &inst, "One", "Two");
        std::function<void(std::string)> bindC1 = std::bind_front(&FooObject::PrintThree, &inst, "One", "Two");

        bindC1("Three");

        // std::bind_back();
        // std::function<void(FooObject&, std::string)> bindC2 = std::bind_back(&FooObject::PrintThree, "Two", "Three");
        // std::function<void(FooObject*, std::string)> bindC2 = std::bind_back(&FooObject::PrintThree, "Two", "Three");
        auto bindC2 = std::bind_back(&FooObject::PrintThree, "Two", "Three");

        // 멤버 함수 호출이 컴파일되면 첫 번째 인자는 멤버 함수를 호출한 인스턴스가 지정되며 이게 바로 this이다.
        // std::bind_back()을 통해 앞의 인자가 아닌 뒤의 인자를 우선적으로 묶게 되면 이 인스턴스에 해당하는 내용이 비게 된다.
        // 본래는 컴파일러가 해준 일이지만 사용하는 쪽에서는 반환된 함수 객체를 사용할 때 첫 번째 인자를 인스턴스로 지정해야 한다.
        // 
        // auto는 아래 코드를 둘 다 적용해서 사용할 수 있지만 std::function<...>로 받으면 타입에 맞게 인자를 맞춰야 한다.
        bindC2(inst, "One");
        bindC2(&inst, "One"); // 두 유형의 호출 둘 다 차이가 없음(참조로 받기 때문에 복사나 이동 생성자 호출하지 않음).

        // auto bindC2의 타입을 확인해보면 std::_Back_binder<...>로 나오고 이것 자체엔 인스턴스의 타입이 명시되어 있지 않다.
        // std::_Back_binder<...>의 operator()를 보면 템플릿으로 작성되어 있는데 
        // 이 덕분에 인스턴스가 참조 타입이든 포인터 타입이든 받아서 처리할 수 있는 것이다.
        //
        // (주의) std::function<..>을 쓰면 지정한 인스턴스의 타입에 맞게 operator()에 인스턴스 인자를 넣어야 한다.

        std::cout << "\n";

        // std::bind()와 마찬가지로 여러 차례에 걸쳐 결합하는 것도 가능하다.
        std::function<void(int, int, int)> lambda = [](int x, int y, int z) { std::cout << (x + y + z) << '\n'; };

        // std::bind_front()
        std::function<void(int, int)> bindComplexA1 = std::bind_front(lambda, 100);
        bindComplexA1(60, 9);

        std::function<void(int)> bindComplexB1 = std::bind_front(bindComplexA1, 40);
        bindComplexB1(7);

        std::function<void()> bindComplexC1 = std::bind_front(bindComplexB1, 3);
        bindComplexC1();

        std::cout << "\n";

        // std::bind_back()
        std::function<void(int, int)> bindComplexA2 = std::bind_back(lambda, 100);
        bindComplexA2(60, 9);

        std::function<void(int)> bindComplexB2 = std::bind_back(bindComplexA2, 40);
        bindComplexB2(7);

        std::function<void()> bindComplexC2 = std::bind_back(bindComplexB2, 3);
        bindComplexC2();

        std::cout << "\n";

        // std::bind_front(), std::bind_back(), std::bind() 혼용
        std::function<void(int, int)> bindComplexA3 = std::bind_front(lambda, 100);
        bindComplexA3(60, 9);

        std::function<void(int)> bindComplexB3 = std::bind_back(bindComplexA3, 40);
        bindComplexB3(7);

        std::function<void()> bindComplexC3 = std::bind(bindComplexB2, 3);
        bindComplexC3();
    }

    std::cout << "\n";

    // std::bind_front()도 std::bind()처럼 기본적으로 인자를 값으로 복사하여 전달하기 때문에
    // 참조 형식으로 전달하려면 std::ref()를 써야 한다.
    {
        CopyTest copyTest;

        std::cout << "--------------------------------------------------\n";

        auto bindA = std::bind_front([](CopyTest& copyTest) { copyTest.Print(); }, copyTest);
        bindA();

        std::cout << "--------------------------------------------------\n";

        auto bindB = std::bind_front([](CopyTest& copyTest) { copyTest.Print(); }, std::ref(copyTest));
        bindB();

        std::cout << "--------------------------------------------------\n";

        // 이건 move로 받는다.
        auto bindC = std::bind_front([](CopyTest& copyTest) { copyTest.Print(); }, CopyTest{ });
        bindC();

        std::cout << "--------------------------------------------------\n";

        // 명시적으로 std::move()를 통해 전달하는 것도 가능하다.
        auto bindD = std::bind_front([](CopyTest& copyTest) { copyTest.Print(); }, std::move(copyTest));
        bindD();
    }

    std::cout << "\n";

    // std::bind_back()으로 멤버 함수를 묶고 첫 번째 인자로 인스턴스를 지정할 때 복사나 이동 생성자는 호출되지 않는다. 
    {
        CopyTest copyTest;

        std::cout << "1-------------------------------------------------\n";

        auto bindA = std::bind_back(&CopyTest::PrintTwo, "Two");

        std::cout << "2-------------------------------------------------\n";

        bindA(copyTest, "One"); // 복사나 이동 생성자 호출하지 않음.

        std::cout << "3-------------------------------------------------\n";

        auto bindB = std::bind_back(&CopyTest::PrintTwo, "Two");

        std::cout << "4-------------------------------------------------\n";

        bindB(&copyTest, "One");
    }

    std::cout << "\n";

    // bind 계열을 통해 인자를 묶는 단계에서는 인자를 값으로 받기 떄문에 그냥 전달하면 복사 생성자나 이동 생성자가 호출된다.
    {
        CopyTest copyTest;

        std::cout << "1-------------------------------------------------\n";

        auto bindA = std::bind_front(&CopyTest::PrintTwo, &copyTest, "One");

        std::cout << "2-------------------------------------------------\n";

        bindA("Two");

        std::cout << "3-------------------------------------------------\n";

        auto bindB = std::bind_front(&CopyTest::PrintTwo, copyTest, "One"); // 복사 생성자 호출

        std::cout << "4-------------------------------------------------\n";

        bindB("Two");

        std::cout << "5-------------------------------------------------\n";

        auto bindC = std::bind_front(&CopyTest::PrintTwo, CopyTest{ }, "One"); // 이동 생성자 호출

        std::cout << "6-------------------------------------------------\n";

        bindC("Two");
    }
}

END_NS

BEGIN_NS(Case02)

// std::bind_front()도 std::bind()처럼 스마트 포인터를 연계해서 사용하는 것이 가능하다.
// 기본적으로 인자는 값으로 전달되어 묶이기 때문에 생명 주기는 알아서 관리된다.

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

    // 스마트 포인터와 std::bind_front() 연계
    {
        auto bindA = std::bind_front(&FooObject::PrintTwo, inst, "Hello");
        bindA("World");

        std::cout << "Count : " << inst.use_count() << '\n';

        std::function<void(std::string)> bindB = std::bind_front(&FooObject::PrintTwo, inst, "Hello");
        bindB("Alice");

        std::cout << "Count : " << inst.use_count() << '\n';

        auto bindC = std::bind_front(bindB, "World");
        bindC();

        std::cout << "Count : " << inst.use_count() << '\n';
    }

    std::cout << "Count : " << inst.use_count() << '\n';
}

END_NS

BEGIN_NS(Case03)

// https://en.cppreference.com/w/cpp/utility/functional#Operator_function_objects
// https://en.wikipedia.org/wiki/Partial_function

// operator function objects와 연계하는 건 std::bind_front()와 std::bind_back()으로도 진행할 수 있다.
// 바인드 시 std::placeholders를 묶을 필요도 없기 때문에 사용하기 더 쉽다.
//
// std::bind_front()와 std::bind_back()을 쓰면 더 쉽게 operator function objects를 통해 
// subset을 제한하는 방식의 partial function을 구현하기 용이하다.

void Run()
{
    // std::plus는 두 값을 더하기 위해 사용하는 함수 객체
    auto add100 = std::bind_front(std::plus<int>{ }, 100);

    std::cout << add100(10) << '\n';
    std::cout << add100(20) << '\n';

    std::cout << '\n';

    // std::multiplies는 두 수를 곱하기 위해 사용하는 함수 객체
    auto threeTimes = std::bind_front(std::multiplies<int>{ }, 3);

    std::cout << threeTimes(10) << '\n';
    std::cout << threeTimes(100) << '\n';

    std::cout << '\n';

    // Comparisons를 통한 비교 작업도 할 수 있다.
    std::vector<int> nums{ 5, 10, 15, 20, 25 };

    auto greaterThan10 = std::bind_back(std::greater<int>{ }, 10); // std::bind_back()

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
                           std::bind_front(std::multiplies<int>(), 10));

    for (int elem : results)
    {
        std::cout << elem << ' ';
    }
    
    std::cout << '\n';

    // find value not equal to 2
    nums = std::vector<int>{ 2, 2, 4, 5, 6 };

    // std::bind()를 통해 이항 predicate를 단항 predicate를 지원하는 함수 객체로 바꿀 때
    // 비어있는 부분은 반드시 std::placeholders::_1(혹은 _1)로 채워야 한다.
    auto iter = std::ranges::find_if(nums, std::bind(std::not_equal_to<int>{ }, 2, std::placeholders::_1));

    if (iter != nums.end())
    {
        std::cout << "std::bind() : Found " << *iter << '\n';
    }

    // 하지만 std::bind_front()를 쓰면 빈 부분을 std::placeholders로 채우지 않아도 된다.
    auto iter2 = std::ranges::find_if(nums, std::bind_front(std::not_equal_to<int>{ }, 2));

    if (iter2 != nums.end())
    {
        std::cout << "std::bind_front() : Found " << *iter << '\n';
    }

    // 보통은 partial function을 통한 프로그래밍보단 람다식을 통한 필터링을 더 많이 하는 편이다.
    auto iter3 = std::ranges::find_if(nums, [](int elem) { return elem != 2; });

    if (iter3 != nums.end())
    {
        std::cout << "lambda expr : Found " << *iter << '\n';
    }
}

END_NS

int main()
{
    // Case01::Run();
    // Case02::Run();
    Case03::Run();

    return 0;
}
