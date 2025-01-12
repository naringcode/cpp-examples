#include <iostream>
#include <atomic>

/**
 * // compare_exchange_xxx()는 이렇게 생겼다.
 * if (value == expected)
 * {
 *     // expected에는 value의 값이 들어가고
 *     // value에는 desired의 값이 들어간다.
 *     expected = value;
 *     value = desired;
 *
 *     return true;
 * }
 * else
 * {
 *     expected = value;
 *
 *     return false;
 * }
 */

// https://cplusplus.com/reference/atomic/atomic/compare_exchange_weak/
// https://cplusplus.com/reference/atomic/atomic/compare_exchange_strong/
// https://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange
// https://www.ldoceonline.com/dictionary/spurious
// spurious : 가짜, 허위, 논리가 잘못된

// compare_exchange_strong()은 spurious failures를 허용하지 않지만, compare_exchange_weak()는 spurious failures를 허용한다.
// !! 이게 두 함수의 가장 큰 차이임. !!
// 
// compare_exchange_strong()은 대상 값과 expected가 같을 경우 항상 true를 반환하지만,
// compare_exchange_weak()는 대상 값과 expected가 같아도 false를 반환하는 경우가 있다.
// !! spurious failures는 성공해야 하는데 실패하는 상황을 말하는 것임. !!
// 
// 이론은 이러하지만 거의 뜨지 않는 문제이기도 하고,
// 보통은 반복문으로 값을 갱신했는지 판단하는 경우가 많기 때문에 이 부분이 크게 문제되는 경우는 많지 않다.
// 
// spurious failures가 걱정된다면 이를 검증하는 코드도 넣어야 한다.
// compare_exchange_weak()에서는 이걸 반복문을 돌려서 해결하는 경우가 많다.
// !! spurious failures는 compare_exchange_weak() 뿐만 아니라 condition_variable::wait()에서도 발생하는 문제임. !!
// 

int main()
{
    // compare_exchange_strong()
    {
        std::atomic<int> atomVal(10);

        int expected = 50;
        int desired  = 100;

        // 값 교환이 실패했을 경우
        bool ret = atomVal.compare_exchange_strong(expected, desired);
        std::cout << "AtomicVal : " << atomVal << ", Expected : " << expected << '\n';
        
        // 값 교환이 성공했을 경우
        expected = atomVal.load();

        ret = atomVal.compare_exchange_strong(expected, desired);
        std::cout << "AtomicVal : " << atomVal << ", Expected : " << expected << '\n';
    }

    // compare_exchange_weak()
    {
        std::atomic<int> atomVal(10);

        int expected = 10;
        int desired  = 100;

        // spurious failures 검증 없이 성공을 가정한 경우
        bool ret = atomVal.compare_exchange_weak(expected, desired);
        std::cout << "AtomicVal : " << atomVal << ", Expected : " << expected << '\n';

        // spurious failures 검증을 하는 경우
        atomVal.store(expected);

        // On these spurious failures, the function returns false while not modifying expected.
        // 이런 설명이 있긴 하지만 다른 스레드에서 값을 갱신하는 경우도 고려해야 하기 때문에 expected를 넣는 방향으로 가는 것이 좋다.
        while (false == atomVal.compare_exchange_weak(expected, desired))
        {
            expected = 10; // 10 대신 temp를 받아서 대입하는 것도 괜찮음.
        }

        std::cout << "AtomicVal : " << atomVal << ", Expected : " << expected << '\n';
    }

    return 0;
}
