#include <bits/stdc++.h>

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
