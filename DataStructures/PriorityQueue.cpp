#include <iostream>
#include <sstream>
#include <vector>

// C++에서 제공하는 priority_queue를 사용하면 내부 요소를 순회해서 조회할 수 없음.
// 때로는 특정 알고리즘을 구현하기 위한 우선순위 큐를 만드는 것이 좋음.
// 예를 들어, 중간 노드의 값이 바뀌었을 때는 UpdateValueAtIndex()로 위치를 갱신할 필요가 있음.

// 지금은 최소 힙만 구현했기 때문에 _heap만 사용하고 있다.
// 하지만 특정 대상을 조회하고자 할 때에 매번 전체 노드를 순회해야 하니
// 조건에 따라 _heap + set을 섞는 방법도 고려해볼만 하다.

class PriorityQueue
{
public:
    void Enqueue(int data)
    {
        _heap.push_back(data);

        this->bubbleUp(_heap.size() - 1);
    }

    bool Dequeue(int* outData)
    {
        if (_heap.empty())
            return false;

        *outData = _heap[0];

        _heap[0] = _heap.back();
        _heap.pop_back();

        this->sinkDown(0);

        return true;
    }

    void UpdateValueAtIndex(int idx, int newValue)
    {
        if (idx >= _heap.size())
            return;

        int diff = newValue - _heap[idx];
        _heap[idx] = newValue;

        if (diff > 0)
        {
            // 중간 노드의 값이 증가한 경우 -> 내려가야 함
            this->sinkDown(idx);

        }
        else if (diff < 0)
        {
            // 중간 노드의 값이 감소한 경우 -> 올라가야 함
            this->bubbleUp(idx);
        }
    }

    void Print()
    {
        std::stringstream ss;

        for (int elem : _heap)
        {
            ss << elem << ' ';
        }

        ss << '\n';

        std::cout << ss.str();
    }

public:
    int Count() { return (int)_heap.size(); }

private:
    // 힙 관련 함수
    void bubbleUp(int idx)
    {
        int child = idx;

        while (child > 0)
        {
            int parent = (child - 1) / 2;

            // 부모의 값이 작으면 종료(자식의 값이 더 큰 상황)
            if (_heap[parent] <= _heap[child])
                break;

            std::swap(_heap[parent], _heap[child]);

            child = parent;
        }
    }

    void sinkDown(int idx)
    {
        int lastIdx = _heap.size() - 1;
        int parent  = idx;

        while (true)
        {
            int leftChild  = (parent * 2) + 1;
            int rightChild = leftChild + 1;

            int smallChild = parent;

            // 왼쪽으로 내려갈 수 있는 상황
            if (leftChild <= lastIdx && _heap[leftChild] < _heap[smallChild])
            {
                smallChild = leftChild;
            }

            // 오른쪽으로 내려갈 수 있는 상황
            if (rightChild <= lastIdx && _heap[rightChild] < _heap[smallChild])
            {
                smallChild = rightChild;
            }

            // 현재 노드의 값보다 자식 노드의 값이 큰 상황
            if (parent == smallChild)
                break;

            std::swap(_heap[parent], _heap[smallChild]);

            parent = leftChild;
        }
    }

private:
    std::vector<int> _heap; // 최소 힙
};

int main()
{
    PriorityQueue pq;

    while (true)
    {
        int select;
        std::cout << "1. Enqueue / 2. Dequeue / 3. UpdateValueAtIndex / 4. Print / 5. Exit : ";
        std::cin >> select;

        if (1 == select)
        {
            int val;
            std::cout << "Value : ";
            std::cin >> val;

            pq.Enqueue(val);
        }
        else if (2 == select)
        {
            int val;

            if (true == pq.Dequeue(&val))
            {
                std::cout << val << '\n';
            }
            else
            {
                std::cout << "Empty.\n";
            }
        }
        else if (3 == select)
        {
            int idx;
            int newValue;

            std::cout << "Index And New Value : ";
            std::cin >> idx >> newValue;

            pq.UpdateValueAtIndex(idx, newValue);
        }
        else if (4 == select)
        {
            std::cout << "Print : ";
            pq.Print();
        }
        else if (5 == select)
        {
            break;
        }
    }

    return 0;
}
