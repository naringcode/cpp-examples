// Update Date : 2025-02-03
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++20
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <random>
#include <concepts>
#include <algorithm>
#include <ranges>
#include <vector>

// https://en.cppreference.com/w/cpp/ranges#Range_adaptors
// https://learn.microsoft.com/ko-kr/cpp/standard-library/range-concepts?view=msvc-170

// 기본적인 내용은 "iterator_concepts.txt"에서 확인하도록 한다.

// 사용자 정의 자료형이라고 해도 range의 어댑터가 요구하는 사양을 충족하는 iterator를 구성하면
// 어댑터에 사용자 정의 자료형을 전달하여 view를 구할 수 있다.
// template <std::integral T>
template <typename T>
class MyIterator
{
public:
    using iterator_category = std::random_access_iterator_tag; // 태그 명칭을 알맞게 수정해야 함(안 맞춰주면 에러 발생).
    using difference_type = std::ptrdiff_t;
    using value_type     = T;
    using pointer_type   = T*;
    using reference_type = T&;

    // const 유형의 iterator를 만들 때 사용할 것
    // using pointer_type   = const T*;
    // using reference_type = const T&;

public:
    MyIterator() = default;
    MyIterator(pointer_type ptr)
        : _ptr{ ptr }
    { }

public: // input/output/forward
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

public: // random_access
    MyIterator& operator+=(const difference_type offset)
    {
        _ptr += offset;

        return *this;
    }

    MyIterator& operator-=(const difference_type offset)
    {
        _ptr -= offset;

        return *this;
    }

    MyIterator operator+(const difference_type offset) const
    {
        return MyIterator{ _ptr + offset };
    }

    MyIterator operator-(const difference_type offset) const
    {
        return MyIterator{ _ptr - offset };
    }

    difference_type operator-(const MyIterator& rhs) const
    {
        return _ptr - rhs._ptr; // 반복자 간 거리
    }

    friend MyIterator operator+(const difference_type offset, const MyIterator& rhs)
    {
        MyIterator ret = rhs;

        ret._ptr += offset;

        return ret;
    }

    friend MyIterator operator-(const difference_type offset, const MyIterator& rhs)
    {
        MyIterator ret = rhs;

        ret._ptr -= offset;

        return ret;
    }

    reference_type operator[](const difference_type offset) const
    {
        return *(_ptr + offset);
    }

    bool operator<(const MyIterator& rhs) const
    {
        return _ptr < rhs._ptr;
    }

    bool operator>(const MyIterator& rhs) const
    {
        return _ptr > rhs._ptr;
    }

    bool operator<=(const MyIterator& rhs) const
    {
        return _ptr <= rhs._ptr;
    }

    bool operator>=(const MyIterator& rhs) const
    {
        return _ptr >= rhs._ptr;
    }

public: // bidirectional
    MyIterator& operator--()
    {
        _ptr--;

        return *this;
    }

    MyIterator operator--(int)
    {
        MyIterator ret = *this; // copy

        _ptr--;

        return ret;
    }

public: // input/output/forward
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

    bool operator==(const MyIterator& rhs) const
    {
        return _ptr == rhs._ptr;
    }

    bool operator!=(const MyIterator& rhs) const
    {
        return !(_ptr == rhs._ptr);
    }

    // friend bool operator==(const MyIterator& lhs, const MyIterator& rhs)
    // {
    //     return lhs._ptr == rhs._ptr;
    // }
    // 
    // friend bool operator!=(const MyIterator& lhs, const MyIterator& rhs)
    // {
    //     return !(lhs == rhs);
    // }

private:
    pointer_type _ptr;
};

// template <std::integral T>
template <typename T>
class MyContainer
{
public:
    using Iterator = MyIterator<T>;

public:
    MyContainer(size_t size = 0)
    {
        _capacity = std::max(size, _kCapacityStep);
        _size = size;

        // _items = new T[_capacity];
        // std::memset(_items, 0x00, sizeof(T) * _capacity); // std::string이 구조체의 멤버로 있으면 문제 생김.
        _items = new T[_capacity]{ }; // 기본 생성자를 불러오는 방식을 씀.
    }

    ~MyContainer()
    {
        delete[] _items;
    }

public:
    Iterator begin() { return Iterator{ &_items[0] }; }
    Iterator end() { return Iterator{ &_items[_size] }; }

    // const iterator
    Iterator begin() const { return Iterator{ &_items[0] }; }
    Iterator end() const { return Iterator{ &_items[_size] }; }

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

            // T* newItems = new T[_capacity];
            // memcpy(newItems, _items, sizeof(T) * _size);

            T* newItems = new T[_capacity];
            for (size_t idx = 0; idx < _size; idx++)
            {
                newItems[idx] = _items[idx];
            }

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

struct Person
{
    friend std::ostream& operator<<(std::ostream& os, const Person& rhs)
    {
        os << "[ name : " << rhs.name << ", age : " << rhs.age << " ]";

        return os;
    }

    std::string name = "";
    int         age  = 0;
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

template <std::ranges::view View>
std::ostream& operator<<(std::ostream& os, View& view)
{
    os << "[ ";

    for (auto elem : view)
    {
        std::cout << elem << ' ';
    }

    os << "]";

    return os;
}

template <typename Container>
    requires (!std::same_as<Container, std::string>) &&
             (std::ranges::range<Container>) &&
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
    std::ranges::sort(container);
    std::cout << container << '\n';

    // std::ranges::filter_view
    std::ranges::filter_view evenView = std::ranges::filter_view{ container, [](int elem) { return elem % 2 == 0; } };
    std::cout << "evens : " << evenView << '\n';
    std::cout << container << '\n'; // 원본 확인
    std::cout << '\n';

    // std::ranges::transform_view
    std::ranges::transform_view tenMulView = std::ranges::transform_view{ container, [](int elem) { return elem * 10; } };
    std::cout << "mul 10 : " << tenMulView << '\n';
    std::cout << container << '\n'; // 원본 확인
    std::cout << '\n';

    // std::ranges::take_view
    std::ranges::take_view takenView = std::ranges::take_view{ container, 4 };
    std::cout << "take 4 elems : " << takenView << '\n';
    std::cout << container << '\n'; // 원본 확인
    std::cout << '\n';

    // std::ranges::take_while_view
    std::ranges::take_while_view takenWhileView = std::ranges::take_while_view{ container, [](int elem) { return elem < 5; } };
    std::cout << "under 5 : " << takenWhileView << '\n';
    std::cout << container << '\n'; // 원본 확인
    std::cout << '\n';

    // std::ranges::drop_view
    std::ranges::drop_view dropView = std::ranges::drop_view{ container, 4 };
    std::cout << "drop 4 elems : " << dropView << '\n';
    std::cout << container << '\n'; // 원본 확인
    std::cout << '\n';

    // std::views::drop_while_view
    std::ranges::drop_while_view dropWhileView = std::ranges::drop_while_view{ container, [](int elem) { return elem < 5; } };
    std::cout << "over 5 : " << dropWhileView << '\n';
    std::cout << container << '\n'; // 원본 확인
    std::cout << '\n';

    // view pipelines
    auto evenFilter = std::views::filter([](int elem) { return elem % 2 == 0; });
    auto productTen = std::views::transform([](int elem) { return elem * 10; });
    auto complexView = container | evenFilter | productTen;
    std::cout << "evens -> mul 10 : " << complexView << '\n';
    std::cout << container << '\n'; // 원본 확인
    std::cout << '\n';

    // TEMP
    // auto ret = std::ranges::sort(container);
    // 
    // std::cout << container << '\n';
    // 
    // auto evens = std::ranges::filter_view{ container, [](int i) { return i % 2 == 0; } };
    //     
    // // for (auto elem : evens)
    // // {
    // //     std::cout << elem << ' ';
    // // }
    // 
    // for (auto elem : container | std::views::drop(4))
    // {
    //     std::cout << elem << ' ';
    // }

    // 
    MyContainer<Person> container2;
    container2.add({ "A", 10 });
    container2.add({ "B", 20 });
    container2.add({ "C", 15 });
    container2.add({ "D", 25 });

    for (auto& elem : container2)
    {
        std::cout << elem << '\n';
    }

    std::cout << '\n';

    //
    std::ranges::sort(container2, std::less{ }, &Person::age);
    std::cout << "Sorting by age :\n";

    for (auto& elem : container2)
    {
        std::cout << elem << '\n';
    }

    std::cout << '\n';

    //
    std::cout << "Age under 18 :\n";
   
    for (auto& elem : container2 | std::views::take_while([](const Person& elem) { return elem.age < 18; }))
    {
        std::cout << elem << '\n';
    }

    return 0;
}
