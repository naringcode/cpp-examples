// Update Date : 2025-01-09
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <format>
#include <vector>
#include <memory>
#include <thread>
#include <future>
#include <mutex>

using namespace std;

#define BEGIN_NS(name) namespace name {
#define END_NS };

// 순서대로 볼 것
// 
// # shared_ptr을 사용할 경우 알아야 할 기본적인 내용
// 1. shared_ptr_with_deleter.cpp
// 2. shared_ptr_with_allocator.cpp
// 3. shared_ptr_with_deleter_and_allocator.cpp
// 4. (중요) shared_ptr_details.cpp (SFINAE 내용 포함)
// 
// # weak_ptr의 유효성 검증 로직에 대한 내용
// 5. weak_ptr.cpp
// 6. weak_ptr_details.cpp
//
// # shared_ptr의 관리 객체에서 자신을 반환할 때 필요한 내용
// 7. enable_shared_from_this.cpp
// 8. enable_shared_from_this_details.cpp
// 9. enable_shared_from_this_examples.cpp <-----
//
// # shared_ptr을 멀티스레딩 환경에서 사용할 때 발생할 수 있는 문제점을 기술한 내용
// 10. allocation_aligned_byte_boundaries.cpp(사전지식)
// 11. volatile_atomic_cache_coherence_and_memory_order.cpp(사전지식)
// 12. (중요) shared_ptr_multi_threading_issues.cpp

BEGIN_NS(GetThisSmartPointerToUseItAsAnArgument)

/***********************************************************
*      Get This Smart Pointer To Use It As An Argument     *
***********************************************************/

// 스마트 포인터가 적용된 클래스 내부에서 자기 자신을 인자로 넘겨야 하는 경우

class GraphNode : public enable_shared_from_this<GraphNode>
{
public:
    GraphNode(int id) : _id{ id }
    { }

public:
    void AddNode(shared_ptr<GraphNode> node)
    {
        string str = std::format("Connect a node{{{}}} to a node{{{}}}...", node->_id, _id);

        std::cout << str.c_str() << '\n';
    }

    void AddSelfNode()
    {
        shared_ptr<GraphNode> selfNode = this->shared_from_this();

        this->AddNode(selfNode);
    }

private:
    int _id = -1;
};

void Run()
{
    shared_ptr<GraphNode> node = make_shared<GraphNode>(1);

    node->AddSelfNode();
}

END_NS

BEGIN_NS(MakeAWorkerThreadAndCaptureThisSmartPointer)

/****************************************************************
*      Make A Worker Thread And Capture This Smart Pointer      *
****************************************************************/

// 작업자 스레드를 생성하고 그 스레드의 인스턴스를 캡처 블록에 넘겨야 하는 경우

class WorkerThread : public enable_shared_from_this<WorkerThread>
{
public:
    void Run()
    {
        shared_ptr<WorkerThread> self = this->shared_from_this();

        _thread = std::thread([self]() {
            for (int i = 0; i < 10; i++)
            {
                cout << "Worker Thread Cnt[" << i << "]...\n";

                this_thread::sleep_for(0.1s);
            }

            cout << "Thread Done\n";
        });

        // std::async() 등을 통해 작업자 스레드가 아닌 비동기 작업(Async Task)을 적용해도 된다.
    }

    void Join()
    {
        _thread.join();
    }

private:
    std::thread _thread;
};

void Run()
{
    shared_ptr<WorkerThread> workerTh = make_shared<WorkerThread>();

    workerTh->Run();

    workerTh->Join();
}

END_NS

BEGIN_NS(MakeAnAsyncTaskAndCaptureThisSmartPointer)

/**************************************************************
*      Make An Async Task And Capture This Smart Pointer      *
**************************************************************/

// 작업자 스레드를 구성하는 유형과 비슷하다.

class AsyncTask : public enable_shared_from_this<AsyncTask>
{
public:
    std::future<bool> DoAsyncTask()
    {
        if (true == _isWorking)
        {
            std::promise<bool> prom;

            prom.set_value(false);

            return prom.get_future();
        }

        _isWorking = true;

        // 작업 시작
        shared_ptr<AsyncTask> self = this->shared_from_this();

        std::future<bool> fut = std::async([self] {
            for (int i = 0; i < 10; i++)
            {
                cout << "Async Task Cnt[" << i << "]...\n";

                this_thread::sleep_for(0.1s);
            }

            // future로 받는 방법도 있고 이처럼 작업이 완료되었을 때 어딘가에 넣는 방법도 있다(ex. JobQueue).
            // C++20의 Coroutine과 연계하는 방법도 충분히 고려해볼 만한 대상이다.
            self->Complete();

            return true;
        });

        return fut;
    }

    void Complete()
    {
        _isDone = true;

        cout << "Async Task Done\n";
    }

private:
    bool _isWorking = false;
    bool _isDone = false;
};

void Run()
{
    shared_ptr<AsyncTask> asyncTask = make_shared<AsyncTask>();

    std::future<bool> fut = asyncTask->DoAsyncTask();

    // 작업이 완료될 때까지 대기
    // cout << "wait until done...\n";
    // fut.wait();
    // cout << "work is done...\n";

    // 작업이 완료되지 않으면 지정한 시간 동안 대기(0초를 넣으면 바로 빠져나옴)
    while (true)
    {
        // future 내부에서 Lock을 잡는 것까진 확인했으나 0초를 넣었을 때 컨텍스트 스위칭이 발생하는지는 확실치 않음.
        // 이 부분은 컴파일러의 구현에 따라 다르겠지만 기다리지 않고 컨텍스트 스위칭 없이 즉시 반환할 것이라 생각함.
        if (fut.wait_for(0.2s) == std::future_status::timeout)
        {
            cout << "future value is not ready...\n";

            continue;
        }

        // std::future_status::ready
        cout << "future value is now ready...\n";

        break;
    }

    cout << "future<bool>.get() : " << std::boolalpha << fut.get() << '\n';
}

END_NS

BEGIN_NS(DeliverThisSmartPointerAsAnArgumentOfACallback)

/*******************************************************************
*      Deliver This Smart Pointer As An Argument Of A Callback     *
*******************************************************************/

// 콜백의 인자로 스마트 포인터를 넘길 수 있다면 스마트 포인터의 생명주기를 쉽게 관리할 수 있다.
// 콜백 함수에 스마트 포인터를 넘기면 쉽게 이벤트 기반 프로그래밍을 진행 할 수 있다.

class CallbackHandler : public enable_shared_from_this<CallbackHandler>
{
public:
    using Callback = std::function<void(shared_ptr<CallbackHandler> callback)>;
    
public:
    CallbackHandler(int id)
        : _id{ id }
    { }

public:
    void RegisterCallback(Callback callback)
    {
        _callback = callback;
    }

    void TriggerEvent()
    {
        _callback(this->shared_from_this());
    }

public:
    int GetID()
    {
        return _id;
    }

public:
    int _id = -1;

    Callback _callback;
};

void Run()
{
    shared_ptr<CallbackHandler> handler = make_shared<CallbackHandler>(1);

    handler->RegisterCallback([](shared_ptr<CallbackHandler> selfHandler) {
        cout << "Some event is triggered... Handler ID[" << selfHandler->GetID() << "]\n";
    });

    handler->TriggerEvent();
}

END_NS

BEGIN_NS(CooperateWithOneAnotherCouplingOnSmartPointers)

/*****************************************************************
*      Cooperate With One Another Coupling On Smart Pointers     *
*****************************************************************/

// 객체 간 커플링 과정에서 인자를 넘기는 용도로 사용할 때

class Player;
class Room;

class Player
{
public:
    Player(string_view name)
        : _name{ name }
    {
        cout << "Player[" << _name << "] is created...\n";
    }

    ~Player()
    {
        cout << "Player[" << _name << "] is destroyed...\n";
    }

public:
    void EnterRoom(shared_ptr<Room> room);

public:
    string GetName() { return _name; }

private:
    string _name;

    // 이 부분을 weak_ptr로 하면 순환 참조 문제를 방지할 수 있다.
    // shared_ptr로 할 경우 명시적으로 참조 관계를 끊는 코드를 어딘가에 작성해야 한다.
    shared_ptr<Room> _room;
};

class Room : public enable_shared_from_this<Room>
{
public:
    Room(string_view name)
        : _name{ name }
    {
        cout << "Room[" << _name << "] is created...\n";
    }

    ~Room()
    {
        cout << "Room[" << _name << "] is destroyed...\n";
    }

public:
    // !! Room 자체를 스레드로 만들 것인지 아니면 따로 외부에서 Update를 호출하게 할 것인지는 선택 사항임. !!
    // !! 계속 존재해야 하는 필드라면 외부에서 Update를 호출하게 하고 인스턴스 던전 같은 경우에는 스레드로 만드는 것도 방법임. !!
    void Run()
    {
        shared_ptr<Room> self = this->shared_from_this();

        _thread = std::thread([self]() {
            self->workerThreadLoop();
        });
    }

    void Join()
    {
        _thread.join();
    }

private:
    // !! 실제 프로그래밍을 할 때는 JobQueue 등을 구성해서 전달할 것 !!
    void workerThreadLoop()
    {
        // 0.5초 간격으로 업데이트를 10번 진행하고 Clear() 호출
        for (int i = 0; i < 10; i++)
        {
            this_thread::sleep_for(0.5s);

            // 방 안에 어떤 플레이어들이 있는지 출력
            std::lock_guard<mutex> lock{ _mtx };

            cout << "Start to Update the Room[" << _name << "], Cnt[" << i << "]...\n";

            for (shared_ptr<Player>& player : _players)
            {
                cout << "\tPlayer[" << player->GetName() << "] is in this Room...\n";
            }
        }

        this->Clear();
    }

public:
    void AddPlayer(shared_ptr<Player> player)
    {
        std::lock_guard<mutex> lock{ _mtx };
        
        _players.push_back(player);

        // Player를 등록하고 Room 자신을 넘기는 방식
        player->EnterRoom(this->shared_from_this());
    }

    void Clear()
    {
        std::lock_guard<mutex> lock{ _mtx };

        cout << "Clear a room...\n";

        _players.clear();
    }

public:
    string GetName() { return _name; }

private:
    string _name;

    thread _thread;

    std::mutex _mtx;
    vector<shared_ptr<Player>> _players;
};

void Player::EnterRoom(shared_ptr<Room> room)
{
    cout << "Player[" << _name << "] enters into the Room[" << room->GetName() << "]...\n";

    _room = room;
}

void Run()
{
    shared_ptr<Room> room = make_shared<Room>("GameRoom");

    room->Run();

    // 0.2초 단위로 Player를 생성해서 Room에 등록
    for (int i = 0; i < 10; i++)
    {
        this_thread::sleep_for(0.2s);

        string name = std::format("Player{}", i);

        shared_ptr<Player> player = make_shared<Player>(name);

        room->AddPlayer(player);
    }

    room->Join();
}

END_NS

BEGIN_NS(DeliverThisSmartPointerToRegisterToModuleSystems)

/**********************************************************************
*      Deliver This Smart Pointer To Register For Module Systems      *
**********************************************************************/

// 옵저버 패턴이나 플러그인 같이 어떤 특정한 시스템(?)에 등록해서 사용하고자 할 때

// 보통은 Subject 쪽에서 Observer를 등록하는 것이 정석이다.
// 만약 이를 변형하여 Observer 자원에서 Subject에 스스로를 전달하여 등록하는 방식을 취한다면?
// 이때는 Observer 내부에서 자기 자신을 Subject에 전달할 수 있어야 한다.
// !! Observer가 스스로 Subject를 등록하는 방식을 취하는 상황을 말하는 것임. !!

// Observer가 스스로를 Subject에 등록하는 방식은 Observer가 자신의 관심사를 명확하게 하는 느낌이 강하며,
// Subject가 Observer를 등록하는 방식은 Subject 차원에서 상태의 변화를 알려주고자 하는 느낌이 강하다.

// Observer
class Subscriber : public std::enable_shared_from_this<Subscriber>
{
public:
    Subscriber(string name)
        : _name{ name }
    { }

    ~Subscriber()
    { }

public:
    void OnNotify(string msg)
    {
        cout << _name << " got a message[" << msg << "]...\n";
    }

public:
    void Subscribe(shared_ptr<class Publisher> publisher);

private:
    string _name;
};

// Subject
class Publisher
{
public:
    Publisher(string name)
        : _name{ name }
    { }

    ~Publisher()
    { }

public:
    void Notify()
    {
        for (shared_ptr<Subscriber> subscriber : _subscribers)
        {
            string msg = std::format("msg from {}", _name);

            subscriber->OnNotify(msg);
        }
    }

public:
    void Attach(shared_ptr<Subscriber> subscriber)
    {
        _subscribers.push_back(subscriber);
    }

private:
    string _name;

    vector<shared_ptr<Subscriber>> _subscribers;
};

void Subscriber::Subscribe(shared_ptr<class Publisher> publisher)
{
    // 자기 자신을 넘겨 Subject에 등록한다.
    publisher->Attach(this->shared_from_this());
}

void Run()
{
    shared_ptr<Subscriber> subscriberA = make_shared<Subscriber>("Subscriber A");
    shared_ptr<Subscriber> subscriberB = make_shared<Subscriber>("Subscriber B");
    shared_ptr<Subscriber> subscriberC = make_shared<Subscriber>("Subscriber C");

    shared_ptr<Publisher> publisherA = make_shared<Publisher>("Publisher A");
    shared_ptr<Publisher> publisherB = make_shared<Publisher>("Publisher B");

    subscriberA->Subscribe(publisherA);
    subscriberA->Subscribe(publisherB);

    subscriberB->Subscribe(publisherA);
    subscriberB->Subscribe(publisherB);

    subscriberC->Subscribe(publisherA);
    subscriberC->Subscribe(publisherB);

    publisherA->Notify();
    publisherB->Notify();
}

END_NS

/*****************
*      Main      *
*****************/

int main()
{
    GetThisSmartPointerToUseItAsAnArgument::Run();
    cout << "\n------------------------------------------------------\n\n";

    MakeAWorkerThreadAndCaptureThisSmartPointer::Run();
    cout << "\n------------------------------------------------------\n\n";
    
    MakeAnAsyncTaskAndCaptureThisSmartPointer::Run();
    cout << "\n------------------------------------------------------\n\n";
    
    DeliverThisSmartPointerAsAnArgumentOfACallback::Run();
    cout << "\n------------------------------------------------------\n\n";
    
    CooperateWithOneAnotherCouplingOnSmartPointers::Run();
    cout << "\n------------------------------------------------------\n\n";
    
    DeliverThisSmartPointerToRegisterToModuleSystems::Run();
    cout << "\n------------------------------------------------------\n\n";

    return 0;
}
