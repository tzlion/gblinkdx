/*********************************************************************/
#ifndef _typedefs_h_
#define _typedefs_h_
/*********************************************************************/

/* data types */
typedef unsigned char		U8;
typedef signed char			S8;
typedef unsigned short		U16;
typedef signed short		S16;
typedef unsigned long		U32;
typedef signed long			S32;
//typedef signed __int64		U64;
//typedef unsigned __int64 	S64;
#ifndef _WINDOWS_
typedef unsigned short		BOOL;
#endif

#define TRUE 				1
#define FALSE 				0


typedef struct {
	char *string;
    int index;
} STRINT;

#include "cbin.h"

enum {
 	_1K		= 0x00400,
 	_2K		= 0x00800,
 	_4K		= 0x01000,
 	_8K		= 0x02000,
 	_16K  	= 0x04000,
 	_32K  	= 0x08000,
	_64K  	= 0x10000ul,
	_128K  	= 0x20000ul,
	_256K  	= 0x40000ul,
 	_512K  	= 0x80000ul
};
/******************************************************************************/
#endif
/******************************************************************************/


