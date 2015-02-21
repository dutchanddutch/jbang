#pragma once
#include "defs.h"

// These types are ultimately what this header accomplishes: you can use them
// like normal integers, but they are stored in big endian format.
template< typename T >
struct ByteSwapped;

using u16be = ByteSwapped< u16 >;
using u32be = ByteSwapped< u32 >;
using u64be = ByteSwapped< u64 >;
using s16be = ByteSwapped< s16 >;
using s32be = ByteSwapped< s32 >;
using s64be = ByteSwapped< s64 >;

// Byte-reversal functions.
let constexpr bswap16( u16 x ) -> u16 { return __builtin_bswap16( x ); }
let constexpr bswap32( u32 x ) -> u32 { return __builtin_bswap32( x ); }
let constexpr bswap64( u64 x ) -> u64 { return __builtin_bswap64( x ); }

// Overloaded version... not recommended with constants
let constexpr bswap( u8  x ) -> u8  { return x; }
let constexpr bswap( u16 x ) -> u16 { return bswap16( x ); }
let constexpr bswap( u32 x ) -> u32 { return bswap32( x ); }
let constexpr bswap( u64 x ) -> u64 { return bswap64( x ); }
let constexpr bswap( s8  x ) -> u8  { return (u8) x; }
let constexpr bswap( s16 x ) -> u16 { return bswap( (u16) x ); }
let constexpr bswap( s32 x ) -> u32 { return bswap( (u32) x ); }
let constexpr bswap( s64 x ) -> u64 { return bswap( (u64) x ); }

template< typename T >
struct ByteSwapped {
	static_assert( is_integral<T>{}, "" );

	using Tbe = ByteSwapped<T>;
	using Traw = make_unsigned_t<T>;

	Traw raw;

	// construct from raw value
	let static constexpr cast( Traw &raw ) -> Tbe & {
		return (Tbe &) raw;
	}
	let static constexpr cast( Traw raw ) -> Tbe {
		return (Tbe const &) raw;
	}

	// default copy/assign
	constexpr ByteSwapped() = default;
	constexpr ByteSwapped( Tbe const & ) = default;
	let operator = ( Tbe const & ) -> Tbe & = default;

	// auto-swapping conversions
	constexpr ByteSwapped( T value ) : raw( bswap( value ) ) {}
	constexpr operator T () const { return (T) bswap( raw ); }

	// take advantage of endian-independence to optimize bitwise ops
	let constexpr operator | ( Tbe arg ) const -> Tbe {
		return cast( raw | arg.raw );
	}
	let constexpr operator & ( Tbe arg ) const -> Tbe {
		return cast( raw & arg.raw );
	}
	let constexpr operator ^ ( Tbe arg ) const -> Tbe {
		return cast( raw ^ arg.raw );
	}
	let constexpr operator ~ () const -> Tbe { return cast( ~raw ); }

	let operator |= ( Tbe val ) -> Tbe & { return self = self | val; }
	let operator &= ( Tbe val ) -> Tbe & { return self = self & val; }
	let operator ^= ( Tbe val ) -> Tbe & { return self = self ^ val; }

	// The rest is done on unswapped values.  For most expressions this
	// happens automagically but in-place modify ops need help.
	let operator += ( T val ) -> Tbe & { return self = self + val; }
	let operator -= ( T val ) -> Tbe & { return self = self - val; }
	let operator *= ( T val ) -> Tbe & { return self = self * val; }
	let operator /= ( T val ) -> Tbe & { return self = self / val; }
	let operator <<= ( T val ) -> Tbe & { return self = self << val; }
	let operator >>= ( T val ) -> Tbe & { return self = self >> val; }

	let operator ++ () -> Tbe & { return self += 1; }
	let operator -- () -> Tbe & { return self -= 1; }

	let operator ++ (int) -> T { T old = self; self = old + 1; return old; }
	let operator -- (int) -> T { T old = self; self = old - 1; return old; }
};
