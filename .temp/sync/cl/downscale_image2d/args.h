#ifndef __ARGS_H
#define __ARGS_H

typedef struct __attribute__ ((packed))
{
    int     src_width;
    int     src_height;
    int     dst_width;
    int     dst_height;
}const_args;

#endif