#include "RWLock.h"

#include <iostream>
#include <thread>
#include <chrono>

// https://en.cppreference.com/w/cpp/numeric/ratio/ratio

thread_local uint32_t tls_threadID = 0;

using my_clock_t       = std::chrono::high_resolution_clock;
using my_millisecond_t = std::chrono::duration<uint64_t, std::milli>;

void RWLock::WriteLock()
{
    const uint32_t lockOwnerID = (_lockState.load() & kWriteOwnerMask) >> 32;

    // 동일 ID에 대한 재귀적 접근 허용
    if (tls_threadID == lockOwnerID)
    {
        _writeCnt++;

        return;
    }

    const auto     startTP = my_clock_t::now();
    const uint64_t desired = (uint64_t(tls_threadID) << 32) & kWriteOwnerMask;

    while (true)
    {
        for (uint64_t spinCnt = 0; spinCnt < kMaximumSpinCount; spinCnt++)
        {
            uint64_t expected = kEmptyState;

            // 소유권 획득 시도
            if (true == _lockState.compare_exchange_strong(expected, desired))
            {
                _writeCnt++;

                return;
            }
        }

        const auto     duration = my_clock_t::now() - startTP;
        const uint64_t interval = std::chrono::duration_cast<my_millisecond_t>(duration).count();

        // 예상한 시간보다 오래 걸린다(문제가 있는 상황).
        if (interval >= kMaximumTimeWait)
        {
            // TIMEOUT
            std::cout << "TimeOut\n";
            exit(-1);
        }

        // 제한된 횟수 내 소유권 획득에 실패하면 타임 슬라이스 포기
        std::this_thread::yield();
    }
}

void RWLock::WriteUnlock()
{
    // WriteLock을 잡은 상태에서 ReadLock을 잡았는데 ReadUnlock을 진행하지 않은 상황
    if (0 != (_lockState.load() & kReadSharedCountMask))
    {
        // UNLOCK_ORDER_MISMATCH
        std::cout << "Unlock Order Mismatch\n";
        exit(-1);
    }

    _writeCnt--;

    if (0 == _writeCnt)
    {
        // store()는 SEQ_CST를 따른다(가시성 보장, 메모리 재배치 X).
        _lockState.store(kEmptyState);
    }
}

void RWLock::ReadLock()
{
    const uint32_t lockOwnerID = (_lockState.load() & kWriteOwnerMask) >> 32;

    // WriteLock을 잡고 있는 쪽에서 ReadLock을 호출하면 허용
    if (tls_threadID == lockOwnerID)
    {
        _lockState.fetch_add(1);

        return;
    }
    
    const auto startTP = my_clock_t::now();

    while (true)
    {
        for (uint64_t spinCnt = 0; spinCnt < kMaximumSpinCount; spinCnt++)
        {
            // WriteLock을 소유하고 있는지 확인하기 위함
            // 그게 아니라면 ReadLock만 잡고 있는지 확인하기 위함
            uint64_t expected = _lockState.load() & kReadSharedCountMask;
            uint64_t desired  = expected + 1;

            // ReadLock만 잡고 있는 상태라면 허용
            if (true == _lockState.compare_exchange_strong(expected, desired))
                return;

            // 실패하는 상황은?
            // - 어디선가 WriteLock을 잡고 있다.
            // - 잠깐 사이에 누군가가 _lockState의 값을 바꿔서 expected와 매칭되지 않는다.

            const auto     duration = my_clock_t::now() - startTP;
            const uint64_t interval = std::chrono::duration_cast<my_millisecond_t>(duration).count();

            // 예상한 시간보다 오래 걸린다(문제가 있는 상황).
            if (interval >= kMaximumTimeWait)
            {
                // TIMEOUT
                std::cout << "TimeOut\n";
                exit(-1);
            }

            // 제한된 횟수 내 소유권 획득에 실패하면 타임 슬라이스 포기
            std::this_thread::yield();
        }
    }
}

void RWLock::ReadUnlock()
{
    if (0 == (_lockState.fetch_sub(1) & kReadSharedCountMask))
    {
        // ReadLock을 잡지 않은 상태에서 ReadUnlock을 호출한 상황
        std::cout << "ReadUnlock Under Zero\n";
        exit(-1);
    }
}
