#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

#include "args.h"

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