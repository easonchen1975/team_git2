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

/**********************************************************************************
 * FILE:  tileManager.c
 *
 * DESCRIPTION:
 *
 *    This file contains Tile Manager implementation. It uses xvmem utility for
 *    buffer allocation and idma library for 2D data transfer.
 *
 *
 ********************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xtensa/tie/xt_ivpn.h>
#include "tileManager.h"

#ifndef XV_EMULATE_DMA
xvTileManager *_ptr_xv_tile_manager  __attribute__ ((section(".dram0.data")));
#else
xvTileManager *_ptr_xv_tile_manager;
#endif

/**********************************************************************************
 * FUNCTION: xvInitIdma()
 *
 * DESCRIPTION:
 *     Function to initialize iDMA library. Tile Manager uses iDMA library
 *     in buffer mode. DMA transfer is scheduled as soon as the descriptor
 *     is added.
 *
 *
 * INPUTS:
 *     xvTileManager        *pxvTM              Tile Manager object
 *     idma_buffer_t        *buf                iDMA library handle; contains descriptors and idma library object
 *     int32_t              numDescs            Number of descriptors that can be added in buffer
 *     int32_t              maxBlock            Maximum block size allowed
 *     int32_t              maxPifReq           Maximum number of outstanding pif requests
 *     idma_err_callback_fn errCallbackFunc     Callback for dma transfer error
 *     idma_callback_fn     cbFunc              Callback for dma transfer completion
 *     void                 *cbData             Data needed for completion callback function
 *
 * OUTPUTS:
 *     Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
 *
 ********************************************************************************** */

int32_t xvInitIdma(xvTileManager *pxvTM, idma_buffer_t *buf, int32_t numDescs, int32_t maxBlock,
                   int32_t maxPifReq, idma_err_callback_fn errCallbackFunc, idma_callback_fn cbFunc, void * cbData)
{
#ifndef XV_EMULATE_DMA
  int retVal;
  if (pxvTM == NULL)
  {
    return(XVTM_ERROR);
  }

  pxvTM->errFlag = XV_ERROR_SUCCESS;
  if (buf == 0)
  {
    pxvTM->errFlag = XV_ERROR_BUFFER_NULL;
    return(XVTM_ERROR);
  }

  if (numDescs < 1)
  {
    pxvTM->errFlag = XV_ERROR_BAD_ARG;
    return(XVTM_ERROR);
  }

  idma_ticks_cyc_t ticksPerCyc = TICK_CYCLES_2;
  int32_t timeoutTicks         = 0;
  int32_t initFlags            = 0;
  idma_type_t type             = IDMA_2D_DESC;
  retVal = idma_init(initFlags, maxBlock, maxPifReq, ticksPerCyc, timeoutTicks, errCallbackFunc);
  if (retVal != IDMA_OK)
  {
    pxvTM->errFlag = XV_ERROR_DMA_INIT;
    return(XVTM_ERROR);
  }

  retVal = idma_init_loop(buf, type, numDescs, cbData, cbFunc);
  if (retVal != IDMA_OK)
  {
    pxvTM->errFlag = XV_ERROR_DMA_INIT;
    return(XVTM_ERROR);
  }
#endif
  return(XVTM_SUCCESS);
}

/**********************************************************************************
 * FUNCTION: xvInitTileManager()
 *
 * DESCRIPTION:
 *     Function to initialize Tile Manager. Resets Tile Manager's internal structure elements.
 *     Initializes Tile Manager's iDMA object.
 *
 * INPUTS:
 *     xvTileManager *pxvTM      Tile Manager object
 *     void          *pdmaObj    iDMA object. It should be initialized before calling this function
 *
 * OUTPUTS:
 *     Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
 *
 ********************************************************************************** */

int32_t xvInitTileManager(xvTileManager *pxvTM, void *pdmaObj)
{
  int32_t index;

  if (pxvTM == NULL)
  {
    return(XVTM_ERROR);
  }

  pxvTM->errFlag = XV_ERROR_SUCCESS;
  pxvTM->idmaErrorFlag = XV_ERROR_SUCCESS;
#ifndef XV_EMULATE_DMA
  if (pdmaObj == NULL)
  {
    pxvTM->errFlag = XV_ERROR_BUFFER_NULL;
    return(XVTM_ERROR);
  }
#endif
  _ptr_xv_tile_manager = pxvTM;
  // Initialize DMA related elements
  pxvTM->pdmaObj             = pdmaObj;
  pxvTM->tileDMApendingCount = 0;
  pxvTM->tileDMAstartIndex   = 0;

  // Initialize Memory banks related elements
#ifndef XV_EMULATE_DMA
  for (index = 0; index < MAX_NUM_MEM_BANKS; index++)
  {
    pxvTM->pMemBankStart[index] = NULL;
    pxvTM->memBankSize[index]   = 0;
  }
#endif

  // Reset tile related elements
  pxvTM->tileCount = 0;
  for (index = 0; index < (MAX_NUM_TILES + 31) / 32; index++)
  {
    pxvTM->tileAllocFlags[index] = 0x00000000;
  }

  // Reset frame related elements
  pxvTM->frameCount = 0;
  for (index = 0; index < (MAX_NUM_FRAMES + 31) / 32; index++)
  {
    pxvTM->frameAllocFlags[index] = 0x00000000;
  }

#ifdef TM_LOG
  pxvTM->tm_log_fp = fopen(TILE_MANAGER_LOG_FILE_NAME, "w");
  if (pxvTM->tm_log_fp == NULL)
  {
    pxvTM->errFlag = XV_ERROR_FILE_OPEN;
    return(XVTM_ERROR);
  }
#endif
  return(XVTM_SUCCESS);
}

/**********************************************************************************
 * FUNCTION: xvResetTileManager()
 *
 * DESCRIPTION:
 *     Function to reset Tile Manager. It closes the log file,
 *     releases the buffers and resets the Tile Manager object
 *
 * INPUTS:
 *     xvTileManager *pxvTM      Tile Manager object
 *
 * OUTPUTS:
 *     Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
 *
 ********************************************************************************** */

int32_t xvResetTileManager(xvTileManager *pxvTM)
{
  if (pxvTM == NULL)
  {
    return(XVTM_ERROR);
  }

  // Close the Tile Manager logging file
#ifdef TM_LOG
  if (pxvTM->tm_log_fp)
  {
    fclose(pxvTM->tm_log_fp);
  }
#endif

  // Free all the xvmem allocated buffers
  xvFreeAllBuffers(pxvTM);

  // Resetting the Tile Manager pointer.
  // This will free all allocated tiles and buffers.
  // It will not reset dma object.
  memset(pxvTM, 0, sizeof(xvTileManager));
  return(XVTM_SUCCESS);
}

/**********************************************************************************
 * FUNCTION: xvInitMemAllocator()
 *
 * DESCRIPTION:
 *     Function to initialize memory allocator. Tile Manager uses xvmem library as memory allocator.
 *     It takes array of pointers to memory pool's start addresses and respective sizes as input and
 *     uses the memory pools when memory is allocated.
 *
 * INPUTS:
 *     xvTileManager *pxvTM      Tile Manager object
 *     int32_t numMemBanks       Number of memory pools
 *     void **pBankBuffPool      Array of memory pool start address
 *     int32_t* buffPoolSize     Array of memory pool sizes
 *
 * OUTPUTS:
 *     Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
 *
 ********************************************************************************** */

int32_t xvInitMemAllocator(xvTileManager *pxvTM, int32_t numMemBanks, void **pBankBuffPool, int32_t* buffPoolSize)
{
#ifndef XV_EMULATE_DMA
  int32_t indx, retVal;
  if (pxvTM == NULL)
  {
    return(XVTM_ERROR);
  }

  pxvTM->errFlag = XV_ERROR_SUCCESS;
  if ((numMemBanks <= 0) || (numMemBanks > MAX_NUM_MEM_BANKS))
  {
    pxvTM->errFlag = XV_ERROR_BAD_ARG;
    return(XVTM_ERROR);
  }
  if ((pBankBuffPool == NULL) || (buffPoolSize == NULL))
  {
    pxvTM->errFlag = XV_ERROR_POINTER_NULL;
    return(XVTM_ERROR);
  }
  xvmem_mgr_t *pmemBankMgr;
  pxvTM->numMemBanks = numMemBanks;

  for (indx = 0; indx < numMemBanks; indx++)
  {
    if ((pBankBuffPool[indx] == NULL) || (buffPoolSize[indx] == 0))
    {
      pxvTM->errFlag = XV_ERROR_BAD_ARG;
      return(XVTM_ERROR);
    }
    pmemBankMgr                = &(pxvTM->memBankMgr[indx]);
    pxvTM->pMemBankStart[indx] = pBankBuffPool[indx];
    pxvTM->memBankSize[indx]   = buffPoolSize[indx];
    retVal                     = xvmem_init(pmemBankMgr, pBankBuffPool[indx], buffPoolSize[indx], 0, 0);
    if (retVal != XVMEM_OK)
    {
      pxvTM->errFlag = XV_ERROR_XVMEM_INIT;
      return(XVTM_ERROR);
    }
  }
#endif
  return(XVTM_SUCCESS);
}

/**********************************************************************************
 * FUNCTION: xvAllocateBuffer()
 *
 * DESCRIPTION:
 *     Allocates buffer from the given buffer pool. It returns aligned buffer.
 *
 * INPUTS:
 *     xvTileManager *pxvTM      Tile Manager object
 *     int32_t buffSize          Size of the requested buffer
 *     int32_t buffColor         Color/index of requested buffer
 *     int32_t buffAlignment     Alignment of requested buffer
 *
 * OUTPUTS:
 *     Returns the buffer with requested parameters. If an error occurs, returns ((void *)(XVTM_ERROR))
 *
 ********************************************************************************** */

void *xvAllocateBuffer(xvTileManager *pxvTM, int32_t buffSize, int32_t buffColor, int32_t buffAlignment)
{
  void *buffOut = NULL;
#ifndef XV_EMULATE_DMA
  int32_t currColor;
  xvmem_status_t errCode;

  if (pxvTM == NULL)
  {
    return((void *) XVTM_ERROR);
  }
  pxvTM->errFlag = XV_ERROR_SUCCESS;

  if (buffSize <= 0)
  {
    pxvTM->errFlag = XV_ERROR_BAD_ARG;
    return((void *) XVTM_ERROR);
  }

  if (((buffColor >= 0) && (buffColor < pxvTM->numMemBanks)) || (buffColor == XV_MEM_BANK_COLOR_ANY))
  {
    // if the color is XV_MEM_BANK_COLOR_ANY, loop through all the buffers and pick one that meets all criteria
    if (buffColor == XV_MEM_BANK_COLOR_ANY)
    {
      currColor = 0;
      while (buffOut == NULL && currColor < pxvTM->numMemBanks)
      {
        buffOut = xvmem_alloc(&(pxvTM->memBankMgr[currColor]), buffSize, buffAlignment, &errCode);
        currColor++;
      }
    }
    else
    {
      buffOut = xvmem_alloc(&(pxvTM->memBankMgr[buffColor]), buffSize, buffAlignment, &errCode);
    }
  }
  else
  {
    pxvTM->errFlag = XV_ERROR_BAD_ARG;
    return((void *) XVTM_ERROR);
  }
  if (buffOut == NULL)
  {
    pxvTM->errFlag = XV_ERROR_ALLOC_FAILED;
    return((void *) XVTM_ERROR);
  }
#else
  buffOut = calloc(buffSize + buffAlignment, 1);
  buffOut = (void *) (((long) buffOut + (buffAlignment - 1)) & (~(buffAlignment - 1)));
#endif
  return(buffOut);
}

/**********************************************************************************
 * FUNCTION: xvFreeBuffer()
 *
 * DESCRIPTION:
 *     Releases the given buffer.
 *
 * INPUTS:
 *     xvTileManager *pxvTM      Tile Manager object
 *     void          *pBuff      Buffer that needs to be released
 *
 * OUTPUTS:
 *     Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
 *
 ********************************************************************************** */

int32_t xvFreeBuffer(xvTileManager *pxvTM, void *pBuff)
{
#ifndef XV_EMULATE_DMA
  int32_t index;
  if (pxvTM == NULL)
  {
    return(XVTM_ERROR);
  }

  pxvTM->errFlag = XV_ERROR_SUCCESS;
  if (pBuff == NULL)
  {
    pxvTM->errFlag = XV_ERROR_BUFFER_NULL;
    return(XVTM_ERROR);
  }

  for (index = 0; index < pxvTM->numMemBanks; index++)
  {
    if ((pxvTM->pMemBankStart[index] <= pBuff) && (pBuff < pxvTM->pMemBankStart[index] + pxvTM->memBankSize[index]))
    {
      xvmem_free(&(pxvTM->memBankMgr[index]), pBuff);
      return(XVTM_SUCCESS);
    }
  }
  pxvTM->errFlag = XV_ERROR_BAD_ARG;
  return(XVTM_ERROR);
#else
  return(XVTM_SUCCESS);
#endif
}

/**********************************************************************************
 * FUNCTION: xvFreeAllBuffers()
 *
 * DESCRIPTION:
 *     Releases all buffers. Reinitializes the memory allocator
 *
 * INPUTS:
 *     xvTileManager *pxvTM      Tile Manager object
 *
 * OUTPUTS:
 *     Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
 *
 ********************************************************************************** */

int32_t xvFreeAllBuffers(xvTileManager *pxvTM)
{
#ifndef XV_EMULATE_DMA
  if (pxvTM == NULL)
  {
    return(XVTM_ERROR);
  }
  pxvTM->errFlag = XV_ERROR_SUCCESS;

  int32_t bankIndex, buffPoolSize;
  xvmem_mgr_t *pmemBankMgr;
  uint8_t *pBankBuffPool;

  // Resetting all memory manager related elements
  for (bankIndex = 0; bankIndex < pxvTM->numMemBanks; bankIndex++)
  {
    pmemBankMgr   = &(pxvTM->memBankMgr[bankIndex]);
    pBankBuffPool = pxvTM->pMemBankStart[bankIndex];
    buffPoolSize  = pxvTM->memBankSize[bankIndex];
    xvmem_init(pmemBankMgr, pBankBuffPool, buffPoolSize, 0, 0);
  }
#endif
  return(XVTM_SUCCESS);
}

/**********************************************************************************
 * FUNCTION: xvAllocateFrame()
 *
 * DESCRIPTION:
 *     Allocates single frame. It does not allocate buffer required for frame data.
 *
 * INPUTS:
 *     xvTileManager *pxvTM      Tile Manager object
 *
 * OUTPUTS:
 *     Returns the pointer to allocated frame.
 *     Returns ((xvFrame *)(XVTM_ERROR)) if it encounters an error.
 *     Does not allocate frame data buffer.
 *
 ********************************************************************************** */

xvFrame *xvAllocateFrame(xvTileManager *pxvTM)
{
  int32_t indx, indxArr, indxShift, allocFlags;
  xvFrame *pFrame = NULL;

  if (pxvTM == NULL)
  {
    return((xvFrame *) (XVTM_ERROR));
  }
  pxvTM->errFlag = XV_ERROR_SUCCESS;

  for (indx = 0; indx < MAX_NUM_FRAMES; indx++)
  {
    indxArr    = indx >> 5;
    indxShift  = indx & 0x1F;
    allocFlags = pxvTM->frameAllocFlags[indxArr];
    if (((allocFlags >> indxShift) & 0x1) == 0)
    {
      break;
    }
  }

  if (indx < MAX_NUM_FRAMES)
  {
    pFrame                          = &(pxvTM->frameArray[indx]);
    pxvTM->frameAllocFlags[indxArr] = allocFlags | (0x1 << indxShift);
    pxvTM->frameCount++;
  }
  else
  {
    pxvTM->errFlag = XV_ERROR_FRAME_BUFFER_FULL;
    return((xvFrame *) (XVTM_ERROR));
  }

  return(pFrame);
}

/**********************************************************************************
 * FUNCTION: xvFreeFrame()
 *
 * DESCRIPTION:
 *     Releases the given frame. Does not release associated frame data buffer.
 *
 * INPUTS:
 *     xvTileManager *pxvTM      Tile Manager object
 *     xvFrame       *pFrame     Frame that needs to be released
 *
 * OUTPUTS:
 *     Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
 *
 ********************************************************************************** */

int32_t xvFreeFrame(xvTileManager *pxvTM, xvFrame *pFrame)
{
  int32_t indx, indxArr, indxShift, allocFlags;

  if (pxvTM == NULL)
  {
    return(XVTM_ERROR);
  }

  pxvTM->errFlag = XV_ERROR_SUCCESS;
  if (pFrame == NULL)
  {
    pxvTM->errFlag = XV_ERROR_FRAME_NULL;
    return(XVTM_ERROR);
  }

  for (indx = 0; indx < MAX_NUM_FRAMES; indx++)
  {
    if (&(pxvTM->frameArray[indx]) == pFrame)
    {
      break;
    }
  }

  if (indx < MAX_NUM_FRAMES)
  {
    indxArr                         = indx >> 5;
    indxShift                       = indx & 0x1F;
    allocFlags                      = pxvTM->frameAllocFlags[indxArr];
    pxvTM->frameAllocFlags[indxArr] = allocFlags & ~(0x1 << indxShift);
    pxvTM->frameCount--;
  }
  else
  {
    pxvTM->errFlag = XV_ERROR_BAD_ARG;
    return(XVTM_ERROR);
  }
  return(XVTM_SUCCESS);
}

/**********************************************************************************
 * FUNCTION: xvFreeAllFrames()
 *
 * DESCRIPTION:
 *     Releases all allocated frames.
 *
 * INPUTS:
 *     xvTileManager *pxvTM      Tile Manager object
 *
 * OUTPUTS:
 *     Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
 *
 ********************************************************************************** */

int32_t xvFreeAllFrames(xvTileManager *pxvTM)
{
  int32_t index;
  if (pxvTM == NULL)
  {
    return(XVTM_ERROR);
  }
  pxvTM->errFlag = XV_ERROR_SUCCESS;

  pxvTM->frameCount = 0;
  for (index = 0; index < (MAX_NUM_FRAMES + 31) / 32; index++)
  {
    pxvTM->frameAllocFlags[index] = 0x00000000;
  }
  return(XVTM_SUCCESS);
}

/**********************************************************************************
 * FUNCTION: xvAllocateTile()
 *
 * DESCRIPTION:
 *     Allocates single tile. It does not allocate buffer required for tile data.
 *
 * INPUTS:
 *     xvTileManager *pxvTM      Tile Manager object
 *
 * OUTPUTS:
 *     Returns the pointer to allocated tile.
 *     Returns ((xvTile *)(XVTM_ERROR)) if it encounters an error.
 *     Does not allocate tile data buffer
 *
 ********************************************************************************** */

xvTile *xvAllocateTile(xvTileManager *pxvTM)
{
  int32_t indx, indxArr, indxShift, allocFlags;
  xvTile *pTile = NULL;
  if (pxvTM == NULL)
  {
    return((xvTile *) (XVTM_ERROR));
  }
  pxvTM->errFlag = XV_ERROR_SUCCESS;

  for (indx = 0; indx < MAX_NUM_TILES; indx++)
  {
    indxArr    = indx >> 5;
    indxShift  = indx & 0x1F;
    allocFlags = pxvTM->tileAllocFlags[indxArr];
    if (((allocFlags >> indxShift) & 0x1) == 0)
    {
      break;
    }
  }

  if (indx < MAX_NUM_TILES)
  {
    pTile                          = &(pxvTM->tileArray[indx]);
    pxvTM->tileAllocFlags[indxArr] = allocFlags | (0x1 << indxShift);
    pxvTM->tileCount++;
  }
  else
  {
    pxvTM->errFlag = XV_ERROR_TILE_BUFFER_FULL;
    return((xvTile *) (XVTM_ERROR));
  }

  return(pTile);
}

/**********************************************************************************
 * FUNCTION: xvFreeTile()
 *
 * DESCRIPTION:
 *     Releases the given tile. Does not release associated tile data buffer.
 *
 * INPUTS:
 *     xvTileManager *pxvTM      Tile Manager object
 *     xvTile        *pTile      Tile that needs to be released
 *
 * OUTPUTS:
 *     Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
 *
 ********************************************************************************** */
int32_t xvFreeTile(xvTileManager *pxvTM, xvTile *pTile)
{
  int32_t indx, indxArr, indxShift, allocFlags;

  if (pxvTM == NULL)
  {
    return(XVTM_ERROR);
  }

  pxvTM->errFlag = XV_ERROR_SUCCESS;
  if (pTile == NULL)
  {
    pxvTM->errFlag = XV_ERROR_TILE_NULL;
    return(XVTM_ERROR);
  }

  for (indx = 0; indx < MAX_NUM_TILES; indx++)
  {
    if (&(pxvTM->tileArray[indx]) == pTile)
    {
      break;
    }
  }

  if (indx < MAX_NUM_TILES)
  {
    indxArr                        = indx >> 5;
    indxShift                      = indx & 0x1F;
    allocFlags                     = pxvTM->tileAllocFlags[indxArr];
    pxvTM->tileAllocFlags[indxArr] = allocFlags & ~(0x1 << indxShift);
    pxvTM->tileCount--;
  }
  else
  {
    pxvTM->errFlag = XV_ERROR_BAD_ARG;
    return(XVTM_ERROR);
  }
  return(XVTM_SUCCESS);
}

/**********************************************************************************
 * FUNCTION: xvFreeAllTiles()
 *
 * DESCRIPTION:
 *     Releases all allocated tiles.
 *
 * INPUTS:
 *     xvTileManager *pxvTM      Tile Manager object
 *
 * OUTPUTS:
 *     Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
 *
 ********************************************************************************** */

int32_t xvFreeAllTiles(xvTileManager *pxvTM)
{
  int32_t index;

  if (pxvTM == NULL)
  {
    return(XVTM_ERROR);
  }
  pxvTM->errFlag = XV_ERROR_SUCCESS;

  pxvTM->tileCount = 0;
  for (index = 0; index < (MAX_NUM_TILES + 31) / 32; index++)
  {
    pxvTM->tileAllocFlags[index] = 0x00000000;
  }
  return(XVTM_SUCCESS);
}

/**********************************************************************************
 * FUNCTION: xvCreateTileManager()
 *
 * DESCRIPTION:
 *     Creates and initializes Tile Manager, Memory Allocator and iDMA.
 *
 * INPUTS:
 *     xvTileManager        *pxvTM              Tile Manager object
 *     idma_buffer_t        *buf0               iDMA object. It should be initialized before calling this function. Contains descriptors and idma library object
 *     int32_t              numMemBanks         Number of memory pools
 *     void                 **pBankBuffPool     Array of memory pool start address
 *     int32_t              *buffPoolSize       Array of memory pool sizes
 *     int32_t              numDescs            Number of descriptors that can be added in buffer
 *     int32_t              maxBlock            Maximum block size allowed
 *     int32_t              maxPifReq           Maximum number of outstanding pif requests
 *     idma_err_callback_fn errCallbackFunc     Callback for dma transfer error
 *     idma_callback_fn     cbFunc              Callback for dma transfer completion
 *     void                 *cbData             Data needed for completion callback function
 *
 *
 * OUTPUTS:
 *     Returns XVTM_ERROR if it encounters an error, else returns XVTM_SUCCESS
 *
 ********************************************************************************** */

int32_t xvCreateTileManager(xvTileManager *pxvTM, void *buf0, int32_t numMemBanks, void **pBankBuffPool, int32_t* buffPoolSize,
		idma_err_callback_fn errCallbackFunc, idma_callback_fn intrCallbackFunc, void *cbData, int32_t descCount, int32_t maxBlock, int32_t numOutReq)
{
  int32_t retVal;

  retVal = xvInitTileManager(pxvTM, (idma_buffer_t *) buf0);
  if (retVal == XVTM_ERROR)
  {
    return (XVTM_ERROR);
  }
  
  retVal = xvInitMemAllocator(pxvTM, numMemBanks, pBankBuffPool, buffPoolSize);
  if (retVal == XVTM_ERROR)
  {
    return (XVTM_ERROR);
  }

  retVal = xvInitIdma(pxvTM, (idma_buffer_t *) buf0, descCount, maxBlock, numOutReq, errCallbackFunc, intrCallbackFunc, cbData);
  if (retVal == XVTM_ERROR)
  {
    return (XVTM_ERROR);
  }

	return retVal;
}

/**********************************************************************************
 * FUNCTION: xvCreateFrame()
 *
 * DESCRIPTION:
 *     Allocates single frame. It does not allocate buffer required for frame data.
 *     Initializes the frame elements
 *
 * INPUTS:
 *     xvTileManager *pxvTM          Tile Manager object
 *     void          *imgBuff        Pointer to image buffer
 *     int32_t       frameBuffSize   Size of allocated image buffer
 *     int32_t       width           Width of image
 *     int32_t       height          Height of image
 *     int32_t       pitch           Pitch of image
 *     int32_t       pixRes          Pixel resolution of image in bytes
 *     int32_t       numChannels     Number of channels in the image
 *     int32_t       paddingtype     Supported padding type
 *     int32_t       paddingVal      Padding value if padding type is edge extension
 *
 * OUTPUTS:
 *     Returns the pointer to allocated frame.
 *     Returns ((xvFrame *)(XVTM_ERROR)) if it encounters an error.
 *     Does not allocate frame data buffer.
 *
 ********************************************************************************** */

xvFrame *xvCreateFrame(xvTileManager *pxvTM, void *imgBuff, int32_t frameBuffSize, int32_t width, int32_t height, int32_t pitch, int32_t pixRes, int32_t numChannels, int32_t paddingtype, int32_t paddingVal)
{
  xvFrame *pFrame = xvAllocateFrame(pxvTM);
  if ((int32_t)pFrame == XVTM_ERROR)
  {
    return ((xvFrame *)XVTM_ERROR);
  }

  SETUP_FRAME(pFrame, imgBuff, frameBuffSize, width, height, pitch, 0, 0, pixRes, numChannels,  paddingtype, paddingVal);
  return pFrame;
}

/**********************************************************************************
 * FUNCTION: xvCreateTile()
 *
 * DESCRIPTION:
 *     Allocates single tile and associated buffer data.
 *     Initializes the elements in tile
 *
 * INPUTS:
 *     xvTileManager *pxvTM          Tile Manager object
 *     int32_t       tileBuffSize    Size of allocated tile buffer
 *     int32_t       width           Width of tile
 *     int32_t       height          Height of tile
 *     int32_t       pitch           Pitch of tile
 *     int32_t       edgeWidth       Edge width of tile
 *     int32_t       edgeHeight      Edge height of tile
 *     int32_t       color           Memory pool from which the buffer should be allocated
 *     xvFrame       *pFrame         Frame associated with the tile
 *     int32_t       tileType        Type of tile
 *     int32_t       alignType       Alignment tpye of tile. could be edge aligned of data aligned
 *
 * OUTPUTS:
 *     Returns the pointer to allocated tile.
 *     Returns ((xvTile *)(XVTM_ERROR)) if it encounters an error.
 *
 ********************************************************************************** */

xvTile *xvCreateTile(xvTileManager *pxvTM, int32_t tileBuffSize, int32_t width, int32_t height, int32_t pitch, int32_t edgeWidth, int32_t edgeHeight, int32_t color, xvFrame *pFrame, int32_t xvTileType, int32_t alignType)
{

  void *tileBuff = NULL;
  if(tileBuffSize)
  {
    tileBuff = xvAllocateBuffer(pxvTM, tileBuffSize, color, 64);
  }
  if ((int32_t)tileBuff == XVTM_ERROR)
  {
    return ((xvTile *)XVTM_ERROR);
  }

  xvTile *pTile = xvAllocateTile(pxvTM);
  if ((int32_t)pTile == XVTM_ERROR)
  {
    return ((xvTile *)XVTM_ERROR);
  }

  SETUP_TILE(pTile, tileBuff, tileBuffSize, pFrame, width, height, pitch, xvTileType, edgeWidth, edgeHeight, 0, 0, alignType);
  return pTile;
}

/**********************************************************************************
 * FUNCTION: xvAddIdmaRequest()
 *
 * DESCRIPTION:
 *     Add iDMA transfer request. Request is scheduled as soon as it is added
 *
 * INPUTS:
 *     xvTileManager *pxvTM                   Tile Manager object
 *     void          * dst                    Pointer to destination buffer
 *     void          *src                     Pointer to source buffer
 *     size_t        rowSize                  Number of bytes to transfer in a row
 *     int32_t       numRows                  Number of rows to transfer
 *     int32_t       srcPitch                 Source buffer's pitch in bytes
 *     int32_t       dstPitch                 Destination buffer's pitch in bytes
 *     int32_t       interruptOnCompletion    If it is set, iDMA will interrupt after completing transfer
 * OUTPUTS:
 *     Returns dmaIndex for this request. It returns XVTM_ERROR if it encounters an error
 *
 ********************************************************************************** */

int32_t xvAddIdmaRequest(xvTileManager *pxvTM, void *dst, void *src, size_t rowSize,
                         int32_t numRows, int32_t srcPitch, int32_t dstPitch, int32_t interruptOnCompletion)
{
  if (pxvTM == NULL)
  {
    return(XVTM_ERROR);
  }

  pxvTM->errFlag = XV_ERROR_SUCCESS;
  if ((dst == NULL) || (src == NULL))
  {
    pxvTM->errFlag = XV_ERROR_POINTER_NULL;
    return(XVTM_ERROR);
  }

  if ((rowSize <= 0) || (numRows <= 0) || (srcPitch < 0) || (dstPitch < 0))
  {
    pxvTM->errFlag = XV_ERROR_BAD_ARG;
    return(XVTM_ERROR);
  }

#ifndef XV_EMULATE_DMA
  int32_t dmaIndex;
  uint32_t intrCompletionFlag;

  if (interruptOnCompletion)
  {
    intrCompletionFlag = DESC_NOTIFY_W_INT;
  }
  else
  {
    intrCompletionFlag = 0;
  }

  TM_LOG_PRINT("src: %p, dst, %p, rowsize: %d, numRows: %d, srcPitch: %d, dstPitch: %d, flags: 0x%x\n",
               src, dst, rowSize, numRows, srcPitch, dstPitch, intrCompletionFlag);
  dmaIndex = idma_copy_2d_desc(dst, src, rowSize, intrCompletionFlag, numRows, srcPitch, dstPitch);
  TM_LOG_PRINT(" dmaIndex: %d\n", dmaIndex);
  return(dmaIndex);
#else
  int32_t indx;
  for (indx = 0; indx < numRows; indx++)
  {
    memcpy(dst, src, rowSize);
    dst = (uint8_t *) dst + dstPitch;
    src = (uint8_t *) src + srcPitch;
  }
  return(XVTM_SUCCESS);
#endif
}

// Made it inline to speed up xvReqTileTransferIn and xvReqTileTransferOut APIs
static inline int32_t addIdmaRequestInline(xvTileManager *pxvTM, void *dst, void *src, size_t rowSize,
                                           int32_t numRows, int32_t srcPitchBytes, int32_t dstPitchBytes, int32_t interruptOnCompletion)
{
#ifndef XV_EMULATE_DMA
  int32_t dmaIndex;
  uint32_t intrCompletionFlag;

  if (interruptOnCompletion)
  {
    intrCompletionFlag = DESC_NOTIFY_W_INT;
  }
  else
  {
    intrCompletionFlag = 0;
  }

  TM_LOG_PRINT("src: %p, dst, %p, rowsize: %d, numRows: %d, srcPitch: %d, dstPitch: %d, flags: 0x%x\n",
               src, dst, rowSize, numRows, srcPitchBytes, dstPitchBytes, intrCompletionFlag);
  dmaIndex = idma_copy_2d_desc(dst, src, rowSize, intrCompletionFlag, numRows, srcPitchBytes, dstPitchBytes);
  TM_LOG_PRINT(" dmaIndex: %d\n", dmaIndex);
  return(dmaIndex);
#else
  int32_t indx;
  for (indx = 0; indx < numRows; indx++)
  {
    memcpy(dst, src, rowSize);
    dst = (uint8_t *) dst + dstPitchBytes;
    src = (uint8_t *) src + srcPitchBytes;
  }
  return(XVTM_SUCCESS);
#endif
}

// Part of tile reuse. Checks X direction boundary condition and performs DMA transfers
uint32_t solveForX(xvTileManager *pxvTM, xvTile *pTile, uint8_t *pCurrBuff, uint8_t *pPrevBuff,
                   int32_t y1, int32_t y2, int32_t x1, int32_t x2, int32_t px1, int32_t px2, int32_t tp, int32_t ptp, int32_t interruptOnCompletion)
{
  uint8_t pixWidth;
  int32_t dmaIndex, framePitchBytes, bytes, bytesCopy;

  uint8_t *pSrc, *pDst;
  uint8_t *pSrcCopy, *pDstCopy;

  dmaIndex = 0;
  xvFrame *pFrame = pTile->pFrame;
  pixWidth        = pFrame->pixelRes * pFrame->numChannels;
  framePitchBytes = pFrame->framePitch * pFrame->pixelRes;

  // Case 1. Only left most part overlaps
  if ((px1 <= x1) && (x1 <= px2) && (px2 < x2))
  {
    pSrcCopy  = pPrevBuff + (x1 - px1) * pixWidth;
    pDstCopy  = pCurrBuff;
    bytesCopy = (px2 - x1 + 1) * pixWidth;
    dmaIndex  = addIdmaRequestInline(pxvTM, pDstCopy, pSrcCopy, bytesCopy, y2 - y1 + 1, ptp, tp, 0);

    pSrc     = (uint8_t *) pFrame->pFrameData + y1 * framePitchBytes + (px2 + 1) * pixWidth;
    pDst     = pCurrBuff + (px2 - x1 + 1) * pixWidth;
    bytes    = (x2 - px2) * pixWidth;
    dmaIndex = addIdmaRequestInline(pxvTM, pDst, pSrc, bytes, y2 - y1 + 1, framePitchBytes, tp, interruptOnCompletion);
  }

  // Case 2. Only mid part overlaps
  if ((x1 < px1) && (px2 < x2))
  {
    pSrc     = (uint8_t *) pFrame->pFrameData + y1 * framePitchBytes + x1 * pixWidth;
    pDst     = pCurrBuff;
    bytes    = (px1 - x1) * pixWidth;
    dmaIndex = addIdmaRequestInline(pxvTM, pDst, pSrc, bytes, y2 - y1 + 1, framePitchBytes, tp, 0);

    pSrcCopy  = pPrevBuff;
    pDstCopy  = pCurrBuff + (px1 - x1) * pixWidth;
    bytesCopy = (px2 - px1 + 1) * pixWidth;
    dmaIndex  = addIdmaRequestInline(pxvTM, pDstCopy, pSrcCopy, bytesCopy, y2 - y1 + 1, ptp, tp, 0);

    pSrc     = (uint8_t *) pFrame->pFrameData + y1 * framePitchBytes + (px2 + 1) * pixWidth;
    pDst     = pCurrBuff + (px2 - x1 + 1) * pixWidth;
    bytes    = (x2 - px2) * pixWidth;
    dmaIndex = addIdmaRequestInline(pxvTM, pDst, pSrc, bytes, y2 - y1 + 1, framePitchBytes, tp, interruptOnCompletion);
  }

  // Case 3. Only right part overlaps
  if ((x1 < px1) && (px1 <= x2) && (x2 <= px2))
  {
    pSrc     = (uint8_t *) pFrame->pFrameData + y1 * framePitchBytes + x1 * pixWidth;
    pDst     = pCurrBuff;
    bytes    = (px1 - x1) * pixWidth;
    dmaIndex = addIdmaRequestInline(pxvTM, pDst, pSrc, bytes, y2 - y1 + 1, framePitchBytes, tp, 0);

    pSrcCopy  = pPrevBuff;
    pDstCopy  = pCurrBuff + (px1 - x1) * pixWidth;
    bytesCopy = (x2 - px1 + 1) * pixWidth;
    dmaIndex  = addIdmaRequestInline(pxvTM, pDstCopy, pSrcCopy, bytesCopy, y2 - y1 + 1, ptp, tp, interruptOnCompletion);
  }

  // Case 4. All three regions overlaps
  if ((px1 <= x1) && (x2 <= px2))
  {
    pSrcCopy  = pPrevBuff + (x1 - px1) * pixWidth;
    pDstCopy  = pCurrBuff;
    bytesCopy = (x2 - x1 + 1) * pixWidth;
    dmaIndex  = addIdmaRequestInline(pxvTM, pDstCopy, pSrcCopy, bytesCopy, y2 - y1 + 1, ptp, tp, interruptOnCompletion);
  }

  return(dmaIndex);
}

/**********************************************************************************
 * FUNCTION: xvReqTileTransferIn()
 *
 * DESCRIPTION:
 *     Requests data transfer from frame present in system memory to local tile memory.
 *     If there is an overlap between previous tile and current tile, tile reuse
 *     functionality can be used.
 *
 * INPUTS:
 *     xvTileManager *pxvTM                   Tile Manager object
 *     xvTile        *pTile                   Destination tile
 *     xvTile        *pPrevTile               Data is copied from this tile to pTile if the buffer overlaps
 *     int32_t       interruptOnCompletion    If it is set, iDMA will interrupt after completing transfer
 *
 * OUTPUTS:
 *     Returns XVTM_ERROR if it encounters an error, else it returns XVTM_SUCCESS
 *
 ********************************************************************************** */

int32_t xvReqTileTransferIn(xvTileManager *pxvTM, xvTile *pTile, xvTile *pPrevTile, int32_t interruptOnCompletion)
{
  xvFrame *pFrame;
  int32_t frameWidth, frameHeight, framePitchBytes, tileWidth, tileHeight, tilePitchBytes;
  int32_t statusFlag, x1, y1, x2, y2, dmaHeight, dmaWidthBytes, dmaIndex;
  int32_t tileIndex;
  int8_t framePadLeft, framePadRight, framePadTop, framePadBottom;
  int16_t tileEdgeLeft, tileEdgeRight, tileEdgeTop, tileEdgeBottom;
  int16_t extraEdgeTop, extraEdgeBottom, extraEdgeLeft, extraEdgeRight;
  int8_t pixWidth, pixRes;
  uint8_t *srcPtr, *dstPtr, *pPrevBuff, *pCurrBuff, *edgePtr;
  int32_t px1, px2, py1, py2;

  if (pxvTM == NULL)
  {
    return(XVTM_ERROR);
  }
  pxvTM->errFlag = XV_ERROR_SUCCESS;

  if (pTile == NULL)
  {
    pxvTM->errFlag = XV_ERROR_TILE_NULL;
    return(XVTM_ERROR);
  }

#ifndef XV_EMULATE_DMA
  if (XV_IS_TILE_OK(pTile) == 0)
  {
    pxvTM->errFlag = XV_ERROR_BAD_ARG;
    return(XVTM_ERROR);
  }
#endif

  pFrame = pTile->pFrame;
  if (pFrame == NULL || pFrame->pFrameBuff == NULL || pFrame->pFrameData == NULL)
  {
    pxvTM->errFlag = XV_ERROR_FRAME_NULL;
    return(XVTM_ERROR);
  }

  pTile->pPrevTile  = NULL;
  pTile->reuseCount = 0;
  frameWidth        = pFrame->frameWidth;
  frameHeight       = pFrame->frameHeight;
  framePadLeft      = pFrame->leftEdgePadWidth;
  framePadRight     = pFrame->rightEdgePadWidth;
  framePadTop       = pFrame->topEdgePadHeight;
  framePadBottom    = pFrame->bottomEdgePadHeight;
  pixRes            = pFrame->pixelRes;
  pixWidth          = pFrame->pixelRes * pFrame->numChannels;
  framePitchBytes   = pFrame->framePitch * pixRes;

  tileWidth      = pTile->width;
  tileHeight     = pTile->height;
  tilePitchBytes = pTile->pitch * pixRes;
  tileEdgeLeft   = pTile->tileEdgeLeft;
  tileEdgeRight  = pTile->tileEdgeRight;
  tileEdgeTop    = pTile->tileEdgeTop;
  tileEdgeBottom = pTile->tileEdgeBottom;

  statusFlag = pTile->status;

  // 1. CHECK IF EXTRA PADDING NEEDED
  // Check top and bottom borders
  y1 = pTile->y - tileEdgeTop;
  if (y1 > frameHeight + framePadBottom)
  {
    y1 = frameHeight + framePadBottom;
  }
  if (y1 < -framePadTop)
  {
    extraEdgeTop = -framePadTop - y1;
    y1           = -framePadTop;
    statusFlag  |= XV_TILE_STATUS_TOP_EDGE_PADDING_NEEDED;
  }
  else
  {
    extraEdgeTop = 0;
  }

  y2 = pTile->y + (tileHeight - 1) + tileEdgeBottom;
  if (y2 < -framePadTop)
  {
    y2 = -framePadTop - 1;
  }
  if (y2 > frameHeight - 1 + framePadBottom)
  {
    extraEdgeBottom = y2 - (frameHeight - 1 + framePadBottom);
    y2              = frameHeight - 1 + framePadBottom;
    statusFlag     |= XV_TILE_STATUS_BOTTOM_EDGE_PADDING_NEEDED;
  }
  else
  {
    extraEdgeBottom = 0;
  }

  // Check left and right borders
  x1 = pTile->x - tileEdgeLeft;
  if (x1 > frameWidth + framePadRight)
  {
    x1 = frameWidth + framePadRight;
  }
  if (x1 < -framePadLeft)
  {
    extraEdgeLeft = -framePadLeft - x1;
    x1            = -framePadLeft;
    statusFlag   |= XV_TILE_STATUS_LEFT_EDGE_PADDING_NEEDED;
  }
  else
  {
    extraEdgeLeft = 0;
  }

  x2 = pTile->x + (tileWidth - 1) + tileEdgeRight;
  if (x2 < -framePadLeft)
  {
    x2 = -framePadLeft - 1;
  }
  if (x2 > frameWidth - 1 + framePadRight)
  {
    extraEdgeRight = x2 - (frameWidth - 1 + framePadRight);
    x2             = frameWidth - 1 + framePadRight;
    statusFlag    |= XV_TILE_STATUS_RIGHT_EDGE_PADDING_NEEDED;
  }
  else
  {
    extraEdgeRight = 0;
  }

  // 2. FILL ALL TILE and DMA RELATED DATA
  // No Need to align srcPtr and dstPtr as DMA does not need aligned start
  srcPtr        = (uint8_t *) pFrame->pFrameData + y1 * framePitchBytes + x1 * pixWidth;
  dmaHeight     = y2 - y1 + 1;
  dmaWidthBytes = (x2 - x1 + 1) * pixWidth;
  edgePtr       = (uint8_t *) pTile->pData - (tileEdgeTop * tilePitchBytes + tileEdgeLeft * pixWidth);
  dstPtr        = edgePtr + (extraEdgeTop * tilePitchBytes + extraEdgeLeft * pixWidth); // For DMA

  // 3. DATA REUSE FROM PREVIOUS TILE
  if (dmaHeight > 0 && dmaWidthBytes > 0)
  {
    if (pPrevTile != NULL)
    {
#ifndef XV_EMULATE_DMA
      if (XV_IS_TILE_OK(pPrevTile) == 0)
      {
        pxvTM->errFlag = XV_ERROR_BAD_ARG;
        return(XVTM_ERROR);
      }
#endif

      py1       = pPrevTile->y - ((int32_t) pPrevTile->tileEdgeTop);
      py2       = pPrevTile->y + pPrevTile->height - 1 + ((int32_t) pPrevTile->tileEdgeBottom);
      px1       = pPrevTile->x - ((int32_t) pPrevTile->tileEdgeLeft);
      px2       = pPrevTile->x + pPrevTile->width - 1 + ((int32_t) pPrevTile->tileEdgeRight);
      pPrevBuff = (uint8_t *) pPrevTile->pData - (pPrevTile->tileEdgeTop * pPrevTile->pitch * pixRes + pPrevTile->tileEdgeLeft * pixWidth); // Same pixRes for current and prev tile
      // Case 1. Two tiles are totally independent. DMA entire tile. No copying
      if ((py2 < y1) || (py1 > y2) || (px2 < x1) || (px1 > x2))
      {
        dmaIndex = addIdmaRequestInline(pxvTM, dstPtr, srcPtr, dmaWidthBytes, dmaHeight, framePitchBytes, tilePitchBytes, interruptOnCompletion);
      }
      else
      {
        pTile->pPrevTile = pPrevTile;
        pPrevTile->reuseCount++;

        // Case 2. Only top part overlaps.
        if ((py1 <= y1) && (y1 <= py2) && (py2 < y2))
        {
          // Top part
          pCurrBuff  = dstPtr;
          pPrevBuff += (y1 - py1) * pPrevTile->pitch * pixRes;
          // Bottom part
          srcPtr    = (uint8_t *) pFrame->pFrameData + ((py2 + 1) * framePitchBytes + x1 * pixWidth);
          dstPtr   += (py2 + 1 - y1) * tilePitchBytes;
          dmaHeight = y2 - py2;  // (y2+1) - (py2+1)
          dmaIndex  = addIdmaRequestInline(pxvTM, dstPtr, srcPtr, dmaWidthBytes, dmaHeight, framePitchBytes, tilePitchBytes, 0);
          // Top part
          dmaIndex = solveForX(pxvTM, pTile, pCurrBuff, pPrevBuff, y1, py2, x1, x2, px1, px2, tilePitchBytes, pPrevTile->pitch * pixRes, interruptOnCompletion);
        }

        // Case 3. Only mid part overlaps
        if ((y1 < py1) && (py2 < y2))
        {
          // Top part
          dmaHeight = py1 + 1 - y1;
          dmaIndex  = addIdmaRequestInline(pxvTM, dstPtr, srcPtr, dmaWidthBytes, dmaHeight, framePitchBytes, tilePitchBytes, 0);
          // Mid overlapping part
          pCurrBuff = dstPtr + (py1 - y1) * tilePitchBytes;
          // Bottom part
          srcPtr    = (uint8_t *) pFrame->pFrameData + ((py2 + 1) * framePitchBytes + x1 * pixWidth);
          dstPtr   += (py2 + 1 - y1) * tilePitchBytes;
          dmaHeight = y2 - py2;  // (y2+1) - (py2+1)
          dmaIndex  = addIdmaRequestInline(pxvTM, dstPtr, srcPtr, dmaWidthBytes, dmaHeight, framePitchBytes, tilePitchBytes, 0);
          dmaIndex  = solveForX(pxvTM, pTile, pCurrBuff, pPrevBuff, py1, py2, x1, x2, px1, px2, tilePitchBytes, pPrevTile->pitch * pixRes, interruptOnCompletion);
        }

        // Case 4. Only bottom part overlaps
        if ((y1 < py1) && (py1 <= y2) && (y2 <= py2))
        {
          dmaHeight = py1 - y1;
          dmaIndex  = addIdmaRequestInline(pxvTM, dstPtr, srcPtr, dmaWidthBytes, dmaHeight, framePitchBytes, tilePitchBytes, 0);
          pCurrBuff = dstPtr + dmaHeight * tilePitchBytes;
          dmaIndex  = solveForX(pxvTM, pTile, pCurrBuff, pPrevBuff, py1, y2, x1, x2, px1, px2, tilePitchBytes, pPrevTile->pitch * pixRes, interruptOnCompletion);
        }

        // Case 5. All the parts overlaps
        if ((py1 <= y1) && (y2 <= py2))
        {
          pCurrBuff  = dstPtr;
          pPrevBuff += (y1 - py1) * pPrevTile->pitch * pixRes;
          dmaIndex   = solveForX(pxvTM, pTile, pCurrBuff, pPrevBuff, y1, y2, x1, x2, px1, px2, tilePitchBytes, pPrevTile->pitch * pixRes, interruptOnCompletion);
        }
      }
    }
    else
    {
      dmaIndex = addIdmaRequestInline(pxvTM, dstPtr, srcPtr, dmaWidthBytes, dmaHeight, framePitchBytes, tilePitchBytes, interruptOnCompletion);
    }
  }
  else
  {
    dmaIndex = XVTM_DUMMY_DMA_INDEX;
  }
  pTile->status = statusFlag | XV_TILE_STATUS_DMA_ONGOING;
  //tileIndex     = (pxvTM->tileDMAreqCount & (MAX_NUM_DMA_QUEUE_LENGTH - 1));
  tileIndex = (pxvTM->tileDMAstartIndex + pxvTM->tileDMApendingCount) % MAX_NUM_DMA_QUEUE_LENGTH;
  pxvTM->tileDMApendingCount++;
  pxvTM->tileProcQueue[tileIndex] = pTile;
  pTile->dmaIndex                 = dmaIndex;
  return(XVTM_SUCCESS);
}

/**********************************************************************************
 * FUNCTION: xvReqTileTransferInFast()
 *
 * DESCRIPTION:
 *     Requests 8b data transfer from frame present in system memory to local tile memory.
 *
 * INPUTS:
 *     xvTileManager *pxvTM                   Tile Manager object
 *     xvTile        *pTile                   Destination tile
 *     int32_t       interruptOnCompletion    If it is set, iDMA will interrupt after completing transfer
 *
 * OUTPUTS:
 *     Returns XVTM_ERROR if it encounters an error, else it returns XVTM_SUCCESS
 *
 ********************************************************************************** */

int32_t xvReqTileTransferInFast(xvTileManager *pxvTM, xvTile *pTile, int32_t interruptOnCompletion)
{
  xvFrame *pFrame;
  int32_t frameWidth, frameHeight, framePitchBytes, tileWidth, tileHeight, tilePitchBytes;
  int32_t statusFlag, x1, y1, x2, y2, dmaHeight, dmaWidthBytes, dmaIndex, copyRowBytes;
  int16_t tileEdgeLeft, tileEdgeRight, tileEdgeTop, tileEdgeBottom;
  int16_t extraEdgeTop, extraEdgeBottom, extraEdgeLeft, extraEdgeRight;
  uint8_t *srcPtr, *dstPtr, *edgePtr;

  pFrame = pTile->pFrame;

  frameWidth      = pFrame->frameWidth;
  frameHeight     = pFrame->frameHeight;
  framePitchBytes = pFrame->framePitch;

  tileWidth      = pTile->width;
  tileHeight     = pTile->height;
  tilePitchBytes = pTile->pitch;
  tileEdgeLeft   = pTile->tileEdgeLeft;
  tileEdgeRight  = pTile->tileEdgeRight;
  tileEdgeTop    = pTile->tileEdgeTop;
  tileEdgeBottom = pTile->tileEdgeBottom;

  statusFlag      = pTile->status;
  extraEdgeTop    = 0;
  extraEdgeBottom = 0;
  extraEdgeLeft   = 0;
  extraEdgeRight  = 0;
  // 1. CHECK IF EXTRA PADDING NEEDED
  // Check top and bottom borders
  y1 = pTile->y - tileEdgeTop;
  if (y1 > frameHeight)
  {
    y1 = frameHeight;
  }
  if (y1 < 0)
  {
    extraEdgeTop = -y1;
    y1           = 0;
    statusFlag  |= XV_TILE_STATUS_TOP_EDGE_PADDING_NEEDED;
  }

  y2 = pTile->y + (tileHeight - 1) + tileEdgeBottom;
  if (y2 < 0)
  {
    y2 = -1;
  }
  if (y2 > frameHeight - 1)
  {
    extraEdgeBottom = y2 - frameHeight + 1;
    y2              = frameHeight - 1;
    statusFlag     |= XV_TILE_STATUS_BOTTOM_EDGE_PADDING_NEEDED;
  }

  // Check left and right borders
  x1 = pTile->x - tileEdgeLeft;
  if (x1 > frameWidth)
  {
    x1 = frameWidth;
  }
  if (x1 < 0)
  {
    extraEdgeLeft = -x1;
    x1            = 0;
    statusFlag   |= XV_TILE_STATUS_LEFT_EDGE_PADDING_NEEDED;
  }

  x2 = pTile->x + (tileWidth - 1) + tileEdgeRight;
  if (x2 < 0)
  {
    x2 = -1;
  }
  if (x2 > frameWidth - 1)
  {
    extraEdgeRight = x2 - frameWidth + 1;
    x2             = frameWidth - 1;
    statusFlag    |= XV_TILE_STATUS_RIGHT_EDGE_PADDING_NEEDED;
  }

  // 2. FILL ALL TILE and DMA RELATED DATA
  // No Need to align srcPtr as DMA does not need aligned start
  // But dstPtr needs to be aligned
  dmaHeight     = y2 - y1 + 1;
  dmaWidthBytes = (x2 - x1 + 1);

  interruptOnCompletion = interruptOnCompletion ? DESC_NOTIFY_W_INT : 0;
  uint32_t intrCompletionFlag;

  if (dmaHeight > 0 && dmaWidthBytes > 0)
  {
    srcPtr  = (uint8_t *) pFrame->pFrameData + y1 * framePitchBytes + x1;
    edgePtr = (uint8_t *) pTile->pData - (tileEdgeTop * tilePitchBytes + tileEdgeLeft);
    dstPtr  = edgePtr + (extraEdgeTop * tilePitchBytes + extraEdgeLeft); // For DMA

    // One interrupt per tile request. Interrupt only if it is last DMA request for this tile.
    intrCompletionFlag = interruptOnCompletion * !((statusFlag & (XV_TILE_STATUS_TOP_EDGE_PADDING_NEEDED | XV_TILE_STATUS_BOTTOM_EDGE_PADDING_NEEDED)) && (pFrame->paddingType == FRAME_EDGE_PADDING));
    TM_LOG_PRINT("src: %p, dst, %p, rowsize: %d, numRows: %d, srcPitch: %d, dstPitch: %d, flags: 0x%x\n",
                 srcPtr, dstPtr, dmaWidthBytes, dmaHeight, framePitchBytes, tilePitchBytes, intrCompletionFlag);
    dmaIndex = idma_copy_2d_desc(dstPtr, srcPtr, dmaWidthBytes, intrCompletionFlag, dmaHeight, framePitchBytes, tilePitchBytes);
    TM_LOG_PRINT(" dmaIndex: %d\n", dmaIndex);

    if ((statusFlag & XV_TILE_STATUS_TOP_EDGE_PADDING_NEEDED) && (pFrame->paddingType == FRAME_EDGE_PADDING))
    {
      srcPtr       = (uint8_t *) pTile->pData - (tileEdgeTop - extraEdgeTop) * tilePitchBytes - tileEdgeLeft;
      dstPtr       = srcPtr - extraEdgeTop * tilePitchBytes;
      copyRowBytes = (tileEdgeLeft + tileWidth + tileEdgeRight);

      // One interrupt per tile request. Interrupt only if it is last DMA request for this tile.
      intrCompletionFlag = interruptOnCompletion * !((statusFlag & XV_TILE_STATUS_BOTTOM_EDGE_PADDING_NEEDED) && (pFrame->paddingType == FRAME_EDGE_PADDING));
      TM_LOG_PRINT("src: %p, dst, %p, rowsize: %d, numRows: %d, srcPitch: %d, dstPitch: %d, flags: 0x%x\n",
                   srcPtr, dstPtr, copyRowBytes, extraEdgeTop, 0, tilePitchBytes, intrCompletionFlag);
      dmaIndex = idma_copy_2d_desc(dstPtr, srcPtr, copyRowBytes, intrCompletionFlag, extraEdgeTop, 0, tilePitchBytes);
      TM_LOG_PRINT(" dmaIndex: %d\n", dmaIndex);
      statusFlag = statusFlag & ~XV_TILE_STATUS_TOP_EDGE_PADDING_NEEDED;
    }

    if ((statusFlag & XV_TILE_STATUS_BOTTOM_EDGE_PADDING_NEEDED) && (pFrame->paddingType == FRAME_EDGE_PADDING))
    {
      srcPtr       = (uint8_t *) pTile->pData - tileEdgeLeft + (tileHeight + tileEdgeBottom - extraEdgeBottom - 1) * tilePitchBytes;
      dstPtr       = srcPtr + tilePitchBytes;
      copyRowBytes = (tileEdgeLeft + tileWidth + tileEdgeRight);
      TM_LOG_PRINT("src: %p, dst, %p, rowsize: %d, numRows: %d, srcPitch: %d, dstPitch: %d, flags: 0x%x\n",
                   srcPtr, dstPtr, copyRowBytes, extraEdgeBottom, 0, tilePitchBytes, interruptOnCompletion);
      dmaIndex = idma_copy_2d_desc(dstPtr, srcPtr, copyRowBytes, interruptOnCompletion, extraEdgeBottom, 0, tilePitchBytes);
      TM_LOG_PRINT(" dmaIndex: %d\n", dmaIndex);
      statusFlag = statusFlag & ~XV_TILE_STATUS_BOTTOM_EDGE_PADDING_NEEDED;
    }
  }
  else
  {
    dmaIndex = XVTM_DUMMY_DMA_INDEX;
  }
  pTile->status   = statusFlag | XV_TILE_STATUS_DMA_ONGOING;
  pTile->dmaIndex = dmaIndex;
  return(XVTM_SUCCESS);
}

/**********************************************************************************
 * FUNCTION: xvReqTileTransferInFast16()
 *
 * DESCRIPTION:
 *     Requests 16b data transfer from frame present in system memory to local tile memory.
 *
 * INPUTS:
 *     xvTileManager *pxvTM                   Tile Manager object
 *     xvTile        *pTile                   Destination tile
 *     int32_t       interruptOnCompletion    If it is set, iDMA will interrupt after completing transfer
 *
 * OUTPUTS:
 *     Returns XVTM_ERROR if it encounters an error, else it returns XVTM_SUCCESS
 *
 ********************************************************************************** */

int32_t xvReqTileTransferInFast16(xvTileManager *pxvTM, xvTile *pTile, int32_t interruptOnCompletion)
{
  xvFrame *pFrame;
  int32_t frameWidth, frameHeight, framePitchBytes, tileWidth, tileHeight, tilePitchBytes;
  int32_t statusFlag, x1, y1, x2, y2, dmaHeight, dmaWidthBytes, dmaIndex, copyRowBytes;
  int16_t tileEdgeLeft, tileEdgeRight, tileEdgeTop, tileEdgeBottom;
  int16_t extraEdgeTop, extraEdgeBottom, extraEdgeLeft, extraEdgeRight;
  uint8_t *srcPtr, *dstPtr, *edgePtr;

  pFrame = pTile->pFrame;

  frameWidth      = pFrame->frameWidth;
  frameHeight     = pFrame->frameHeight;
  framePitchBytes = pFrame->framePitch * 2;

  tileWidth      = pTile->width;
  tileHeight     = pTile->height;
  tilePitchBytes = pTile->pitch * 2;
  tileEdgeLeft   = pTile->tileEdgeLeft;
  tileEdgeRight  = pTile->tileEdgeRight;
  tileEdgeTop    = pTile->tileEdgeTop;
  tileEdgeBottom = pTile->tileEdgeBottom;

  statusFlag      = pTile->status;
  extraEdgeTop    = 0;
  extraEdgeBottom = 0;
  extraEdgeLeft   = 0;
  extraEdgeRight  = 0;
  // 1. CHECK IF EXTRA PADDING NEEDED
  // Check top and bottom borders
  y1 = pTile->y - tileEdgeTop;
  if (y1 > frameHeight)
  {
    y1 = frameHeight;
  }
  if (y1 < 0)
  {
    extraEdgeTop = -y1;
    y1           = 0;
    statusFlag  |= XV_TILE_STATUS_TOP_EDGE_PADDING_NEEDED;
  }

  y2 = pTile->y + (tileHeight - 1) + tileEdgeBottom;
  if (y2 < 0)
  {
    y2 = -1;
  }
  if (y2 > frameHeight - 1)
  {
    extraEdgeBottom = y2 - frameHeight + 1;
    y2              = frameHeight - 1;
    statusFlag     |= XV_TILE_STATUS_BOTTOM_EDGE_PADDING_NEEDED;
  }

  // Check left and right borders
  x1 = pTile->x - tileEdgeLeft;
  if (x1 > frameWidth)
  {
    x1 = frameWidth;
  }
  if (x1 < 0)
  {
    extraEdgeLeft = -x1;
    x1            = 0;
    statusFlag   |= XV_TILE_STATUS_LEFT_EDGE_PADDING_NEEDED;
  }

  x2 = pTile->x + (tileWidth - 1) + tileEdgeRight;
  if (x2 < 0)
  {
    x2 = -1;
  }
  if (x2 > frameWidth - 1)
  {
    extraEdgeRight = x2 - frameWidth + 1;
    x2             = frameWidth - 1;
    statusFlag    |= XV_TILE_STATUS_RIGHT_EDGE_PADDING_NEEDED;
  }

  // 2. FILL ALL TILE and DMA RELATED DATA
  // No Need to align srcPtr as DMA does not need aligned start
  // But dstPtr needs to be aligned
  dmaHeight     = y2 - y1 + 1;
  dmaWidthBytes = (x2 - x1 + 1) * 2;

  interruptOnCompletion = interruptOnCompletion ? DESC_NOTIFY_W_INT : 0;
  uint32_t intrCompletionFlag;

  if (dmaHeight > 0 && dmaWidthBytes > 0)
  {
    srcPtr  = (uint8_t *) pFrame->pFrameData + y1 * framePitchBytes + (x1 * 2);
    edgePtr = (uint8_t *) pTile->pData - (tileEdgeTop * tilePitchBytes + (tileEdgeLeft * 2));
    dstPtr  = edgePtr + (extraEdgeTop * tilePitchBytes + (extraEdgeLeft * 2)); // For DMA

    // One interrupt per tile request. Interrupt only if it is last DMA request for this tile.
    intrCompletionFlag = interruptOnCompletion * !((statusFlag & (XV_TILE_STATUS_TOP_EDGE_PADDING_NEEDED | XV_TILE_STATUS_BOTTOM_EDGE_PADDING_NEEDED)) && (pFrame->paddingType == FRAME_EDGE_PADDING));
    TM_LOG_PRINT("src: %p, dst, %p, rowsize: %d, numRows: %d, srcPitch: %d, dstPitch: %d, flags: 0x%x\n",
                 srcPtr, dstPtr, dmaWidthBytes, dmaHeight, framePitchBytes, tilePitchBytes, intrCompletionFlag);
    dmaIndex = idma_copy_2d_desc(dstPtr, srcPtr, dmaWidthBytes, intrCompletionFlag, dmaHeight, framePitchBytes, tilePitchBytes);
    TM_LOG_PRINT(" dmaIndex: %d\n", dmaIndex);

    if ((statusFlag & XV_TILE_STATUS_TOP_EDGE_PADDING_NEEDED) && (pFrame->paddingType == FRAME_EDGE_PADDING))
    {
      srcPtr       = (uint8_t *) pTile->pData - (tileEdgeTop - extraEdgeTop) * tilePitchBytes - (tileEdgeLeft * 2);
      dstPtr       = srcPtr - extraEdgeTop * tilePitchBytes;
      copyRowBytes = (tileEdgeLeft + tileWidth + tileEdgeRight) * 2;

      // One interrupt per tile request. Interrupt only if it is last DMA request for this tile.
      intrCompletionFlag = interruptOnCompletion * !((statusFlag & XV_TILE_STATUS_BOTTOM_EDGE_PADDING_NEEDED) && (pFrame->paddingType == FRAME_EDGE_PADDING));
      TM_LOG_PRINT("src: %p, dst, %p, rowsize: %d, numRows: %d, srcPitch: %d, dstPitch: %d, flags: 0x%x\n",
                   srcPtr, dstPtr, copyRowBytes, extraEdgeTop, 0, tilePitchBytes, intrCompletionFlag);
      dmaIndex = idma_copy_2d_desc(dstPtr, srcPtr, copyRowBytes, intrCompletionFlag, extraEdgeTop, 0, tilePitchBytes);
      TM_LOG_PRINT(" dmaIndex: %d\n", dmaIndex);
      statusFlag = statusFlag & ~XV_TILE_STATUS_TOP_EDGE_PADDING_NEEDED;
    }

    if ((statusFlag & XV_TILE_STATUS_BOTTOM_EDGE_PADDING_NEEDED) && (pFrame->paddingType == FRAME_EDGE_PADDING))
    {
      srcPtr       = (uint8_t *) pTile->pData - (tileEdgeLeft * 2) + (tileHeight + tileEdgeBottom - extraEdgeBottom - 1) * tilePitchBytes;
      dstPtr       = srcPtr + tilePitchBytes;
      copyRowBytes = (tileEdgeLeft + tileWidth + tileEdgeRight) * 2;
      TM_LOG_PRINT("src: %p, dst, %p, rowsize: %d, numRows: %d, srcPitch: %d, dstPitch: %d, flags: 0x%x\n",
                   srcPtr, dstPtr, copyRowBytes, extraEdgeBottom, 0, tilePitchBytes, interruptOnCompletion);
      dmaIndex = idma_copy_2d_desc(dstPtr, srcPtr, copyRowBytes, interruptOnCompletion, extraEdgeBottom, 0, tilePitchBytes);
      TM_LOG_PRINT(" dmaIndex: %d\n", dmaIndex);
      statusFlag = statusFlag & ~XV_TILE_STATUS_BOTTOM_EDGE_PADDING_NEEDED;
    }
  }
  else
  {
    dmaIndex = XVTM_DUMMY_DMA_INDEX;
  }
  pTile->status   = statusFlag | XV_TILE_STATUS_DMA_ONGOING;
  pTile->dmaIndex = dmaIndex;
  return(XVTM_SUCCESS);
}

/**********************************************************************************
 * FUNCTION: xvReqTileTransferOut()
 *
 * DESCRIPTION:
 *     Requests data transfer from tile present in local memory to frame in system memory.
 *
 * INPUTS:
 *     xvTileManager *pxvTM                   Tile Manager object
 *     xvTile        *pTile                   Source tile
 *     int32_t       interruptOnCompletion    If it is set, iDMA will interrupt after completing transfer
 *
 * OUTPUTS:
 *     Returns XVTM_ERROR if it encounters an error, else it returns XVTM_SUCCESS
 *
 ********************************************************************************** */

int32_t xvReqTileTransferOut(xvTileManager *pxvTM, xvTile *pTile, int32_t interruptOnCompletion)
{
  xvFrame *pFrame;
  uint8_t *srcPtr, *dstPtr;
  int32_t pixWidth, tileIndex, dmaIndex;
  int32_t srcHeight, srcWidth, srcPitchBytes;
  int32_t dstPitchBytes, numRows, rowSize;

  if (pxvTM == NULL)
  {
    return(XVTM_ERROR);
  }
  pxvTM->errFlag = XV_ERROR_SUCCESS;

  if (pTile == NULL)
  {
    pxvTM->errFlag = XV_ERROR_TILE_NULL;
    return(XVTM_ERROR);
  }

  if (XV_IS_TILE_OK(pTile) == 0)
  {
    pxvTM->errFlag = XV_ERROR_BAD_ARG;
    return(XVTM_ERROR);
  }

  pFrame = pTile->pFrame;
  if (pFrame == NULL || pFrame->pFrameBuff == NULL || pFrame->pFrameData == NULL)
  {
    pxvTM->errFlag = XV_ERROR_FRAME_NULL;
    return(XVTM_ERROR);
  }
  pixWidth      = pFrame->pixelRes * pFrame->numChannels;
  srcHeight     = pTile->height;
  srcWidth      = pTile->width;
  srcPitchBytes = pTile->pitch * pFrame->pixelRes;
  dstPitchBytes = pFrame->framePitch * pFrame->pixelRes;
  numRows       = XVTM_MIN(srcHeight, pFrame->frameHeight - pTile->y);
  rowSize       = XVTM_MIN(srcWidth, (pFrame->frameWidth - pTile->x));

  srcPtr = (uint8_t *) pTile->pData;
  dstPtr = (uint8_t *) pFrame->pFrameData + (pTile->y * dstPitchBytes + pTile->x * pixWidth);

  if (pTile->x < 0)
  {
    rowSize += pTile->x;       // x is negative;
    srcPtr  += (-pTile->x * pixWidth);
    dstPtr  += (-pTile->x * pixWidth);
  }

  if (pTile->y < 0)
  {
    numRows += pTile->y;       // y is negative;
    srcPtr  += (-pTile->y * srcPitchBytes);
    dstPtr  += (-pTile->y * dstPitchBytes);
  }
  rowSize *= pixWidth;

  if ((rowSize != 0) && (numRows != 0))
  {
    pTile->status = pTile->status | XV_TILE_STATUS_DMA_ONGOING;
    tileIndex     = (pxvTM->tileDMAstartIndex + pxvTM->tileDMApendingCount) % MAX_NUM_DMA_QUEUE_LENGTH;
    pxvTM->tileDMApendingCount++;
    pxvTM->tileProcQueue[tileIndex] = pTile;
    dmaIndex                        = addIdmaRequestInline(pxvTM, dstPtr, srcPtr, rowSize, numRows, srcPitchBytes, dstPitchBytes, interruptOnCompletion);
    pTile->dmaIndex                 = dmaIndex;
  }
  return(XVTM_SUCCESS);
}

/**********************************************************************************
 * FUNCTION: xvReqTileTransferOutFast()
 *
 * DESCRIPTION:
 *     Requests data transfer from tile present in local memory to frame in system memory.
 *
 * INPUTS:
 *     xvTileManager *pxvTM                   Tile Manager object
 *     xvTile        *pTile                   Source tile
 *     int32_t       interruptOnCompletion    If it is set, iDMA will interrupt after completing transfer
 *
 * OUTPUTS:
 *     Returns XVTM_ERROR if it encounters an error, else it returns XVTM_SUCCESS
 *
 ********************************************************************************** */

int32_t xvReqTileTransferOutFast(xvTileManager *pxvTM, xvTile *pTile, int32_t interruptOnCompletion)
{
  xvFrame *pFrame;
  uint8_t *srcPtr, *dstPtr;
  int32_t dmaIndex;
  int32_t srcHeight, srcWidth, srcPitchBytes;
  int32_t dstPitchBytes, numRows, rowSize;

  pFrame = pTile->pFrame;

  srcHeight     = pTile->height;
  srcWidth      = pTile->width;
  srcPitchBytes = pTile->pitch;
  dstPitchBytes = pFrame->framePitch;
  numRows       = XVTM_MIN(srcHeight, pFrame->frameHeight - pTile->y);
  rowSize       = XVTM_MIN(srcWidth, (pFrame->frameWidth - pTile->x));

  srcPtr = (uint8_t *) pTile->pData;
  dstPtr = (uint8_t *) pFrame->pFrameData + (pTile->y * dstPitchBytes + pTile->x);

  if (pTile->x < 0)
  {
    rowSize += pTile->x;       // x is negative;
    srcPtr  += (-pTile->x);
    dstPtr  += (-pTile->x);
  }

  if (pTile->y < 0)
  {
    numRows += pTile->y;       // y is negative;
    srcPtr  += (-pTile->y * srcPitchBytes);
    dstPtr  += (-pTile->y * dstPitchBytes);
  }

  uint32_t intrCompletionFlag = interruptOnCompletion ? DESC_NOTIFY_W_INT : 0;

  if ((rowSize != 0) && (numRows != 0))
  {
    pTile->status = pTile->status | XV_TILE_STATUS_DMA_ONGOING;
    TM_LOG_PRINT("src: %p, dst, %p, rowsize: %d, numRows: %d, srcPitch: %d, dstPitch: %d, flags: 0x%x\n",
                 srcPtr, dstPtr, rowSize, numRows, srcPitchBytes, dstPitchBytes, intrCompletionFlag);
    dmaIndex = idma_copy_2d_desc(dstPtr, srcPtr, rowSize, intrCompletionFlag, numRows, srcPitchBytes, dstPitchBytes);
    TM_LOG_PRINT(" dmaIndex: %d\n", dmaIndex);
    pTile->dmaIndex = dmaIndex;
  }
  return(XVTM_SUCCESS);
}

/**********************************************************************************
 * FUNCTION: xvReqTileTransferOutFast16()
 *
 * DESCRIPTION:
 *     Requests 16b data transfer from tile present in local memory to frame in system memory.
 *
 * INPUTS:
 *     xvTileManager *pxvTM                   Tile Manager object
 *     xvTile        *pTile                   Source tile
 *     int32_t       interruptOnCompletion    If it is set, iDMA will interrupt after completing transfer
 *
 * OUTPUTS:
 *     Returns XVTM_ERROR if it encounters an error, else it returns XVTM_SUCCESS
 *
 ********************************************************************************** */

int32_t xvReqTileTransferOutFast16(xvTileManager *pxvTM, xvTile *pTile, int32_t interruptOnCompletion)
{
  xvFrame *pFrame;
  uint8_t *srcPtr, *dstPtr;
  int32_t dmaIndex;
  int32_t srcHeight, srcWidth, srcPitchBytes;
  int32_t dstPitchBytes, numRows, rowSize;

  pFrame = pTile->pFrame;

  srcHeight     = pTile->height;
  srcWidth      = pTile->width;
  srcPitchBytes = pTile->pitch * 2;
  dstPitchBytes = pFrame->framePitch * 2;
  numRows       = XVTM_MIN(srcHeight, pFrame->frameHeight - pTile->y);
  rowSize       = XVTM_MIN(srcWidth, (pFrame->frameWidth - pTile->x));

  srcPtr = (uint8_t *) pTile->pData;
  dstPtr = (uint8_t *) pFrame->pFrameData + (pTile->y * dstPitchBytes + (pTile->x * 2));

  if (pTile->x < 0)
  {
    rowSize += pTile->x;       // x is negative;
    srcPtr  += (-pTile->x * 2);
    dstPtr  += (-pTile->x * 2);
  }

  if (pTile->y < 0)
  {
    numRows += pTile->y;       // y is negative;
    srcPtr  += (-pTile->y * srcPitchBytes);
    dstPtr  += (-pTile->y * dstPitchBytes);
  }
  rowSize *= 2;

  uint32_t intrCompletionFlag = interruptOnCompletion ? DESC_NOTIFY_W_INT : 0;

  if ((rowSize != 0) && (numRows != 0))
  {
    pTile->status = pTile->status | XV_TILE_STATUS_DMA_ONGOING;
    TM_LOG_PRINT("src: %p, dst, %p, rowsize: %d, numRows: %d, srcPitch: %d, dstPitch: %d, flags: 0x%x\n",
                 srcPtr, dstPtr, rowSize, numRows, srcPitchBytes, dstPitchBytes, intrCompletionFlag);
    dmaIndex = idma_copy_2d_desc(dstPtr, srcPtr, rowSize, intrCompletionFlag, numRows, srcPitchBytes, dstPitchBytes);
    TM_LOG_PRINT(" dmaIndex: %d\n", dmaIndex);
    pTile->dmaIndex = dmaIndex;
  }
  return(XVTM_SUCCESS);
}

/**********************************************************************************
 * FUNCTION: xvCheckForIdmaIndex()
 *
 * DESCRIPTION:
 *     Checks if DMA transfer for given index is completed
 *
 * INPUTS:
 *     xvTileManager *pxvTM                   Tile Manager object
 *     int32_t       index                    Index of the dma transfer request, returned by xvAddIdmaRequest()
 *
 * OUTPUTS:
 *     Returns ONE if transfer is complete and ZERO if it is not
 *     Returns XVTM_ERROR if an error occurs
 *
 ********************************************************************************** */

int32_t xvCheckForIdmaIndex(xvTileManager *pxvTM, int32_t index)
{
  int32_t retVal = 1;
  if (pxvTM == NULL)
  {
    return(XVTM_ERROR);
  }
#ifndef XV_EMULATE_DMA
  retVal = idma_desc_done(index);
#endif
  return(retVal);
}

/**********************************************************************************
 * FUNCTION: xvCheckTileReady()
 *
 * DESCRIPTION:
 *     Checks if DMA transfer for given tile is completed.
 *     It checks all the tile in the transfer request buffer
 *     before the given tile and updates their respective
 *     status. It pads edges wherever required.
 *
 * INPUTS:
 *     xvTileManager *pxvTM                   Tile Manager object
 *     xvTile        *pTile                   Input tile
 *
 * OUTPUTS:
 *     Returns ONE if dma transfer for input tile is complete and ZERO if it is not
 *     Returns XVTM_ERROR if an error occurs
 *
 ********************************************************************************** */

int32_t xvCheckTileReady(xvTileManager *pxvTM, xvTile *pTile)
{
  int32_t loopInd, index, retVal, dmaIndex, statusFlag, doneCount;
  xvTile *pTile1;
  xvFrame *pFrame;
  int32_t frameWidth, frameHeight, framePitchBytes, tileWidth, tileHeight, tilePitchBytes;
  int32_t x1, y1, x2, y2, copyRowBytes, copyHeight;
  int8_t framePadLeft, framePadRight, framePadTop, framePadBottom;
  int16_t tileEdgeLeft, tileEdgeRight, tileEdgeTop, tileEdgeBottom;
  int16_t extraEdgeTop, extraEdgeBottom, extraEdgeLeft, extraEdgeRight;
  int8_t pixWidth;
  uint8_t *srcPtr, *dstPtr;

  if (pxvTM == NULL)
  {
    return(XVTM_ERROR);
  }
  pxvTM->errFlag = XV_ERROR_SUCCESS;

  if (pTile == NULL)
  {
    pxvTM->errFlag = XV_ERROR_TILE_NULL;
    return(XVTM_ERROR);
  }

  doneCount = 0;
  index     = 0;
  for (loopInd = 0; loopInd < pxvTM->tileDMApendingCount; loopInd++)
  {
    index    = (pxvTM->tileDMAstartIndex + loopInd) % MAX_NUM_DMA_QUEUE_LENGTH;
    pTile1   = pxvTM->tileProcQueue[index % MAX_NUM_DMA_QUEUE_LENGTH];
    dmaIndex = pTile1->dmaIndex;
    if (dmaIndex != XVTM_DUMMY_DMA_INDEX)
    {
      retVal = xvCheckForIdmaIndex(pxvTM, dmaIndex);
      if (retVal == 1)
      {
        statusFlag = pTile1->status;
        statusFlag = statusFlag & ~XV_TILE_STATUS_DMA_ONGOING;

        pFrame          = pTile1->pFrame;
        pixWidth        = pFrame->pixelRes * pFrame->numChannels;
        frameWidth      = pFrame->frameWidth;
        frameHeight     = pFrame->frameHeight;
        framePitchBytes = pFrame->framePitch * pFrame->pixelRes;
        framePadLeft    = pFrame->leftEdgePadWidth;
        framePadRight   = pFrame->rightEdgePadWidth;
        framePadTop     = pFrame->topEdgePadHeight;
        framePadBottom  = pFrame->bottomEdgePadHeight;

        tileWidth      = pTile1->width;
        tileHeight     = pTile1->height;
        tilePitchBytes = pTile1->pitch * pFrame->pixelRes;
        tileEdgeLeft   = pTile1->tileEdgeLeft;
        tileEdgeRight  = pTile1->tileEdgeRight;
        tileEdgeTop    = pTile1->tileEdgeTop;
        tileEdgeBottom = pTile1->tileEdgeBottom;

        if (statusFlag & XV_TILE_STATUS_EDGE_PADDING_NEEDED)
        {
          if (statusFlag & XV_TILE_STATUS_TOP_EDGE_PADDING_NEEDED)
          {
            y1           = pTile1->y - tileEdgeTop;
            extraEdgeTop = -framePadTop - y1;
            dstPtr       = (uint8_t *) pTile1->pData - (tileEdgeTop * tilePitchBytes + tileEdgeLeft * pixWidth);
            srcPtr       = dstPtr + extraEdgeTop * tilePitchBytes;
            copyRowBytes = (tileEdgeLeft + tileWidth + tileEdgeRight) * XV_TYPE_ELEMENT_SIZE(pTile1->type);
            copyBufferEdgeDataH(srcPtr, dstPtr, copyRowBytes, extraEdgeTop, tilePitchBytes, pFrame->paddingType, pFrame->paddingVal);
          }

          if (statusFlag & XV_TILE_STATUS_BOTTOM_EDGE_PADDING_NEEDED)
          {
            y2              = pTile1->y + (tileHeight - 1) + tileEdgeBottom;
            extraEdgeBottom = y2 - (frameHeight - 1 + framePadBottom);
            y2              = frameHeight - 1 + framePadBottom;
            dstPtr          = (uint8_t *) pTile1->pData + (y2 - pTile1->y + 1) * tilePitchBytes - tileEdgeLeft * pixWidth;
            srcPtr          = dstPtr - tilePitchBytes;
            copyRowBytes    = (tileEdgeLeft + tileWidth + tileEdgeRight) * XV_TYPE_ELEMENT_SIZE(pTile1->type);
            copyBufferEdgeDataH(srcPtr, dstPtr, copyRowBytes, extraEdgeBottom, tilePitchBytes, pFrame->paddingType, pFrame->paddingVal);
          }

          if (statusFlag & XV_TILE_STATUS_LEFT_EDGE_PADDING_NEEDED)
          {
            x1            = pTile1->x - tileEdgeLeft;
            extraEdgeLeft = -framePadLeft - x1;
            dstPtr        = (uint8_t *) pTile1->pData - (tileEdgeTop * tilePitchBytes + tileEdgeLeft * pixWidth);
            srcPtr        = dstPtr + extraEdgeLeft * pixWidth;
            copyHeight    = tileEdgeTop + tileHeight + tileEdgeBottom;
            copyBufferEdgeDataV(srcPtr, dstPtr, extraEdgeLeft, pixWidth, copyHeight, tilePitchBytes, pFrame->paddingType, pFrame->paddingVal);
          }

          if (statusFlag & XV_TILE_STATUS_RIGHT_EDGE_PADDING_NEEDED)
          {
            x2             = pTile1->x + (tileWidth - 1) + tileEdgeRight;
            extraEdgeRight = x2 - (frameWidth - 1 + framePadRight);
            x2             = frameWidth - 1 + framePadRight;
            dstPtr         = (uint8_t *) pTile1->pData - tileEdgeTop * tilePitchBytes + (x2 - pTile1->x + 1) * pixWidth;
            srcPtr         = dstPtr - 1 * pixWidth;
            copyHeight     = tileEdgeTop + tileHeight + tileEdgeBottom;
            copyBufferEdgeDataV(srcPtr, dstPtr, extraEdgeRight, pixWidth, copyHeight, tilePitchBytes, pFrame->paddingType, pFrame->paddingVal);
          }
        }

        statusFlag = statusFlag & ~XV_TILE_STATUS_EDGE_PADDING_NEEDED;

        if (pTile1->pPrevTile)
        {
          pTile1->pPrevTile->reuseCount--;
          pTile1->pPrevTile = NULL;
        }

        pTile1->status = statusFlag;
        doneCount++;
      }
      else    // DMA not done for this tile
      {
        break;
      }
    }
    else
    {
      // Tile is not part of frame. Make everything constant
      pFrame         = pTile1->pFrame;
      tileWidth      = pTile1->width;
      tileHeight     = pTile1->height;
      pixWidth       = pFrame->pixelRes * pFrame->numChannels;
      tilePitchBytes = pTile1->pitch * pFrame->pixelRes;
      tileEdgeLeft   = pTile1->tileEdgeLeft;
      tileEdgeRight  = pTile1->tileEdgeRight;
      tileEdgeTop    = pTile1->tileEdgeTop;
      tileEdgeBottom = pTile1->tileEdgeBottom;

      dstPtr       = (uint8_t *) pTile1->pData - (tileEdgeTop * tilePitchBytes + tileEdgeLeft * pixWidth);
      srcPtr       = NULL;
      copyRowBytes = (tileEdgeLeft + tileWidth + tileEdgeRight) * XV_TYPE_ELEMENT_SIZE(pTile1->type);
      copyHeight   = tileHeight + tileEdgeTop + tileEdgeBottom;
      if (pFrame->paddingType != FRAME_EDGE_PADDING)
      {
        copyBufferEdgeDataH(srcPtr, dstPtr, copyRowBytes, copyHeight, tilePitchBytes, pFrame->paddingType, pFrame->paddingVal);
      }
      pTile1->status = 0;
      doneCount++;
    }

    // Break if we reached the required tile
    if (pTile1 == pTile)
    {
      index = (index + 1) % MAX_NUM_DMA_QUEUE_LENGTH;
      break;
    }
  }
  pxvTM->tileDMAstartIndex   = index;
  pxvTM->tileDMApendingCount = pxvTM->tileDMApendingCount - doneCount;
  return(pTile->status == 0);
}

/**********************************************************************************
 * FUNCTION: xvPadEdges()
 *
 * DESCRIPTION:
 *     Pads edges of the given 8b tile. If FRAME_EDGE_PADDING mode is used,
 *     padding is done using edge values of the frame else if
 *     FRAME_CONSTANT_PADDING or FRAME_ZERO_PADDING mode is used,
 *     constant or zero value is padded
 *
 * INPUTS:
 *     xvTileManager *pxvTM                   Tile Manager object
 *     xvTile        *pTile                   Input tile
 *
 * OUTPUTS:
 *     Returns XVTM_ERROR if an error occurs
 *
 ********************************************************************************** */
// xvPadEdges should be used with Fast functions
int32_t xvPadEdges(xvTileManager *pxvTM, xvTile *pTile)
{
  int32_t x1, x2, y1, y2, indy, copyHeight, copyRowBytes, wb, padVal;
  int32_t tileWidth, tileHeight, tilePitchBytes, frameWidth, frameHeight;

  int16_t tileEdgeLeft, tileEdgeRight, tileEdgeTop, tileEdgeBottom;
  int16_t extraEdgeLeft, extraEdgeRight, extraEdgeTop, extraEdgeBottom;

  uint8_t * __restrict srcPtr, * __restrict dstPtr;
  xvFrame *pFrame;

  xb_vec2Nx8U dvec1, * __restrict pdvecDst;
  valign vas1;

  if (pTile == NULL)
  {
    pxvTM->errFlag = XV_ERROR_TILE_NULL;
    return(XVTM_ERROR);
  }

  if (pTile->status & XV_TILE_STATUS_EDGE_PADDING_NEEDED)
  {
    pFrame = pTile->pFrame;
    if (pFrame == NULL)
    {
      pxvTM->errFlag = XV_ERROR_FRAME_NULL;
      return(XVTM_ERROR);
    }

    tileEdgeTop    = pTile->tileEdgeTop;
    tileEdgeBottom = pTile->tileEdgeBottom;
    tileEdgeLeft   = pTile->tileEdgeLeft;
    tileEdgeRight  = pTile->tileEdgeRight;
    tilePitchBytes = pTile->pitch;
    tileHeight     = pTile->height;
    tileWidth      = pTile->width;
    frameWidth     = pFrame->frameWidth;
    frameHeight    = pFrame->frameHeight;

    if (pFrame->paddingType == FRAME_EDGE_PADDING)
    {
      if (pTile->status & XV_TILE_STATUS_LEFT_EDGE_PADDING_NEEDED)
      {
        extraEdgeLeft = tileEdgeLeft - pTile->x;
        dstPtr        = (uint8_t *) pTile->pData - (tileEdgeTop * tilePitchBytes + tileEdgeLeft);
        srcPtr        = dstPtr + extraEdgeLeft;
        copyHeight    = tileEdgeTop + tileHeight + tileEdgeBottom;

        vas1 = IVP_ZALIGN();
        for (indy = 0; indy < copyHeight; indy++)
        {
          dvec1    = *srcPtr;
          pdvecDst = (xb_vec2Nx8U *) dstPtr;
          IVP_SAV2NX8U_XP(dvec1, vas1, pdvecDst, extraEdgeLeft);
          IVP_SAPOS2NX8U_FP(vas1, pdvecDst);
          dstPtr += tilePitchBytes;
          srcPtr += tilePitchBytes;
        }
      }

      if (pTile->status & XV_TILE_STATUS_RIGHT_EDGE_PADDING_NEEDED)
      {
        x2             = pTile->x + tileWidth + tileEdgeRight;
        extraEdgeRight = x2 - frameWidth;
        dstPtr         = (uint8_t *) pTile->pData - tileEdgeTop * tilePitchBytes + (frameWidth - pTile->x);
        srcPtr         = dstPtr - 1;
        copyHeight     = tileEdgeTop + tileHeight + tileEdgeBottom;

        vas1 = IVP_ZALIGN();
        for (indy = 0; indy < copyHeight; indy++)
        {
          dvec1    = *srcPtr;
          pdvecDst = (xb_vec2Nx8U *) dstPtr;
          IVP_SAV2NX8U_XP(dvec1, vas1, pdvecDst, extraEdgeRight);
          IVP_SAPOS2NX8U_FP(vas1, pdvecDst);
          dstPtr += tilePitchBytes;
          srcPtr += tilePitchBytes;
        }
      }
    }
    else
    {
      padVal = 0;
      if (pFrame->paddingType == FRAME_CONSTANT_PADDING)
      {
        padVal = pFrame->paddingVal;
      }
      dvec1 = padVal;

      if (pTile->dmaIndex != XVTM_DUMMY_DMA_INDEX)
      {
        if (pTile->status & XV_TILE_STATUS_LEFT_EDGE_PADDING_NEEDED)
        {
          x1            = pTile->x - tileEdgeLeft;
          extraEdgeLeft = -x1;
          dstPtr        = (uint8_t *) pTile->pData - (tileEdgeTop * tilePitchBytes + tileEdgeLeft);
          copyHeight    = tileEdgeTop + tileHeight + tileEdgeBottom;

          pdvecDst = (xb_vec2Nx8U *) dstPtr;
          vas1     = IVP_ZALIGN();
          for (indy = 0; indy < copyHeight; indy++)
          {
            for (wb = extraEdgeLeft; wb > 0; wb -= (2 * IVP_SIMD_WIDTH))
            {
              IVP_SAV2NX8U_XP(dvec1, vas1, pdvecDst, wb);
            }
            IVP_SAPOS2NX8U_FP(vas1, pdvecDst);
            dstPtr  += tilePitchBytes;
            pdvecDst = (xb_vec2Nx8U *) dstPtr;
          }
        }

        if (pTile->status & XV_TILE_STATUS_RIGHT_EDGE_PADDING_NEEDED)
        {
          x2             = pTile->x + (tileWidth - 1) + tileEdgeRight;
          extraEdgeRight = x2 - (frameWidth - 1);
          x2             = frameWidth - 1;
          dstPtr         = (uint8_t *) pTile->pData - tileEdgeTop * tilePitchBytes + (x2 - pTile->x + 1);
          copyHeight     = tileEdgeTop + tileHeight + tileEdgeBottom;

          pdvecDst = (xb_vec2Nx8U *) dstPtr;
          vas1     = IVP_ZALIGN();
          for (indy = 0; indy < copyHeight; indy++)
          {
            for (wb = extraEdgeRight; wb > 0; wb -= (2 * IVP_SIMD_WIDTH))
            {
              IVP_SAV2NX8U_XP(dvec1, vas1, pdvecDst, wb);
            }
            IVP_SAPOS2NX8U_FP(vas1, pdvecDst);
            dstPtr  += tilePitchBytes;
            pdvecDst = (xb_vec2Nx8U *) dstPtr;
          }
        }

        if (pTile->status & XV_TILE_STATUS_TOP_EDGE_PADDING_NEEDED)
        {
          y1           = pTile->y - tileEdgeTop;
          extraEdgeTop = -y1;
          dstPtr       = (uint8_t *) pTile->pData - (tileEdgeTop * tilePitchBytes + tileEdgeLeft);
          copyRowBytes = (tileEdgeLeft + tileWidth + tileEdgeRight);

          pdvecDst = (xb_vec2Nx8U *) dstPtr;
          vas1     = IVP_ZALIGN();
          for (indy = 0; indy < extraEdgeTop; indy++)
          {
            for (wb = copyRowBytes; wb > 0; wb -= (2 * IVP_SIMD_WIDTH))
            {
              IVP_SAV2NX8U_XP(dvec1, vas1, pdvecDst, wb);
            }
            IVP_SAPOS2NX8U_FP(vas1, pdvecDst);
            dstPtr  += tilePitchBytes;
            pdvecDst = (xb_vec2Nx8U *) dstPtr;
          }
        }

        if (pTile->status & XV_TILE_STATUS_BOTTOM_EDGE_PADDING_NEEDED)
        {
          y2              = pTile->y + tileHeight + tileEdgeBottom;
          extraEdgeBottom = y2 - frameHeight;
          dstPtr          = (uint8_t *) pTile->pData + (frameHeight - pTile->y) * tilePitchBytes - tileEdgeLeft;
          copyRowBytes    = (tileEdgeLeft + tileWidth + tileEdgeRight);

          pdvecDst = (xb_vec2Nx8U *) dstPtr;
          vas1     = IVP_ZALIGN();
          for (indy = 0; indy < extraEdgeBottom; indy++)
          {
            for (wb = copyRowBytes; wb > 0; wb -= (2 * IVP_SIMD_WIDTH))
            {
              IVP_SAV2NX8U_XP(dvec1, vas1, pdvecDst, wb);
            }
            IVP_SAPOS2NX8U_FP(vas1, pdvecDst);
            dstPtr  += tilePitchBytes;
            pdvecDst = (xb_vec2Nx8U *) dstPtr;
          }
        }
      }
      else
      {
        // Tile is not part of frame. Make it constant
        dstPtr       = (uint8_t *) pTile->pData - (tileEdgeTop * tilePitchBytes + tileEdgeLeft);
        copyHeight   = tileHeight + tileEdgeTop + tileEdgeBottom;
        copyRowBytes = (tileEdgeLeft + tileWidth + tileEdgeRight);
        copyBufferEdgeDataH(NULL, dstPtr, copyRowBytes, copyHeight, tilePitchBytes, pFrame->paddingType, pFrame->paddingVal);
      }
    }

    pTile->status = pTile->status & ~(XV_TILE_STATUS_EDGE_PADDING_NEEDED);
  }
  return(XVTM_SUCCESS);
}

/**********************************************************************************
 * FUNCTION: xvPadEdges16()
 *
 * DESCRIPTION:
 *     Pads edges of the given 16b tile. If FRAME_EDGE_PADDING mode is used,
 *     padding is done using edge values of the frame else if
 *     FRAME_CONSTANT_PADDING or FRAME_ZERO_PADDING mode is used,
 *     constant or zero value is padded
 *
 * INPUTS:
 *     xvTileManager *pxvTM                   Tile Manager object
 *     xvTile        *pTile                   Input tile
 *
 * OUTPUTS:
 *     Returns XVTM_ERROR if an error occurs
 *
 ********************************************************************************** */
// xvPadEdges should be used with Fast functions
int32_t xvPadEdges16(xvTileManager *pxvTM, xvTile *pTile)
{
  int32_t x1, x2, y1, y2, indy, copyHeight, copyRowBytes, wb, padVal;
  int32_t tileWidth, tileHeight, frameWidth, frameHeight;

  int16_t tileEdgeLeft, tileEdgeRight, tileEdgeTop, tileEdgeBottom;
  int16_t extraEdgeLeft, extraEdgeRight, extraEdgeTop, extraEdgeBottom;

  uint16_t * __restrict srcPtr, * __restrict dstPtr;
  xvFrame *pFrame;

  xb_vecNx16 vec1, * __restrict pvecDst;
  valign vas1;

  if (pTile == NULL)
  {
    pxvTM->errFlag = XV_ERROR_TILE_NULL;
    return(XVTM_ERROR);
  }

  if (pTile->status & XV_TILE_STATUS_EDGE_PADDING_NEEDED)
  {
    pFrame = pTile->pFrame;
    if (pFrame == NULL)
    {
      pxvTM->errFlag = XV_ERROR_FRAME_NULL;
      return(XVTM_ERROR);
    }

    tileEdgeTop    = pTile->tileEdgeTop;
    tileEdgeBottom = pTile->tileEdgeBottom;
    tileEdgeLeft   = pTile->tileEdgeLeft;
    tileEdgeRight  = pTile->tileEdgeRight;
    tileHeight     = pTile->height;
    tileWidth      = pTile->width;
    frameWidth     = pFrame->frameWidth;
    frameHeight    = pFrame->frameHeight;

    if (pFrame->paddingType == FRAME_EDGE_PADDING)
    {
      if (pTile->status & XV_TILE_STATUS_LEFT_EDGE_PADDING_NEEDED)
      {
        extraEdgeLeft = tileEdgeLeft - pTile->x;
        dstPtr        = (uint16_t *) pTile->pData - (tileEdgeTop * pTile->pitch + tileEdgeLeft); // No need of multiplying by 2
        srcPtr        = dstPtr + extraEdgeLeft;                                                  // No need of multiplying by 2 as pointers are uint16_t *
        copyHeight    = tileEdgeTop + tileHeight + tileEdgeBottom;

        vas1 = IVP_ZALIGN();
        for (indy = 0; indy < copyHeight; indy++)
        {
          vec1    = *srcPtr;
          pvecDst = (xb_vecNx16 *) dstPtr;
          IVP_SAVNX16_XP(vec1, vas1, pvecDst, extraEdgeLeft * 2);
          IVP_SAPOSNX16_FP(vas1, pvecDst);
          dstPtr += pTile->pitch;
          srcPtr += pTile->pitch;
        }
      }

      if (pTile->status & XV_TILE_STATUS_RIGHT_EDGE_PADDING_NEEDED)
      {
        x2             = pTile->x + tileWidth + tileEdgeRight;
        extraEdgeRight = x2 - frameWidth;
        dstPtr         = (uint16_t *) pTile->pData - tileEdgeTop * pTile->pitch + (frameWidth - pTile->x);
        srcPtr         = dstPtr - 1;
        copyHeight     = tileEdgeTop + tileHeight + tileEdgeBottom;

        vas1 = IVP_ZALIGN();
        for (indy = 0; indy < copyHeight; indy++)
        {
          vec1    = *srcPtr;
          pvecDst = (xb_vecNx16 *) dstPtr;
          IVP_SAVNX16_XP(vec1, vas1, pvecDst, extraEdgeRight * 2);
          IVP_SAPOSNX16_FP(vas1, pvecDst);
          dstPtr += pTile->pitch;
          srcPtr += pTile->pitch;
        }
      }
    }
    else
    {
      padVal = 0;
      if (pFrame->paddingType == FRAME_CONSTANT_PADDING)
      {
        padVal = pFrame->paddingVal;
      }
      vec1 = padVal;

      if (pTile->dmaIndex != XVTM_DUMMY_DMA_INDEX)
      {
        if (pTile->status & XV_TILE_STATUS_LEFT_EDGE_PADDING_NEEDED)
        {
          x1            = pTile->x - tileEdgeLeft;
          extraEdgeLeft = -x1;
          dstPtr        = (uint16_t *) pTile->pData - (tileEdgeTop * pTile->pitch + tileEdgeLeft);
          copyHeight    = tileEdgeTop + tileHeight + tileEdgeBottom;

          pvecDst = (xb_vecNx16 *) dstPtr;
          vas1    = IVP_ZALIGN();
          for (indy = 0; indy < copyHeight; indy++)
          {
            for (wb = (extraEdgeLeft * 2); wb > 0; wb -= (2 * IVP_SIMD_WIDTH))
            {
              IVP_SAVNX16_XP(vec1, vas1, pvecDst, wb);
            }
            IVP_SAPOSNX16_FP(vas1, pvecDst);
            dstPtr += pTile->pitch;
            pvecDst = (xb_vecNx16 *) dstPtr;
          }
        }

        if (pTile->status & XV_TILE_STATUS_RIGHT_EDGE_PADDING_NEEDED)
        {
          x2             = pTile->x + tileWidth + tileEdgeRight;
          extraEdgeRight = x2 - frameWidth;
          dstPtr         = (uint16_t *) pTile->pData - tileEdgeTop * pTile->pitch + (frameWidth - pTile->x);
          copyHeight     = tileEdgeTop + tileHeight + tileEdgeBottom;

          pvecDst = (xb_vecNx16 *) dstPtr;
          vas1    = IVP_ZALIGN();
          for (indy = 0; indy < copyHeight; indy++)
          {
            for (wb = (extraEdgeRight * 2); wb > 0; wb -= (2 * IVP_SIMD_WIDTH))
            {
              IVP_SAVNX16_XP(vec1, vas1, pvecDst, wb);
            }
            IVP_SAPOSNX16_FP(vas1, pvecDst);
            dstPtr += pTile->pitch;
            pvecDst = (xb_vecNx16 *) dstPtr;
          }
        }

        if (pTile->status & XV_TILE_STATUS_TOP_EDGE_PADDING_NEEDED)
        {
          y1           = pTile->y - tileEdgeTop;
          extraEdgeTop = -y1;
          dstPtr       = (uint16_t *) pTile->pData - (tileEdgeTop * pTile->pitch + tileEdgeLeft);
          copyRowBytes = (tileEdgeLeft + tileWidth + tileEdgeRight) * 2;

          pvecDst = (xb_vecNx16 *) dstPtr;
          vas1    = IVP_ZALIGN();
          for (indy = 0; indy < extraEdgeTop; indy++)
          {
            for (wb = copyRowBytes; wb > 0; wb -= (2 * IVP_SIMD_WIDTH))
            {
              IVP_SAVNX16_XP(vec1, vas1, pvecDst, wb);
            }
            IVP_SAPOSNX16_FP(vas1, pvecDst);
            dstPtr += pTile->pitch;
            pvecDst = (xb_vecNx16 *) dstPtr;
          }
        }

        if (pTile->status & XV_TILE_STATUS_BOTTOM_EDGE_PADDING_NEEDED)
        {
          y2              = pTile->y + tileHeight + tileEdgeBottom;
          extraEdgeBottom = y2 - frameHeight;
          dstPtr          = (uint16_t *) pTile->pData + (frameHeight - pTile->y) * pTile->pitch - tileEdgeLeft;
          copyRowBytes    = (tileEdgeLeft + tileWidth + tileEdgeRight) * 2;

          pvecDst = (xb_vecNx16 *) dstPtr;
          vas1    = IVP_ZALIGN();
          for (indy = 0; indy < extraEdgeBottom; indy++)
          {
            for (wb = copyRowBytes; wb > 0; wb -= (2 * IVP_SIMD_WIDTH))
            {
              IVP_SAVNX16_XP(vec1, vas1, pvecDst, wb);
            }
            IVP_SAPOSNX16_FP(vas1, pvecDst);
            dstPtr += pTile->pitch;
            pvecDst = (xb_vecNx16 *) dstPtr;
          }
        }
      }
      else
      {
        // Tile is not part of frame. Make it constant
        dstPtr       = (uint16_t *) pTile->pData - (tileEdgeTop * pTile->pitch + tileEdgeLeft);
        copyHeight   = tileHeight + tileEdgeTop + tileEdgeBottom;
        copyRowBytes = (tileEdgeLeft + tileWidth + tileEdgeRight) * 2;
        copyBufferEdgeDataH(NULL, (uint8_t *) dstPtr, copyRowBytes, copyHeight, (pTile->pitch * 2), pFrame->paddingType, pFrame->paddingVal);
      }
    }

    pTile->status = pTile->status & ~(XV_TILE_STATUS_EDGE_PADDING_NEEDED);
  }
  return(XVTM_SUCCESS);
}

/**********************************************************************************
 * FUNCTION: xvCheckInputTileFree()
 *
 * DESCRIPTION:
 *     A tile is said to be free if all data transfers pertaining to data resue
 *     from this tile is completed
 *
 * INPUTS:
 *     xvTileManager *pxvTM                   Tile Manager object
 *     xvTile        *pTile                   Input tile
 *
 * OUTPUTS:
 *     Returns ONE if input tile is free and ZERO if it is not
 *     Returns XVTM_ERROR if an error occurs
 *
 ********************************************************************************** */

int32_t xvCheckInputTileFree(xvTileManager *pxvTM, xvTile *pTile)
{
  if (pxvTM == NULL)
  {
    return(XVTM_ERROR);
  }
  pxvTM->errFlag = XV_ERROR_SUCCESS;

  if (pTile == NULL)
  {
    pxvTM->errFlag = XV_ERROR_TILE_NULL;
    return(XVTM_ERROR);
  }
  if (pTile->reuseCount == 0)
  {
    return(1);
  }
  else
  {
    return(0);
  }
}

/**********************************************************************************
 * FUNCTION: xvGetErrorInfo()
 *
 * DESCRIPTION:
 *
 *     Prints the most recent error information.
 *
 * INPUTS:
 *     xvTileManager *pxvTM                   Tile Manager object
 *
 * OUTPUTS:
 *     It returns the most recent error code.
 *
 ********************************************************************************** */

xvError_t xvGetErrorInfo(xvTileManager *pxvTM)
{
  if (pxvTM == NULL)
  {
    TM_PRINT("Tile Manager pointer is NULL");
    return(XV_ERROR_TILE_MANAGER_NULL);
  }
  switch (pxvTM->errFlag)
  {
  case XV_ERROR_SUCCESS:
    TM_PRINT("No Errors detected\n");
    break;
  case XV_ERROR_POINTER_NULL:
    TM_PRINT("Pointer is NULL\n");
    break;
  case XV_ERROR_FRAME_NULL:
    TM_PRINT("Frame is NULL\n");
    break;
  case XV_ERROR_TILE_NULL:
    TM_PRINT("Tile is NULL\n");
    break;
  case XV_ERROR_BUFFER_NULL:
    TM_PRINT("Buffer is NULL\n");
    break;
  case XV_ERROR_ALLOC_FAILED:
    TM_PRINT("Buffer allocation failed\n");
    break;
  case XV_ERROR_FRAME_BUFFER_FULL:
    TM_PRINT("Frame buffer full. Try releasing unused frames\n");
    break;
  case XV_ERROR_TILE_BUFFER_FULL:
    TM_PRINT("Tile buffer full. Try releasing unused tiles\n");
    break;
  case XV_ERROR_BAD_ARG:
    TM_PRINT("Inconsistent arguments. Give correct arguments\n");
    break;
  case XV_ERROR_DMA_INIT:
	TM_PRINT("XV_ERROR_DMA_INIT. Give correct arguments\n");
	break;
  default:
    TM_PRINT("Incorrect error flag\n");
    break;
  }
  return(pxvTM->errFlag);
}

#ifdef XV_EMULATE_DMA
idma_error_details_t* dma_buffer_error_details()
{
  TM_PRINT("Error: In dma_buffer_error_details function !!!\n");
  return((idma_error_details_t *) NULL);
}
#endif
