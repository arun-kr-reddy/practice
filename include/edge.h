#pragma once

#include "pgm.h"

void apply_edgeX(pgm_t *src_img, pgm_t *dst_img, uint8_t threshold) {
    int width = src_img->width;
    int height = src_img->height;
    uint8_t *src_buff = src_img->ptr;
    uint8_t *dst_buff = dst_img->ptr;

    uint8_t k_size = 3;
    uint8_t k_half_size = (k_size - 1) / 2;
    int8_t kernel[9] = {1, 0, -1, 1, 0, -1, 1, 0, -1};

    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            int16_t sum = 0;
            for (int k_y = -k_half_size; k_y <= k_half_size; k_y++) {
                for (int k_x = -k_half_size; k_x <= k_half_size; k_x++) {
                    int pos_y = y + k_y;
                    int pos_x = x + k_x;
                    // printf("%d %d %d %d\n", y, x, pos_y, pos_x);
                    if ((pos_x >= 0) && (pos_x < width) && (pos_y >= 0) &&
                        (pos_y < height)) {
                        sum += src_buff[pos_y * width + pos_x] *
                               kernel[(k_y + k_half_size) * k_size +
                                      (k_x + k_half_size)];
                    }
                }
            }
            sum /= (k_size * 2);
            sum += 127;
            dst_buff[y * width + x] = (sum > threshold) ? ((uint8_t)sum) : (0);
        }
    }
}

void apply_edgeY(pgm_t *src_img, pgm_t *dst_img, uint8_t threshold) {
    int width = src_img->width;
    int height = src_img->height;
    uint8_t *src_buff = src_img->ptr;
    uint8_t *dst_buff = dst_img->ptr;

    uint8_t k_size = 3;
    uint8_t k_half_size = (k_size - 1) / 2;
    int8_t kernel[9] = {1, 1, 1, 0, 0, 0, -1, -1, -1};

    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            int16_t sum = 0;
            for (int k_y = -k_half_size; k_y <= k_half_size; k_y++) {
                for (int k_x = -k_half_size; k_x <= k_half_size; k_x++) {
                    int pos_y = y + k_y;
                    int pos_x = x + k_x;
                    // printf("%d %d %d %d\n", y, x, pos_y, pos_x);
                    if ((pos_x >= 0) && (pos_x < width) && (pos_y >= 0) &&
                        (pos_y < height)) {
                        sum += src_buff[pos_y * width + pos_x] *
                               kernel[(k_y + k_half_size) * k_size +
                                      (k_x + k_half_size)];
                    }
                }
            }
            sum /= (k_size * 2);
            sum += 127;
            dst_buff[y * width + x] = (sum > threshold) ? ((uint8_t)sum) : (0);
        }
    }
}