#include "defs.h"
#include "hw-subarctic.h"
#include "map-phys.h"
#include "kmemcpy.h"
#include "ti/subarctic/prcm.h"
#include "ti/subarctic/ctrl.h"
#include "ti/subarctic/gpio.h"


//-------------- Memory-mapped peripherals -----------------------------------//
//
// prcm, to manually enable clocks (this is very wrong and rude but it'll
//		do for this proof-of-concept).
// ctrl, for padconf, where the real work is done.
// gpio for reading back TDO

Prcm &prcm = map_phys<Prcm>( 0x44e'00'000 );
Ctrl &ctrl = map_phys<Ctrl>( 0x44e'10'000 );
Io   &io3  = map_phys<Io>  ( 0x481'ae'000 );

template< typename ...Args >
let static padconf( uint pin, Args ...args )
{
	// have kernel perform a privileged write for us
	kassign( ctrl.pad[ pin ], Pad { args... } );
}


//-------------- JTAG pin i/o ------------------------------------------------//

// JTAG inputs controlled by toggling receiver-enable (pins must be pulled high
// externally or left floating to allow internal pull-up to work).
//
let static sim_input( uint pin, bool level )
{
	padconf( pin, 0u, Pad::pull_up, level ? Pad::rx_en : Pad::rx_dis );
}

// JTAG inputs controlled via padconf
let trst( bool level ) -> void {  sim_input( 120, level );  }
let tck(  bool level ) -> void {  sim_input( 119, level );  }
let tms(  bool level ) -> void {  sim_input( 116, level );  }
let tdi(  bool level ) -> void {  sim_input( 117, level );  }

// JTAG output (TDO) monitored via gpio
let tdo() -> bool {
	return has_tdo ? dev_recv( io3.in ) >> 7 & 1 : 0;
}

// I connected TDO to the nearby EMU0 pin, reconfigure it to gpio 3.7
let static tdo_init()
{
	padconf( 121, Pad::in( 7, Pad::pull_up ) );

	prcm.mod_io3.enable();
	wait_until( prcm.mod_io3.ready() );
}

// RTCK unavailable
let rtck() -> bool {  return false;  }

let hw_init() -> void
{
	prcm.mod_dbgss.enable();
	wait_until( prcm.mod_dbgss.ready() );

	if( has_tdo )
		tdo_init();
}
