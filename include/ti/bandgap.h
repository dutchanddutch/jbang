#pragma once
#include "defs.h"

struct alignas(4) Bandgap {
/*0*/	bool tshut	: 1;  //r-  1=thermal shutdown (> 147C)
	bool ecoz	: 1;  //r-  0=end of conversion
	bool contconv	: 1;  //rw  0=single  1=continuous conversion
	bool clrz	: 1;  //rw  0=reset digital outputs
	bool soc	: 1;  //rw  0->1 starts ADC conversion cycle
	bool tmpsoff	: 1;  //rw  1=temperature sensor off
	bool bgroff	: 1;  //rw  1=bandgap off (OFF mode)
	bool cbias_sel	: 1;  //rw  reference:  0=bandgap  1=resistor divider
	// When taking bandgap out of OFF-mode, wait 150μs before switching
	// cbias_sel to bandgap.

/*1*/	u8  dtemp;	//r-  temperature data from ADC (when ecoz=0)

alignas(4)
/*4*/	u8  dtrtempsc;	//rw  temperature sensor trim value
/*5*/	u8  dtrtemps;	//rw  temperature sensor trim value
/*6*/	u8  dtrbgapv;	//rw  bandgap trim value
/*7*/	u8  dtrbgapc;	//rw  bandgap trim value
};
