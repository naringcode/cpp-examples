// Update Date : 2025-01-12
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <memory>
#include <atomic>
#include <barrier> // 메모리 배리어가 아닌 C++20부터 지원하는 동기 과정에서 사용되는 배리어임.
#include <thread>
#include <vector>
#include <string>
#include <cassert>

// SIMD 관련 헤더 파일로 메모리 배리어 정책에 대한 함수도 담고 있다.
#include <immintrin.h>

using namespace std;

#define BEGIN_NS(name) namespace name {
#define END_NS };

// C++에서의 volatile은 캐시와 직접적인 연관이 있는 그런 키워드는 아니지만
// 컴파일러의 최적화에 따라 메모리 접근 방식에 영향을 주는 키워드이다.
// !! 캐시 동작 자체는 CPU와 하드웨어의 메모리 모델에 의해 관리되는 것이기에 volatile을 통해 이를 제어할 순 없음. !!
// 
// 다른 언어에도 volatile 키워드가 있는 경우도 있는데 언어마다 기능은 상이하니 주의하도록 한다.
// 
// C++에서 volatile 키워드를 지정하면 컴파일러는 최대한 변수를 최적화하지 않고 가능하면 메모리에서 읽고 쓰게(read/write)끔 컴파일한다.
// !! 이것 또한 정확한 동작은 컴파일러의 구현이나 하드웨어를 구성하는 아키텍처에 따라 약간은 상이할 수 있음. !!
// 
// !! 중요 !!
// 위에서 언급하는 메모리는 일반적인 물리 메모리인 RAM 뿐만 아니라 CPU의 캐시 계층 메모리도 포함하는 개념이다.
// 쉽게 말해서 volatile 키워드를 지정한다고 해도 멀티스레딩 환경에서 발생하는 원자성과 가시성 문제는 해결되지 않는다.
// 또한 volatile 키워드는 CPU 캐시를 무효화하지 않는다(캐시 일관성이 틀어질 때 발생하는 캐시 일관성 프로토콜에는 영향을 받음).
// 
// --------------------------------------------------
// 
// https://en.cppreference.com/w/cpp/atomic/memory_order // Relationship with volatile 쪽에 나오는 내용
// In addition, volatile accesses are not atomic (concurrent read and write is a data race)
// and do not order memory (non-volatile memory accesses may be freely reordered around the volatile access).
// !! volatile을 통한 접근은 원자성이나 가시성과는 무관하다는 의미임. !!
// 
// C++에서의 volatile은 변수를 대상으로 한 최적화만을 방지하기 위한 목적으로 쓰기 때문에 가시성이나 원자성과는 무관하다.
// volatile을 적용한다고 해도 CPU의 명령어 재배치 문제와 경합에 따른 순차적인 관찰 순서(?) 및 갱신을 보장하지 않는다.
// 
// 매번 메모리에서 읽고 쓴다고 메모리에서 읽고 쓴다고 해도 레지스터에 처리할 값을 일시적으로 저장하고 나중에 메모리에 반영하는 로직을 사용하는 건 똑같다.
// 또한 메모리 배리어 정책이 적용되지 않으면 캐시 메모리를 사용하든 물리 메모리를 사용하든 CPU는 명령어 재배치를 통해 가급적 코드를 빠르게 실행하려고 한다.
// 
// !! volatile은 메모리 배리어 정책을 적용하지 않으면 스레드 간 동기화 문제도 해결해주지 않는다(명령어 재배치 문제는 여전함). !!
// 
// (중요하니까 한 번 더) volatile이 적용된다고 해도 메모리 자체는 일반적인 물리 메모리인 RAM 뿐만 아니라 CPU의 캐시 계층 메모리도 포함하는 개념이다.
// 
// --------------------------------------------------
// 
// atomic은 실제 물리 메모리를 대상으로 연산을 진행하기 위한 그런 자료형이 아니다.
// atomic은 "메모리 일관성 정책"과 "메모리 배리어 정책"이 적용된 자료형이다.
// 
// atomic에 적용되는 대상을 CPU가 한 번에 처리할 수 있으면 Lock-Free하게 연산을 진행한다(한 번에 처리할 수 없다면 Lock을 검).
// - 64비트 환경이라면 64바이트까지 원자적으로 처리 가능(구조체를 대상으로 해도 크기가 64바이트 이하면 원자적으로 처리 가능)
// - 32비트 환경이라면 32바이트까지 원자적으로 처리 가능(구조체를 대상으로 해도 크기가 32바이트 이하면 원자적으로 처리 가능)
// 
// --------------------------------------------------
// 
// 메모리 일관성 정책?
// 
// 다음은 캐시 일관성이 깨지는 예시이다.
// 1. Thread 1과 Thread 2를 돌리고 있는 각 코어의 캐시 메모리가 변수 A를 반영하고 있음.
// 2. Thread 1 쪽에서 변수 A의 값을 바꿈 -> Thread 1을 돌리고 있는 CPU 코어의 캐시 메모리가 갱신됨.
// 3. Thread 2를 돌리고 있는 CPU 코어의 캐시 메모리와의 일관성이 깨짐.
// 
// 일반 변수에만 국한된 것이 아닌 atomic 자료형을 써도 조건에 따라서 캐시 일관성이 깨질 수 있다.
// 
// 캐시 일관성이 깨지면 메모리 일관성이 깨지기 때문에 이를 방지하기 위한 정책이 존재한다.
// 
// !! 중요 !!
// 캐시 일관성은 언어가 보장하는 것이 아닌 하드웨어의 아키텍처가 택한 정책이 보장하는 것이다.
// 언어는 캐시 일관성이 유도되게 하드웨어 아키텍처(x64, x84 등)에 맞게 코드를 컴파일할 뿐이다.
// 
// https://en.wikipedia.org/wiki/Cache_coherence#Coherence_protocols
// https://en.wikipedia.org/wiki/Cache_coherency_protocols_(examples)
// https://www.geeksforgeeks.org/cache-coherence-protocols-in-multiprocessor-system/
// https://en.wikipedia.org/wiki/MESI_protocol
// 
// 캐시 일관성 정책은 CPU의 각 코어가 캐시 일관성 프로토콜을 이용한 통신을 통해서 보장된다.
// 
// 대표적인 일관성 프로토콜인 MESI protocol의 정책에 따라 캐시 메모리가 가질 수 있는 상태는 다음과 같다.
// - Modified (M) : 독점적으로 사용하고 있는 캐시 라인이 수정된 상황(언젠가 메인 메모리에 반영되어야 함)
// - Exclusive (E) : 해당 캐시 라인이 다른 프로세서에 적용되지 않아 독점적으로 사용하고 있는 상태(메인 메모리의 유일한 복사본)
// - Shared (S) : 동일한 캐시 라인이 다른 프로세서에도 적용되어 있으며 메인 메모리와 일치하는 상태
// - Invalid (I) : 다른 프로세서에 의해 데이터가 수정되어 무효화된 상태(캐시 라인이 유효하지 않으니 다시 읽어야 함)
// 
// 일관성 프로토콜 정책은 CPU마다 상이하지만 공통적으로 하드웨어 차원에서 각 코어가 반영한 캐시의 일관성이 깨지지 않게 해준다.
// 
// (중요) 캐시 일관성을 보장하는 주체는 volatile도 아니고 atomic도 아니고 언어도 아니다(캐시 일관성은 하드웨어 수준에서 보장함).
// 
// 위 캐시 일관성 정책이 올바로 반영되려면 매번 메모리를 읽어야 하기 때문에 atomic에선 최적화를 방지하기 위해 내부에서 volatile을 사용한다.
// 
// --------------------------------------------------
// 
// https://en.wikipedia.org/wiki/Memory_barrier
// https://en.wikipedia.org/wiki/Memory_ordering
// 
// 메모리 배리어 정책은 atomic을 구현하기 위한 핵심 중 하나이며 이건 명령어 재배치(또는 코드 재배치)와 깊은 연관성이 있다.
// 
// l_fence, s_fence, m_fence와 같은 메모리 배리어 코드에 적용해서 CPU의 메모리 정책에 따른 동작을 프로그래머 수준에서 결정할 수 있다.
// - l_fence : load fence
// - s_fence : store fence
// - m_fence : full memory fence(load fence와 store fence의 역할을 모두 수행함)
// 
// ## l_fence ##
// - 메모리에서 값을 읽는 load 작업에 대한 배리어를 설정함(읽기 순서 강제).
// - l_fence 이전의 모든 읽기 연산이 완료될 때까지 l_fence 이후의 읽기 연산이 수행되지 않게 함.
//   - 이전의 읽기 연산이 완료된 후에 다음 읽기 연산이 실행되게 함.
// - 이전의 load 연산이 완료된 후에 후속 load 연산을 실행하게 한다는 점에서 memory_order_acquire와 유사하게 동작함(동일한 기능은 아님).
// 
// ## s_fence ##
// - 메모리에서 값을 쓰는 store 작업에 대한 배리어를 설정함(쓰기 순서 강제).
// - s_fence 이전의 모든 쓰기 연산이 완료될 때까지 s_fence 이후의 쓰기 연산이 수행되지 않게 함.
//   - 이전의 쓰기 연산이 완료된 후에 다음 쓰기 연산이 실행되게 함.
// - 이전 store 연산이 완료된 후에 후속 store 연산을 실행하게 한다는 점에서 memory_order_release와 유사하게 동작함(동일한 기능은 아님).
// 
// ## m_fence ##
// - l_fence와 s_fence의 기능을 모두 수행하여 load와 store에 대한 종합적인 배리어를 설정함.
// - m_fence 이전의 모든 읽기와 쓰기 연산이 완료될 때까지 m_fence 이후의 읽기와 쓰기 연산이 수행되지 않게 함.
// - memory_order_seq_cst와 유사하게 동작함.
//   - memory_order_seq_cst처럼 모든 연산에 대해 순차적 일관성(Sequential Consistency)을 강제하는 건 아님.
//   - m_fence는 후속 메모리 연산을 실행하기 전에 앞서 작성한 연산들이 먼저 실행되는 것을 보장하기 위해 사용함(연산 간 순서를 제어하기 위한 목적으로 사용함).
// 
// !! 중요 !!
// 이러한 작업은 컴파일러의 최적화 작업에만 영향을 미치는 것이 아닌 CPU가 코드를 읽는 방식에도 영향을 미친다.
// CPU는 어셈블리 코드가 생성된 그대로 코드를 실행하지 않고 정책에 따라 코드를 재배치하여 실행할 수 있다.
// 
// 해당 명령어는 단일 스레드 프로그래밍 뿐만 아니라 병렬 프로그래밍에서 여러 스레드가 공유 메모리에 접근하는 과정에서도 영향을 미친다.
// - 단일 스레드에선 명령어 배치를 위한 성능 최적화나 특정 연산 순서를 강제하기 위한 목적으로 사용함.
// - 멀티 스레딩 프로그래밍에선 메모리 일관성을 유지하기 위한 목적으로 사용함.
// 
// l_fence, s_fence, m_fence는 하드웨어 수준의 명령어지만
// memory_order_acquire, memory_order_release, memory_order_seq_cst는 C++의 원자적 연산과 소프트웨어 수준의 동기화를 위한 정책이다.
// 
// l_fence, s_fence, m_fence는 직접적으로 하드웨어 명령어를 사용하지만
// memory_order_acquire, memory_order_release, memory_order_seq_cst는 컴파일 과정에서 명령어를 생성을 제어하기 위해 사용되는 정책이다.
// 
// --------------------------------------------------
// 
// https://en.cppreference.com/w/cpp/atomic/memory_order
// https://en.cppreference.com/w/cpp/atomic/atomic_thread_fence
// https://modoocode.com/271
// https://en.cppreference.com/w/cpp/atomic/kill_dependency
// 
// 메모리 정책에 대한 코드를 직접 배치하는 것보다 실제 개발할 때는 atomic을 사용하여 메모리 배리어의 수준을 결정하는 것이 좋다.
//
// atomic 관련 기능을 사용할 때는 memory_order를 통해서 메모리 배리어의 수준을 결정할 수 있다.
// 
// enum class memory_order : int {
//     relaxed,
//     consume,
//     acquire,
//     release,
//     acq_rel,
//     seq_cst,
// 
//     // LWG-3268
//     memory_order_relaxed = relaxed,
//     memory_order_consume = consume,
//     memory_order_acquire = acquire,
//     memory_order_release = release,
//     memory_order_acq_rel = acq_rel,
//     memory_order_seq_cst = seq_cst
// };
// 
// inline constexpr memory_order memory_order_relaxed = memory_order::relaxed;
// inline constexpr memory_order memory_order_consume = memory_order::consume;
// inline constexpr memory_order memory_order_acquire = memory_order::acquire;
// inline constexpr memory_order memory_order_release = memory_order::release;
// inline constexpr memory_order memory_order_acq_rel = memory_order::acq_rel;
// inline constexpr memory_order memory_order_seq_cst = memory_order::seq_cst;
// 
// atomic 관련 작업을 진행할 때 메모리 정책을 별도로 지정하지 않으면 가장 강력한 정책인 memory_oder_seq_cst(Sequential Consistency)가 적용되는데
// 이걸 쓰면 순차적 일관성이 보장되어 모든 스레드에서 모든 시점에 동일한 값을 관찰할 수 있게 해준다.
// 
// (중요) memory_order_seq_cst 정책은 컴파일러나 하드웨어가 메모리 연산을 재배치하는 것을 방지하며 런타임 시 메모리 연산이 순차적 일관성을 보장하게 해준다.
// 
// 가장 강력한 정책인 memory_order_seq_cst를 쓰면 순차적 일관성 정책으로 인해 원자적인 연산이 순서대로 처리되는 것을 보장하지만
// 가장 약한 정책인 memory_order_relaxed를 쓰면 대상의 원자적인 연산만 보장하고 재배치 관련해서는 일반 변수한다.
// 
// memory_order_acquire와 memory_order_release를 적절하게 연계하여 Release-Acquire ordering에 맞게 명령어가 생성되게 유도하는 방식도 있지만
// 코어를 설계하는 수준이 아니거나 유지보수 측면을 중시한다면 memory_order_seq_cst를 써서 개발해도 된다.
// 
// 직접 메모리 배리어의 수준을 지정하는 방식을 통해 최적화하면 성능은 좋아질 수 있으나 비효율적이고 유지보수하기도 힘들다.
// 특별한 이유가 없다면 가장 안전하고 직관적인 메모리 정책인 memory_order_seq_cst를 쓰도록 하자.
// 
// memory_order_seq_cst의 장점
// - 전역적인 순서를 보장하기에 동기화가 필요한 부분과 동작을 비교적 명확하게 예측 가능함.
// - 복잡한 동기화 관리가 필요하지 않기 때문에 안정성에 초점을 맞춘 프로그래밍이 가능함.
// - 현대 CPU와 컴파일러의 성능이 좋기 때문에 정말 성능이 민감한 부분이 아니라면 성능 저하가 그렇게 큰 것도 아님.
// 
// (중요) memory_order는 컴파일 과정에서 생성되는 dependency tree에 영향을 미친다(컴파일러마다 상이할 수 있음).
// 
// --------------------------------------------------
// 
// https://www.scs.stanford.edu/05au-cs240c/lab/i386/s03_01.htm
// https://preshing.com/20130618/atomic-vs-non-atomic-operations/
// http://cloudrain21.com/atomic-vs-non-atomic-calculation
// https://c9x.me/x86/html/file_module_x86_id_184.html
// https://www.felixcloutier.com/x86/cmpxchg8b:cmpxchg16b
// https://www.felixcloutier.com/x86/movdqu:vmovdqu8:vmovdqu16:vmovdqu32:vmovdqu64
// 
// 현대 CPU 기준 mov 연산은 다음 조건을 충족할 때 원자적으로 동작한다고 추정하고 있다(아키텍처에 따라 다를 수 있으며 내 견해가 들어가 있음).
// - 연산 대상이 되는 메모리는 경계에 맞게 정렬된(aligned) 상태로 있어야 함.
//   - 32비트면 4바이트, 64비트면 8바이트
// - 연산 크기는 레지스터나 메모리 버스가 허용하는 최대 크기 이내, 즉 CPU가 연산 가능한 최대 크기인 워드 단위 이내여야 함.
//   - 워드 단위 크기를 벗어나는 큰 데이터를 복사할 때는 여러 차례에 걸쳐 mov를 호출하도록 명령어를 생성함.
//   - DCAS를 구현할 때 사용하는 InterlockedCompareExchange128()는 16바이트 경계를 맞춰야 하는데 이건 mov가 아닌 cmpxchg16b로 구현되며 lock prefix가 붙으니 혼동하지 말 것
//   - 64비트 환경에서 8바이트 크기를 벗어나는 연산을 위한 cmpxchg16b나 movdqu 같은 명령어가 보이는데 이것 자체는 원자적인 연산이 아님.
// - mov에 의해 수행되는 메모리 읽기나 쓰기는 캐시 일관성 프로토콜의 영향을 받는 것으로 추정하고 있음.
//   - 연산의 원자성을 보장한다는 것을 의미하는 것은 아니지만 캐시 일관성은 유지시켜 준다는 것을 뜻함.
//   - 따라서 mov 연산과 다른 코어에서 동일 메모리 주소를 대상으로 한 mov 연산 사이에 경합이 발생했어도 중첩되지 않게 명령어가 설계되었다는 것을 뜻함.
// 
// --------------------------------------------------
// 
// https://modoocode.com/en/inst/lock
// https://stackoverflow.com/questions/8891067/what-does-the-lock-instruction-mean-in-x86-assembly
// https://hackmd.io/@vesuppi/Syvoiw1f8
// 
// atomic 연산을 통해 생성된 명령어를 보면 lock이 자주 보일 것이다.
// lock은 명령어가 아니라 명령어 앞에 붙는 instruction prefix이다(일명 lock 접두사).
// 
// (중요) lock이 붙으면 연산 대상이 되는 메모리의 캐시 라인에 락을 걸어 연산이 원자적으로 처리되게 해준다.
// 이를 통해 메모리 일관성을 보장할 수 있기에 lock은 캐시 일관성을 구현하는데 있어서 핵심이 되는 키워드이자 힌트이다.
// 
// !! 과거 CPU에 적용된 lock 접두사는 실제 메모리의 버스를 차단하는 BUS LOCK을 사용했으나 현대 CPU는 캐시 락(Cache Lock)을 사용한다. !!
// 
// lock이 적용된 명령어의 예시 코드를 보도록 하자.
// - lock inc [atomVal]
// : atomVal을 프로세서의 캐시 라인으로 가져오고 이를 독점하기 위해 다른 프로세서에서 atomVal을 가져온 캐시 라인이 있을 경우 잠근다.
// : 그 다음 소유권을 독점한 프로세서 쪽에서 atomVal을 증가시키는 연산을 수행하는데 이는 원자적인 연산이며 메모리 일관성을 보장한다.
// : 이때 다른 프로세서에서 atomVal을 캐싱한 캐시 라인이 있을 경우 캐시 무효화 작업이 이루어지며 캐시 일관성 프로토콜에 의해 동기화 과정이 수행된다.
// 
// https://modoocode.com/en/inst/lock
// https://modoocode.com/en/inst/xchg
// The XCHG instruction always asserts the LOCK# signal regardless of the presence or absence of the LOCK prefix.
// 
// 어셈블리 명령어 중에는 lock이 붙지 않았음에도 붙은 것처럼 취급하는 명령어들이 존재하며
// 원자적으로 값을 교환하는 xchg 명령어가 이러한 유형에 해당한다.
// 
// 다음 명령어에서 lock 접두사는 편의상 내가 넣은 것이지 디스어셈블러 상에서는 보이는 것이 아니다(혼동하지 말 것).
// 설명을 읽어보면 실제로 이렇게 동작한다고 되어 있다.
// - (lock) xchg ecx, [atomVal] : atomVal에 있는 값과 ecx 레지스터에 있는 값을 원자적으로 교환함.
// 
// (중요) lock 접두사가 붙은 명령어는 메모리 배리어가 적용되어 명령어 재배치 문제를 방지하는데 이는 xchg에도 동일하게 적용된다.
// 
// 종합적으로 보면 atomic은 메모리 일관성과 메모리 배리어 정책이 적용된다고 볼 수 있다.
// 
/** 
 * (진짜 중요) 요약
 * - atomic은 컴파일 단계에서 메모리 일관성과 메모리 배리어를 적용될 수 있게 명령어를 생성하기 위한 자료형이다.
 * - atomic 자체는 메모리 일관성과 메모리 배리어가 적용될 수 있게 명령어를 생성할 뿐이다.
 * - 실질적으로 메모리 일관성과 메모리 배리어를 적용하는 건 운영체제나 CPU 그리고 하드웨어 아키텍처 쪽이다.
 * - !! 원자성과 Lock은 다르다. !!
 */

BEGIN_NS(Case01)

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
volatile int atomVal1ToVolatileInt;
int atomVal1ToBasicInt;

atomic<int> atomVal2;
volatile int atomVal2ToVolatileInt;
int atomVal2ToBasicInt;

// 자료형 두 개 다 테스트해서 어셈블리 명령어가 어떻게 생성되는지 확인할 것
int incrementor; // volatile을 붙이지 않고 변수가 최적화되게 유도
// atomic<int> incrementor;

// Debug와 Release 모드로 코드를 컴파일하면 어셈블리 명령어가 어떻게 생성되는지 확인하기 위한 코드
// 디스어셈블러를 통해서 코드 최적화가 어떻게 이루어지는지 확인할 것
void ThreadMain()
{
    // 일반 정수 타입과 atomic 타입을 번갈아가며 incrementor가 어떻게 적용되는지 확인할 것
    for (int i = 0; i < 100'000; i++)
    {
        // volatile int(a) : volatile int(e)와 대응
        a = incrementor++;

        // int(b) : volatile int(f)와 대응
        b = incrementor++;

        // volatile int(c)
        c = incrementor++;

        // int(d)
        d = incrementor++;
    }

    // volatile int(e) = volatile int(a)
    e = a;

    // volatile int(f) = int(b)
    f = b;

    // int(g) = volatile int(c)
    g = c;

    // int(h) = int(d)
    h = d;

    // atomic 자료형을 대상으로 하는 확인 작업(Debug와 Release 모드 둘 다 확인할 것)
    //
    // https://modoocode.com/en/inst/lock
    // https://modoocode.com/en/inst/xchg
    // The XCHG instruction always asserts the LOCK# signal regardless of the presence or absence of the LOCK prefix.
    // 
    // Release 모드에서 atomic 자료형을 대상으로 store()를 진행하면 xchg 명령어가 생성될 것이다.
    atomVal1.store(incrementor++);
    atomVal1.fetch_add(1);
    atomVal1ToVolatileInt = atomVal1.load(); // volatile int(atomTemp1)
    atomVal1ToBasicInt    = atomVal1.load(); // int(atomTemp2)

    //
    atomVal2 = incrementor++;
    atomVal2++;
    atomVal2ToVolatileInt = atomVal2; // volatile int(atomTemp3)
    atomVal2ToBasicInt    = atomVal2; // int(atomTemp4)
}

__declspec(noinline) void Run()
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

    // 결과를 출력하기 위한 용도(큰 뜻이 있는 건 아님)
    cout << a << '\n'; // volatile int(a)
    cout << b << '\n'; // int(b)
    cout << c << '\n'; // volatile int(c)
    cout << d << '\n'; // int(d)
    cout << e << '\n'; // volatile int(e)
    cout << f << '\n'; // volatile int(f)
    cout << g << '\n'; // int(g)
    cout << h << '\n'; // int(h)
    cout << atomVal1 << '\n';  // atomic<int>
    cout << atomVal1ToVolatileInt << '\n'; // volatile int(atomTemp1)
    cout << atomVal1ToBasicInt << '\n';    // int(atomTemp2)
    cout << atomVal2 << '\n';  // atomic<int>
    cout << atomVal2ToVolatileInt << '\n'; // volatile int(atomTemp3)
    cout << atomVal2ToBasicInt << '\n';    // int(atomTemp4)
}

END_NS

BEGIN_NS(Case02)

int a = 0;
int b = 0;
int c = 0;

int temp[10];

__declspec(noinline) void Run_lfence()
{
    temp[0] = a;
    temp[1] = b;
    temp[2] = c;

    // _mm_lfence()가 적용되면 위에 있는 a, b, c를 읽는 작업을 전부 마쳐야 아래 있는 읽는 작업을 진행할 수 있다.
    _mm_lfence(); // 어셈블리 명령어 "lfence"가 생성됨.
    // std::atomic_thread_fence(memory_order_acquire); // 어셈블리 명령어를 생성하지는 않지만 lfence가 적용된 것과 유사하게 코드를 컴파일함(컴파일러나 플랫폼에 따라 상이할 수 있음).

    temp[3] = a;
    temp[4] = b;
    temp[5] = c;
}

__declspec(noinline)void Run_sfence()
{
    a = 100;
    b = 200;
    c = 300;

    // _mm_sfence()가 적용되면 위에 있는 a, b, c를 갱신하는 쓰기 작업이 전부 완료되어야 아래의 a, b, c를 갱신할 수 있다.
    _mm_sfence(); // 어셈블리 명령어 "sfence"가 생성됨.
    // std::atomic_thread_fence(memory_order_release); // 어셈블리 명령어를 생성하지는 않지만 sfence가 적용된 것과 유사하게 코드를 컴파일함(컴파일러나 플랫폼에 따라 상이할 수 있음).

    a = 400;
    b = 500;
    c = 600;
}

__declspec(noinline)void Run_mfence()
{
    temp[0] = a;
    temp[1] = b;
    temp[2] = c;

    a = 100;
    b = 200;
    c = 300;

    // _mm_mfence()가 적용되면 위에 있는 모든 읽기 및 쓰기 작업이 완료되어 메모리에 반영되어야 아래 코드를 실행할 수 있다.
    _mm_mfence(); // 어셈블리 명령어 "mfence"가 생성됨.
    // std::atomic_thread_fence(memory_order_seq_cst); // 어셈블리 명령어를 생성하지는 않지만 mfence가 적용된 것과 유사하게 코드를 컴파일함(컴파일러나 플랫폼에 따라 상이할 수 있음).
    // memory_order_seq_cst는 가장 강력한 정책이며 순차적 일관성과 가시성을 보장함.

    temp[3] = a;
    temp[4] = b;
    temp[5] = c;

    a = 400;
    b = 500;
    c = 600;
}

__declspec(noinline) void Run()
{
    Run_lfence();
    
    cout << "--------------------------------------------------\n";

    Run_sfence();

    cout << "--------------------------------------------------\n";

    Run_mfence();
}

END_NS

BEGIN_NS(Case03)

barrier syncPoint{ 2 };

atomic<int> x{ 0 };
atomic<int> y{ 0 };

// https://en.cppreference.com/w/cpp/atomic/memory_order
// 위 문서에 나오는 Relaxed ordering 예시(relaxed에서 발생하는 문제 상황)
__declspec(noinline) void Run()
{
    thread th1{ [] {
        syncPoint.arrive_and_wait();

        auto r1 = y.load(memory_order_relaxed); // A
        x.store(r1, memory_order_relaxed);      // B

        cout << r1 << '\n';
    } };

    thread th2{ [] {
        syncPoint.arrive_and_wait();

        auto r2 = x.load(memory_order_relaxed); // C
        y.store(42, memory_order_relaxed);      // D
        
        cout << r2 << '\n';
    } };

    // memory_order_relaxed는 명령어의 원자성과 수정 순서에 대한 일관성은 보장하지만 연산 순서에 대한 동기화는 진행하지 않기에 명령어 재배치를 허용한다.
    // 
    // 위 코드가 최적화되어 실행되면 r1와 r2에는 둘 다 42가 저장될 수 있다(x와 y 아님).
    // 눈으로만 보면 이해가 안 되는 내용이지만 코드가 최적화되는 방식을 보면 말이 안 되는 건 아니다.
    // 
    // 코드를 보면 Thread 1은 A를 실행한 다음 B를 실행하고, Thread 2를 보면 Thread C를 실행한 다음 D를 실행하게 되어 있다.
    // 하지만 y의 수정 순서를 보면 D가 A보다 먼저 실행되면 안 된다고 막는 제약은 없다.
    // 마찬가지로 x의 수정 순서를 보면 B가 C보다 먼저 실행되면 안 된다고 막는 제약도 없다.
    // 
    // 즉, D에 의한 예상치 못한 결과가 A에 반영되면 다음과 같이 코드가 실행될 수 있다.
    // Run : D -> A -> B -> C
    // 
    // memory_order_relaxed 정책이 메모리 접근 순서를 접근 순서를 보장하는 건 아니지만
    // Thread 1의 코드는 데이터 의존성에 의해 컴파일되면 A -> B의 순서로 명령어가 실행되게 코드가 생성된다.
    // 하지만 Thread 2를 보면 데이터 의존성이 없기 때문에 컴파일되면 D -> C의 순서로 명령어가 생성될 수도 있다.

    th1.join();
    th2.join();

    cout << "Main Done.\n";
}

END_NS

BEGIN_NS(Case04)

constexpr int kThreadNum = 10;

barrier syncPoint{ kThreadNum };

atomic<int> sharedCnt;

// https://en.cppreference.com/w/cpp/atomic/memory_order
// 위 문서에 나오는 Relaxed ordering 예시(relaxed의 올바른 사용법)
void ThreadMain()
{
    syncPoint.arrive_and_wait();

    for (int i = 0; i < 100'000; i++)
    {
        sharedCnt.fetch_add(1, memory_order_relaxed);
    }
}

__declspec(noinline) void Run()
{
    vector<thread> thVec;

    for (int i = 0; i < kThreadNum; i++)
    {
        thVec.emplace_back(ThreadMain);
    }

    for (thread& th : thVec)
    {
        th.join();
    }

    cout << "Final counter value is " << sharedCnt << '\n';

    // 동기화는 중요하지 않지만 변수 자체에 원자적인 연산이 이루어져야 경우라면 memory_order_relaxed를 써도 된다.
    // 
    // 문서를 보면 스마트 포인터와 같이 레퍼런스 카운트를 올리는 과정이 필요한 경우 memory_order_relaxed를 써도 된다고 나와 있다.
    // 
    // note that decrementing the std::shared_ptr counters requires acquire-release synchronization with the destructor
    // 반면에 스마트 포인터의 경우 차감 로직까지 구현한다면 acquire-release synchronization가 필요하다고 나와 있으니 주의하자.
}

END_NS

BEGIN_NS(Case05)

// acquire와 release는 메모리 재배치 문제를 해결하기 위한 정책으로 코드 생성 시 스레드에 제약을 가한다.
// !! 일종의 lock과 unlock의 개념이라 생각하면 됨. !!
// 
// 1. acquire 이후에 나오는 코드들은 acquire 이전으로 재배치되지 않음.
// #============================================================#
// #             ↑ reordering some codes is not allowed         #
// # -----acquire--------------------                           #
// # some codes  ↓ reordering some codes is allowed             #
// #============================================================#
// 
// 2. release 이전에 나오는 코드들은 release 이후로 재배치되지 않음.
// #============================================================#
// # some codes  ↑ reordering some codes is allowed             #
// # -----release--------------------                           #
// #             ↓ reordering some codes is not allowed         #
// #============================================================#
// 
// 이러한 내용을 종합하면 이런 그림이 나온다.
// #============================================================#
// #     ↑ reordering some codes is not allowed                 #
// # --acquire--------------------                              #
// #     ↓ reordering some codes is allowed                     #
// # some codes                                                 #
// #     ↑ reordering some codes is allowed                     #
// # --release--------------------                              #
// #     ↓ reordering some codes is not allowed                 #
// #============================================================#
// 
// 이러한 동작은 멀티 스레드 프로그래밍을 할 때도 반영된다.
// 한 스레드에선 변수를 store(release)로 저장하고, 다른 스레드에선 load(acquire)로 값에 접근할 경우,
// load(acquire)를 사용하는 스레드 쪽에선 store(release)가 수행되기 이전에 반영된 메모리 변경 내역을 조회할 수 있다.
// 
// 코드 재배치 정책의 기준점은 서로 재배치될 수 없기에 acquire와 release가 위치를 바꿔가며 재배치되는 것은 불가능하다.
// - acquire -> release 순으로 작성한 경우
//   - acquire는 release 아래로 재배치되지 않음.
//   - release는 acquire 위로 재배치되지 않음.
// - release -> acquire 순으로 작성한 경우
//   - release는 acquire 아래로 재배치되지 않음.
//   - acquire는 release 위로 재배치되지 않음.
// - 마찬가지로 acquire -> acquire나 release -> release도 위치를 바꿔가며 재배치되는 것은 불가능하다.
//

atomic<string*> ptr;
int data;

// https://en.cppreference.com/w/cpp/atomic/memory_order
// 위 문서에 나오는 Release-Acquire ordering 예시
void Producer()
{
    string* p = new string{ "Hello" };
    data = 42;

    // release 이전에 나오는 코드들은 release 이후로 재배치되지 않음.
    ptr.store(p, memory_order_release);
}

void Consumer()
{
    string* p2;

    // acquire 이후에 나오는 코드들은 acquire 이전으로 재배치되지 않음.
    while (nullptr == (p2 = ptr.load(memory_order_acquire)))
           ;

    assert(*p2 == "Hello"); // never fires
    assert(data == 42); // never fires
}

void Run()
{
    thread th1{ Producer };
    thread th2{ Consumer };

    th1.join();
    th2.join();

    cout << "Main Done.\n";
}

END_NS


BEGIN_NS(Case06)

vector<int> data;
atomic<int> flag{ 0 };

// https://en.cppreference.com/w/cpp/atomic/memory_order
// 위 문서에 나오는 Release-Acquire ordering 예시(relaxed를 추가하여 3개의 스레드를 돌림)
void ReleaseThread()
{
    data.push_back(42);

    // 위 코드는 항상 아래 코드보다 선행된다.
    flag.store(1, memory_order_release);
}

void RelaxedThread()
{
    int expected = 1;
    int desired  = 2;

    // memory_order_relaxed is okay because this is an RMW(Read-Modify-Write),
    // and RMWs (with any ordering) following a release form a release sequence
    while (false == flag.compare_exchange_strong(expected, desired, memory_order_relaxed))
    {
        expected = 1;
    }
}

void AcquireThread()
{
    // 아래 코드는 위로 재배치될 수 없다.
    while (flag.load(memory_order_acquire) < 2)
        ;

    // if we read the value 2 from the atomic flag, we see 42 in the vector
    assert(data.at(0) == 42); // will never fire
}

void Run()
{
    thread th1{ ReleaseThread };
    thread th2{ RelaxedThread };
    thread th3{ AcquireThread };

    th1.join();
    th2.join();
    th3.join();

    cout << "Main Done.\n";
}

END_NS
 
// The specification of release-consume ordering is being revised, and the use of memory_order_consume is temporarily discouraged. (since C++17)
// 문서를 보면 consume은 개정 중이라고 되어 있으니 따로 적지는 않았으며,
// memory_order_seq_cst은 가장 강력한 정책이라 보이는 순서대로 동작하기에(순차적 일관성과 가시성 보장) 적지 않았다.

int main()
{
    // Case01::Run();
    // Case02::Run();
    // Case03::Run();
    // Case04::Run();
    // Case05::Run();
    Case06::Run();

    return 0;
}
