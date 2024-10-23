#include <iostream>
#include <queue>

using namespace std;

#define COMP_MODE 1
#define USE_LESS_COMP 1

// !! PriorityQueue를 사용하면 Move 계열 연산이 일어남. !!
// !! 경험상 컴파일러에 따라 Copy 연산이 일어나는 것을 본 적이 있으니 이 부분은 설계할 때 약간 주의할 필요가 있음. !!
//
// 예약 시스템과 같은 기능을 제작할 때 예약 대상의 정보를 구조체나 클래스의 멤버 형태로 들고 있게 하면 Pop과 Push 시 Move로 인해 코스트가 낭비될 수 있다.
// 따라서 예약 대상이 되는 데이터는 따로 묶어서 포인터로 전달하는 구조를 쓰도록 한다.
// 
// ex)
// struct ReservedData
// {
//     // TODO
// };
// 
// struct PQItem
// {
//     ...
// 
//     // Bad
//     // ReservedData data;
// 
//     // Bad(컴파일러에 따라 복사 연산이 일어날 수도 있음 -> 스마트 포인터의 레퍼런스 카운팅은 메모리 버스를 대상으로 lock을 건다는 사실을 잊으면 안 됨)
//     shared_ptr<ReservedData> data;
// 
//     // Good
//     ReservedData* data = nullptr;
// };
//
// ReservedData에 모든 걸 넣어서 관리해도 된다.
// 하지만 PQ를 쓸 때 Copy와 Move 연산으로 인한 코스트를 최소화할 방법을 모색해야 한다는 것을 기억하자.
//
// 데이터가 적다면 동적할당으로 인한 코스트가 더 들 수 있으나 규모가 큰 시스템을 만들 것을 상정해야 한다.
// 무엇보다 규모가 큰 프로그램이라면 자체적인 할당자를 제작하여 사용할 확률이 높기에 이 부분은 큰 문제가 되지 않는다.

struct Item
{
    Item(int elem)
        : elem{ elem }
    { }

    Item(const Item& rhs)
        : elem{ rhs.elem }
    {
        cout << "Item Copy Constructor - elem : " << elem << '\n';
    }

    Item(Item&& rhs) noexcept 
        : elem{ rhs.elem }
    {
        cout << "Item Move Constructor - elem : " << elem << '\n';
    }

    Item& operator=(const Item& rhs)
    {
        elem = rhs.elem;

        cout << "Item Copy Operator - elem : " << elem << '\n';

        return *this;
    }

    Item& operator=(Item&& rhs) noexcept
    {
        elem = rhs.elem;

        cout << "Item Move Operator - elem : " << elem << '\n';

        return *this;
    }

#if COMP_MODE == 1
    // PQ Comp(less)
    bool operator<(const Item& rhs) const
    {
        // PQ의 우선순위는 다른 컨테이너와 다르게 조금 다르게 생각해야 함.
        return this->elem > rhs.elem;
    }
    
    // PQ Comp(greater)
    bool operator>(const Item& rhs) const
    {
        return this->elem > rhs.elem;
    }
#endif
    
    int elem = 0;
};

#if COMP_MODE != 1
struct Comp
{
    bool operator()(const Item& lhs, const Item& rhs)
    {
        return lhs.elem > rhs.elem;
    }
};
#endif

int main()
{
    // TEMP
    // {
    //     Item itemA{ 10 };
    // 
    //     Item itemB = itemA;
    //     itemA = itemB;
    // 
    //     Item itemC = std::move(itemA);
    //     itemA = std::move(itemC);
    // }

#if COMP_MODE == 1
#if USE_LESS_COMP == 1
    priority_queue<Item, vector<Item>> pq;
#else
    priority_queue<Item, vector<Item>, greater<Item>> pq;
#endif
#else
    priority_queue<Item, vector<Item>, Comp> pq;
#endif

    for (int i = 0; i < 10; i++)
    {
        pq.push(Item{ rand() % 100 });

        cout << "Push -------------------------\n";
    }

    cout << "End Push -------------------------\n";

    while (pq.size() > 0)
    {
        cout << "top - elem : " << pq.top().elem << '\n';

        pq.pop();

        cout << "Pop -------------------------\n";
    }

    return 0;
}
