/*
 * Copyright (c) 2019 by Cadence Design Systems. ALL RIGHTS RESERVED.
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

/* Default implementation of OS interface. If no OS is actually present,
   this defaults to using XTOS.
 */

#ifdef _FREERTOS_
  #include "FreeRTOS.h"
  #include "semphr.h"
  //#include "xtensa_rtos.h"

#if  (_RTOS_VER_ == 20)
#elif  (_RTOS_VER_ == 17)
#else
#error  "_RTOS_VER_ must be defined as 17 or 20"
#endif


#endif

#include "idma_internal.h"
#include "_idma.h"
#include "k_debug.h"


#include <xtensa/xtruntime.h>
#if XCHAL_HAVE_INTERRUPTS
#include <xtensa/tie/xt_interrupt.h>
#endif


static ALIGNDCACHE idma_buf_t * buf_list[XCHAL_IDMA_NUM_CHANNELS];
#ifdef _FREERTOS_
  char idma_lib_ver[] = "_FREERTOS_";
#else
  char idma_lib_ver[] = "_XTOS_";
#endif


#ifdef _FREERTOS_
  QueueHandle_t IDMA_SEMA;
#endif


#ifdef _FREERTOS_

idma_status_t idma_task_processing_i(int32_t ch);

/*
 * IDMA interrupt handler. Retires any pending task and invokes the
 * callback (if present) associated with the completed requests
 */
void
idma_task_done_intr_handler_freertos(void * arg)
{
  int32_t ch = cvt_voidp_to_int32(arg);
  int ret;
  BaseType_t pxHigherPriorityTaskWoken;

  XLOG(ch, "task_done intr\n");
  // Return value not useful here.
  idma_task_processing_i(ch);
  //idma_thread_unblock(NULL);

  ret = xSemaphoreGiveFromISR(IDMA_SEMA, &pxHigherPriorityTaskWoken);

  if(ret == errQUEUE_FULL)
	  K_ASSERT(0, "idma_task_done_intr_handler_freertos\n");
  if(ret != pdTRUE)
	  K_ASSERT(0, "idma_task_done_intr_handler_freertos");
}

void
idma_task_err_intr_handler_freertos(void * arg)
{
  int32_t ch = cvt_voidp_to_int32(arg);

  XLOG(ch, "task_err intr\n");
  // Return value not useful here.
  (void) idma_task_processing_i(ch);
  //idma_thread_unblock(NULL);
}
#endif

#include "xtensa_api.h"
#include "xtensa_rtos.h"



__attribute__((weak)) int32_t
idma_register_interrupts(int32_t ch, os_handler done_handler, os_handler err_handler)
{

	//idma_print(0, idma_lib_ver);
#if XCHAL_HAVE_INTERRUPTS
#ifdef _FREERTOS_


    uint32_t doneint = (uint32_t) XCHAL_IDMA_CH0_DONE_INTERRUPT + (uint32_t) ch;
    uint32_t errint  = (uint32_t) XCHAL_IDMA_CH0_ERR_INTERRUPT + (uint32_t) ch;
    xt_handler  ret     = 0;

    if (ch < XCHAL_IDMA_NUM_CHANNELS) {
    ret = xt_set_interrupt_handler( (uint32_t)doneint, (xt_handler) done_handler, cvt_int32_to_voidp(ch));
    ret = xt_set_interrupt_handler( errint, (xt_handler) err_handler, cvt_int32_to_voidp(ch));


#if (_RTOS_VER_ == 17)
    xt_ints_on(1 << doneint);
    xt_ints_on(1 << errint);
#else
    xt_interrupt_enable( doneint );
    xt_interrupt_enable( errint );

#endif


#define uxMaxCount 10
    IDMA_SEMA = xSemaphoreCreateCounting( uxMaxCount, 0 );

    if(IDMA_SEMA == NULL)
    	return -1;


#ifndef  XTENSA_PORT_VERSION
#error "XTENSA_PORT_VERSION should be defined to for different FreeRTOS"
#endif

    if(XTENSA_PORT_VERSION == 1.7 )
      if(ret == NULL)
        return IDMA_OK ;

    if( (XTENSA_PORT_VERSION == 2.1) || (XTENSA_PORT_VERSION == 2.01) )
      if(ret != NULL)
        return IDMA_OK ;

    }

	return -1;




#else


	if (ch < XCHAL_IDMA_NUM_CHANNELS) {
		int32_t  ret     = 0;
        uint32_t doneint = (uint32_t) XCHAL_IDMA_CH0_DONE_INTERRUPT + (uint32_t) ch;
        uint32_t errint  = (uint32_t) XCHAL_IDMA_CH0_ERR_INTERRUPT + (uint32_t) ch;

        // Normally none of these calls should fail. We don't check them individually,
        // any one failing will cause this function to return failure.
        ret += xtos_set_interrupt_handler(doneint, done_handler, cvt_int32_to_voidp(ch), NULL);
        ret += xtos_interrupt_enable(doneint);

        ret += xtos_set_interrupt_handler(errint, err_handler, cvt_int32_to_voidp(ch), NULL);
        ret += xtos_interrupt_enable(errint);

        return ret;
    }

	return -1;

#endif


#endif
}

__attribute__((weak)) uint32_t
idma_disable_interrupts(void)
{
#ifdef _FREERTOS_
	taskDISABLE_INTERRUPTS();
	//(void)XTOS_SET_INTLEVEL(XCHAL_EXCM_LEVEL);
	//portbenchmarkINTERRUPT_DISABLE();
	return 0;
#else
    return XTOS_SET_INTLEVEL(XCHAL_NUM_INTLEVELS);
#endif

}

__attribute__((weak)) void
idma_enable_interrupts(uint32_t level)
{
#ifdef _FREERTOS_
	taskENABLE_INTERRUPTS();
#else
    XTOS_RESTORE_INTLEVEL(level);
#endif
}

__attribute__((weak)) void *
idma_thread_id(void)
{
#ifdef _FREERTOS_
	TaskHandle_t task_handle;
	task_handle= xTaskGetCurrentTaskHandle();
	return (void *)task_handle;
#else
    return NULL;
#endif
}

__attribute__((weak)) void
idma_thread_block(void * thread) // parasoft-suppress MISRA2012-RULE-8_13_a-4 "Cannot use const in generic API"
{
   // (void) thread;    // Unused

#ifdef _FREERTOS_
    K_ASSERT(0, "should not call here");
#else


#if XCHAL_HAVE_XEA3
    XTOS_SET_INTLEVEL(0);
    XTOS_DISABLE_INTERRUPTS();
#endif
#if XCHAL_HAVE_INTERRUPTS
    XT_WAITI(0);
#endif

#endif
}

__attribute__((weak)) void
idma_thread_unblock(void * thread) // parasoft-suppress MISRA2012-RULE-8_13_a-4 "Cannot use const in generic API"
{

   // (void) thread;    // Unused

#ifdef _FREERTOS_
    K_ASSERT(0, "should not call here");
#else

#endif
}

__attribute__((weak)) void
idma_chan_buf_set(int32_t ch, idma_buf_t * buf)
{
    if (ch < XCHAL_IDMA_NUM_CHANNELS) {
        buf_list[ch] = buf;
    }
}

__attribute__((weak)) idma_buf_t *
idma_chan_buf_get(int32_t ch)
{
    if (ch < XCHAL_IDMA_NUM_CHANNELS) {
        return buf_list[ch];
    }
    return NULL;
}

