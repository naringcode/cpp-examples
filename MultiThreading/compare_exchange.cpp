#include <iostream>
#include <atomic>

/* // compare_exchange_xxx()는 이렇게 생겼다.
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

// https://cplusplus.com/reference/atomic/atomic/compare_exchange_strong/
// https://www.ldoceonline.com/dictionary/spurious
// spurious : 가짜, 허위, 논리가 잘못된

// compare_exchange_strong()과 compare_exchange_weak()의 차이는 spurious failures에 있다.
//
// compare_exchange_strong()은 spurious failures를 허용하지 않지만
// compare_exchange_weak()는 spurious failures를 허용한다.
//
// 다시 말해 compare_exchange_weak()에서 교환에 실패했을 때 spurious failures가 뜰 수도 있고 안 뜰 수도 있다는 것이다.
// compare_exchange_xxx()는 false를 반환한다고 해도 expected의 값이 바뀌어야 한다.
// 하지만 spurious failures가 뜨면 expected의 값이 갱신되지 않는 문제가 생긴다.
// 이론은 이러한데 환상 속의 존재처럼 거의 뜨지 않는 문제이기도 하다.
// 하지만 만에 하나라는 것이 있으니 실제로 사용할 일이 있다면 expected에 대한 비교도 넣어야 한다.

int main()
{
    // 값 교환이 실패했을 때를 가정한 코딩
    std::atomic<int> atom(10);
    int expected = 50;

    // compare_exchange_strong()
    bool ret = atom.compare_exchange_strong(expected, 100);
    std::cout << "Atomic : " << atom << ", Expected : " << expected << '\n';

    // Reset
    expected = 50;

    // compare_exchange_weak()
    ret = atom.compare_exchange_weak(expected, 100);
    std::cout << "Atomic : " << atom << ", Expected : " << expected << '\n';

    return 0;
}
