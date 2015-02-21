#pragma once
#include "defs.h"
#include "util/meta.h"
#include <array>

// While a countof() macro is simpler, this thing is actually safe from abuse.
// For example, you can't accidently use it on an array-parameter.

template< typename Array, typename = if_array_t<Array> >
let constexpr countof( Array & ) {
	return extent<Array>{};
}

// Plus, it can now be overloaded easily:

template< typename T, size_t n >
let constexpr countof( array<T,n> const volatile & ) {
	return n;
}

// (Exercise: why are the cv-qualifiers needed in the definition of countof
// for array-objects, yet not in the definition for regular arrays?)
