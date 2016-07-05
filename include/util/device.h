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
[[ gnu::always_inline ]]
let inline barrier()
{
	asm volatile ( "" ::: "memory" );
}

// full memory barrier
[[ gnu::always_inline ]]
let inline barrier_sync()
{
	asm volatile ( "dsb" ::: "memory" );
}

// full compiler barrier, limited to specified object
template< typename T >
[[ gnu::always_inline ]]
let inline barrier( T &target )
{
	asm volatile ( "" : "+m"(target) );
}

// full compiler barrier, limited to specified objects
template< typename T, typename ...Ts >
[[ gnu::always_inline ]]
let inline barrier( T const &target, Ts const &...targets )
{
	barrier( target );
	barrier( targets... );
}

// compiler write barrier, limited to specified object
template< typename T >
[[ gnu::always_inline ]]
let inline write_barrier( T const &target )
{
	asm volatile ( "" :: "m"(target) );
}

// compiler write barrier, limited to specified objects
template< typename T, typename ...Ts >
[[ gnu::always_inline ]]
let inline write_barrier( T const &target, Ts const &...targets )
{
	write_barrier( target );
	write_barrier( targets... );
}

// send a value to a write-only register
// XXX deprecated
template< typename T >
let inline dev_send( T volatile &reg, T val ) {
	reg = val;
	barrier( reg );
}

// read a value from a read-only register
// XXX deprecated
template< typename T >
let inline dev_recv( T volatile &reg ) -> T {
	let tmp = reg;
	barrier( reg );
	return tmp;
}

// render a pointer value opaque to the optimizer
template< typename T >
[[ gnu::always_inline, gnu::const ]]
let inline conceal( T *p ) -> T *
{
	asm( "" : "+r"(p) );
	return p;
}

// render a reference value opaque to the optimizer
template< typename T >
[[ gnu::always_inline, gnu::const ]]
let inline conceal( T &m ) -> T &
{
	return *conceal( &m );
}


//============== Memory-mapped I/O types =====================================//
//
// Note that I try use these only when reads have side-effects or when writes
// have side-effects that cannot be explained as being the result of changing
// the register's value.  Otherwise, a limited barrier probably suffices.
//
// Peripherals which are fussy about access types often also need volatile
// slapped on, although I really wish there was a better alternative.
//
// I'm just trying different approaches to see which is more reusable and
// least disgusting...


// Common base class
//
template< typename T >
struct IoRegBase {
	// TODO right now it really only works for scalars in practice

	static_assert( is_trivially_copyable<T>{}, "" );

	IoRegBase( IoRegBase<T> const & ) = delete;

private:
	T volatile _io;

protected:
	let read() -> T {
		return _io;
	}
	let write( T val ) -> T {
		_io = val;
		return val;
	}
};


// I/O registers.  For now they emulate the type 'T volatile' itself, but they
// also have explicit read and write methods.
//
template< typename T >
struct IoReg : IoRegBase<T> {
	using IoRegBase<T>::read;
	using IoRegBase<T>::write;

	// compatibility stuff begins here

	operator T () volatile {  return read();  }

	let operator = ( T value ) volatile -> T {  return write( value );  }
};

using io_u8  = IoReg<u8>;
using io_u16 = IoReg<u16>;
using io_u32 = IoReg<u32>;
using io_u64 = IoReg<u64>;


// Read-only registers, callable objects with signature () -> T
//
template< typename T >
struct InReg : IoRegBase<T> {
	using IoRegBase<T>::read;

	let operator () () -> T {  return read();  }
};

using in_u8  = InReg<u8>;
using in_u16 = InReg<u16>;
using in_u32 = InReg<u32>;
using in_u64 = InReg<u64>;


// Write-only registers, callable objects with signature ( T ) -> T
//
template< typename T >
struct OutReg : IoRegBase<T> {
	using IoRegBase<T>::write;

	let operator () ( T val ) -> T {  return write( val );  }
};

using wo_u8  = OutReg<u8>;
using wo_u16 = OutReg<u16>;
using wo_u32 = OutReg<u32>;
using wo_u64 = OutReg<u64>;


template< u32 value >
struct CmdReg : wo_u32 {
	let operator () () -> void {  self( value );  }
};


template< typename T >
struct EvReg : IoRegBase<T> {

	// contains events: set by hardware, cleared by writing 1

private:
	using IoRegBase<T>::read;
	using IoRegBase<T>::write;

public:
	// full-register operations
	let check()         -> T {  return read();  }
	let clear( T bits ) -> T {  return write( bits );  }
	let take()          -> T {  return clear( check() );  }

	// masked operations
	let check( T bits ) -> T {  return check() & bits;  }
	let take(  T bits ) -> T {  return clear( check( bits ) );  }
};
