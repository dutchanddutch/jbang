#pragma once
#include "defs.h"

let close_devmem() -> void;


// map physical address range
let map_phys( u32 pa, size_t size ) -> void *;

template< typename T >
let static map_phys( u32 pa ) -> T &
{
 	return *(T *) map_phys( pa, sizeof(T) );
}


// map physical address range over preallocated virtual memory
let map_phys( void *va, u32 pa, size_t size ) -> void;
