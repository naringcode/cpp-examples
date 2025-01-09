#pragma once

template <typename T>
class MyArray
{
public:
    void Print();

public:
    void SetElement(size_t idx, T data);

private:
    T _data[10];
};

template <typename T>
void MyArray<T>::SetElement(size_t idx, T data)
{
    _data[idx] = data;
}
