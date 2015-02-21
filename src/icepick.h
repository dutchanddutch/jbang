#include "defs.h"

namespace icepick {

let constexpr ir_len = 6;

enum {
	// read-only registers
	ir_bypass	= 0b111111,  // 1 bit
	ir_idcode	= 0b000100,  // 32 bits
	ir_icepickid	= 0b000101,  // 32 bits
	ir_usercode	= 0b001000,  // 32 bits

	// pass-through to boundary scan, recommended ir codes
	ir_clamp	= 0b011101,  // bypass
	ir_highz	= 0b011110,  // bypass
	ir_runbist	= 0b011010,  // bist
	ir_sample	= 0b011011,  // boundary
	ir_preload	= 0b011100,  // boundary
	ir_intest	= 0b011001,  // boundary
	ir_extest	= 0b011000,  // boundary
	ir_extest_nopull= 0b010111,  // boundary
	ir_extest_pulse	= 0b100100,  // boundary
	ir_extest_train	= 0b100101,  // boundary

	// icepick services (public connect required for remaining IRs)
	ir_pub_connect	= 0b000111,  // 8-bit (3 -> 4 bit indirect rw)
	ir_priv_connect	= 0b011111,  // 1-bit
	ir_router	= 0b000010,  // 32-bit (7 -> 24 bit indirect rw)
};

} // namespace icepick
