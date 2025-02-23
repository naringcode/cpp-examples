#include <bits/stdc++.h>

using namespace std;

string print(int seconds)
{
    string mm = "00" + to_string(seconds / 60);
    string ss = "00" + to_string(seconds % 60);

    return mm.substr(mm.size() - 2, 2) + ":" + ss.substr(ss.size() - 2, 2);
}

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    for (int i = 0; i < 1000; i++)
    {
        cout << print(i) << '\n';
    }

    return 0;
}
