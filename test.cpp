#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include <unistd.h>

enum temp
{
    e_a = 10,
    e_b,
    e_c
};
int main()
{

    int a = 10;
    int b = 20;
    int *c = (int *)malloc(4);
    int *d = (int *)malloc(4);
    printf("%p %p %p %p\n", &a, &b, c, d);
    printf("%d %d %d\n", e_a, e_b, e_c);

    printf("size %d\n", sizeof(c));

    return 0;
}