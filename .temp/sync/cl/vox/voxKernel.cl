#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

#include "vox_structs.h"

//---------------------------------------------------------------------------
//
float32_t sin_approx(float32_t x)
{
    const float32_t pib2 = (double)1.57079632679489661923;
    const float32_t pi = (double)3.14159265358979323846;

    // increase range of x from [-pi/2, pi2] to [-5/2*pi, 7/2*pi] by
    // reflecting the x coordinate 3 times
    if (x > pib2)
    {
        x = pi - x; // reflection at x = pi/2
    }
    if (x < -pib2)
    {
        x = -pi - x; // reflection at x = -pi/2
    }
    if (x > pib2)
    {
        x = pi - x; // reflection at x = pi/2 again to reach at least -2*pi
    }

    const float32_t x2 = x*x;
    // Calculate the polynomial of the form P(x) = a'*x + b'*x^3 + c'*x^5 + d'*x^7
    // The coefficients have been found solving the following equations based on z = x/(pi/2), S(z) = a*z + b*z^3 + c*z^5 + d*z^7
    // with the help of Wolfram alpha ;)
    // (I) S(1) = 1, (II) S'(1) = 0, (III)  S'(0) = pi/2, (IV) avg(S(x), 0, 1) = avg(sin(x*pi/2), 0, 1) -> a/2+b/4+c/6+d/8 = 2/pi
    // a' = a * (2/pi), b' = b * (2/pi)^3, c' = c * (2/pi)^5, d' = d * (2/pi)^7
    // Average error: 1e-6, maximum error: 2.4e-6
    return x * (x2 * (x2 * ((x2 * -0.00018269040923000167F) + 0.008304602241873476F) - 0.16665101214369515F) + 1.0F);
}

float32_t cos_approx(float32_t x)
{
    return sin_approx(x + f32_(M_PI_2));
}

//---------------------------------------------------------------------------
//
__kernel void cylindrical(__global uint16_t* pDispLeft,
                          __global uint8_t* pDispConf,
                          __global LutMeta* pLutMeta,
                          __global VoxelPoint* voxels,
                          __global uint32_t* hashIDs,
                          __global uint8_t* count,
                          __global uint32_t* numVoxels,
                          __global const_args* args) {
   
    int i = get_global_id(0);
    int j = get_global_id(1);

    // set constant arguments
    int32_t                 m_dispScale;
    MOS_VoxelGridSizes_t    g_mosVoxelGridSizes;
    VoxelCellData           voxelData;

    int32_t roiMinX = args->roiMinX;
    int32_t roiMaxX = args->roiMaxX;
    int32_t roiMinY = args->roiMinY;
    int32_t roiMaxY = args->roiMaxY;
    int32_t width = args->width;
    int32_t height = args->height;
    int32_t dispOffset = args->dispOffset;
    float32_t startAzim_rad = args->startAzim_rad;
    int32_t  gridWidth = args->gridWidth;
    int32_t  gridHeight = args->gridHeight;
    int32_t  gridCellWidth = args->gridCellWidth;
    int32_t  gridCellHeight = args->gridCellHeight;

    voxelData.cellIdx           = args->cellIdx;         
    voxelData.cellWidth         = args->cellWidth;       
    voxelData.cellHeight        = args->cellHeight;      
    voxelData.startX            = args->startX;          
    voxelData.startY            = args->startY;          
    voxelData.lutShiftX         = args->lutShiftX;       
    voxelData.lutShiftY         = args->lutShiftY;       
    voxelData.lutWidth          = args->lutWidth;        
    voxelData.imgWidth          = args->imgWidth;        
    voxelData.minDisp           = args->minDisp;         
    voxelData.maxDisp           = args->maxDisp;         
    voxelData.dispScaleInv      = args->dispScaleInv;    
    voxelData.b_mm              = args->b_mm;            
    voxelData.startN_px         = args->startN_px;       
    voxelData.azimAngle_rad     = args->azimAngle_rad;   
    voxelData.azimDelta_rad     = args->azimDelta_rad;   
    voxelData.polarAngle_rad    = args->polarAngle_rad;  
    voxelData.polarDelta_rad    = args->polarDelta_rad;  
    voxelData.cenX_px           = args->cenX_px;
    voxelData.cenY_px           = args->cenY_px;
    voxelData.planarAspect      = args->planarAspect;
    voxelData.offset_mm         = args->offset_mm;
    voxelData.f_px              = args->f_px;
    voxelData.scaleN            = args->scaleN;
    voxelData.scaleR            = args->scaleR;
    voxelData.baselineError_mm  = args->baselineError_mm;
    voxelData.pos_mm.x          = args->pos_mm_x;
    voxelData.pos_mm.y          = args->pos_mm_y;
    voxelData.pos_mm.z          = args->pos_mm_z;
    voxelData.rot.xx            = args->rot_xx;
    voxelData.rot.xy            = args->rot_xy;
    voxelData.rot.xz            = args->rot_xz;
    voxelData.rot.yx            = args->rot_yx;
    voxelData.rot.yy            = args->rot_yy;
    voxelData.rot.yz            = args->rot_yz;
    voxelData.rot.zx            = args->rot_zx;
    voxelData.rot.zy            = args->rot_zy;
    voxelData.rot.zz            = args->rot_zz;

    m_dispScale                 = args->m_dispScale;

    g_mosVoxelGridSizes.mmPerVoxel          = args->mmPerVoxel;
    g_mosVoxelGridSizes.mmPerVoxelInv       = args->mmPerVoxelInv;
    g_mosVoxelGridSizes.voxelGridLength_vx  = args->voxelGridLength_vx;
    g_mosVoxelGridSizes.voxelGridWidth_vx   = args->voxelGridWidth_vx;
    g_mosVoxelGridSizes.voxelGridHeight_vx  = args->voxelGridHeight_vx;
    g_mosVoxelGridSizes.numVoxels           = args->numVoxels;
    g_mosVoxelGridSizes.voxelGridZmin_mm    = args->voxelGridZmin_mm;
    g_mosVoxelGridSizes.voxelGridZmax_mm    = args->voxelGridZmax_mm;
    g_mosVoxelGridSizes.voxelGridXmax_mm    = args->voxelGridXmax_mm;
    g_mosVoxelGridSizes.voxelGridXmin_mm    = args->voxelGridXmin_mm;
    g_mosVoxelGridSizes.voxelGridYmax_mm    = args->voxelGridYmax_mm;
    g_mosVoxelGridSizes.voxelGridYmin_mm    = args->voxelGridYmin_mm;
 
 
    // main function
    voxelData.startY = (j * gridCellHeight) + roiMinY;
    voxelData.startX = (i * gridCellWidth) + roiMinX;
    voxelData.cellIdx = (j * gridWidth) + i;

    // If we are the last cell in horizontal or vertical direction the size might be different
    voxelData.cellHeight = (j == (gridHeight - 1)) ? (roiMaxY + 1) - voxelData.startY : gridCellHeight;
    voxelData.cellWidth = (i == (gridWidth - 1)) ? (roiMaxX + 1) - voxelData.startX : gridCellWidth;

    // The azimuthal angle is mapped to the Y-coordinate
    voxelData.azimAngle_rad = startAzim_rad + (f32_(voxelData.startY) * voxelData.azimDelta_rad);

    int32_t y = voxelData.startY;
    FLOAT azimAngle_rad = voxelData.azimAngle_rad;
    VoxelScratch l_scratch;

    l_scratch.numVoxels = 0;
    for (int i = 0; i < VOXEL_SCRATCH_HASH_SIZE; i++)
    {
        l_scratch.hashTable[i] = 0xFF;
    }
    for (int i = 0; i < VOXEL_SCRATCH_SIZE; i++)
    {
        l_scratch.voxels[i].x_mm = 0;
        l_scratch.voxels[i].y_mm = 0;
        l_scratch.voxels[i].z_mm = 0;
    }

    // scale odometry uncertainty by the disparity scale since the baseline is pre-adjusted by it
    const float32_t odomUncertainty = voxelData.baselineError_mm * (FLOAT)(m_dispScale);



	
#define LOCAL1 1  
#define LOCAL2 0                
#if LOCAL1 
	int32_t disp[12*10];  
#endif
#if LOCAL2
    uint8_t pLocalDispConf[12*10];
#endif

#if LOCAL1
	//printf("%d %d\n", voxelData.cellHeight, voxelData.cellWidth);
    for (int32_t v = 0; v < voxelData.cellHeight; v++)
    {
        int32_t x = voxelData.startX;
        for(int32_t u = 0; u < voxelData.cellWidth; u++)
        {
            const int32_t dispIdx = ((y+v)*voxelData.imgWidth) + x+u;   
			disp[v*voxelData.cellWidth+u] = (int32_t)(pDispLeft[dispOffset + dispIdx]);
  		}                                  
	}
	//synchronization barrier
	barrier(CLK_LOCAL_MEM_FENCE);
#endif	

#if LOCAL2
	for (int32_t v = 0; v < voxelData.cellHeight; v++)
    {    
        int32_t x = voxelData.startX;	
		for(int32_t u = 0; u < voxelData.cellWidth; u++)
        {   
            const int32_t dispIdx = ((y+v)*voxelData.imgWidth) + x+u;   
			pLocalDispConf[v*voxelData.cellWidth+u] = (int32_t)(pDispConf[dispOffset + dispIdx]);
  		}
	}
	//synchronization barrier
	barrier(CLK_LOCAL_MEM_FENCE);
#endif	

	for (int32_t v = 0; v < voxelData.cellHeight; v++)
    {
        int32_t x = voxelData.startX;
        float32_t n_px = voxelData.startN_px + (FLOAT)(x);

        const int32_t lutIdxLine = ((y >> voxelData.lutShiftY) * voxelData.lutWidth);

        // Use approximation of sine and cosine since no visible difference can be seen
        const float32_t sinAzim = sin_approx(azimAngle_rad);
        const float32_t cosAzim = cos_approx(azimAngle_rad);

        for (int32_t u = 0; u < voxelData.cellWidth; u++)
        {
            const int32_t lutIdx = lutIdxLine + (x >> voxelData.lutShiftX);
 			const int32_t dispIdx = ((y)*voxelData.imgWidth) + x;
			#if !LOCAL1
			//printf("%d %d,\t", y, x); 
            const int32_t disp = (int32_t)(pDispLeft[dispOffset + dispIdx]);
			#endif
            const LutMeta meta = pLutMeta[lutIdx];
			#if !LOCAL1
            if ((disp >= voxelData.minDisp) && (disp <= voxelData.maxDisp) && (meta.structure != 0))
			#else
			if ((disp[v*voxelData.cellWidth+u] >= voxelData.minDisp) && (disp[v*voxelData.cellWidth+u] <= voxelData.maxDisp) && (meta.structure != 0))
			#endif
            {
                // The calculation here are similar to the planar case, the coordinate system is
                // however derived from the spherical view ports 
                // (n/x points backwards, v/y upwards, u/z right in ISO based epipolar coordinate system)
                //
                // Therefore the calculation is slightly different
                // The stereo equations in the cylindrical case provide the distance from the principal point
                // in direction of the cylindar and the distance (radius) from its axis
                // Therefore sin and cos of the azimutal angle have to be used to derive
                // the remaining two coordinate values from the radius
				#if !LOCAL1
                const float32_t dispfInv = 1.0F / (FLOAT)(disp);
				#else
				const float32_t dispfInv = 1.0F / (FLOAT)(disp[v*voxelData.cellWidth + u]);
				#endif
                const float32_t ratio = voxelData.b_mm * dispfInv;
                float32_t z_mm = (n_px * ratio) + voxelData.offset_mm;
                float32_t r_mm = (voxelData.f_px * ratio);
                float32_t y_mm = sinAzim * r_mm;
                float32_t x_mm = cosAzim * r_mm;

                // Compute spatial uncertainty
                // We use the derivatives with respect to the disparity of the equations above to determine how
                // sensitive the calculated coordinates are to changes in the disparity

                // First assume a certain uncertainty in pixel space in units of disp (1/16th of a pixel since b_mm is scaled by 16)
                const float32_t pixelUncertainty = (SCALING_RATIO * STEREO_PIXEL_UNCERTAINTY) * (FLOAT)(meta.scaling);

                // Calculate the derivatives for functions z_mm(disp), r_mm(disp) plus z_mm(b_mm), r_mm(b_mm)
                // to calculate the sum of the partial derivatives wrt disparity (pixel inaccuracies) and baseline (odometry inaccuracies)
                const float32_t ratioDer = fabs_(ratio * dispfInv);
                const float32_t maxFactor = MAX(fabs_(voxelData.f_px), fabs_(n_px));
                // Take the maximum value and multiply by the pixel uncertainty
                float32_t uncertainty_mm = maxFactor * ((pixelUncertainty * ratioDer) +
                    (odomUncertainty * dispfInv));

                // convert point into vehicle coordinate system
                const float32_t l_Zmm = (voxelData.rot.zx * x_mm) + (voxelData.rot.zy * y_mm) + (voxelData.rot.zz * z_mm);
                const float32_t l_Xmm = (voxelData.rot.xx * x_mm) + (voxelData.rot.xy * y_mm) + (voxelData.rot.xz * z_mm);
                const float32_t l_Ymm = (voxelData.rot.yx * x_mm) + (voxelData.rot.yy * y_mm) + (voxelData.rot.yz * z_mm);
                
                uncertainty_mm += ODOMETRY_UNCERTAINTY_RAD * (fabs_(x_mm) + fabs_(y_mm) + fabs_(z_mm));

                z_mm = l_Zmm + voxelData.pos_mm.z;
                x_mm = l_Xmm + voxelData.pos_mm.x;
                y_mm = l_Ymm + voxelData.pos_mm.y;

                // voxelise
                const FLOAT l_x_mm = x_mm;
                const FLOAT l_y_mm = y_mm;
                const FLOAT l_z_mm = z_mm;

                const FLOAT mmPerVoxelInv = g_mosVoxelGridSizes.mmPerVoxelInv;
                const FLOAT voxelGridXmin_mm = g_mosVoxelGridSizes.voxelGridXmin_mm;
                const FLOAT voxelGridYmin_mm = g_mosVoxelGridSizes.voxelGridYmin_mm;
                const FLOAT voxelGridZmin_mm = g_mosVoxelGridSizes.voxelGridZmin_mm;
                const FLOAT voxelGridXmax_mm = g_mosVoxelGridSizes.voxelGridXmax_mm;
                const FLOAT voxelGridYmax_mm = g_mosVoxelGridSizes.voxelGridYmax_mm;
                const FLOAT voxelGridZmax_mm = g_mosVoxelGridSizes.voxelGridZmax_mm;

                if ((l_x_mm > voxelGridXmin_mm) && (l_x_mm < voxelGridXmax_mm)
                    && (l_y_mm > voxelGridYmin_mm) && (l_y_mm < voxelGridYmax_mm)
                    && (l_z_mm > voxelGridZmin_mm) && (l_z_mm < voxelGridZmax_mm)
                    )
                {
                    const uint32_t numVoxels = l_scratch.numVoxels;
                    int32_t index;

                    // Get voxel grid coordinates. The origin of the grid is in the point with lowest z coordinate and the highest x and
                    // y coordinates of the voxel grid.
                    int32_t xGridCoor_px = (int32_t)((voxelGridXmax_mm - l_x_mm) * mmPerVoxelInv);
                    int32_t yGridCoor_px = (int32_t)((voxelGridYmax_mm - l_y_mm) * mmPerVoxelInv);
                    int32_t zGridCoor_px = (int32_t)((l_z_mm - voxelGridZmin_mm) * mmPerVoxelInv);

                    const int32_t offsetX_mm = (uint8_t)(((int32_t)voxelGridXmax_mm - (xGridCoor_px * g_mosVoxelGridSizes.mmPerVoxel)) - (int32_t)l_x_mm);
                    const int32_t offsetY_mm = (uint8_t)(((int32_t)voxelGridYmax_mm - (yGridCoor_px * g_mosVoxelGridSizes.mmPerVoxel)) - (int32_t)l_y_mm);
                    const int32_t offsetZ_mm = (uint8_t)((int32_t)l_z_mm - ((int32_t)voxelGridZmin_mm + (zGridCoor_px * g_mosVoxelGridSizes.mmPerVoxel)));

                    // Store final uncertainty value in cm with uint8 range
                    const uint8_t uncertainty_cm = u8_(MIN(uncertainty_mm * 0.1F, 255.0F));

                    // InsertScratchHash
                    uint32_t k = (xGridCoor_px << 16) | (yGridCoor_px << 8) | zGridCoor_px;

                    const int32_t  shift = 32 - 8;
                    const uint32_t a = 2654435769U;     /* 2^32 / golden ratio */
                    uint32_t ak = a * k;
                    const uint32_t maxJ = 12;           /* 12 attempts for finding an entry after collision */
                    uint32_t j = 1;
                    enum INSERT_STATUS st = COLLISION;
                    uint32_t hash;
                    uint32_t pcIdx;

                    do
                    {
                        hash = ak >> shift;
                        pcIdx = l_scratch.hashTable[hash];

                        /* check if the entry is empty. If so get out of the loop */
                        if ((pcIdx != 0xFFU) && (pcIdx < VOXEL_SCRATCH_SIZE))  /* the compiler should combine these two comparisons */
                        {
                            /* If the entry is not empty we have to check if it is the one
                            * we want to insert. If not we have to try another hash */
                            if (l_scratch.hashIDs[pcIdx] == k)
                            {
                                /* We found an existing entry that matches */
                                st = ALREADY_EXISTS;
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                        ak *= a;        /* we multiply again by a to get multiple different hashes for the same k */
                        j++;
                    } while (j < maxJ);


                    /* If the voxel is not filled yet insert it.
                    *  We cannot exceed capcity since the size of the grid cell is small enough */
                    if ((pcIdx == 0xFFU))
                    {
                        pcIdx = l_scratch.numVoxels;
                        l_scratch.hashTable[hash] = pcIdx;
                        l_scratch.hashIDs[pcIdx] = k;
                        l_scratch.numVoxels = pcIdx + 1;
                        st = INSERTED;
                    }

                    index = (int32_t)pcIdx;

                    if (st == INSERTED)
                    {
                        // This is a new voxel so fill all the initial data for it 
                        l_scratch.voxels[index].x_mm = (int16_t)offsetX_mm;
                        l_scratch.voxels[index].y_mm = (int16_t)offsetY_mm;
                        l_scratch.voxels[index].z_mm = (int16_t)offsetZ_mm;
                        l_scratch.voxels[index].uncertainty_cm = uncertainty_cm;
						#if !LOCAL2
                        l_scratch.voxels[index].existence = pDispConf[dispOffset + dispIdx];
						#else
						l_scratch.voxels[index].existence = pLocalDispConf[v*voxelData.cellWidth+u];
						#endif
                        l_scratch.hashCount[index] = 1;
                    }
                    else if (st == ALREADY_EXISTS)
                    {
                        // The voxel already exists. Add the new point on top of it
                        // The average will be formed in \ref MOS_FinaliseVoxels after all instances of MOS have finished

                        // Assume that we have less then 256 points, each with less then -+127 coordinate offset and therefore
                        // The range of -+2^15 is kept
                        l_scratch.voxels[index].x_mm += (int16_t)offsetX_mm;
                        l_scratch.voxels[index].y_mm += (int16_t)offsetY_mm;
                        l_scratch.voxels[index].z_mm += (int16_t)offsetZ_mm;
                        l_scratch.voxels[index].uncertainty_cm = MIN(l_scratch.voxels[index].uncertainty_cm, uncertainty_cm);
						#if !LOCAL2
                        l_scratch.voxels[index].existence = MAX(l_scratch.voxels[index].existence, pDispConf[dispOffset + dispIdx]);
						#else
						l_scratch.voxels[index].existence = MAX(l_scratch.voxels[index].existence, pLocalDispConf[v*voxelData.cellWidth+u]);
						#endif
                        l_scratch.hashCount[index]++;
                    }
                    else
                    {
                        // This means we had a collision so we have to skip this point
                    }
                }
            }
            x++;
            n_px += 1.0F;
        }
		//printf("\n");
        y++;
        azimAngle_rad += voxelData.azimDelta_rad;
    }

    // copyScratch
    // copy out the data from scratch to our buffer
    for (uint32_t i = 0U; i < l_scratch.numVoxels; i++)
    {
        const int32_t numOut = atomic_inc(numVoxels);
        voxels[numOut] = l_scratch.voxels[i];
    }
}