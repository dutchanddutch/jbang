#include "defs.h"
#include "map-phys.h"
#include "die.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

let static devmem() -> int
{
	let static fd = open( "/dev/mem", O_RDWR | O_DSYNC );
	if( fd < 0 )
		die( "open /dev/mem: %m\n" );
	return fd;
}

// Map physical address range.
// Tries to use same va as pa, but no guarantee.
// No alignment restrictions.
let map_phys( pa_t pa, size_t size ) -> va_t
{
	let offset = pa & 0xfff;
	pa -= offset;
	size += offset;

	size += -size & 0xfff;

	let va = (void *)pa;  // just as a suggestion to the kernel
	va = mmap( va, size, PROT_READ | PROT_WRITE, MAP_SHARED, devmem(), pa );
	if( va == MAP_FAILED )
		die( "mmap: %m\n" );

	return (va_t)va + offset;
}
