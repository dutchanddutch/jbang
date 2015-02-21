#pragma once
#include "defs.h"


let hw_init() -> void;


//-------------- JTAG pin i/o ------------------------------------------------//

// control JTAG inputs
let trst( bool out ) -> void;
let tck(  bool out ) -> void;
let tms(  bool out ) -> void;
let tdi(  bool out ) -> void;

// monitor JTAG output
let tdo() -> bool;
let rtck() -> bool;

let constexpr has_tdo = true;
let constexpr has_rtck = false;


//-------------- Debug hw config ---------------------------------------------//

constexpr u32 idcode_mask  = 0x0'ffff'fff;
constexpr u32 idcode_match = 0x0'b944'02f;

// initialization of icepick registers
constexpr u32 icepick_init_regs[] = {
	0x60'002000,  // assert cortex-a8 DBGEN
	0x2c'002100,  // link DAP into chain (takes effect at run)
};

// address of cortex-a8 debug regs on debug APB
constexpr u32 a8_debug = 0x800'01'000;
