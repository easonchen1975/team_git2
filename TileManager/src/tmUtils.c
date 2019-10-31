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

#include <stdint.h>
#include <xtensa/tie/xt_ivpn.h>
#include "tileManager.h"

#ifndef XV_EMULATE_DMA

/****************************************************************************************
*
*	XVMEM allocation utility
*
*****************************************************************************************/

xvmem_status_t xvmem_init(xvmem_mgr_t *mgr, void *buf, int32_t size, uint32_t num_blocks, void *header)
{

  if (buf == 0)
  {
    return(XVMEM_ERROR_POOL_NULL);
  }

  mgr->_buffer      = buf;
  mgr->_buffer_size = size;

  uint32_t num_words_in_block_bitvec;
  /* If there is an externally provided header, the blocks and the bitvector
   * are allocated from the header, else they are from the user provided pool */
  if (header)
  {
    mgr->_num_blocks          = num_blocks;
    num_words_in_block_bitvec = (mgr->_num_blocks + 31) >> 5;
    mgr->_blocks              = (xvmem_block_t *) header;
    mgr->_has_header          = 1;
  }
  else
  {
    mgr->_num_blocks          = XVMEM_HEAP3_DEFAULT_NUM_BLOCKS;
    num_words_in_block_bitvec = (mgr->_num_blocks + 31) >> 5;
    mgr->_blocks              = (xvmem_block_t *) buf;
    mgr->_has_header          = 0;
  }
  mgr->_block_free_bitvec =
    (uint32_t *) ((uintptr_t) mgr->_blocks +
                  sizeof(xvmem_block_t) * mgr->_num_blocks);
  mgr->_header_size = sizeof(xvmem_block_t) * mgr->_num_blocks +
                      num_words_in_block_bitvec * 4;

  /* Check if there is the minimum required buffer size and number of blocks */
  if (!header)
  {
    if (size < (mgr->_header_size + XVMEM_HEAP3_MIN_ALLOC_SIZE))
    {
      return(XVMEM_ERROR_POOL_SIZE);
    }
  }
  else
  {
  }

  /* Initialize free and allocated list */
  mgr->_free_list_head._block_size     = 0;
  mgr->_free_list_head._buffer         = 0;
  mgr->_free_list_head._aligned_buffer = 0;
  mgr->_free_list_tail._block_size     = 0xffffffff;
  mgr->_free_list_tail._buffer         = (void *) 0xffffffff;
  mgr->_free_list_tail._aligned_buffer = (void *) 0xffffffff;

  mgr->_alloc_list_head._block_size     = 0;
  mgr->_alloc_list_head._buffer         = 0;
  mgr->_alloc_list_head._aligned_buffer = 0;

  mgr->_allocated_bytes = 0;

  if (header)
  {
    /* Initialize the first block with the start address of the buffer and
     * size as the whole buffer and add to the free list */
    mgr->_blocks[0]._block_size     = size;
    mgr->_blocks[0]._buffer         = buf;
    mgr->_blocks[0]._aligned_buffer = buf;

    mgr->_free_list_head._next_block = &mgr->_blocks[0];
    mgr->_free_list_tail._next_block = 0;
    mgr->_blocks[0]._next_block      = &mgr->_free_list_tail;

    mgr->_alloc_list_head._next_block = 0;

    /* Set the first block as in use (set to 0) and rest as
     * available (set to 1) */
    mgr->_block_free_bitvec[0] = 0xfffffffe;

    mgr->_free_bytes   = size;
    mgr->_unused_bytes = 0;
  }
  else
  {
    /* The header (array of xvmem_block_t and the bitvector) is placed at the
     * start of buf. Data allocation begins after that. Initialize the first
     * block with the header address (start of buf) and header size. */
    mgr->_blocks[0]._block_size     = mgr->_header_size;
    mgr->_blocks[0]._buffer         = buf;
    mgr->_blocks[0]._aligned_buffer = buf;

    mgr->_alloc_list_head._next_block = &mgr->_blocks[0];
    mgr->_blocks[0]._next_block       = 0;

    /* Initialize the second block with the address of the area in buffer
     * following the xvmem_block_t array and bitvector that is available for
     * user allocation. Size is set as the buffer size minus the space required
     * for the block array and bitvector */
    mgr->_blocks[1]._block_size = size - mgr->_blocks[0]._block_size;
    mgr->_blocks[1]._buffer     = (void *) ((uintptr_t) mgr->_blocks[0]._buffer +
                                            mgr->_blocks[0]._block_size);
    mgr->_blocks[1]._aligned_buffer = mgr->_blocks[1]._buffer;

    mgr->_free_list_head._next_block = &mgr->_blocks[1];
    mgr->_free_list_tail._next_block = 0;
    mgr->_blocks[1]._next_block      = &mgr->_free_list_tail;

    /* Set the first two blocks as in use (set to 0) and rest as
     * available (set to 1) */
    mgr->_block_free_bitvec[0] = 0xfffffffc;

    mgr->_free_bytes   = mgr->_blocks[1]._block_size;
    mgr->_unused_bytes = mgr->_blocks[0]._block_size;
    mgr->_allocated_bytes = mgr->_unused_bytes;
  }

  mgr->_initialized = XVMEM_INITIALIZED;

  return(XVMEM_OK);
}

static int32_t xvmem_find_msbit(uint32_t n)
{
#if XCHAL_HAVE_NSA
  return(31 - XT_NSAU(n));
#else
  uint32_t nbits = 0;
  while (n && (n & 1) == 0)
  {
    n >>= 1;
    nbits++;
  }
  return(nbits);
#endif
}

static uint32_t xvmem_compute_new_size(xvmem_block_t *block, size_t size, uint32_t align)
{
  /* Align the block buffer address to the specified alignment */
  uint32_t p              = xvmem_find_msbit(align);
  uintptr_t aligned_block = (uintptr_t) block->_buffer;
  aligned_block = ((aligned_block + align - 1) >> p) << p;
  /* compute extra space required for alignment */
  uint32_t extra_size = aligned_block - (uintptr_t) block->_buffer;
  uint32_t new_size   = size + extra_size;
  return(new_size);
}

static uint32_t leading_zero_count(const uint32_t *bitvec, uint32_t num_bits, uint32_t pos,
                                 uint32_t w, uint32_t zero_count)
{
  /* Find the word corresponding to bit position pos */
  uint32_t word_idx = pos >> 5;
  /* Find bit offset within this word */
  uint32_t word_off = pos & 31;
  /* Not all of the word will be in use. If num_bits is < 32, then only
   * num_bits are used else all 32-bits are used */
  uint32_t num_bits_used_in_word = 32;
  XT_MOVLTZ(num_bits_used_in_word, num_bits, (int32_t) (num_bits - 32));
  /* Find remaining bits in word starting from off to end of word */
  uint32_t num_rem_bits_in_word = num_bits_used_in_word - word_off;
  uint32_t num_words_in_bitvec  = (num_bits + 31) >> 5;
  w ^= zero_count;
  uint32_t ws = w >> word_off;
  /* Find position of the least signficant 1b */
  uint32_t ls1b     = ws & - ws;
  uint32_t ls1b_pos = xvmem_find_msbit(ls1b);
  /* If there are no 1s in the word then the leading count is the remaining
   * bits in the word */
  uint32_t lzc = ls1b_pos;
  XT_MOVEQZ(lzc, num_rem_bits_in_word, ls1b);
  uint32_t num_rem_words = num_words_in_bitvec - (word_idx + 1);
  /* While there is no 1 in the word and there are words remaining, continue */
  while (ls1b == 0 && num_rem_words != 0)
  {
    num_bits -= 32;
    word_idx++;
    num_rem_words--;
    w                     = bitvec[word_idx];
    w                    ^= zero_count;
    ls1b                  = w & - w;
    ls1b_pos              = xvmem_find_msbit(ls1b);
    num_bits_used_in_word = 32;
    XT_MOVLTZ(num_bits_used_in_word, num_bits, (int32_t) (num_bits - 32));
    uint32_t l = ls1b_pos;
    XT_MOVEQZ(l, num_bits_used_in_word, ls1b);
    lzc += l;
  }
  return(lzc);
}

static void xvmem_toggle_bitvec(uint32_t *bitvec, uint32_t num_bits_in_bitvec, uint32_t pos, uint32_t num_bits)
{
  /* Find the word corresponding to bit position pos */
  uint32_t word_idx = pos >> 5;
  /* Find bit offset within this word */
  uint32_t word_off = pos & 31;
  /* Find remaining bits in word starting from off to end of word */
  uint32_t rem_bits_in_word = 32 - word_off;
  uint32_t nbits            = rem_bits_in_word;
  /* If num_bits is greater than the remaining number of bits to end of word
   * toggle the remaining number of bits in word, else only toggle num_bits */
  XT_MOVLTZ(nbits, num_bits, num_bits - rem_bits_in_word);
  /* Form bit mask of the number of bits to toggle */
  uint32_t n = 1 << nbits;
  XT_MOVEQZ(n, 0, nbits - 32);
  /* Align the mask to the word offset */
  uint32_t bmask = (n - 1) << word_off;
  /* Negate the bits in the word */
  *(bitvec + word_idx) ^= bmask;
  num_bits             -= nbits;
  word_idx++;
  /* If there are more bits remaining, continue with the next word */
  while (num_bits > 0)
  {
    nbits = 32;
    /* Form bit mask of either 32 1s or num_bits 1s if num_bits < 32 */
    XT_MOVLTZ(nbits, num_bits, num_bits - 32);
    n = 1 << nbits;
    XT_MOVEQZ(n, 0, nbits - 32);
    bmask = n - 1;
    /* Negate the bits in the word */
    *(bitvec + word_idx) ^= bmask;
    num_bits             -= nbits;
    word_idx++;
  }
}

void *
xvmem_alloc(xvmem_mgr_t *mgr, size_t size, uint32_t align, xvmem_status_t *err_code)
{
  void *r;

  if (!align || (align & (align - 1)))
  {
    *err_code = XVMEM_ERROR_ILLEGAL_ALIGN;
    return(0);
  }

  xvmem_block_t *prev_block = &mgr->_free_list_head;
  xvmem_block_t *curr_block = mgr->_free_list_head._next_block;


  /* Compute required size based on alignment */
  uint32_t new_size = xvmem_compute_new_size(curr_block, size, align);

  /* Check for the first available block in the free list that satisfies
   * the required size */
  while (curr_block->_block_size < new_size)
  {
    prev_block = curr_block;
    curr_block = curr_block->_next_block;
    new_size = xvmem_compute_new_size(curr_block, size, align);
  }

  if (curr_block != &mgr->_free_list_tail)
  {
    curr_block->_aligned_buffer = (void *) ((uintptr_t) curr_block->_buffer +
                                            new_size - size);
    /* The aligned buffer address gets returned to the user */
    r                   = curr_block->_aligned_buffer;
    mgr->_unused_bytes += (new_size - size);

    /* If the remaining space in the block is < than the minimum alloc size,
     * just allocate the whole block to this request, else split the block
     * and add the remaining to the free list */
    if ((curr_block->_block_size - new_size) > XVMEM_HEAP3_MIN_ALLOC_SIZE)
    {
      /* Find a free block to hold the remaining of the current block */
      uint32_t avail_buf_idx = leading_zero_count(mgr->_block_free_bitvec, mgr->_num_blocks,
                                         0, mgr->_block_free_bitvec[0], 0);

      /* Run out of block headers.
       * TODO: Do reallocate and expand the block header */
      if (avail_buf_idx >= mgr->_num_blocks)
      {
        *err_code = XVMEM_ERROR_ALLOC_FAILED;
        return(0);
      }
      /* Mark the new created block as allocated */
      xvmem_toggle_bitvec(mgr->_block_free_bitvec, mgr->_num_blocks, avail_buf_idx, 1);
      xvmem_block_t *new_block = &mgr->_blocks[avail_buf_idx];
      new_block->_block_size     = curr_block->_block_size - new_size;
      new_block->_buffer         = (void *) ((uintptr_t) curr_block->_buffer + new_size);
      new_block->_aligned_buffer = new_block->_buffer;
      curr_block->_block_size    = new_size;
      /* Add to free list */
      new_block->_next_block  = curr_block->_next_block;
      prev_block->_next_block = new_block;
    }
    else
    {
      new_size                = curr_block->_block_size;
      prev_block->_next_block = curr_block->_next_block;
    }
    /* Add the allocated block to beginning of the allocated list */
    curr_block->_next_block           = mgr->_alloc_list_head._next_block;
    mgr->_alloc_list_head._next_block = curr_block;
    mgr->_free_bytes                 -= curr_block->_block_size;
    mgr->_allocated_bytes            += curr_block->_block_size;
    *err_code = XVMEM_OK;
  }
  else
  {
    *err_code = XVMEM_ERROR_ALLOC_FAILED;
    r         = 0;
  }

  return(r);
}

static void xvmem_add_block_to_free_list(xvmem_block_t *new_block, xvmem_mgr_t *mgr)
{
  xvmem_block_t *block;
  /* Find block whose next free block's buffer address is >= buffer address of
   * the new block to be added */
  for (block = &mgr->_free_list_head;
       (uintptr_t) block->_next_block->_buffer < (uintptr_t) new_block->_buffer;
       block = block->_next_block)
  {
    ;
  }
  int merge_with_prev = 0;
  int merge_with_next = 0;
  /* Check if the previous block can be merged with the new block */
  if (block != &mgr->_free_list_head)
  {
    if ((uintptr_t) block->_buffer + block->_block_size ==
        (uintptr_t) new_block->_buffer)
    {
      block->_block_size += new_block->_block_size;
      uint32_t new_block_bv_idx =
        ((uintptr_t) new_block - (uintptr_t) &mgr->_blocks[0]) >>
        XVMEM_HEAP3_BLOCK_STRUCT_SIZE_LOG2;
      xvmem_toggle_bitvec(mgr->_block_free_bitvec, mgr->_num_blocks,
                         new_block_bv_idx, 1);
      new_block       = block;
      merge_with_prev = 1;
    }
  }

  /* Check if new_block (or new_block that is merged with previous from above)
   * can be merged with the next */
  if (block->_next_block != &mgr->_free_list_tail)
  {
    if ((uintptr_t) new_block->_buffer + new_block->_block_size ==
        (uintptr_t) block->_next_block->_buffer)
    {
      uint32_t next_block_bv_idx =
        ((uintptr_t) block->_next_block - (uintptr_t) &mgr->_blocks[0]) >>
        XVMEM_HEAP3_BLOCK_STRUCT_SIZE_LOG2;
      xvmem_toggle_bitvec(mgr->_block_free_bitvec, mgr->_num_blocks, next_block_bv_idx, 1);
      new_block->_block_size += block->_next_block->_block_size;
      new_block->_next_block  = block->_next_block->_next_block;
      if (merge_with_prev == 0)
      {
        block->_next_block = new_block;
      }
      merge_with_next = 1;
    }
  }

  if (merge_with_prev == 0 && merge_with_next == 0)
  {
    new_block->_next_block = block->_next_block;
    block->_next_block     = new_block;
  }
}

void  xvmem_free(xvmem_mgr_t *mgr, void *p)
{
	  if (p == NULL)
	  {
		return;
	  }
	
	  xvmem_block_t *block;
	  xvmem_block_t *prev_block;
	
	  /* Check the allocated list to find the matching block that holds p */
	  for (block = mgr->_alloc_list_head._next_block,
		   prev_block = &mgr->_alloc_list_head;
		   block != 0;
		   prev_block = block, block = block->_next_block)
	  {
		if (block->_aligned_buffer == p)
		{
		  break;
		}
	  }
	
	  if (block == 0)
	  {
		return;
	  }
	
	  /* Remove from allocated list and add to free list */
	  prev_block->_next_block = block->_next_block;
	  mgr->_free_bytes		 += block->_block_size;
	  mgr->_allocated_bytes  -= block->_block_size;
	  mgr->_unused_bytes	 -= ((uintptr_t) block->_buffer - (uintptr_t) block->_aligned_buffer);
	  xvmem_add_block_to_free_list(block, mgr);
	
}
#endif
/****************************************************************************************
*
*	Tile Manager internal functions
*
*****************************************************************************************/

// Used for padding horizontal rows. Supports both zero padding and edge padding.
void copyBufferEdgeDataH(uint8_t * __restrict srcPtr, uint8_t * __restrict dstPtr, int32_t widthBytes, int32_t height, int32_t pitchBytes, uint8_t paddingType, uint8_t paddingVal)
{
  int32_t indy, wb;
  xb_vec2Nx8U dvec1, * __restrict pdvecSrc, * __restrict pdvecDst;
  valign vas1, val1;
  pdvecSrc = (xb_vec2Nx8U *) srcPtr;
  pdvecDst = (xb_vec2Nx8U *) dstPtr;

  if (paddingType == FRAME_EDGE_PADDING)
  {
    for (indy = 0; indy < height; indy++)
    {
      val1 = IVP_LA2NX8U_PP(pdvecSrc);
      vas1 = IVP_ZALIGN();
      for (wb = widthBytes; wb > 0; wb -= (2 * IVP_SIMD_WIDTH))
      {
        IVP_LAV2NX8U_XP(dvec1, val1, pdvecSrc, wb);
        IVP_SAV2NX8U_XP(dvec1, vas1, pdvecDst, wb);
      }
      IVP_SAPOS2NX8U_FP(vas1, pdvecDst);
      dstPtr  += pitchBytes;
      pdvecSrc = (xb_vec2Nx8U *) srcPtr;
      pdvecDst = (xb_vec2Nx8U *) dstPtr;
    }
  }
  else
  {
    dvec1 = 0;
    if (paddingType == FRAME_CONSTANT_PADDING)
    {
      dvec1 = paddingVal;
    }
    vas1 = IVP_ZALIGN();
    for (indy = 0; indy < height; indy++)
    {
      for (wb = widthBytes; wb > 0; wb -= (2 * IVP_SIMD_WIDTH))
      {
        IVP_SAV2NX8U_XP(dvec1, vas1, pdvecDst, wb);
      }
      IVP_SAPOS2NX8U_FP(vas1, pdvecDst);
      dstPtr  += pitchBytes;
      pdvecDst = (xb_vec2Nx8U *) dstPtr;
    }
  }
}

// Can be used for padding vertical columns. Supports both zero padding and edge padding .
void  copyBufferEdgeDataV(uint8_t * __restrict srcPtr, uint8_t * __restrict dstPtr, int32_t width, int32_t pixWidth, int32_t height, int32_t pitchBytes, uint8_t paddingType, uint8_t paddingVal)
{
  int32_t indy, wb, widthBytes;
  xb_vec2Nx8U dvec1, * __restrict pdvecDst;
  xb_vecNx16U vec1, * __restrict pvecDst;
  valign vas1;

  widthBytes = width * pixWidth;

  if (paddingType == FRAME_EDGE_PADDING)
  {
    for (indy = 0; indy < height; indy++)
    {
      if (pixWidth == 1)
      {
        dvec1    = *srcPtr;
        vas1     = IVP_ZALIGN();
        pdvecDst = (xb_vec2Nx8U *) dstPtr;
        for (wb = widthBytes; wb > 0; wb -= (2 * IVP_SIMD_WIDTH))
        {
          IVP_SAV2NX8U_XP(dvec1, vas1, pdvecDst, wb);
        }
        IVP_SAPOS2NX8U_FP(vas1, pdvecDst);
        dstPtr += pitchBytes;
        srcPtr += pitchBytes;
      }
      else
      {
        if (pixWidth == 2)
        {
          vec1    = *((uint16_t *) srcPtr);
          vas1    = IVP_ZALIGN();
          pvecDst = (xb_vecNx16U *) dstPtr;
          for (wb = widthBytes; wb > 0; wb -= (2 * IVP_SIMD_WIDTH))
          {
            IVP_SAVNX16U_XP(vec1, vas1, pvecDst, wb);
          }
          IVP_SAPOSNX16U_FP(vas1, pvecDst);
          dstPtr += pitchBytes;
          srcPtr += pitchBytes;
        }
        else
        {
          // TODO: Vectorize it
          for (wb = 0; wb < widthBytes; wb += pixWidth)
          {
            int32_t index;
            for (index = 0; index < pixWidth; index++)
            {
              dstPtr[wb + index] = srcPtr[index];
            }
          }
          dstPtr += pitchBytes;
          srcPtr += pitchBytes;
        }
      }
    }
  }
  else
  {
    dvec1 = 0;

    if (paddingType == FRAME_CONSTANT_PADDING)
    {
      dvec1 = paddingVal;
    }
    pdvecDst = (xb_vec2Nx8U *) dstPtr;
    vas1     = IVP_ZALIGN();
    for (indy = 0; indy < height; indy++)
    {
      for (wb = widthBytes; wb > 0; wb -= (2 * IVP_SIMD_WIDTH))
      {
        IVP_SAV2NX8U_XP(dvec1, vas1, pdvecDst, wb);
      }
      IVP_SAPOS2NX8U_FP(vas1, pdvecDst);
      dstPtr  += pitchBytes;
      pdvecDst = (xb_vec2Nx8U *) dstPtr;
    }
  }
}

#ifdef XV_EMULATE_DMA
int32_t dma_desc_done(int32_t index)
{
  return(1);
}

// Emulates 2D dma copy. Used for debugging
// Assumes 8 bit planar data
int32_t copy2d(void *pDst, void *pSrc, size_t width, int32_t flags, int32_t height, int32_t srcPitchBytes, int32_t dstPitchBytes)
{
  int32_t indx, indy;
  for (indy = 0; indy < height; indy++)
  {
    for (indx = 0; indx < (int32_t) width; indx++)
    {
      ((uint8_t *) pDst)[indy * dstPitchBytes + indx] = ((uint8_t *) pSrc)[indy * srcPitchBytes + indx];
    }
  }
  return(0);
}

int dma_sleep()
{
	return 1;
}

#endif //XV_EMULATE_DMA

