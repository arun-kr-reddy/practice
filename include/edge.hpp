#pragma once

#include <cmath>

#include "enums.hpp"
#include "pgm.hpp"

void edgeX(pgm_t &src_img, pgm_t &dst_img, edge_e edge, uint8_t threshold) {
    int width = src_img.width();
    int height = src_img.height();
    uint8_t *src_ptr = src_img.ptr();
    uint8_t *dst_ptr = dst_img.ptr();

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
                    if ((pos_x >= 0) && (pos_x < width) && (pos_y >= 0) && (pos_y < height)) {
                        sum += src_ptr[pos_y * width + pos_x] *
                               kernel[(k_y + k_half_size) * k_size + (k_x + k_half_size)];
                    } else if (edge == clamp) {
                        if (pos_x < 0) {
                            pos_x = 0;
                        } else if (pos_x >= width) {
                            pos_x = width - 1;
                        }

                        if (pos_y < 0) {
                            pos_y = 0;
                        } else if (pos_y >= height) {
                            pos_y = height - 1;
                        }

                        sum += src_ptr[pos_y * width + pos_x] *
                               kernel[(k_y + k_half_size) * k_size + (k_x + k_half_size)];
                    } else if (edge == mirror) {
                        if (pos_x < 0) {
                            pos_x = -(pos_x);
                        } else if (pos_x >= width) {
                            pos_x = width - (pos_x - width) - 1;
                        }

                        if (pos_y < 0) {
                            pos_y = -(pos_y);
                        } else if (pos_y >= height) {
                            pos_y = height - (pos_y - height) - 1;
                        }

                        sum += src_ptr[pos_y * width + pos_x] *
                               kernel[(k_y + k_half_size) * k_size + (k_x + k_half_size)];
                    }
                }
            }
            sum /= (k_size * 2);
            sum += 127;
            dst_ptr[y * width + x] = (sum > threshold) ? ((uint8_t)sum) : (0);
        }
    }
}

void edgeY(pgm_t &src_img, pgm_t &dst_img, edge_e edge, uint8_t threshold) {
    int width = src_img.width();
    int height = src_img.height();
    uint8_t *src_ptr = src_img.ptr();
    uint8_t *dst_ptr = dst_img.ptr();

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
                    if ((pos_x >= 0) && (pos_x < width) && (pos_y >= 0) && (pos_y < height)) {
                        sum += src_ptr[pos_y * width + pos_x] *
                               kernel[(k_y + k_half_size) * k_size + (k_x + k_half_size)];
                    } else if (edge == clamp) {
                        if (pos_x < 0) {
                            pos_x = 0;
                        } else if (pos_x >= width) {
                            pos_x = width - 1;
                        }

                        if (pos_y < 0) {
                            pos_y = 0;
                        } else if (pos_y >= height) {
                            pos_y = height - 1;
                        }

                        sum += src_ptr[pos_y * width + pos_x] *
                               kernel[(k_y + k_half_size) * k_size + (k_x + k_half_size)];
                    } else if (edge == mirror) {
                        if (pos_x < 0) {
                            pos_x = -(pos_x);
                        } else if (pos_x >= width) {
                            pos_x = width - (pos_x - width) - 1;
                        }

                        if (pos_y < 0) {
                            pos_y = -(pos_y);
                        } else if (pos_y >= height) {
                            pos_y = height - (pos_y - height) - 1;
                        }

                        sum += src_ptr[pos_y * width + pos_x] *
                               kernel[(k_y + k_half_size) * k_size + (k_x + k_half_size)];
                    }
                }
            }
            sum /= (k_size * 2);
            sum += 127;
            dst_ptr[y * width + x] = (sum > threshold) ? ((uint8_t)sum) : (0);
        }
    }
}

void edgeRms(pgm_t &edgeX_img, pgm_t &edgeY_img, pgm_t &dst_img, uint8_t threshold) {
    int width = dst_img.width();
    int height = dst_img.height();
    uint8_t *edgeX_ptr = edgeX_img.ptr();
    uint8_t *edgeY_ptr = edgeY_img.ptr();
    uint8_t *dst_ptr = dst_img.ptr();

    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            uint8_t x_val = edgeX_ptr[y * width + x];
            uint8_t y_val = edgeY_ptr[y * width + x];

            uint8_t rms = sqrt((x_val * x_val + y_val * y_val) / 2);
            dst_ptr[y * width + x] = (rms > threshold) ? ((uint8_t)rms) : (0);
        }
    }
}