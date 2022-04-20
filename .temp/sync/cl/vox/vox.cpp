#include <cstdio>
#include <cstdint>
#include <cmath>
#include <iostream>

//---------------------------------------------------------------------------
//
typedef float float32_t;
typedef float FLOAT;

//---------------------------------------------------------------------------
//
#define STEREO_PIXEL_UNCERTAINTY    (4.0F)
#define SCALING_RATIO               (1.0F / 64.0F)
#define ODOMETRY_UNCERTAINTY_RAD    (0.02F)
#define VOXEL_SCRATCH_SIZE          (20U)
#define VOXEL_BUFFER_SIZE           (20000U)
#define VOXEL_SCRATCH_HASH_SIZE     (256U)
//---------------------------------------------------------------------------
//
#define f32_                (float32_t)
#define u8_                 (uint8_t)
#define fabs_               ABS
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define ABS(x) (((x) > 0) ? (x) : ((-1)*(x)))
#define M_PI            3.14159265358979323846
#define M_PI_2          1.57079632679489661923

//---------------------------------------------------------------------------
//
typedef struct
{
    float32_t x;    //!< X component
    float32_t y;    //!< Y component
    float32_t z;    //!< Z component
}Vec3f;

typedef struct
{
    float32_t xx;   //!< X'-axis component X or X-axis component X'
    float32_t xy;   //!< X'-axis component Y or Y-axis component X'
    float32_t xz;   //!< X'-axis component Z or Z-axis component X'
    float32_t yx;   //!< Y'-axis component X or X-axis component Y'
    float32_t yy;   //!< Y'-axis component Y or Y-axis component Y'
    float32_t yz;   //!< Y'-axis component Z or Z-axis component Y'
    float32_t zx;   //!< Z'-axis component X or X-axis component Z'
    float32_t zy;   //!< Z'-axis component Y or Y-axis component Z'
    float32_t zz;   //!< Z'-axis component Z or Z-axis component Z'
}Mat3f;

typedef struct
{
    uint8_t         structure;          //!< Image structure based on Harris feature score (currently fixed value)
    uint8_t         scaling;            //!< Scaling factor, 64 - means 1.0
}LutMeta;

typedef struct
{
    int16_t             x_mm;               //!< X offset in mm
    int16_t             y_mm;               //!< Y offset in mm
    int16_t             z_mm;               //!< Z offset in mm
    uint8_t             uncertainty_cm;     //!< Spatial uncertainty of the point in mm
    uint8_t             existence;          //!< STV confidence
}VoxelPoint;

typedef struct
{
    VoxelPoint voxels[VOXEL_SCRATCH_SIZE];          //!< Voxel coordinate offsets
    uint32_t   hashIDs[VOXEL_SCRATCH_SIZE];         //!< Hash ID value used to identify a hash entry
    uint8_t    hashCount[VOXEL_SCRATCH_SIZE];       //!< Number of 3D points added to this location
    uint8_t    hashTable[VOXEL_SCRATCH_HASH_SIZE];  //!< Hash table used to find existing entries
    uint32_t   numVoxels;                           //!< Number of voxels contained 
}VoxelScratch;

typedef struct
{
    int32_t     cellIdx;                    //!< Index of the current grid cell
    int32_t     cellWidth;                  //!< Cell width (normal)
    int32_t     cellHeight;                 //!< Cell height
    int32_t     startX;                     //!< Start X coordinate in disparity image
    int32_t     startY;                     //!< Start Y coordinate in disparity image
    int32_t     lutShiftX;                  //!< LUT shift value in X direction
    int32_t     lutShiftY;                  //!< LUT shift value in Y direction
    int32_t     lutWidth;                   //!< Width of the LUT table
    int32_t     imgWidth;                   //!< Disparity image width
    int32_t     minDisp;                    //!< Minimal disparity value
    int32_t     maxDisp;                    //!< Maximum disparity value
    FLOAT       dispScaleInv;               //!< Inverse disparity scale value
    FLOAT       b_mm;                       //!< baseline length
    FLOAT       startN_px;                  //!< Start pixel in n direction (cylindrical / conical viewports)
    FLOAT       azimAngle_rad;              //!< Start azimuth angle (all except planar viewports)
    FLOAT       azimDelta_rad;              //!< Azimuth delta angle (all except planar viewports)
    FLOAT       polarAngle_rad;             //!< Start polar angle (spherical only)
    FLOAT       polarDelta_rad;             //!< Polar delta angle (spherical only)
    FLOAT       cenX_px;                    //!< Center pixel X coordinate (planar only)
    FLOAT       cenY_px;                    //!< Center pixel Y coordinate (planar only)
    FLOAT       planarAspect;               //!< Aspect Ratio (planar only)
    FLOAT       offset_mm;                  //!< Streo mode offset (u)
    FLOAT       f_px;                       //!< Focal length in pixel
    FLOAT       scaleN;                     //!< Conical scale parameter for N coordinate
    FLOAT       scaleR;                     //!< Conical scale parameter for radius
    FLOAT       baselineError_mm;           //!< Assumed baseline error
    Vec3f       pos_mm;                     //!< View position in vehicle coordinates
    Mat3f       rot;                        //!< View to vehicle rotation
}VoxelCellData;

typedef struct
{
    VoxelPoint *voxels;         //!< Voxel coordinate offsets
    uint32_t   *hashIDs;        //!< Hash ID for each entry
    uint8_t    *count;          //!< Number of added 3D points for each entry
    uint32_t   numVoxels;       //!< Number of voxels contained 
}VoxelBuffer;

typedef struct
{
    //! Millimeters per voxel, range [0, 10000]
    int32_t mmPerVoxel;
    //! Inverse Millimeters per voxel, range [0, 10000]
    FLOAT mmPerVoxelInv;
    //! Voxel grid size (in voxels) in X direction, range [0, 255]
    int32_t voxelGridLength_vx;
    //! Voxel grid size (in voxels) in Y direction, range [0, 255]
    int32_t voxelGridWidth_vx;
    //! Voxel grid size (in voxels) in Z direction, range [0, 31]
    int32_t voxelGridHeight_vx;
    //! Number of voxels of the point cloud grid, range [0, 2097152] 
    int32_t numVoxels;
    //! Minimum height (Z vehicle axis) value, covered by the voxel grid (mm), range [-100000, 100000]
    FLOAT voxelGridZmin_mm;
    //! Maximum height (Z vehicle axis) value, covered by the voxel grid (mm), range [-100000, 100000]
    FLOAT voxelGridZmax_mm;
    //! Maximum vehicle X axis value covered by the voxel grid (mm), range [-100000, 100000]
    FLOAT voxelGridXmax_mm;
    //! Minimum vehicle X axis value covered by the voxel grid (mm), range [-100000, 100000]
    FLOAT voxelGridXmin_mm;
    //! Maximum vehicle Y axis value covered by the voxel grid (mm), range [-100000, 100000]
    FLOAT voxelGridYmax_mm;
    //! Minimum vehicle Y axis value covered by the voxel grid (mm), range [-100000, 100000]
    FLOAT voxelGridYmin_mm;
}MOS_VoxelGridSizes_t;

enum INSERT_STATUS
{
    INSERTED = 0,       //!< The element was inserted
    ALREADY_EXISTS,     //!< The element already exists
    COLLISION           //!< The element could not be inserted since no free spot could be found
};

//---------------------------------------------------------------------------
//
VoxelBuffer         m_voxelBuffer;
VoxelScratch        *m_voxelScratch;

int32_t             m_dispScale;
MOS_VoxelGridSizes_t g_mosVoxelGridSizes;

//---------------------------------------------------------------------------
//
float32_t sin_approx(float32_t x)
{
    const float32_t pib2 = f32_(M_PI_2);
    const float32_t pi = f32_(M_PI);

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

void
resetDispGridHash(
    VoxelScratch&        f_scratch
)
{
    f_scratch.numVoxels = 0U;
    for (int32_t i = 0; i < VOXEL_SCRATCH_HASH_SIZE; i++)
    {
        f_scratch.hashTable[i] = 0xFFU;
    }
}

INSERT_STATUS
insertScratchHash(
    VoxelScratch&        f_scratch,
    int32_t&             f_index,
    const uint32_t       f_x_px,
    const uint32_t       f_y_px,
    const uint32_t       f_z_px
)
{
    uint32_t k = (f_x_px << 16) | (f_y_px << 8) | f_z_px;

    const int32_t  shift = 32 - 8;
    const uint32_t a = 2654435769U;     /* 2^32 / golden ratio */
    uint32_t ak = a * k;
    const uint32_t maxJ = 12;           /* 12 attempts for finding an entry after collision */
    uint32_t j = 1;
    enum INSERT_STATUS res = COLLISION;
    uint32_t hash;
    uint32_t pcIdx;

    do
    {
        hash = ak >> shift;
        pcIdx = f_scratch.hashTable[hash];

        /* check if the entry is empty. If so get out of the loop */
        if ((pcIdx != 0xFFU) && (pcIdx < VOXEL_SCRATCH_SIZE))  /* the compiler should combine these two comparisons */
        {
            /* If the entry is not empty we have to check if it is the one
            * we want to insert. If not we have to try another hash */
            if (f_scratch.hashIDs[pcIdx] == k)
            {
                /* We found an existing entry that matches */
                res = ALREADY_EXISTS;
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
        pcIdx = f_scratch.numVoxels;
        f_scratch.hashTable[hash] = pcIdx;
        f_scratch.hashIDs[pcIdx] = k;
        f_scratch.numVoxels = pcIdx + 1;
        res = INSERTED;
    }

    f_index = (int32_t)pcIdx;

    return res;
}

void
addVoxel(
    VoxelScratch&       f_scratch,
    const MOS_VoxelGridSizes_t& f_grid,
    const Vec3f&   f_pos,
    const FLOAT         f_uncertainty_mm,
    const uint8_t       f_confidence
)
{
    const FLOAT l_x_mm = f_pos.x;
    const FLOAT l_y_mm = f_pos.y;
    const FLOAT l_z_mm = f_pos.z;

    const FLOAT mmPerVoxelInv = f_grid.mmPerVoxelInv;
    const FLOAT voxelGridXmin_mm = f_grid.voxelGridXmin_mm;
    const FLOAT voxelGridYmin_mm = f_grid.voxelGridYmin_mm;
    const FLOAT voxelGridZmin_mm = f_grid.voxelGridZmin_mm;
    const FLOAT voxelGridXmax_mm = f_grid.voxelGridXmax_mm;
    const FLOAT voxelGridYmax_mm = f_grid.voxelGridYmax_mm;
    const FLOAT voxelGridZmax_mm = f_grid.voxelGridZmax_mm;

    if ((l_x_mm > voxelGridXmin_mm) && (l_x_mm < voxelGridXmax_mm)
        && (l_y_mm > voxelGridYmin_mm) && (l_y_mm < voxelGridYmax_mm)
        && (l_z_mm > voxelGridZmin_mm) && (l_z_mm < voxelGridZmax_mm)
        )
    {
        const uint32_t numVoxels = f_scratch.numVoxels;
        int32_t index;

        // Get voxel grid coordinates. The origin of the grid is in the point with lowest z coordinate and the highest x and
        // y coordinates of the voxel grid.
        int32_t xGridCoor_px = static_cast<int32_t>((voxelGridXmax_mm - l_x_mm) * mmPerVoxelInv);
        int32_t yGridCoor_px = static_cast<int32_t>((voxelGridYmax_mm - l_y_mm) * mmPerVoxelInv);
        int32_t zGridCoor_px = static_cast<int32_t>((l_z_mm - voxelGridZmin_mm) * mmPerVoxelInv);

        const int32_t offsetX_mm = (uint8_t)(((int32_t)voxelGridXmax_mm - (xGridCoor_px * f_grid.mmPerVoxel)) - (int32_t)l_x_mm);
        const int32_t offsetY_mm = (uint8_t)(((int32_t)voxelGridYmax_mm - (yGridCoor_px * f_grid.mmPerVoxel)) - (int32_t)l_y_mm);
        const int32_t offsetZ_mm = (uint8_t)((int32_t)l_z_mm - ((int32_t)voxelGridZmin_mm + (zGridCoor_px * f_grid.mmPerVoxel)));

        // Store final uncertainty value in cm with uint8 range
        const uint8_t uncertainty_cm = u8_(MIN(f_uncertainty_mm * 0.1F, 255.0F));

        INSERT_STATUS st = insertScratchHash(f_scratch, index, xGridCoor_px, yGridCoor_px, zGridCoor_px);

        if (st == INSERTED)
        {
            // This is a new voxel so fill all the initial data for it 
            f_scratch.voxels[index].x_mm = (int16_t)offsetX_mm;
            f_scratch.voxels[index].y_mm = (int16_t)offsetY_mm;
            f_scratch.voxels[index].z_mm = (int16_t)offsetZ_mm;
            f_scratch.voxels[index].uncertainty_cm = uncertainty_cm;
            f_scratch.voxels[index].existence = f_confidence;
            f_scratch.hashCount[index] = 1;
        }
        else if (st == ALREADY_EXISTS)
        {
            // The voxel already exists. Add the new point on top of it
            // The average will be formed in \ref MOS_FinaliseVoxels after all instances of MOS have finished

            // Assume that we have less then 256 points, each with less then -+127 coordinate offset and therefore
            // The range of -+2^15 is kept
            f_scratch.voxels[index].x_mm += (int16_t)offsetX_mm;
            f_scratch.voxels[index].y_mm += (int16_t)offsetY_mm;
            f_scratch.voxels[index].z_mm += (int16_t)offsetZ_mm;
            f_scratch.voxels[index].uncertainty_cm = MIN(f_scratch.voxels[index].uncertainty_cm, uncertainty_cm);
            f_scratch.voxels[index].existence = MAX(f_scratch.voxels[index].existence, f_confidence);

            f_scratch.hashCount[index]++;
        }
        else
        {
            // This means we had a collision so we have to skip this point
        }
    }
}

void
copyScratch(
    VoxelBuffer&            f_out,
    const VoxelScratch&     f_scratch
)
{
    // check that we won't exceed the buffer
    if ((f_out.numVoxels + f_scratch.numVoxels) < VOXEL_BUFFER_SIZE)
    {
        // copy out the data from scratch to our buffer
        for (uint32_t i = 0U; i < f_scratch.numVoxels; i++)
        {
            const int32_t numOut = f_out.numVoxels;
            f_out.hashIDs[numOut] = f_scratch.hashIDs[i];
            f_out.voxels[numOut] = f_scratch.voxels[i];
            f_out.count[numOut] = f_scratch.hashCount[i];
            f_out.numVoxels = numOut + 1;
            //printf("%d %d %d\n",f_scratch.voxels[i].x_mm, f_scratch.voxels[i].y_mm, f_scratch.voxels[i].z_mm);
        }
    }
}

FLOAT
rotateAndTranslatePoint(
    float32_t&              f_Xmm,
    float32_t&              f_Ymm,
    float32_t&              f_Zmm,
    const Vec3f&       f_translation,
    const Mat3f&       f_rotation
)
{
    const float32_t l_Zmm = (f_rotation.zx * f_Xmm) + (f_rotation.zy * f_Ymm) + (f_rotation.zz * f_Zmm);
    const float32_t l_Xmm = (f_rotation.xx * f_Xmm) + (f_rotation.xy * f_Ymm) + (f_rotation.xz * f_Zmm);
    const float32_t l_Ymm = (f_rotation.yx * f_Xmm) + (f_rotation.yy * f_Ymm) + (f_rotation.yz * f_Zmm);

    // To determine the uncertainty introduced by the rotation above we remember that the coefficients
    // of the rotation matrix above come directly from the baseline direction as return by the odometry.
    // This direction is a point on the unit sphere. Now to model the error simply we assume a certain 
    // angle around that point on the sphere (circle on the sphere). To simplify even more we convert this
    // circle on the sphere to a little cube around that point with the side length of the radius of that 
    // circle. 
    // This results in a same error value for all the coefficients of the rotation matrix. Now if we derive
    // each of the coordinates above wrt the coefficients we get e.g. dx / dzx = f_Xmm and the error is 
    // e * |f_Xmm|. If we sum this for all coefficients we get e *(|f_Xmm| + |f_Ymm| + |f_Zmm|) for all
    // three coordinates
    //
    // We don't consider the error for the rotation around the baseline since this onl shifts the image vertically 
    // and only might degrade detection performance in general but not its accuracy
    const float32_t l_uncertainty_mm = ODOMETRY_UNCERTAINTY_RAD * (fabs_(f_Xmm) + fabs_(f_Ymm) + fabs_(f_Zmm));

    f_Zmm = l_Zmm + f_translation.z;
    f_Xmm = l_Xmm + f_translation.x;
    f_Ymm = l_Ymm + f_translation.y;

    return l_uncertainty_mm;
}

//---------------------------------------------------------------------------
//
void
fillVoxelsCylindricalCell(
    const uint16_t  *const pDispLeft,
    const uint8_t   *const pDispConf,
    const LutMeta *const    pLutMeta,
    const VoxelCellData&            voxelData
)
{
    int32_t y = voxelData.startY;
    FLOAT azimAngle_rad = voxelData.azimAngle_rad;
    VoxelScratch *l_scratch = m_voxelScratch;
    MOS_VoxelGridSizes_t& l_grid = g_mosVoxelGridSizes;
    // scale odometry uncertainty by the disparity scale since the baseline is pre-adjusted by it
    const float32_t odomUncertainty = voxelData.baselineError_mm * static_cast<FLOAT>(m_dispScale);

    for (int32_t v = 0; v < voxelData.cellHeight; v++)
    {
        int32_t x = voxelData.startX;
        float32_t n_px = voxelData.startN_px + static_cast<FLOAT>(x);

        const int32_t lutIdxLine = ((y >> voxelData.lutShiftY) * voxelData.lutWidth);

        // Use approximation of sine and cosine since no visible difference can be seen
        const float32_t sinAzim = sin_approx(azimAngle_rad);
        const float32_t cosAzim = cos_approx(azimAngle_rad);

        for (int32_t u = 0; u < voxelData.cellWidth; u++)
        {
            const int32_t lutIdx = lutIdxLine + (x >> voxelData.lutShiftX);
            const int32_t dispIdx = (y * voxelData.imgWidth) + x;
            const int32_t disp = static_cast<int32_t>(pDispLeft[dispIdx]);
            const LutMeta meta = pLutMeta[lutIdx];

            if ((disp >= voxelData.minDisp) && (disp <= voxelData.maxDisp) && (meta.structure != 0))
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
                const float32_t dispfInv = 1.0F / static_cast<FLOAT>(disp);
                const float32_t ratio = voxelData.b_mm * dispfInv;
                float32_t z_mm = (n_px * ratio) + voxelData.offset_mm;
                float32_t r_mm = (voxelData.f_px * ratio);
                float32_t y_mm = sinAzim * r_mm;
                float32_t x_mm = cosAzim * r_mm;

                // Compute spatial uncertainty
                // We use the derivatives with respect to the disparity of the equations above to determine how
                // sensitive the calculated coordinates are to changes in the disparity

                // First assume a certain uncertainty in pixel space in units of disp (1/16th of a pixel since b_mm is scaled by 16)
                const float32_t pixelUncertainty = (SCALING_RATIO * STEREO_PIXEL_UNCERTAINTY) * static_cast<FLOAT>(meta.scaling);

                // Calculate the derivatives for functions z_mm(disp), r_mm(disp) plus z_mm(b_mm), r_mm(b_mm)
                // to calculate the sum of the partial derivatives wrt disparity (pixel inaccuracies) and baseline (odometry inaccuracies)
                const float32_t ratioDer = fabs_(ratio * dispfInv);
                const float32_t maxFactor = MAX(fabs_(voxelData.f_px), fabs_(n_px));
                // Take the maximum value and multiply by the pixel uncertainty
                float32_t uncertainty_mm = maxFactor * ((pixelUncertainty * ratioDer) +
                    (odomUncertainty * dispfInv));

                // convert point into vehicle coordinate system
                uncertainty_mm += rotateAndTranslatePoint(x_mm, y_mm, z_mm, voxelData.pos_mm, voxelData.rot);

                // voxelise
                Vec3f pos = { x_mm, y_mm, z_mm };
                addVoxel(*l_scratch, l_grid, pos, uncertainty_mm, pDispConf[dispIdx]);
            }

            x++;
            n_px += 1.0F;
        }

        y++;
        azimAngle_rad += voxelData.azimDelta_rad;
    }
    copyScratch(m_voxelBuffer, *l_scratch);
}

void
fillVoxelsCylindrical(
    const uint16_t  *const pDispLeft,
    const uint8_t   *const pDispConf,
    const LutMeta *const    pLutMeta
) 
{
    //validate_ptr(pDispLeft);
    //validate_ptr(m_buff.pVoxel);
    //validate_(m_rec3dParamsSet);

    //const uint8_t camMask = 1U << (m_camIdx);
    int32_t roiMinX;
    int32_t roiMaxX;
    int32_t roiMinY;
    int32_t roiMaxY;
    int32_t width;
    int32_t height;

    // call function to retrieve these values due to HIS (Number of Calls CALLS)
    int32_t dispOffset;// = getDispRoiLimits(roiMinX, roiMaxX, roiMinY, roiMaxY, width); TODO

    VoxelCellData voxelData;

    voxelData.lutShiftX; // = m_rec3d.lut.shiftX;TODO
    voxelData.lutShiftY; // = m_rec3d.lut.shiftY;
    voxelData.lutWidth; // = m_rec3d.lut.width;
    voxelData.imgWidth; // = width;

    float32_t startAzim_rad; // = m_rec3d.cylindrical.startAzimAngle_rad;
    voxelData.azimDelta_rad; // = m_rec3d.cylindrical.azimDelta_rad;
    voxelData.startN_px; // = m_rec3d.cylindrical.startX_px;

    voxelData.pos_mm; // = m_rec3d.cylindrical.viewPos_mm;
    voxelData.rot; // = m_rec3d.cylindrical.viewToWorldRot;
    voxelData.b_mm; // = m_rec.b_mm; 

    voxelData.minDisp; // = m_rec.minDisp;
    voxelData.maxDisp; // = m_rec.maxDisp;
    voxelData.offset_mm; // = m_rec.offset_mm;
    voxelData.f_px; // = m_rec.f_px;
    voxelData.dispScaleInv; // = m_dispScaleInv;
    voxelData.baselineError_mm; // = static_cast<FLOAT>(m_rec3d.assumedBaselineError_mm);

    int32_t  gridWidth; // = m_dispGrid.width;
    int32_t  gridHeight; // = m_dispGrid.height;
    int32_t  gridCellWidth; // = m_dispGrid.cellWidth;
    int32_t  gridCellHeight; // = m_dispGrid.cellHeight;

    FILE *fpIn4;

    fpIn4 = fopen("./dump/init_input.bin", "rb");

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

    printf("W:%d H:%d\n", width, height);

    fclose(fpIn4);

#if 1
    for (int32_t j = 0; j < gridHeight; j++)
    {
        for (int32_t i = 0; i < gridWidth; i++)
        {
            voxelData.startY = (j * gridCellHeight) + roiMinY;
            voxelData.startX = (i * gridCellWidth) + roiMinX;
            voxelData.cellIdx = (j * gridWidth) + i;

            // If we are the last cell in horizontal or vertical direction the size might be different
            voxelData.cellHeight = (j == (gridHeight - 1)) ? (roiMaxY + 1) - voxelData.startY : gridCellHeight;
            voxelData.cellWidth = (i == (gridWidth - 1)) ? (roiMaxX + 1) - voxelData.startX : gridCellWidth;

            // The azimuthal angle is mapped to the Y-coordinate
            voxelData.azimAngle_rad = startAzim_rad + (f32_(voxelData.startY) * voxelData.azimDelta_rad);

            resetDispGridHash(*m_voxelScratch);
            fillVoxelsCylindricalCell(pDispLeft + dispOffset, pDispConf + dispOffset, pLutMeta, voxelData);
        }
    }
#else
    for (int32_t j = 0; j < gridHeight; j++)
    {
        for (int32_t i = 0; i < gridWidth; i++)
        {
            voxelData.startY = (j * gridCellHeight) + roiMinY;
            voxelData.startX = (i * gridCellWidth) + roiMinX;
            voxelData.cellIdx = (j * gridWidth) + i;

            // If we are the last cell in horizontal or vertical direction the size might be different
            voxelData.cellHeight = (j == (gridHeight - 1)) ? (roiMaxY + 1) - voxelData.startY : gridCellHeight;
            voxelData.cellWidth = (i == (gridWidth - 1)) ? (roiMaxX + 1) - voxelData.startX : gridCellWidth;

            // The azimuthal angle is mapped to the Y-coordinate
            voxelData.azimAngle_rad = startAzim_rad + (f32_(voxelData.startY) * voxelData.azimDelta_rad);

            //resetDispGridHash(*m_voxelScratch);
#if 0
            fillVoxelsCylindricalCell(pDispLeft + dispOffset, pDispConf + dispOffset, pLutMeta, voxelData);
#else
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
                    const int32_t dispIdx = (y * voxelData.imgWidth) + x;
                    const int32_t disp = (int32_t)(pDispLeft[dispOffset + dispIdx]);
                    const LutMeta meta = pLutMeta[lutIdx];

                    if ((disp >= voxelData.minDisp) && (disp <= voxelData.maxDisp) && (meta.structure != 0))
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
                        const float32_t dispfInv = 1.0F / (FLOAT)(disp);
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
#if 0
                        uncertainty_mm += rotateAndTranslatePoint(x_mm, y_mm, z_mm, voxelData.pos_mm, voxelData.rot);
#else
                        const float32_t l_Zmm = (voxelData.rot.zx * x_mm) + (voxelData.rot.zy * y_mm) + (voxelData.rot.zz * z_mm);
                        const float32_t l_Xmm = (voxelData.rot.xx * x_mm) + (voxelData.rot.xy * y_mm) + (voxelData.rot.xz * z_mm);
                        const float32_t l_Ymm = (voxelData.rot.yx * x_mm) + (voxelData.rot.yy * y_mm) + (voxelData.rot.yz * z_mm);
                        
                        uncertainty_mm += ODOMETRY_UNCERTAINTY_RAD * (fabs_(x_mm) + fabs_(y_mm) + fabs_(z_mm));

                        z_mm = l_Zmm + voxelData.pos_mm.z;
                        x_mm = l_Xmm + voxelData.pos_mm.x;
                        y_mm = l_Ymm + voxelData.pos_mm.y;
#endif

#if 0
                        // voxelise
                        Vec3f pos = { x_mm, y_mm, z_mm };
                        addVoxel(l_scratch, l_grid, pos, uncertainty_mm, pDispConf[dispOffset + dispIdx]);
#else
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

#if 0
                            INSERT_STATUS st = insertScratchHash(l_scratch, index, xGridCoor_px, yGridCoor_px, zGridCoor_px);
#else
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
#endif

                            if (st == INSERTED)
                            {
                                // This is a new voxel so fill all the initial data for it 
                                l_scratch.voxels[index].x_mm = (int16_t)offsetX_mm;
                                l_scratch.voxels[index].y_mm = (int16_t)offsetY_mm;
                                l_scratch.voxels[index].z_mm = (int16_t)offsetZ_mm;
                                l_scratch.voxels[index].uncertainty_cm = uncertainty_cm;
                                l_scratch.voxels[index].existence = pDispConf[dispOffset + dispIdx];
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
                                l_scratch.voxels[index].existence = MAX(l_scratch.voxels[index].existence, pDispConf[dispOffset + dispIdx]);

                                l_scratch.hashCount[index]++;
                            }
                            else
                            {
                                // This means we had a collision so we have to skip this point
                            }
                        }
#endif
                    }

                    x++;
                    n_px += 1.0F;
                }

                y++;
                azimAngle_rad += voxelData.azimDelta_rad;
            }
#if 0
            copyScratch(m_voxelBuffer, l_scratch);
#else
            // check that we won't exceed the buffer
            if ((m_voxelBuffer.numVoxels + l_scratch.numVoxels) < VOXEL_BUFFER_SIZE)
            {
                // copy out the data from scratch to our buffer
                for (uint32_t i = 0U; i < l_scratch.numVoxels; i++)
                {
                    const int32_t numOut = m_voxelBuffer.numVoxels;
                    m_voxelBuffer.hashIDs[numOut] = l_scratch.hashIDs[i];
                    m_voxelBuffer.voxels[numOut] = l_scratch.voxels[i];
                    m_voxelBuffer.count[numOut] = l_scratch.hashCount[i];
                    m_voxelBuffer.numVoxels = numOut + 1;
                }
            }
#endif
#endif
        }
    }
#endif

    /* Test the result */
    FILE *fpOut1 = fopen("./points_c.txt", "w");

#if 0
    fprintf(fpOut1, "%d\n",voxelData.cellIdx);
    fprintf(fpOut1, "%d\n",voxelData.cellWidth);
    fprintf(fpOut1, "%d\n",voxelData.cellHeight);
    fprintf(fpOut1, "%d\n",voxelData.startX);
    fprintf(fpOut1, "%d\n",voxelData.startY);
    fprintf(fpOut1, "%d\n",voxelData.lutShiftX);
    fprintf(fpOut1, "%d\n",voxelData.lutShiftY);
    fprintf(fpOut1, "%d\n",voxelData.lutWidth);
    fprintf(fpOut1, "%d\n",voxelData.imgWidth);
    fprintf(fpOut1, "%d\n",voxelData.minDisp);
    fprintf(fpOut1, "%d\n",voxelData.maxDisp);
    fprintf(fpOut1, "%f\n",voxelData.dispScaleInv);
    fprintf(fpOut1, "%f\n",voxelData.b_mm);
    fprintf(fpOut1, "%f\n",voxelData.startN_px);
    fprintf(fpOut1, "%f\n",voxelData.azimAngle_rad);
    fprintf(fpOut1, "%f\n",voxelData.azimDelta_rad);
    fprintf(fpOut1, "%f\n",voxelData.polarAngle_rad);
    fprintf(fpOut1, "%f\n",voxelData.polarDelta_rad);
    fprintf(fpOut1, "%f\n",voxelData.cenX_px);
    fprintf(fpOut1, "%f\n",voxelData.cenY_px);
    fprintf(fpOut1, "%f\n",voxelData.planarAspect);
    fprintf(fpOut1, "%f\n",voxelData.offset_mm);
    fprintf(fpOut1, "%f\n",voxelData.f_px);
    fprintf(fpOut1, "%f\n",voxelData.scaleN);
    fprintf(fpOut1, "%f\n",voxelData.scaleR);
    fprintf(fpOut1, "%f\n",voxelData.baselineError_mm);
    fprintf(fpOut1, "%f\n",voxelData.pos_mm.x);
    fprintf(fpOut1, "%f\n",voxelData.pos_mm.y);
    fprintf(fpOut1, "%f\n",voxelData.pos_mm.z);
    fprintf(fpOut1, "%f\n",voxelData.rot.xx);
    fprintf(fpOut1, "%f\n",voxelData.rot.xy);
    fprintf(fpOut1, "%f\n",voxelData.rot.xz);
    fprintf(fpOut1, "%f\n",voxelData.rot.yx);
    fprintf(fpOut1, "%f\n",voxelData.rot.yy);
    fprintf(fpOut1, "%f\n",voxelData.rot.yz);
    fprintf(fpOut1, "%f\n",voxelData.rot.zx);
    fprintf(fpOut1, "%f\n",voxelData.rot.zy);
    fprintf(fpOut1, "%f\n",voxelData.rot.zz);

    fprintf(fpOut1, "%d\n", roiMinX);
    fprintf(fpOut1, "%d\n", roiMaxX);
    fprintf(fpOut1, "%d\n", roiMinY);
    fprintf(fpOut1, "%d\n", roiMaxY);
    fprintf(fpOut1, "%d\n", width);
    fprintf(fpOut1, "%d\n", height);
    fprintf(fpOut1, "%d\n", dispOffset);
    fprintf(fpOut1, "%f\n", startAzim_rad);
    fprintf(fpOut1, "%d\n", gridWidth);
    fprintf(fpOut1, "%d\n", gridHeight);
    fprintf(fpOut1, "%d\n", gridCellWidth);
    fprintf(fpOut1, "%d\n", gridCellHeight);
    fprintf(fpOut1, "%d\n", m_dispScale);

    fprintf(fpOut1, "%d\n", g_mosVoxelGridSizes.mmPerVoxel);
    fprintf(fpOut1, "%f\n", g_mosVoxelGridSizes.mmPerVoxelInv);
    fprintf(fpOut1, "%d\n", g_mosVoxelGridSizes.voxelGridLength_vx);
    fprintf(fpOut1, "%d\n", g_mosVoxelGridSizes.voxelGridWidth_vx);
    fprintf(fpOut1, "%d\n", g_mosVoxelGridSizes.voxelGridHeight_vx);
    fprintf(fpOut1, "%d\n", g_mosVoxelGridSizes.numVoxels);
    fprintf(fpOut1, "%f\n", g_mosVoxelGridSizes.voxelGridZmin_mm);
    fprintf(fpOut1, "%f\n", g_mosVoxelGridSizes.voxelGridZmax_mm);
    fprintf(fpOut1, "%f\n", g_mosVoxelGridSizes.voxelGridXmax_mm);
    fprintf(fpOut1, "%f\n", g_mosVoxelGridSizes.voxelGridXmin_mm);
    fprintf(fpOut1, "%f\n", g_mosVoxelGridSizes.voxelGridYmax_mm);
    fprintf(fpOut1, "%f\n", g_mosVoxelGridSizes.voxelGridYmin_mm);
#else
    for(int i=0; i<m_voxelBuffer.numVoxels; i++) {
        fprintf(fpOut1, "%d %d %d\n",m_voxelBuffer.voxels[i].x_mm, m_voxelBuffer.voxels[i].y_mm, m_voxelBuffer.voxels[i].z_mm);
    }
#endif

    fclose(fpOut1);
}

//---------------------------------------------------------------------------
//
void 
fillVoxelsConicalCell(
    const uint16_t  *const pDispLeft,
    const uint8_t   *const pDispConf,
    const LutMeta *const    pLutMeta,
    const VoxelCellData&            voxelData
)
{
    int32_t y = voxelData.startY;
    FLOAT azimAngle_rad = voxelData.azimAngle_rad;

    VoxelScratch *l_scratch = m_voxelScratch;
    MOS_VoxelGridSizes_t& l_grid = g_mosVoxelGridSizes;

    // adjustment multiplier for displarity quality value
    const FLOAT invAbsB = 1.0F / fabs_(voxelData.b_mm);

    for (int32_t v = 0; v < voxelData.cellHeight; v++)
    {
        int32_t x = voxelData.startX;
        float32_t n_px = voxelData.startN_px + static_cast<float32_t>(x);

        const int32_t lutIdxLine = ((y >> voxelData.lutShiftY) * voxelData.lutWidth);

        // Use approximation of sine and cosine since no visible difference can be seen
        const float32_t sinAzim = sin_approx(azimAngle_rad);
        const float32_t cosAzim = cos_approx(azimAngle_rad);

        for (int32_t u = 0; u < voxelData.cellWidth; u++)
        {
            const int32_t lutIdx = lutIdxLine + (x >> voxelData.lutShiftX);
            const int32_t dispIdx = (y * voxelData.imgWidth) + x;
            const int32_t disp = static_cast<int32_t>(pDispLeft[dispIdx]);
            const LutMeta meta = pLutMeta[lutIdx];

            if ((disp >= voxelData.minDisp) && (disp <= voxelData.maxDisp) && (meta.structure != 0))
            {
                // The following calculation for the coordinates of the 
                // 3D point is based on similar triangles, assuming the 
                // height of the cone is 1
                //
                //                   |
                //                  /|
                //                 / |
                //              r'/  | 
                //               /   | n'
                //              /    |
                //             /     |
                //            /      |
                //           /  r    |
                //        P *---------          phi - half aperture angle of the cones
                //          \        | n
                //           \\      |         x, x' - distance from apex in units of cone height (1)
                //            \ \ x /|\ 
                //             \  \/ | \  <---- cone left
                //              \ / \|  \
                //               /   * O'\      O' - O = b (baseline length)
                //                \x'|\  
                //                 / | \  <---- cone right
                //                / \|  \
                //               /   * O \
                // 
                //  (I) n'/ 1 = r' / x    (II) (n' + b) / 1 = r' / x'
                //  
                //  and
                //  r = sin (phi) * r'     n = n' - cos(phi) * r'
                // 
                // yield the following equations
                //
                //  r = sin (phi) * x*x' * b / (x - x')   and n = (x' - cos (phi) * x*x') * b / (x - x')
                // 
                const float32_t dispf = static_cast<float32_t>(disp) * voxelData.dispScaleInv;
                const float32_t dispfInv = 1.0F / dispf;
                const float32_t ratio = voxelData.b_mm * dispfInv;  // b / (x - x')

                const float32_t n_px_right = (n_px - dispf); // x'
                const float32_t n_right_left_px = n_px_right * n_px;  // x*x'
                const float32_t z_tmp = (n_px_right - (n_right_left_px * voxelData.scaleN));
                const float32_t r_tmp = n_right_left_px * voxelData.scaleR;
                float32_t z_mm = (z_tmp * ratio) + voxelData.offset_mm;
                const float32_t r_mm = r_tmp * ratio;
                float32_t y_mm = sinAzim * r_mm;
                float32_t x_mm = cosAzim * r_mm;

                // Compute spatial uncertainty
                // We use the derivatives with respect to (x - x') = disparity of the two equations above to determine how
                // sensitive the calculated coordinates are to changes in the disparity and baseline

                // First assume a certain uncertainty in pixel space in units of pixels 
                const float32_t pixelUncertainty = ((SCALING_RATIO * STEREO_PIXEL_UNCERTAINTY) * 
                                                    voxelData.dispScaleInv) * static_cast<float32_t>(meta.scaling);
                // Calculate the derivatives for functions z_mm(disp) and r_mm(disp) plus z_mm(b_mm), r_mm(b_mm)
                // to calculate the sum of the partial derivatives wrt disparity (pixel inaccuracies) and baseline (odometry inaccuracies)
                const float32_t ratioDer = ratio * dispfInv;
                const float32_t ratioDerNpx = fabs_(n_px * ratioDer);
                const float32_t maxFactor = MAX(fabs_(voxelData.scaleR * n_px), fabs_(1.0F - (voxelData.scaleN * n_px)));
                // Take the maximum value and multiply by the pixel uncertainty
                float32_t uncertainty_mm = (pixelUncertainty * ratioDerNpx * maxFactor) +
                    (MAX(fabs_(r_tmp), fabs_(z_tmp)) * dispfInv * voxelData.baselineError_mm);

                // convert point into vehicle coordinate system
                uncertainty_mm += rotateAndTranslatePoint(x_mm, y_mm, z_mm, voxelData.pos_mm, voxelData.rot);

                // voxelise
                Vec3f pos = { x_mm, y_mm, z_mm };
                addVoxel(*l_scratch, l_grid, pos, uncertainty_mm, pDispConf[dispIdx]);
            }

            x++;
            n_px += 1.0F;
        }

        y++;
        azimAngle_rad += voxelData.azimDelta_rad;
    }
    copyScratch(m_voxelBuffer, *l_scratch);
}

void
fillVoxelsConical(
    const uint16_t  *const pDispLeft,
    const uint8_t   *const pDispConf,
    const LutMeta *const    pLutMeta
)
{
    //validate_ptr(pDispLeft);
    //validate_ptr(m_buff.pVoxel);
    //validate_(m_rec3dParamsSet);

    //const uint8_t camMask = 1U << (m_camIdx);
    int32_t roiMinX;
    int32_t roiMaxX;
    int32_t roiMinY;
    int32_t roiMaxY;
    int32_t width;
    int32_t height;

    // call function to retrieve these values due to HIS (Number of Calls CALLS)
    int32_t dispOffset;// = getDispRoiLimits(roiMinX, roiMaxX, roiMinY, roiMaxY, width);

    VoxelCellData voxelData;

    voxelData.lutShiftX; // = m_rec3d.lut.shiftX;
    voxelData.lutShiftY; // = m_rec3d.lut.shiftY;
    voxelData.lutWidth; // = m_rec3d.lut.width;
    voxelData.imgWidth; // = width;

    float32_t startAzim_rad; // = m_rec3d.conical.startAzimAngle_rad;
    voxelData.azimDelta_rad; // = m_rec3d.conical.azimDelta_rad;
    voxelData.startN_px; // = m_rec3d.conical.startX_px;

    voxelData.pos_mm; // = m_rec3d.conical.viewPos_mm;
    voxelData.rot; // = m_rec3d.conical.viewToWorldRot;
    voxelData.b_mm; // = -m_rec.b_mm; // negate due to polar angle definition

    voxelData.minDisp; // = m_rec.minDisp;
    voxelData.maxDisp; // = m_rec.maxDisp;
    voxelData.dispScaleInv; // = m_dispScaleInv;
    voxelData.offset_mm; // = m_rec.offset_mm;
    voxelData.f_px; // = m_rec.f_px;
    voxelData.baselineError_mm; // = static_cast<FLOAT>(m_rec3d.assumedBaselineError_mm);

    voxelData.scaleN; // = cos_approx(m_rec3d.conical.apertureHalfAngle_rad) * m_rec3d.conical.pixelScale;
    voxelData.scaleR; // = sin_approx(m_rec3d.conical.apertureHalfAngle_rad) * m_rec3d.conical.pixelScale;

    int32_t  gridWidth; // = m_dispGrid.width;
    int32_t  gridHeight; // = m_dispGrid.height;
    int32_t  gridCellWidth; // = m_dispGrid.cellWidth;
    int32_t  gridCellHeight; // = m_dispGrid.cellHeight;

    FILE *fpIn4;

    fpIn4 = fopen("./dump/init_input.bin", "rb");

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

    printf("W:%d H:%d\n", width, height);

    fclose(fpIn4);

    for (int32_t j = 0; j < gridHeight; j++)
    {
        for (int32_t i = 0; i < gridWidth; i++)
        {
            voxelData.startY = (j * gridCellHeight) + roiMinY;
            voxelData.startX = (i * gridCellWidth) + roiMinX;
            voxelData.cellIdx = (j * gridWidth) + i;

            // If we are the last cell in horizontal or vertical direction the size might be different
            voxelData.cellHeight = (j == (gridHeight - 1)) ? (roiMaxY + 1) - voxelData.startY : gridCellHeight;
            voxelData.cellWidth = (i == (gridWidth - 1)) ? (roiMaxX + 1) - voxelData.startX : gridCellWidth;

            // The azimuthal angle is mapped to the Y-coordinate
            voxelData.azimAngle_rad = startAzim_rad + (f32_(voxelData.startY) * voxelData.azimDelta_rad);

            resetDispGridHash(*m_voxelScratch);
            fillVoxelsConicalCell(pDispLeft + dispOffset, pDispConf + dispOffset, pLutMeta, voxelData);
        }
    }
}

//---------------------------------------------------------------------------
//
int main()
{
    uint16_t *pDispLeft = (uint16_t *)malloc(512000);
    uint8_t *pDispConf = (uint8_t *)malloc(256000);
    LutMeta *pLutMeta = (LutMeta *)malloc(8282);
    m_voxelBuffer.voxels = (VoxelPoint *)malloc(VOXEL_BUFFER_SIZE*sizeof(VoxelPoint));
    m_voxelBuffer.hashIDs = (uint32_t *)malloc(VOXEL_BUFFER_SIZE*sizeof(uint32_t));
    m_voxelBuffer.count = (uint8_t *)malloc(VOXEL_BUFFER_SIZE*sizeof(uint8_t));
    m_voxelScratch = (VoxelScratch *)malloc(sizeof(VoxelScratch));

    // FILE IO
    FILE *fpIn1, *fpIn2, *fpIn3;

    fpIn1 = fopen("./dump/pDispLeft_input.bin", "rb");
    fpIn2 = fopen("./dump/pDispConf_input.bin", "rb");
    fpIn3 = fopen("./dump/pLutMeta_input.bin", "rb");

    fread(pDispLeft, 1, 512000, fpIn1);
    fread(pDispConf, 1, 256000, fpIn2);
    fread(pLutMeta, 1, 8282, fpIn3);

    fclose(fpIn1);
    fclose(fpIn2);
    fclose(fpIn3);

    m_voxelBuffer.numVoxels = 0;

    fillVoxelsCylindrical(pDispLeft, pDispConf, pLutMeta); 

    printf("numVoxels : %d\n", m_voxelBuffer.numVoxels);

    free(pDispLeft);
    free(pDispConf);
    free(pLutMeta);
    free(m_voxelBuffer.voxels);
    free(m_voxelBuffer.hashIDs);
    free(m_voxelBuffer.count);
    free(m_voxelScratch);

    return(0);
}