#pragma once
#include "defs.h"

using pa_t = size_t;
using va_t = byte *;	// appropriate for pointer arithmetic


// map anywhere (no alignment restrictions)
let map_phys( pa_t pa, size_t size ) -> va_t;

template< typename T >
let static map_phys( pa_t pa ) -> T &
{
	return *(T *) map_phys( pa, sizeof(T) );
}
