#define _CRT_SECURE_NO_WARNINGS
#define CL_TARGET_OPENCL_VERSION 210
#define PROGRAM_FILE "downscale.cl"
#define KERNEL_FUNC "downscale"

#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "args.h"
#include "pgm.h"

int main()
{
    // init
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_event event;
    cl_program program;
    FILE *program_handle;
    char *program_buffer, *program_log;
    size_t program_size, log_size;
    cl_kernel kernel;
    cl_int err;
    cl_mem src_buff;
    cl_mem dst_buff;
    cl_mem args_buff;

    // initialize data
    PGMData src_pgm;
    PGMData dst_pgm;
    readPGM("./input.pgm", &src_pgm);
    //writePGM("./input_copy.pgm", &src_pgm);

    const_args args;
    args.src_width = src_pgm.width;
    args.src_height = src_pgm.height;
    args.dst_width = dst_pgm.width = 900;
    args.dst_height = dst_pgm.height = 300;
    dst_pgm.max_gray = src_pgm.max_gray;
    dst_pgm.ptr = (uint8_t *)malloc(dst_pgm.width * dst_pgm.height * sizeof(uint8_t));

    printf("\ninput w:%d h:%d\n", src_pgm.width, src_pgm.height, src_pgm.max_gray);
    printf("output w:%d h:%d\n", dst_pgm.width, dst_pgm.height, dst_pgm.max_gray);

    // get device
    err = clGetPlatformIDs(1, &platform, NULL);
    assert(err == CL_SUCCESS);

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    assert(err == CL_SUCCESS);

    char device_name[50];
    err = clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
    printf("using %s\n", device_name);

    // create context
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    assert(err == CL_SUCCESS);

    // read program file
    program_handle = fopen(PROGRAM_FILE, "r");
    assert(program_handle != NULL);
    fseek(program_handle, 0, SEEK_END);
    program_size = ftell(program_handle);
    rewind(program_handle);
    program_buffer = (char *)malloc(program_size + 1);
    program_buffer[program_size] = '\0';
    fread(program_buffer, sizeof(char), program_size, program_handle);
    fclose(program_handle);

    // create program
    program = clCreateProgramWithSource(context, 1, (const char **)&program_buffer, &program_size, &err);
    assert(err == CL_SUCCESS);
    free(program_buffer);

    // build program
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        program_log = (char *)malloc(log_size + 1);
        program_log[log_size] = '\0';
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size + 1, program_log, NULL);
        printf("%s\n", program_log);
        free(program_log);
        assert(0);
    }

    // create kernel
    kernel = clCreateKernel(program, KERNEL_FUNC, &err);
    assert(err == CL_SUCCESS);

    // create buffers
    src_buff = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, src_pgm.width * src_pgm.height * sizeof(uint8_t), src_pgm.ptr, &err);
    assert(err == CL_SUCCESS);
    dst_buff = clCreateBuffer(context, CL_MEM_WRITE_ONLY, dst_pgm.width * dst_pgm.height * sizeof(uint8_t), NULL, &err);
    assert(err == CL_SUCCESS);
    args_buff = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(args), &args, &err);
    assert(err == CL_SUCCESS);

    // set kernel args
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &src_buff);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &dst_buff);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &args_buff);
    assert(err == CL_SUCCESS);

    // create command queue
    queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);
    assert(err == CL_SUCCESS);

#if 0
    // enquque input write
    err |= clEnqueueWriteBuffer(queue, src_buff, CL_TRUE, 0, src_pgm.width * src_pgm.height * sizeof(uint8_t), src_pgm.ptr, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(queue, args_buff, CL_TRUE, 0, sizeof(args), &args, 0, NULL, NULL);
    assert(err == CL_SUCCESS);
    err = clFinish(queue);
    assert(err == CL_SUCCESS);
#endif

    // NDRange
    size_t global_dim[2] = {args.dst_height, args.dst_width};
    err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, (const size_t *)&global_dim, NULL, 0, NULL, NULL);
    assert(err == CL_SUCCESS);

    err = clFinish(queue);
    assert(err == CL_SUCCESS);

    // enquque output read
    err = clEnqueueReadBuffer(queue, dst_buff, CL_TRUE, 0, dst_pgm.width * dst_pgm.height * sizeof(uint8_t), dst_pgm.ptr, 0, NULL, &event);
    assert(err == CL_SUCCESS);

    // wait for read buff
    clWaitForEvents(1, &event);
    writePGM("./output.pgm", &dst_pgm);

#if 0
    // profiling
    cl_ulong starttime;
    cl_ulong endtime;
    err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &starttime, NULL);
    err |= clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endtime, NULL);
    assert(err == CL_SUCCESS);
    unsigned long elapsed = (unsigned long)(endtime - starttime);
    printf("executed in \t %lu us\n", elapsed/1000);
#endif

    // deinit
    clReleaseMemObject(src_buff);
    clReleaseMemObject(dst_buff);
    clReleaseMemObject(args_buff);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);

    free(src_pgm.ptr);
    free(dst_pgm.ptr);

    return 0;
}
