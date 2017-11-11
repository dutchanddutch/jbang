#include "defs.h"
#include "map-phys.h"
#include "die.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

let static devmem_fd = -1;

let close_devmem() -> void
{
	if( devmem_fd >= 0 )
		close( devmem_fd );
	devmem_fd = -1;
}

let static devmem() -> int
{
	if( devmem_fd < 0 )
		devmem_fd = open( "/dev/mem", O_RDWR | O_DSYNC | O_CLOEXEC );
	if( devmem_fd < 0 )
		die( "open /dev/mem: %m\n" );
	return devmem_fd;
}

// map physical address range
let map_phys( uintptr_t pa, size_t size ) -> void *
{
	size += -size & 0xfff;
	let va = mmap( (void *)pa, size, PROT_READ | PROT_WRITE,
				MAP_SHARED, devmem(), pa );
	if( va == MAP_FAILED )
		die( "map_phys(0x%x, 0x%x): mmap: %m\n", pa, size );
	return va;
}

// map physical address range over preallocated virtual memory
let map_phys( void *va, uintptr_t pa, size_t size ) -> void
{
	if( size & 0xfff )
		die( "map_phys(%p, 0x%x, 0x%x): Invalid size\n", va, pa, size );
	if( mmap( va, size, PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_FIXED, devmem(), pa )
			== MAP_FAILED )
		die( "map_phys(%p, 0x%x, 0x%x): mmap: %m\n", va, pa, size );
}
