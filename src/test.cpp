// ************************************************
// INCLUDES
// ************************************************
#include "dsa.h"
#include "peak_finding.h"

// ************************************************
// MACROS
// ************************************************
#define MAX_INPUT_SIZE_PEAKFINDING 5
// ************************************************
// TYPEDEF & ENUMS
// ************************************************

// ************************************************
// FUNCTION DECLARATIONS
// ************************************************
void peakFinding();

// ************************************************
// FUNCTION DEFINITIONS
// ************************************************
int main()
{
    peakFinding();
    return 0;
}

void peakFinding()
{
    int32_t peak;

    LOG("=========================== 1D ===========================\n");

    uint32_t size   = MAX_INPUT_SIZE_PEAKFINDING;
    uint32_t *input = (uint32_t *)malloc(size * sizeof(uint32_t));

    fillArray(input, size);

    LOG("input array:\n");
    printArray(input, size);

    peak = find1DPeakStraightforward(input, size);
    if (peak != NOT_FOUND)
    {
        LOG("straightforward peak: %d\n", peak);
    }

    peak = find1DPeakDivideConquer(input, size);
    if (peak != NOT_FOUND)
    {
        LOG("divide conquer peak: %d\n", peak);
    }

    LOG("=========================== 2D ===========================\n");

    point2d_t size2d  = {MAX_INPUT_SIZE_PEAKFINDING, MAX_INPUT_SIZE_PEAKFINDING};
    uint32_t *input2d = (uint32_t *)malloc(size2d.x * size2d.y * sizeof(uint32_t));

    fillMatrix(input2d, size2d);

    LOG("input matrix:\n");
    printMatrix(input2d, size2d);

    peak = find2DPeakGreedyAscent(input2d, size2d);
    if (peak != NOT_FOUND)
    {
        LOG("greedy ascent peak: %d\n", peak);
    }
}