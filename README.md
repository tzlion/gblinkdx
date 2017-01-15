GBlinkDX PC Client
==================
Based on Brian Provinciano's GBlinkdl client from Nov 2005
([archived page](http://web.archive.org/web/20070203014624/http://www.bripro.com/low/hardware/index.php?page=gblinkdl))  
Modifications by taizou 2016-2017

A small tool designed to communicate with the GBlinkdl program for Game Boy via a parallel -> GB link cable and allow
for dumping of ROMs, reverse engineering of mappers and other fun stuff.

The original version was designed for Windows, but under modern version of Windows - ESPECIALLY 64-bit - it is pretty
hard to get raw parallel port access, so this new version has been developed primarily for Linux instead.

I have tried to maintain the Windows compatibility too and have compiled a Windows executable which should work at least
in 32-bit XP, but no guarantees since I've been unable to test it under this OS.

HOW TO USE
==========

You will need
-------------

* A Game Boy Color (GBA probably can't do the cart swap due to hardware switching, wasn't able to get it working on a GB
  Pocket either)
* A computer with an onboard parallel port (not tested with PCI cards or USB adapters)
* An OS running on the computer which will give you direct parallel port access - e.g Linux or Windows XP, maybe later
  32-bit Windows versions with some third-party drivers installed (untested)
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
1. Install [Visual C++ Redistributable 2015](https://www.microsoft.com/en-us/download/details.aspx?id=48145) if you
   don't already have it
2. Download the Windows executable from the latest release
3. Open a command prompt in the directory it's in
4. Run gblinkdx e.g. `gblinkdx dump.gb`

* If you would like to compile this program as-is for Windows, you will need to use some version of Visual C++ as it
  uses VC-specific ASM syntax for parallel port access.

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

Options/modes
-------------

The first parameter is always the ROM name. The second can be a few different things - examples:

**No second parameter** e.g. `gblinkdx dump.gb`:
Auto-detect cart type based on header, and dump the cartridge to dump.gb

**-o** e.g. `gblinkdx dump.gb -o`
Dump the cartridge to dump.gb as 4mb standard MBC (ignoring the header, useful for unlicensed carts with wrong headers)

**-i** e.g. `gblinkdx dump.gb -i`
Enter interactive mode, see below

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
