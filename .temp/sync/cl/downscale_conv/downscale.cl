#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

#include "args.h"

__kernel void downscale(__global unsigned char* src_buff,
                        __global unsigned char* dst_buff,
                        __global const_args* args)
{
    int Row = get_global_id(0); // horz
    int Col = get_global_id(1); // vert

    // init
    int iWidth = args->src_width;
    int iHeight = args->src_height;
    int OutWidth = args->dst_width;
    int OutHeight = args->dst_height;

    float x_ratio = ((float)(iWidth - 1)) / OutWidth;
    float y_ratio = ((float)(iHeight - 1)) / OutHeight;

    // processing
    int x = (int)(x_ratio * Col);
    int y = (int)(y_ratio * Row);
    float x_diff = (x_ratio * Col) - x;
    float y_diff = (y_ratio * Row) - y;
    int index = y * iWidth + x;
    int A = src_buff[index] & 0xff;
    int B = src_buff[index + 1] & 0xff;
    int C = src_buff[index + iWidth] & 0xff;
    int D = src_buff[index + iWidth + 1] & 0xff;
    char Pixel = (char)( A * (1 - x_diff) * (1 - y_diff) 
                        + B * (x_diff) * (1 - y_diff) 
                        + C * (y_diff) * (1 - x_diff) 
                        + D * (x_diff * y_diff));
    dst_buff[(Row * OutWidth) + Col] = Pixel;
}

__kernel void convolution(__global unsigned char* src_buff,
                        __global unsigned char* dst_buff,
                        __global const_args* args)
{
    int y = get_global_id(0); // vert
    int x = get_global_id(1); // horz

    // init
    int width = args->dst_width;
    int height = args->dst_height;

    int ker_size = 9;
    int ker_half_size = (ker_size - 1)/2;
    unsigned int sum = 0;
    for(int ker_y = -ker_half_size ; ker_y <= ker_half_size ; ker_y++)
    {
        for(int ker_x = -ker_half_size ; ker_x <= ker_half_size ; ker_x++)
        {
            int pos_y = y + ker_y;
            int pos_x = x + ker_x;
            if((pos_x >= 0) || (pos_x < width) || (pos_y >= 0) || (pos_y < height))
            {
                sum += src_buff[pos_y * width + pos_x];
                //printf("%d %d %d\n", pos_x, pos_y, src_buff[pos_y * width + pos_x]);
            }
        }
    }
    dst_buff[y * width + x] = (unsigned char)(sum / (ker_size * ker_size));
}