#include <iostream>
#include <random>

// 선택 정렬(Selection Sort)

int main()
{
    using namespace std;

    constexpr int kLen = 10;
    int arr[kLen];

    random_device device;

    for (int i = 0; i < kLen; i++)
    {
        arr[i] = device() % 20;
    }

    for (int i = 0; i < kLen - 1; i++)
    {
        int small = i;

        for (int j = i + 1; j < kLen; j++)
        {
            if (arr[small] > arr[j])
            {
                small = j;
            }
        }

        swap(arr[i], arr[small]);
    }

    for (int i = 0; i < kLen; i++)
    {
        cout << arr[i] << '\n';
    }

    return 0;
}
