#include <bits/stdc++.h>

using namespace std;

// X macro
// https://en.wikipedia.org/wiki/X_macro
// https://www.geeksforgeeks.org/x-macros-in-c/

// X 매크로의 진가는 같은 데이터 셋을 어디선가 재사용하고자 할 때 드러난다.
// 다만 과도하게 사용하면 코드를 읽기 힘든 코드가 되니 적당히 사용하도록 한다.

// FOREACH_COLORS는 X_MACRO_LIST라고 보면 된다.
// DO는 일종의 생성 Pattern이다.
#define FOREACH_COLORS(DO) \
    DO(RED) \
    DO(GREEN) \
    DO(BLUE) \
    DO(YELLOW)

// enum으로 정의
enum class Color
{
    #define X(value) value, // 마지막에 ','를 붙이는 것이 핵심
        FOREACH_COLORS(X)   // X로 정의된 기능을 수행하도록 확장(DO는 X로 치환되는 것이지, X(value)로 치환되는 것이 아님을 주의)
    #undef X
    MAX

    /*
    DO는 X로 치환되고 곧 "value,"로 확장됨.

    // 날 것의 상태
    FOREACH_COLORS(DO) \
        DO(RED) \
        DO(GREEN) \
        DO(BLUE) \
        DO(YELLOW)

    // 치환된 상태
    FOREACH_COLORS(X) \
        X(RED) \
        X(GREEN) \
        X(BLUE) \
        X(YELLOW)

    // X에 정의된 기능에 따라 "value,"로 확장된 상태
    RED,
    GREEN,
    BLUE,
    YELLOW,

    * 혼동 주의 *
    FOREACH_COLORS(DO)는 FOREACH_COLORS(X(value))로 확장되는 것이 아니다.
    FOREACH_COLORS(DO) -> FOREACH_COLORS(X(value)) -> X(value)(RED) 이런 식으로 되는 것이 아니라는 뜻.

    단순히 DO가 X로 치환되고 최종적으로는 X에 정의된 기능을 토대로 확장되는 것.
    FOREACH_COLORS(DO) -> FOREACH_COLORS(X) -> X(RED) 이렇게 확장된다.

    X는 "X(value) value,"라는 기능을 수행하는 것일 뿐이지
    DO가 "X(value)" 이렇게 통째로 치환되는 것을 말하는 게 아님.
     */
};

// 함수로 받아오는 방식
const char* ToString(Color color)
{
    switch (color)
    {
        #define X(value) \
            case Color::value:  \
                return #value;

            FOREACH_COLORS(X)
        #undef X
    }

    return "";
}

// 변수로 매핑하는 방식
static const char* colorString[] = {
    // C++에선 "ABC" "DEF" 이렇게 연속된 문자열 리터럴을 나열하면 컴파일러가 알아서 하나의 문자열로 합친다.
    #define X(value) "Color::" #value,
    FOREACH_COLORS(X)
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
