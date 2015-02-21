#include "defs.h"

namespace dap {

let constexpr ir_len = 4;

enum {
	// read-only registers
	ir_bypass	= 0b1111,
	ir_idcode	= 0b1110,

	// dap commands
	ir_abort	= 0b1000,
	ir_dpacc	= 0b1010,
	ir_apacc	= 0b1011,
};

enum {
	// ir_abort
	dp_abort	= 0x0,

	// ir_dpacc
	dp_rd_idr	= 0x1,	// DPv1 and later
	dp_wr_csw	= 0x2,
	dp_rd_csw	= 0x3,
	dp_wr_sel	= 0x4,
	dp_rd_sel	= 0x5,	// DPv0 only (deprecated)
	dp_wr_null	= 0x6,
	dp_rd_null	= 0x7,
};

enum {
	// ir_apacc
	ap_wr_csw	= 0x0,
	ap_rd_csw	= 0x1,
	ap_wr_addr	= 0x2,
	ap_rd_addr	= 0x3,
	ap_wr_data	= 0x6,
	ap_rd_data	= 0x7,
};

} // namespace dap
