// Update Date : 2025-02-03
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <random>
#include <algorithm>
#include <vector>

// https://en.cppreference.com/w/cpp/iterator/output_iterator
// https://learn.microsoft.com/en-us/cpp/standard-library/iterator-concepts?view=msvc-170

// 기본적인 내용은 "iterator_concepts.txt"에서 확인하도록 한다.

// output_iterator는 한 방향으로 이동이 가능하며 쓰기(출력)가 가능한 반복자에 대한 컨셉이다.
// 해당 제약을 요구하는 ranges 알고리즘이 있다면 맞춰서 구현해야 한다.
//
// ranges 알고리즘은 보통 입력을 기반으로 하기 때문에 출력에 대한 제약인 output_iterator를 요구하는 경우는 거의 없다.

template <std::integral T>
class MyIterator
{
public:
    // using iterator_category = std::output_iterator_tag; // output_iterator는 딱히 태그에 제약을 걸지 않음.
    using difference_type = std::ptrdiff_t;
    using value_type     = T;
    using pointer_type   = T*;
    using reference_type = T&;

public:
    MyIterator() = default;
    MyIterator(pointer_type ptr)
        : _ptr{ ptr }
    { }

public: // input/output
    // input과 output은 같은 연산자를 요구하는데 용도가 살짝 다르다.
    //
    // *outIter = 100;  // output_iterator가 요구되는 경우
    // cout << *inIter; // input_iterator가 요구되는 경우

    reference_type operator*() const
    {
        return *_ptr;
    }
    
    pointer_type operator->() const
    {
        return _ptr;
    }

public: // input/output
    MyIterator& operator++()
    {
        _ptr++;

        return *this;
    }

    MyIterator operator++(int)
    {
        MyIterator ret = *this; // copy

        _ptr++;

        return ret;
    }

    friend bool operator==(const MyIterator& lhs, const MyIterator& rhs)
    {
        return lhs._ptr == rhs._ptr;
    }
    
    friend bool operator!=(const MyIterator& lhs, const MyIterator& rhs)
    {
        return !(lhs == rhs);
    }

private:
    pointer_type _ptr;
};

template <std::integral T>
class MyContainer
{
public:
    using Iterator = MyIterator<T>;

public:
    MyContainer(size_t size = 0)
    {
        _capacity = std::max(size, _kCapacityStep);
        _size = size;

        _items = new T[_capacity];
        std::memset(_items, 0x00, sizeof(T) * _capacity);
    }

    ~MyContainer()
    {
        delete[] _items;
    }

public:
    Iterator begin() { return Iterator{ &_items[0] }; }
    Iterator end() { return Iterator{ &_items[_size] }; }

public:
    friend std::ostream& operator<<(std::ostream& os, const MyContainer& container)
    {
        os << "Container[ size : " << container._size << ", capacity : " << container._capacity << " ] : [ ";

        for (size_t idx = 0; idx < container._size; idx++)
        {
            os << container._items[idx] << ' ';
        }

        os << "]";

        return os;
    }

public:
    void add(const T& item)
    {
        if (_capacity == _size)
        {
            _capacity += _kCapacityStep;

            T* newItems = new T[_capacity];
            memcpy(newItems, _items, sizeof(T) * _size);

            delete[] _items;
            _items = newItems;
        }

        _items[_size] = item;
        _size++;
    }

public:
    size_t capacity() const { return _capacity; }
    size_t size() const { return _size; }

private:
    T* _items;

    size_t _capacity;
    size_t _size = 0;

    static const size_t _kCapacityStep = 5;
};

// 무한 재귀 발생
// template <typename Printable>
//     requires (!std::ranges::range<Printable>) &&
//     requires (Printable& printable) { std::cout << printable; }
// std::ostream& operator<<(std::ostream& os, Printable& printable)
// {
//     os << printable;
// 
//     return os;
// }

template <typename Container>
    requires (std::ranges::range<Container>) &&
    requires (typename Container::value_type elem) { std::cout << elem; }
std::ostream& operator<<(std::ostream& os, const Container& container)
{
    os << "[ ";

    for (auto& elem : container)
    {
        os << elem << ' ';
    }

    os << "]";

    return os;
}

int main()
{
    std::random_device rd;
    std::mt19937 mt{ rd() };
    std::uniform_int_distribution<int> dist{ 1, 10 };
    
    auto randGen = [&]()
        {
            return dist(mt);
        };

    MyContainer<int> container(10);
    std::ranges::generate(container, randGen); // input_or_output_iterator
    
    MyContainer<int> destContainer(container.size());

    std::cout << "Source - " << container << '\n';
    std::cout << "Dest - " << destContainer << '\n';

    // copy : 마지막 인자가 output_iterator를 요구한다.
    // std::ranges::copy(container.begin(), container.end(), destContainer.begin());
    std::ranges::copy(container, destContainer.begin());
    std::cout << "Copying...\n";
    
    std::cout << "Source - " << container << '\n';
    std::cout << "Dest - " << destContainer << '\n';

    return 0;
}
