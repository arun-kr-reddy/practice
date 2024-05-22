// ************************************************
// PRAGMAS
// ************************************************
#pragma once

// ************************************************
// INCLUDES
// ************************************************
#include "document_distance.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void splitDocument(document *document)
{
    // take a local copy of string
    char *string = (char *)calloc(strlen(document->line) + 1, sizeof(char));
    if (!string)
        assert(0);

    strcpy(string, document->line);

    // parse out words from line
    size_t pos = 0;
    char *word = strtok(string, " ,.-");
    while (word != NULL && (pos < document->max_word_size * document->max_num_words))
    {
        strcpy(&document->words[pos], word);
        pos += document->max_word_size;
        document->num_words++;

        word = strtok(NULL, " ,.-");
    }

    // free local buffer
    free(string);
}

void countWordFrequencies(document *document)
{
    // initialize frequency
    for (size_t i = 0; i < document->num_words; i++)
    {
        document->frequency[i] = 1;
    }

    // check for frequencies by checking for duplicates
    for (size_t curr = 0; curr < document->num_words; curr++)
    {
        for (size_t search = curr + 1; search < document->num_words; search++)
        {
            if (!strcmp(&document->words[curr * document->max_word_size], &document->words[search * document->max_word_size]))
            {
                document->frequency[search] = 0;
                document->frequency[curr]++;
                document->words[search * document->max_word_size] = 0;
                break;
            }
        }
    }
}

uint32_t computeDotProduct(document *document1, document *document2)
{
    uint32_t dot_product = 0;

    for (size_t curr = 0; curr < document1->num_words; curr++)
    {
        for (size_t ref = 0; ref < document2->num_words; ref++)
        {
            if (!strcmp(&document1->words[curr * document1->max_word_size], &document2->words[ref * document2->max_word_size]))
            {
                dot_product += document1->frequency[curr] * document2->frequency[ref];
                break;
            }
        }
    }

    return dot_product;
}