// ************************************************
// INCLUDES
// ************************************************
#include "peak_finding.h"
#include <stdlib.h>

// ************************************************
// MACROS
// ************************************************
#define MAX_INPUT_SIZE_PEAKFINDING 10U

// ************************************************
// TYPEDEF & ENUMS
// ************************************************

// ************************************************
// FUNCTION DECLARATIONS
// ************************************************
void peakFinding1D();
void peakFinding2D();

// ************************************************
// FUNCTION DEFINITIONS
// ************************************************
int main()
{
    printf("===================== 1D peakfinding =====================\n");
    peakFinding1D();
    printf("==========================================================\n");

    printf("===================== 2D peakfinding =====================\n");
    peakFinding2D();
    printf("==========================================================\n");

    return 0;
}

void peakFinding1D()
{
    uint32_t peak;
    array_t input = {0, MAX_INPUT_SIZE_PEAKFINDING};

    input.addr = (uint8_t *)malloc(input.size);

    fillArray(input);

    peak = find1DPeakStraightforward(input);
    if (peak != NOT_FOUND)
    {
        printf("straight forward peak: %d\n", peak);
    }

    peak = find1DPeakDivideConquer(input);
    if (peak != NOT_FOUND)
    {
        printf("divide conquer peak: %d\n", peak);
    }

    free(input.addr);
}

void peakFinding2D()
{
    uint32_t peak;

    matrix_t input = {0, MAX_INPUT_SIZE_PEAKFINDING, MAX_INPUT_SIZE_PEAKFINDING};

    input.addr = (uint8_t *)malloc(input.width * input.height);

    fillMatrix(input);

    peak = find2DPeakGreedyAscent(input);
    if (peak != NOT_FOUND)
    {
        printf("greedy ascent peak: %d\n", peak);
    }

    peak = find2DPeakDivideConquer(input);
    if (peak != NOT_FOUND)
    {
        printf("divide conquer peak: %d\n", peak);
    }

    free(input.addr);
}