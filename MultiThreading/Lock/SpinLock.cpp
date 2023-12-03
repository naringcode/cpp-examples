#include <Windows.h>

#include <iostream>

#include <thread>
#include <mutex>

// SpinLock은 CAS를 이용해서 구현하는 것이 기본이다.
// CAS : Compare and Swap

// https://cplusplus.com/reference/atomic/atomic/compare_exchange_strong/
// https://cplusplus.com/reference/atomic/atomic_flag/
// https://cplusplus.com/reference/atomic/memory_order/
// https://learn.microsoft.com/ko-kr/windows/win32/api/winnt/nf-winnt-interlockedexchange

// std::atomic에서 제공하는 compare_exchange_weak()는 spurious failures를 반환할 수도 있음.
// 따라서 이것에 대한 예외 처리를 하기 귀찮다면 그냥 compare_exchange_strong()을 써도 됨.

// 자원 획득 정책에서 memory_order_seq_cst는 가장 엄격하기에 동기화가 중요하다면 이걸 쓰도록 함.
// memory_order_acquire는 load(읽기)에 관한 정책이기 때문에 혼동하지 말 것(반대로 store는 쓰기에 대한 정책임).
// memory_order_acquire와 memory_order_release는 메모리 배리어와도 관련이 있기 때문에 관련 자료는 따로 찾아서 보도록 함.

// InterlockedExchange()의 반환값은 Target이 가지고 있던 초기값이다.
// 만약 초기값이 TRUE라면 누군가가 이미 자원을 획득해서 사용하고 있었다는 뜻이 된다.

class CPPSpinLockByCAS
{
public:
    void lock()
    {
        bool expected = false;

        // 자원 획득 시도
        while (false == _locked.compare_exchange_strong(expected, true))
        {
            expected = false;
        }
    }

    void unlock()
    {
        // 복구
        _locked.store(false);
    }

private:
    std::atomic<bool> _locked;
};

class CPPSpinLockByFlag
{
public:
    void lock()
    {
        bool expected = false;

        // 자원 획득 시도
        while (_flag.test_and_set())
        { }
    }

    void unlock()
    {
        _flag.clear();
    }

private:
    std::atomic_flag _flag = ATOMIC_FLAG_INIT;
};

class WindowsSpinLock
{
public:
    void lock()
    {
        bool expected = false;

        // 자원 획득 시도
        while (TRUE == InterlockedExchange(&_flag, TRUE))
        { }
    }

    void unlock()
    {
        _flag = FALSE;
    }

private:
    volatile LONG _flag = FALSE;
};

template <typename T>
void Increase(T& lock, int* updateSum)
{
    for (int i = 0; i < 100'000; i++)
    {
        std::lock_guard<T> guard(lock);
    
        (*updateSum)++;
    }
}

template <typename T>
void Decrease(T& lock, int* updateSum)
{
    for (int i = 0; i < 100'000; i++)
    {
        std::lock_guard<T> guard(lock);
    
        (*updateSum)--;
    }
}

int main()
{
    CPPSpinLockByCAS  cppLockCAS;
    CPPSpinLockByFlag cppLockFlag;

    WindowsSpinLock winLock;

    std::thread th1;
    std::thread th2;

    // Case 1
    {
        int sum = 0;

        th1 = std::thread(Increase<CPPSpinLockByCAS>, std::ref(cppLockCAS), &sum);
        th2 = std::thread(Decrease<CPPSpinLockByCAS>, std::ref(cppLockCAS), &sum);

        th1.join();
        th2.join();

        std::cout << "CPPSpinLockByCAS : " << sum << '\n';
    }

    // Case 2
    {
        int sum = 0;

        th1 = std::thread(Increase<CPPSpinLockByFlag>, std::ref(cppLockFlag), &sum);
        th2 = std::thread(Decrease<CPPSpinLockByFlag>, std::ref(cppLockFlag), &sum);

        th1.join();
        th2.join();

        std::cout << "CPPSpinLockByFlag : " << sum << '\n';
    }

    // Case 3
    {
        int sum = 0;

        th1 = std::thread(Increase<WindowsSpinLock>, std::ref(winLock), &sum);
        th2 = std::thread(Decrease<WindowsSpinLock>, std::ref(winLock), &sum);

        th1.join();
        th2.join();

        std::cout << "WindowsSpinLock : " << sum << '\n';
    }

    return 0;
}
