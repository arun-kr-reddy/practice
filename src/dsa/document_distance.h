// ************************************************
// PRAGMAS
// ************************************************
#pragma once

// ************************************************
// INCLUDES
// ************************************************
#include "dsa.h"

// ************************************************
// MACROS
// ************************************************

// ************************************************
// TYPEDEF & ENUMS
// ************************************************

// ************************************************
// FUNCTION DECLARATIONS
// ************************************************
int32_t find1DPeakStraightforward(uint32_t *arr, size_t size);
int32_t find1DPeakDivideConquer(uint32_t *arr, size_t size);
int32_t find2DPeakGreedyAscent(uint32_t *arr, point2d_t size);
int32_t find2DPeakDivideConquer(uint32_t *arr, point2d_t size, point2d_t position);
point2d_t findMatrixColumnMax(uint32_t *arr, point2d_t size, point2d_t position);

// ************************************************
// FUNCTION DEFINITIONS
// ************************************************
int32_t find1DPeakStraightforward(uint32_t *arr, size_t size)
{
    // check first & last elements first
    if (arr[0] > arr[1])
    {
        return arr[0];
    }
    else if (arr[size - 1] > arr[size - 2])
    {
        return arr[size - 1];
    }

    // check remaining ones
    for (int i = 1; i < size - 2; ++i)
    {
        uint32_t left_value   = arr[i - 1];
        uint32_t centre_value = arr[i];
        uint32_t right_value  = arr[i + 1];

        if ((centre_value >= left_value) && (centre_value >= right_value))
        {
            return centre_value;
        }
    }

    return NOT_FOUND;
}

int32_t find1DPeakDivideConquer(uint32_t *arr, size_t size)
{
    size_t midpoint  = size / 2;
    size_t new_start = 0;
    size_t new_end   = size;

    // printArray(arr, size);

    if (size > 1)
    {
        uint32_t left_value   = arr[midpoint - 1];
        uint32_t centre_value = arr[midpoint];
        uint32_t right_value  = arr[midpoint + 1];

        if (left_value > centre_value) // check left first
        {
            new_end = midpoint;
        }
        else if (right_value > centre_value) // then check right
        {
            new_start = midpoint;
        }
        else // midpoint is the peak
        {
            return arr[midpoint];
        }
    }
    else
    {
        return NOT_FOUND;
    }

    // search peak in new subarray
    return (find1DPeakDivideConquer(arr + new_start, new_end - new_start));
}

int32_t find2DPeakGreedyAscent(uint32_t *arr, point2d_t size)
{
    point2d_t position = {size.x / 2, size.y / 2};

    while (1)
    {
        int32_t centre_value = arr[position.x * size.x + position.y];
        int32_t left_value, right_value, up_value, down_value;

        // init all neighbors
        left_value = right_value = up_value = down_value = INVALID;

        // check for edges
        if (position.y > 0)
            left_value = arr[position.x * size.x + (position.y - 1)];
        else if (position.y < (size.y - 1))
            right_value = arr[position.x * size.x + (position.y + 1)];
        if (position.x > 0)
            up_value = arr[(position.x - 1) * size.x + position.y];
        else if (position.x < (size.x - 1))
            down_value = arr[(position.x + 1) * size.x + position.y];

        // compare to neighbors
        if (size.x > 1 && size.y > 1)
        {
            if (left_value > centre_value) // check left first
            {
                position.y--;
            }
            else if (right_value > centre_value) // then check right
            {
                position.y++;
            }
            else if (up_value > centre_value) // then check up
            {
                position.x--;
            }
            else if (down_value > centre_value) // then check down
            {
                position.x++;
            }
            else // midpoint is the peak
            {
                return arr[position.x * size.x + position.y];
            }
        }
        else
        {
            return NOT_FOUND;
        }
    }

    return NOT_FOUND;
}

int32_t find2DPeakDivideConquer(uint32_t *arr, point2d_t size, point2d_t position)
{
    point2d_t new_position = {(position.x + size.x) / 2, (position.y + size.y) / 2};
    int32_t peak           = INVALID;

    if (size.x > 1 && size.y > 1)
    {
        new_position = findMatrixColumnMax(arr, size, new_position);
        LOG("max in column %d is %d\n", new_position.y, arr[new_position.x * size.y + new_position.y]);

        return find1DPeakDivideConquer(arr + new_position.x * size.y, size.y);
    }
    else
    {
        return NOT_FOUND;
    }

    return NOT_FOUND;
}

point2d_t findMatrixColumnMax(uint32_t *arr, point2d_t size, point2d_t position)
{
    uint32_t *base         = arr + position.y;
    point2d_t max_position = {0, position.y};
    for (int32_t i = 1; i < size.x; i++)
    {
        if (base[i * size.y] > base[max_position.x * size.y])
        {
            max_position.x = i;
        }
    }
    return max_position;
}