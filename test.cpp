#include <stdio.h>

#define macro_func(a, b) \
        printf("val " #a " : %d, " #b " : %d\n", val##a, val##b)

int main(void)
{
    int val1 = 40;
    int val2 = 30;
    macro_func(1, 2);
    return 0;
}