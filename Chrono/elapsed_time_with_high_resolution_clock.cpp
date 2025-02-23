#include <iostream>
#include <chrono>

// using namespace std;

int main()
{
    // https://en.cppreference.com/w/cpp/chrono/high_resolution_clock
    // high_resolution_clock은 가장 높은 해상도를 제공하는 시계이지만 어떤 clock을 택하냐에 따라 구현이 달라진다.
    // GCC  : using high_resolution_clock = system_clock;
    // MSVC : using high_resolution_clock = steady_clock;
    //
    // 어떤 환경에서도 가장 정밀한 시계를 쓰고 싶은 경우 high_resolution_clock을 쓰면 되지만 일관성은 보장하지 못 한다.
    // 일관성을 보장하고 싶다면 system_clock과 steady_clock 중 하나를 택해서 사용하는 것이 좋다.
    //
    // 순전히 경과한 시간만 측정할 용도면 steady_clock을 쓰는 것이 좋고,
    // 시스템 시간(운영체제 시간)에 영향을 받는 기능을 구현할 때는 system_clock을 쓰는 게 좋다.
    // - system_clock : 현재 시스템 시간을 조회하는 용도, 파일 생성 및 수정 시간 저장 용도 등
    // - steady_clock : 코드의 실행 시간 측정, 이벤트 간 정확한 시간 측정 등
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point endTime;

    startTime = std::chrono::high_resolution_clock::now();

    while (true)
    {
        char ch;

        // Do Something
        std::cin >> ch;

        if ('0' == ch)
            break;

        endTime = std::chrono::high_resolution_clock::now();

        double elapsedTime = (double)std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

        startTime = endTime;

        std::cout << "Elapsed Time : " << elapsedTime / 1000.0 << "ms\n";
    }

    return 0;
}
