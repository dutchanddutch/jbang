#pragma once
#include "defs.h"

// map physical address range.
// there are no alignment restrictions on pa and size.
let map_phys( uintptr_t pa, size_t size, bool readonly = false ) -> void *;

template< typename T >
let static inline map_phys( uintptr_t pa, bool readonly = false ) -> T &
{
	return *(T *) map_phys( pa, sizeof(T), readonly );
}


// map physical address range over preallocated virtual memory.
// va, pa, and size must all be page-aligned.
let map_phys( void *va, uintptr_t pa, size_t size, bool readonly = false ) -> void;
