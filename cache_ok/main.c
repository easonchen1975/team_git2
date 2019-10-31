

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define POOL_SIZE 64*64

#include "commonDef.h"

unsigned char  pBankBuffPool0[POOL_SIZE] __attribute__((section(".dram0.data")));
uint8_t  pBankBuffPool1[POOL_SIZE] __attribute__((section(".dram1.data")));

void process_data(); // __attribute__((section(".iram.text")));


void process_data(){

	  uint8_t *pSrc, *pDst;
	  int indy, indx, width, height, index, pitch,cycleStop, cycleStart ;
	  int loop = 20 ;

	  for(loop = 0; loop< 20; loop++){
#pragma no_reorder
      TIME_STAMP(cycleStart);

	  pSrc = pBankBuffPool0;
	  pDst = pBankBuffPool1;
	  width =64;
	  height = 64;
	  pitch = 64;

/*
	  if(loop >10 ){

			__asm__ __volatile__("movi	a15, 0xfc0000");//  disable cache
			__asm__ __volatile__("nop");__asm__ __volatile__("nop");__asm__ __volatile__("nop");
			__asm__ __volatile__("wsr.memctl	a15");
	  }
*/
	  for (indy = 0; indy < height; indy++)
	  {
	    for (indx = 0; indx < width; indx++)
	    {
	      index       = indy * pitch + indx;
	      pDst[index] = pSrc[index];
	    }
	  }

#pragma no_reorder
      TIME_STAMP(cycleStop);
      printf("cycles = %d\n", cycleStop-cycleStart);
	  }
}



int main( int argc, char **argv )
{
	printf("Hello World\n");
	printf("process data with noncache\n");
		__asm__ __volatile__("movi	a15, 0x0");//  disable cache
		__asm__ __volatile__("nop");__asm__ __volatile__("nop");__asm__ __volatile__("nop");
		__asm__ __volatile__("wsr.memctl	a15");
	process_data();

	__asm__ __volatile__("movi	a15, 0xfc0000");//  enable cache
	__asm__ __volatile__("nop");__asm__ __volatile__("nop");__asm__ __volatile__("nop");
	__asm__ __volatile__("wsr.memctl	a15");

	printf("process data with cache enable\n");
	process_data();
	printf("END\n");


	return 0;
}
