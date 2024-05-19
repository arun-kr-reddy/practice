// ************************************************
// PRAGMAS
// ************************************************
#pragma once

// ************************************************
// INCLUDES
// ************************************************
#include "peak_finding.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>

// ************************************************
// MACROS
// ************************************************
#define RAND_BOUND_MAX 255U
// ************************************************
// TYPEDEF & ENUMS
// ************************************************

// ************************************************
// FUNCTION DECLARATIONS
// ************************************************
static point2d_t findMatrixColumnMax(vector<vector<uint32_t>> input, point2d_t position);

// ************************************************
// FUNCTION DEFINITIONS
// ************************************************
uint32_t rand_bounded()
{
    return (std::rand() % RAND_BOUND_MAX);
}

uint32_t find1DPeakStraightforward(vector<uint32_t> input)
{
    size_t size = input.size();

    // check first & last elements first
    if (input.at(0) > input.at(1))
    {
        return input.at(0);
    }
    else if (input.at(size - 1) > input.at(size - 2))
    {
        return input.at(size - 1);
    }

    // check remaining ones
    for (size_t i = 1; i < size - 2; ++i)
    {
        uint32_t left_value   = input.at(i - 1);
        uint32_t centre_value = input.at(i);
        uint32_t right_value  = input.at(i + 1);

        if ((centre_value >= left_value) && (centre_value >= right_value))
        {
            return centre_value;
        }
    }

    return NOT_FOUND;
}

uint32_t find1DPeakDivideConquer(vector<uint32_t> input)
{
    size_t size = input.size();

    size_t midpoint  = size / 2;
    size_t new_start = 0;
    size_t new_end   = size;

    // check first & last elements first
    if (input.at(0) > input.at(1))
    {
        return input.at(0);
    }
    else if (input.at(size - 1) > input.at(size - 2))
    {
        return input.at(size - 1);
    }

    if (size > 2)
    {
        for (size_t i = 0; i < input.size(); i++)
        {
            std::cout << std::setw(4);
            cout << input.at(i) << " ";
        }
        cout << endl;

        uint32_t left_value   = input.at(midpoint - 1);
        uint32_t centre_value = input.at(midpoint);
        uint32_t right_value  = input.at(midpoint + 1);

        if (right_value > centre_value) // then check right
        {
            new_start = midpoint;
        }
        else if (left_value > centre_value) // check left first
        {
            new_end = midpoint + 1;
        }
        else // midpoint is the peak
        {
            return input.at(midpoint);
        }
    }
    else
    {
        return NOT_FOUND;
    }

    // search peak in new subarray
    vector<uint32_t> new_vector(input.begin() + new_start, input.begin() + new_end);
    return (find1DPeakDivideConquer(new_vector));
}

uint32_t find2DPeakGreedyAscent(vector<vector<uint32_t>> input)
{
    point2d_t size     = {input.size(), input.at(0).size()};
    point2d_t position = {size.x / 2, size.y / 2};

    while (1)
    {
        uint32_t centre_value = input.at(position.x).at(position.y);
        uint32_t left_value, right_value, up_value, down_value;

        // init all neighbors
        left_value = right_value = up_value = down_value = INVALID;

        // check for edges
        if (position.y > 0)
            left_value = input.at(position.x).at(position.y - 1);
        else if (position.y < (size.y - 1))
            right_value = input.at(position.x).at(position.y + 1);
        if (position.x > 0)
            up_value = input.at(position.x - 1).at(position.y);
        else if (position.x < (size.x - 1))
            down_value = input.at(position.x + 1).at(position.y);

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
                return centre_value;
            }
        }
        else
        {
            return NOT_FOUND;
        }
    }

    return NOT_FOUND;
}

uint32_t find2DPeakDivideConquer(vector<vector<uint32_t>> input)
{

    for (size_t row = 0; row < input.size(); row++)
    {
        for (size_t col = 0; col < input.at(row).size(); col++)
        {
            std::cout << std::setw(4);
            cout << input.at(row).at(col) << " ";
        }
        cout << endl;
    }

    point2d_t size     = {input.size(), input.at(0).size()};
    point2d_t position = {size.x / 2, size.y / 2};

    if (size.x > 1 && size.y > 1)
    {
        uint32_t row = position.x;
        position     = findMatrixColumnMax(input, position);
        cout << "max in column " << position.y << " is " << input.at(position.x).at(position.y) << endl;

        uint32_t centre_value = input.at(position.x).at(position.y);
        uint32_t left_value, right_value, up_value, down_value;

        // init all neighbors
        left_value = right_value = up_value = down_value = INVALID;

        // check for edges
        if (position.y > 0)
            left_value = input.at(position.x).at(position.y - 1);
        else if (position.y < (size.y - 1))
            right_value = input.at(position.x).at(position.y + 1);

        // compare to neighbors
        if (left_value > centre_value) // check left first
        {
            position.y = 0;
        }
        else if (right_value > centre_value) // then check right
        {
            position.y = size.y / 2 - 1;
        }
        else // midpoint is the peak
        {
            return centre_value;
        }

        // vector<vector<uint32_t>> new_vector(input.at(0).begin() + position.y, input.at(input.size() - 1).end());
        vector<vector<uint32_t>> new_vector(size.x, vector<uint32_t>(size.y / 2 + 1));
        for (size_t row = 0; row < new_vector.size(); row++)
        {
            for (size_t col = 0; col < new_vector.at(row).size(); col++)
            {
                new_vector.at(row).at(col) = input.at(row).at(position.y + col);
            }
        }

        return find2DPeakDivideConquer(new_vector);
    }
    else
    {
        return NOT_FOUND;
    }

    return NOT_FOUND;
}

static point2d_t findMatrixColumnMax(vector<vector<uint32_t>> input, point2d_t position)
{
    point2d_t max_position = {0, position.y};
    for (size_t i = 1; i < input.at(0).size(); i++)
    {
        if (input.at(i).at(position.y) > input.at(max_position.x).at(position.y))
        {
            max_position.x = i;
        }
    }

    return max_position;
}