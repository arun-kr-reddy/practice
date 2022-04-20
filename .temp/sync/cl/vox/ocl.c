#define _CRT_SECURE_NO_WARNINGS
#define CL_TARGET_OPENCL_VERSION 120
#define PROGRAM_FILE "voxKernel.cl"
#define KERNEL_FUNC "cylindrical"

#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "vox_structs.h"

int main() {

    /* Host/device data structures */
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_int i, err;
    cl_event event;
    /* Program/kernel data structures */
    cl_program program;
    FILE *program_handle;
    char *program_buffer, *program_log;
    size_t program_size, log_size;
    cl_kernel kernel;

    /* Data and buffers */
    uint16_t dispLeft[512000/sizeof(uint16_t)];
    uint8_t dispConf[256000/sizeof(uint8_t)];
    LutMeta lutMeta[8282/sizeof(LutMeta)];
    VoxelPoint voxels[VOXEL_BUFFER_SIZE];
    uint32_t hashIDs[VOXEL_BUFFER_SIZE];
    uint8_t count[VOXEL_BUFFER_SIZE];
    uint32_t numVoxels;
    const_args args;
    cl_mem dispLeft_buff, dispConf_buff, lutMeta_buff, voxels_buff, hashIDs_buff, count_buff, numVoxels_buff, args_buff;
    size_t global_dim[3];

    /* Initialize data to be processed by the kernel */
    VoxelCellData voxelData;
    int32_t roiMinX;
    int32_t roiMaxX;
    int32_t roiMinY;
    int32_t roiMaxY;
    int32_t width;
    int32_t height;
    int32_t dispOffset;
    float32_t startAzim_rad;
    int32_t  gridWidth;
    int32_t  gridHeight;
    int32_t  gridCellWidth;
    int32_t  gridCellHeight;
    int32_t  m_dispScale;
    MOS_VoxelGridSizes_t g_mosVoxelGridSizes;

    FILE *fpIn1, *fpIn2, *fpIn3, * fpIn4;
    fpIn1 = fopen("./dump/pDispLeft_input.bin", "rb");
    fpIn2 = fopen("./dump/pDispConf_input.bin", "rb");
    fpIn3 = fopen("./dump/pLutMeta_input.bin", "rb");
    fpIn4 = fopen("./dump/init_input.bin", "rb");

    fread(dispLeft, 1, 512000, fpIn1);
    fread(dispConf, 1, 256000, fpIn2);
    fread(lutMeta, 1, 8282, fpIn3);

    fread(&voxelData, sizeof(VoxelCellData), 1, fpIn4);
	fread(&roiMinX, sizeof(int32_t), 1, fpIn4);
	fread(&roiMinY, sizeof(int32_t), 1, fpIn4);
	fread(&roiMaxX, sizeof(int32_t), 1, fpIn4);
	fread(&roiMaxY, sizeof(int32_t), 1, fpIn4);
	fread(&width, sizeof(int32_t), 1, fpIn4);
    fread(&height, sizeof(int32_t), 1, fpIn4);
	fread(&startAzim_rad, sizeof(float32_t), 1, fpIn4);
	fread(&gridWidth, sizeof(int32_t), 1, fpIn4);
	fread(&gridHeight, sizeof(int32_t), 1, fpIn4);
	fread(&gridCellWidth, sizeof(int32_t), 1, fpIn4);
	fread(&gridCellHeight, sizeof(int32_t), 1, fpIn4);
	fread(&g_mosVoxelGridSizes, sizeof(MOS_VoxelGridSizes_t), 1, fpIn4);
	fread(&m_dispScale, sizeof(int32_t), 1, fpIn4);
	fread(&dispOffset, sizeof(int32_t), 1, fpIn4);

    fclose(fpIn1);
    fclose(fpIn2);
    fclose(fpIn3);
    fclose(fpIn4);

    printf("W:%d H:%d\n", width, height);
    printf("gW:%d gH:%d\n", gridWidth, gridHeight);

    args.cellIdx            = voxelData.cellIdx;
    args.cellWidth          = voxelData.cellWidth;
    args.cellHeight         = voxelData.cellHeight;
    args.startX             = voxelData.startX;
    args.startY             = voxelData.startY;
    args.lutShiftX          = voxelData.lutShiftX;
    args.lutShiftY          = voxelData.lutShiftY;
    args.lutWidth           = voxelData.lutWidth;
    args.imgWidth           = voxelData.imgWidth;
    args.minDisp            = voxelData.minDisp;
    args.maxDisp            = voxelData.maxDisp;
    args.dispScaleInv       = voxelData.dispScaleInv;
    args.b_mm               = voxelData.b_mm;
    args.startN_px          = voxelData.startN_px;
    args.azimAngle_rad      = voxelData.azimAngle_rad;
    args.azimDelta_rad      = voxelData.azimDelta_rad;
    args.polarAngle_rad     = voxelData.polarAngle_rad;
    args.polarDelta_rad     = voxelData.polarDelta_rad;
    args.cenX_px            = voxelData.cenX_px;
    args.cenY_px            = voxelData.cenY_px;
    args.planarAspect       = voxelData.planarAspect;
    args.offset_mm          = voxelData.offset_mm;
    args.f_px               = voxelData.f_px;
    args.scaleN             = voxelData.scaleN;
    args.scaleR             = voxelData.scaleR;
    args.baselineError_mm   = voxelData.baselineError_mm;
    args.pos_mm_x           = voxelData.pos_mm.x;
    args.pos_mm_y           = voxelData.pos_mm.y;
    args.pos_mm_z           = voxelData.pos_mm.z;
    args.rot_xx             = voxelData.rot.xx;
    args.rot_xy             = voxelData.rot.xy;
    args.rot_xz             = voxelData.rot.xz;
    args.rot_yx             = voxelData.rot.yx;
    args.rot_yy             = voxelData.rot.yy;
    args.rot_yz             = voxelData.rot.yz;
    args.rot_zx             = voxelData.rot.zx;
    args.rot_zy             = voxelData.rot.zy;
    args.rot_zz             = voxelData.rot.zz;

    args.roiMinX            = roiMinX;
    args.roiMaxX            = roiMaxX;
    args.roiMinY            = roiMinY;
    args.roiMaxY            = roiMaxY;
    args.width              = width;
    args.height             = height;
    args.dispOffset         = dispOffset;
    args.startAzim_rad      = startAzim_rad ;
    args.gridWidth          = gridWidth;
    args.gridHeight         = gridHeight;
    args.gridCellWidth      = gridCellWidth;
    args.gridCellHeight     = gridCellHeight;
    args.m_dispScale        = m_dispScale;

    args.mmPerVoxel         = g_mosVoxelGridSizes.mmPerVoxel;
    args.mmPerVoxelInv      = g_mosVoxelGridSizes.mmPerVoxelInv;
    args.voxelGridLength_vx = g_mosVoxelGridSizes.voxelGridLength_vx;
    args.voxelGridWidth_vx  = g_mosVoxelGridSizes.voxelGridWidth_vx;
    args.voxelGridHeight_vx = g_mosVoxelGridSizes.voxelGridHeight_vx;
    args.numVoxels          = g_mosVoxelGridSizes.numVoxels;
    args.voxelGridZmin_mm   = g_mosVoxelGridSizes.voxelGridZmin_mm;
    args.voxelGridZmax_mm   = g_mosVoxelGridSizes.voxelGridZmax_mm;
    args.voxelGridXmax_mm   = g_mosVoxelGridSizes.voxelGridXmax_mm;
    args.voxelGridXmin_mm   = g_mosVoxelGridSizes.voxelGridXmin_mm;
    args.voxelGridYmax_mm   = g_mosVoxelGridSizes.voxelGridYmax_mm;
    args.voxelGridYmin_mm   = g_mosVoxelGridSizes.voxelGridYmin_mm;

    /* Identify a platform */
    err = clGetPlatformIDs(1, &platform, NULL);
    if(err < 0) {
        perror("Couldn't find any platforms");
        exit(1);
    }

    /* Access a device */
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    if(err < 0) {
        perror("Couldn't find any devices");
        exit(1);
    }

    /* Create the context */
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if(err < 0) {
        perror("Couldn't create a context");
        exit(1);   
    }

    /* Read program file and place content into buffer */
    program_handle = fopen(PROGRAM_FILE, "r");
    if(program_handle == NULL) {
        perror("Couldn't find the program file");
        exit(1);   
    }
    fseek(program_handle, 0, SEEK_END);
    program_size = ftell(program_handle);
    rewind(program_handle);
    program_buffer = (char*)malloc(program_size + 1);
    program_buffer[program_size] = '\0';
    fread(program_buffer, sizeof(char), program_size, program_handle);
    fclose(program_handle);

    /* Create program from file */
    program = clCreateProgramWithSource(context, 1, (const char**)&program_buffer, &program_size, &err);
    if(err < 0) {
        perror("Couldn't create the program");
        exit(1);   
    }
    free(program_buffer);

    /* Build program */
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(err < 0) {

        /* Find size of log and print to std output */
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 
            0, NULL, &log_size);
        program_log = (char*) malloc(log_size + 1);
        program_log[log_size] = '\0';
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 
            log_size + 1, program_log, NULL);
        printf("%s\n", program_log);
        free(program_log);
        exit(1);
    }

    /* Create kernel for the function */
    kernel = clCreateKernel(program, KERNEL_FUNC, &err);
    if(err < 0) {
        perror("Couldn't create the kernel");
        exit(1);   
    }

    /* Create CL buffers to hold input and output data */
    dispLeft_buff = clCreateBuffer(context, CL_MEM_READ_ONLY | 
        CL_MEM_COPY_HOST_PTR, sizeof(dispLeft), dispLeft, &err);

    dispConf_buff = clCreateBuffer(context, CL_MEM_READ_ONLY | 
        CL_MEM_COPY_HOST_PTR, sizeof(dispConf), dispConf, &err);

    lutMeta_buff = clCreateBuffer(context, CL_MEM_READ_ONLY | 
        CL_MEM_COPY_HOST_PTR, sizeof(lutMeta), lutMeta, &err);

    voxels_buff = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 
        VOXEL_BUFFER_SIZE*sizeof(VoxelPoint), NULL, &err);

    hashIDs_buff = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 
        VOXEL_BUFFER_SIZE*sizeof(uint32_t), NULL, &err);

    count_buff = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 
        VOXEL_BUFFER_SIZE*sizeof(uint8_t), NULL, &err);

    numVoxels_buff = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 
        sizeof(uint32_t), NULL, &err);

    args_buff = clCreateBuffer(context, CL_MEM_READ_ONLY | 
        CL_MEM_COPY_HOST_PTR, sizeof(args), &args, &err);

    /* Create kernel arguments from the CL buffers */
    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &dispLeft_buff);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &dispConf_buff);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &lutMeta_buff);
    err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &voxels_buff);
    err |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &hashIDs_buff);
    err |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &count_buff);
    err |= clSetKernelArg(kernel, 6, sizeof(cl_mem), &numVoxels_buff);
    err |= clSetKernelArg(kernel, 7, sizeof(cl_mem), &args_buff);

    if(err < 0) {
        perror("Couldn't set the kernel argument");
        exit(1);   
    }

    /* Create a CL command queue for the device*/
    //queue = clCreateCommandQueueWithProperties(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
    queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);

    if(err < 0) {
        perror("Couldn't create the command queue");
        exit(1);
    }
    err = clFinish(queue);

    /* Enqueue the command queue to the device */
    global_dim[0] = gridWidth;
    global_dim[1] = gridHeight;
    err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, (const size_t *)&global_dim, NULL, 0, NULL, &event);
    if(err < 0) {
        perror("Couldn't enqueue the kernel execution command");
        exit(1);   
    }

    err = clFinish(queue);

    /* Read the result */
    err = clEnqueueReadBuffer(queue, voxels_buff, CL_TRUE, 0, sizeof(voxels), 
        voxels, 0, NULL, NULL);
    err = clEnqueueReadBuffer(queue, hashIDs_buff, CL_TRUE, 0, sizeof(hashIDs), 
        hashIDs, 0, NULL, NULL);
    err = clEnqueueReadBuffer(queue, count_buff, CL_TRUE, 0, sizeof(count), 
        count, 0, NULL, NULL);
    err = clEnqueueReadBuffer(queue, numVoxels_buff, CL_TRUE, 0, sizeof(numVoxels), 
        &numVoxels, 0, NULL, NULL);
    if(err < 0) {
        perror("Couldn't enqueue the read buffer command");
        exit(1);   
    }

    //wait for the event to complete the kernel
    clWaitForEvents(1, &event);

    /* Test the result */
    printf("numVoxels %d\n", numVoxels);



    // Sample Code that can be used for timing kernel execution duration
    // Using different parameters for cl_profiling_info allows us to
    // measure the wait time
    cl_int err_code;
    cl_ulong starttime;
    cl_ulong endtime;
    err_code = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &starttime, NULL);
    err_code = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,   sizeof(cl_ulong), &endtime,   NULL);
    unsigned long elapsed = (unsigned long)(endtime - starttime);
    printf("opencl Kernel Execution \t %lu us \n",elapsed/1000);

    FILE *fpOut1 = fopen("./points_ocl.txt", "w");
    for(int i=0; i<numVoxels; i++) {
        fprintf(fpOut1, "%d %d %d\n",voxels[i].x_mm, voxels[i].y_mm, voxels[i].z_mm);
    }

    fclose(fpOut1);
    /* Deallocate resources */
    clReleaseMemObject(dispLeft_buff);
    clReleaseMemObject(dispConf_buff);
    clReleaseMemObject(lutMeta_buff);
    clReleaseMemObject(voxels_buff);
    clReleaseMemObject(hashIDs_buff);
    clReleaseMemObject(count_buff);
    clReleaseMemObject(numVoxels_buff);
    clReleaseMemObject(args_buff);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);

    return 0;
}

