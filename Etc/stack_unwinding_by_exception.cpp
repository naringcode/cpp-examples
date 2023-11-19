#include <iostream>

// exception specifier : throw를 던질 가능성이 있는 함수라는 뜻(생략해도 됨)
// (...) 대신에 명시적으로 int, const char* 등을 적어도 됨.
void func(int idx) throw(...)
{
    std::cout << "Enter : " << idx << '\n';

    if (5 == idx)
        throw "Exception!";

    func(idx + 1);

    std::cout << "Exit : " << idx << '\n';
}

int main()
{
    using namespace std;

    try
    {
        func(0);
    }
    catch (const char* e)
    {
        std::cout << e << '\n';
    }

    return 0;
}
