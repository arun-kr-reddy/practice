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