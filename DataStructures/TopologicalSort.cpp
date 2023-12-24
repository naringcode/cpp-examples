#include <bits/stdc++.h>

using namespace std;

// 위상 정렬
// : 어떠한 정점이 다른 점정과의 관계 속에서 가지는 위치
// - (진입 간선 indegree) -> (정점) -> (진출 간선 outdegree)
// - 정점인데 앞과 뒤가 있음.

// 알고리즘 조건
// - 그래프에 방향성이 있어야 함.
// - 그래프에 사이클이 없어야 함.
// -> 유향 비순환 그래프(Directed Acyclic Graph)의 형태에서 성립

// 알고리즘 형태
// 1. 그래프와 리스트 준비
// 2. 진입 간선이 없는 정점을 리스트에 추가
// 3. 해당 정점과 그것이 가지는 진출 간선을 그래프에서 제거
// 4. 2번과 3번을 반복해서 정점이 남지 않을 때까지 수행

// DFS를 사용한 위상 정렬 구현
// 1. 리스트와 그래프 준비
// 2. 진입 간선이 없는 정점을 대상으로 DFS 실행
// 3. 더 이상 탐색할 수 없는 정점에 도달했다면 해당 정점을 리스트의 새로운 헤더로 등록
// 4. 2번과 3번을 반복해서 정점이 남지 않을 때까지 수행

// 응용 문제) 백준 2252 : 줄 세우기

int n;
int m;

int a;
int b;

vector<int> adj[32004];
int indegrees[32004];

bool visited[32004];

list<int> res;

void go(int from)
{
    visited[from] = true;

    for (int to : adj[from])
    {
        if (true == visited[to])
            continue;

        go(to);
    }

    res.push_front(from);
}

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    cin >> n >> m;

    for (int i = 0; i < m; i++)
    {
        cin >> a >> b;
        a--;
        b--;

        adj[a].push_back(b);
        indegrees[b]++;
    }

    for (int i = 0; i < n; i++)
    {
        if (0 == indegrees[i])
        {
            go(i);
        }
    }

    for (int elem : res)
    {
        cout << elem + 1 << ' ';
    }

    return 0;
}
