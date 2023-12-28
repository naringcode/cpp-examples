#include <bits/stdc++.h>

using namespace std;

// 신장 트리(Spanning Tree)
// : 사이클 없이 그래프의 모든 정점을 연결하는 트리

// 최소 신장 트리(Minimum Spanning Tree)
// : 가중치의 합이 최소가 되는 신장 트리

// 최소 신장 트리를 구현하는 알고리즘
// - 프림 알고리즘(Prim's Algorithm)
// : 사이클 판별 시 visited 배열 사용
// - 크루스칼 알고리즘(Kruskal's Algorithm)
// : 사이클 판별 시 Union-Find 사용

// 프림 알고리즘
// 1. 그래프와 최소 신장 트리를 저장할 노드 준비(노드는 비어있는 상태)
// 2. 임의의 정점을 시작 노드로 설정하여 이걸 루트 노드로 삽입
// 3. 최소 신장 트리에 삽입되어 있는 모든 인접 정접들 사이의 간선이 가지는 가중치 조사
// 4. 간선들 중 가중치가 가장 작은 것을 골라서 해당 간선에 연결되어 있는 인접 정점을 최소 신장 트리에 삽입
// 5. 단, 인접 정점을 추가했을 때 최소 신장 트리에 사이클이 형성되어선 안 됨.
// 6. 3번 ~ 5번 과정을 반복해서 모든 정점이 연결될 때까지 수행

// 크루스칼 알고리즘
// 1. 그래프가 가진 모든 간선의 가중치를 오름차순으로 정렬
// 2. 간선 목록을 순회하면서 최소 신장 트리에 추가
// 3. 단, 간선을 추가했을 때 사이클이 형성되어선 안 됨

// 문제) 백준 1197번 : 최소 스패닝 트리

struct Edge
{
    int v1;
    int v2;
    int weight;
};

struct Comp
{
    bool operator()(const Edge& lhs, const Edge& rhs)
    {
        return lhs.weight > rhs.weight;
    }
};

int v;
int e;

int a;
int b;
int c;

priority_queue<Edge, vector<Edge>, Comp> edges;
int ufSet[10004];

int res;

int doFind(int here)
{
    // 루트 노드(분리집합의 대표 원소)
    if (here == ufSet[here])
        return here;

    // 최적화(경로 압축)
    ufSet[here] = doFind(ufSet[here]);

    return ufSet[here];
}

// a 집합을 b 집합에 연결
bool doUnion(int a, int b)
{
    a = doFind(a);
    b = doFind(b);

    // 같은 집합에 속해 있음
    if (a == b)
        return false;

    // 합집합(b 집합의 대표 원소를 a 집합의 대표 원소에 연결)
    ufSet[b] = a;

    return true;
}

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    cin >> v >> e;

    for (int i = 0; i < 10004; i++)
    {
        // 정점 별로 분리 집합 생성
        ufSet[i] = i;
    }

    for (int i = 0; i < e; i++)
    {
        cin >> a >> b >> c;

        edges.push({ a, b, c });
    }

    while (false == edges.empty())
    {
        Edge edge = edges.top();
        edges.pop();

        // cout << edge.weight << ' ';

        if (true == doUnion(edge.v1, edge.v2))
        {
            res += edge.weight;
        }
    }

    cout << res;

    return 0;
}
