#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t twosComplement(uint8_t input)
{
    uint8_t temp = ~input;
    temp += 1;

    return temp;
}

int main(void)
{
    uint8_t a = 253;
    uint8_t b = 255;
    uint8_t *c = (uint8_t *)malloc(sizeof(uint8_t));
    uint8_t *d = (uint8_t *)malloc(sizeof(uint8_t));
    a+=4;
    printf("%p %p %p %p\n", &a, &b, c, d);

    return 0;
} 