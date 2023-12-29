#include <bits/stdc++.h>

// 플로이드 워셜은 모든 순서쌍의 가장 짧은 거리를 구하기 위한 알고리즘이다.
// 다익스트라와 벨만포드는 하나의 시작지점으로부터 가장 짧은 거리를 구하기 때문에 차별되는 부분.
//
// 다만 시간복잡도가 O(V^3)이기 때문에 주어진 정점이 적을 경우에만 사용할 수 있다.

// 문제) 백준 11404 : 플로이드

using namespace std;

constexpr int INF = 999'999'999;

int n;
int m;

int a;
int b;
int c;

int arr[104][104];

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    fill(&arr[0][0], &arr[0][0] + 104 * 104, INF);

    cin >> n;
    cin >> m;

    for (int i = 0; i < m; i++)
    {
        cin >> a >> b >> c;

        // 시작 지점과 도착 지점이 동일한데 비용이 다른 입력이 올 수 있음.
        arr[a][b] = min(arr[a][b], c);
    }

    for (int k = 1; k <= n; k++)
    {
        for (int i = 1; i <= n; i++)
        {
            for (int j = 1; j <= n; j++)
            {
                arr[i][j] = min(arr[i][j], arr[i][k] + arr[k][j]);
            }
        }
    }

    for (int i = 1; i <= n; i++)
    {
        for (int j = 1; j <= n; j++)
        {
            if (INF == arr[i][j] || i == j)
            {
                cout << "0 ";
            }
            else
            {
                cout << arr[i][j] << ' ';
            }
        }

        cout << '\n';
    }

    return 0;
}
