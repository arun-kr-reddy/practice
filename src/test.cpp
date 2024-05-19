// ************************************************
// INCLUDES
// ************************************************
#include "peak_finding.h"
#include <algorithm>
#include <iomanip>
#include <vector>

// ************************************************
// MACROS
// ************************************************
#define MAX_INPUT_SIZE_PEAKFINDING 10U
#define TABLE_WIDTH                4U

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
    cout << "=========================== 1D ===========================" << endl;
    peakFinding1D();
    cout << "==========================================================" << endl;

    cout << "=========================== 2D ===========================" << endl;
    peakFinding2D();
    cout << "==========================================================" << endl;

    return 0;
}

void peakFinding1D()
{
    uint32_t peak;

    vector<uint32_t> input(MAX_INPUT_SIZE_PEAKFINDING);

    std::srand(unsigned(std::time(nullptr)));
    std::generate(input.begin(), input.end(), rand_bounded);
    for (size_t i = 0; i < input.size(); i++)
    {
        std::cout << std::setw(TABLE_WIDTH);
        cout << input.at(i) << " ";
    }
    cout << endl;

    peak = find1DPeakStraightforward(input);
    if (peak != NOT_FOUND)
    {
        cout << "straight forward peak: " << peak << endl;
    }

    peak = find1DPeakDivideConquer(input);
    if (peak != NOT_FOUND)
    {
        cout << "divide conquer peak: " << peak << endl;
    }
}

void peakFinding2D()
{
    uint32_t peak;

    vector<vector<uint32_t>> input(MAX_INPUT_SIZE_PEAKFINDING, vector<uint32_t>(MAX_INPUT_SIZE_PEAKFINDING));

    std::srand(unsigned(std::time(nullptr)));
    for (size_t row = 0; row < input.size(); row++)
    {
        std::generate(input.at(row).begin(), input.at(row).end(), rand_bounded);
        for (size_t col = 0; col < input.at(row).size(); col++)
        {
            std::cout << std::setw(TABLE_WIDTH);
            cout << input.at(row).at(col) << " ";
        }
        cout << endl;
    }

    peak = find2DPeakGreedyAscent(input);
    if (peak != NOT_FOUND)
    {
        cout << "greedy ascent peak: " << peak << endl;
    }

    peak = find2DPeakDivideConquer(input);
    if (peak != NOT_FOUND)
    {
        cout << "greedy ascent peak: " << peak << endl;
    }
}

#if 0

    peak = find2DPeakGreedyAscent(input2d, size2d);
    if (peak != NOT_FOUND)
    {
        LOG("greedy ascent peak: %d\n", peak);
    }

    peak = find2DPeakDivideConquer(input2d, size2d, {0, 0});
    if (peak != NOT_FOUND)
    {
        LOG("divide conquer peak: %d\n", peak);
    }
    free(input2d);
#endif