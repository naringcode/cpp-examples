#include <bits/stdc++.h>

// 다익스트라는 하나의 정점으로부터 다른 정점으로의 가장 짧은 거리를 구하기 위해서 사용한다.
//
// 비용이 가장 저렴한 노드를 찾아서 해당 노드의 인접 노드를 갱신하는 방식으로 동작한다.
// 여기서 비용이 가장 저렴한 노드를 찾을 때 최소힙 기반의 우선순위 큐를 활용하면 좋다.
//
// 다익스트라에 휴리스틱 값을 주면 A* 알고리즘이 된다.
// 다시 말해 다익스트라는 휴리스틱 값이 0인 A* 알고리즘으로 봐도 된다.

// 1. 다익스트라의 미확인 노드는 무한의 값을 가지게 초기화한다.
// 2. 시작 노드가 가진 값은 0으로 초기화한다.
// 3. 다음 내용 반복
//    a) 비용이 가장 저렴한 노드를 찾는다(찾을 수 없으면 알고리즘 종료).
//    b) 해당 노드의 인접 노드가 가진 가중치에 대한 갱신을 시도한다.
//    c) 갱신했을 때 더욱 저렴해진다면 이를 반영한다.

// 문제) 백준 1753 : 최단경로

using namespace std;

struct Edge
{
    int to;
    int weight;
};

constexpr int INF = 999'999'999;

int v;
int e;

int k;

int a;
int b;
int c;

int cost[20'004];
vector<Edge> adj[20'004];

bool visited[20'004];

// 비용, 노드
priority_queue<pair<int, int>> pq;

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    cin >> v >> e;

    for (int i = 1; i <= v; i++)
    {
        cost[i] = INF;
    }

    cin >> k;

    cost[k] = 0;

    // 비용, 노드
    pq.push({ 0, k });

    for (int i = 0; i < e; i++)
    {
        cin >> a >> b >> c;

        adj[a].push_back({ b, -c });
    }

    while (pq.size() > 0)
    {
        auto pair = pq.top();
        pq.pop();

        if (true == visited[pair.second])
            continue;

        visited[pair.second] = true;

        for (Edge there : adj[pair.second])
        {
            if (true == visited[there.to])
                continue;

            there.weight = -(there.weight);

            cost[there.to] = min(cost[there.to], cost[pair.second] + there.weight);

            pq.push({ -cost[there.to], there.to });
        }
    }

    for (int i = 1; i <= v; i++)
    {
        if (INF == cost[i])
        {
            cout << "INF\n";
        }
        else
        {
            cout << cost[i] << '\n';
        }
    }

    return 0;
}
