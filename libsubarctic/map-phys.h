#pragma once
#include "defs.h"

// flat-map physical address range
let map_phys( u32 addr, size_t size ) -> void;

template< typename T >
let static map_phys( T &obj ) -> void
{
	map_phys( (u32) &obj, sizeof obj );
}
