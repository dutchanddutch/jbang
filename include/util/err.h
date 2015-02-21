#pragma once
#include "defs.h"
#include <cerrno>

let constexpr is_err( u64 x ) -> bool { return x > (u64) -4096; }
let constexpr is_err( s64 x ) -> bool { return is_err( (u64) x ); }
let constexpr is_err( u32 x ) -> bool { return x > (u32) -4096; }
let constexpr is_err( s32 x ) -> bool { return is_err( (s64) x ); }

template< typename T >
let is_err( T const volatile *x ) -> bool { return is_err( (uintptr_t) x ); }

template< typename T >
let err_val( int err ) -> T {
	// requires is_err(err)
	return (T)(uintptr_t)err;
}
