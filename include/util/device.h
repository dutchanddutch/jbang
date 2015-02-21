#pragma once
#include "defs.h"


//============== Here be dragons =============================================//
//
// Ugly macros first, so you know they exist.

#define wait_while( condition )   do barrier(); while( condition )
#define wait_until( condition )   do barrier(); while( !( condition ))
//
// Although busy-waiting on a device sucks, sometimes there's simply no usable
// alternative.  Note that they always act as a full compiler-barrier before
// testing the condition.  This is makes things like
//
//	dev.reset = true;
//	wait_while( dev.reset );
//
// work without requiring dev.reset to be made volatile.


//============== Types =======================================================//
//
// memory-mapped I/O types
//
// "in_" and "wo_" indicate read-only and write-only registers.  I used to
// declare the read-only registers as const volatile but that sometimes
// triggered weird compiler errors.
//
// I really wish I could better express this to the compiler.
//
// Note that I use these only when reads have side-effects or when writes have
// side-effects that cannot be explained as being the result of changing the
// register's value.  Otherwise, a limited barrier probably suffices.

using io_s32 = s32 volatile;
using in_s32 = s32 volatile;
using wo_s32 = s32 volatile;

using io_u32 = u32 volatile;
using in_u32 = u32 volatile;
using wo_u32 = u32 volatile;

using io_u16 = u16 volatile;
using in_u16 = u16 volatile;
using wo_u16 = u16 volatile;

using io_u8  = u8  volatile;
using in_u8  = u8  volatile;
using wo_u8  = u8  volatile;


//============== Barriers ====================================================//
//
// And other measures to force the optimizer to enhance its calm.

// compiler barriers suffice for:
//	synchronization with other code on the same core (including interrupts)
//	maintaining ordering of accesses to a single peripheral, assuming the
//		memory type of its mapping is "device" or "strongly ordered"
//
// Maintaining ordering of accesses to non-cacheable normal memory requires
// genuine memory barriers.  Achieving a data sync barrier with a peripheral
// outside the coherency domain can often be done with a dummy read, but in
// general you'd better know the interconnect's topology and behaviour if you
// want to have confidence in some synchronization procedure, especially when
// multiple peripherals become involved.
//

// full compiler barrier
[[ gnu::always_inline, gnu::artificial ]]
let inline barrier()
{
	asm volatile ( "" ::: "memory" );
}

// full memory barrier
[[ gnu::always_inline, gnu::artificial ]]
let inline barrier_sync()
{
	asm volatile ( "dsb" ::: "memory" );
}

// full compiler barrier, limited to specified object
template< typename T >
[[ gnu::always_inline, gnu::artificial ]]
let inline barrier( T &target )
{
	asm volatile ( "" : "+m"(target) );
}

// compiler write barrier, limited to specified object
template< typename T >
[[ gnu::always_inline, gnu::artificial ]]
let inline write_barrier( T const &target )
{
	asm volatile ( "" :: "m"(target) );
}

// send a value to a write-only register
template< typename T >
let inline dev_send( T volatile &reg, T val ) {
	reg = val;
	barrier( reg );
}

// read a value from a read-only register
template< typename T >
let inline dev_recv( T volatile &reg ) -> T {
	let tmp = reg;
	barrier( reg );
	return tmp;
}

// render a pointer value opaque to the optimizer
template< typename T >
[[ gnu::always_inline, gnu::artificial, gnu::const ]]
let inline conceal( T *p ) -> T *
{
	asm( "" : "+r"(p) );
	return p;
}

// render a reference value opaque to the optimizer
template< typename T >
[[ gnu::always_inline, gnu::artificial, gnu::const ]]
let inline conceal( T &m ) -> T &
{
	return *conceal( &m );
}
