#pragma once

#include "pgm.h"

typedef struct {
    uint32_t freq[255];
    uint32_t cum_freq[255];
    double cum_prob[255];
    uint32_t num_samples;
} histogram_t;

void calc_histogram(pgm_t *img, histogram_t *hist) {
    memset(hist->freq, 0, sizeof(hist->freq));
    hist->num_samples = img->width * img->height;
    for (uint32_t i = 0; i < hist->num_samples; i++) {
        uint8_t val = img->ptr[i];
        hist->freq[val]++;
    }
}

void calc_cum_freq(histogram_t *hist) {
    memset(hist->cum_freq, 0, sizeof(hist->freq));
    uint32_t cum_sum = 0;
    for (uint32_t i = 0; i < 255; i++) {
        cum_sum += hist->freq[i];
        hist->cum_freq[i] = cum_sum;
        hist->cum_prob[i] = cum_sum / (double)(hist->num_samples);
    }
}

void apply_hist_eq(pgm_t *img, histogram_t *hist) {
    for (uint32_t i = 0; i < hist->num_samples; i++) {
        uint8_t val = img->ptr[i];
        uint8_t eq_val = val * hist->cum_prob[val];
        img->ptr[i] = eq_val;
    }
}