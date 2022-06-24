#define CL_TARGET_OPENCL_VERSION 210
#define PROGRAM_FILE "kernels.cl"
#define KERNEL_FUNC "convolution"

#define TILE_SIZE (16U)
#define K_SIZE (3U)

#include <CL/cl.h>
#include <assert.h>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>

#include "./pgm.hpp"

int
main (void)
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

    // read input & init output
    pgm_t src_pgm ("./input.pgm");
    pgm_t dst_pgm (src_pgm.width (), src_pgm.height ());

    // get device
    err = clGetPlatformIDs (1, &platform, NULL);
    assert (err == CL_SUCCESS);
    err = clGetDeviceIDs (platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    assert (err == CL_SUCCESS);

    // create context
    context = clCreateContext (NULL, 1, &device, NULL, NULL, &err);
    assert (err == CL_SUCCESS);

    // read program file
    program_handle = fopen (PROGRAM_FILE, "r");
    assert (program_handle != NULL);
    fseek (program_handle, 0, SEEK_END);
    program_size = ftell (program_handle);
    rewind (program_handle);
    program_buffer = (char *)malloc (program_size + 1);
    program_buffer[program_size] = '\0';
    fread (program_buffer, sizeof (char), program_size, program_handle);
    fclose (program_handle);

    // create program
    program = clCreateProgramWithSource (
        context, 1, (const char **)&program_buffer, &program_size, &err);
    assert (err == CL_SUCCESS);
    free (program_buffer);

    // build program
    char build_flags[100];
    sprintf (build_flags, "-DWIDTH=%u -DHEIGHT=%u -DTILE_SIZE=%u -DK_SIZE=%u",
             src_pgm.width (), src_pgm.height (), TILE_SIZE, K_SIZE);
    err = clBuildProgram (program, 0, NULL, build_flags, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        clGetProgramBuildInfo (program, device, CL_PROGRAM_BUILD_LOG, 0, NULL,
                               &log_size);
        program_log = (char *)malloc (log_size + 1);
        program_log[log_size] = '\0';
        clGetProgramBuildInfo (program, device, CL_PROGRAM_BUILD_LOG,
                               log_size + 1, program_log, NULL);
        printf ("%s\n", program_log);
        free (program_log);
        assert (0);
    }

    // create kernel
    kernel = clCreateKernel (program, KERNEL_FUNC, &err);
    assert (err == CL_SUCCESS);

    // create buffers
    src_buff = clCreateBuffer (
        context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
        src_pgm.width () * src_pgm.height () * sizeof (uint8_t), NULL, &err);
    assert (err == CL_SUCCESS);
    dst_buff = clCreateBuffer (
        context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
        dst_pgm.width () * dst_pgm.height () * sizeof (uint8_t), NULL, &err);

    assert (err == CL_SUCCESS);

    // set kernel args
    err = clSetKernelArg (kernel, 0, sizeof (cl_mem), &src_buff);
    err |= clSetKernelArg (kernel, 1, sizeof (cl_mem), &dst_buff);
    assert (err == CL_SUCCESS);

    // create command queue
    queue = clCreateCommandQueueWithProperties (context, device, NULL, &err);
    assert (err == CL_SUCCESS);

    for (int i = 0; i < 10; i++)
    {
        auto begin = std::chrono::high_resolution_clock::now ();

        // write input
        cl_uchar *src_buff_ptr = (cl_uchar *)clEnqueueMapBuffer (
            queue, src_buff, CL_TRUE, CL_MAP_WRITE, 0,
            src_pgm.width () * src_pgm.height () * sizeof (uint8_t), 0, NULL,
            NULL, &err);
        assert (err == CL_SUCCESS);
        memcpy (src_buff_ptr, src_pgm.ptr (),
                src_pgm.width () * src_pgm.height () * sizeof (uint8_t));
        err = clEnqueueUnmapMemObject (queue, src_buff, (void *)src_buff_ptr,
                                       0, NULL, NULL);
        assert (err == CL_SUCCESS);

        // NDRange
        size_t global_dim[2]
            = { dst_pgm.width () / TILE_SIZE, dst_pgm.height () / TILE_SIZE };
        err = clEnqueueNDRangeKernel (queue, kernel, 2, NULL,
                                      (const size_t *)&global_dim, NULL, 0,
                                      NULL, NULL);
        assert (err == CL_SUCCESS);
        err = clFinish (queue);
        assert (err == CL_SUCCESS);

        // read output
        cl_uchar *dst_buff_ptr = (cl_uchar *)clEnqueueMapBuffer (
            queue, dst_buff, CL_TRUE, CL_MAP_READ, 0,
            dst_pgm.width () * dst_pgm.height () * sizeof (uint8_t), 0, NULL,
            NULL, &err);
        assert (err == CL_SUCCESS);
        memcpy (dst_pgm.ptr (), dst_buff_ptr,
                dst_pgm.width () * dst_pgm.height () * sizeof (uint8_t));
        err = clEnqueueUnmapMemObject (queue, dst_buff, (void *)dst_buff_ptr,
                                       0, NULL, NULL);
        assert (err == CL_SUCCESS);
        clWaitForEvents (1, &event);
        auto end = std::chrono::high_resolution_clock::now ();
        auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds> (
            end - begin);
        printf ("runtime %f ms\n", elapsed.count () * 1e-6);
    }

    // write output
    dst_pgm.write ("./output.pgm");

    // deinit
    clReleaseMemObject (src_buff);
    clReleaseMemObject (dst_buff);
    clReleaseKernel (kernel);
    clReleaseCommandQueue (queue);
    clReleaseProgram (program);
    clReleaseContext (context);

    return 0;
}