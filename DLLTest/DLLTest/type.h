#ifndef __TYPE_H_
#define __TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SIMC_CALLBACK_OK 1
#define SIMC_OK 0
#define SIMC_ERROR -1


//队列操作相关错误码
#define SIMC_QUEUE_NULL -10
#define SIMC_QUEUE_NODE_NULL -11
//cadi相关错误码
#define SIMC_CALLBACK_ERROR -12
#define SIMC_CADI_NULL -13
#define	SIMC_CADI_CALLBACK_NULL -14

//
#define	SIMC_MSG_LEN 4

#define true 1
#define false 0

	/* exact-width signed integer types */
//	typedef   char         s8;
//	typedef   short int      s16;
//	typedef   int             s32;

	/* exact-width unsigned integer types */
//	typedef unsigned          char  u8;
//	typedef unsigned short     int u16;
//	typedef unsigned           int  u32;

#define GET_ARRAY_LEN(array) (sizeof(array)/sizeof(array[0]))


#ifdef __cplusplus
}
#endif

#endif