/*
 * Customer ID=14510; Build=0x82e2e; Copyright (c) 2014 by Cadence Design Systems. ALL RIGHTS RESERVED.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
/* EXAMPLE:
   Schedule various tasks and wait for their completion.  
   Wait for task completion using polling wait.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xtensa/idma.h>
#include "idma_tests.h"

#define IMAGE_CHUNK_SIZE	100
#define IDMA_XFER_SIZE		IDMA_SIZE(IMAGE_CHUNK_SIZE)
#define IDMA_IMAGE_SIZE		IMAGE_CHUNK_SIZE*200


IDMA_BUFFER_DEFINE(task_test1npu, 1, IDMA_1D_DESC);
IDMA_BUFFER_DEFINE(task, 10, IDMA_1D_DESC);
#define NPU_SIZE 512

ALIGNDCACHE char npu_base[NPU_SIZE] __attribute__ ((section(".sram.npu")));

ALIGNDCACHE char dst1[IDMA_XFER_SIZE] ; //__attribute__ ((section(".sram.npu")));
ALIGNDCACHE char src1[IDMA_XFER_SIZE] IDMA_DRAM;
ALIGNDCACHE char dst2[IDMA_XFER_SIZE];
ALIGNDCACHE char src2[IDMA_XFER_SIZE] IDMA_DRAM;
ALIGNDCACHE char dst3[IDMA_XFER_SIZE];
ALIGNDCACHE char src3[IDMA_XFER_SIZE] IDMA_DRAM;

void compareBuffers_task(idma_buffer_t* task, char* src, char* dst, int size)
{
  int status = idma_task_status(task);
  printf("Task %p status: %s\n", task,
                    (status == IDMA_TASK_DONE)    ? "DONE"       :
                    (status > IDMA_TASK_DONE)     ? "PENDING"    :
                    (status == IDMA_TASK_ABORTED) ? "ABORTED"    :
                    (status == IDMA_TASK_ERROR)   ? "ERROR"      :  "UNKNOWN"   );

  if(status == IDMA_TASK_ERROR) {
    idma_error_details_t* error = idma_error_details();
    printf("COPY FAILED, Error 0x%x at desc:%p, PIF src/dst=%x/%x\n", error->err_type, (void*)error->currDesc, error->srcAddr, error->dstAddr);
    return;
  }

  if(src && dst) {
    xthal_dcache_region_invalidate(dst, size);
    compareBuffers(src, dst, size);
  }
}


void seq_fill(unsigned char *dst, int size, unsigned char a)
{
   int i;

   for(i=0; i< size; i++){

	   *(dst+i) = a;
   }

}

#define NPU_TEST1_SYSRAM 0
#define NPU_TEST2_SYSRAM 0x4000000
#define NPU_TEST3_SYSRAM 0x8000008
#define NPU_TEST4_SYSRAM 0xC000000
#define XSIZE 512
#define DESC_NOTIFY_W_INT         0x80000000  /* trigger interrupt on completion */

/* THIS IS THE EXAMPLE, main() CALLS IT */
int
test(void* arg, int unused)
{
//   bufRandomize(src1, IDMA_XFER_SIZE);
//   bufRandomize(src2, IDMA_XFER_SIZE);
//   bufRandomize(src3, IDMA_XFER_SIZE);

   unsigned char *npu_test1_base = npu_base + NPU_TEST1_SYSRAM;
   unsigned char *npu_test1_base_results = npu_test1_base + 0x80000;
   unsigned char *npu_test2_base = npu_base + NPU_TEST2_SYSRAM;
   unsigned char *npu_test3_base = npu_base + NPU_TEST3_SYSRAM;
   unsigned char *npu_test4_base = npu_base + NPU_TEST4_SYSRAM;

   *(uint8_t *)(0x7000fff0) = 0x11;
   *(uint8_t *)(0x7000fff4) = 0x11;
   seq_fill(npu_test1_base, XSIZE, 0xFF );
   seq_fill(npu_test1_base_results, XSIZE, 0x11 );
//   seq_fill(npu_test2_base, 128, 0x5A );
//   seq_fill(npu_test3_base, 128, 0xA5 );
//   seq_fill(npu_test4_base, 128, 0xAA );


   idma_log_handler(idmaLogHander);
   idma_init(0, MAX_BLOCK_2, 16, TICK_CYCLES_8, 100000, NULL);

   idma_init_task(task_test1npu, IDMA_1D_DESC, 1, NULL, NULL);
   idma_add_desc(task_test1npu, src1, npu_test1_base, XSIZE, DESC_NOTIFY_W_INT );
   idma_schedule_task(task_test1npu);

   while (idma_task_status(task_test1npu) > 0)
      idma_process_tasks();

   //compareBuffers_task(task_test1npu, src1, npu_test1_base, XSIZE );
   //compareBuffers_task(task_test1npu, npu_test1_base, npu_test1_base_results, XSIZE );


   //  move internal to 0x71000000


   idma_init_task(task_test1npu, IDMA_1D_DESC, 1, NULL, NULL);
   idma_add_desc(task_test1npu, npu_test1_base_results, src1, XSIZE, DESC_NOTIFY_W_INT );
   idma_schedule_task(task_test1npu);

   while (idma_task_status(task_test1npu) > 0)
      idma_process_tasks();

   //compareBuffers_task(task_test1npu, src1, npu_test1_base_results, XSIZE );
   //compareBuffers_task(task_test1npu, npu_test1_base, npu_test1_base_results, XSIZE );
   *(uint8_t *)(0x7000fff0) = 0x12;


   /*
   idma_init_task(task, IDMA_1D_DESC, 10, NULL, NULL);

   DPRINT("Setting IDMA request %p (4 descriptors)\n", task);
   idma_add_desc(task, dst1, src1, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task, dst2, src2, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task, dst3, src3, IDMA_XFER_SIZE, 0 );

   idma_add_desc(task, dst1, src1, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task, dst2, src2, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task, dst3, src3, IDMA_XFER_SIZE, 0 );

   idma_add_desc(task, dst1, src1, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task, dst2, src2, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task, dst3, src3, IDMA_XFER_SIZE, 0 );

   idma_add_desc(task, dst1, src1, IDMA_XFER_SIZE, 0 );

   DPRINT("Schedule IDMA request\n");
   idma_schedule_task(task);

   while (idma_task_status(task) > 0)
      idma_process_tasks();

   compareBuffers_task(task, src1, dst1, IDMA_XFER_SIZE );
   compareBuffers_task(task, src2, dst2, IDMA_XFER_SIZE );
   compareBuffers_task(task, src3, dst3, IDMA_XFER_SIZE );
*/
   exit(0);
   return -1;
}


int
main (int argc, char**argv)
{
   int ret = 0;
//   printf("\n\n\nTest '%s'\n\n\n", argv[0]);


#if defined _XOS
   ret = test_xos();
#else
   ret = test(0, 0);
#endif
  // test() exits so this is never reached
  return ret;
}

