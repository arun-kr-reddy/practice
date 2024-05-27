// ************************************************
// PRAGMAS
// ************************************************
#pragma once

// ************************************************
// INCLUDES
// ************************************************
#include "sorting.h"
#include <stdlib.h>
#include <time.h>

// ************************************************
// MACROS
// ************************************************

// ************************************************
// TYPEDEF & ENUMS
// ************************************************

// ************************************************
// FUNCTION DECLARATIONS
// ************************************************

// ************************************************
// FUNCTION DEFINITIONS
// ************************************************
void insertionSort(array_t array)
{
    for (int init_pos = 1; init_pos < array.size; ++init_pos)
    {
        uint32_t final_pos = init_pos;
        while ((final_pos > 0) && (array.addr[final_pos - 1] > array.addr[final_pos]))
        {
            uint32_t temp             = array.addr[final_pos - 1];
            array.addr[final_pos - 1] = array.addr[final_pos];
            array.addr[final_pos]     = temp;
            final_pos--;
        }
    }

    return;
}

void binaryInsertionSort(array_t array)
{
    for (int init_pos = 1; init_pos < array.size; ++init_pos)
    {
        uint32_t final_pos = init_pos;
        uint32_t start     = 0,
                 end       = init_pos;
        while (start < end)
        {
            uint32_t midpoint   = (start + end) / 2;
            uint32_t centre_val = array.addr[init_pos];

            if ((array.addr[midpoint] <= centre_val) && ((array.addr[midpoint + 1] >= array.addr[init_pos])))
            {
                final_pos = midpoint + 1;
                break;
            }
            else if (array.addr[midpoint] < array.addr[init_pos])
            {
                start = midpoint;
            }
            else if (array.addr[midpoint] > array.addr[init_pos])
            {
                end = midpoint;
            }
        }

        for (int i = init_pos; i > final_pos; --i)
        {
            uint32_t temp     = array.addr[i - 1];
            array.addr[i - 1] = array.addr[i];
            array.addr[i]     = temp;
        }
    }

    return;
}