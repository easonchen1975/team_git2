/*
 * Copyright (c) 2016 by Cadence Design Systems, Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of
 * Cadence Design Systems Inc.  They may be adapted and modified by bona fide
 * purchasers for internal use, but neither the original nor any adapted
 * or modified version may be disclosed or distributed to third parties
 * in any manner, medium, or form, in whole or in part, without the prior
 * written consent of Cadence Design Systems Inc.  This software and its
 * derivatives are to be executed solely on products incorporating a Cadence
 * Design Systems processor.
 */

#ifndef __TILE_MANAGER_H__
#define __TILE_MANAGER_H__

//#define XV_EMULATE_DMA //Enable, XV_EMULATE_DMA, to run CSTUB on Xplorer
#if !defined(__XTENSA__) || !defined(XCHAL_HAVE_VISION) || !XCHAL_HAVE_VISION
#ifndef XV_EMULATE_DMA
  #define XV_EMULATE_DMA
#endif
#endif

#if defined (__cplusplus)
extern "C"
{
#endif
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "tmUtils.h"

#ifndef XV_EMULATE_DMA
//#define IDMA_APP_USE_XTOS
#include <xtensa/idma.h>
#include <xtensa/tie/xt_misc.h>
#endif //XV_EMULATE_DMA

// MAX limits for number of tiles, frames memory banks and dma queue length
#define MAX_NUM_MEM_BANKS         8
#define MAX_NUM_TILES             32
#define MAX_NUM_FRAMES            8
#define MAX_NUM_DMA_QUEUE_LENGTH  32 // Optimization, multiple of 2

// Bank colors. XV_MEM_BANK_COLOR_ANY is an unlikely enum value
#define XV_MEM_BANK_COLOR_0    0x0
#define XV_MEM_BANK_COLOR_1    0x1
#define XV_MEM_BANK_COLOR_2    0x2
#define XV_MEM_BANK_COLOR_3    0x3
#define XV_MEM_BANK_COLOR_4    0x4
#define XV_MEM_BANK_COLOR_5    0x5
#define XV_MEM_BANK_COLOR_6    0x6
#define XV_MEM_BANK_COLOR_7    0x7
#define XV_MEM_BANK_COLOR_ANY  0xBEEDDEAF

// Edge padding format
#define FRAME_ZERO_PADDING      0
#define FRAME_CONSTANT_PADDING  1
#define FRAME_EDGE_PADDING      2
#ifndef IVP_SIMD_WIDTH
#define IVP_SIMD_WIDTH          XCHAL_IVPN_SIMD_WIDTH
#endif
#define IVP_ALIGNMENT           0x1F
#define XVTM_MIN(a, b)  (((a) < (b)) ? (a) : (b))

#define XVTM_DUMMY_DMA_INDEX  -2
#define XVTM_ERROR            -1
#define XVTM_SUCCESS          0

#define ENABLE_PRINTF
#ifdef ENABLE_PRINTF
#define TM_PRINT(...)  do { printf(__VA_ARGS__); } while (0)
#else
#define TM_PRINT(...)  do {} while (0)
#endif

//#define TM_LOG
#ifdef TM_LOG
#define TILE_MANAGER_LOG_FILE_NAME  "tileManager.log"
#define TM_LOG_PRINT(fmt, ...)  do { fprintf(pxvTM->tm_log_fp, fmt, __VA_ARGS__); } while (0)
#else
#define TM_LOG_PRINT(fmt, ...)  do {} while (0)
#endif

#ifdef XV_EMULATE_DMA

#if !defined(IDMA_1D_DESC) || !defined(IDMA_2D_DESC)
#define IDMA_1D_DESC 1
#define IDMA_2D_DESC 2
#endif

#if !defined(MAX_BLOCK_2) || !defined(MAX_BLOCK_4) || !defined(MAX_BLOCK_8) || !defined(MAX_BLOCK_16)
#define MAX_BLOCK_2  0
#define MAX_BLOCK_4  1
#define MAX_BLOCK_8  2
#define MAX_BLOCK_16 3
#endif

#ifdef idma_type_t
#undef idma_type_t
#endif
#define idma_type_t int32_t

#ifdef idma_max_block_t
#undef idma_max_block_t
#endif
#define idma_max_block_t int32_t

#if !defined(TICK_CYCLES_1) || !defined(TICK_CYCLES_2)
#define TICK_CYCLES_1          1
#define TICK_CYCLES_2          2
#endif

#if !defined(DESC_NOTIFY_W_INT)
#define DESC_NOTIFY_W_INT      0
#endif

#if !defined(XT_ISS_CYCLE_ACCURATE) || !defined(XT_ISS_FUNCTIONAL)
#define XT_ISS_CYCLE_ACCURATE  0
#define XT_ISS_FUNCTIONAL      1
#endif

#ifdef xt_iss_switch_mode
#undef xt_iss_switch_mode
#endif
#define xt_iss_switch_mode(a)

#ifdef xt_profile_enable
#undef xt_profile_enable
#endif
#define xt_profile_enable(void)

#ifdef xt_profile_enable
#undef xt_profile_enable
#endif
#define xt_profile_disable(void)

#ifdef IDMA_DISABLE_INTS
#undef IDMA_DISABLE_INTS
#endif
#define IDMA_DISABLE_INTS(void)

#ifdef IDMA_ENABLE_INTS
#undef IDMA_ENABLE_INTS
#endif
#define IDMA_ENABLE_INTS(void)

#ifdef XT_RSR_CCOUNT
#undef XT_RSR_CCOUNT
#endif
#define XT_RSR_CCOUNT() 0

#ifdef no_reorder
#undef no_reorder
#endif
#define no_reorder

#ifndef IDMA_BUFFER_DEFINE
#define IDMA_BUFFER_DEFINE(name, n, type) void* name = NULL;
#endif

#ifdef idma_copy_2d_desc
#undef idma_copy_2d_desc
#endif
#define idma_copy_2d_desc copy2d

#ifdef idma_desc_done
#undef idma_desc_done
#endif
#define idma_desc_done dma_desc_done

#ifdef idma_sleep
#undef idma_sleep
#endif
#define idma_sleep dma_sleep

#ifndef idma_status_t
#define idma_status_t int32_t
#endif

#ifndef idma_max_pif_t
#define idma_max_pif_t int32_t
#endif

#ifndef idma_error_details_t
#define idma_error_details_t int32_t
#endif

#ifndef idma_buffer_t
#define idma_buffer_t int32_t
#endif

typedef void (*idma_callback_fn)(void*);

#ifdef idma_err_callback_fn
#undef idma_err_callback_fn
#endif
#if defined(__XTENSA__)
#define idma_err_callback_fn void *
#else
typedef void (*idma_err_callback_fn)(/*const*/ idma_error_details_t*);
#endif

#ifndef xvmem_mgr_t
#define xvmem_mgr_t int32_t
#endif

#ifndef xvmem_status_t
#define xvmem_status_t int32_t
#endif


int32_t dma_desc_done(int32_t index);

#ifdef idma_buffer_error_details
#undef idma_buffer_error_details
#endif
#define idma_buffer_error_details     dma_buffer_error_details
#endif

typedef enum
{
  TILE_UNALIGNED,
  EDGE_ALIGNED_32,
  DATA_ALIGNED_32,
  EDGE_ALIGNED_64,
  DATA_ALIGNED_64,
} buffer_align_type_t;

typedef enum
{
  XV_ERROR_SUCCESS            = 0,
  XV_ERROR_TILE_MANAGER_NULL  = 1,
  XV_ERROR_POINTER_NULL       = 2,
  XV_ERROR_FRAME_NULL         = 3,
  XV_ERROR_TILE_NULL          = 4,
  XV_ERROR_BUFFER_NULL        = 5,
  XV_ERROR_ALLOC_FAILED       = 6,
  XV_ERROR_FRAME_BUFFER_FULL  = 7,
  XV_ERROR_TILE_BUFFER_FULL   = 8,
  XV_ERROR_DIMENSION_MISMATCH = 9,
  XV_ERROR_BUFFER_OVERFLOW    = 10,
  XV_ERROR_BAD_ARG            = 11,
  XV_ERROR_FILE_OPEN          = 12,
  XV_ERROR_DMA_INIT           = 13,
  XV_ERROR_XVMEM_INIT          = 14,
  XV_ERROR_IDMA               = 15
}xvError_t;


typedef struct xvFrameStruct
{
  void     *pFrameBuff;
  uint32_t frameBuffSize;
  void     *pFrameData;
  int32_t  frameWidth;
  int32_t  frameHeight;
  int32_t  framePitch;
  uint8_t  pixelRes;
  uint8_t  numChannels;
  uint8_t  leftEdgePadWidth;
  uint8_t  topEdgePadHeight;
  uint8_t  rightEdgePadWidth;
  uint8_t  bottomEdgePadHeight;
  uint8_t  paddingType;
  uint8_t  paddingVal;
} xvFrame, *xvpFrame;


#define XV_ARRAY_FIELDS \
  void     *pBuffer;    \
  uint32_t bufferSize;  \
  void *pData;          \
  int32_t width;        \
  int32_t pitch;        \
  uint32_t status;      \
  uint16_t type;        \
  uint16_t height;


typedef struct xvArrayStruct
{
  XV_ARRAY_FIELDS
} xvArray, *xvpArray;


typedef struct xvTileStruct
{
  XV_ARRAY_FIELDS
  xvFrame             *pFrame;
  int32_t             x;
  int32_t             y;
  uint16_t            tileEdgeLeft;
  uint16_t            tileEdgeTop;
  uint16_t            tileEdgeRight;
  uint16_t            tileEdgeBottom;
  int32_t             dmaIndex;
  int32_t             reuseCount;
  struct xvTileStruct *pPrevTile;
} xvTile, *xvpTile;


typedef struct xvTileManagerStruct
{
  // iDMA related
  void    *pdmaObj;
  int32_t tileDMApendingCount;       // Incremented when new request is added. Decremented when request is completed.
  int32_t tileDMAstartIndex;         // Incremented when request is completed
  xvTile  *tileProcQueue[MAX_NUM_DMA_QUEUE_LENGTH];

  // Mem Banks
#ifndef XV_EMULATE_DMA
  int32_t    numMemBanks;                       // Number of memory banks/pools
  xvmem_mgr_t memBankMgr[MAX_NUM_MEM_BANKS];    // xvmem memory manager, one for each bank
  void       *pMemBankStart[MAX_NUM_MEM_BANKS]; // Start address of bank
  int32_t    memBankSize[MAX_NUM_MEM_BANKS];    // size of each bank
#endif

#ifdef TM_LOG
  FILE *tm_log_fp;
#endif
  // Tiles and frame allocation
  xvTile    tileArray[MAX_NUM_TILES];
  xvFrame   frameArray[MAX_NUM_FRAMES];
  int32_t   tileAllocFlags[(MAX_NUM_TILES + 31) / 32];      // Each bit of tileAllocFlags and frameAllocFlags
  int32_t   frameAllocFlags[(MAX_NUM_FRAMES + 31) / 32];    // indicates if a particular tile/frame is allocated
  int32_t   tileCount;
  int32_t   frameCount;
  xvError_t errFlag;
  xvError_t idmaErrorFlag;
} xvTileManager;

extern xvTileManager *_ptr_xv_tile_manager;
#define XVTM_RAISE_EXCEPTION() _ptr_xv_tile_manager->idmaErrorFlag = XV_ERROR_IDMA;
#define XVTM_RESET_EXCEPTION() _ptr_xv_tile_manager->idmaErrorFlag = XV_ERROR_SUCCESS;
#define XVTM_IS_TRANSFER_SUCCESS(pxvTM) (((pxvTM)->idmaErrorFlag == XV_ERROR_SUCCESS) ? 1 : 0)
/*****************************************
*   Individual status flags definitions
*****************************************/

#define XV_TILE_STATUS_DMA_ONGOING                 (0x01 << 0)
#define XV_TILE_STATUS_LEFT_EDGE_PADDING_NEEDED    (0x01 << 1)
#define XV_TILE_STATUS_RIGHT_EDGE_PADDING_NEEDED   (0x01 << 2)
#define XV_TILE_STATUS_TOP_EDGE_PADDING_NEEDED     (0x01 << 3)
#define XV_TILE_STATUS_BOTTOM_EDGE_PADDING_NEEDED  (0x01 << 4)
#define XV_TILE_STATUS_EDGE_PADDING_NEEDED    \
  (XV_TILE_STATUS_LEFT_EDGE_PADDING_NEEDED |  \
   XV_TILE_STATUS_RIGHT_EDGE_PADDING_NEEDED | \
   XV_TILE_STATUS_TOP_EDGE_PADDING_NEEDED |   \
   XV_TILE_STATUS_BOTTOM_EDGE_PADDING_NEEDED)


/*****************************************
*   Data type definitions
*****************************************/

#define XV_TYPE_SIGNED_BIT         (1 << 15)
#define XV_TYPE_TILE_BIT           (1 << 14)

#define XV_TYPE_ELEMENT_SIZE_BITS  10
#define XV_TYPE_ELEMENT_SIZE_MASK  ((1 << XV_TYPE_ELEMENT_SIZE_BITS) - 1)
#define XV_TYPE_CHANNELS_BITS      2
#define XV_TYPE_CHANNELS_MASK      (((1 << XV_TYPE_CHANNELS_BITS) - 1) << XV_TYPE_ELEMENT_SIZE_BITS)

// XV_MAKETYPE accepts 3 parameters
// 1: flag: Denotes whether the entity is a tile (XV_TYPE_TILE_BIT is set) or an array (XV_TYPE_TILE_BIT is not set),
//    and also if the data is a signed(XV_TYPE_SIGNED_BIT is set) or unsigned(XV_TYPE_SIGNED_BIT is not set).
// 2: depth: Denotes number of bytes per pel.
//    1 implies the data is 8bit, 2 implies the data is 16bit and 4 implies the data is 32bit.
// 3: Denotes number of channels.
//    1 implies gray scale, 3 implies RGB

#define XV_MAKETYPE(flags, depth, channels)  (((depth) * (channels)) | (((channels) - 1) << XV_TYPE_ELEMENT_SIZE_BITS) | (flags))
#define XV_CUSTOMTYPE(type)                  XV_MAKETYPE(0, sizeof(type), 1)

#define XV_TYPE_ELEMENT_SIZE(type)           ((type) & (XV_TYPE_ELEMENT_SIZE_MASK))
#define XV_TYPE_ELEMENT_TYPE(type)           ((type) & (XV_TYPE_SIGNED_BIT | XV_TYPE_CHANNELS_MASK | XV_TYPE_ELEMENT_SIZE_MASK))
#define XV_TYPE_IS_TILE(type)                ((type) & (XV_TYPE_TILE_BIT))
#define XV_TYPE_IS_SIGNED(type)              ((type) & (XV_TYPE_SIGNED_BIT))
#define XV_TYPE_CHANNELS(type)               ((((type) & (XV_TYPE_CHANNELS_MASK)) >> (XV_TYPE_ELEMENT_SIZE_BITS)) + 1)

// Common XV_MAKETYPEs
#define XV_U8         XV_MAKETYPE(0, 1, 1)
#define XV_U16        XV_MAKETYPE(0, 2, 1)
#define XV_U32        XV_MAKETYPE(0, 4, 1)

#define XV_S8         XV_MAKETYPE(XV_TYPE_SIGNED_BIT, 1, 1)
#define XV_S16        XV_MAKETYPE(XV_TYPE_SIGNED_BIT, 2, 1)
#define XV_S32        XV_MAKETYPE(XV_TYPE_SIGNED_BIT, 4, 1)

#define XV_ARRAY_U8   XV_U8
#define XV_ARRAY_S8   XV_S8
#define XV_ARRAY_U16  XV_U16
#define XV_ARRAY_S16  XV_S16
#define XV_ARRAY_U32  XV_U32
#define XV_ARRAY_S32  XV_S32

#define XV_TILE_U8    (XV_U8 | XV_TYPE_TILE_BIT)
#define XV_TILE_S8    (XV_S8 | XV_TYPE_TILE_BIT)
#define XV_TILE_U16   (XV_U16 | XV_TYPE_TILE_BIT)
#define XV_TILE_S16   (XV_S16 | XV_TYPE_TILE_BIT)
#define XV_TILE_U32   (XV_U32 | XV_TYPE_TILE_BIT)
#define XV_TILE_S32   (XV_S32 | XV_TYPE_TILE_BIT)

/*****************************************
*    Frame Access Macros
*****************************************/

#define XV_FRAME_GET_BUFF_PTR(pFrame)                   ((pFrame)->pFrameBuff)
#define XV_FRAME_SET_BUFF_PTR(pFrame, pBuff)            (pFrame)->pFrameBuff = ((void *) (pBuff))

#define XV_FRAME_GET_BUFF_SIZE(pFrame)                  ((pFrame)->frameBuffSize)
#define XV_FRAME_SET_BUFF_SIZE(pFrame, buffSize)        (pFrame)->frameBuffSize = ((uint32_t) (buffSize))

#define XV_FRAME_GET_DATA_PTR(pFrame)                   ((pFrame)->pFrameData)
#define XV_FRAME_SET_DATA_PTR(pFrame, pData)            (pFrame)->pFrameData = ((void *) (pData))

#define XV_FRAME_GET_WIDTH(pFrame)                      ((pFrame)->frameWidth)
#define XV_FRAME_SET_WIDTH(pFrame, width)               (pFrame)->frameWidth = ((int32_t) (width))

#define XV_FRAME_GET_HEIGHT(pFrame)                     ((pFrame)->frameHeight)
#define XV_FRAME_SET_HEIGHT(pFrame, height)             (pFrame)->frameHeight = ((int32_t) (height))

#define XV_FRAME_GET_PITCH(pFrame)                      ((pFrame)->framePitch)
#define XV_FRAME_SET_PITCH(pFrame, pitch)               (pFrame)->framePitch = ((int32_t) (pitch))
#define XV_FRAME_GET_PITCH_IN_BYTES(pFrame)             ((pFrame)->framePitch * (pFrame)->pixRes)

#define XV_FRAME_GET_PIXEL_RES(pFrame)                  ((pFrame)->pixelRes)
#define XV_FRAME_SET_PIXEL_RES(pFrame, pixRes)          (pFrame)->pixelRes = ((uint8_t) (pixRes))

#define XV_FRAME_GET_NUM_CHANNELS(pFrame)               ((pFrame)->numChannels)
#define XV_FRAME_SET_NUM_CHANNELS(pFrame, pixelFormat)  (pFrame)->numChannels = ((uint8_t) (pixelFormat))

#define XV_FRAME_GET_EDGE_WIDTH(pFrame)                 ((pFrame)->leftEdgePadWidth < (pFrame)->rightEdgePadWidth ? (pFrame)->leftEdgePadWidth : (pFrame)->rightEdgePadWidth)
#define XV_FRAME_SET_EDGE_WIDTH(pFrame, padWidth)       (pFrame)->leftEdgePadWidth = (pFrame)->rightEdgePadWidth = ((uint8_t) (padWidth))

#define XV_FRAME_GET_EDGE_HEIGHT(pFrame)                ((pFrame)->topEdgePadHeight < (pFrame)->bottomEdgePadHeight ? (pFrame)->topEdgePadHeight : (pFrame)->bottomEdgePadHeight)
#define XV_FRAME_SET_EDGE_HEIGHT(pFrame, padHeight)     (pFrame)->topEdgePadHeight = (pFrame)->bottomEdgePadHeight = ((uint8_t) (padHeight))

#define XV_FRAME_GET_EDGE_LEFT(pFrame)                  ((pFrame)->leftEdgePadWidth)
#define XV_FRAME_SET_EDGE_LEFT(pFrame, padWidth)        (pFrame)->leftEdgePadWidth = ((uint8_t) (padWidth))

#define XV_FRAME_GET_EDGE_RIGHT(pFrame)                 ((pFrame)->rightEdgePadWidth)
#define XV_FRAME_SET_EDGE_RIGHT(pFrame, padWidth)       (pFrame)->rightEdgePadWidth = ((uint8_t) (padWidth))

#define XV_FRAME_GET_EDGE_TOP(pFrame)                   ((pFrame)->topEdgePadHeight)
#define XV_FRAME_SET_EDGE_TOP(pFrame, padHeight)        (pFrame)->topEdgePadHeight = ((uint8_t) (padHeight))

#define XV_FRAME_GET_EDGE_BOTTOM(pFrame)                ((pFrame)->bottomEdgePadWidth)
#define XV_FRAME_SET_EDGE_BOTTOM(pFrame, padHeight)     (pFrame)->bottomEdgePadHeight = ((uint8_t) (padHeight))

#define XV_FRAME_GET_PADDING_TYPE(pFrame)               ((pFrame)->paddingType)
#define XV_FRAME_SET_PADDING_TYPE(pFrame, padType)      (pFrame)->paddingType = (padType)

#define XV_FRAME_GET_PADDING_VALUE(pFrame)              ((pFrame)->paddingVal)
#define XV_FRAME_SET_PADDING_VALUE(pFrame, padVal)      (pFrame)->paddingVal = (padVal)

/*****************************************
*    Array Access Macros
*****************************************/

#define XV_ARRAY_GET_BUFF_PTR(pArray)              ((pArray)->pBuffer)
#define XV_ARRAY_SET_BUFF_PTR(pArray, pBuff)       (pArray)->pBuffer = ((void *) (pBuff))

#define XV_ARRAY_GET_BUFF_SIZE(pArray)             ((pArray)->bufferSize)
#define XV_ARRAY_SET_BUFF_SIZE(pArray, buffSize)   (pArray)->bufferSize = ((uint32_t) (buffSize))

#define XV_ARRAY_GET_DATA_PTR(pArray)              ((pArray)->pData)
#define XV_ARRAY_SET_DATA_PTR(pArray, pArrayData)  (pArray)->pData = ((void *) (pArrayData))

#define XV_ARRAY_GET_WIDTH(pArray)                 ((pArray)->width)
#define XV_ARRAY_SET_WIDTH(pArray, value)          (pArray)->width = ((int32_t) (value))

#define XV_ARRAY_GET_PITCH(pArray)                 ((pArray)->pitch)
#define XV_ARRAY_SET_PITCH(pArray, value)          (pArray)->pitch = ((int32_t) (value))

#define XV_ARRAY_GET_HEIGHT(pArray)                ((pArray)->height)
#define XV_ARRAY_SET_HEIGHT(pArray, value)         (pArray)->height = ((uint16_t) (value))

#define XV_ARRAY_GET_STATUS_FLAGS(pArray)          ((pArray)->status)
#define XV_ARRAY_SET_STATUS_FLAGS(pArray, value)   (pArray)->status = ((uint8_t) (value))

#define XV_ARRAY_GET_TYPE(pArray)                  ((pArray)->type)
#define XV_ARRAY_SET_TYPE(pArray, value)           (pArray)->type = ((uint16_t) (value))

#define XV_ARRAY_GET_CAPACITY(pArray)              XV_ARRAY_GET_PITCH(pArray)
#define XV_ARRAY_SET_CAPACITY(pArray, value)       XV_ARRAY_SET_PITCH(pArray, value)

#define XV_ARRAY_GET_ELEMENT_TYPE(pArray)          XV_TYPE_ELEMENT_TYPE(XV_ARRAY_GET_TYPE(pArray))
#define XV_ARRAY_GET_ELEMENT_SIZE(pArray)          XV_TYPE_ELEMENT_SIZE(XV_ARRAY_GET_TYPE(pArray))
#define XV_ARRAY_IS_TILE(pArray)                   XV_TYPE_IS_TILE(XV_ARRAY_GET_TYPE(pArray))

#define XV_ARRAY_GET_AREA(pArray)                  (((pArray)->width) * ((int32_t) (pArray)->height))

/*****************************************
*    Tile Access Macros
*****************************************/

#define XV_TILE_GET_BUFF_PTR   XV_ARRAY_GET_BUFF_PTR
#define XV_TILE_SET_BUFF_PTR   XV_ARRAY_SET_BUFF_PTR

#define XV_TILE_GET_BUFF_SIZE  XV_ARRAY_GET_BUFF_SIZE
#define XV_TILE_SET_BUFF_SIZE  XV_ARRAY_SET_BUFF_SIZE

#define XV_TILE_GET_DATA_PTR   XV_ARRAY_GET_DATA_PTR
#define XV_TILE_SET_DATA_PTR   XV_ARRAY_SET_DATA_PTR

#define XV_TILE_GET_WIDTH      XV_ARRAY_GET_WIDTH
#define XV_TILE_SET_WIDTH      XV_ARRAY_SET_WIDTH

#define XV_TILE_GET_PITCH      XV_ARRAY_GET_PITCH
#define XV_TILE_SET_PITCH      XV_ARRAY_SET_PITCH
#define XV_TILE_GET_PITCH_IN_BYTES(pTile)  ((pTile)->pitch * (int32_t) ((pTile)->pFrame->pixRes))

#define XV_TILE_GET_HEIGHT        XV_ARRAY_GET_HEIGHT
#define XV_TILE_SET_HEIGHT        XV_ARRAY_SET_HEIGHT

#define XV_TILE_GET_STATUS_FLAGS  XV_ARRAY_GET_STATUS_FLAGS
#define XV_TILE_SET_STATUS_FLAGS  XV_ARRAY_SET_STATUS_FLAGS

#define XV_TILE_GET_TYPE          XV_ARRAY_GET_TYPE
#define XV_TILE_SET_TYPE          XV_ARRAY_SET_TYPE

#define XV_TILE_GET_ELEMENT_TYPE  XV_ARRAY_GET_ELEMENT_TYPE
#define XV_TILE_GET_ELEMENT_SIZE  XV_ARRAY_GET_ELEMENT_SIZE
#define XV_TILE_IS_TILE           XV_ARRAY_IS_TILE

#define XV_TILE_RESET_DMA_INDEX(pTile)                         ((pTile)->dmaIndex = 0)
#define XV_TILE_RESET_PREVIOUS_TILE(pTile)                     ((pTile)->pPrevTile = NULL)
#define XV_TILE_RESET_REUSE_COUNT(pTile)                       ((pTile)->reuseCount = 0)

#define XV_TILE_GET_FRAME_PTR(pTile)                           ((pTile)->pFrame)
#define XV_TILE_SET_FRAME_PTR(pTile, ptrFrame)                 (pTile)->pFrame = ((xvFrame *) (ptrFrame))

#define XV_TILE_GET_X_COORD(pTile)                             ((pTile)->x)
#define XV_TILE_SET_X_COORD(pTile, xcoord)                     (pTile)->x = ((int32_t) (xcoord))

#define XV_TILE_GET_Y_COORD(pTile)                             ((pTile)->y)
#define XV_TILE_SET_Y_COORD(pTile, ycoord)                     (pTile)->y = ((int32_t) (ycoord))

#define XV_TILE_GET_EDGE_LEFT(pTile)                           ((pTile)->tileEdgeLeft)
#define XV_TILE_SET_EDGE_LEFT(pTile, edgeWidth)                (pTile)->tileEdgeLeft = ((uint16_t) (edgeWidth))

#define XV_TILE_GET_EDGE_RIGHT(pTile)                          ((pTile)->tileEdgeRight)
#define XV_TILE_SET_EDGE_RIGHT(pTile, edgeWidth)               (pTile)->tileEdgeRight = ((uint16_t) (edgeWidth))

#define XV_TILE_GET_EDGE_TOP(pTile)                            ((pTile)->tileEdgeTop)
#define XV_TILE_SET_EDGE_TOP(pTile, edgeHeight)                (pTile)->tileEdgeTop = ((uint16_t) (edgeHeight))

#define XV_TILE_GET_EDGE_BOTTOM(pTile)                         ((pTile)->tileEdgeBottom)
#define XV_TILE_SET_EDGE_BOTTOM(pTile, edgeHeight)             (pTile)->tileEdgeBottom = ((uint16_t) (edgeHeight))

#define XV_TILE_GET_EDGE_WIDTH(pTile)                          (((pTile)->tileEdgeLeft < (pTile)->tileEdgeRight) ? (pTile)->tileEdgeLeft : (pTile)->tileEdgeRight)
#define XV_TILE_SET_EDGE_WIDTH(pTile, edgeWidth)               ((pTile)->tileEdgeLeft = (pTile)->tileEdgeRight = ((uint16_t) (edgeWidth)))

#define XV_TILE_GET_EDGE_HEIGHT(pTile)                         (((pTile)->tileEdgeTop < (pTile)->tileEdgeBottom) ? (pTile)->tileEdgeTop : (pTile)->tileEdgeBottom)
#define XV_TILE_SET_EDGE_HEIGHT(pTile, edgeHeight)             ((pTile)->tileEdgeTop = (pTile)->tileEdgeBottom = ((uint16_t) (edgeHeight)))

#define XV_TILE_CHECK_STATUS_FLAGS_DMA_ONGOING(pTile)          (((pTile)->status & XV_TILE_STATUS_DMA_ONGOING) > 0)
#define XV_TILE_CHECK_STATUS_FLAGS_EDGE_PADDING_NEEDED(pTile)  (((pTile)->status & XV_TILE_STATUS_EDGE_PADDING_NEEDED) > 0)

/***********************************
*    Other Marcos
***********************************/

#define XV_TILE_CHECK_VIRTUAL_FRAME(pTile)    ((pTile)->pFrame->pFrameBuff == NULL)
#define XV_FRAME_CHECK_VIRTUAL_FRAME(pFrame)  ((pFrame)->pFrameBuff == NULL)

// If type is zero, bytes per pixel is eight bits and number of channels is one
#define SETUP_TILE(pTile, pBuf, bufSize, pFrame, width, height, pitch, type, edgeWidth, edgeHeight, x, y, alignType)          \
  {                                                                                                                           \
    int32_t tileType, bytesPerPixel, channels, bytesPerPel;                                                                   \
    uint8_t *edgePtr = (uint8_t *) pBuf, *dataPtr;                                                                            \
    int alignment    = 63;                                                                                                    \
    if ((alignType == EDGE_ALIGNED_32) || (alignType == DATA_ALIGNED_32)) { alignment = 31; }                                 \
    tileType      = (type) ? (type) : XV_U8;                                                                                  \
    bytesPerPixel = XV_TYPE_ELEMENT_SIZE(tileType);                                                                           \
    channels      = XV_TYPE_CHANNELS(tileType);                                                                               \
    bytesPerPel   = bytesPerPixel / channels;                                                                                 \
    XV_TILE_SET_FRAME_PTR((xvTile *) (pTile), ((xvFrame *) (pFrame)));                                                        \
    XV_TILE_SET_BUFF_PTR((xvTile *) (pTile), (pBuf));                                                                         \
    XV_TILE_SET_BUFF_SIZE((xvTile *) (pTile), (bufSize));                                                                     \
    if ((alignType == EDGE_ALIGNED_32) || (alignType == EDGE_ALIGNED_64))                                                     \
    {                                                                                                                         \
      edgePtr = (uint8_t *) (((long) (pBuf) + alignment) & (~alignment));                                                     \
    }                                                                                                                         \
    XV_TILE_SET_DATA_PTR((xvTile *) (pTile), edgePtr + ((edgeHeight) * (pitch) * bytesPerPel + (edgeWidth) * bytesPerPixel)); \
    if ((alignType == DATA_ALIGNED_32) || (alignType == DATA_ALIGNED_64))                                                     \
    {                                                                                                                         \
      dataPtr = (uint8_t *) XV_TILE_GET_DATA_PTR((xvTile *) (pTile));                                                         \
      dataPtr = (uint8_t *) (((long) (dataPtr) + alignment) & (~alignment));                                                  \
      XV_TILE_SET_DATA_PTR((xvTile *) (pTile), dataPtr);                                                                      \
    }                                                                                                                         \
    XV_TILE_SET_WIDTH((xvTile *) (pTile), (width));                                                                           \
    XV_TILE_SET_HEIGHT((xvTile *) (pTile), (height));                                                                         \
    XV_TILE_SET_PITCH((xvTile *) (pTile), (pitch));                                                                           \
    XV_TILE_SET_TYPE((xvTile *) (pTile), (tileType | XV_TYPE_TILE_BIT));                                                      \
    XV_TILE_SET_EDGE_WIDTH((xvTile *) (pTile), (edgeWidth));                                                                  \
    XV_TILE_SET_EDGE_HEIGHT((xvTile *) (pTile), (edgeHeight));                                                                \
    XV_TILE_SET_X_COORD((xvTile *) (pTile), (x));                                                                             \
    XV_TILE_SET_Y_COORD((xvTile *) (pTile), (y));                                                                             \
    XV_TILE_SET_STATUS_FLAGS((xvTile *) (pTile), 0);                                                                          \
    XV_TILE_RESET_DMA_INDEX((xvTile *) (pTile));                                                                              \
    XV_TILE_RESET_PREVIOUS_TILE((xvTile *) (pTile));                                                                          \
    XV_TILE_RESET_REUSE_COUNT((xvTile *) (pTile));                                                                            \
  }

#define SETUP_FRAME(pFrame, pFrameBuffer, buffSize, width, height, pitch, padWidth, padHeight, pixRes, numCh, paddingType, paddingVal)     \
  {                                                                                                                                        \
    XV_FRAME_SET_BUFF_PTR((xvFrame *) (pFrame), (pFrameBuffer));                                                                           \
    XV_FRAME_SET_BUFF_SIZE((xvFrame *) (pFrame), (buffSize));                                                                              \
    XV_FRAME_SET_WIDTH((xvFrame *) (pFrame), (width));                                                                                     \
    XV_FRAME_SET_HEIGHT((xvFrame *) (pFrame), (height));                                                                                   \
    XV_FRAME_SET_PITCH((xvFrame *) (pFrame), (pitch));                                                                                     \
    XV_FRAME_SET_PIXEL_RES((xvFrame *) (pFrame), (pixRes));                                                                                \
    XV_FRAME_SET_DATA_PTR((xvFrame *) (pFrame), ((uint8_t *) (pFrameBuffer)) + ((pitch) * (padHeight) + (padWidth) * (numCh)) * (pixRes)); \
    XV_FRAME_SET_EDGE_WIDTH((xvFrame *) (pFrame), (padWidth));                                                                             \
    XV_FRAME_SET_EDGE_HEIGHT((xvFrame *) (pFrame), (padHeight));                                                                           \
    XV_FRAME_SET_NUM_CHANNELS((xvFrame *) (pFrame), (numCh));                                                                              \
    XV_FRAME_SET_PADDING_TYPE((xvFrame *) (pFrame), (paddingType));                                                                        \
    XV_FRAME_SET_PADDING_VALUE((xvFrame *) (pFrame), (paddingVal));                                                                        \
  }

#define WAIT_FOR_TILE(pxvTM, pTile)                                       \
  {                                                                       \
    int32_t status;                                                       \
    status = xvCheckTileReady((pxvTM), (pTile));                          \
    while ( (status == 0) && (pxvTM->idmaErrorFlag == XV_ERROR_SUCCESS) ) \
    {                                                                     \
      status = xvCheckTileReady((pxvTM), (pTile));                        \
    }                                                                     \
  }

#define SLEEP_FOR_TILE(pxvTM, pTile)                                      \
  {                                                                       \
    int32_t status;                                                       \
    IDMA_DISABLE_INTS();                                                  \
    status = xvCheckTileReady((pxvTM), (pTile));                          \
    while ( (status == 0) && (pxvTM->idmaErrorFlag == XV_ERROR_SUCCESS) ) \
    {                                                                     \
      idma_sleep();                                                       \
      status = xvCheckTileReady((pxvTM), (pTile));                        \
    }                                                                     \
    IDMA_ENABLE_INTS();                                                   \
  }

#define WAIT_FOR_DMA(pxvTM, dmaIndex)                                     \
  {                                                                       \
    int32_t status;                                                       \
    status = idma_desc_done((dmaIndex));                                  \
    while ( (status == 0) && (pxvTM->idmaErrorFlag == XV_ERROR_SUCCESS) ) \
    {                                                                     \
      status = idma_desc_done((dmaIndex));                                \
    }                                                                     \
  }

#define SLEEP_FOR_DMA(pxvTM, dmaIndex)                                    \
  {                                                                       \
    int32_t status;                                                       \
    IDMA_DISABLE_INTS();                                                  \
    status = idma_desc_done((dmaIndex));                                  \
    while ( (status == 0) && (pxvTM->idmaErrorFlag == XV_ERROR_SUCCESS) ) \
    {                                                                     \
      idma_sleep();                                                       \
      status = idma_desc_done((dmaIndex));                                \
    }                                                                     \
    IDMA_ENABLE_INTS();                                                   \
  }

#define WAIT_FOR_TILE_FAST(pxvTM, pTile)                                  \
  {                                                                       \
    int32_t status;                                                       \
    status = idma_desc_done((pTile)->dmaIndex);                           \
    while ( (status == 0) && (pxvTM->idmaErrorFlag == XV_ERROR_SUCCESS) ) \
    {                                                                     \
      status = idma_desc_done((pTile)->dmaIndex);                         \
    }                                                                     \
    (pTile)->status = (pTile)->status & ~XV_TILE_STATUS_DMA_ONGOING;      \
  }

#define SLEEP_FOR_TILE_FAST(pxvTM, pTile)                                 \
  {                                                                       \
    int32_t status;                                                       \
    IDMA_DISABLE_INTS();                                                  \
    status = idma_desc_done((pTile)->dmaIndex);                           \
    while ( (status == 0) && (pxvTM->idmaErrorFlag == XV_ERROR_SUCCESS) ) \
    {                                                                     \
      idma_sleep();                                                       \
      status = idma_desc_done((pTile)->dmaIndex);                         \
    }                                                                     \
    IDMA_ENABLE_INTS();                                                   \
    (pTile)->status = (pTile)->status & ~XV_TILE_STATUS_DMA_ONGOING;      \
  }

// Assumes both top and bottom edges are equal
#define XV_TILE_UPDATE_EDGE_HEIGHT(pTile, newEdgeHeight)                            \
  {                                                                                 \
    uint32_t tileType       = XV_TILE_GET_TYPE(pTile);                              \
    uint32_t bytesPerPixel  = XV_TYPE_ELEMENT_SIZE(tileType);                       \
    uint32_t channels       = XV_TYPE_CHANNELS(tileType);                           \
    uint32_t bytesPerPel    = bytesPerPixel / channels;                             \
    uint16_t currEdgeHeight = (uint16_t) XV_TILE_GET_EDGE_HEIGHT(pTile);            \
    uint32_t tilePitch      = (uint32_t) XV_TILE_GET_PITCH(pTile);                  \
    uint32_t tileHeight     = (uint32_t) XV_TILE_GET_HEIGHT(pTile);                 \
    uint32_t dataU32        = (uint32_t) XV_TILE_GET_DATA_PTR(pTile);               \
    dataU32 = dataU32 + tilePitch * bytesPerPel * (newEdgeHeight - currEdgeHeight); \
    XV_TILE_SET_DATA_PTR(pTile, (void *) dataU32);                                  \
    XV_TILE_SET_EDGE_HEIGHT(pTile, newEdgeHeight);                                  \
    XV_TILE_SET_HEIGHT(pTile, tileHeight + 2 * (currEdgeHeight - newEdgeHeight));   \
  }

// Assumes both left and right edges are equal
#define XV_TILE_UPDATE_EDGE_WIDTH(pTile, newEdgeWidth)                        \
  {                                                                           \
    uint32_t tileType      = pTile->type;                                     \
    uint32_t bytesPerPixel = XV_TYPE_ELEMENT_SIZE(tileType);                  \
    uint16_t currEdgeWidth = (uint16_t) XV_TILE_GET_EDGE_WIDTH(pTile);        \
    uint32_t tileWidth     = (uint32_t) XV_TILE_GET_WIDTH(pTile);             \
    uint32_t dataU32       = (uint32_t) XV_TILE_GET_DATA_PTR(pTile);          \
    dataU32 = dataU32 + (newEdgeWidth - currEdgeWidth) * bytesPerPixel;       \
    XV_TILE_SET_DATA_PTR(pTile, (void *) dataU32);                            \
    XV_TILE_SET_EDGE_WIDTH(pTile, newEdgeWidth);                              \
    XV_TILE_SET_WIDTH(pTile, tileWidth + 2 * (currEdgeWidth - newEdgeWidth)); \
  }

#define XV_TILE_UPDATE_DIMENSIONS(pTile, x, y, w, h, p) \
  {                                                     \
    XV_TILE_SET_X_COORD(pTile, x);                      \
    XV_TILE_SET_Y_COORD(pTile, y);                      \
    XV_TILE_SET_WIDTH(pTile, w);                        \
    XV_TILE_SET_HEIGHT(pTile, h);                       \
    XV_TILE_SET_PITCH(pTile, p);                        \
  }

#define XV_TILE_GET_CHANNEL(t)  XV_TYPE_CHANNELS(XV_TILE_GET_TYPE(t))
#define XV_TILE_PEL_SIZE(t)     (XV_TILE_GET_ELEMENT_SIZE(t) / XV_TILE_GET_CHANNEL(t))

// Line 1-3: Checks if tile, data pointer or buffer pointer is NULL
// Line 4: Pitch should not be less than width + 2 times the edgeWidth
// Line 5: buffer pointer should not be greater than address of first (edge) element
//         First (edge) element should be inside the buffer
// Line 6: Last ((edge) element in data should be in the allocated buffer.
#define XV_IS_TILE_OK(t)                                                                                                                                                                               \
  ((t) &&                                                                                                                                                                                              \
   (XV_TILE_GET_DATA_PTR(t)) &&                                                                                                                                                                        \
   (XV_TILE_GET_BUFF_PTR(t)) &&                                                                                                                                                                        \
   ((XV_TILE_GET_PITCH(t)) >= ((XV_TILE_GET_WIDTH(t) + XV_TILE_GET_EDGE_LEFT(t) + XV_TILE_GET_EDGE_RIGHT(t)) * XV_TILE_GET_CHANNEL(t))) &&                                                             \
   (((uint8_t *) XV_TILE_GET_DATA_PTR(t) - (XV_TILE_GET_EDGE_LEFT(t) * XV_TILE_GET_ELEMENT_SIZE(t) + XV_TILE_GET_PITCH(t) * XV_TILE_GET_EDGE_TOP(t) * XV_TILE_PEL_SIZE(t)))                            \
    >= ((uint8_t *) XV_TILE_GET_BUFF_PTR(t))) &&                                                                                                                                                       \
   (((uint8_t *) XV_TILE_GET_DATA_PTR(t) - XV_TILE_GET_EDGE_LEFT(t) * XV_TILE_GET_ELEMENT_SIZE(t) + XV_TILE_GET_PITCH(t) * (XV_TILE_GET_HEIGHT(t) + XV_TILE_GET_EDGE_BOTTOM(t)) * XV_TILE_PEL_SIZE(t)) \
    <= ((uint8_t *) XV_TILE_GET_BUFF_PTR(t) + XV_TILE_GET_BUFF_SIZE(t))))

/***********************************
*    Function  Prototypes
***********************************/

// Initializes iDMA
// pxvTM            - Tile Manager object
// buf              - iDMA object and buffer
// numDesc          - Number of descriptors
// maxBlock         - Maximum block size
// maxPifReq        - Maximum pif request
// errCallbackFunc  - Callback function for iDMA interrupt error handling
// cbFunc           - Callback function for iDMA interrupt handling
// cbData           - Data required for interrupt callback function, cbFunc
// Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
int32_t  xvInitIdma(xvTileManager *pxvTM, idma_buffer_t *buf, int32_t numDescs, int32_t maxBlock,
                    int32_t maxPifReq, idma_err_callback_fn errCallbackFunc, idma_callback_fn cbFunc, void * cbData);

// Initializes Tile Manager
// pxvTM        - Tile Manager object
// pdmaObj      - iDMA buffer
// Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
int32_t xvInitTileManager(xvTileManager *pxvTM, void *pdmaObj);

// Resets Tile Manager
// pxvTM        - Tile Manager object
// Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
int32_t xvResetTileManager(xvTileManager *pxvTM);

// Initializes memory manager
// pxvTM         - Tile Manager object
// numMemBanks   - Number of memory pools
// pBankBuffPool - Array of start addresses of memory bank
// buffPoolSize  - Array of sizes of memory bank
// Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
int32_t xvInitMemAllocator(xvTileManager *pxvTM, int32_t numMemBanks, void **pBankBuffPool, int32_t* buffPoolSize);


// Allocates buffer from the pool
// pxvTM         - Tile Manager object
// buffSize      - size of requested buffer
// buffColor     - color/index of requested bufffer
// buffAlignment - Alignment of requested buffer
// Returns the buffer with requested parameters. If an error occurs, returns ((void *)(XVTM_ERROR))
void *xvAllocateBuffer(xvTileManager *pxvTM, int32_t buffSize, int32_t buffColor, int32_t buffAlignment);


// Releases the given buffer
// pxvTM - Tile Manager object
// pBuff - Pointer to buffer that needs to be released
// Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
int32_t xvFreeBuffer(xvTileManager *pxvTM, void *pBuff);


// Releases all buffers. Reinitializes the memory allocator
// pxvTM - Tile Manager object
// Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
int32_t xvFreeAllBuffers(xvTileManager *pxvTM);


// Allocates single frame
// pxvTM - Tile Manager object
// Returns the pointer to allocated frame. Does not allocate frame data buffer.
// Returns ((xvFrame *)(XVTM_ERROR)) if it encounters an error.
xvFrame *xvAllocateFrame(xvTileManager *pxvTM);


// Releases given frame
// pxvTM  - Tile Manager object
// pFrame - Frame that needs to be released. Does not release buffer
// Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
int32_t xvFreeFrame(xvTileManager *pxvTM, xvFrame *pFrame);


// Releases all allocated frames
// pxvTM  - Tile Manager object
// Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
int32_t  xvFreeAllFrames(xvTileManager *pxvTM);


// Allocates single tile
// pxvTM - Tile Manager object
// Returns the pointer to allocated tile. Does not allocate tile data buffer
// Returns ((xvTile *)(XVTM_ERROR)) if it encounters an error.
xvTile *xvAllocateTile(xvTileManager *pxvTM);


// Releases given tile
// pxvTM  - Tile Manager object
// pFrame - Tile that needs to be released. Does not release buffer
// Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
int32_t xvFreeTile(xvTileManager *pxvTM, xvTile *pTile);


// Releases all allocated tiles
// pxvTM  - Tile Manager object
// Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
int32_t xvFreeAllTiles(xvTileManager *pxvTM);


// Add iDMA transfer request
// pxvTM                 - Tile Manager object
// dst                   - pointer to destination buffer
// src                   - pointer to source buffer
// rowSize               - number of bytes to transfer in a row
// numRows               - number of rows to transfer
// srcPitch              - source buffer's pitch in bytes
// dstPitch              - destination buffer's pitch in bytes
// interruptOnCompletion - if it is set, iDMA will interrupt after completing transfer
// Returns dmaIndex for this request. It returns -1 if it encounters an error
int32_t xvAddIdmaRequest(xvTileManager *pxvTM, void *dst, void *src, size_t rowSize,
                         int32_t numRows, int32_t srcPitch, int32_t dsPitch, int32_t interruptOnCompletion);


// Requests data transfer from frame present in system memory to local tile memory
// pxvTM          - Tile Manager object
// pTile          - destination tile
// pPrevTile - data is copied from this tile to pTile if the buffer overlaps
// interruptOnCompletion - if it is set, iDMA will interrupt after completing transfer
// Returns XVTM_ERROR if it encounters an error, else it returns XVTM_SUCCESS
int32_t xvReqTileTransferIn(xvTileManager *pxvTM, xvTile *pTile, xvTile *pPrevTile, int32_t interruptOnCompletion);


// Requests 8b data transfer from frame present in system memory to local tile memory
// pxvTM          - Tile Manager object
// pTile          - destination tile
// interruptOnCompletion - if it is set, iDMA will interrupt after completing transfer
// Returns XVTM_ERROR if it encounters an error, else it returns XVTM_SUCCESS
int32_t xvReqTileTransferInFast(xvTileManager *pxvTM, xvTile *pTile, int32_t interruptOnCompletion);


// Requests 16b data transfer from frame present in system memory to local tile memory
// pxvTM          - Tile Manager object
// pTile          - destination tile
// interruptOnCompletion - if it is set, iDMA will interrupt after completing transfer
// Returns XVTM_ERROR if it encounters an error, else it returns XVTM_SUCCESS
int32_t xvReqTileTransferInFast16(xvTileManager *pxvTM, xvTile *pTile, int32_t interruptOnCompletion);


// Requests data transfer from tile present in local memory to frame in system memory
// pxvTM - Tile Manager object
// pTile - source tile
// interruptOnCompletion - if it is set, iDMA will interrupt after completing transfer
// Returns XVTM_ERROR if it encounters an error, else it returns XVTM_SUCCESS
int32_t xvReqTileTransferOut(xvTileManager *pxvTM, xvTile *pTile, int32_t interruptOnCompletion);


// Requests 8b data transfer from tile present in local memory to frame in system memory
// pxvTM - Tile Manager object
// pTile - source tile
// interruptOnCompletion - if it is set, iDMA will interrupt after completing transfer
// Returns XVTM_ERROR if it encounters an error, else it returns XVTM_SUCCESS
int32_t xvReqTileTransferOutFast(xvTileManager *pxvTM, xvTile *pTile, int32_t interruptOnCompletion);


// Requests 16b data transfer from tile present in local memory to frame in system memory
// pxvTM - Tile Manager object
// pTile - source tile
// interruptOnCompletion - if it is set, iDMA will interrupt after completing transfer
// Returns XVTM_ERROR if it encounters an error, else it returns XVTM_SUCCESS
int32_t xvReqTileTransferOutFast16(xvTileManager *pxvTM, xvTile *pTile, int32_t interruptOnCompletion);


//Pads 8b edge of the given tile
// pxvTM - Tile Manager object
// pTile - tile
int32_t xvPadEdges(xvTileManager *pxvTM, xvTile *pTile);


//Pads 16b edge of the given tile
// pxvTM - Tile Manager object
// pTile - tile
int32_t xvPadEdges16(xvTileManager *pxvTM, xvTile *pTile);


// Check if dma transfer is done
// pxvTM - Tile Manager object
// index - index for dma transfer request
// Returns XVTM_ERROR if an error occurs
int32_t xvCheckForIdmaIndex(xvTileManager *pxvTM, int32_t index);


// Check if tile is ready
// pxvTM - Tile Manager object
// pTile - input tile
// Takes care of padding, tile reuse
// Completes all tile transfers before the input tile
// Returns 1 if tile is ready, else returns 0
// Returns XVTM_ERROR if an error occurs
int32_t xvCheckTileReady(xvTileManager *pxvTM, xvTile *pTile);


// Check if input tile is free.
// A tile is said to be free if all data transfers pertaining to data resue from this tile is completed
// pxvTM - Tile Manager object
// pTile - output tile
// Returns 1 if tile is free, else returns 0
// Returns -1 if an error occurs
int32_t xvCheckInputTileFree(xvTileManager *pxvTM, xvTile *pTile);


// Prints the most recent error information.
// It returns the most recent error code.
xvError_t xvGetErrorInfo(xvTileManager *pxvTM);

// Creates and initializes Tile Manager, Memory Allocator and iDMA.
// Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
// pxvTM - Tile Manager object
// buf0 - iDMA object. It should be initialized before calling this function. Contains descriptors and idma library object
// numMemBanks - Number of memory pools
// pBankBuffPool - Array of memory pool start address
// buffPoolSize - Array of memory pool sizes
// numDescs - Number of descriptors that can be added in buffer
// maxBlock - Maximum block size allowed
// maxPifReq - Maximum number of outstanding pif requests
// errCallbackFunc - Callback for dma transfer error
// cbFunc - Callback for dma transfer completion
// cbData - Data needed for completion callback function
// Returns XVTM_SUCCESS on successful initializations,
// else returns XVTM_ERROR in case of error
int32_t xvCreateTileManager(xvTileManager *pxvTM, void *buf0, int32_t numMemBanks, void **pBankBuffPool, int32_t* buffPoolSize,
		idma_err_callback_fn errCallbackFunc, idma_callback_fn intrCallbackFunc, void *cbData, int32_t descCount, int32_t maxBlock, int32_t numOutReq);

// Allocates single frame. It does not allocate buffer required for frame data.
// Initializes the frame elements
// pxvTM - Tile Manager object
// imgBuff - Pointer to image buffer
// frameBuffSize - Size of allocated image buffer
// width - Width of image
// height - Height of image
// pitch - Pitch of image
// pixRes - Pixel resolution of image in bytes
// numChannels - Number of channels in the image
// paddingtype - Supported padding type
// paddingVal - Padding value if padding type is edge extension
// Returns the pointer to allocated frame.
// Returns ((xvFrame *)(XVTM_ERROR)) if it encounters an error.
// Does not allocate frame data buffer.
xvFrame *xvCreateFrame(xvTileManager *pxvTM, void *imgBuff, int32_t frameBuffSize, int32_t width, int32_t height, int32_t pitch, int32_t pixRes, int32_t numChannels, int32_t paddingtype, int32_t paddingVal);

// Allocates single tile and associated buffer data.
// Initializes the elements in tile
// pxvTM - Tile Manager object
// tileBuffSize - Size of allocated tile buffer
// width - Width of tile
// height - Height of tile
// pitch - Pitch of tile
// edgeWidth - Edge width of tile
// edgeHeight - Edge height of tile
// color - Memory pool from which the buffer should be allocated
// pFrame - Frame associated with the tile
// tileType - Type of tile
// alignType - Alignment tpye of tile. could be edge aligned of data aligned
// Returns the pointer to allocated tile.
// Returns ((xvTile *)(XVTM_ERROR)) if it encounters an error.
xvTile *xvCreateTile(xvTileManager *pxvTM, int32_t tileBuffSize, int32_t width, int32_t height, int32_t pitch, int32_t edgeWidth, int32_t edgeHeight, int32_t color, xvFrame *pFrame, int32_t tileType, int32_t alignType);


#if defined (__cplusplus)
}
#endif

#endif

