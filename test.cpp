#include <stdio.h>

#define MIN(A, B) ((A) <= (B) ? (A) : (B))

main()
{
    int a[2] = {10, 20};
    int *p = a;
    printf("%d\n", MIN(*p++, 10));
}