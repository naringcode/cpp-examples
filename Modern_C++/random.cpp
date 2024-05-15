#include <iostream>
#include <random>

int main()
{
    using namespace std;

    // random_device : 난수 생성 장치
    // 하드웨어 기반를 기반으로 엔트로피를 수집하여 난수를 생성
    // 여기서 말하는 엔트로피는 물리적 현상의 무작위성으로 마우스 움직임, 키보드 타이핑, 디스크 I/O 등과 같은 하드웨어 이벤트를 의미한다.
    random_device device;

    cout << device() << '\n';
    cout << device() << '\n';
    cout << device() << '\n';

    cout << "----- 0 -----\n";

    // 랜덤 방식
    // 메르센 트위스터 알고리즘은 난수 생성을 2^19937 주기로 반복한다.
    // 따라서 반복이 일어날 가능성이 전무하기 때문에 고품질의 난수를 얻을 수 있다.
    mt19937_64 mersenne(0); // 메르센 트위스터 생성기(seed는 0으로 함)

    // 시드를 0으로 고정했기 때문에 출력 결과가 매번 똑같음
    for (int i = 0; i < 10; i++)
    {
        cout << mersenne() << '\n';
    }
    
    cout << "-----  1 -----\n";

    mersenne.seed(device()); // 난수 생성 장치로 시드 설정

    for (int i = 0; i < 10; i++)
    {
        cout << mersenne() << '\n';
    }

    cout << "----- 2 -----\n";

    // 고르게 분포한 랜덤 값 가지고 오기
    // 1 ~ 6 랜덤이 고르게 분포(랜덤 생성 방식이 아닌 분포 방식을 말함)
    uniform_int_distribution<> intDist(1, 6); // 기본 타입은 int

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
        cout << "[" << i << "] : " << intDistCnt[i] << '\n';
    }

    cout << "----- 3 -----\n";

    // uniform_int_distibution<>에는 int 외 다양한 정수 타입을 넣을 수 있음
    uniform_int_distribution<uint64_t> int64Dist(0, UINT32_MAX); // 0 ~ uint64_t의 크기

    // 10번 돌려서 확인
    for (int i = 0; i < 10; i++)
    {
        cout << int64Dist(mersenne) << '\n';
    }

    cout << "----- 4 -----\n";

    // 실수(real number)를 생성하고 싶다면 다음 클래스를 사용할 것
    uniform_real_distribution<> realDist(-10.0, 10.0); // 기본 타입은 double

    // 10번 돌려서 확인
    for (int i = 0; i < 10; i++)
    {
        cout << realDist(mersenne) << '\n';
    }

    return 0;
}
