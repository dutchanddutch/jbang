#include "defs.h"
#include "die.h"
#include <unistd.h>
#include <sys/uio.h>

//-------------- Privileged memory access ------------------------------------//
//
// This exploits the fact that, unless CONFIG_CPU_USE_DOMAINS is set (ARMv5 and
// earlier), the kernel never uses user-mode (unprivileged) loads and stores,
// even if doing them on behalf of userspace.
//
// The defines responsible are in arch/arm/include/asm/domain.h :
//	/*
//	 * Generate the T (user) versions of the LDR/STR and related
//	 * instructions
//	 */
//	 #ifdef CONFIG_CPU_USE_DOMAINS
//	 #define TUSER(instr)    instr ## t
//	 #else
//	 #define TUSER(instr)    instr
//	 #endif
//
// I consider this to be a bug, but right now it's helpful since the control
// module does not support user-mode writes.  There are many ways to implement
// this kmemcpy() -- anything that makes the kernel write data you control to a
// location of your choice.
//
// XXX The plot thickens, it seems process_vm_readv only works for writes and
// process_vm_writev only for reads.  Can't be bothered to investigate the
// cause of this at the moment; just replaced kmemcpy() by an accessor class.
//
template< typename T >
struct PrivilegedProxy {
	static_assert( __has_trivial_copy(T), "" );

	T &target;

	constexpr PrivilegedProxy( T &target ) : target( target ) {}

	let set( T const &value ) const -> T const & {
		let dstv = iovec { &target, sizeof(T) };
		let srcv = iovec { (void *)&value, sizeof(T) };
		if( process_vm_readv( getpid(), &dstv, 1, &srcv, 1, 0 ) < 0 )
			die( "process_vm_readv: %m\n" );
		return value;
	}

	let get( T &value ) const -> T & {
		let dstv = iovec { &value, sizeof(T) };
		let srcv = iovec { (void *)&target, sizeof(T) };
		if( process_vm_writev( getpid(), &srcv, 1, &dstv, 1, 0 ) < 0 )
			die( "process_vm_writev: %m\n" );
		return value;
	}

	operator T () const {
		T value;
		return get( value );
	}

	let operator = ( T const &value ) const -> T const & {
		return set( value );
	}
};

template< typename T >
let constexpr privileged( T &target ) -> PrivilegedProxy<T> {  return target;  }
