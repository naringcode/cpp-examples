#include <bits/stdc++.h>

/**
 * x_macro_1.cpp와 동일한 기능을 수행하지만 FOREACH_COLORS(DO)를 간소화한 형태
 * FOREACH_COLORS(DO) -> FOREACH_COLORS
 * 
 * DO라는 패턴을 명시하지 않고 최초 리스트 나열 시 X가 사용될 것임을 이용한 방식.
 */

using namespace std;

// DO라는 생성 패턴을 작성하지 않고 임의의 기능 X를 사용하는 방식
#define FOREACH_COLORS \
    X(RED) \
    X(GREEN) \
    X(BLUE) \
    X(YELLOW)

// enum으로 정의
enum class Color
{
    #define X(value) value, // 마지막에 ','를 붙이는 것이 핵심
        FOREACH_COLORS
    #undef X
};

// 함수로 받아오는 방식
const char* ToString(Color color)
{
    switch (color)
    {
        #define X(value) \
            case Color::value:  \
                return #value;

            FOREACH_COLORS
        #undef X
    }

    return "";
}

// 변수로 매핑하는 방식
static const char* colorString[] = {
    // C++에선 "ABC" "DEF" 이렇게 연속된 문자열 리터럴을 나열하면 컴파일러가 알아서 하나의 문자열로 합친다.
    #define X(value) "Color::" #value,
    FOREACH_COLORS
    #undef X
};

int main() 
{
    Color myColor = Color::RED;
    std::cout << "The color is " << ToString(myColor) << std::endl;

    myColor = Color::BLUE;
    std::cout << "The color is " << colorString[(int)myColor] << std::endl;

    return 0;
}
