// ************************************************
// PRAGMAS
// ************************************************
#pragma once

// ************************************************
// INCLUDES
// ************************************************
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

// ************************************************
// MACROS
// ************************************************

// ************************************************
// TYPEDEF & ENUMS
// ************************************************
using std::cout;
using std::endl;
using std::vector;

typedef struct
{
    int32_t x;
    int32_t y;
} point2d_t;

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
uint32_t rand_bounded();
uint32_t find1DPeakStraightforward(vector<uint32_t> input);
uint32_t find1DPeakDivideConquer(vector<uint32_t> input);
uint32_t find2DPeakGreedyAscent(vector<vector<uint32_t>> input);
uint32_t find2DPeakDivideConquer(vector<vector<uint32_t>> input);