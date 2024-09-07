/**
 * PPGB v0.1.0
 * Parallel port communication with Game Boy consoles
 * by taizou, based on code from gblinkdl by Brian Provinciano
 */

#ifndef PPGB_H
#define PPGB_H

typedef void(*PPGBPrint)(const char*);

/**
 * Initialise the port. Must be called before transferring any data
 * @param basePort Base port address, standard onboard parallel port is 0x378
 * @param xbooCable Set to 1 if using an Xboo cable or 0 if using a GBlink cable
 * @param minDelay Minimum delay per byte (in number of port accesses) - 2 seems to be good for GBA and 8 for GB
 * @param maxDelay Maximum delay per byte, set to -1 for no max
 * Actual delay will be determined automatically between min and max. Set them both the same for a specific delay
 * @param printFunction function to log port initialisation info, set to null for no logging
 * @return 1 if success, -1 if failed
 */
int PPGBInit(unsigned short basePort, int xbooCable, int minDelay, int maxDelay, PPGBPrint printFunction);

/**
 * Deinitialise the port. Probably not super necessary unless you're going to initialise it again afterwards
 */
void PPGBDeinit();

/**
 * Transfer a single byte to/from the GB
 * @param value Value to send. If you only want to read, send 00
 * @return Value received from the GB
 */
unsigned char PPGBTransfer(unsigned char value);

/**
 * Just get the raw state of the GB's serial out
 * @return 0 or 1
 */
unsigned char PPGBRawOutputRead();

#endif //PPGB_H
