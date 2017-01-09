/******************************************************************************/
// gblinkdl.cpp
// By Brian Provinciano
// http://www.bripro.com
// November 2nd, 2005
/******************************************************************************/
#include "stdafx.h"
// #include "conio.h"  // WIN ONLY
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <sys/io.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;
/******************************************************************************/
GBHDR hdr;
U8 bank0[0x4000];
char szt[1000];
/******************************************************************************/
unsigned char inportb(unsigned short port)
{
#ifdef WINDOWS
	unsigned char res;
	__asm (
		mov dx, port
		in al,dx
		mov res, al
	);
	return res;
#else
   return inb(port);
#endif
}
/******************************************************************************/
void outportb(unsigned short port, unsigned char value)
{
#ifdef WINDOWS
   __asm (
		mov dx, port
		mov al, value
		out dx, al
   );
#else
   outb(value,port);
#endif
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
	// This isn't used BUT must have bank0 populated before its run
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

void readBankZero()
{
    // read the first bank of ROM
	printf("\nDownloading first bank...\n");
	for(int i=0;i<0x4000;i++) {
		bank0[i] = gb_readbyte();
		///// why tho
	}
}

int doDump(int argc, char* argv[], U8 carttype, int bankscount)
{
	char* filename;
	if ( argc < 2 || memcmp(argv[1],"-i",2) == 0 ) {
		filename="idump.gb";
	} else {
		filename=argv[1];
	}
	FILE *f = fopen(filename,"wb");
    if(!f) {
        printf("Unable to open file: %s for writing!\n",filename);
        return 2;
    }

	printf("\nWriting to file: %s\n",filename);


	if (argc >= 3) {

		printf("CUSTOM MODE\n");
		bankscount = 0x100;
		carttype = 0xFF;

		if ( memcmp(argv[2],"override",8) != 0 ) {
			FILE *script = fopen(argv[2], "r");
			if (!script) {
				printf("Unable to open file: %s for reading!\n", argv[2]);
				return 2;
			}


			char line[10];
			bool oddline = false;
			int lastval = 0;
			while (!feof(script)) {
				fgets(line, 10, script);
				int i = strtol(line, NULL, 16);
				//printf("%04X\n", i);
				if (!oddline) {
					oddline = true;
				}
				else {
					gb_sendwrite(lastval, i);
					oddline = false;
				}
				lastval = i;
			}
		}



		// re-read the first bank of ROM
		//for (int i = 0; i<0x4000; i++)
		//	bank0[i] = gb_readbyte();
	}



    // dump the data
    switch(carttype) {
        case 0: // 0 - ROM ONLY
			// readBankZero();
		    fwrite(bank0,0x4000,1,f);
            gb_readblock(f, 0x4000,0x4000);
            break;
        case 1: // ROM+MBC1
        case 2: // ROM+MBC1+RAM
        case 3: // ROM+MBC1+RAM+BATT
			//readBankZero();
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
			printf("Cartridge type (real): %02X\n", hdr.carttype);
			if (false&&memcmp(hdr.gamename, "TIMER MONSTER", 13) == 0) {
				printf("VF Multi test\n");
				gb_sendwrite(0x5000, 0xAA);
				gb_sendwrite(0x7000, 0x80);
				gb_sendwrite(0x5000, 0xBB);
				gb_sendwrite(0x7000, 0x32);
				gb_sendwrite(0x5000, 0x55);
				gb_sendwrite(0x7000, 0x82);
				gb_readblock(f, 0x0000, 0x4000);
			} else {
				printf("Trying as regular ass MBC(5)\n");
				//fwrite(bank0, 0x4000, 1, f);
				gb_readblock(f, 0x0000, 0x4000);
			}
			for(int bank = 1; bank<bankscount; bank++) {
                    gb_sendwrite(0x2000,bank);
                    gb_readblock(f, 0x4000,0x4000);
                    printf("bank %02x/%02x transferred.\n",bank,bankscount-1);
            }
            break;
	}
	fclose(f);

	printf("\ndone.\n");

	return 0;

}


// http://stackoverflow.com/questions/236129/split-a-string-in-c
void split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
}
vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

int interactive()
{
	printf("\nInteractive mode\n");
	printf("Usage:\nr xxxx yy to read yy bytes from xxxx\nw xxxx yy to write yy to xxxx\nd to exit interactive & dump to idump.gb\nx to exit\nMultiple commands can be separated by semicolons\n");

	while(1) {
		std::string inpstring = "";
		std::string mode = "";
		//while (mode != "x" && mode != "w" && mode != "r" && mode != "d") {
		printf("\nEnter command: ");
		getline(std::cin,inpstring);
		//}

		printf("\n");

		vector<string> commands = split(inpstring,';');

		for(uint s=0;s<commands.size();s++) {

			string command = commands[s];

			mode = command.substr(0,1);

			if ( mode == "x" ) {
				return -1;
			}
			if ( mode == "d" ) {
				return 1;
			}
			if ( mode == "t" ) {
				gb_sendblockread(0x134,16);
				char newtitle[17];
				gb_readstring(newtitle,16);
				newtitle[16] = '\0';
				printf("Title: %s\n",newtitle);
			}
			if ( ( mode == "r" || mode == "w" ) && ( command.length() != 9 || command.substr(1,1) != " " || command.substr(6,1) != " " ) )  {
				printf("Unrecognised format\n");
				continue;
			}
			if ( mode == "r" ) {
				//printf("enter hex addr to read\n");
				std::string addrs=command.substr(2,4);
				//getline(std::cin,addrs);
				int addr = strtol(addrs.c_str(), NULL, 16);

				//printf("enter length to read in hex\n");
				std::string lens=command.substr(7,2);
				//getline(std::cin,lens);
				int len= strtol(lens.c_str(), NULL, 16);

				gb_sendblockread(addr,len);
				for(int i=0;i<len;i++) {
					if (i>0 && i % 16 == 0) {
						printf("\n");
					}
					printf("%02x ",gb_readbyte());
				}

				printf("\n");
			}
			if ( mode == "w" ) {
				//printf("enter hex addr to write\n");
				std::string addrs=command.substr(2,4);
				//getline(std::cin,addrs);
				int addr = strtol(addrs.c_str(), NULL, 16);
				//printf("enter hex val to write\n");
				std::string vals=command.substr(7,2);
				//getline(std::cin,vals);
				int val = strtol(vals.c_str(), NULL, 16);
				gb_sendwrite(addr,val);
			}

		}

	}
	return 0;
}

/******************************************************************************/
int main(int argc, char* argv[])
{
    printf(
        "GBlinkdl PC Client\n"
        "Original by Brian Provinciano November 2nd, 2005 http://www.bripro.com\n"
		"Modified by some lion Aug 2016\n\n");

    if(argc < 2) {
		printf("Usage: gblinkdl \"output filename\" \"pre-dump script (optional)\"\n\n");
        return 3;
    }

	printf("Setting up ports...\n");
#ifndef WINDOWS
	ioperm(0x378,3,true);
#endif
    // set up the parallel port
	outportb(LPTREG_CONTROL, inportb(LPTREG_CONTROL)&(~CTL_MODE_DATAIN));
	outportb(LPTREG_DATA, 0xFF);
	outportb(LPTREG_DATA, D_CLOCK_HIGH);

    // perform communication
	printf("Waiting for Game Boy...\n");
    while(gb_sendbyte(0x9A)!=0xB4)
		if(/*kbhit()*/false) return 1;
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

	printf("press enter to continue");
	std::string z;
	getline(std::cin,z);

	if(gb_readbyte() != 0) {// verify we're done
		printf("expected 0x00 from GB, bad connection\n");
		//getch();
		return 1;
	}
	if(gb_readbyte() != 0xFF) {// verify we're done
		printf("expected 0xFF from GB, bad connection\n");
		//getch();
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

	U8 carttype = hdr.carttype;
	int bankscount = hdr.totalbanks;

	readBankZero(); // This is where this apparently needs to be done // tried to remove it but it fails if no

	while(1) {

		bool dump = false;
		bool isInter = memcmp(argv[1],"-i",2) == 0;

		if (argc >= 2 && isInter) {
			int intret = interactive();
			if ( intret == 1 )  {
				dump = true;
			} else if ( intret == -1 ) {
				break;
			}
		} else {
			dump = true;
		}

		if ( dump ) {
			int retval= doDump(argc,argv, carttype, bankscount);
			if ( retval != 0 ) return retval;
		}

		if ( isInter ) {
			printf("\nReturn to interactive mode y/n?\n");
		} else {
			printf("\nDump again y/n?\n");
		}
		std::string z;
		getline(std::cin,z);
		if ( z != "y" ) {
			break;
		}

	}

	outportb(LPTREG_DATA, D_CLOCK_HIGH);
	outportb(LPTREG_CONTROL, inportb(LPTREG_CONTROL)&(~CTL_MODE_DATAIN));
	outportb(LPTREG_DATA, 0xFF);

	printf("exiting\n");

	return 0;
}
/******************************************************************************/


