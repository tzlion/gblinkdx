/******************************************************************************/
// gblinkdl.cpp
// By Brian Provinciano
// http://www.bripro.com
// November 2nd, 2005
/******************************************************************************/
#include "stdafx.h"
#include "conio.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
/******************************************************************************/
GBHDR hdr;
U8 bank0[0x4000];
char szt[1000];   
/******************************************************************************/
unsigned char inportb(unsigned short port)
{
    unsigned char res;
    asm {
        mov dx, port
        in al,dx
        mov res, al
    }
    return res;
}
/******************************************************************************/
void outportb(unsigned short port, unsigned char value)
{
    asm {
        mov dx, port
        mov al, value
        out dx, al
    }
}
/******************************************************************************/
char *printbin(U8 v)
{
	char *s = szt;
	for(int i=7;i>=0;i--) {
		*s++ = (v>>i)&1?'1':'.';
		if(i==4) *s++ = '-';
	}
	*s = '\0';
	return szt;
}
/******************************************************************************/
void lptdelay(int amt)
{
    for(int i=0;i<amt;i++)
		inportb(LPTREG_DATA);
}
/******************************************************************************/
U8 gb_sendbyte(U8 value)
{
	U8 read = 0;
	for(int i=7;i>=0;i--) {
		U8 v = (value>>i)&1;

		outportb(LPTREG_DATA, v|D_CLOCK_HIGH);
		outportb(LPTREG_DATA, v);
		
		U8 stat = inportb(LPTREG_STATUS);

		if(!(stat&STATUS_BUSY))
			read |= (1<<i);

		outportb(LPTREG_DATA, v|D_CLOCK_HIGH);
	}
	lptdelay(64);
	return read;
}
/******************************************************************************/
U8 gb_readbyte()
{
	U8 read = 0;
	for(int i=7;i>=0;i--) {
		outportb(LPTREG_DATA, D_CLOCK_HIGH);
		outportb(LPTREG_DATA, 0);

		if(!(inportb(LPTREG_STATUS)&STATUS_BUSY))
			read |= (1<<i);
		outportb(LPTREG_DATA, D_CLOCK_HIGH);
	}
	// delay between bytes
	lptdelay(64);
	return read;
}     
/******************************************************************************/
U16 gb_readword()
{
	return (gb_readbyte()<<8) | (gb_readbyte());
}    
/******************************************************************************/
char *gb_readstring(char *out, int len)
{
	for(int i=0;i<len;i++)
		out[i] = gb_readbyte();
	return out;
}
/******************************************************************************/
// can try to write to an area with the same value in case of bus conflicts
void gb_sendbankwrite(U16 start, U16 end, U8 val)
{
	while(start<=end) {
		if(bank0[start]==val)
			break;
		start++;
	}
	if(start>end)
		start = end;
	gb_sendbyte(start>>8);
	gb_sendbyte(start&0xFF);
	gb_sendbyte(val);
	printf("  Wrote %02X -> %04X\n",val,start);

}      
/******************************************************************************/
void gb_sendwrite(U16 addr, U8 val)
{
	gb_sendbyte(0x49);
	gb_sendbyte(addr>>8);
	gb_sendbyte(addr&0xFF);
	gb_sendbyte(val);
	printf("  Wrote %02X -> %04X\n",val,addr);
}      
/******************************************************************************/
void gb_sendblockread(U16 addr, U16 length)
{
	gb_sendbyte(0x59);
	gb_sendbyte(addr>>8);
	gb_sendbyte(addr&0xFF);
	gb_sendbyte(length>>8);
	gb_sendbyte(length&0xFF);
	printf("  Starting Block Read: %04X (%04X in size)\n",addr, length);
}
/******************************************************************************/
void gb_readblock(FILE *f, U16 addr, int len)
{
    printf("READ BLOCK: %04X, %04X\n",addr,len);
        gb_sendblockread(addr,len);
        for(int i=0;i<len;i++)
            fputc(gb_readbyte(),f);
}
/******************************************************************************/
int main(int argc, char* argv[])
{
    printf(
        "GBlinkdl PC Client\n"
        "By Brian Provinciano\n"
        "November 2nd, 2005\n"
        "http://www.bripro.com\n\n");

    if(argc != 2) {
        printf("Useage: gblinkdl \"output filename\"\n\n");
        return 3;
    }

    // set up the parallel port
	outportb(LPTREG_CONTROL, inportb(LPTREG_CONTROL)&(~CTL_MODE_DATAIN));
	outportb(LPTREG_DATA, 0xFF);

	outportb(LPTREG_DATA, D_CLOCK_HIGH);

    // perform communication
	printf("Waiting for Game Boy...");
    while(gb_sendbyte(0x9A)!=0xB4)
		if(kbhit()) return 1;
    lptdelay(2000);
    if(gb_sendbyte(0x9A)!=0x1D) {
        printf("Bad connection\n");
        return 1;
    }

	printf("Connected.\n\n");

    // read header info (not really needed anymore as I read the first block later)
	hdr.carttype = gb_readbyte();
	hdr.romsize = gb_readbyte();
	hdr.ramsize = gb_readbyte();
	hdr.checksum = gb_readword();

	gb_readstring(hdr.gamename,16);
	hdr.gamename[16] = '\0';

	printf(
		"GAME:     %s\n"
		"CARTTYPE: %02Xh\n"
		"ROMSIZE:  %02Xh\n"
		"RAMSIZE:  %02Xh\n"
		"CHECKSUM: %04Xh\n\n",
		hdr.gamename,hdr.carttype,hdr.romsize,hdr.ramsize,hdr.checksum
	);

	if(gb_readbyte() != 0) {// verify we're done
		printf("expected 0x00 from GB, bad connection\n");
		getch();
		return 1;
	}
	if(gb_readbyte() != 0xFF) {// verify we're done
		printf("expected 0xFF from GB, bad connection\n");
		getch();
		return 1;
	}


	hdr.totalbanks = (1<<(hdr.romsize&0xF)) * 2;
	if(hdr.romsize&0xF0)
		hdr.totalbanks += (1<<((hdr.romsize>>4)&0xF)) * 2;
	int banksize = 0x4000;
	printf(
		"size: %d KB\n",
		hdr.totalbanks*16
	);
                                 
	printf("\nReceiving...\n");
	printf("\nDownloading first bank...\n");
    // read the first bank of ROM
	for(int i=0;i<0x4000;i++)
		bank0[i] = gb_readbyte();


	FILE *f = fopen(argv[1],"wb");
    if(!f) {
        printf("Unable to open file: %s for writing!\n",argv[1]);
        return 2;
    }

	printf("\nWriting to file: %s\n",argv[1]);

    // dump the data
    switch(hdr.carttype) {
        case 0: // 0 - ROM ONLY
		    fwrite(bank0,0x4000,1,f);
            gb_readblock(f, 0x4000,0x4000);
            break;
        case 1: // ROM+MBC1
        case 2: // ROM+MBC1+RAM
        case 3: // ROM+MBC1+RAM+BATT
		    fwrite(bank0,0x4000,1,f);

            if(memcmp(hdr.gamename,"SUPER MARIO 3",13)==0) {
                printf("Detected Super Mario 3 Special. Doing RAW Dump\n");
                const int remap[13] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x08,0x0B,0x0C,0x0D,0x0F,0x13};
                gb_sendwrite(0x6000,5);
                gb_sendwrite(0x5000,0);
                for(int bank = 1; bank<13; bank++) {
                    gb_sendwrite(0x2000,remap[bank]);
                    gb_readblock(f, 0x4000,0x4000);
                    printf("bank %02x/%02x transferred.\n",bank,13);
                }
                for(int i=0;i<3*0x4000;i++)
                    fputc(0xFF,f);
            } else {
                // normal MBC1
                for(int bank = 1; bank<hdr.totalbanks; bank++) {
                    gb_sendwrite(0x2000,bank);
                    gb_readblock(f, 0x4000,0x4000);
                    printf("bank %02x/%02x transferred.\n",bank,hdr.totalbanks-1);
                }
            }
            break;
        default:
            printf("Unsupported cartridge type: %02X\n", hdr.carttype);
            break;
	}
	fclose(f);

	printf("\ndone.");

	outportb(LPTREG_DATA, D_CLOCK_HIGH);
	outportb(LPTREG_CONTROL, inportb(LPTREG_CONTROL)&(~CTL_MODE_DATAIN));
	outportb(LPTREG_DATA, 0xFF);

	printf("exiting\n");

	return 0;
}
/******************************************************************************/


