/*
 * test_idma.c
 *
 *  Created on: 2019¦~9¤ë23¤é
 *      Author: eason.chen
 */


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

#include "commonDef.h"

#define IMAGE_CHUNK_SIZE	2
#define IDMA_XFER_SIZE		IDMA_SIZE(IMAGE_CHUNK_SIZE)
#define IDMA_IMAGE_SIZE		IMAGE_CHUNK_SIZE*200
#define IDMA_INTERRUPT_ENABLE  0x80000000
#define IDMA_INTERRUPT_DISABLE 0x00000000
IDMA_BUFFER_DEFINE(task, 3, IDMA_1D_DESC);
IDMA_BUFFER_DEFINE(task2, 3, IDMA_1D_DESC);
IDMA_BUFFER_DEFINE(task3, 30, IDMA_1D_DESC);
IDMA_BUFFER_DEFINE(task4, 1, IDMA_1D_DESC);

ALIGNDCACHE char dst1[IDMA_XFER_SIZE] __attribute__ ((section(".sram.data")));;;
ALIGNDCACHE char src1[IDMA_XFER_SIZE] IDMA_DRAM;
ALIGNDCACHE char dst2[IDMA_XFER_SIZE] __attribute__ ((section(".sram.data")));;;
ALIGNDCACHE char src2[IDMA_XFER_SIZE] IDMA_DRAM;
ALIGNDCACHE char dst3[IDMA_XFER_SIZE] __attribute__ ((section(".sram.data")));;;
ALIGNDCACHE char src3[IDMA_XFER_SIZE] IDMA_DRAM;

// DRAM to DRAM move large block
/*
#define DST7_TOTALSIZE 1024*64
#define DST7_SIZE2 1024*10
#define IDMA_MEM1 1024*100
#define UNALIGNED_SIZE 64*64
*/

#define DST7_TOTALSIZE 1024*6
#define DST7_SIZE2 1024*1
#define IDMA_MEM1 1024*10
#define UNALIGNED_SIZE 64*6


ALIGNDCACHE char src4[DST7_TOTALSIZE] __attribute__ ((section(".dram1.data")));
ALIGNDCACHE char i_dst9[UNALIGNED_SIZE] IDMA_DRAM;
ALIGNDCACHE char src6[UNALIGNED_SIZE] IDMA_DRAM;

ALIGNDCACHE char dst4[IDMA_XFER_SIZE] __attribute__ ((section(".sram.data")));;;
ALIGNDCACHE char dst5[IDMA_XFER_SIZE] __attribute__ ((section(".sram.data")));;;
ALIGNDCACHE char dst6[IDMA_XFER_SIZE] __attribute__ ((section(".sram.data")));;;



ALIGNDCACHE char dst7[DST7_TOTALSIZE] __attribute__ ((section(".sram.data")));
ALIGNDCACHE char dst8[DST7_TOTALSIZE] __attribute__ ((section(".sram.data")));;
ALIGNDCACHE char golden[UNALIGNED_SIZE] __attribute__ ((section(".sram.data")));;

uint32_t g_taskstatus;

#define PROFILE_TIME

#ifndef PROFILE_TIME
 #define TIME_STAMP
#endif

void compareBuffers_task(idma_buffer_t* task, char* src, char* dst, int size)
{
  int status = idma_task_status(task);
  printf("Task %p status: %s size=%d\n", task,
                    (status == IDMA_TASK_DONE)    ? "DONE"       :
                    (status > IDMA_TASK_DONE)     ? "PENDING"    :
                    (status == IDMA_TASK_ABORTED) ? "ABORTED"    :
                    (status == IDMA_TASK_ERROR)   ? "ERROR"      :  "UNKNOWN" , size  );

  if(status == IDMA_TASK_ERROR) {
    idma_error_details_t* error = idma_error_details();
    printf("COPY FAILED, Error 0x%x at desc:%p, PIF src/dst=%x/%x\n", error->err_type, (void*)error->currDesc, error->srcAddr, error->dstAddr);
    g_taskstatus = IDMA_TASK_ERROR;
    return;
  }

  if(src && dst) {
    xthal_dcache_region_invalidate(dst, size);
    compareBuffers(src, dst, size);
  }
}


void seq_fill(unsigned char *dst, int size)
{
   int i;

   for(i=0; i< size; i++){

	   *(dst+i) = i;
   }

}

/* THIS IS THE EXAMPLE, main() CALLS IT */
int
test(void* arg, int unused)
{
   idma_state_t istate;
   uint32_t cyclesStart, cyclesStop,cyclesIVP;
   uint32_t verify_item_num=0;

           cyclesStart=0;
           cyclesStop=0;
           cyclesIVP=0;

   TIME_STAMP(cyclesStart);

   idma_log_handler(idmaLogHander);
   idma_init(0, MAX_BLOCK_2, 16, TICK_CYCLES_8, 100000, NULL);

#if 1

   bufRandomize(src1, IDMA_XFER_SIZE);
   bufRandomize(src2, IDMA_XFER_SIZE);
   bufRandomize(src3, IDMA_XFER_SIZE);
   memset(dst1, 0, IDMA_XFER_SIZE);
   memset(dst2, 0, IDMA_XFER_SIZE);
   memset(dst3, 0, IDMA_XFER_SIZE);



   idma_init_task(task, IDMA_1D_DESC, 3, NULL, NULL);
   idma_init_task(task3, IDMA_1D_DESC, 30, NULL, NULL);

   DPRINT("Setting IDMA request %p (4 descriptors)\n", task);
   idma_add_desc(task, dst1, src1, IDMA_XFER_SIZE, IDMA_INTERRUPT_ENABLE );
   idma_add_desc(task, dst2, src2, IDMA_XFER_SIZE, IDMA_INTERRUPT_ENABLE );
   idma_add_desc(task, dst3, src3, IDMA_XFER_SIZE, IDMA_INTERRUPT_ENABLE );

   DPRINT("Schedule IDMA request\n");
   idma_schedule_task(task);

   while (idma_task_status(task) > 0)
      idma_process_tasks();

   TIME_STAMP(cyclesStop);
   cyclesIVP = cyclesStop - cyclesStart;
   DPRINT("time cyle = %d", cyclesIVP);


   compareBuffers_task(task, src1, dst1, IDMA_XFER_SIZE );
   compareBuffers_task(task, src2, dst2, IDMA_XFER_SIZE );
   compareBuffers_task(task, src3, dst3, IDMA_XFER_SIZE );

   K_ASSERT(g_taskstatus != IDMA_TASK_ERROR, "IDMA_TASK_ERRPR");
   DPRINT("===> [Item %d]: Verify 1D DMEM to DMEM XFER SIZE =%d: PASS]\n", verify_item_num, IDMA_XFER_SIZE);
   verify_item_num++;

#endif

#if 0
   // 2. Verify IDMA Halt using 2 tasks
   //IDMA_BUFFER_DEFINE(task2, 3, IDMA_1D_DESC);
   DPRINT("Verify IDMA Halt");
   memset(dst1, 0, IDMA_XFER_SIZE);
   memset(dst2, 0, IDMA_XFER_SIZE);
   memset(dst3, 0, IDMA_XFER_SIZE);
   memset(dst4, 0x55, IDMA_XFER_SIZE);
   memset(dst5, 0x66, IDMA_XFER_SIZE);
   memset(dst6, 0x77, IDMA_XFER_SIZE);

   DPRINT("Schedule IDMA task1 request\n");
   idma_schedule_task(task);
   idma_pause();

   istate =idma_get_state();
   DPRINT("IDMA HW state =%d \n", istate);

   idma_init_task(task2, IDMA_1D_DESC, 3, NULL, NULL);
   idma_add_desc(task2, dst4, src1, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task2, dst5, src2, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task2, dst6, src3, IDMA_XFER_SIZE, 0 );
   //idma_schedule_task(task2);


   compareBuffers_task(task, src1, dst1, IDMA_XFER_SIZE );
   compareBuffers_task(task, src2, dst2, IDMA_XFER_SIZE );
   compareBuffers_task(task, src3, dst3, IDMA_XFER_SIZE );

   compareBuffers_task(task, src1, dst4, IDMA_XFER_SIZE );
   compareBuffers_task(task, src2, dst5, IDMA_XFER_SIZE );
   compareBuffers_task(task, src3, dst6, IDMA_XFER_SIZE );

   idma_schedule_task(task2);
   //idma_resume();
   while (idma_task_status(task) > 0)
      idma_process_tasks();

   while (idma_task_status(task2) > 0)
      idma_process_tasks();

   compareBuffers_task(task, src1, dst1, IDMA_XFER_SIZE );
   compareBuffers_task(task, src2, dst2, IDMA_XFER_SIZE );
   compareBuffers_task(task, src3, dst3, IDMA_XFER_SIZE );

   compareBuffers_task(task2, src1, dst4, IDMA_XFER_SIZE );
   compareBuffers_task(task2, src2, dst5, IDMA_XFER_SIZE );
   compareBuffers_task(task2, src3, dst6, IDMA_XFER_SIZE );

#endif


   // 3. Verify IDMA reset,
   idma_abort_tasks();


   // 4. Verify stress DMA test 30 descriptor
   idma_init(0, MAX_BLOCK_2, 16, TICK_CYCLES_8, 0, NULL);
   idma_init_task(task3, IDMA_1D_DESC, 30, NULL, NULL);

   memset(dst1, 0, IDMA_XFER_SIZE);
   memset(dst2, 0, IDMA_XFER_SIZE);
   memset(dst3, 0, IDMA_XFER_SIZE);
   DPRINT("%d. Setting IDMA request %p (4 descriptors)\n", verify_item_num, task);
   idma_add_desc(task3, dst1, src1, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task3, dst2, src2, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task3, dst3, src3, IDMA_XFER_SIZE, 0 );

   idma_add_desc(task3, dst1, src1, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task3, dst2, src2, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task3, dst3, src3, IDMA_XFER_SIZE, 0 );

   idma_add_desc(task3, dst1, src1, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task3, dst2, src2, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task3, dst3, src3, IDMA_XFER_SIZE, 0 );

   idma_add_desc(task3, dst1, src1, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task3, dst2, src2, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task3, dst3, src3, IDMA_XFER_SIZE, 0 );

   idma_add_desc(task3, dst1, src1, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task3, dst2, src2, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task3, dst3, src3, IDMA_XFER_SIZE, 0 );

   idma_add_desc(task3, dst1, src1, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task3, dst2, src2, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task3, dst3, src3, IDMA_XFER_SIZE, 0 );

   idma_add_desc(task3, dst1, src1, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task3, dst2, src2, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task3, dst3, src3, IDMA_XFER_SIZE, 0 );

   idma_add_desc(task3, dst1, src1, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task3, dst2, src2, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task3, dst3, src3, IDMA_XFER_SIZE, 0 );

   idma_add_desc(task3, dst1, src1, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task3, dst2, src2, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task3, dst3, src3, IDMA_XFER_SIZE, 0 );

   idma_add_desc(task3, dst1, src1, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task3, dst2, src2, IDMA_XFER_SIZE, 0 );
   idma_add_desc(task3, dst3, src3, IDMA_XFER_SIZE, 0 );
   idma_schedule_task(task3);
   idma_pause();   //<-- pause take effect
   // pause must test with debug print on,  or it will be too fast to pause

   // PUASE test
   istate =idma_get_state();
   DPRINT("IDMA HW state =%d \n", istate);
   if( istate != IDMA_STATE_HALT ){
	   DPRINT(" PAUSE test PASS \n");
	   K_ASSERT(0, "PAUSE test fail istate=%d\n",istate);
   }
   else
	   idma_resume();



   istate =idma_get_state();
   DPRINT("IDMA HW state =%d \n", istate);
   while (idma_task_status(task3) > 0)
      idma_process_tasks();

   compareBuffers_task(task, src1, dst1, IDMA_XFER_SIZE );
   compareBuffers_task(task, src2, dst2, IDMA_XFER_SIZE );
   compareBuffers_task(task, src3, dst3, IDMA_XFER_SIZE );

   K_ASSERT(g_taskstatus != IDMA_TASK_ERROR, "IDMA_TASK_ERRPR");
   DPRINT("===> [Item %d]: Verify 1D DMEM to DMEM 30 Descriptor  XFER SIZE =%d: PASS]\n", verify_item_num, IDMA_XFER_SIZE);
   verify_item_num++;



#if 1
   // DRAM to DMEM small  block
   DPRINT("%d. Setting IDMA SYSDRAM to DMEM\n", verify_item_num, task);
   idma_abort_tasks();
   bufRandomize(dst7, DST7_SIZE2);
   bufRandomize(dst8, DST7_SIZE2);
   bufRandomize(src1, IDMA_XFER_SIZE);
   idma_init_task(task4, IDMA_1D_DESC, 1, NULL, NULL);
   idma_add_desc(task4, src1, dst7/*src1*/, IDMA_XFER_SIZE, 0 );
   idma_schedule_task(task4);
   while (idma_task_status(task4) > 0)
      idma_process_tasks();
   compareBuffers_task(task4, src1, dst7, IDMA_XFER_SIZE );

   K_ASSERT(g_taskstatus != IDMA_TASK_ERROR, "IDMA_TASK_ERRPR");
   DPRINT("===> [Item %d]: Verify 1D DRAM  -> DMEM XFER SIZE =%d: PASS]\n", verify_item_num, IDMA_XFER_SIZE);
   verify_item_num++;



   // Dmem to DRAM small block
   DPRINT("%d. Setting IDMA DMEM to SYSDRAM\n", verify_item_num, task);
   idma_abort_tasks();
   bufRandomize(dst7, DST7_SIZE2);
   bufRandomize(dst8, DST7_SIZE2);
   bufRandomize(src1, IDMA_XFER_SIZE);
   idma_init_task(task4, IDMA_1D_DESC, 1, NULL, NULL);
   idma_add_desc(task4, dst8, src1, IDMA_XFER_SIZE, 0 );
   idma_schedule_task(task4);
   while (idma_task_status(task4) > 0)
      idma_process_tasks();
   compareBuffers_task(task4, src1, dst8, IDMA_XFER_SIZE );

   K_ASSERT(g_taskstatus != IDMA_TASK_ERROR, "IDMA_TASK_ERRPR");
   DPRINT("===> [Item %d]: Verify 1D DMEM -> DRAM XFER SIZE =%d: PASS]\n", verify_item_num, IDMA_XFER_SIZE);
   verify_item_num++;


   // DRAM to DMEM large block
   DPRINT("%d. Setting IDMA DMEM to SYSDRAM\n", verify_item_num, task);
   idma_abort_tasks();
   bufRandomize(dst7, DST7_TOTALSIZE);
   bufRandomize(dst8, DST7_TOTALSIZE);
   bufRandomize(src4, DST7_TOTALSIZE);
   idma_init_task(task4, IDMA_1D_DESC, 1, NULL, NULL);
   idma_add_desc(task4, dst8, src4, DST7_SIZE2, 0 );
   idma_schedule_task(task4);
   while (idma_task_status(task4) > 0)
      idma_process_tasks();
   compareBuffers_task(task4, src4, dst8, DST7_SIZE2 );


   K_ASSERT(g_taskstatus != IDMA_TASK_ERROR, "IDMA_TASK_ERRPR");
   DPRINT("===> [Item %d]: Verify 1D DMEM -> DRAM XFER SIZE =%d: PASS]\n", verify_item_num, DST7_SIZE2);
   verify_item_num++;

   // Dmem to DRAM large block
   DPRINT("%d. Setting IDMA SYSDRAM -> DMEM  \n", verify_item_num, task);
   idma_abort_tasks();
   bufRandomize(dst7, DST7_TOTALSIZE);
   bufRandomize(dst8, DST7_TOTALSIZE);
   bufRandomize(src4, DST7_TOTALSIZE);
   idma_init_task(task4, IDMA_1D_DESC, 1, NULL, NULL);
   idma_add_desc(task4, src4, dst8, DST7_SIZE2, 0 );
   idma_schedule_task(task4);
   while (idma_task_status(task4) > 0)
      idma_process_tasks();
   compareBuffers_task(task4, src4, dst8, DST7_SIZE2 );

   K_ASSERT(g_taskstatus != IDMA_TASK_ERROR, "IDMA_TASK_ERRPR");
   DPRINT("===> [Item %d]: Verify 1D DRAM -> DMEM XFER SIZE =%d: PASS]\n", verify_item_num, DST7_SIZE2);
   verify_item_num++;


   // Dmem to DRAM large block
   DPRINT("%d. Setting IDMA SYSDRAM to DMEM  256K\n", verify_item_num, task);
   idma_abort_tasks();
   bufRandomize(dst7, DST7_TOTALSIZE);
   bufRandomize(dst8, DST7_TOTALSIZE);
   bufRandomize(src4, DST7_TOTALSIZE);
   idma_init_task(task4, IDMA_1D_DESC, 1, NULL, NULL);
   idma_add_desc(task4, src4, dst8, DST7_TOTALSIZE, 0 );
   idma_schedule_task(task4);
   while (idma_task_status(task4) > 0)
      idma_process_tasks();
   compareBuffers_task(task4, src4, dst8, DST7_TOTALSIZE );

   K_ASSERT(g_taskstatus != IDMA_TASK_ERROR, "IDMA_TASK_ERRPR");
   DPRINT("===> [Item %d]: Verify 1D DRAM -> DMEM XFER SIZE =%d: PASS]\n", verify_item_num, DST7_TOTALSIZE);
   verify_item_num++;

   // Dmem to DRAM large block
   DPRINT("%d. Setting IDMA  to SYSDRAM 256K\n", verify_item_num, task);
   idma_abort_tasks();
   bufRandomize(dst7, DST7_TOTALSIZE);
   bufRandomize(dst8, DST7_TOTALSIZE);
   bufRandomize(src4, DST7_TOTALSIZE);
   idma_init_task(task4, IDMA_1D_DESC, 1, NULL, NULL);
   idma_add_desc(task4, dst8, src4,  DST7_TOTALSIZE, 0 );
   idma_schedule_task(task4);
   while (idma_task_status(task4) > 0)
      idma_process_tasks();
   compareBuffers_task(task4, src4, dst8, DST7_TOTALSIZE );

   K_ASSERT(g_taskstatus != IDMA_TASK_ERROR, "IDMA_TASK_ERRPR");
   DPRINT("===> [Item %d]: Verify 1D DMEM -> DRAM XFER SIZE =%d: PASS]\n", verify_item_num, DST7_TOTALSIZE);
   verify_item_num++;

#endif


#if 0 // not support
   // SYSDRAM to SYSDRAM 128byte block
   DPRINT("%d. Setting IDMA SYSDRAM to SYSDRAM 256K\n", verify_item_num, task);
   idma_abort_tasks();
   bufRandomize(dst7, IDMA_XFER_SIZE);
   bufRandomize(dst8, IDMA_XFER_SIZE);
   bufRandomize(src4, IDMA_XFER_SIZE);
   idma_init_task(task4, IDMA_1D_DESC, 1, NULL, NULL);
   idma_add_desc(task4, dst8, dst7,  IDMA_XFER_SIZE, 0 );
   idma_schedule_task(task4);
   while (idma_task_status(task4) > 0)
      idma_process_tasks();
   compareBuffers_task(task4, dst7, dst8, IDMA_XFER_SIZE );
#endif

   // from src6 dmem to i_dst9 dmem
   DPRINT("\n%d. Setting IDMA UNALIGNED dmem to dmem \n", verify_item_num, task);
   idma_abort_tasks();

   seq_fill(src6, UNALIGNED_SIZE);
   seq_fill(i_dst9, UNALIGNED_SIZE);
   seq_fill(golden,UNALIGNED_SIZE);

   bufRandomize(dst7, IDMA_XFER_SIZE-1);
   bufRandomize(dst8, IDMA_XFER_SIZE-1);
   bufRandomize(src6+5, IDMA_XFER_SIZE-1);
   bufRandomize(golden+3, IDMA_XFER_SIZE-1);

   memcpy(golden+3, src6+5, IDMA_XFER_SIZE-1);

   idma_init_task(task4, IDMA_1D_DESC, 1, NULL, NULL);

   //idma_add_desc(task4, i_dst9, src6,  IDMA_XFER_SIZE, 0 );
   idma_add_desc(task4, i_dst9+3, src6+5,  IDMA_XFER_SIZE-1, 0 );
   idma_schedule_task(task4);
   while (idma_task_status(task4) > 0)
      idma_process_tasks();
   compareBuffers_task(task4, i_dst9, golden, UNALIGNED_SIZE );

   K_ASSERT(g_taskstatus != IDMA_TASK_ERROR, "IDMA_TASK_ERRPR");
   DPRINT("===> [Item %d]: Verify 1D DMEM to DMEM XFER UNALIGNED_SIZE SIZE =%d: PASS]\n", verify_item_num, UNALIGNED_SIZE);
   verify_item_num++;


   // from src6 dmem to dst7 dram
   DPRINT("%d. Setting IDMA UNALIGNED dmem to dram \n", verify_item_num, task);
   idma_abort_tasks();

   seq_fill(src6, UNALIGNED_SIZE);
   seq_fill(dst7, UNALIGNED_SIZE);
   seq_fill(golden,UNALIGNED_SIZE);

   //bufRandomize(dst7, IDMA_XFER_SIZE-1);
   //bufRandomize(dst8, IDMA_XFER_SIZE-1);
   bufRandomize(src6+5, IDMA_XFER_SIZE-1);
   //bufRandomize(golden+3, IDMA_XFER_SIZE-1);

   memcpy(golden+3, src6+5, IDMA_XFER_SIZE-1);

   idma_init_task(task4, IDMA_1D_DESC, 1, NULL, NULL);

   //idma_add_desc(task4, i_dst9, src6,  IDMA_XFER_SIZE, 0 );
   idma_add_desc(task4, dst7+3, src6+5,  IDMA_XFER_SIZE-1, 0 );
   idma_schedule_task(task4);
   while (idma_task_status(task4) > 0)
      idma_process_tasks();
   compareBuffers_task(task4, dst7, golden, UNALIGNED_SIZE );

   K_ASSERT(g_taskstatus != IDMA_TASK_ERROR, "IDMA_TASK_ERRPR");
   DPRINT("===> [Item %d]: Verify 1D DMEM -> dram XFER UNALIGNED_SIZE SIZE =%d: PASS]\n", verify_item_num, UNALIGNED_SIZE);
   verify_item_num++;


   // from dst7 dram to _idst9 dmem
   DPRINT("%d. Setting IDMA UNALIGNED dram to dmem \n", verify_item_num, task);
   idma_abort_tasks();

   seq_fill(i_dst9, UNALIGNED_SIZE);
   seq_fill(dst7, UNALIGNED_SIZE);
   seq_fill(golden,UNALIGNED_SIZE);

   bufRandomize(dst7+5, IDMA_XFER_SIZE-1);
   //bufRandomize(dst8, IDMA_XFER_SIZE-1);
   //bufRandomize(+5, IDMA_XFER_SIZE-1);
   //bufRandomize(golden+3, IDMA_XFER_SIZE-1);

   memcpy(golden+3, dst7+5, IDMA_XFER_SIZE-1);

   idma_init_task(task4, IDMA_1D_DESC, 1, NULL, NULL);

   //idma_add_desc(task4, i_dst9, src6,  IDMA_XFER_SIZE, 0 );
   idma_add_desc(task4, i_dst9+3, dst7+5,  IDMA_XFER_SIZE-1, 0 );
   idma_schedule_task(task4);
   while (idma_task_status(task4) > 0)
      idma_process_tasks();
   compareBuffers_task(task4, i_dst9, golden, UNALIGNED_SIZE );

   K_ASSERT(g_taskstatus != IDMA_TASK_ERROR, "IDMA_TASK_ERRPR");
   DPRINT("===> [Item %d]: Verify 1D DRAM -> DMEM XFER UNALIGNED_SIZE SIZE =%d: PASS]\n", verify_item_num, UNALIGNED_SIZE);
   verify_item_num++;


   exit(0);
   return -1;
}


int
main (int argc, char**argv)
{
   int ret = 0;
   printf("\n\n\nTest '%s'\n\n\n", argv[0]);

  // main2();

#if defined _XOS
   ret = test_xos();
#else
   ret = test(0, 0);
#endif

   //main_2d();
  // test() exits so this is never reached
  return ret;
}

