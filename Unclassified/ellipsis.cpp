#include <iostream>
#include <cstdarg> // ellipsis

// https://stackoverflow.com/questions/11270588/variadic-function-va-arg-doesnt-work-with-float

int calc_sum(int cnt, ...)
{
    int sum = 0;

    va_list ap;

    va_start(ap, cnt);
    {
        for (int i = 0; i < cnt; i++)
        {
            sum += va_arg(ap, int);
        }
    }
    va_end(ap);

    return sum;
}

void my_print(const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    {
        int idx = 0;

        while ('\0' != fmt[idx])
        {
            if ('d' == fmt[idx])
            {
                printf("%d ", va_arg(ap, int));
            }
            else if ('f' == fmt[idx])
            {
                printf("%lf ", va_arg(ap, double)); // float is promoted to double.
            }
            else if ('c' == fmt[idx])
            {
                printf("%c ", va_arg(ap, char));
            }
            else if ('s' == fmt[idx])
            {
                printf("%s ", va_arg(ap, char*));
            }

            idx++;
        }
    }
    va_end(ap);

    printf("\n");
}

int main()
{
    using namespace std;

    cout << calc_sum(5, 1, 2, 3, 4, 5) << '\n';
    cout << calc_sum(4, 1, 2, 3, 4) << '\n';
    cout << calc_sum(3, 1, 2, 3);

    cout << "\n\n";

    my_print("dfcs", 123, 3.14f, 'B', "String Hello");

    return 0;
}
