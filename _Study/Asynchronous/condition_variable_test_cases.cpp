// Update Date : 2025-01-10
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

using namespace std;

#define BEGIN_NS(name) namespace name {
#define END_NS };

// https://en.cppreference.com/w/cpp/thread/condition_variable
// 위 사이트에서 condition_variable에 대한 내용 한 번은 읽고 오기.

// condition_variable을 사용하면서 들었던 의문점을 정리한 내용
//
// !! 중요 !!
// 조건 변수 자체는 mutex만 알맞게 적용한다면 원자적으로 동작한다.
// 조건 변수를 대상으로 notify 계열의 함수를 호출하면 mutex 변수에 의해 동기화된다.
//  
// !! 주의 !!
// 조건 변수가 대기 상태에 빠지는 것과 mutex의 lock에 의해 대기하는 건 다른 개념이다.
// 또한 Predicate 내에서 공유 자원에 접근하여 유효성을 체크하는 로직이 들어갔을 경우에는
// 다른 스레드에서 해당 공유 자원을 사용할 때는 조건 변수를 대상으로 사용한 mutex에 lock을 걸고 사용해야 한다.
// 조건 변수의 Predicate에서 공유 자원에 접근하는데 다른 스레드에서 lock을 걸지 않고 공유 자원을 사용하는 건 잘못된 사용 방식이다.
// 
// https://en.cppreference.com/w/cpp/thread/condition_variable
// 위 문서를 보면 다음 내용이 있다.
// ~~ until the condition variable is notified, a timeout expires, or a spurious wakeup occurs,
// then atomically acquires the mutex before returning
// 
// https://en.wikipedia.org/wiki/Spurious_wakeup
// 사용자가 notify 계열의 함수를 호출하지 않아도 Spurious Wakeup이 발생할 경우 깨어난다고 되어 있다.
// 일단 Spurious Wakeup으로 인해 깨어나는 현상을 주의하도록 하자.
// 
// 대기 중인 스레드가 깨어나면 다음 과정을 거친다(Predicate를 전달한 상태라고 가정).
// 1. 대기 상태
// 2. wakeup
// <----- 다른 쪽에서 먼저 lock을 잡고 사용할 수도 있다면 대기
// 3. lock 걸기
// 4. 조건 체크

// 결론 정리
// 
// ## 조건이 있는 wait()를 사용했을 경우 ##
// 1. Lock의 소유권을 획득하는 것이 먼저임.
// 2. Predicate를 통해 조건 확인함(wait()를 호출한 직후라면 notify 계열의 함수를 호출하지 않아도 조건을 확인함).
// 3-1. Predicate가 true를 반환하면 wait()를 빠져나옴.
// 3-2. Predicate가 false를 반환하면 조건 변수는 대기 상태에 들어가고 wait 계열의 함수가 호출되었을 때 깨어나서 1번 로직부터 다시 실행함.
// 
// ## 조건이 없는 wait()를 사용했을 경우 ##
// 1. 대기 상태로 진입함.
// 2. notify 계열의 함수가 호출될 때까지 대기함(lock에 대한 소유권은 없는 상태).
// 3. notify 계열의 함수가 호출된다면 lock에 대한 소유권 획득을 시도함.
// 4. lock의 소유권을 획득했다면 wait()를 빠져나옴.
// 
// !! 중요 !!
// notify 계열의 함수를 먼저 호출한 다음 wait()를 호출하면?
// wait()를 빠져나올 것 같지만 조건 변수는 대기 상태에 들어간다(Windows의 Event와는 다른 부분).
// 조금 이상하긴 해도 옵저버 패턴처럼 동작한다고 숙지하자.
// 

// 조건 변수는 다음 3가지가 함께 묶여서 동작한다고 외워야 한다.
// 1. std::mutex mtx // unique_lock<mutex> lock(mtx) // lock_guard<mutex> lock(mtx);
// 2. std::condition_variable
// 3. 공유 자원(실제로 사용하기도 하고 cv 내에서 조건을 확인하는데 사용하기도 함)
//
// code)---------------------------------------------
// struct CvContext
// {
//     std::mutex mtx;
//     std::condition_variable cv;
//     bool ready = false; // 공유 자원이라 가정
// } context;
// 
// void ThreadA()
// {
//     unique_lock<mutex> lock(context.mtx);
// 
//     // Do Something...
//     context.ready = true; // lock을 걸고 공유 자원 수정
// 
//     // 대기 중인 스레드가 있으면 깨우고 조건을 다시 확인하게 한다.
//     context.cv.notify_one();
// }
// 
// void ThreadB()
// {
//     unique_lock<mutex> lock(context.mtx);
// 
//     // lock을 소유한 쪽이 있으면 소유권을 반납할 때까지 대기한다.
//     // 조건 체크 후 조건 변수가 대기 상태에 들어갔다면 notify 계열의 함수를 호출할 때까지 대기한다.
//     context.cv.wait(lock, [&]{ return context.ready; });
// }
// --------------------------------------------------
//

BEGIN_NS(Case01)

mutex mtx;
condition_variable cv;

// 조건 상태를 체크하고 대기 상태에 들어가는지, 대기 상태에 들어간 것을 깨울 때 조건을 체크하는지 확인하기 위한 케이스
void Run()
{
    bool chk = true;

    unique_lock<mutex> lock(mtx);

    // 조건은 만족하지만 notify_one()을 호출하지 않은 상태라면?
    cv.wait(lock, [&]() { 
        cout << "Call the Predicate to check whether the waiting can be completed.\n";

        if (true == lock.owns_lock())
        {
            cout << "lock(mtx) is locked by the calling thread[" << this_thread::get_id() << "] (1).\n";
        }

        return true == chk;
    });

    // 조건 변수에 전달한 mutex의 lock이 unlock을 진행하는지 확인
    if (true == lock.owns_lock())
    {
        cout << "lock(mtx) is locked by the calling thread[" << this_thread::get_id() << "] (2).\n";
    }

    // notify_one()을 호출하지 않았음에도 조건을 만족하기 때문에 통과한다.
    // 
    // 결론 : 조건을 만족한다면 조건 변수는 대기 상태에 빠지지 않음.
    cout << "Main Done.\n";

    // !! 주의 !!
    // lock 변수 자체는 condition_variable이 대기 상태로 진입할 때 영향을 미치지 않으니 혼동하면 안 된다.
    // wait()에 의해 대기 상태로 진입하기 전에 lock 변수에 걸린 mutex의 lock을 해제한다.
    // 위 예시에선 조건 변수가 대기 상태로 진입하지 않으니 계속 lock 상태를 유지한다.
    //
    // condition_variable에서 lock 변수가 필요한 이유는 다음과 같다.
    // - 조건 변수 자체는 뮤텍스와 같은 lock 자원과 결합하여 데이터를 동기화하기 위한 목적이 큼.
    // - 멀티스레딩 환경에서 동일 condition_variable에 대한 경합이 발생할 경우 lock이 필요함(다만 넘겨지는 mutex 자원은 반드시 잠긴 상태여야 함).
    // - 조건 변수의 wait()에서 사용되는 조건 인자(?) Predicate에서 사용되는 공유 변수에 대한 안전한 접근을 위해선 lock 변수가 필요함.
}

END_NS

BEGIN_NS(Case02)

mutex mtx;
condition_variable cv;

// 1. 조건을 만족하지 않아서 대기 상태에 들어갔는데 이후 notify 계열의 함수를 호출한다면 깨어나는가?
// 2. 조건 변수의 대기 상태는 mutex의 lock 기능에 영향을 주는가?
void Run()
{
    bool chk = false;

    // 2초 뒤에 notify_one() 호출(조건 대상 변수 수정 안 함)
    thread th1{ []() {
        cout << "Thread 01 waits for 2 seconds...\n";

        this_thread::sleep_for(2s); // chrono::seconds(2));

        // 조건 변수가 mutex의 lock을 잡고 있는지 확인
        unique_lock<mutex> innerLock(mtx);

        // 조건 대상 변수는 여전히 false
        cv.notify_one();

        cout << "Thread 01 Done\n";
    } };
    
    // 4초 뒤에 notify_one() 호출(조건 대상 변수 수정 함)
    thread th2{ [&]() {
        cout << "Thread 02 waits for 4 seconds...\n";

        this_thread::sleep_for(4s); // chrono::seconds(4));

        // 조건 변수가 mutex의 lock을 잡고 있는지 확인
        unique_lock<mutex> innerLock(mtx);

        chk = true;

        // 조건 대상 변수를 true로 바꾸고 호출
        cv.notify_one();
        
        cout << "Thread 02 Done\n";
    } };

    unique_lock<mutex> lock(mtx);

    // 1) Thread 01 Done -> Thread 02 Done -> Main Done(메인 스레드가 조금 늦게 깨어난 경우)
    // 2) Thread 01 Done -> Main Done -> Thread Done(메인 스레드가 좀 빨리 깨어난 경우)
    // 
    // 결론 : notify 계열의 함수를 호출하면 다음 과정을 거침.
    // - mutex에 lock을 걸고 대기 상태의 조건 변수를 깨움.
    // - 조건 변수의 조건 인자 Predicate의 반환 값을 확인함.
    // - Predicate의 반환 값이 true라면 wait()를 빠져나옴(mutex의 lock은 계속 걸린 상태).
    // - Predicate의 반환 값이 false라면 mutex에 unlock을 걸고 조건 변수는 다시 대기 상태에 들어감.
    // 
    // 결론 : 조건 변수의 대기 상태와 mutex의 lock으로 인한 대기 상태는 다른 것임(중요).
    cv.wait(lock, [&]() {
        cout << "Call the Predicate to check whether the waiting can be completed.\n";

        if (true == lock.owns_lock())
        {
            cout << "lock(mtx) is locked by the calling thread[" << this_thread::get_id() << "] (1).\n";
        }

        return true == chk;
    });

    // 조건 변수에 전달한 mutex의 lock이 unlock을 진행하는지 확인
    if (true == lock.owns_lock())
    {
        cout << "lock(mtx) is locked by the calling thread[" << this_thread::get_id() << "] (2).\n";
    }

    cout << "Main Done\n";

    // !! 중요 !!
    // 조건 변수에 넣는 lock 변수는 조건 변수 자체에 대한 경합에 대한 순차적 접근 및 Predicate에서 사용하는 공유 자원에 대한 안전한 접근을 위한 목적으로 사용한다.
    // 조건 변수에 넣는 lock으로 인한 대기와 조건 변수 자체의 대기 상태로의 진입은 다른 것이니 혼동하면 안 된다.
    // 
    // condition_variable의 wait()를 호출하는 쪽이 메인 스레드이기 때문에 깨어나서 스레드 ID를 조회하면 메인 스레드의 아이디로 조회된다.
    // notify 계열의 함수를 호출한 스레드의 ID가 출력되는 것이 아니니까 주의해야 한다.

    th1.join();
    th2.join();
}

END_NS

BEGIN_NS(Case03)

mutex mtx;
condition_variable cv;

// notity 계열의 함수를 먼저 호출한 다음 wait()를 걸면 대기 상태에 진입하지 않고 진행하는가?
void Run()
{
    // notify 계열의 함수를 미리 한 번 호출
    cv.notify_one();

    // 2초 뒤에 다시 notify 계열의 함수를 호출
    thread th{ []() {
        cout << "Wait for 2 seconds...\n";

        this_thread::sleep_for(2s); // chrono::seconds(2));

        cv.notify_one();
    } };

    unique_lock<mutex> lock(mtx);

    // 먼저 호출한 notify_one()에 반응하는지 아니면 뒤늦게 호출한 notify_one()에 반응하는지 확인
    cv.wait(lock);

    // 사전에 notify_one()을 호출했다고 해도 wait()를 호출하면 대기 상태에 진입한다(Windows의 Event 방식과는 다른 부분).
    // 
    // 결론 : notify_one()을 먼저 호출한다고 해도 뒤늦게 wait()를 호출한다면 대기 상태에 진입함.
    cout << "Main Done\n";

    th.join();
}

END_NS

BEGIN_NS(Case04)

mutex mtx;
condition_variable cv;

// 조건 없이 wait()를 호출했다면 lock에 대한 소유권의 행방은?
// 소유권을 획득했다면 해당 lock 자원을 소유한 스레드의 ID를 조회하면?
void Run()
{
    // 2초 뒤에 다시 lock에 대한 소유권 획득 시도
    thread th{ []() {
        cout << "Wait for 2 seconds...\n";

        this_thread::sleep_for(2s); // chrono::seconds(2));

        unique_lock<mutex> lock(mtx);

        // lock에 대한 소유권을 wait()가 계속 물리고 있으면 출력되지 않을 것이다.
        cout << "Locking is allowed\n";

        // lock을 건 스레드의 ID를 확인하기 위한 용도
        if (true == lock.owns_lock())
        {
            cout << "lock(mtx) is locked by the calling thread[" << this_thread::get_id() << "] (1).\n";
        }

        // wait()에서 lock을 물리고 있지 않다면 메인 스레드를 깨운다.
        cv.notify_one();
    } };

    unique_lock<mutex> lock(mtx);

    cv.wait(lock);

    // lock을 건 스레드의 ID를 확인하기 위한 용도
    if (true == lock.owns_lock())
    {
        cout << "lock(mtx) is locked by the calling thread[" << this_thread::get_id() << "] (2).\n";
    }

    // Locking is allowed -> Main Done
    // 
    // 결론 : 조건 없는 wait() 또한 mutex의 lock을 물리지 않고(unlock을 진행하고) 대기 상태에 돌입함.
    // 결론 : 당연하겠지만 lock(mtx)를 획득한 스레드의 ID를 조회하면 생성된 스레드와 메인 스레드의 ID가 각각 조회됨.
    cout << "Main Done\n";

    th.join();
}

END_NS

BEGIN_NS(Case05)

mutex mtx;
condition_variable cv;

// 다음 케이스는 단순히 lock의 소유권을 획득하기 위한 대기 과정이다.
// mutex의 lock을 소유하기 위한 대기 상태이기 때문에 condition_variable과의 연관성은 없다고 봐야 하지만 잠깐 혼동한 경우가 있어서 케이스로 적어 둔다.
void Run()
{
    // lock을 우선적으로 획득하기 위한 스레드 생성
    thread th{ []() {
        unique_lock<mutex> lock(mtx);

        cout << "Wait for 5 seconds holding on the lock(mtx)...\n";

        // 5초 동안 lock 소유
        this_thread::sleep_for(5s); // chrono::seconds(5));
    } };

    // 상단의 스레드가 lock을 소유할 때까지 대기
    this_thread::sleep_for(1s); // chrono::seconds(1));

    cout << "Before unique_lock<mutex> lock(mtx)\n";
    
    unique_lock<mutex> lock(mtx);
    
    cout << "After unique_lock<mutex> lock(mtx)\n";

    // 상단의 스레드가 mutex의 lock을 풀 때까지 대기
    cv.wait(lock);

    // 이건 mutex의 lock을 소유하는 과정에서 생긴 대기 상태이기 때문에 조건 변수와의 연관성이 있는 것은 아니다.
    cout << "Main Done\n";

    th.join();
}

END_NS

BEGIN_NS(Case06)

mutex mtx;
condition_variable cv;

// 조건을 만족하지 않아 대기 상태에 진입했으면 lock에 대한 소유권의 행방은?
void Run()
{
    // 조건 변수에서 사용할 변수
    bool chk = false;

    thread th{ [&]() {
        // 메인 로직이 lock을 소유할 때까지 대기
        this_thread::sleep_for(1s); // chrono::seconds(1));

        // 메인 로직의 cv.wait()에서 Predicate가 false를 반환하면 mutex를 unlock하는가?
        // 그렇다면 아래 로직이 실행된다.
        {
            unique_lock<mutex> lock(mtx);

            cout << "Thread OK 1\n";
        }

        // cv.wait()의 Predicate에서 true를 반환할 수 있게 chk를 true로 변경하고 notify_one() 호출
        chk = true;
        cv.notify_one();

        // 이 로직이 없으면 Thread OK 2가 먼저 반응하기에 쓴 것
        this_thread::sleep_for(1s); // chrono::seconds(1));

        {
            unique_lock<mutex> lock(mtx);

            cout << "Thread OK 2\n";
        }
    } };

    // Main
    {
        unique_lock<mutex> lock(mtx);

        // 3초 동안 lock 소유
        this_thread::sleep_for(3s); // chrono::seconds(3));

        // 조건이 일치하지 않게 한다.
        cv.wait(lock, [&]() {
            cout << "Call the Predicate to check whether the waiting can be completed.\n";

            return true == chk;
        });

        // 3초 동안 lock 소유
        this_thread::sleep_for(3s); // chrono::seconds(3));
    }

    // Thread OK 1 -> Main Done -> Thread OK 2(일반적인 경우라면 이 순서대로 출력될 것임)
    // Thread OK 1 -> Thread OK 2 -> Main Done(생성한 스레드 쪽에서 반환한 lock을 빠르게 잡고 먼저 출력까지 한다면 희박한 확률로 이 순서대로 출력될 것임)
    // 
    // 결론 : 조건 변수는 Predicate가 false를 반환하면 lock의 소유권을 반납하고 다시 notify 계열의 함수를 호출했을 때 lock의 소유권을 획득함.
    cout << "Main Done\n";

    th.join();
}

END_NS

BEGIN_NS(Case07)

mutex mtx;
condition_variable cv;

// notify_all()로 대기하고 있는 스레드들을 깨운다고 할 때
// 한꺼번에 깨어나는지 아니면 순차적으로 lock의 소유권을 획득하면서 깨어나는지 확인하기 위한 테스트 케이스
void Run()
{
    vector<thread> thVec;

    for (int i = 0; i < 5; i++)
    {
        thVec.push_back(thread{ [&, i] {
            unique_lock<mutex> lock(mtx);

            cv.wait(lock);

            cout << "Wake Up! Thread " << i << '\n';

            // 순차적으로 깨어나는지 확인하기 위해 깨어난 쪽에서 2초 대기
            this_thread::sleep_for(2s); // chrono::seconds(2));
        } });
    }

    // 스레드가 순차적으로 대기 상태에 들어갈 때까지 잠깐 대기
    this_thread::sleep_for(1s);

    // 모든 스레드를 깨운다.
    cv.notify_all();

    for (thread& th : thVec)
    {
        th.join();
    }

    // 순차적으로 깨어나되 다음 스레드는 2초 간격으로 깨어난다.
    // 신호 상태에 따라 대기 중인 스레드들을 한 번에 깨우는 기능이 있는 Windows의 Event와 같이 동작할 줄 알았는데 아니었음.
    // 
    // 결론 : 조건 변수에 의해 대기하고 있는 스레드는 한 번에 깨어나는 것이 아닌 lock을 소유할 때까지 기다리면서 순차적으로 깨어남.
    cout << "Main Done\n";
}

END_NS

BEGIN_NS(Case08)

mutex mtx;
condition_variable cv;

// 해당 케이스는 조건 변수에 lock 변수를 unlock 상태로 전달하는 잘못된 방식을 다루고 있다.
void Run()
{
    // 2초 뒤에 notify_one() 호출(조건 대상 변수 수정 안 함)
    thread th{ []() {
        cout << "Wait for 2 seconds...\n";

        this_thread::sleep_for(2s); // chrono::seconds(2));

        // notify_one()을 호출해서 메인 스레드는 wait()를 반환하지 않고 무한히 대기한다.
        cv.notify_one();
    } };

    unique_lock<mutex> lock(mtx);

    // 이렇게 쓰면 안 된다.
    lock.unlock();

    // 조건 변수에 전달하는 lock 변수는 소유권을 반환한 상태면 안 된다.
    cv.wait(lock);

    // notify_one()을 호출해서 wait()를 빠져나오지 않고 무한히 대기하는 문제가 발생한다.
    // !! 이건 정의되지 않은 동작이기 때문에 컴파일러마다 차이가 있을 것으로 예상됨. !!
    //
    // 결론 : 조건 변수에 전달되는 lock 변수는 반드시 소유권을 획득한 상태(잠긴 상태)로 전달되어야 함.
    cout << "Main Done\n";

    th.join();
}

END_NS

BEGIN_NS(Case09)

mutex mtxThread;
mutex mtxMain;
condition_variable cv;

vector<int> sharedVec;

// 조건 변수, mutex(lock 자원), 공유 자원은 하나의 세트로 동작해야 한다.
// 다음과 같이 "조건 변수 + 조건 변수용 mutex" 따로 "공유 자원 + 공유 자원용 mutex" 따로 둬서 사용하는 건 잘못된 사용 방법이다.
void Run()
{
    thread th1{ []() {
        cout << "Thread 01 waits for 2 seconds...\n";

        this_thread::sleep_for(2s); // chrono::seconds(2));

        while (true)
        {
            this_thread::sleep_for(1s); // chrono::seconds(1));

            unique_lock<mutex> lock(mtxThread);

            sharedVec.push_back(rand() % 100);

            cv.notify_one();
        }
    } };

    thread th2{ []() {
        cout << "Thread 02 waits for 2 seconds...\n";

        this_thread::sleep_for(2s); // chrono::seconds(2));

        while (true)
        {
            this_thread::sleep_for(1s); // chrono::seconds(1));

            unique_lock<mutex> lock(mtxThread);

            sharedVec.push_back(rand() % 100);

            cv.notify_one();
        }
    } };

    while (true)
    {
        unique_lock<mutex> lock(mtxMain);

        // 생성한 스레드에서 공유 자원에 접근할 때 사용하는 mutex와 조건 변수에서 사용하는 mutex가 다른 상태이다.
        // 이건 잘못된 설계이지만 코드 복잡도가 높아지면 충분히 발생할 수 있으니 주의하자.
        cv.wait(lock, [&]() { return sharedVec.size() > 0; });

        // sharedVec은 스레드 안전하지 않은 변수이기에 경합으로 인한 덮어쓰기나 잘못된 할당이 발생할 수 있다.
        // 해당 예시에선 3개의 스레드가 하나의 sharedVec을 대상으로 경합하는 만큼 문제가 쉽게 발생할 것이다.
        sharedVec = vector<int>{ }; // 매번 새로운 vector로 갱신해서 할당 에러가 발생하게 유도함.

        // 할당 작업에 대한 잘못된 경합이 먼저 수행되게 가장 마지막에 출력한다(출력은 굉장히 느린 작업이라 할당보다 선행되면 문제가 잘 발생하지 않음).
        cout << "Main Thread Logic :)\n";
    }

    th1.join();
    th2.join();
}

END_NS

int main()
{
    // Case01::Run();
    // Case02::Run();
    // Case03::Run();
    // Case04::Run();
    // Case05::Run();
    // Case06::Run();
    Case07::Run();
    // Case08::Run();
    // Case09::Run();

    return 0;
}
