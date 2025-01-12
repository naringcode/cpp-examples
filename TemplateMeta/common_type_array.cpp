#include <bits/stdc++.h>

using namespace std;

// https://learn.microsoft.com/ko-kr/cpp/cpp/ellipses-and-variadic-templates?view=msvc-170
// https://learn.microsoft.com/ko-kr/cpp/standard-library/common-type-class?view=msvc-170

// 원소의 타입을 결정하기 위해서 common_type을 사용.
// 아래 함수는 인자에 의존적이기 때문에 함수 반환형을 후행 리턴 타입으로 지정.
template<typename ... Args>
auto build_array(Args&&... args) -> array<typename common_type<Args...>::type, sizeof...(args)>
{
    using commonType = typename common_type<Args...>::type;

    return { std::forward<commonType>((Args&&)args)... };
}

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    auto data = build_array(1, 0u, 'a', 3.2f, false);

    for (auto elem : data)
    {
        cout << elem << '\n';
    }

    return 0;
}
