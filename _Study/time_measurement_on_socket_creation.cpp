// Update Date : 2024-10-11
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

// 사양
// CPU : AMD Ryzen 5 7600 6-Core Processor 3.80 GHz
// RAM : 32GB

#include <iostream>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <thread>

#include <vector>

#pragma comment(lib, "ws2_32")
#include <WinSock2.h>

using namespace std;

int main()
{
    // 소켓을 재사용할 필요가 있을까?
    // 보급형 가정용 데스크탑 기준으로 계산해도 소켓 생성과 반환에 들어가는 비용은 마이크로 초 단위도 아니고 나노 초 단위이다.
    // DisconnectEx()를 통해 소켓을 재사용하기보다는 크로스플랫폼 작업의 용이성을 위해 그냥 반환하는 방식으로 가도 괜찮을 듯 함.

    constexpr int32_t kSockCnt = 1'000'000;

    // 생성과 동시에 반환으로 테스트
    auto startTimePoint = chrono::high_resolution_clock::now();

    for (int32_t i = 0; i < kSockCnt; i++)
    {
        auto sock = socket(AF_INET, SOCK_STREAM, 0);

        closesocket(sock);

        // 컨텍스트 스위칭이 일어났다는 것을 가정하고 yield()
        this_thread::yield();
    }

    auto endTimePoint = chrono::high_resolution_clock::now();

    auto duration = chrono::duration_cast<chrono::duration<double, micro>>(endTimePoint - startTimePoint).count();

    cout << "Total : " << duration << ", Per : " << std::fixed << std::setprecision(15) << double(duration) / double(kSockCnt) << '\n';

    cout << "--------------------------------------------------\n";

    vector<SOCKET> sockVec;
    sockVec.reserve(kSockCnt);

    // 생성을 전부 끝내고 마지막에 반환 테스트
    startTimePoint = chrono::high_resolution_clock::now();

    for (int32_t i = 0; i < kSockCnt; i++)
    {
        sockVec.push_back(socket(AF_INET, SOCK_STREAM, 0));

        // 컨텍스트 스위칭이 일어났다는 것을 가정하고 yield()
        this_thread::yield();
    }

    std::for_each(sockVec.begin(), sockVec.end(), [](SOCKET hSock) { closesocket(hSock); });
    sockVec.clear();

    endTimePoint = chrono::high_resolution_clock::now();

    duration = chrono::duration_cast<chrono::duration<double, micro>>(endTimePoint - startTimePoint).count();

    cout << "Total : " << duration << ", Per : " << std::fixed << std::setprecision(15) << double(duration) / double(kSockCnt) << '\n';

    return 0;
}
