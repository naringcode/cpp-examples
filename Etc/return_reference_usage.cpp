#include <iostream>
#include <array>

template <typename T, size_t N>
int& get(std::array<T, N>& arr, size_t idx)
{
    return arr[idx];
}

int main()
{
    using namespace std;

    std::array<int, 10> arrA;
    std::array<int, 20> arrB;

    for (size_t idx = 0; idx < arrA.size(); idx++)
    {
        arrA[idx] = (int)idx;
    }

    for (size_t idx = 0; idx < arrB.size(); idx++)
    {
        arrB[idx] = 100 + (int)idx;
    }

    // 레퍼런스로 반환한 것 활용하는 방법
    get(arrA, 5) = 1234;
    get(arrB, 15) = 1234;

    for (size_t idx = 0; idx < arrA.size(); idx++)
    {
        std::cout << idx << " : " << arrA[idx] << '\n';
    }

    std::cout << '\n';

    for (size_t idx = 0; idx < arrB.size(); idx++)
    {
        std::cout << idx << " : " << arrB[idx] << '\n';
    }
    
    

    return 0;
}
