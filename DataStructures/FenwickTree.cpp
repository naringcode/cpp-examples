#include <bits/stdc++.h>

using namespace std;

// 펜윅트리
// - 세그먼트 트리의 변형.
//   - 세그먼트 트리는 모든 세그먼트를 만들어야 하지만 펜윅트리는 그럴 필요가 없음.
// - 노드는 1부터 시작함.
// - 각 노드는 구간 합와 연관이 있음.
// - 각 노드는 인덱스의 최하위 비트를 뺀 값에 해당하는 이전 노드와 연결됨.
//   : prev = idx - (idx & -idx) // 쿼리 함수에서 사용
//   : next = idx + (idx & -idx) // 업데이트 함수에서 사용
// - 업데이트 함수와 쿼리 함수로 구성하는 것이 일반적임.
// - 업데이트 함수는 영향을 미치는 모든 노드의 값을 갱신해야 함(더하거나 빼거나).

// 최하위 비트를 구하는 공식
// : (idx & -idx)

// 쿼리 함수는 구간 전체의 값을 구하기 때문에
// 중간의 누적합만 도출하고 싶다면 쿼리 함수를 2번 사용해서 이전의 값을 빼야 함.
// int range(int left, int right)
// {
//     return query(right) - query(left - 1);
// }

int n;
int m;
int k;

int a;
int b;
int64_t c;

int64_t arr[1'000'004];
int64_t tree[1'000'004];

void update(int idx, int64_t amount)
{
    while (idx <= n)
    {
        tree[idx] += amount;
        idx += (idx & -idx);
    }
}

int64_t sum(int idx)
{
    int64_t ret = 0;

    while (0 != idx)
    {
        ret += tree[idx];
        idx -= (idx & -idx);
    }

    return ret;
}

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    cin >> n >> m >> k;

    for (int i = 1; i <= n; i++)
    {
        cin >> arr[i];

        update(i, arr[i]);
    }

    for (int i = 0; i < m + k; i++)
    {
        cin >> a >> b >> c;

        if (1 == a)
        {
            int64_t amount = c - arr[b];
            arr[b] = c;

            update(b, amount);
        }
        else if (2 == a)
        {
            cout << sum(c) - sum(b - 1) << '\n';
        }
    }

    return 0;
}
