#include "defs.h"
#include "die.h"
#include "icepick.h"
#include "dap.h"
#include "hw-subarctic.h"
#include <stdio.h>
#include <unistd.h>

// For completeness I defined some utility functions that are currently unused
#pragma GCC diagnostic ignored "-Wunused-function"


//-------------- JTAG protocol -----------------------------------------------//
//
// bit-banging JTAG via padconf is so slow that we really don't need to bother
// inserting any explicit setup/hold time delays...

let constexpr jtag_verbose = false;

let static tck_pulse()
{
	// <setup time for TMS/TDI>
	tck( 1 );
	// <hold time for TMS/TDI>
	tck( 0 );
	// <delay until output data valid>
}

let static cmd( uint nbits, uint data )
{
	forseq( i, 0, nbits ) {
		tms( data >> i & 1 );
		tck_pulse();
	}
}

enum class State {
	rst,
	commit,
	run,
	data,
//	pause,  // not used
};

let static state = State::rst;

let static commit()
{
	if( state == State::data ) {
		cmd( 2, 0b11 );
		state = State::commit;
		if( jtag_verbose ) printf( ".\n" );
	}
}

let static run( uint ncycles = 1 ) {
	commit();
	cmd( ncycles, 0 );
	state = State::run;
	if( jtag_verbose ) printf( "run <%u>\n", ncycles );
}

let static dr()  {
	commit();
	cmd( 2, 0b01 );
	state = State::data;
	if( jtag_verbose ) printf( "dr " );
}

let static ir()  {
	commit();
	cmd( 3, 0b011 );
	state = State::data;
	if( jtag_verbose ) printf( "ir " );
}


let static xfer( uint nbits ) -> uint
{
	uint in = 0;
	forseq( i, 0, nbits ) {
		tck_pulse();
		in |= tdo() << i;
	}
	if( jtag_verbose ) printf( "<%u> 0x%x ", nbits, in );
	return in;
}

let static xfer( uint nbits, uint out ) -> uint
{
	uint in = 0;
	forseq( i, 0, nbits ) {
		tck_pulse();
		tdi( out >> i & 1 );
		in |= tdo() << i;
	}
	if( jtag_verbose ) printf( "<%u> 0x%x / 0x%x ", nbits, in, out );
	return in;
}

let static dr( uint nbits, uint out = 0 ) {
	dr();
	let in = xfer( nbits, out );
	commit();
	return in;
}

let static ir( uint nbits, uint out ) {
	ir();
	let in = xfer( nbits, out );
	commit();
	return in;
}

let static jtag_reset()
{
	trst( 0 );
	tck( 0 );
	tdi( 1 );
	cmd( 5, 0b11111 );
	if( jtag_verbose ) printf( "reset\n" );
}

let static jtag_init()
{
	jtag_reset();

	trst( 1 );
	run( 100 );

	if( ! has_tdo )
		return;

	let idcode = dr( 32 );
	printf( "JTAG ID: %08x\n", idcode );
	if( ( idcode & idcode_mask ) != idcode_match )
		die( "Device not recognized" );
}


//-------------- ICEPick-C/D -------------------------------------------------//

#if 0
let static icepick_check( uint reg) -> u32
{
	let x = dr( 32, 0 );
	if( ( x >> 24 ) != reg )
		die( "icepick error" );
	return x & 0xffffff;
}

let static icepick_read( uint reg ) -> u32
{
	dr( 32, 0 << 31 | reg << 24 );
	return icepick_check( reg );
}

let static icepick_write( uint reg, u32 data ) -> u32
{
	dr( 32, 1 << 31 | reg << 24 | ( data & 0xffffff ) );
	return icepick_check( reg );
}

let static icepick_dump( uint reg ) -> u32
{
	let x = icepick_read( reg );
	printf( "icepick [%02x]: %06x\n", reg, x );
	return x;
}
#endif

let static icepick_init()
{
	ir( icepick::ir_len, icepick::ir_pub_connect );
	dr( 8, 0b1'000'1001 );
	if( has_tdo && dr( 8 ) != 0b1001 )
		die( "icepick connect failed" );

	ir( icepick::ir_len, icepick::ir_router );

	for( let x : icepick_init_regs ) {
		dr( 32, x | 1 << 31 );
		if( has_tdo && dr( 32 ) >> 24 != x >> 24 )
			die( "icepick write error" );
	}

	ir( icepick::ir_len, icepick::ir_bypass );
	run( 16 );
}


//-------------- ARM Debug Access Port (DAP) ---------------------------------//

let static dap_last_ir = (uint) dap::ir_idcode;

let static dap_ir( uint reg )
{
	// avoid doing an IR-scan for _every_ dap op, that would be silly.
	if( reg == dap_last_ir )
		return;
	dap_last_ir = reg;

	ir();
	xfer( dap::ir_len, reg );
	xfer( icepick::ir_len, icepick::ir_bypass );
	commit();
}

let static dap_op( uint ir, uint op, u32 arg ) -> u32
{
	dap_ir( ir );
	dr();
	let stat = xfer( 3, op );
	if( has_tdo && stat != 0b010 )
		die( "DAP status code 0b%03b\n", stat );
	let res = xfer( 32, arg );
	xfer( 1 );	// icepick in bypass
	run();		// not always needed, but doesn't hurt
	return res;	// response data (if any) of _previous_ dap op
}

let static dp_abort()       {         dap_op( dap::ir_abort, 0b000, 1 );  }
let static dp_csw( u32 x )  {  return dap_op( dap::ir_dpacc, 0b010, x );  }
let static dp_csw()         {  return dap_op( dap::ir_dpacc, 0b011, 0 );  }
let static dp_sel( u32 x )  {  return dap_op( dap::ir_dpacc, 0b100, x );  }
let static dp_nop()         {  return dap_op( dap::ir_dpacc, 0b110, 0 );  }

let static ap_csw( u32 x )  {  return dap_op( dap::ir_apacc, 0b000, x );  }
let static ap_csw()         {  return dap_op( dap::ir_apacc, 0b001, 0 );  }
let static ap_addr( u32 x ) {  return dap_op( dap::ir_apacc, 0b010, x );  }
let static ap_addr()        {  return dap_op( dap::ir_apacc, 0b011, 0 );  }
let static ap_data( u32 x ) {  return dap_op( dap::ir_apacc, 0b110, x );  }
let static ap_data()        {  return dap_op( dap::ir_apacc, 0b111, 0 );  }

let static dap_check() -> u32
{
	let data = dp_csw();
	let csw = dp_nop();
	if( has_tdo && csw != 0xf0000000 )
		die( "DP-CSW unexpected: %08x\n", csw );
	return data;
}

let static dap_init()
{
	if( has_tdo ) {
		let idcode = dr( 32 );
		printf( "DAP JTAG ID: %08x\n", idcode );
		if( idcode != 0x3ba00477 )
			die( "Device not recognized" );
	}

	// power up and clear errors
	dp_csw( 0x50000032 );
	dap_check();

	// select and configure APB-AP
	dp_sel( 1 << 24 );
	ap_csw( 0xe3000012 );
}

let static ap_read( u32 addr ) -> u32
{
	ap_addr( addr );
	ap_data();
	let data = dap_check();
	if( has_tdo )
		printf( "read 0x%08x -> 0x%08x\n", addr, data );
	return data;
}

let static ap_write( u32 addr, u32 data )
{
	ap_addr( addr );
	ap_data( data );
	return dap_check();
}


//-------------- ARM CoreSight -----------------------------------------------//

let static show_auth_status( u32 addr )
{
	constexpr char const *privs[] = {
		"public invasive debug",
		"public non-invasive debug",
		"secure invasive debug",
		"secure non-invasive debug",
	};

	let x = ap_read( addr + 0xfb8 );

	for( let s : privs ) {
		if( x & 1 )
			printf( "%s: granted\n", s );
		else if( x & 2 )
			printf( "%s: denied\n", s );
		x >>= 2;
	}
}


//-------------- debug communication channel ---------------------------------//

let static dbg_status()  // debugger -> core
{
	u32 status;
	asm( "mrc p14, 0, %0, c0, c1, 0" : "=r"(status) );
	return status;
}

let static dbg_rx()  // debugger -> core
{
	u32 data;
	asm volatile( "mrc p14, 0, %0, c0, c5, 0" : "=r"(data) );
	return data;
}


//-------------- main --------------------------------------------------------//

let main() -> int
{
	hw_init();
	jtag_init();
	icepick_init();
	dap_init();

	if( has_tdo )
		show_auth_status( a8_debug );

	ap_read( a8_debug + 0x314 );  // clear power/reset status bits
	ap_read( a8_debug + 0x088 );  // clear debug comm bits
	let pid = (u32) getpid();
	printf( "our pid: %d\n", pid );
	ap_write( a8_debug + 0x080, pid );
	usleep( 1000 );
	printf( "our pid via scenic route: %d\n", dbg_rx() );

	return 0;
}
