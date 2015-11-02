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

// flat-map physical address range
let map_phys( uintptr_t addr, size_t size ) -> void
{
	size += addr & 0xfff;
	addr -= addr & 0xfff;
	size += -size & 0xfff;
	if( mmap( (void *)addr, size, PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_FIXED, devmem(), addr )
			== MAP_FAILED )
		die( "mmap: %m\n" );
}
