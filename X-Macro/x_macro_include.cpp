#include <iostream>
#include <array>

// 멀티플레이어 게임 프로그래밍 | 챕터 12에서 등장하는 X Macro
// 자세한 내용은 예제 파일에 수록된 GamerServices 클래스를 보도록 하기

/**
 * 여기서는 데이터가 정의된 Stats.def 파일을 활용한다.
 *
 * Stats.def에는 이름과 자료형이 STAT(NumGames, INT)과 같은 형식으로 작성되어 있다.
 * 이렇게 정의 내용을 파일 한 군데 몰아서 작성하면 다른 여러 곳에서 각기 다른 문맥으로 인클루드해서 재사용할 수 있다.
 */
enum class StatType
{
    INT,
    FLOAT,
    AVGRATE
};

// 통계 데이터를 위한 열거형
enum Stat
{
#define STAT(a, b) Stat_##a,
#include "Stats.def"
#undef STAT
    MAX_STAT

    // 최종적으로는 다음 열거 데이터가 나열됨.
    /**
     * Stat_NumGames,
     * Stat_NumWins,
     * Stat_NumLosses,
     * Stat_FeetTraveled,
     * Stat_MaxFeetTraveled,
     * Stat_AverageSpeed,
     * MAX_STAT
     */
};

struct StatData
{
    const char* name;
    StatType    type;

    int   intStat = 0;
    float floatStat = 0;

    struct
    {
        float sessionValue = 0.0f;
        float sessionLength = 0.0f;
    } avgRateStat;

    StatData(const char* name, StatType type)
        : name(name), type(type)
    { }
};

int main()
{
    // "StatType::##b" 이렇게 결합하는 것은 컴파일러에 따라 지원하지 않을 수도 있음
    std::array<StatData, MAX_STAT> statArr{
        #define STAT(a, b) StatData(#a, StatType::##b),
        #include "Stats.def"
        #undef STAT
    };

    // 통계 정보를 수신하는 내용은 책을 참고하도록 하기
    // 멀티플레이어 게임 프로그래밍 | 챕터 12 | 조슈아 글레이저, 산자이 마드하브
    for (int idx = 0; idx < MAX_STAT; idx++)
    {
        StatData& stat = statArr[idx];

        if (stat.type == StatType::INT)
        {
            std::cout << "Name : " << stat.name << ", StatType::INT, Value : " << stat.intStat << '\n';
        }
        else if (stat.type == StatType::FLOAT)
        {
            std::cout << "Name : " << stat.name << ", StatType::FLOAT, Value : " << stat.floatStat << '\n';
        }
        else // if (stat.type == StatType::AVGRATE)
        {
            float val = stat.avgRateStat.sessionValue / stat.avgRateStat.sessionLength;

            std::cout << "Name : " << stat.name << ", StatType::AVGRATE, Value : " << val << '\n';
        }
    }

    return 0;
}
