// Update Date : 2024-10-11
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>

/**
 * volatile은 캐시와 직접적인 연관이 있는 그런 키워드가 아니다.
 * C++에서의 volatile은 컴파일러 최적화와 관련이 있는 키워드이다
 * 
 * !! 다른 언어도 volatile 키워드가 있긴 언어마다 그 기능이 상이하니 주의할 것 !!
 * 
 * C++에서 volatile 키워드를 지정하면 컴파일러는 변수를 최적화하지 않고 매번 항상 메모리에서 읽고 쓰게 한다.
 * 
 * !! (중요) 이 메모리는 일반적인 물리 메모리 RAM 뿐만 아니라 CPU의 캐시 계층 메모리도 포함하는 개념이다. !!
 */
/**
 * C++에서의 volatile은 변수를 대상으로 하는 최적화만 막기 때문에 가시성과는 무관하다.
 * volatile을 적용해도 CPU의 명령어 재배치 문제는 해결되지 않는다.
 * 
 * 매번 메모리에서 읽고 쓴다고 해도 레지스터에 처리할 값을 일시적으로 저장하고 나중에 메모리에 반영하는 로직을 사용하는 건 똑같다.
 * 또한 캐시 메모리를 사용하든 물리 메모리를 사용하든 CPU는 명령어 재배치를 통해 코드를 빠르게 실행하고자 한다.
 * 
 * !! volatile은 메모리 배리어 정책이나 스레드 간의 동기화 문제를 해결해주지 않는다(명령어 재배치 문제는 여전하다는 뜻). !!
 * 
 * !! (중요하니까 한 번 더) 이 메모리는 일반적인 물리 메모리 RAM 뿐만 아니라 CPU의 캐시 계층 메모리도 포함하는 개념이다. !!
 */
/**
 * (중요) atomic은 실제 물리 메모리를 대상으로 연산하는 그런 자료형이 아니다.
 * (중요) atomic은 "메모리 일관성 정책"과 "메모리 배리어 정책"을 다루는 자료형이다.
 * 
 * atomic에 적용되는 대상을 CPU가 한 번에 처리할 수 있으면 Lock을 걸지도 않는다.
 * - 64비트 환경에선 64바이트까지 원자적으로 처리 가능(구조체를 대상으로 해도 64바이트 이하면 원자적 처리 가능)
 * - 32비트 환경에선 32바이트까지 원자적으로 처리 가능(구조체를 대상으로 해도 32바이트 이하면 원자적 처리 가능)
 * 
 * --------------------------------------------------
 * 
 * 메모리 일관성 정책부터 보도록 하자.
 * 
 * 다음은 캐시 일관성이 깨지는 예시이다.
 * 1. Thread 1과 Thread 2의 각 캐시 메모리에 변수 A가 들어가 있음.
 * 2. Thread 1이 변수 A의 값을 바꿈 -> Thread 1을 돌리고 있는 CPU 코어의 캐시 메모리가 갱신됨.
 * 3. Thread 2를 돌리고 있는 CPU 코어의 캐시 메모리와의 일관성이 깨짐.
 * 
 * 일반 변수 뿐만 아니라 atomic 자료형을 써도 조건만 충족하면 캐시 일관성이 깨지는 문제가 발생한다.
 * 
 * !! 캐시 일관성이 깨지면 메모리 일관성도 깨진다. !!
 * 
 * !! (중요) 캐시 일관성은 언어가 보장하는 것이 아니다(언어는 캐시 일관성이 유도되게 할 뿐임). !!
 * 
 * https://en.wikipedia.org/wiki/Cache_coherence#Coherence_protocols
 * https://www.geeksforgeeks.org/cache-coherence-protocols-in-multiprocessor-system/
 * https://en.wikipedia.org/wiki/MESI_protocol
 * 
 * 캐시 일관성은 CPU의 각 코어가 캐시 일관성 프로토콜을 이용한 통신을 통해서 보장하는 것이다.
 * 
 * 대표적인 일관성 프로토콜인 MESI protocol의 정책에 따라 캐시 메모리가 가질 수 있는 상태는 다음과 같다.
 * - Modified (M) : 독점적으로 사용하고 있는 캐시 라인이 수정된 상황(언젠가 메인 메모리에 반영되어야 함)
 * - Exclusive (E) : 해당 캐시 라인이 다른 프로세서에 적용되지 않아 독점적으로 사용하고 있는 상태(메인 메모리의 유일한 복사본)
 * - Shared (S) : 동일한 캐시 라인이 다른 프로세서에도 적용되어 있으며 메인 메모리와 일치하는 상태
 * - Invalid (I) : 다른 프로세서에 의해 데이터가 수정되어 무효화된 상태(캐시 라인이 유효하지 않으니 다시 읽어야 함)
 * 
 * 일관성 프로토콜 정책은 CPU마다 상이하지만 어찌되었든 하드웨어 차원에서 각 코어가 가진 캐시의 일관성이 깨지지 않게 해준다.
 * 캐시 무효화가 발생할 수도 있고 코어 간 통신을 통해 캐시의 값만 갱신할 수도 있는 것이다.
 * 
 * 매번 메모리를 대상으로 작업을 진행해야 위 정책을 반영할 수 있기 때문에 atomic은 내부에서 volatile을 사용한다.
 * 
 * !! atomic은 메모리 일관성을 유지하기 위해 CPU 차원에서 캐시 일관성을 보장할 수 있게 명령어를 생성할 뿐이다. !!
 * 
 * !! (중요) 캐시 일관성을 보장하는 주체는 atomic도 아니고 언어도 아니다. !!
 * 
 * --------------------------------------------------
 * 
 * atomic의 메모리 배리어 정책은 핵심 중 핵심이며 이건 명령어 재배치(코드 재배치)와 깊은 연관성이 있다.
 * 
 * l_fence, s_fence, m_fence를 직접 구성해서 메모리 배리어를 칠 수 있지만
 * CPU의 특정 명령어를 코드에 배치하여 메모리 배리어를 구성하는 것도 가능하다(atomic이 사용하는 방식).
 * 
 * https://modoocode.com/271
 * 
 * atomic을 사용할 때 memory_order를 통해서 메모리 배리어의 수준을 결정할 수 있다.
 * 
 * enum class memory_order : int {
 *     relaxed,
 *     consume,
 *     acquire,
 *     release,
 *     acq_rel,
 *     seq_cst,
 * 
 *     // LWG-3268
 *     memory_order_relaxed = relaxed,
 *     memory_order_consume = consume,
 *     memory_order_acquire = acquire,
 *     memory_order_release = release,
 *     memory_order_acq_rel = acq_rel,
 *     memory_order_seq_cst = seq_cst
 * };
 * 
 * 메모리 배리어는 CPU의 명령어 재배치 기능을 제한하기 위해서 사용한다(완전히 막거나 차단한다는 의미가 아님(정책에 수준이 다를 뿐임)).
 * 
 * !! (중요) atomic은 메모리 배리어를 형성하기 위해 컴파일 단계에서 적절한 명령어를 생성할 수 있게 도와주는 역할을 할 뿐이다. !!
 * 
 * 메모리 정책을 별도로 지정하지 않으면 가장 강력한 정책인 memory_order_seq_cst(Sequential Consistency)가 적용되는데
 * 이걸 쓰면 순차적 일관성이 보장되어 모든 스레드에서 모든 시점에 동일한 값을 관찰할 수 있게 해준다.
 * 
 * !! (중요) memory_order_seq_cst 정책은 메모리나 명령어가 재배치되지 하지 않게 컴파일 단계에서 명령어를 생성해주겠다는 뜻이다. !!
 * 
 * 1)
 * atomVal1.store(incrementor++, memory_order_seq_cst);
 * atomVal1.fetch_add(1, memory_order_relaxed);
 * 
 * 2)
 * atomVal1.store(incrementor++, memory_order_relaxed);
 * atomVal1.fetch_add(1, memory_order_seq_cst);
 * 
 * 정책을 어떻게 적용하냐에 따라 명령어가 다르게 생성된다.
 * 디스어셈블러를 통해서 생성된 명령어를 확인하는 방식으로 분석하면 된다.
 * 
 * memory_order_acquire와 memory_order_release를 매칭하여 명령어를 배치하는 방식도 있는데
 * 일반 PC에서는 memory_order_seq_cst를 써도 퍼포먼스에 크게 성능에 영향을 주지 않으니 그냥 기본적으로 적용되는 것을 쓰도록 하자.
 * 
 * !! 직접 메모리 배리어 수준을 지정하여 최적화하는 건 비효율적이고 유지보수하기도 힘들다. !!
 * !! 일반적인 상황이라면 그냥 memory_order_seq_cst를 쓰도록 하자. !!
 * 
 * --------------------------------------------------
 * 
 * https://modoocode.com/en/inst/lock
 * https://stackoverflow.com/questions/8891067/what-does-the-lock-instruction-mean-in-x86-assembly
 * 
 * atomic으로 생성된 명령어를 볼 때 가장 많이 나오는 것이 lock이다.
 * lock은 명령어가 아니고 명령어 앞에 붙는 instruction prefix이다(일명 lock 접두사).
 * 
 * (중요) lock은 "메모리 버스"를 잠가서 다른 CPU의 코어가 해당 메모리 위치에 접근하지 못하게 차단한다.
 * 메모리 일관성을 보장하기 위해서 사용되기에 lock은 캐시 일관성을 구현하는데 있어서 핵심이 되는 키워드이다.
 * 
 * 다음은 아래 예시 코드를 컴파일하고 디스어셈블러로 관찰하면 볼 수 있는 명령어이다.
 * - lock inc [atomVal1] : atomVal1의 메모리 주소에 lock을 걸고서 값을 증가시킴(원자적 증가)
 * 
 * https://modoocode.com/en/inst/lock
 * https://modoocode.com/en/inst/xchg
 * The XCHG instruction always asserts the LOCK# signal regardless of the presence or absence of the LOCK prefix.
 * 
 * atomic을 통해 명령어를 생성했음에도 정책에 따라 lock이 붙지 않는 경우도 있다.
 * 하지만 모든 명령어가 그런 것은 아니고 어떤 명령어는 항상 lock이 붙어 있다고 취급해야 한다(xchg가 이런 유형에 해당).
 * 
 * 마찬가지로 예시 코드를 컴파일하고 디스어셈블러로 관찰하면 볼 수 있는 명령어이다.
 * lock 접두사는 디스어셈블러 상에서 보이지 않지만 편의상 내가 넣은 것이니 혼동하면 안 된다.
 * - (lock) xchg ecx, [atomVal1] : atomVal1의 메모리 주소에 lock을 걸고서 ecx 레지스터와 값을 교환함(원자적 교환)
 * 
 * !! (중요) xchg는 원자적으로 데이터를 교환하기 위해 사용하지만 그 이전에 메모리 배리어를 적용하여 명령어 재배치를 방지한다. !!
 * 
 * xchg는 메모리의 일관성(혹은 데이터나 캐시 일관성)을 유지하기 위해서 사용되며 동시에 명령어 재배치 문제를 막는다.
 * 
 * 즉, atomic이 적용하는 메모리 일관성과 메모리 배리어 정책은 떼려야 뗄 수 없는 관계인 셈이다.
 * 
 */
/** 
 * (진짜 중요) 요약
 * - atomic은 컴파일 단계에서 적절한 명령어를 배치하여 메모리 일관성과 메모리 배리어를 적용할 수 있게 해준다.
 * - atomic 자체는 메모리 일관성과 메모리 배리어가 적용될 수 있게 명령어를 생성할 뿐이다.
 * - 실질적으로 메모리 일관성과 메모리 배리어를 적용하는 건 운영체제나 CPU이다.
 * - !! 원자성과 Lock은 다르다. !!
 */

using namespace std;

// 테스트 케이스
// 1. volatile int = volatile int
// 2. volatile int = int
// 3. int = volatile int
// 4. int = int
volatile int a;
int b;
volatile int c;
int d;

volatile int e;
volatile int f;
int g;
int h;

atomic<int> atomVal1;
volatile int atomTemp1;
int atomTemp2;

atomic<int> atomVal2;
volatile int atomTemp3;
int atomTemp4;

//
int incrementor; // incrementor 자체는 최적화가 적용되게 volatile을 붙이지 않음.
// atomic<int> incrementor;

void ThreadMain()
{
    // !! Debug와 Release 모드 둘 다 돌려서 디스어셈블러로 확인할 것 !!

    // !! 최적화 적용 여부 확인 !!

    // volatile int(a) : volatile int(e)와 대응
    a = incrementor++; // 100;

    // int(b) : volatile int(f)와 대응
    b = incrementor++; // 200;

    // volatile int(c)
    c = incrementor++; // 300;

    // int(d)
    d = incrementor++; // 400;

    // volatile int(e) = volatile int(a)
    e = a;

    // volatile int(f) = int(b)
    f = b;

    // int(g) = volatile int(c)
    g = c;

    // int(h) = int(d)
    h = d;

    // atomic 자료형을 대상으로 확인(Debug와 Release 모드 둘 다 확인할 것)
    // 
    // 최적화 모드에서 atomic 자료형에 값을 설정할 때는 xchg 명령어가 사용됨.
    //
    // https://modoocode.com/en/inst/lock
    // https://modoocode.com/en/inst/xchg
    // The XCHG instruction always asserts the LOCK# signal regardless of the presence or absence of the LOCK prefix.

    atomVal1.store(incrementor++); // 500);
    atomVal1.fetch_add(1);
    atomTemp1 = atomVal1.load(); // volatile int(atomTemp1)
    atomTemp2 = atomVal1.load(); // int(atomTemp2)

    //
    atomVal2 = incrementor++; // 600;
    atomVal2++;
    atomTemp3 = atomVal2; // volatile int(atomTemp3)
    atomTemp4 = atomVal2; // int(atomTemp4)
}

int main()
{
    vector<thread> thVec;

    for (int32_t i = 0; i < 10; i++)
    {
        thread th(ThreadMain);

        thVec.push_back(std::move(th));
    }

    for (thread& th : thVec)
    {
        th.join();
    }

    // 최적화를 했을 때와 하지 않았을 때 어떤 값이 들어가는지 확인
    cout << a; // volatile int(a)
    cout << b; // int(b)
    cout << c; // volatile int(c)
    cout << d; // int(d)
    cout << e; // volatile int(e)
    cout << f; // volatile int(f)
    cout << g; // int(g)
    cout << h; // int(h)
    cout << atomVal1;  // atomic<int>
    cout << atomTemp1; // volatile int(atomTemp1)
    cout << atomTemp2; // int(atomTemp2)
    cout << atomVal2;  // atomic<int>
    cout << atomTemp3; // volatile int(atomTemp3)
    cout << atomTemp4; // int(atomTemp4)

    return 0;
}
