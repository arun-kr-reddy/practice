// ************************************************
// PRAGMAS
// ************************************************
#pragma once

// ************************************************
// INCLUDES
// ************************************************
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// ************************************************
// MACROS
// ************************************************
#define RAND_MAX_VALUE 1000

#define ENABLE_DEBUG_LOGS 1

#if ENABLE_DEBUG_LOGS
    #define LOG(...) fprintf(stdout, __VA_ARGS__)
#else
    #define LOG(...)
#endif

// ************************************************
// TYPEDEF & ENUMS
// ************************************************
typedef enum
{
    SUCCESS   = 0,
    NOT_FOUND = -1
} error_t;

typedef enum
{
    ZERO    = 0,
    INVALID = -1
} value_t;

typedef struct
{
    uint32_t x;
    uint32_t y;
} point2d_t;

// ************************************************
// FUNCTION DECLARATIONS
// ************************************************
void fillArray(uint32_t *arr, size_t size);
void printArray(uint32_t *arr, size_t size);
void fillMatrix(uint32_t *arr, point2d_t size);
void printMatrix(uint32_t *arr, point2d_t size);

// ************************************************
// FUNCTION DEFINITIONS
// ************************************************
void fillArray(uint32_t *arr, size_t size)
{
    for (uint32_t i = 0; i < size; ++i)
    {
        arr[i] = rand() % RAND_MAX_VALUE;
    }

    return;
}

void printArray(uint32_t *arr, size_t size)
{
    for (uint32_t i = 0; i < size; ++i)
    {
        LOG("%d ", arr[i]);
    }
    LOG("\n");
}

void fillMatrix(uint32_t *arr, point2d_t size)
{
    for (uint32_t i = 0; i < size.x; ++i)
    {
        for (uint32_t j = 0; j < size.y; ++j)
        {
            arr[i * size.x + j] = rand() % RAND_MAX_VALUE;
        }
    }

    return;
}

void printMatrix(uint32_t *arr, point2d_t size)
{
    for (uint32_t i = 0; i < size.x; ++i)
    {
        for (uint32_t j = 0; j < size.y; ++j)
        {
            LOG("%d ", arr[i * size.x + j]);
        }
        LOG("\n");
    }
    LOG("\n");
}