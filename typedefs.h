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
#ifndef _WIN32
typedef unsigned short		BOOL;
#endif

//data		register(baseaddress + 0)  	0x378  	0x278
#define LPTREG_DATA					0x378

typedef struct {
	U8 carttype,romsize,ramsize;
	U16 checksum;
	char gamename[17];

	int totalbanks;
} GBHDR;

/******************************************************************************/
#endif
/******************************************************************************/

