#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

#include "args.h"

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_LINEAR | CLK_ADDRESS_CLAMP_TO_EDGE;

__kernel void downscale(__read_only image2d_t src_image,
                        __write_only image2d_t dst_image,
                        __global const_args* args)
{
    int Row = get_global_id(0);
    int Col = get_global_id(1);

    // init
    int iWidth = args->src_width;
    int iHeight = args->src_height;
    int OutWidth = args->dst_width;
    int OutHeight = args->dst_height;

    int4 src_pix = read_imagei(src_image, sampler, (int2)(Row, Col));
    write_imagei(dst_image, (int2)(Row, Col), src_pix);
#if 0
    float x_ratio = ((float)(iWidth - 1)) / OutWidth;
    float y_ratio = ((float)(iHeight - 1)) / OutHeight;

    // processing
    int x = (int)(x_ratio * Col);
    int y = (int)(y_ratio * Row);
    float x_diff = (x_ratio * Col) - x;
    float y_diff = (y_ratio * Row) - y;
    int index = y * iWidth + x;
    int A = src_image[index] & 0xff;
    int B = src_image[index + 1] & 0xff;
    int C = src_image[index + iWidth] & 0xff;
    int D = src_image[index + iWidth + 1] & 0xff;
    char Pixel = (char)( A * (1 - x_diff) * (1 - y_diff) 
                        + B * (x_diff) * (1 - y_diff) 
                        + C * (y_diff) * (1 - x_diff) 
                        + D * (x_diff * y_diff));
    dst_image[(Row * OutWidth) + Col] = Pixel;
#endif
}