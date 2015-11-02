#pragma once
#include "config/init.h"
#include "util/meta.h"
#include <cstddef>
#include <cstdint>

// Remove pollution that compiler or standard headers may have introduced.
#undef linux		// __linux__
#undef unix		// __unix__

using namespace std;


//============== Here be dragons =============================================//
//
// Ugly macros first, so you know they exist.

#undef NULL
#define NULL nullptr
// duh.

#define self (*this)
// It's very obvious that 'this' being a pointer rather than a reference is
// purely historical baggage and a mistake in hindsight.


#define let auto
//
// The intention here is best explained by example:
//
//	let i = 42;
//	for( let &x : things ) { ... }
//	let init_frobs() -> void;
//
// 'auto' still has its uses in the rare situations where 'let' really makes no
// sense, e.g. decltype( auto ).  For consistency's sake, I do still stretch
// the applicability of 'let' to declaring functions types like:
//
//	using int_eater_t = let ( int ) -> void;
//
// If possible, add a custom syntax highlighting rule to your editor that
// highlights 'let' as statement and 'auto' as type.


#define alignas(x)	__attribute__((aligned( x )))
//
// Override C++ built-in alignas() with one that _can_ also be applied to types
// instead of being limited to variables.  The downsize is that the argument
// must be numerical, a type is not allowed.
//
// Also, can't use the new attribute syntax [[ gnu::aligned( x ) ]] here.
// It breaks in gcc when alignment > 64 bytes, wtf...

#define packed_alignas( x ) __attribute__(( packed, aligned( x ) ))
//
// Same story, and can't combine C++-style and GCC-style attributes.


#define ELF_IMPORTED	__attribute__((visibility( "default" )))
#define ELF_EXPORTED	__attribute__((visibility( "protected" )))
#define ELF_INTERNAL	__attribute__((visibility( "hidden" )))
//
// ELF visibility scopes.  Again I have to use old-style attributes, there's no
// way to get both gcc and clang to accept it on a namespace otherwise:  clang
// only accepts it standalone in the namespace, e.g.
//	namespace foo {
//		[[ gnu::visibility( "default" ) ]];
//		...
//	}
// while gcc rejects this (but accepts it between foo and the '{')


#define forseq( loopvar, start, end )			\
		for( typeof((start)+((end)-(start)))	\
				loopvar = (start),	\
				_forseq_end = (end);	\
		     loopvar != _forseq_end;		\
		     loopvar++ )
// You'd think this should be replaceable by for( let i : Seq{ start, end })
// for some suitable definition of template class Seq, but sadly this does not
// seem to be the case.  In particular, forseq( i, 0, n ) with n is an unsigned
// int (or bigger) makes i the same type, while 0 always becomes a signed int
// with regard to overload resolution, so the Seq class would have no choice
// but to use a signed integer type of a bigger size (if any is available).


//============== Integer types ===============================================//
//
// For some annoying reason, int32_t is long instead of int.

using uint = unsigned;

using u64 = uint64_t;
using u32 = uint;
using u16 = uint16_t;
using u8  = uint8_t;

using s64 = int64_t;
using s32 = int;
using s16 = int16_t;
using s8  = int8_t;


//============== min/max that actually do what you want ======================//

template< typename L, typename R >
let constexpr min( L a, R b ) {  return a < b ? a : b;  }

template< typename L, typename R >
let constexpr max( L a, R b ) {  return a < b ? b : a;  }


//============== Euclidean remainder =========================================//
//
// i.e. mod( -1, 10 ) == 9

let inline constexpr mod( uint n, uint d ) -> uint
{
	return n % d;
}

let inline constexpr mod( int n, uint d ) -> uint
{
	if( n >= 0 )
		return (uint)n % d;
	return d + ~( ~(uint)n % d );
}


//============== Bit-test queries ============================================//
//
// Arbitrary combinations of signed or unsigned types are allowed, with the
// semantics of being upgraded to some type big enough to hold them both (even
// if none is available).
//
// Left-hand side is also allowed to be a pointer.

template< typename Val, typename Mask >
let inline constexpr any_set( Val val, Mask bits ) -> bool
{
	using IVal = conditional_t< is_pointer<Val>{}, uintptr_t, Val >;
	let ival = (IVal) val;

	using T = common_type_t< make_unsigned_t<IVal>, make_unsigned_t<Mask> >;
	return ( (T) ival & (T) bits ) != 0;
}

template< typename Val, typename Mask >
let inline constexpr any_clear( Val val, Mask bits ) -> bool
{
	// An alternative to the extra test is first converting both operands
	// to their common type, but they might not have one.
	return ( ! is_signed<Val>{} && bits < 0 ) || any_set( ~val, bits );
}

template< typename Val, typename Mask >
let inline constexpr all_set( Val val, Mask bits ) -> bool
{
	return ! any_clear( val, bits );
}

template< typename Val, typename Mask >
let inline constexpr all_clear( Val val, Mask bits ) -> bool
{
	return ! any_set( val, bits );
}
