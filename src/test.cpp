// includes
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// macros
#define MAX_INPUT_SIZE 10
#define MAX_VALUE 100

#define ERROR -1

#define DBG 1

// global

// function prototypes
void fillRandomNumbers(uint32_t* arr, size_t size);
void printArray1D(uint32_t* arr, size_t size);

uint32_t findPeak1D_1(uint32_t* arr, size_t size);
uint32_t findPeak1D_2(uint32_t* arr, size_t size);

// functions
int main() {
  uint32_t* input = (uint32_t*)malloc(MAX_INPUT_SIZE * sizeof(uint32_t));
  uint32_t peak = ERROR;

  fillRandomNumbers(input, MAX_INPUT_SIZE);

#if DBG
  printf("input array: ");
  printArray1D(input, MAX_INPUT_SIZE);
#endif

  peak = findPeak1D_1(input, MAX_INPUT_SIZE);
  if (peak != ERROR) {
    printf("peak: %d\n", peak);
  } else {
    printf("no peak \n");
  }

  peak = findPeak1D_2(input, MAX_INPUT_SIZE);
  if (peak != ERROR) {
    printf("peak: %d\n", peak);
  } else {
    printf("no peak \n");
  }

  return 0;
}

void fillRandomNumbers(uint32_t* arr, size_t size) {
  for (uint32_t i = 0; i < size; ++i) {
    arr[i] = rand() % MAX_VALUE;
  }

  return;
}

void printArray1D(uint32_t* arr, size_t size) {
  for (uint32_t i = 0; i < size; ++i) {
    printf("%d ", arr[i]);
  }
  printf("\n");
}

uint32_t findPeak1D_1(uint32_t* arr, size_t size) {
  if (arr[0] > arr[1])
    return arr[0];
  else if (arr[size - 1] > arr[size - 2])
    return arr[size - 1];

  for (int i = 1; i < size - 2; ++i) {
    if ((arr[i] >= arr[i - 1]) && (arr[i] >= arr[i + 1])) return arr[i];
  }

  return ERROR;
}

uint32_t findPeak1D_2(uint32_t* arr, size_t size) {
  size_t half_size = size / 2;
  size_t new_start = 0;
  size_t new_end = size;

#if DBG
  printArray1D(arr, size);
#endif

  if (size > 1) {
    if (arr[half_size - 1] > arr[half_size]) {
      new_end = half_size;
    } else if (arr[half_size + 1] > arr[half_size]) {
      new_start = half_size;
    } else {
      return arr[half_size];
    }
  } else {
    return ERROR;
  }

  return (findPeak1D_2(arr + new_start, new_end - new_start));
}