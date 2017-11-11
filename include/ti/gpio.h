#pragma once
#include "defs.h"
#include "util/device.h"
#include "ti/irqd.h"

// OMAP4 GPIO controller
// (based on OMAP2 GPIO controller with a wrapper)

struct alignas(0x400) Io {

	//-------- Highlander-compliant wrapper ------------------------------//

/*000*/	u32 ident;	//r-  5'060'08'01

alignas(0x10)
/*010*/	u32 sysconfig;
	// bit   0	rw  autoidle (adds 1 OCP cycle to data in latency)
	// bit   1	-x  reset
	// bit   2	rw  wakeup enabled
	// bits  3- 4	rw  idlemode (all modes supported)

//	u32 sysstatus is at offset 0x114

	// irq handling:
	//	32 inputs (one per GPIO line)
	//	2 outputs (each actually produces a pulsed and a level output)
alignas(0x20)
/*020*/	wo_u32 eoi;	//->  write irq output number
/*024*/	Irqd4w<u32[2]> irq;

	// Notes on the pulsed irq outputs:
	//
	// It seems eoi register doesn't work, but eoi is implicit when
	// clearing any enabled irq bits (even if not set), for example:
	//
	//	(assuming all bits are initially clear and enabled)
	//
	//	set bit 0	-> output pulsed
	//	set bit 1
	//	set bit 2
	//	clear bit 0	-> output pulsed
	//	clear bit 1	-> output pulsed
	//	clear bit 1	-> output pulsed (even though it wasn't set)
	//	clear bit 2
	//
	// Beware however that
	//	- disabling an irq doesn't trigger eoi
	//	- clearing a disabled irq doesn't trigger eoi
	//	- eoi isn't required to pulse again on rising edge of level irq


	// AM335x TRM mentions writing 0 to eoi is required after DMA event.


	//-------- original module -------------------------------------------//

alignas(0x100)
/*100*/	u32 _old_ident;

alignas(0x10)
/*110*/	u32 _old_sysconfig;
/*114*/	u32 sysstatus;
	// bit   0	r-  reset done

/*118*/	Irqdw<u32> _old_irq0;
/*124*/	u32 _124;
/*128*/	Irqd<u32> _old_irq1;

/*130*/	u32 control;
	// bit   0	rw  functional clock gated
	// bits  1- 2	rw  log2( ick -> fck divider )

/*134*/	u32 _highz;	//rw  0=driver enabled, 1=driver disabled
/*138*/	u32 in;		//r-  received level
/*13c*/	u32 _out;	//rw  driven level (if output)
/*140*/	u32 irq_low;	//rw  detect low level
/*144*/	u32 irq_high;	//rw  detect high level
/*148*/	u32 irq_rise;	//rw  detect rising edge
/*14c*/	u32 irq_fall;	//rw  detect falling edge
/*150*/	u32 debounce;	//rw  debounce enabled

/*154*/	u32 debounce_time;  //rw  in dbck-cycles (32768 Hz), max 255

alignas(0x10)
/*160*/	wo_u32 _old_irq0_disable;	//-c
/*164*/	wo_u32 _old_irq0_enable;	//-s
alignas(0x10)
/*170*/	wo_u32 _old_irq1_disable;	//-c
/*174*/	wo_u32 _old_irq1_enable;	//-s
alignas(0x10)
/*180*/	wo_u32 _old_wake_disable;	//-c
/*184*/	wo_u32 _old_wake_enable;	//-s

  union {
alignas(0x10)
/*190*/	wo_u64 _setclear;	//-x
    struct {
/*190*/	wo_u32 _clear;		//-c
/*194*/	wo_u32 _set;		//-s
    };
  };

	// Output value/driver getters
	//
	let out() -> u32 {  return _out;  }
	let highz() -> u32 {  return _highz;  }

	// Output value setters
	//
	// These use atomic ops in the GPIO module to change the output value,
	// making them race-free even across different initiators.
	//
	// It also makes them fast by avoiding a device-read.
	//
	// XXX I'm not 100% sure setclear will be race-free against other
	// initiators, although it will be locally.
	//
	let setclear( u32 setbits, u32 clearbits ) -> Io & {
		write_barrier( _highz, _out );
		if( __builtin_constant_p( clearbits == 0 ) && clearbits == 0 )
			_set( setbits );
		else if( __builtin_constant_p( setbits == 0 ) && setbits == 0 )
			_clear( clearbits );
		else
			_setclear( clearbits | (u64) setbits << 32 );
		barrier( _out );
		return self;
	}
	let set( u32 bits ) -> Io & {
		return setclear( bits, 0 );
	}
	let clear( u32 bits ) -> Io & {
		return setclear( 0, bits );
	}
	let out( u32 bits, bool value ) -> Io & {
		return value ? set( bits ) : clear( bits );
	}
	let out( u32 bits, u32 value ) -> Io & {
		return setclear( value & bits, ~value & bits );
	}

	// Output driver setters
	//
	// These require read-modify-update and are therefore NOT atomic.
	//
	// Use of atomic ops to provide cpu-local safety doesn't work either:
	// the exclusive load is exported to the interconnect and results in
	// a bus error.
	//
	let highz( u32 bits ) -> Io & {
		_highz |= bits;
		return self;
	}
	let drive( u32 bits ) -> Io & {
		write_barrier( _out );
		_highz &= ~bits;
		return self;
	}
	template< typename T >
	let drive( u32 bits, T value ) -> Io & {
		return out( bits, value ).drive( bits );
	}
};
