#include "defs.h"
#include "map-phys.h"
#include "die.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

let static map_phys( void *va, uintptr_t pa, size_t size, bool readonly, int flags )
{
	let fd = open( "/dev/mem", (readonly ? O_RDONLY : O_RDWR) | O_DSYNC | O_CLOEXEC );
	if( fd < 0 )
		return MAP_FAILED;
	let prot = readonly ? PROT_READ : (PROT_READ | PROT_WRITE);
	va = mmap( va, size, prot, flags, fd, pa );
	close( fd );
	return va;
}


// map physical address range.
// there are no alignment restrictions on pa and size.
let map_phys( uintptr_t pa, size_t size, bool readonly ) -> void *
{
	let offset = pa & 0xfff;
	pa -= offset;
	size += offset;
	size += -size & 0xfff;
	let va = map_phys( (void *)pa, pa, size, readonly, MAP_SHARED );
	if( va == MAP_FAILED )
		die( "map_phys(0x%x, 0x%x): mmap: %m\n", pa, size );
	return (char *)va + offset;
}

// map physical address range over preallocated virtual memory.
// va, pa, and size must all be page-aligned.
let map_phys( void *va, uintptr_t pa, size_t size, bool readonly ) -> void
{
	if( size & 0xfff )
		die( "map_phys(%p, 0x%x, 0x%x): Invalid size\n", va, pa, size );
	if( pa & 0xfff )
		die( "map_phys(%p, 0x%x, 0x%x): Invalid physical address\n", va, pa, size );
	if( (uintptr_t)va & 0xfff )
		die( "map_phys(%p, 0x%x, 0x%x): Invalid virtual address\n", va, pa, size );
	if( map_phys( va, pa, size, readonly, MAP_SHARED | MAP_FIXED ) == MAP_FAILED )
		die( "map_phys(%p, 0x%x, 0x%x): mmap: %m\n", va, pa, size );
}
