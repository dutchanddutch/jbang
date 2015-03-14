#pragma once
#include "defs.h"
#include "util/device.h"
#include "ti/irqd.h"

struct alignas(0x400) Io {

//===== new OCP wrapper =====//
//
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


//===== wrapped peripheral =====//

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

/*134*/	u32 dir;	//rw  0=output, 1=input
/*138*/	u32 in;		//r-  received level
/*13c*/	u32 out;	//rw  driven level (if output)
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

alignas(0x10)
/*190*/	wo_u32 _clear;	//-c
/*194*/	wo_u32 _set;	//-s

	let set( u32 pins, bool value = true ) -> Io & {
		dev_send( value ? _set : _clear, pins );
		barrier( out );
		return self;
	}

	let clear( u32 pins ) -> Io & {
		return set( pins, false );
	}

	let highz( u32 pins ) -> Io & {
		dir |= pins;
		return self;
	}

	let drive( u32 pins ) -> Io & {
		write_barrier( out );
		dir &= ~pins;
		return self;
	}

	let drive( u32 pins, bool value ) -> Io & {
		return set( pins, value ).drive( pins );
	}
};
