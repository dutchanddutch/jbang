# jbang
This quick and dirty hack allows the BeagleBone Black (and more generally any
other AM335x) to self-jtag without external hardware by manipulating the pinmux
registers to make the processor think the pins are toggling.

The main purpose of this is to make ICEPick assert the DBGEN signal to the
Cortex-A8, which allows the OS to use processor debug features such as
watchpoints and more generally enable "debug monitor" mode.  It would in
principle also allow halting debug to be performed on the cortex-a8 with
assistance of code running e.g. in PRUSS.

## Hardware requirements

All that is required is that all relevant JTAG inputs (nTRST, TMS, TDI, TCK)
are logic-high.  The only obstacle on the BeagleBone Black is the pull-down
resistor R25 on nTRST which needs to be removed or overridden e.g. by
connecting nTRST (P2 pin 2) to VDD_3V3B (available on P2 pin 5 "TVDD").

By default the code also expects TDO (P2 pin 7) to be connected to EMU0 (P2 pin
13) to allow readback of JTAG data. EMU0 is just used as a conveniently nearby
GPIO, you can use any other by changing src/hw-subarctic.cc.  You can also
disable TDO readback entirely by setting `has_tdo` to false in hw-subarctic.h,
in which case the demo will just blindly perform its writes.

## Software overview

The include/ dir is a bunch of common files from my baremetal projects, here
reused to directly meddle with the hardware via /dev/mem.  Most of these
registers are also managed by the kernel, so please don't view this code as a
good example.  It's gross, but it was just the easiest way for me to verify
this trick really works.

The jtag bit-banging demo itself (src/jbang.cc) is however reasonably
stand-alone and should be easily ported to any other mechanism to control the
JTAG port.

Oh, and yeah the whole thing is written in my rather eccentric style of C++.
It requires gcc 4.9 to compile, older versions will not work.  It should be
readable enough if you pretend it's some unfamiliar C++-ish language, but if
you're really trying to make sense of it, you may wish to start by reading
include/defs.h.
