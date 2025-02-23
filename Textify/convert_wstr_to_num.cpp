#include <iostream>
#include <cwchar>
#include <cerrno>
#include <numeric>
#include <type_traits>

using namespace std;

// https://en.cppreference.com/w/cpp/types/is_integral
// https://learn.microsoft.com/ko-kr/cpp/c-runtime-library/reference/strtol-wcstol-strtol-l-wcstol-l?view=msvc-170#return-value
// https://www.ibm.com/docs/ko/i/7.5?topic=lf-wcstol-wcstoll-convert-wide-character-string-long-long-long-integer#wcstol__title__9

// 해당 기능을 넣는다 하면 NumericHelper 정도로 분류하면 적당할 듯.

template <typename IntType>
bool CheckIntegerBound(int64_t value)
{
    static_assert(std::is_integral_v<IntType>, "IntType must be an integer type.");

    // if (value > numeric_limits<IntType>::max() || value < numeric_limits<IntType>::min())
    //     return false;
    // 
    // return true;

    return (value <= numeric_limits<IntType>::max()) && (value >= numeric_limits<IntType>::min());
}

#define DECLARE_CONVERT_WSTRING_TO_INTEGER_FUNC(type) \
bool ConvertTo##type(const wchar_t* wStr, type* outValue);

#define DEFINE_CONVERT_WSTRING_TO_INTEGER_FUNC(type)        \
bool ConvertTo##type(const wchar_t* wStr, type* outValue)   \
{                                                           \
    if (nullptr == wStr || nullptr == outValue)             \
        return false;                                       \
                                                            \
    wchar_t* end = nullptr;                                 \
    errno = 0;                                              \
                                                            \
    int64_t res = ::wcstoll(wStr, &end, 10);                \
                                                            \
    /* 오버플로우, 언더플로우 체크 */ \
    if (ERANGE == errno)                                    \
        return false;                                       \
                                                            \
    /* 문자열 끝에 도달 안 한 경우 */ \
    if (L'\0' != *end)                                      \
        return false;                                       \
                                                            \
    /* 데이터가 경계 안에 있는지 확인 */ \
    if (false == CheckIntegerBound<std::remove_pointer_t<decltype(outValue)>>(res))  \
        return false;                                       \
                                                            \
    *outValue = static_cast<type>(res);                     \
                                                            \
    return true;                                            \
}

using Int8 = int8_t;
using Int16 = int16_t;
using Int32 = int32_t;
using Int64 = int64_t;

DECLARE_CONVERT_WSTRING_TO_INTEGER_FUNC(Int8)
DEFINE_CONVERT_WSTRING_TO_INTEGER_FUNC(Int8)

DECLARE_CONVERT_WSTRING_TO_INTEGER_FUNC(Int16)
DEFINE_CONVERT_WSTRING_TO_INTEGER_FUNC(Int16)

DECLARE_CONVERT_WSTRING_TO_INTEGER_FUNC(Int32)
DEFINE_CONVERT_WSTRING_TO_INTEGER_FUNC(Int32)

DECLARE_CONVERT_WSTRING_TO_INTEGER_FUNC(Int64)
DEFINE_CONVERT_WSTRING_TO_INTEGER_FUNC(Int64)

// bool ConvertToInt8(const wchar_t* wStr, int8_t* outValue)
// {
//     if (nullptr == wStr || nullptr == outValue)
//         return false;
// 
//     wchar_t* end = nullptr;
//     errno = 0;
// 
//     int64_t res = ::wcstoll(wStr, &end, 10);
// 
//     /* 오버플로우, 언더플로우 체크 */
//     if (ERANGE == errno)
//         return false;
// 
//     /* 문자열 끝에 도달 안 한 경우 */
//     if (L'\0' != *end)
//         return false;
// 
//     /* 데이터가 경계 안에 있는지 확인 */
//     if (false == CheckIntegerBound<std::remove_pointer_t<decltype(outValue)>>(res))
//         return false;
// 
//     *outValue = static_cast<int32_t>(res);
// 
//     return true;
// }

bool ConvertToFloat(const wchar_t* wStr, float* outValue)
{
    if (nullptr == wStr || nullptr == outValue)
        return false;

    wchar_t* end = nullptr;
    errno = 0;

    float res = ::wcstof(wStr, &end);

    /* 오버플로우, 언더플로우 체크 */
    if (errno == ERANGE)
        return false;

    /* 문자열 끝에 도달 안 한 경우 */
    if (L'\0' != *end)
        return false;

    *outValue = res;

    return res;
}

bool ConvertToDouble(const wchar_t* wStr, double* outValue)
{
    if (nullptr == wStr || nullptr == outValue)
        return false;

    wchar_t* end = nullptr;
    errno = 0;

    double res = ::wcstod(wStr, &end);

    /* 오버플로우, 언더플로우 체크 */
    if (errno == ERANGE)
        return false;

    /* 문자열 끝에 도달 안 한 경우 */
    if (L'\0' != *end)
        return false;

    *outValue = res;

    return res;
}

int main()
{
    wchar_t wNumStr[] = L"111222333444"; // 111'222'333'444

    Int8  outInt8;
    Int16 outInt16;
    Int32 outInt32;
    Int64 outInt64;

    if (ConvertToInt8(wNumStr, &outInt8))
    {
        cout << "Succeeded -  ConvertToInt8() : " << outInt8 << '\n';
    }
    else
    {
        cout << "Failed - ConvertToInt8()\n";
    }

    if (ConvertToInt16(wNumStr, &outInt16))
    {
        cout << "Succeeded -  ConvertToInt16() : " << outInt16 << '\n';
    }
    else
    {
        cout << "Failed - ConvertToInt16()\n";
    }

    if (ConvertToInt32(wNumStr, &outInt32))
    {
        cout << "Succeeded -  ConvertToInt32() : " << outInt32 << '\n';
    }
    else
    {
        cout << "Failed - ConvertToInt32()\n";
    }

    if (ConvertToInt64(wNumStr, &outInt64))
    {
        cout << "Succeeded -  ConvertToInt64() : " << outInt64 << '\n';
    }
    else
    {
        cout << "Failed - ConvertToInt64()\n";
    }

    cout << "--------------------------------------------------\n";

    wchar_t wMultiNumStr[] = L"123 456 789";

    if (ConvertToInt64(wMultiNumStr, &outInt64))
    {
        cout << "Succeeded -  ConvertToInt64() : " << outInt64 << '\n';
    }
    else
    {
        cout << "Failed - ConvertToInt64()\n";
    }

    cout << "--------------------------------------------------\n";

    wchar_t wBigNumStr[] = L"111222333444555666777888999";

    if (ConvertToInt64(wBigNumStr, &outInt64))
    {
        cout << "Succeeded -  ConvertToInt64() : " << outInt64 << '\n';
    }
    else
    {
        cout << "Failed - ConvertToInt64()\n";
    }

    cout << "--------------------------------------------------\n";

    wchar_t wStrCombined1[] = L"111abc";
    wchar_t wStrCombined2[] = L"abc111";

    if (ConvertToInt64(wStrCombined1, &outInt64))
    {
        cout << "Succeeded -  ConvertToInt64() : " << outInt64 << '\n';
    }
    else
    {
        cout << "Failed - ConvertToInt64()\n";
    }

    if (ConvertToInt64(wStrCombined2, &outInt64))
    {
        cout << "Succeeded -  ConvertToInt64() : " << outInt64 << '\n';
    }
    else
    {
        cout << "Failed - ConvertToInt64()\n";
    }

    cout << "--------------------------------------------------\n";

    wchar_t wRealNum[] = L"3.14";

    float  outFloat;
    double outDouble;

    if (ConvertToFloat(wRealNum, &outFloat))
    {
        cout << "Succeeded -  ConvertToFloat() : " << outFloat << '\n';
    }
    else
    {
        cout << "Failed - ConvertToFloat()\n";
    }

    if (ConvertToDouble(wRealNum, &outDouble))
    {
        cout << "Succeeded -  ConvertToDouble() : " << outDouble << '\n';
    }
    else
    {
        cout << "Failed - ConvertToDouble()\n";
    }

    cout << "--------------------------------------------------\n";

    wchar_t wMultiRealNum[] = L"3.14 1.4141";

    if (ConvertToFloat(wMultiRealNum, &outFloat))
    {
        cout << "Succeeded -  ConvertToFloat() : " << outFloat << '\n';
    }
    else
    {
        cout << "Failed - ConvertToFloat()\n";
    }

    if (ConvertToDouble(wMultiRealNum, &outDouble))
    {
        cout << "Succeeded -  ConvertToDouble() : " << outDouble << '\n';
    }
    else
    {
        cout << "Failed - ConvertToDouble()\n";
    }

    return 0;
}
