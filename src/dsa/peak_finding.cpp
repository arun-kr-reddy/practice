// ************************************************
// PRAGMAS
// ************************************************
#pragma once

// ************************************************
// INCLUDES
// ************************************************
#include "peak_finding.h"
#include <stdlib.h>
#include <time.h>

// ************************************************
// MACROS
// ************************************************
#define RAND_MAX 255U
// ************************************************
// TYPEDEF & ENUMS
// ************************************************

// ************************************************
// FUNCTION DECLARATIONS
// ************************************************
static uint32_t findMatrixColumnMax(matrix_t matrix, uint32_t col);

// ************************************************
// FUNCTION DEFINITIONS
// ************************************************
void fillArray(array_t array)
{
    srand(time(NULL));
    for (size_t i = 0; i < array.size; i++)
    {
        array.addr[i] = rand() % RAND_MAX;
    }
}

void printArray(array_t array)
{
    for (size_t i = 0; i < array.size; i++)
    {
        printf("%4d ", array.addr[i]);
    }
    printf("\n");
}

uint32_t find1DPeakStraightforward(array_t array)
{
    printArray(array);

    // check first & last elements first
    if (array.addr[0] > array.addr[1])
    {
        return array.addr[0];
    }
    else if (array.addr[array.size - 1] > array.addr[array.size - 2])
    {
        return array.addr[array.size - 1];
    }

    // check remaining ones
    for (int i = 1; i < array.size - 2; ++i)
    {
        uint32_t left_value   = array.addr[i - 1];
        uint32_t centre_value = array.addr[i];
        uint32_t right_value  = array.addr[i + 1];

        if ((centre_value >= left_value) && (centre_value >= right_value))
        {
            return centre_value;
        }
    }

    return NOT_FOUND;
}

uint32_t find1DPeakDivideConquer(array_t array)
{
    size_t midpoint  = array.size / 2;
    size_t new_start = 0;
    size_t new_end   = array.size;

    printArray(array);

    // check first & last elements first
    if (array.addr[0] > array.addr[1])
    {
        return array.addr[0];
    }
    else if (array.addr[array.size - 1] > array.addr[array.size - 2])
    {
        return array.addr[array.size - 1];
    }

    if (array.size > 2)
    {
        uint32_t left_value   = array.addr[midpoint - 1];
        uint32_t centre_value = array.addr[midpoint];
        uint32_t right_value  = array.addr[midpoint + 1];

        if (left_value > centre_value)    // check left first
        {
            new_end = midpoint;
        }
        else if (right_value > centre_value)    // then check right
        {
            new_start = midpoint;
        }
        else    // midpoint is the peak
        {
            return array.addr[midpoint];
        }
    }
    else
    {
        return NOT_FOUND;
    }

    // search peak in new subarray
    array_t new_array = {0, new_end - new_start + 1};
    new_array.addr    = (uint8_t *)malloc(new_array.size);
    for (size_t i = 0; i < new_array.size; i++)
    {
        new_array.addr[i] = array.addr[new_start + i];
    }
    uint32_t peak = find1DPeakDivideConquer(new_array);

    free(new_array.addr);

    return peak;
}

void fillMatrix(matrix_t matrix)
{
    srand(time(NULL));
    for (size_t row = 0; row < matrix.height; row++)
    {
        for (size_t col = 0; col < matrix.width; col++)
        {
            matrix.addr[row * matrix.width + col] = rand() % RAND_MAX;
        }
    }
}

void printMatrix(matrix_t matrix)
{
    for (size_t row = 0; row < matrix.height; row++)
    {
        for (size_t col = 0; col < matrix.width; col++)
        {
            printf("%4d ", matrix.addr[row * matrix.width + col]);
        }
        printf("\n");
    }
}

uint32_t find2DPeakGreedyAscent(matrix_t matrix)
{
    printMatrix(matrix);

    point2d_t position = {matrix.height / 2, matrix.width / 2};

    while (1)
    {
        int32_t centre_value = matrix.addr[position.row * matrix.width + position.col];
        int32_t left_value, right_value, up_value, down_value;

        printf("%4d ", centre_value);

        // init all neighbors
        left_value = right_value = up_value = down_value = INVALID;

        // check for edges
        if (position.col > 0)
            left_value = matrix.addr[position.row * matrix.width + (position.col - 1)];
        if (position.col < (matrix.width - 1))
            right_value = matrix.addr[position.row * matrix.width + (position.col + 1)];
        if (position.row > 0)
            up_value = matrix.addr[(position.row - 1) * matrix.width + position.col];
        if (position.row < (matrix.height - 1))
            down_value = matrix.addr[(position.row + 1) * matrix.width + position.col];

        // compare to neighbors
        if (matrix.width > 1 && matrix.height > 1)
        {
            if (left_value > centre_value)    // check left first
            {
                printf(" -> ");
                position.col--;
            }
            else if (right_value > centre_value)    // then check right
            {
                printf(" -> ");
                position.col++;
            }
            else if (up_value > centre_value)    // then check up
            {
                printf(" -> ");
                position.row--;
            }
            else if (down_value > centre_value)    // then check down
            {
                printf(" -> ");
                position.row++;
            }
            else    // midpoint is the peak
            {
                printf("\n");
                return matrix.addr[position.row * matrix.width + position.col];
            }
        }
        else
        {
            return NOT_FOUND;
        }
    }

    return NOT_FOUND;
}

uint32_t find2DPeakDivideConquer(matrix_t matrix)
{
    printMatrix(matrix);

    uint32_t peak      = INVALID;
    point2d_t position = {matrix.height / 2, matrix.width / 2};

    position.row = findMatrixColumnMax(matrix, position.col);
    printf("max in column %d is %d\n", position.row, matrix.addr[position.row * matrix.width + position.col]);

    uint32_t centre_value = matrix.addr[position.row * matrix.width + position.col];
    uint32_t left_value, right_value;

    // check for edges
    left_value = right_value = INVALID;
    if (position.col > 0)
        left_value = matrix.addr[position.row * matrix.width + (position.col - 1)];
    if (position.col < (matrix.width - 1))
        right_value = matrix.addr[position.row * matrix.width + (position.col + 1)];

    // compare to neighbors
    if (left_value > centre_value)    // check left first
    {
        position.col = 0;
    }
    else if (right_value > centre_value)    // then check right
    {
        position.col = (matrix.width / 2) - 1;
    }
    else
    {
        return centre_value;
    }

    // search peak in new subarray
    matrix_t new_matrix = {0, (matrix.width / 2) + 1, matrix.height};
    new_matrix.addr     = (uint8_t *)malloc(new_matrix.width * new_matrix.height);
    for (size_t row = 0; row < new_matrix.height; row++)
    {
        for (size_t col = 0; col < new_matrix.width; col++)
        {
            new_matrix.addr[row * new_matrix.width + col] = matrix.addr[row * matrix.width + (col + position.col)];
        }
    }

    peak = find2DPeakDivideConquer(new_matrix);

    free(new_matrix.addr);

    return peak;
}

uint32_t findMatrixColumnMax(matrix_t matrix, uint32_t col)
{
    uint32_t column_max_row = 0;
    for (int32_t i = 1; i < matrix.height; i++)
    {
        if (matrix.addr[i * matrix.width + col] > matrix.addr[column_max_row * matrix.width + col])
        {
            column_max_row = i;
        }
    }

    return column_max_row;
}