#pragma once
#include "defs.h"

// Phys<T> is the type of an object of type T located in physical memory.
//
// By default, assume the linker will resolve the symbols to their appropriate
// address in freestanding environments while non-trivial work is required in
// hosted environments.

#if __STDC_HOSTED__
template< typename T >
using Phys = T &;
#else
template< typename T >
using Phys = T;
#endif
