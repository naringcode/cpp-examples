#include <iostream>

#include <thread>
#include <mutex>

// https://en.cppreference.com/w/cpp/thread/condition_variable

// condition_variable을 사용하면서 들었던 의문점을 정리한 파일.
//
// !!!중요!!!
// 조건 변수에 notify를 거는 것과 공유 자원을 대상으로 lock을 잡고 푸는 것은 원자적이지 않음.
// Spurious Wakeup을 주의할 것! // https://en.wikipedia.org/wiki/Spurious_wakeup
// 
// 1. 대기 상태
// 2. wakeup
// <<-- 다른 쪽에서 먼저 lock을 잡고 사용할 수도 있음
// 3. lock 걸기
// 4. 조건 체크

// 결론 정리
// ** 조건이 있는 wait()를 사용했을 경우
// 1. Lock의 소유권을 획득하는 것이 먼저
// 2-1. 조건이 true라면 다음 로직을 실행(notify 계열이 없어도 됨)
// 2-1-1. 조건이 false라면 대기 상태로 진입(Lock에 대한 소유권 반환)
// 2-1-2. notify로 인해 깨어났다면 1번 로직부터 다시 실행
// 
// ** 조건이 없는 wait()를 사용했을 경우
// 1. 대기 상태로 진입
// 2. notify가 발생할 때까지 대기(Lock에 대한 소유권은 없음)
// 3. notify가 발생한다면 Lock에 대한 소유권 획득을 시도
// 4. 다음 로직을 실행
// 
// ** notify를 호출한 다음 wait()를 호출하면?
// ** wait()가 반응할 것 같지만 대기 상태에 들어감(Windows Event와는 다른 부분).
// ** 옵저버 패턴처럼 동작한다고 숙지하자.
// 

// 조건 변수는 다음 3가지가 함께 묶여서 동작한다고 생각하자.
// 1. std::unique_lock<std::mutex> lock(mtx); // std::mutex mtx
// 2. std::condition_variable
// 3. cv 내 조건 확인용 변수
//

std::mutex mtx;
std::condition_variable cv;

void RunCase01()
{
    // Test Case
    // 대기 상태가 먼저냐 조건 체크가 먼저냐?
    // notify_one()을 호출하지 않은 상황이라면?
    //
    bool chk = true;

    std::unique_lock<std::mutex> lock(mtx);

    cv.wait(lock, [&]() { return true == chk; });

    // notify_one()을 호출하지 않았음에도 통과함(조건을 먼저 체크하고 아니면 대기 상태에 빠짐).
    // 결론 : 조건 체크가 먼저임.
    std::cout << "OK\n";
}

void RunCase02()
{
    // Test Case
    // notify_one()을 한 다음에 wait()를 걸면 반응하는가?
    //
    cv.notify_one();

    // 2초 뒤에 다시 notify_one() 호출
    std::thread([]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));

        cv.notify_one();
    }).detach();

    std::unique_lock<std::mutex> lock(mtx);

    cv.wait(lock);

    // 먼저 notify_one()을 걸어도 반응하지 않는다(Windows의 Event 방식과는 다른 부분).
    // 결론 : notify_one()에 진입하기 이전에 wait()가 선행되어야 함.
    std::cout << "OK\n";
}

void RunCase03()
{
    // Test Case
    // 조건 없는 wait()를 사용했으면 lock에 대한 소유권의 행방은?
    //
    // 2초 뒤에 다시 lock에 대한 소유권 획득 시도
    std::thread([]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));

        std::unique_lock<std::mutex> lock(mtx);

        // 소유권을 wait()가 계속 물리고 있으면 출력 안 될 것임.
        std::cout << "Thread OK\n";

        cv.notify_one(); // 깨우기
    }).detach();

    std::unique_lock<std::mutex> lock(mtx);

    cv.wait(lock);

    // Thread OK -> Main OK
    // 결론 : 조건 없는 wait()를 사용했으면 lock을 물리지 않고 대기함.
    std::cout << "Main OK\n";
}

void RunCase04()
{
    // Test Case
    // Lock을 획득하지 못 하면 대기 상태에 진입하는가?
    //
    std::thread([]() {
        std::unique_lock<std::mutex> lock(mtx);

        // 5초 동안 lock 소유
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }).detach();

    // 상단의 스레드가 lock을 소유할 때까지 대기
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    bool chk = true;

    std::unique_lock<std::mutex> lock(mtx);

    cv.wait(lock, [&]() { return true == chk; });

    // 상단의 스레드가 종료되면 OK를 출력한다.
    // 결론 : lock을 소유할 때까지 대기 상태에 진입하지 않는다.
    std::cout << "OK\n";
}

void RunCase05()
{
    bool chk = false;

    // Test Case
    // 조건을 만족하지 않아 대기 상태에 진입했으면 lock에 대한 소유권의 행방은?
    //
    std::thread([&]() {
        // 메인 로직이 lock을 소유할 때까지 대기
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        //
        {
            std::unique_lock<std::mutex> lock(mtx);

            // 이게 먼저 출력된다.
            std::cout << "Thread OK 1\n";
        }

        // cv.wait()가 반응할 수 있게
        chk = true;
        cv.notify_one();

        // 이걸 안 쓰면 Thread OK 2가 먼저 반응함
        std::this_thread::sleep_for(std::chrono::seconds(1));

        //
        {
            std::unique_lock<std::mutex> lock(mtx);

            std::cout << "Thread OK 2\n";
        }

    }).detach();

    //
    {
        std::unique_lock<std::mutex> lock(mtx);

        // 5초 동안 lock 소유
        std::this_thread::sleep_for(std::chrono::seconds(5));

        // 조건이 일치하지 않게 한다.
        cv.wait(lock, [&]() { return true == chk; });

        // 5초 동안 lock 소유
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    // Thread OK1 -> Main OK -> Thread OK2
    // 결론 : 조건이 일치하지 않으면 lock의 소유권을 반납한다.
    std::cout << "Main OK\n";

    // 스레드가 종료될 때까지 대기
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

int main()
{
    // RunCase01();
    // RunCase02();
    RunCase03();
    // RunCase04();
    // RunCase05();

    return 0;
}
