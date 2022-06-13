#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main()
{
    printf("hello\n");
    uint8_t ipMat_1[3][3]   = {0};
    uint8_t ipMat_1[3][3]   = {0};
    uint8_t opMat[3][3]     = {0};

    for (int x = 0; x < size; x++)
    {
        for (int y = 0; y < size; y++)
        {
            int16_t l_sum = 0;
            for(int i = 0; i < size; i++)
            {
                l_sum += mat1[x+i][y]*mat2[x][y+i];
            }
            mat1[x][y] = l_sum;
        }
    }
    return 0
}