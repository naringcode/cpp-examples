#include <iostream>
#include <memory>

// shared_ptr : 소유권을 여러 군데에서 공유할 수 있는 스마트 포인터

int main()
{
    using namespace std;

    std::shared_ptr<int> ptrA = std::make_shared<int>(10);
    std::shared_ptr<int> ptrB;

    std::cout << "ptrA use_count() : " << ptrA.use_count() << '\n';
    std::cout << "ptrB use_count() : " << ptrB.use_count() << '\n';

    // 소유권 공유
    ptrB = ptrA;

    std::cout << "ptrA use_count() : " << ptrA.use_count() << '\n';
    std::cout << "ptrB use_count() : " << ptrB.use_count() << '\n';

    std::cout << "-------------------------\n";

    std::shared_ptr<int> ptrC = std::make_shared<int>(10);
    std::shared_ptr<int> ptrD;

    std::cout << "ptrC use_count() : " << ptrC.use_count() << '\n';
    std::cout << "ptrD use_count() : " << ptrD.use_count() << '\n';

    // 소유권 이전
    ptrD = std::move(ptrC);

    std::cout << "ptrC use_count() : " << ptrC.use_count() << '\n';
    std::cout << "ptrD use_count() : " << ptrD.use_count() << '\n';

    std::cout << "-------------------------\n";

    std::unique_ptr<int> ptrE = std::make_unique<int>(10);
    std::shared_ptr<int> ptrF;

    std::cout << "ptrF use_count() : " << ptrF.use_count() << '\n';

    // 소유권 이전(unique -> shared)
    // 가능!
    ptrF = std::move(ptrE);

    std::cout << "ptrF use_count() : " << ptrF.use_count() << '\n';

    std::cout << "-------------------------\n";

    // 소유권 이전(shared -> unique)
    // 이 경우에는 안 됨!
    // ptrE = std::move(ptrF);

    return 0;
}
