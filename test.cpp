#include <stdint.h>
#include <stdio.h>

uint8_t twosComplement(uint8_t input)
{
    uint8_t temp = ~input;
    temp += 1;

    return temp;
}

int main(void)
{
    uint8_t a = 253;
    a+=4;
    printf("%u\n", a);

    return 0;
} 