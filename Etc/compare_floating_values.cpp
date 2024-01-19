#include <iostream>
#include <cmath>

static const float kEpsilon = 0.0001f;

bool IsEqual(float x, float y)
{
    return fabs(x - y) < kEpsilon;
}

bool IsGreaterThanOrEqual(float x, float y)
{
    return x > y || IsEqual(x, y);
}

bool IsLessThanOrEqual(float x, float y)
{
    return x < y || IsEqual(x, y);
}

int main() 
{
    float a;
    float b;

    std::cin >> a >> b;

    if (IsEqual(a, b))
    {
        std::cout << "Equal\n";
    }

    if (IsGreaterThanOrEqual(a, b))
    {
        std::cout << "Greater Than or Equal\n";
    }

    if (IsLessThanOrEqual(a, b))
    {
        std::cout << "Less Than or Equal\n";
    }

    return 0;
}
