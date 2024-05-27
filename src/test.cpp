// ************************************************
// INCLUDES
// ************************************************
#include "document_distance.h"
#include "peak_finding.h"
#include "sorting.h" a
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// ************************************************
// MACROS
// ************************************************
#define MAX_INPUT_SIZE_PEAKFINDING 10U
#define MAX_LINE_SIZE              100U
#define MAX_WORD_SIZE              20U
#define MAX_NUM_WORDS              20U

// ************************************************
// TYPEDEF & ENUMS
// ************************************************

// ************************************************
// FUNCTION DECLARATIONS
// ************************************************
void peakFinding1D();
void peakFinding2D();
void calculateDocumentDistance();
void sorting();

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

    printf("===================== document distance ===================\n");
    calculateDocumentDistance();
    printf("==========================================================\n");

    printf("===================== sorting ============================\n");
    sorting();
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

void calculateDocumentDistance()
{
    document doc1 = {NULL, NULL, NULL, 0, MAX_WORD_SIZE, MAX_NUM_WORDS};
    document doc2 = {NULL, NULL, NULL, 0, MAX_WORD_SIZE, MAX_NUM_WORDS};

    doc1.words     = (char *)calloc(MAX_NUM_WORDS, doc1.max_word_size);
    doc1.line      = (char *)calloc(MAX_LINE_SIZE, sizeof(char));
    doc1.frequency = (uint32_t *)calloc(MAX_NUM_WORDS, sizeof(uint32_t));

    doc2.words     = (char *)calloc(MAX_NUM_WORDS, doc2.max_word_size);
    doc2.line      = (char *)calloc(MAX_LINE_SIZE, sizeof(char));
    doc2.frequency = (uint32_t *)calloc(MAX_NUM_WORDS, sizeof(uint32_t));

    if (!doc1.line || !doc2.line || !doc1.words || !doc2.words)
        assert(0);

    strcpy(doc1.line, "why are we waiting here");
    strcpy(doc2.line, "we are waiting here for the bus");

    splitDocument(&doc1);
    splitDocument(&doc2);

    countWordFrequencies(&doc1);
    countWordFrequencies(&doc2);

    uint32_t document_distance = computeDotProduct(&doc1, &doc2);

    printf("first string: \"%s\"\n", doc1.line);
    printf("second string: \"%s\"\n", doc2.line);
    printf("document distance: %d\n", document_distance);
}

void sorting()
{
    array_t input = {0, MAX_INPUT_SIZE_PEAKFINDING};
    array_t copy  = input;

    input.addr = (uint8_t *)malloc(input.size);
    copy.addr  = (uint8_t *)malloc(input.size);

    if ((NULL == input.addr) || (NULL == copy.addr))
        assert(0);

    fillArray(input);
    memcpy(copy.addr, input.addr, input.size);

    printArray(input);

    insertionSort(input);
    printf("insertion sorted: ");
    printArray(input);

    memcpy(input.addr, copy.addr, input.size);
    binaryInsertionSort(input);
    printf("binary insertion sorted: ");
    printArray(input);

    free(input.addr);
    free(copy.addr);
}