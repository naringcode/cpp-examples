#include <bits/stdc++.h>

using namespace std;

// https://www.geeksforgeeks.org/bellman-ford-algorithm-dp-23/

// 벨만-포드 알고리즘
// : 하나의 정점에서 모든 정점까지의 최단거리를 구하는 알고리즘.

// 다익스트라와 차이점
// - 음의 가중치에 대한 처리가 가능하다
// - 음의 사이클을 체크할 수 있다.

// 다익스트라가 최소 비용의 노드를 확인하는 것이 핵심이었다면...
// 벨만-포드는 모든 간선을 (노드의 개수 - 1)만큼 체크하는 것이 핵심이다.
// 그래서 벨만-포드의 시간복잡도는 O(VE)
//
// 벨만-포드에 노드를 엮어서 생각하려고 하면 오히려 더 헷갈린다.
// 다익스트라는 노드에 초점, 벨만-포드는 간선에 초점을 두었다고 생각하자.

// 간선 정보만 따로 저장했을 경우
// for (int i = 0; i < V - 1; i++)
// {
//     for (int j = 0; j < E; j++) // 모든 간선 확인
//     {
//         int u = graph->edge[j].src;
//         int v = graph->edge[j].dest;
//
//         int weight = graph->edge[j].weight;
//
//         if (dist[u] != INT_MAX && dist[u] + weight < dist[v])
//             dist[v] = dist[u] + weight;
//     }
// }

// 인접리스트 형태로 간선 정보를 저장했을 경우
// 벨만-포드의 작동 원리를 모르고 보면 더 헷갈리고 또한 불필요한 오버헤드가 발생함.
// for (int i = 0; i < V - 1; i++)
// {
//     // 모든 정점에 대한
//     for (int here = 0; here < V; here++)
//     {
//         // 간선 정보 조회
//         for (auto there : adj[here])
//         {
//             int cost = there.first;
//             int to   = there.second;
//  
//             if (dist[here] != INT_MAX && dist[here] + cost < dist[there])
//                 dist[there] = dist[here] + cost;
//         }
//     }
// }

// (V - i)만큼 반복하는 이유는 이게 모든 노드를 갱신하기 위한 최소 횟수이기 때문에 그렇다.
// INF       INF        0
// (A) <---- (B) <---- (C)
//       -1        -2
//
// 시작 노드를 C라고 하고
// 간선은 -1 -> -2 순서로 순회한다고 하자.
//
// 첫 번째 순회
// INF       -2         0
// (A) <---- (B) <---- (C)
//       -1        -2
//
// 두 번째 순회
// -3        -2         0
// (A) <---- (B) <---- (C)
//       -1        -2

// 모든 간선이 연결되었다고 가정한 다음 모든 간선을 한 번 더 순회하는 이유?
// 최단 거리 테이블이 갱신 여부로 음의 사이클을 판별할 수 있기 때문임.
// for (int j = 0; j < E; j++) // 모든 간선 다시 확인
// {
//     int u = graph->edge[j].src;
//     int v = graph->edge[j].dest;
//
//     int weight = graph->edge[j].weight;
//
//     if (dist[u] != INT_MAX && dist[u] + weight < dist[v])
//     {
//         std::cout << "Negative Cycle\n";
//
//         return;
//     }
// }

// 문제) 백준 11657 : 타임머신

struct Edge
{
    int from;
    int to;
    int64_t weight;
};

constexpr int64_t INF = 999'999'999'999;

int n;
int m;

int a;
int b;
int64_t c;

vector<Edge> edges;
int64_t cost[504];

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    cin >> n >> m;

    for (int i = 0; i < m; i++)
    {
        cin >> a >> b >> c;

        edges.push_back({ a, b, c });
    }

    // go
    fill(cost, cost + 504, INF);

    cost[1] = 0;

    for (int i = 0; i < n - 1; i++)
    {
        for (const Edge& edge : edges)
        {
            if (INF == cost[edge.from])
                continue;

            cost[edge.to] = min(cost[edge.to], cost[edge.from] + edge.weight);
        }
    }

    // check a negative cycle.
    for (const Edge& edge : edges)
    {
        if (INF == cost[edge.from])
            continue;

        if (cost[edge.to] > cost[edge.from] + edge.weight)
        {
            cout << "-1";
            
            return 0;
        }
    }

    for (int i = 2; i <= n; i++)
    {
        if (INF == cost[i])
        {
            cout << "-1\n";
        }
        else
        {
            cout << cost[i] << '\n';
        }
    }

    return 0;
}
