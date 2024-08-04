#include <iostream>

class SomeClass
{
public:
    SomeClass() 
    {
        static int cnt = 0;

        std::cout << "Constructor : " << cnt++ << '\n';
    }

    SomeClass(const SomeClass& rhs)
    {
        std::cout << "Copy Constructor\n";
    }

    SomeClass(const SomeClass&& rhs) noexcept
    {
        std::cout << "Move Constructor\n";
    }

    SomeClass& operator=(const SomeClass& rhs)
    {
        std::cout << "Copy Assignment\n";

        return *this;
    }

    SomeClass& operator=(SomeClass&& rhs) noexcept
    {
        std::cout << "Move Assignment\n";

        return *this;
    }

public:
    int val = 0;
};

template <typename T>
constexpr std::remove_reference_t<T>&& MoveFunc(T&& univRef)
{
    return static_cast<std::remove_reference_t<T>&&>(univRef);
}

SomeClass MakeFunc()
{
    SomeClass someClass;

    return someClass;
}

void ReadFunc(SomeClass& func)
{
    std::cout << "ReadFunc(SomeClass& func)\n";
}

void ReadFunc(SomeClass&& func)
{
    std::cout << "ReadFunc(SomeClass&& func)\n";
}

int main()
{
    SomeClass   someClass;
    SomeClass&& rrefClass = (SomeClass&&)someClass; // 참조 형태(생성자 호출 안 함)

    std::cout << '\n';

    SomeClass copyClass1 = someClass; // 복사 생성자
    SomeClass moveClass1 = (SomeClass&&)someClass; // 이동 생성자

    SomeClass copyClass2 = rrefClass; // 복사 생성자
    SomeClass moveClass2 = (SomeClass&&)rrefClass; // 이동 생성자

    std::cout << '\n';

    copyClass1 = someClass; // 복사 할당
    moveClass1 = (SomeClass&&)someClass; // 이동 할당

    copyClass2 = rrefClass; // 복사 할당
    moveClass2 = (SomeClass&&)rrefClass; // 이동 할당

    std::cout << "--------------------\n";

    ReadFunc(someClass); // SomeClass&로 받음
    ReadFunc(rrefClass); // ## SomeClass&로 받음 ##

    std::cout << '\n';

    ReadFunc(std::move(someClass)); // SomeClass&&로 받음
    ReadFunc(std::move(rrefClass)); // SomeClass&&로 받음

    std::cout << '\n';

    ReadFunc(SomeClass{ }); // SomeClass&&로 받음
    ReadFunc(MakeFunc()); // ## SomeClass&&로 받음 ##

    std::cout << "--------------------\n";

    SomeClass retClassA = MoveFunc(someClass); // 이동 생성자
    SomeClass retClassB = MoveFunc(rrefClass); // 이동 생성자

    std::cout << "--------------------\n";

    // 캐스팅 테스트
    char* ptr = new char[128];

    // ((SomeClass&&)(*ptr))->val = 10; // 에러
    ((SomeClass&)(*ptr)).val = 10;
    ((SomeClass*)ptr)->val = 10;

    int* pVal = (int*)ptr;

    std::cout << "ptr : " << *pVal << '\n';

    delete[] ptr;

    return 0;
}
