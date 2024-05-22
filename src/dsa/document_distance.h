// ************************************************
// PRAGMAS
// ************************************************
#pragma once

// ************************************************
// INCLUDES
// ************************************************
#include <stdint.h>

// ************************************************
// MACROS
// ************************************************

// ************************************************
// TYPEDEF & ENUMS
// ************************************************
typedef struct
{
    char *line;
    char *words;
    uint32_t *frequency;
    size_t num_words;
    size_t max_word_size;
    size_t max_num_words;
} document;

// ************************************************
// FUNCTION DECLARATIONS
// ************************************************
void splitDocument(document *document);
void countWordFrequencies(document *document);
uint32_t computeDotProduct(document *document1, document *document2);