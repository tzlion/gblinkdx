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
//status	register (baseaddress + 1) 	0x379 	0x279
#define LPTREG_STATUS				0x379
//control	register (baseaddress + 2) 	0x37a 	0x27a
#define LPTREG_CONTROL				0x37A

enum {
	STATUS_BUSY				= 0x80,
	STATUS_ACK				= 0x40,
	STATUS_PAPER			= 0x20,
	STATUS_SELECTIN			= 0x10,
	STATUS_ERROR			= 0x08,
	STATUS_NIRQ				= 0x04,
};

enum {
	CTL_MODE_DATAIN			= 0x20,
	CTL_MODE_IRQACK			= 0x10,
	CTL_SELECT				= 0x08,
	CTL_INIT				= 0x04,
	CTL_LINEFEED			= 0x02,
	CTL_STROBE				= 0x01,
};

enum {
	// .... ..cd
	//	c:	clock
	//	d:	data (serial bit)
	D_CLOCK_HIGH	= 0x02,
};

typedef struct {
	U8 carttype,romsize,ramsize;
	U16 checksum;
	char gamename[17];

	int totalbanks;
} GBHDR;

/******************************************************************************/
#endif
/******************************************************************************/

