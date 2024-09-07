/******************************************************************************/
// gblinkdl.cpp
// Original by Brian Provinciano
// http://www.bripro.com
// November 2nd, 2005
// Modified by taizou 2016-2017
/******************************************************************************/
#include "ppgb.h"
#include "typedefs.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#ifdef __linux__
#include <unistd.h>
#include <sys/io.h>
#elif defined(__FreeBSD__)
#include <unistd.h>
#include <sys/types.h>
#include <machine/cpufunc.h>
#include <machine/sysarch.h>
#elif defined(_WIN32)
#include "windows.h"
#else
#error Unsupported platform
#endif
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;
/******************************************************************************/
GBHDR hdr;
U8 bank0[0x4000];
char szt[1000];
bool quietMode = false;
bool xbooCompat = false;

unsigned short dataPort = LPTREG_DATA;
unsigned short statusPort = LPTREG_STATUS;
unsigned short controlPort = LPTREG_CONTROL;

#ifdef _WIN32
typedef void(__stdcall *lpOut32)(short, short);
typedef short(__stdcall *lpInp32)(short);
lpOut32 gfpOut32;
lpInp32 gfpInp32;
#endif

void printMessage(const char* message)
{
    printf("%s\n", message);
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
U8 gb_sendbyte(U8 value)
{
    return PPGBTransfer(value);
}
/******************************************************************************/
U8 gb_readbyte()
{
	return PPGBTransfer(0);
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
	if (!quietMode) printf("  Wrote %02X -> %04X\n",val,start);

}
/******************************************************************************/
void gb_sendwrite(U16 addr, U8 val)
{
	gb_sendbyte(0x49);
	gb_sendbyte(addr>>8);
	gb_sendbyte(addr&0xFF);
	gb_sendbyte(val);
	if (!quietMode) printf("  Wrote %02X -> %04X\n",val,addr);
}
/******************************************************************************/
void gb_sendblockread(U16 addr, U16 length)
{
	gb_sendbyte(0x59);
	gb_sendbyte(addr>>8);
	gb_sendbyte(addr&0xFF);
	gb_sendbyte(length>>8);
	gb_sendbyte(length&0xFF);
	if (!quietMode) printf("  Starting Block Read: %04X (%04X in size)\n",addr, length);
}
/******************************************************************************/
void gb_readblock(FILE *f, U16 addr, int len)
{
	if (!quietMode) printf("READ BLOCK: %04X, %04X\n",addr,len);
        gb_sendblockread(addr,len);
        for(int i=0;i<len;i++)
            fputc(gb_readbyte(),f);
}
/******************************************************************************/
void readBankZero()
{
    // read the first bank of ROM
	if (!quietMode) printf("\nDownloading first bank...\n");
	for(int i=0;i<0x4000;i++) {
		bank0[i] = gb_readbyte();
	}
}
/******************************************************************************/
int doKindaCrappyScriptedWrites(char* scriptName)
{
	FILE *script = fopen(scriptName, "r");
	if (!script) {
		printf("Unable to open file: %s for reading!\n", scriptName);
		return 2;
	}

	char line[10];
	bool oddline = false;
	int lastval = 0;
	while (!feof(script)) {
		fgets(line, 10, script);
		int i = strtol(line, NULL, 16);
		if (!oddline) {
			oddline = true;
		}
		else {
			gb_sendwrite(lastval, i);
			oddline = false;
		}
		lastval = i;
	}
	return 0;
}
/******************************************************************************/
int doDump(char* filename,bool overrideMode,U8 carttype, int bankscount)
{
	FILE *f = fopen(filename,"wb");
    if(!f) {
        printf("Unable to open file: %s for writing!\n",filename);
        return 2;
    }

	printf("\nWriting to file: %s\n",filename);


	if (overrideMode) {
		printf("CUSTOM MODE\n");
		bankscount = 0x100;
		carttype = 0xFF;
	}

    // dump the data
    switch(carttype) {
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
			printf("Cartridge type (real): %02X\n", hdr.carttype);
			printf("Trying as regular ass MBC(5)\n");
			gb_readblock(f, 0x0000, 0x4000);
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
/******************************************************************************/
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
/******************************************************************************/
int interactive()
{
	printf("\n* Interactive mode *\n\n");
	printf("Usage:\nr xxxx yy to read yy bytes from xxxx\nw xxxx yy to write yy to xxxx\nd to dump 4mb as standard mbc\na to attempt auto-detect and dump\nt to reread title\nx to exit\n"
	       "All numbers should be in hexadecimal\nMultiple commands can be separated by semicolons\nDon't put a space after the semicolon tho\n");

	while(1) {
		std::string inpstring = "";
		std::string mode = "";
		printf("\nEnter command: ");
		getline(std::cin,inpstring);

		printf("\n");

		vector<string> commands = split(inpstring,';');

		for(int s=0;s<commands.size();s++) {

			string command = commands[s];

			mode = command.substr(0,1);

			if ( mode == "x" ) {
				return -1;
			}
			if ( mode == "d" ) {
				return 1;
			}
			if ( mode == "a" ) {
				return 2;
			}
			if ( mode == "t" ) {
				gb_sendblockread(0x134,16);
				char newtitle[17];
				gb_readstring(newtitle,16);
				newtitle[16] = '\0';
				printf("Title: %s\n",newtitle);
			}
			if ( mode == "l" ) {
				printf("\n");
			}
			if ( ( mode == "r" || mode == "w" ) )  {

				vector<string> parts = split(command,' ');

			    if ( parts.size() != 3 ) {
                    printf("Unrecognised format\n");
                    continue;
			    }
				if ( mode == "r" ) {
					int addr = strtol(parts[1].c_str(), NULL, 16);
					int len= strtol(parts[2].c_str(), NULL, 16);

					gb_sendblockread(addr,len);
					for(int i=0;i<len;i++) {
						if (i>0 && i % 16 == 0) {
							if (!quietMode) printf("\n");
						}
						printf("%02x ",gb_readbyte());
					}

					if (!quietMode) printf("\n");
				}
				if ( mode == "w" ) {
					int addr = strtol(parts[1].c_str(), NULL, 16);
					int val = strtol(parts[2].c_str(), NULL, 16);
					gb_sendwrite(addr,val);
				}
			}


		}

	}
	return 0;
}

/******************************************************************************/
int main(int argc, char* argv[])
{
#ifdef _WIN32
	HINSTANCE hInpOutDll;
	hInpOutDll = LoadLibrary("inpout32.dll");
	if (hInpOutDll != NULL) {
		gfpOut32 = (lpOut32)GetProcAddress(hInpOutDll, "Out32");
		gfpInp32 = (lpInp32)GetProcAddress(hInpOutDll, "Inp32");
	} else {
		printf("Unable to load inpout32.dll\n");
		return -1;
	}
#endif
    printf(
        "GBlinkDX PC Client v0.4\n"
        "Original GBlinkdl by Brian Provinciano November 2nd, 2005 http://www.bripro.com\n"
		"Modified by taizou 2016-2024\n\n");

    if(argc < 2) {
		printf("Usage: gblinkdx \"output filename\" [options]\n"
		" Options can be: "
        "  Modes (Set only one of these)\n"
        "   -i for interactive mode\n"
		"   -q for quiet interactive mode\n"
		"   -o to override auto-detection and dump max size as standard MBC\n"
        "   If no mode set, will dump with auto-detected header values\n"
        "  Connection options\n"
        "   -pXXXX Use 4-digit hexadecimal port base address XXXX e.g. -pD010. Default is 0378\n"
        "   -x Use xboo cable compatibility mode. Default is gblink cable mode\n"
		"  Any other value will be treated as a pre-dump script filename\n"
		"                 (Scripted mode implies -o)\n\n");
        return 3;
    }

    FILE *portIni = fopen("port.ini", "r");
    if (portIni) {
        char portStr[16];
        fgets(portStr, 16, portIni);
        char *endptr;
        long port = strtol(portStr, &endptr, 16);
        if (endptr != portStr && *endptr == '\0') {
            dataPort = (unsigned short)port;
            statusPort = dataPort + 1;
            controlPort = dataPort + 2;
        }
    }

    if (argc >= 3) {
        for (int argno = 2; argno < argc; argno++) {
            if ( memcmp(argv[argno],"-x",2) == 0 ) {
                xbooCompat = true;
            } else if ( memcmp(argv[argno],"-p",2) == 0 && strlen(argv[argno]) == 6) {
                char portStr[5];
                strncpy(portStr, argv[argno]+2, 5);
                char *endptr;
                long port = strtol(portStr, &endptr, 16);
                if (endptr != portStr && *endptr == '\0') {
                    dataPort = (unsigned short)port;
                    statusPort = dataPort + 1;
                    controlPort = dataPort + 2;
                }
            }
        }
    }

    PPGBInit(dataPort, xbooCompat, 8, -1, printMessage);

    // perform communication
	printf("Waiting for Game Boy...\n");
    while(gb_sendbyte(0x9A)!=0xB4) {}

#ifdef _WIN32
    Sleep(10);
#else
    usleep(10000);
#endif

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
		return 1;
	}
	if(gb_readbyte() != 0xFF) {// verify we're done
		printf("expected 0xFF from GB, bad connection\n");
		return 1;
	}

	hdr.totalbanks = (1<<(hdr.romsize&0xF)) * 2;
	if(hdr.romsize&0xF0)
		hdr.totalbanks += (1<<((hdr.romsize>>4)&0xF)) * 2;

	printf(
		"size: %d KB\n",
		hdr.totalbanks*16
	);

	printf("\nReceiving...\n");

	U8 carttype = hdr.carttype;
	int bankscount = hdr.totalbanks;

	readBankZero();

	char* filename=argv[1];

	bool interactiveMode = false;
	bool overrideMode = false;
	bool scriptedMode = false;
	quietMode = false;
	char* scriptName;
	if (argc >= 3) {
        for (int argno = 2; argno < argc; argno++) {
            if ( memcmp(argv[argno],"-x",2) == 0 || memcmp(argv[argno],"-p",2) == 0 ) {
                continue;
            }
            if ( memcmp(argv[argno],"-o",2) == 0 )
                overrideMode = true;
            else if ( memcmp(argv[argno],"-i",2) == 0 )
                interactiveMode = true;
            else if ( memcmp(argv[argno],"-q",2) == 0 ) {
                interactiveMode = true;
                quietMode = true;
            } else {
                scriptedMode = true;
                overrideMode = true;
                scriptName = argv[argno];
            }
        }
	}

	while(1) {

		bool dump = false;

		if (interactiveMode) {
			int intret = interactive();
			if ( intret >= 1 )  {
				dump = true;
				if (intret == 2) overrideMode = false;
				else overrideMode = true;
			} else if ( intret == -1 ) {
				break;
			}
		} else {
			dump = true;
		}

		if ( dump ) {

			if (scriptedMode){
				int scretval = doKindaCrappyScriptedWrites(scriptName);
				if (scretval > 0) return scretval;
			}

			int retval= doDump(filename,overrideMode,carttype, bankscount);
			if ( retval != 0 ) return retval;
		}

		if ( interactiveMode ) {
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

    PPGBDeinit();

	printf("exiting\n");

	return 0;
}
/******************************************************************************/
