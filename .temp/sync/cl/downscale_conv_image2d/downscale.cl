#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

#include "args.h"

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

__kernel void downscale(__read_only image2d_t src_image,
                        __write_only image2d_t dst_image,
                        __global const_args* args)
{
    int y_dst = get_global_id(1); // vert
    int x_dst = get_global_id(0); // horz

    // init
    int width_src = args->src_width;
    int height_src = args->src_height;
    int width_dst = args->dst_width;
    int height_dst = args->dst_height;

    float x_ratio = ((float)(width_src - 1)) / width_dst;
    float y_ratio = ((float)(height_src - 1)) / height_dst;

    // processing
    float x_src = (float)(x_ratio * x_dst);
    float y_src = (float)(y_ratio * y_dst);
    uint4 color;
    color.x = read_imageui(src_image, sampler,(float2)(x_src,y_src)).x;
    write_imageui(dst_image, (int2)(x_dst,y_dst), color);
}

__kernel void convolution(__read_only image2d_t src_image,
                        __write_only image2d_t dst_image,
                        __global const_args* args)
{
    int y = get_global_id(1); // vert
    int x = get_global_id(0); // horz

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
                sum += read_imageui(src_image, sampler,(int2)(pos_x,pos_y)).x;
                //printf("%d %d %d\n", pos_x, pos_y, src_buff[pos_y * width + pos_x]);
            }
        }
    }
    //dst_buff[y * width + x] = (unsigned char)(sum / (ker_size * ker_size));
   //dst_buff[y * width + x] = read_imageui(src_image, sampler,(int2)(x,y)).x;
   uint4 color;
   color.x =  (unsigned char)(sum / (ker_size * ker_size));
   write_imageui(dst_image, (int2)(x,y), color);
}