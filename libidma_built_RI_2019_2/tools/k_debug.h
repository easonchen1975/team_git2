
#ifndef _INCLUDE_K_DEBUG_
#define _INCLUDE_K_DEBUG_

void idma_print(int ch, const char* fmt,...);
#define K_ASSERT assert_msg2
#define K_PrintASSERT assert_msg3
/*
#define assert_msg(val, _msg) \
{ \
	 if(val==0){ \
	    printf("%s \n",_msg); \
	    printf("ASSERT FAIL at"__FILE__" line:%d\n", __LINE__) ; \
	    while(0); \
     } \
}


#define assert_msg2(val, _msg... ) \
{ \
	 if(val==0){ \
	    printf("ASSERT FAIL at"__FILE__" line:%d\n", __LINE__) ; \
	    printf(_msg); \
	    while(1); \
     } \
}\


#ifdef IDMA_DEBUG
#define DPRINT(fmt...)	do { printf(fmt);} while (0);
#else
#define DPRINT(fmt...)	do {} while (0);

*/


#define assert_msg(val, _msg) \
{ \
	 if(val==0){ \
	    idma_print(0xff,"%s \n",_msg); \
	    idma_print(0xff,"ASSERT FAIL at"__FILE__" line:%d\n", __LINE__) ; \
	    while(0); \
     } \
}


#define assert_msg2(val, _msg... ) \
{ \
	 if(val==0){ \
	    idma_print(0xff,"ASSERT FAIL at"__FILE__" line:%d\n", __LINE__) ; \
	    idma_print(0xff,_msg); \
	    while(1); \
     } \
}\

#define assert_msg3(val, _msg... ) \
{ \
	 if(val==0){ \
	    printf("ASSERT FAIL at"__FILE__" line:%d\n", __LINE__) ; \
	    printf(_msg); \
	    while(1); \
     } \
}\


/*
#define assert_msg(val, _msg) \
{ \
	 if(val==0){ \
	    printf("%s \n",_msg); \
	    printf("ASSERT FAIL at"__FILE__" line:%d\n", __LINE__) ; \
	    while(0); \
     } \
}


#define assert_msg2(val, _msg... ) \
{ \
	 if(val==0){ \
		 printf("ASSERT FAIL at"__FILE__" line:%d\n", __LINE__) ; \
		 printf(_msg); \
	    while(1); \
     } \
}\

*/

//#define _FREERTOS_VER_  17




extern void  K_dump(uint32_t a);
extern void K_dump_exc(uint32_t a);











#endif


