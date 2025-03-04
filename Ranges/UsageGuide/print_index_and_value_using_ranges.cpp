// Update Date : 2025-03-05
// OS : Windows 10 64bit
// Program : Visual Studio 2022
// Version : C++23
// Configuration : Debug-x64, Release-x64

#include <iostream>
#include <random>
#include <ranges>
#include <vector>

int main()
{
    auto randomView = std::views::iota(0, 10) | std::views::transform([](int) { return std::random_device{ }() % 100; });

    std::vector<int> data(randomView.begin(), randomView.end());
    std::ranges::sort(data);

    // transform을 이용한 조회
    auto resView1 = data | std::views::transform([idx = 0](int elem) mutable {
                                              return std::pair{ idx++, elem };
                                          });

    for (const auto& [idx, elem] : resView1) 
    {
        std::cout << idx << " : " << elem << '\n';
    }

    std::cout << '\n';

    // for_each를 이용한 조회
    std::ranges::for_each(data, [idx = 0](int elem) mutable
                          {
                              std::cout << idx << " : " << elem << '\n';

                              idx++;
                          });

    std::cout << '\n';

    // iota + zip_view를 연계한 조회
    for (const auto& [idx, elem] : std::views::zip(std::views::iota(0), data))
    {
        std::cout << idx << " : " << elem << '\n';
    }

    return 0;
}
