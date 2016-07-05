#pragma once
#include "defs.h"
#include "util/device.h"
#include "ti/gpio.h"

inline namespace hw {
	extern Io io0;  // 0x44e'07'000  (== l4wk + 0x2'07'000)
	extern Io io1;  // 0x480'4c'000  (== l4ls + 0x0'4c'000)
	extern Io io2;  // 0x481'ac'000  (== l4ls + 0x1'ac'000)
	extern Io io3;  // 0x481'ae'000  (== l4ls + 0x1'ae'000)
}

constexpr Io *io_ptr[] = { &io0, &io1, &io2, &io3 };

//-------------- IO pin accessor ---------------------------------------------//
//
// I'm not really happy with it yet, and it certainly doesn't belong in *this*
// file, but I'll sort that out later...

struct IoPin {
	u8 index;

	constexpr IoPin() : index( 255 ) {}
	constexpr IoPin( uint bank, uint bit )
		: index( (u8)( bank << 5 | bit ) ) {}

	template< char bank, char dot, char dig0, char dig1 >
	let static constexpr from_chars() -> IoPin {
		static_assert( bank >= '0' && bank <= '9', "" );
		static_assert( bank - '0' < countof( io_ptr ), "" );
		static_assert( dot == '.', "" );
		static_assert( dig0 >= '0' && dig0 <= '3', "" );
		static_assert( dig1 >= '0' && dig1 <= '9', "" );
		static_assert( dig0 < '3' || dig1 < '2', "" );
		return { bank - '0', ( dig0 - '0' ) * 10 + ( dig1 - '0' ) };
	}

	let constexpr valid() const -> bool {  return index != 255;  }

	let constexpr bank_num() const -> uint {  return index >> 5;  }
	let constexpr bank() const -> Io & {  return *io_ptr[ bank_num() ];  }

	let constexpr bit() const -> uint {  return index & 31;  }
	let constexpr bits() const -> u32 {  return 1 << bit();  }


	// read current input/output value
	let in()  const -> bool {  return any_set( bank().in,    bits() );  }
	let out() const -> bool {  return any_set( bank().out(), bits() );  }

	// control output
	// see ti/gpio.h on comments w.r.t. the atomicity of these
	let set()   const -> void {  bank().set( bits() );  }
	let clear() const -> void {  bank().clear( bits() );  }
	let highz() const -> void {  bank().highz( bits() );  }
	let drive() const -> void {  bank().drive( bits() );  }
	let out( bool value ) const -> void {
		bank().out( bits(), value );
	}
	let drive( bool value ) const -> void {
		bank().drive( bits(), value );
	}
	let toggle() const -> void {  out( ! out() );  }

	// control irq
	let irq_clear( uint output ) const -> void {
		dev_send( bank().irq.clear[ output ], bits() );
	}
};

let constexpr operator == ( IoPin const &a, IoPin const &b ) -> bool {
	return a.index == b.index;
}
let constexpr operator != ( IoPin const &a, IoPin const &b ) -> bool {
	return a.index != b.index;
}

template< char ...chars >
let constexpr operator ""_io() -> IoPin {
	return IoPin::from_chars< chars... >();
}

let constexpr no_io = IoPin {};

//-------------- Pad to GPIO mapping -----------------------------------------//

constexpr IoPin pad_io_table[] = {
/*  0*/	1.00_io, 1.01_io, 1.02_io, 1.03_io, 1.04_io, 1.05_io, 1.06_io, 1.07_io,
/*  8*/	0.22_io, 0.23_io, 0.26_io, 0.27_io, 1.12_io, 1.13_io, 1.14_io, 1.15_io,
/* 16*/	1.16_io, 1.17_io, 1.18_io, 1.19_io, 1.20_io, 1.21_io, 1.22_io, 1.23_io,
/* 24*/	1.24_io, 1.25_io, 1.26_io, 1.27_io, 0.30_io, 0.31_io, 1.28_io, 1.29_io,
/* 32*/	1.30_io, 1.31_io, 2.00_io, 2.01_io, 2.02_io, 2.03_io, 2.04_io, 2.05_io,
/* 40*/	2.06_io, 2.07_io, 2.08_io, 2.09_io, 2.10_io, 2.11_io, 2.12_io, 2.13_io,
/* 48*/	2.14_io, 2.15_io, 2.16_io, 2.17_io, 0.08_io, 0.09_io, 0.10_io, 0.11_io,
/* 56*/	2.22_io, 2.23_io, 2.24_io, 2.25_io, 2.26_io, 2.27_io, 2.28_io, 2.29_io,
/* 64*/	2.30_io, 2.31_io, 3.00_io, 3.01_io, 3.02_io, 3.03_io, 3.04_io, 0.16_io,
/* 72*/	0.17_io, 0.21_io, 0.28_io, 3.09_io, 3.10_io, 2.18_io, 2.19_io, 2.20_io,
/* 80*/	2.21_io, 0.29_io, 0.00_io, 0.01_io, 0.02_io, 0.03_io, 0.04_io, 0.05_io,
/* 88*/	0.06_io, 0.07_io, 1.08_io, 1.09_io, 1.10_io, 1.11_io, 0.12_io, 0.13_io,
/* 96*/	0.14_io, 0.15_io, 3.05_io, 3.06_io, 3.14_io, 3.15_io, 3.16_io, 3.17_io,
/*104*/	3.18_io, 3.19_io, 3.20_io, 3.21_io, 0.19_io, 0.20_io,	no_io,   no_io,
/*112*/	  no_io,   no_io,   no_io,   no_io,   no_io,   no_io,   no_io,   no_io,
/*120*/	  no_io, 3.07_io, 3.08_io,   no_io,   no_io,   no_io,   no_io,   no_io,
/*128*/	  no_io,   no_io,   no_io,   no_io,   no_io,   no_io,   no_io, 0.18_io,
/*136*/	  no_io,   no_io,   no_io,   no_io,   no_io, 3.13_io,
};

let inline constexpr pad_io( uint pin ) -> IoPin {
	return pin < countof( pad_io_table ) ? pad_io_table[ pin ] : no_io;
}
