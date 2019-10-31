
#include <stdint.h>
#include <string.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "event_groups.h"

void test_time(){

	int count_after, count_before;
	printf("start test_time\n");
	  __asm__ __volatile__ ("rsr     %0, ccount" : "=a" (count_before) : : "memory");

	vTaskDelay(100*20);
	  __asm__ __volatile__ ("rsr     %0, ccount" : "=a" (count_after) : : "memory");

	printf("\ntest time at start clk=%d endclk=%d peirod=%d\n",count_before, count_after ,count_after-count_before );






}
