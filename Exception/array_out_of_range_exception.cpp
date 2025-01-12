#include <bits/stdc++.h>

using namespace std;

int main()
{
    array<int, 4> arr{ 1, 2, 3, 4 };

    try
    {
        cout << arr.at(4) << '\n';    
    } 
    catch (const out_of_range& err)
    {
        cout << err.what() << '\n';
    }

    return 0;
}
