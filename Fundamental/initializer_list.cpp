#include <bits/stdc++.h>

using namespace std;

class MyArray
{
public:
    MyArray() = default;
    
    MyArray(const initializer_list<int>& ilist)
    {
        _len = ilist.size();
        _arr = new int[_len];

        int idx = 0;
        for (auto iter = ilist.begin(); iter != ilist.end(); iter++)
        {
            _arr[idx++] = *iter;
        }

        // int idx = 0;
        // for (auto iter = rbegin(ilist); iter != rend(ilist); iter++)
        // {
        //     _arr[idx++] = *iter;
        // }
    }

    ~MyArray()
    {
        delete[] _arr;
    }

public:
    void Print()
    {
        for (int idx = 0; idx < _len; idx++)
        {
            cout << _arr[idx] << ' ';
        }

        cout << '\n';
    }

private:
    int* _arr;
    int  _len;
};

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    MyArray arr{ 1, 2, 3, 4, 5 };

    arr.Print();

    return 0;
}
