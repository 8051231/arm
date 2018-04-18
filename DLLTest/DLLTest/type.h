#ifndef __TYPE_H_
#define __TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SIMC_CALLBACK_OK 1
#define SIMC_OK 0
#define SIMC_ERROR -1
#define SIMC_TIMEOUT -2
#define SIMC_LEN_LACK -3
#define SIMC_CONNECT_ERROR -4
#define SIMC_CHANGE -5
#define SIMC_TIEM_LACK -6
#define SIMC_COMNECT_REJECT -7
#define SIMC_FLIE_NONENTITY -8

#define SIMC_NETLINK_NONE -7
#define SIMC_NETLINK_DOWN -8
#define SIMC_NETLINK_UNPLUGGED -9

#define SIMC_QUEUE_NULL -10
#define SIMC_QUEUE_NODE_NULL -11
#define SIMC_CALLBACK_ERROR -12

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