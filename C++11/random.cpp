#include <iostream>
#include <random>

int main()
{
    using namespace std;

    // 난수 생성 장치
    random_device device;

    cout << device() << '\n';
    cout << device() << '\n';
    cout << device() << '\n';

    cout << "----------\n";

    // 랜덤 방식
    mt19937_64 mersenne(0); // 메르센 트위스터 생성기

    // 시드를 0으로 고정했기 때문에 출력 결과가 매번 똑같음
    for (int i = 0; i < 10; i++)
    {
        cout << mersenne() << '\n';
    }

    cout << "----------\n";

    mersenne.seed(device()); // 난수 생성 장치로 시드 설정

    for (int i = 0; i < 10; i++)
    {
        cout << mersenne() << '\n';
    }

    cout << "----------\n";

    // 고르게 분포한 랜덤 값 가지고 오기
    // 1 ~ 6 랜덤이 고르게 분포(랜덤 생성 방식이 아닌 분포 방식을 말함)
    uniform_int_distribution<> intDist(1, 6);

    int intDistCnt[6] = { 0 };
    
    for (int i = 0; i < 66666; i++)
    {
        // 분포는 생성기를 통해 랜덤 값을 구할 수 있음
        // cout << intDist(mersenne) << '\n';

        intDistCnt[intDist(mersenne) - 1]++;
    }

    // 분포가 고르게 되었는지 확인
    for (int i = 0; i < 6; i++)
    {
        cout << "[i] : " << intDistCnt[i] << '\n';
    }

    return 0;
}
