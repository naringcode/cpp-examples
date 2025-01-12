#include <bits/stdc++.h>

using namespace std;

// https://stackoverflow.com/questions/14218894/number-of-days-between-two-dates-c

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    tm tmStruct;
    {
        memset(&tmStruct, 0x00, sizeof(tmStruct));

        tmStruct.tm_year = 107; // 2007

        cin >> tmStruct.tm_mon; // 1 ~ 12
        cin >> tmStruct.tm_mday; // 1 ~ 31

        tmStruct.tm_mon--; // 0 ~ 11
    }

    time_t myTime = mktime(&tmStruct);

    const char* timeStr = ctime(&myTime);

    cout << timeStr[0] 
        << (char)(timeStr[1] - ('a' - 'A')) 
        << (char)(timeStr[2] - ('a' - 'A'));

    return 0;
}
