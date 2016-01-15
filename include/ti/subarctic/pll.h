#pragma once
#include "defs.h"
#include "util/device.h"

// Subarctic has one DPLLLJ instance and four DPLLS instances (one with HSD).
//
// Their configuration is split across multiple registers which are allocated
// in random order within the CM_WKUP region of PRCM.
//
// Aegis uses the same registers, but laid out cleanly in its CM_WKUP.
//
// The same PLLs are used on Centaurus and e.g. OMAP4, but different register
// interfaces.  (Centaurus seems to give raw access to all PLL signals.)
//
// Features unavailable in subarctic:
//	bypass divider (N2)
//	fractional multiplier (except implicitly as part of SSC)
//	bandwidth control
//
// The last two are available on Aegis for two of the PLLs.
//
// Note that there are also some PLL-related registers in the control module,
// specifically power control and muxes for ulow.


enum PllType {
	DPLLS,	// also known as "type A"
	DPLLLJ,	// also known as "type B"
};


//-------------- PLL configuration -------------------------------------------//

template< PllType type >
struct PllConfig;

template<>
struct alignas(4) PllConfig< DPLLS > {
	uint _prediv	:  7;
	uint		:  1;
	uint _mult	: 11;
	uint		:  3;
	bool dcc_en	:  1;
	bool alt_bypass	:  1;  // use ulow as bypass clock
	uint dcc_count	:  8;

	constexpr PllConfig( uint prediv, uint mult )
		: _prediv( prediv - 1 ), _mult( mult / 2 ),
		  dcc_en( false ), alt_bypass( false ), dcc_count( false ) {}

	// TODO these constants are properties of the PLL, not of the register
	// interface.  Move them to a traits class or something...

	let static constexpr predivider_scale = 1u;
	let static constexpr predivider_min = 1u;
	let static constexpr predivider_max = 128u;

	let static constexpr multiplier_scale = 2u;
	let static constexpr multiplier_min = 4u;
	let static constexpr multiplier_max = 4094u;
	// note: if fractional multiplier or SSC is used, valid range becomes:
	let static constexpr multiplier_min_f = 40u;
	let static constexpr multiplier_max_f = 4090u;
	// and full interval swept by SSC needs to lie inside this range.
	// Note also that low values cause worse jitter in this case.

	let constexpr predivider() const -> uint {  return 1 + _prediv;  }
	let constexpr multiplier() const -> uint {  return 2 * _mult;  }

	// DC-correction (DCC) is only used on pll_mpu on aegis: it allows
	// the dco output to be used directly for frequencies > 1 GHz by making
	// it DC-balanced (50% duty cycle).
};

template<>
struct alignas(4) PllConfig< DPLLLJ > {
	uint _prediv	:  8;
	uint _mult	: 12;  // must be >= 2
	uint		:  3;
	bool alt_bypass	:  1;  // use ulow as bypass clock
	uint sd_divider	:  8;  // must be set to ceil( dco / 250 MHz )

	constexpr PllConfig( uint prediv, uint mult, uint sddiv )
		: _prediv( prediv - 1 ), _mult( mult ),
		  alt_bypass( false ), sd_divider( sddiv ) {}

	let static constexpr predivider_scale = 1u;
	let static constexpr predivider_min = 1u;
	let static constexpr predivider_max = 256u;

	let static constexpr multiplier_scale = 1u;
	let static constexpr multiplier_min = 2u;
	let static constexpr multiplier_max = 4095u;
	// note: if fractional multiplier or SSC is used, valid range becomes:
	let static constexpr multiplier_min_f = 2u;
	let static constexpr multiplier_max_f = 4093u;
	// and full interval swept by SSC needs to lie inside this range.

	let constexpr predivider() const -> uint {  return 1 + _prediv;  }
	let constexpr multiplier() const -> uint {  return _mult;  }
};

using PllCfgS  = PllConfig< DPLLS >;
using PllCfgLJ = PllConfig< DPLLLJ >;

struct alignas(4) PllCfg2 {
	uint mult_frac	: 18;  // fractional part of multiplier
	uint freqsel	:  4;
	uint		:  1;
	uint bw_adjust	:  4;
	//	0b0101	nominal * 2
	//	0b0001	nominal * 1
	//	0b0000	nominal / 1
	//	0b0100	nominal / 2
	//	0b1000	nominal / 4
	//	0b1100	nominal / 8
};

// When PLL is enabled (locking or locked):
//	ref = in / predivider		reference clock
//	dco = ref * multiplier		undivided PLL output
//
// Bypass clock (used when not locked) is:
//	bypass = alt_bypass ? ulow : in
//
// If the ulow input is not used (tied off), alt_bypass is RAZ/WI.



// Spread-spectrum clocking config

struct alignas(4) PllSSC {

	using fp18_t = int;  // fixed-point with 18 fractional bits


	fp18_t delta;	//rw  modulation step size (2 integer + 18 frac bits)


	// modulation quarter-period in cycles of refclk (range 14..16256)
	//	range follows from the encoding together with the constraint
	//	f_m < 70 f_ref  where f_m is the modulation frequency.
	//
	// For periods with more than one representation, the one with minimal
	// exponent is preferred.
	//
	// (Note: multiplying by 1 or adding 0 is just a way to take care of
	// integer promotion in a less ugly way than a type-cast.)

	u8 qperiod_mantissa;	//rw  range 1..127
	u8 qperiod_exponent;	//rw  range 0..7

	let constexpr qperiod_cycles() const -> int {
		return int { qperiod_mantissa } << qperiod_exponent;
	}
	let constexpr period_cycles() const -> int {
		return qperiod_cycles() << 2;
	}

	// Apparently having period_exponent > 3 causes the delta step to be
	// applied only once every 2^(period_exponent-3) refclks.
	//
	let constexpr interval_log2() const -> int {
		return max( qperiod_exponent - 3, 0 );
	}
	let constexpr interval_cycles() const -> int {
		return 1 << interval_log2();
	}

	// (quarter-)period in step intervals.
	let constexpr qperiod_steps() const -> int {
		return int { qperiod_mantissa } << min( qperiod_exponent, 3 );
		// equivalent to:
		// return qperiod_cycles() / interval_cycles();
	}
	let constexpr period_steps() const -> int {
		return qperiod_steps() << 2;
		// equivalent to:
		// return period_cycles() / interval_cycles();
	}

	// modulation amplitude of PLL multiplier
	//
	// The range of the modulated multiplier is
	//	multiplier + [-1..1] * amplitude	if center-spreading,
	//	multiplier + [-2..0] * amplitude	if down-spreading.
	// (The spreading mode is configured in PllControl)
	//
	// In case of DPLLS this concerns the raw _multiplier, hence needs to
	// be multiplied by two.
	//
	// Documentation mentioned a 20% overshoot is possible.
	//
	let constexpr amplitude_fp18() const -> fp18_t {
		return delta * qperiod_steps();
	}
	let constexpr amplitude() const -> float {
		return __builtin_ldexpf( amplitude_fp18(), -18 );
	}
};



//-------------- PLL control/status ------------------------------------------//

struct alignas(4) PllControl {
/*0*/	uint mode	:  3;
	//	1  low-power stop (DPLLLJ only)
	//	4  disabled / "MN-bypass" (reset default)
	//	5  low-power idle bypass
	//	6  fast-relock idle bypass (DPLLS only)
	//	7  lock

	// Changing mode to disabled has side-effect of setting multiplier to 0.
	// Dispite its name, "low-power stop" seems to behave like idle bypass.

	// DPLLS only {
	uint ramp_type	: 2;  //rw
	//	ramp up clock after lock (and optionally relock) in 4 stages:
	//	0  ramp disabled
	//	1  bypass => /8 => /4 => /2 => /1
	//	2  bypass => /4 => /2 => /1.5 => /1
	uint ramp_rate	: 3;  //rw  log2( ramp step time / 2 refclks )
/*1*/	bool auto_recal	: 1;  //rw  aka "driftguard"
	bool relock_ramp: 1;  //rw  also ramp up clock when relocking
	bool low_power	: 1;  //rw  see notes
	bool regm4x_en	: 1;  //z-
	// }

	bool ssc_en	: 1;  //rw
	bool ssc_ack	: 1;  //r-
	bool ssc_down	: 1;  //rw  downspread instead of center-spread
	uint ssc_type	: 1;  //rw
	//	0=triangular
	//	1="only available under proper licensing agreement"
};

struct alignas(4) PllIdle {
/*0*/	u32 autoidle;	//rw
	//	0  manual control only
	//	1  auto enter/leave low-power stop mode
	//	5  auto enter/leave low-power idle bypass mode
	//
	// XXX TRM warns this feature is not supported.
};

struct alignas(4) PllStatus {
/*0*/	bool locked;	//r-  ack of mode == 7
/*1*/	bool disabled;	//r-  ack of mode == 4
};


//-------------- PLL output controls -----------------------------------------//
//
// Annoyingly variable layout...

// divider and divider-change-ack

template< int w, int scale = 1 >
struct PllOutDiv {
	static_assert( w >= 1 && w < 8, "" );

	u8   _div	: w;
	bool _ack	: 1;
	u8		: 0;

	let static constexpr postdiv_step = scale;
	let static constexpr postdiv_min = scale;
	let static constexpr postdiv_max = ( ( 1 << w ) - 1 ) * scale;

	let postdiv() const -> int {  return _div * scale;  }
	let postdiv_ack() const -> bool {  return _ack;  }
};

// special-case fixed divider (normally /1)

template< int value >
struct PllOutDiv<0,value> {
	u8		: 8;

	let static constexpr postdiv_step = 0;
	let static constexpr postdiv_min = value;
	let static constexpr postdiv_max = value;

	let postdiv() const -> int {  return value;  }
	let postdiv_ack() const -> bool {  return false;  }
};

// remaining registers

struct PllOutCtl {
	bool forced	: 1;  //rw
	bool active	: 1;  //r-
	u8		: 2;
	bool auto_pwrdn	: 1;  //rw  HSD and DCOLDO only
	u8		: 0;
};

// combine

template< int w, int scale = 1 >
struct alignas(4) PllOut : public PllOutDiv< w, scale>, PllOutCtl {};

// create relevant subtypes and aliases:

template< PllType type >
struct PllOutM2;

template<>
struct PllOutM2< DPLLS > : public PllOut<5,2> {
private:
	using PllOutCtl::auto_pwrdn;
};

template<>
struct PllOutM2< DPLLLJ > : public PllOut<7> {
private:
	using PllOutCtl::auto_pwrdn;
};

using PllOutDCO = PllOut<0>;
using PllOutHSD = PllOut<5>;
using PllOutS   = PllOutM2< DPLLS >;
using PllOutLJ  = PllOutM2< DPLLLJ >;

// verify these still qualify as "trivial" types and have the right size:

static_assert( is_trivial< PllOutDCO >{}, "" );
static_assert( is_trivial< PllOutHSD >{}, "" );
static_assert( is_trivial< PllOutS >{}, "" );
static_assert( is_trivial< PllOutLJ >{}, "" );

static_assert( sizeof( PllOutDCO ) == 4, "" );
static_assert( sizeof( PllOutHSD ) == 4, "" );
static_assert( sizeof( PllOutS ) == 4, "" );
static_assert( sizeof( PllOutLJ ) == 4, "" );
