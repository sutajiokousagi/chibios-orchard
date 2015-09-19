Orchard
======

Orchard is the internal codename for the Freescale KW-01 reference platform.
It is a Kinetis chip bonded to a radio.  Despite the name, the status
registers identify the chip as a KL17 core.

The chip contains a Cortex-M0+, which is based on ARMv6M.


Debugging and Loading Code
==========================

Kinetis chips do not seem to come with a bootloader of any sort.  The preferred
mechanism for loading code appears to be via SWD.

As development on this platform is primarily done via Novena boards, an
OpenOCD configuration file for bitbanging SWD is included.  This requires a
version of OpenOCD built from source that is newer than around 20 February 2015.

In addition to loading code, you can also debug applications running on the
board using gdb.


Starting OpenOCD
----------------

Connect Orchard to the target machine.  Here, we assume that Orchard will be
connected to UART4 on a Novena.  You will need a newer kernel that contains
properly muxed pins, as UART4's CTS and RTS are muxed as UART pins in earlier
kernels, and not GPIO pins.  For more information on the pinout, see
comments in sysfsgpio-novena.cfg.

    sudo openocd -f sysfsgpio-novena.cfg

If you're using a Raspberry Pi, use the sysfsgpio-rpi.cfg script.  The
pinout is documented in the board-specific file.

    sudo openocd -f sysfsgpio-rpi.cfg

If you're flashing boards for the first time, add '-c noinit' before the
board-specific config file.  E.g.:

    sudo openocd -c noinit -f sysfsgpio-rpi.cfg

You can then connect to OpenOCD either via telnet (to load code or do basic
debugging functions), or via gdb (in order to debug applications.)


Loading code
------------

To load code, telnet to the OpenOCD daemon and run the "program" command.  You
may program ELF files, which obviates the need to specify loading offsets:

    telnet localhost 4444
    program build/orchard.elf
    reset
    exit


Debugging
---------

To debug code, connect via GDB.  Note that GDB sometimes refuses to connect
to a device that is running, so you may need to connect first via telnet
and halt the board:

    telnet localhost 4444
    reset halt
    exit

To connect via GDB, specify the ELF file for the current program and connect
to localhost:

    gdb build/orchard.elf
    target remote localhost:3333
    fg

You can then break into the current program by hitting Control-C.  It is
possible to set breakpoints, investigate threads, and examine areas
of memory.  Two hardware breakpoints are supported, meaning you can break
somewhere in program code and then "step" or "stepi" through execution.
If you have two breakpoints set up already, you will not be able to step
through code as gdb won't be able to insert a breakpoint after the current
instruction.

You can load new code from GDB.  This will load a new program into GDB, and
will flash a new program onto the board and halt the CPU:

    load build/orchard.elf


Licensing
---------

This project is BSD 2-clause licensed.

This project includes the libc and libgcc libraries for convenience, but does
not include their source code due to the fact that we did not build these
libraries ourselves.  They are included unmodified from ARM's toolchain
labeled gcc-arm-none-eabi-4_9-2014q4, available from ARM's website.
