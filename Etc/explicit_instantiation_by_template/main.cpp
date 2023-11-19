#include <iostream>

#include "MyArray.h"

int main()
{
    using namespace std;

    MyArray<char> chArray;
    MyArray<int> intArray;

    for (size_t idx = 0; idx < 10; idx++)
    {
        chArray.SetElement(idx, 'A' + idx);
    }

    for (size_t idx = 0; idx < 10; idx++)
    {
        intArray.SetElement(idx, idx * 10);
    }

    chArray.Print();
    intArray.Print();

    return 0;
}
