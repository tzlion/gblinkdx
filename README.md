GBlinkDX PC Client
==================
Based on Brian Provinciano's GBlinkdl client from Nov 2005
([archived page](http://web.archive.org/web/20070203014624/http://www.bripro.com/low/hardware/index.php?page=gblinkdl))  
Modifications by taizou 2016-2024

A small tool designed to communicate with the GBlinkdl program for Game Boy via a parallel -> GB link cable and allow
for dumping of ROMs, reverse engineering of mappers and other fun stuff.

The original version was designed for Windows, but this new version now supports Linux too.

Parallel port access should now be possible under both OSs, including modern and 64-bit versions of Windows thanks to
[inpout32.dll](http://www.highrez.co.uk/downloads/inpout32/).

HOW TO USE
==========

You will need
-------------

* A Game Boy Color (GBA probably can't do the cart swap due to hardware switching, wasn't able to get it working on a GB
  Pocket either)
* A computer running Windows or Linux with an onboard parallel port or compatible expansion card (not tested with USB
  adapters)
  * (The StarTech PEX1P2 PCIe card with AX99100 chip has been tested and works, at least under Windows) 
* A Game Boy flash cart
* The gblinkdl.gb Game Boy ROM which you can find in
  [Brian Provinciano's original GBlinkdl package](http://web.archive.org/web/20070203014624/http://www.bripro.com/low/hardware/gblinkdl/files/gblinkdl.zip)
* A GB Link -> Parallel cable, which you can build according to the schematic in the package linked above (the one
  supplied with the GB PC Linker will also work, but is vanishingly rare at this point)

Running this software
--------------------

### Linux
1. Download the source code archive from the latest release and extract
2. Open a terminal in the source code directory
3. To build in-place type `make`, or to install globally type `sudo make install`
4. Run gblinkdx as superuser (necessary for parallel port access) e.g. `sudo gblinkdx dump.gb`

### Windows
1. Download the Windows executable from the latest release - this should also come with inpout32.dll
2. Open a command prompt in the directory it's in. The first time you run it, you may need to run as administrator
   to allow inpout32 to install its driver.
3. Run gblinkdx e.g. `gblinkdx dump.gb`

### Using a custom port
If you are using a parallel port on an expansion card, you will probably need to override the port address from the
default.

To do this, save the 4-digit hexadecimal base address in a text file named port.ini and place it in the same directory
as the gblinkdx executable.

In Windows you can find this in Device Manager -> Ports (COM & LPT) -> right-click on your expansion card -> Properties
-> Resources -> I/O Range. The number at the start of the range is the base address. For example for the StarTech card
it is D010.

Linking with a Game Boy
-----------------------

1. Flash gblinkdl.gb to your flash cart
2. Connect Game Boy to PC
3. Insert flash cart in Game Boy, run gblinkdl.gb
4. When you reach the black screen saying "Run GBLinkdl.exe on your PC now".. do NOT do that yet
5. Remove the flash cart and insert the cartridge you would like to dump. (If the GB resets at this point see
   'Reset avoidance' below)
6. Run the PC client software at a command line, as detailed in "Running this software" above.
7. If the connection has worked properly you will see the GB cart's name etc appear on both the PC and Game Boy screens.

Reset avoidance
---------------

Many carts will reset the gameboy either when inserted or when you start to read from them. To circumvent this, you can
cover the reset pin, pin 30 as per this diagram. http://www.rickard.gunee.com/projects/playmobile/html/3/Image9.gif

Note that the cartridge will not boot on a Game Boy with this pin not connected, so don't do anything drastic like
severing it altogether, just stick something over it.

To avoid the need to cover the pin on every cartridge you dump, you can do this on some kind of passthrough device like
a cheat cartridge. I use an Xploder Lite for this purpose. However ensure the cheat functionality is switched OFF and
that the cheat cart acts as a direct passthrough in this state (ie the game starts directly without any menus) or you
may end up dumping the cheat cartridge's internal ROM instead!

Another option detailed on the original [GBlinkdl page] (http://web.archive.org/web/20070203014624/http://www.bripro.com/low/hardware/index.php?page=gblinkdl)
is to use a Game Genie, bypass the cheat screen and boot the GBlinkdl program, and then swap the cart while keeping the
Game Genie in the system.

Options/modes
-------------

The first parameter is always the ROM name. The second can be a few different things - examples:

**No second parameter** e.g. `gblinkdx dump.gb`:
Auto-detect cart type based on header, and dump the cartridge to dump.gb

**-o** e.g. `gblinkdx dump.gb -o`
Dump the cartridge to dump.gb as 4mb standard MBC (ignoring the header, useful for unlicensed carts with wrong headers)

**-i** e.g. `gblinkdx dump.gb -i`
Enter interactive mode, see below

**-q** e.g. `gblinkdx dump.gb -q`
Enter quiet interactive mode. This works the same as interactive mode but does not output the actions it is performing,
it only outputs data read from the Game Boy.

**Script filename** (any other value) e.g. `gblinkdx dump.gb somescript.txt`
Perform writes defined in somescript.txt before dumping (implies -o)

Interactive mode
----------------

Interactive mode allows you to perform arbitrary reads/writes and dump. Useful for reverse engineering mappers.

Supported commands:
* `r xxxx yy` reads yy bytes from address xxxx
* `w xxxx yy` writes value yy to address xxxx
* `d` dump rom as 4mb standard MBC (as in -o mode)
* `a` dump rom based on header values (as in default mode)
* `t` re-read title (useful for checking connection is still ok)
* `l` line break (useful for splitting up sequences of reads in quiet mode)
* `x` exit

Numbers entered in `r` and `w` commands must be hexadecimal

You can chain multiple commands using semicolons (no spaces before or after), for example:
`w 2000 0A;w 3080 FF;r 10AF 10;d`

Scripted mode
-------------

This is much more simplistic/crappy than interactive mode but allows you to perform a preset set of writes before a dump

Your script should be alternating lines address/value like this

```
2000
0A
3080
FF
```

Big ol caveat
-------------

There's no error checking and the software won't detect if the connection fails during dumping, so I recommend you dump
multiple times and disconnect/reconnect everything between dumps to ensure you have a good copy
