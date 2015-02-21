#include "defs.h"
#include "die.h"
#include <unistd.h>
#include <sys/uio.h>

//-------------- Kernel-mode (privileged) memcpy -----------------------------//
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
let inline kmemcpy( void *dst, void const *src, size_t len )
{
	let static pid = getpid();
	let dstv = iovec { dst, len };
	let srcv = iovec { (void *)src, len };
	if( process_vm_readv( pid, &dstv, 1, &srcv, 1, 0 ) < 0 )
		die( "kmemcpy: %m\n" );
}

template< typename T >
let inline kassign( T &dst, T const &src )
{
	kmemcpy( (void *)&dst, (void *)&src, sizeof(T) );
}
