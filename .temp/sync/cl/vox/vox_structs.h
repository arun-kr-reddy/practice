#ifndef __VOX_STRUCTS_H
#define __VOX_STRUCTS_H

#ifdef __OPENCL_C_VERSION__
    typedef unsigned char   uint8_t;
    typedef signed char     int8_t;
    typedef unsigned short  uint16_t;
    typedef signed short    int16_t;
    typedef unsigned int    uint32_t;
    typedef signed int      int32_t;

    typedef float float32_t;
    typedef float FLOAT;
#else
    typedef cl_uchar    uint8_t;
    typedef cl_char     int8_t;
    typedef cl_ushort   uint16_t;
    typedef cl_short    int16_t;
    typedef cl_uint     uint32_t;
    typedef cl_int      int32_t;

    typedef cl_float    float32_t;
    typedef cl_float    FLOAT;
#endif

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
#define fabs_               fabs
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define ABS(x) (((x) > 0) ? (x) : ((-1)*(x)))
#define M_PI            3.14159265358979323846
#define M_PI_2          1.57079632679489661923

//---------------------------------------------------------------------------
//
typedef struct __attribute__ ((packed))
{
    float32_t x;    //!< X component
    float32_t y;    //!< Y component
    float32_t z;    //!< Z component
}Vec3f;

typedef struct __attribute__ ((packed))
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

typedef struct __attribute__ ((packed))
{
    uint8_t         structure;          //!< Image structure based on Harris feature score (currently fixed value)
    uint8_t         scaling;            //!< Scaling factor, 64 - means 1.0
}LutMeta;

typedef struct __attribute__ ((packed))
{
    int16_t             x_mm;               //!< X offset in mm
    int16_t             y_mm;               //!< Y offset in mm
    int16_t             z_mm;               //!< Z offset in mm
    uint8_t             uncertainty_cm;     //!< Spatial uncertainty of the point in mm
    uint8_t             existence;          //!< STV confidence
}VoxelPoint;

typedef struct __attribute__ ((packed))
{
    VoxelPoint voxels[VOXEL_SCRATCH_SIZE];          //!< Voxel coordinate offsets
    uint32_t   hashIDs[VOXEL_SCRATCH_SIZE];         //!< Hash ID value used to identify a hash entry
    uint8_t    hashCount[VOXEL_SCRATCH_SIZE];       //!< Number of 3D points added to this location
    uint8_t    hashTable[VOXEL_SCRATCH_HASH_SIZE];  //!< Hash table used to find existing entries
    uint32_t   numVoxels;                           //!< Number of voxels contained 
}VoxelScratch;

typedef struct __attribute__ ((packed))
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

typedef struct __attribute__ ((packed))
{
    VoxelPoint *voxels;         //!< Voxel coordinate offsets
    uint32_t   *hashIDs;        //!< Hash ID for each entry
    uint8_t    *count;          //!< Number of added 3D points for each entry
    uint32_t   numVoxels;       //!< Number of voxels contained 
}VoxelBuffer;

typedef struct __attribute__ ((packed))
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

typedef struct __attribute__ ((packed))
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
    FLOAT       pos_mm_x;                     //!< View position in vehicle coordinates
    FLOAT       pos_mm_y;                               
    FLOAT       pos_mm_z;                                           
    FLOAT       rot_xx;                        //!< View to vehicle rotation
    FLOAT       rot_xy;                                             
    FLOAT       rot_xz;                                 
    FLOAT       rot_yx;                                     
    FLOAT       rot_yy;                                     
    FLOAT       rot_yz;                         
    FLOAT       rot_zx;                                 
    FLOAT       rot_zy;                         
    FLOAT       rot_zz;                                 

    int32_t     roiMinX;
    int32_t     roiMaxX;
    int32_t     roiMinY;
    int32_t     roiMaxY;
    int32_t     width;
    int32_t     height;
    int32_t     dispOffset;
    float32_t   startAzim_rad;
    int32_t     gridWidth;
    int32_t     gridHeight;
    int32_t     gridCellWidth;
    int32_t     gridCellHeight;
    int32_t     m_dispScale;

    int32_t     mmPerVoxel;             
    FLOAT       mmPerVoxelInv;          
    int32_t     voxelGridLength_vx;     
    int32_t     voxelGridWidth_vx;      
    int32_t     voxelGridHeight_vx;     
    int32_t     numVoxels;              
    FLOAT       voxelGridZmin_mm;       
    FLOAT       voxelGridZmax_mm;       
    FLOAT       voxelGridXmax_mm;       
    FLOAT       voxelGridXmin_mm;       
    FLOAT       voxelGridYmax_mm;       
    FLOAT       voxelGridYmin_mm;       
}const_args;

enum INSERT_STATUS
{
    INSERTED = 0,       //!< The element was inserted
    ALREADY_EXISTS,     //!< The element already exists
    COLLISION           //!< The element could not be inserted since no free spot could be found
};

//---------------------------------------------------------------------------

#endif