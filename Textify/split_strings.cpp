// #include <bits/stdc++.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

string str;
int    res;

std::vector<string> split(const string& str, const string& delimiter)
{
    std::vector<string> ret;

    string token;

    int prevPos = 0;
    int pos;

    while ((pos = str.find(delimiter, prevPos)) != string::npos)
    {
        token = str.substr(prevPos, pos - prevPos);

        ret.push_back(token);

        // case 1)
        // const string& str 앞에 붙은 const를 제거해야 함.
        // 원본 string에 영향을 준다.
        // str.erase(0, pos + delimiter.length());

        // case 2)
        // 원본 string에 영향을 주지 않는다.
        prevPos = pos + delimiter.length();
    }

    token = str.substr(prevPos, pos);

    ret.push_back(token);

    return ret;
}

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    getline(cin, str);

    auto vec = split(str, " ");

    for (auto& elem : vec)
    {
        cout << elem << '\n';
    }

    return 0;
}
