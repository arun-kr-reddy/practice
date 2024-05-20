// ************************************************
// PRAGMAS
// ************************************************
#pragma once

// ************************************************
// INCLUDES
// ************************************************
#include <stdint.h>
#include <stdio.h>

// ************************************************
// MACROS
// ************************************************

// ************************************************
// TYPEDEF & ENUMS
// ************************************************
typedef struct
{
    int32_t row;
    int32_t col;
} point2d_t;

typedef struct
{
    uint8_t *addr;
    uint32_t size;
} array_t;

typedef struct
{
    uint8_t *addr;
    uint32_t width;
    uint32_t height;
} matrix_t;

typedef enum
{
    SUCCESS   = 0,
    NOT_FOUND = -1
} error_t;

typedef enum
{
    INVALID = 0
} value_t;

// ************************************************
// FUNCTION DECLARATIONS
// ************************************************
void fillArray(array_t arr);
void printArray(array_t arr);
uint32_t find1DPeakStraightforward(array_t arr);
uint32_t find1DPeakDivideConquer(array_t array);
void fillMatrix(matrix_t array);
void printMatrix(matrix_t array);
uint32_t find2DPeakGreedyAscent(matrix_t matrix);
uint32_t find2DPeakDivideConquer(matrix_t matrix);