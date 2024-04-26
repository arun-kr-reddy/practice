// includes
#include <algorithm>
#include <cstdbool>
#include <ctime>
#include <iostream>
#include <vector>

// macros
#define MAX_INPUT_SIZE 100

// using
using std::cout;
using std::endl;
using std::vector;

// global

// function prototypes
uint32_t fillRandomNumbers(std::vector<uint32_t> &vec);
uint32_t findPeak(std::vector<uint32_t> input);

// functions
int main()
{
    vector<uint32_t> input(MAX_INPUT_SIZE);
    fillRandomNumbers(input);

    findPeak(input);

    return 0;
}

uint32_t fillRandomNumbers(vector<uint32_t> &vec)
{
    for (uint32_t i = 0; i < vec.size(); ++i)
    {
        vec.at(i) = 1;  // rand();
    }

    return 0;
}

uint32_t findPeak(std::vector<uint32_t> input)
{
    uint32_t half_size = input.size() / 2;
    if (input.at(half_size - 1) > input.at(half_size))
    {
        std::vector<uint32_t> new_input{input.begin(),
                                        input.begin() + half_size};
        findPeak(new_input);
    }
    else if (input.at(half_size + 1) > input.at(half_size))
    {
        std::vector<uint32_t> new_input{input.begin() + half_size, input.end()};
        findPeak(new_input);
    }
    else
    {
        cout << input.at(half_size - 1) << " " << input.at(half_size) << " "
             << input.at(half_size + 1) << " " << endl;
    }

    return 0;
}