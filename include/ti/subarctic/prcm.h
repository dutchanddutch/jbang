#pragma once
#include "defs.h"
#include "util/device.h"
#include "ti/irqd.h"
#include "ti/subarctic/pll.h"

struct Prcm;

inline namespace hw {
	extern Prcm prcm;  // 0x44e'00'000 (== l4wk + 0x2'00'000)
}


// Subarctic has simplified OMAP4/5 PRCM, but violates the standard register
// layouts and therefore needs custom definitions.  Nicely done guys!

struct alignas(4) ClockDomain {
	enum tr_t {
		hw_wakeup	= 0,  // auto wakeup, don't go to sleep
		sw_sleep	= 1,  // no wakeup, attempt to enter sleep
		sw_wakeup	= 2,  // manual wakeup
	};

	union {
		u32 w;
		u8  b;
	};

	// only subarctic sticks clock-active bits into byte 0
	let tr() const -> tr_t { return (tr_t)( b & 3 ); }
	let tr( tr_t control ) -> void { b = control; barrier( b ); }

	let autowake() -> void	{ tr( hw_wakeup ); }
	let sleep() -> void	{ tr( sw_sleep ); }
	let wake() -> void	{ tr( sw_wakeup ); }

	// Get clock activity bits.  Note that there's rarely a need to inspect
	// these, and they are also true during transitions hence useless for
	// confirming the readiness of anything.
	let operator [] ( uint bit ) const -> bool { return w >> bit & 1; }
};


// Module clock control register.
//
struct alignas(4) Module {
	// PRCM supports byte/halfword writes, no need for read-modify-update.
	union {
		u32 w;
		u16 h[2];
		u8  b[4];
	};

	enum mode_t : u8 {
		sw_disable	= 0,
		sw_enable	= 2,
	};
	let mode() -> mode_t & { return (mode_t &) b[0]; }
	let mode() const -> mode_t { return (mode_t) b[0]; }
	let enabled() const -> bool { return mode() != sw_disable; }

	enum idle_t {
		functional	= 0,
		transitioning	= 1,
		disconnected	= 2,
		idle		= 3,
	};
	let status() const -> idle_t { return (idle_t)( b[2] & 3 ); }
	let ready() const -> bool { return status() == functional; }
};

// Initiator module clock control register.
//
struct IModule : public Module {
	// bit 18 is normally reserved for this purpose alone, except of course
	// subarctic reuses it for other purposes in some non-initiators (e.g.
	// it enables the debounce clock of gpio modules).
	let standby() const -> bool { return any_set( b[2], 4 ); }
};


// Note: there seem to be quite a few "ghost" modules, which have a module
// register, a target agent, but aren't mentioned in the TRM or datasheet.
// A few do in fact exist but are apparently deemed "not for general public"
// by TI. These may or may not be disabled by eFuse.


// General procedure of enabling a module:
//	prcm.clk_foo.wake();
//	prcm.mod_foo.enable();
//	wait_until( prcm.mod_foo.ready() );
//
// Some modules need more than one clock domain awake, some clock domains can
// be safely assumed to be awake under all normal circumstances, so you can
// skip explicitly waking those.  A few modules also require a reset signal to
// be explicitly deasserted.




struct Prcm {
	// clock domain register
	using Clk = ClockDomain;

	// module control register types
	struct Mod : public Module {
		let enable()  -> void { mode() = sw_enable; }
		let disable() -> void { mode() = sw_disable; }
	};

	using ModAE = Module;
	using IModAE = IModule;
	using IMod = Mod;

	// gpio module register
	struct ModIO : public Mod {
		let static constexpr dbck_en = (u32) 1 << 18;
		let enable()  -> void { w = sw_enable | dbck_en; }
		let suspend() -> void { w = sw_disable | dbck_en; }
		let disable() -> void { w = sw_disable; }
	};

	//-------- Peripheral domain clock management ------------------------//
/*000*/	Clk clk_l4ls;
	// bit   8	l4ls_gclk = core_m4 / 2
	// bit   9
	// bit  10	uart_gfclk
	// bit  11	can_clk
	// bit  12
	// bit  13	timer7_gclk
	// bit  14	timer2_gclk
	// bit  15	timer3_gclk
	// bit  16	timer4_gclk
	// bit  17	lcdc_gclk
	// bit  18
	// bit  19	gpio_1_gdbclk
	// bit  20	gpio_2_gdbclk
	// bit  21	gpio_3_gdbclk
	// bit  22
	// bit  23
	// bit  24	i2c_fclk
	// bit  25	spi_gclk
	// bit  26
	// bit  27	timer5_gclk
	// bit  28	timer6_gclk

/*004*/	Clk clk_l3s;
	// bit   3	l3s_gclk = core_m4 / 2

/*008*/	Clk clk_l4fw;

/*00c*/	Clk clk_l3;
	// bit   2	emif_gclk
	// bit   3	mmc_fclk
	// bit   4	l3_gclk
	// bit   5
	// bit   6	cpts_rft_gclk
	// bit   7	mcasp_gclk

/*010*/	IMod mod_pcie;		// ?
/*014*/	IMod mod_eth;		// clk_eth
/*018*/	IMod mod_lcdc;		// clk_lcdc
/*01c*/	IMod mod_usb;		// clk_l3s.l3s_gclk (also per_dcoldo for phys)
/*020*/	IMod mod_mlb;
/*024*/	IMod mod_tptc0;		// clk_l3.l3_gclk
/*028*/	Mod mod_emif;
/*02c*/	Mod mod_ocmc;
/*030*/	Mod mod_gpmc;
/*034*/	Mod mod_asp0;
/*038*/	Mod mod_uart5;		// clk_l4ls
/*03c*/	Mod mod_mmc0;
/*040*/	Mod mod_elm;
/*044*/	Mod mod_i2c2;
/*048*/	Mod mod_i2c1;
/*04c*/	Mod mod_spi0;
/*050*/	Mod mod_spi1;
/*054*/	Mod mod_spi2;		// ?
/*058*/	Mod mod_spi3;		// ?
/*05c*/	u32 _05c;
/*060*/	Mod mod_l4ls;
/*064*/	Mod mod_l4fw;
/*068*/	Mod mod_asp1;
/*06c*/	Mod mod_uart1;		// clk_l4ls
/*070*/	Mod mod_uart2;		// clk_l4ls
/*074*/	Mod mod_uart3;		// clk_l4ls
/*078*/	Mod mod_uart4;		// clk_l4ls
/*07c*/	Mod mod_timer7;		// clk_l4ls.timer7_fck
/*080*/	Mod mod_timer2;		// clk_l4ls.timer2_fck
/*084*/	Mod mod_timer3;		// clk_l4ls.timer3_fck
/*088*/	Mod mod_timer4;		// clk_l4ls.timer4_fck
/*08c*/	Mod mod_asp2;		// ?
/*090*/	Mod mod_rng;		// clk_l4ls
/*094*/	Mod mod_aes0;
/*098*/	Mod mod_aes1;		// ?
/*09c*/	Mod mod_des;		// ?
/*0a0*/	Mod mod_hash;
/*0a4*/	Mod mod_pka;
/*0a8*/	ModIO mod_io6;		// ?
/*0ac*/	ModIO mod_io1;
/*0b0*/	ModIO mod_io2;
/*0b4*/	ModIO mod_io3;
/*0b8*/	ModIO mod_io4;		// ?
/*0bc*/	Mod mod_tpcc;		// clk_l3.l3_gclk
/*0c0*/	Mod mod_can0;		// clk_l4ls.dcan0_fck
/*0c4*/	Mod mod_can1;		// clk_l4ls.dcan1_fck
/*0c8*/	Mod mod_spare;		// ?
/*0cc*/	Mod mod_pwmss1;
/*0d0*/	Mod mod_emif_fw;	// enabled by secrom
/*0d4*/	Mod mod_pwmss0;
/*0d8*/	Mod mod_pwmss2;
/*0dc*/	Mod mod_l3_instr;
/*0e0*/	Mod mod_l3;
/*0e4*/	IMod mod_dft;
/*0e8*/	IMod mod_pruss;		// clk_pruss
/*0ec*/	Mod mod_timer5;
/*0f0*/	Mod mod_timer6;
/*0f4*/	Mod mod_mmc1;
/*0f8*/	Mod mod_mmc2;
/*0fc*/	IMod mod_tptc1;		// clk_l3.l3_gclk
/*100*/	IMod mod_tptc2;		// clk_l3.l3_gclk
/*104*/	ModIO mod_io5;		// ?
/*108*/	u32 _108;
/*10c*/	Mod mod_spinlock;
/*110*/	Mod mod_mailbox0;	// clk_l4ls.l4ls_gclk
/*114*/	u32 _114;
/*118*/	u32 _118;

/*11c*/	Clk clk_l4hs;
	// bit   3	l4hs_gclk
	// bit   4	cpsw_250MHz_gclk
	// bit   5	cpsw_50MHz_gclk
	// bit   6	cpsw_5MHz_gclk

/*120*/	Mod mod_l4hs;
/*124*/	Mod mod_exp_master;
/*128*/	Mod mod_exp_slave;

/*12c*/	Clk clk_ocpwp_l3;
	// bit   4	ocpwp_l3_gclk
	// bit   5	ocpwp_l4_gclk

/*130*/	IMod mod_ocpwp;		// clk_l4ls
/*134*/	Mod mod_mailbox1;	// ?
/*138*/	Mod mod_spare0;		// ?
/*13c*/	Mod mod_spare1;		// ?

/*140*/	Clk clk_pruss;
	// bit   4	pru_icss_ocp_gclk
	// bit   5	pru_icss_iep_gclk
	// bit   6	pru_icss_uart_gclk

/*144*/	Clk clk_eth;
	// bit   4	cpsw_125MHz_gclk

/*148*/	Clk clk_lcdc;
	// bit   4	lcdc_l3_ocp_gclk
	// bit   5	lcdc_l4_ocp_gclk


/*14c*/	Mod mod_clkdiv32k;	// clk_clk24mhz

/*150*/	Clk clk_clk24mhz;
	// bit   4	clk_24MHz_gclk = per_out_m3 / 8


alignas(0x400)
	//-------- Wakeup domain clock management ----------------------------//
/*400*/	Clk clk_l4wkup;
	// bit   2	l4_wkup_gclk
	// bit   3	sr_sysclk
	// bit   4	wdt1_gclk
	// bit   5	-
	// bit   6	-
	// bit   7	-
	// bit   8	gpio0_gdbclk
	// bit   9	-
	// bit  10	timer0_gclk
	// bit  11	i2c0_gfclk
	// bit  12	uart0_gfclk
	// bit  13	timer1_gclk
	// bit  14	adc_fclk

/*404*/	Mod mod_ctrl;		// clk_l4wkup; enabled by secrom
/*408*/	ModIO mod_io0;		// clk_l4wkup
/*40c*/	ModAE mod_l4wkup;
/*410*/	Mod mod_timer0;

/*414*/	IMod mod_dbgss;		// clk_l3_aon
	// bit  19	rw  dbgsysclk_en
	// bits 20-21	rw  trc pmd clksel
	// bits 22-23	rw  stm pmd clksel
	// bits 24-26	rw  trc pmd clkdiv
	// bits 27-29	rw  stm pmd clkdiv
	// bit  30	rw  debug_clka_en

/*418*/	Clk clk_l3_aon;
	// bit   2	dbgsysclk
	// bit   3	l3_aon_gclk
	// bit   4	debug_clka

	//-------- PLL registers ---------------------------------------------//
	//
	//	mpu	ddr	disp	core	per	pll
	//
	//	S	S	S	S+hsd	LJ	type
	//
	//	11	14	15	12	13	recalibration irq
	//
	//	1c	30	44	58	6c	common regs
	//	2c	40	54	68	9c	config ("clksel")
	//	88	94	98	90	8c	control ("clkmode")
	//	-	-	-	7c	-	clkdcoldo output control
	//	a8	a0	a4	-	ac	clkout-m2 output control
	//	-	-	-	80	-	clkout-m4 output control
	//	-	-	-	84	-	clkout-m5 output control
	//	-	-	-	d8	-	clkout-m6 output control
	//
	// input clock for all pll (and bypass for hsdivider) is main osc.
	// unused for all pll:  out-m2x2, out-m3, out-hif
	//
	// core out-dco-ldo -> core hsdivider
	// core out-m2 and in-ulow are unused.
	//
	// per out-dco-ldo -> usb phy clock (must be 960 MHz)
	// per in-ulow is unused.
	//
	// in-ulow of remaining pll (mpu, disp, ddr) is core-m6 or per-m2, mux
	// per pll in control module.


	// ROM bootloader setup:
	//
	//	core, per, mpu:
	//		1 MHz	ref (if osc is integer MHz)
	//		1.6 MHz	ref (if osc == 19.2 MHz)
	//	core pll
	//		  1 GHz	dco
	//		200 MHz	m4 -> l3f, l3s, sgx, pru iep, dbgss clka
	//		250 MHz	m5 -> ethernet
	//		125 MHz	m6
	//	mpu pll
	//		  1 GHz	dco
	//		500 MHz	m2 -> mpu
	//	per pll
	//		960 MHz	dco  -> usb phys
	//		192 MHz	m2   -> icss uart
	//		 96 MHz	m2/2 -> mmc
	//		 48 MHz	m2/4 -> uarts

/*41c*/	PllIdle    pll_mpu_idle;
/*420*/	PllStatus  pll_mpu_status;
/*424*/	PllSSC     pll_mpu_ssc;
/*42c*/	PllCfgS    pll_mpu_config;
/*430*/	PllIdle    pll_ddr_idle;
/*434*/	PllStatus  pll_ddr_status;
/*438*/	PllSSC     pll_ddr_ssc;
/*440*/	PllCfgS    pll_ddr_config;
/*444*/	PllIdle    pll_disp_idle;
/*448*/	PllStatus  pll_disp_status;
/*44c*/	PllSSC     pll_disp_ssc;
/*454*/	PllCfgS    pll_disp_config;
/*458*/	PllIdle    pll_core_idle;
/*45c*/	PllStatus  pll_core_status;
/*460*/	PllSSC     pll_core_ssc;
/*468*/	PllCfgS    pll_core_config;
/*46c*/	PllIdle    pll_per_idle;
/*470*/	PllStatus  pll_per_status;
/*474*/	PllSSC     pll_per_ssc;
/*47c*/	PllOutDCO  pll_per_out_dco;
/*480*/	PllOutHSD  pll_core_out_m4;
/*484*/	PllOutHSD  pll_core_out_m5;
/*488*/	PllControl pll_mpu_control;
/*48c*/	PllControl pll_per_control;
/*490*/	PllControl pll_core_control;
/*494*/	PllControl pll_ddr_control;
/*498*/	PllControl pll_disp_control;
/*49c*/	PllCfgLJ   pll_per_config;
/*4a0*/	PllOutS    pll_ddr_out_m2;
/*4a4*/	PllOutS    pll_disp_out_m2;
/*4a8*/	PllOutS    pll_mpu_out_m2;
/*4ac*/	PllOutLJ   pll_per_out_m2;

	//-------- Wakeup domain clock management (cont'd) -------------------//
/*4b0*/	IModAE mod_cm3;		// clk_l4wkup_aon

/*4b4*/	Mod mod_uart0;		// clk_l4wkup
/*4b8*/	Mod mod_i2c0;		// clk_l4wkup
/*4bc*/	Mod mod_adc;		// clk_l4wkup.adc_fclk
/*4c0*/	Mod mod_sr0;		// clk_l4wkup.sr_sysclk
/*4c4*/	Mod mod_timer1;		// clk_l4wkup.timer1_gclk
/*4c8*/	Mod mod_sr1;		// clk_l4wkup.sr_sysclk

/*4cc*/	Clk clk_l4wkup_aon;
	// bit   2	l4_wkup_aon_gclk

/*4d0*/	Mod mod_wdog0;
/*4d4*/	Mod mod_wdog1;

	//-------- PLL control registers (cont'd) ----------------------------//
/*4d8*/	PllOutHSD  pll_core_out_m6;


alignas(0x100)
	//-------- Clock muxes and dividers ----------------------------------//
	//
	// Note: warm reset insensitive
	//
/*500*/	u32 _500;

/*504*/	u32 clksel_timer7;
/*508*/	u32 clksel_timer2;
/*50c*/	u32 clksel_timer3;
/*510*/	u32 clksel_timer4;
	// bits  0- 1	rw  clock mux:
	//			0 = timer clkin
	//			1 = main osc (default)
	//			2 = clk_32khz

/*514*/	u32 clksel_eth;
	// bits  0- 1	r-  clock mux:  0 = core-m5
	// bit   2	rw  clock div:  0 = /2, 1 = /5
	//
	// Output of this should be 50 MHz, hence either:
	//	/2 selected, core-m5 at 100 MHz  (opp50)
	//	/5 selected, core-m5 at 250 MHz  (opp100 and above)

/*518*/	u32 clksel_timer5;
/*51c*/	u32 clksel_timer6;
	// same as timers above

/*520*/	u32 clksel_eth_ts_ref;
	// bit   0	rw  clock mux:
	//			0 = core-m5 (default)
	//			1 = core-m4

/*524*/	u32 _524;

/*528*/	u32 clksel_timer1;
	// bits  0- 2	rw  clock mux:
	//			0 = main osc (default)
	//			1 = clk_32khz
	//			2 = timer clkin
	//			3 = rc osc
	//			4 = rtc osc

/*52c*/	u32 clksel_gfx_fclk;
	// bit   0	rw  clock mux:
	//			0 = core-m4
	//			1 = per-m2
	// bit   1	rw  clock div:  0 = /1, 1 = /2
	//
/*530*/	u32 clksel_pruss_ocp;
	// bit   0	rw  clock mux:
	//			0 = core-m4 (default)
	//			1 = disp-m2

/*534*/	u32 clksel_lcd;
	// bits  0- 1	rw  clock mux:
	//			0 = disp-m2 (default)
	//			1 = core-m5
	//			2 = per-m2

/*538*/	u32 clksel_wdog1;
	// bit   0	rw  clock mux:
	//			0 = rc osc
	//			1 = clk_32khz

/*53c*/	u32 clksel_io0_dbck;
	// bits  0- 2	rw  clock mux:
	//			0 = rc osc
	//			1 = rtc osc
	//			2 = clk_32khz


alignas(0x100)
	//-------- MPU domain clock management -------------------------------//
/*600*/	Clk clk_mpu;
	// bit   2	mpu_clk

/*604*/	IMod mod_mpu;
	// Idle is acknowledged when MPU is suspended in WFI.  This causes PRCM
	// to assert an irq to the wakeup-M3.


alignas(0x100)
	//-------- Clock muxes and dividers (cont'd) -------------------------//

	// clkout0 always provides main soc

/*700*/	u32 clksel_clkout1;
	// bits  0- 2	rw  clock mux:
	//			0 = rtc osc
	//			1 = core-m4
	//			2 = ddr phy
	//			3 = per-m2
	//			4 = lcd
	// bits  3- 5	rw  clock div:  0..6=/1../7
	// bit   6	z-
	// bit   7	rw  clkout1 enabled


alignas(0x100)
	//-------- RTC domain clock management -------------------------------//
/*800*/	Mod mod_rtc;

/*804*/	Clk clk_rtc;
	// bit   8	l4_rtc_gclk
	// bit   9	rtc_32kclk


alignas(0x100)
	//-------- Graphics domain clock management --------------------------//
/*900*/	Clk clk_l3_gfx;
	// bit   8	gfx_l3_gclk
	// bit   9	gfx_fclk

/*904*/	IMod mod_sgx;
/*908*/	IMod mod_bitblt;	// ?

/*90c*/	Clk clk_l4ls_gfx;
	// bit   8	l4ls_gfx_gclk

/*910*/	Mod mod_mmu_cfg;	// ?
/*914*/	Mod mod_mmu_data;	// ?


alignas(0x100)
	//-------- Customer eFuse domain clock management --------------------//
/*a00*/	Clk clk_cefuse;	// l4_cefuse_clkdm
	// bit   8	l4_cefuse_giclk
	// bit   9	cust_efuse_sys_clk

alignas(0x20)
/*a20*/	Mod mod_cefuse;


alignas(0x100)
	//-------- PRCM module interface -------------------------------------//
/*b00*/	u32 revision;  //r-
	// bits  0- 3	minor revision (0)
	// bits  4- 7	major revision (0)

/*b04*/	Irqd<u32> mpu_irq;
/*b0c*/	Irqd<u32> m3_irq0;
	// bits  0- 7	z-
	// bit   8	rc  software-requested domain transition completed
	// bit   9	z-
	// bit  10	rc  software-requested wakeup completed
	// bit  11	rc  mpu pll recalibration required
	// bit  12	rc  core pll recalibration required
	// bit  13	rc  per pll recalibration required
	// bit  14	rc  ddr pll recalibration required
	// bit  15	rc  disp pll recalibration required

	// m3_irq1 (always enabled) indicates MPU clock is gated, asserted when
	// mod_mpu.mode() == sw_disable and mpu executes WFI instruction.



alignas(0x100)
	//-------- Peripheral domain power/reset management ------------------//

/*c00*/	u32 per_reset_active;
	// bit   0	rw  ? sw reset asserted
	// bit   1	rw  pruss sw reset asserted

/*c04*/	EvReg<u32> per_reset_event;

/*c08*/	u32 per_power_state;
	// bits  0- 1	r-  power domain state
	// bit   2	r-  logic state
	// bits 17-18	r-  "other memories" state
	// bit  20	r-  transition in progress
	// bits 21-22	r-  ocmc memory state
	// bits 23-24	r-  pruss memory state

/*c0c*/	u32 per_power_ctrl;
	// bits  0- 1	rw  requested power domain state
	// bit   3	rw  logic retention-state
	// bit   4	rx  request low power state change (auto-clears)
	// bits  5-6	rw  pruss memory on-state
	// bit   7	rw  pruss memory retention-state
	// bits 25-26	rw  "other memories" on-state
	// bit  27	rw  ocmc memory retention-state
	// bit  29	rw  "other memories" retention-state
	// bits 30-31	rw  ocmc memory on-state


alignas(0x100)
	//-------- Wakeup domain power/reset management ----------------------//

/*d00*/	u32 wkup_reset_active;
	// bit   3	rw  wakeup m3 sw reset asserted

/*d04*/	u32 wkup_power_ctrl;
	// bit   3	rw  logic retention-state
	// bit   4	rx  request low power state change (auto-clears)

/*d08*/	u32 wkup_power_state;
	// bit   2	r-  logic state
	// bits 17-18	r-  debugss memory state
	// bit  20	r-  transition in progress

/*d0c*/	EvReg<u32> wkup_reset_event;
	// bit   5	rc  wakeup-m3 software reset occurred
	// bit   6	rc  wakeup-m3 jtag/icepick reset occurred
	// bit   7	rc  wakeup-m3 icecrusher reset occurred


alignas(0x100)
	//-------- MPU domain power/reset management -------------------------//

/*e00*/	u32 mpu_power_ctrl;
	// bits  0- 1	rw  requested power domain state
	// bit   2	rw  logic retention-state
	// bit   4	rx  request low power state change (auto-clears)
	// bit  22	rw  l1 cache retention-state
	// bit  23	rw  l2 cache retention-state
	// bit  24	rw  ram retention-state

/*e04*/	u32 mpu_power_state;
	// bits  0- 1	r-  power domain state
	// bit   2	r-  logic state
	// bits  4- 5	r-  ram state
	// bits  6- 7	r-  l1 cache state
	// bits  8- 9	r-  l2 cache state
	// bit  20	r-  transition in progress

/*e08*/	EvReg<u32> mpu_reset_event;
	// bit   5	rc  cortex-a8 jtag/icepick reset occurred
	// bit   6	rc  cortex-a8 icecrusher reset occurred


alignas(0x100)
	//-------- Device power/reset management -----------------------------//

/*f00*/	OutReg<u32> dev_reset_cmd;
	// bit   0	-x  software warm reset
	// bit   1	-x  software cold reset

/*f04*/	u8  global_rsttime;  //rw  u8  global reset cycles
/*f05*/	u8  local_rsttime;   //rw  u5  local reset cycles

/*f08*/	EvReg<u32> dev_reset_event;
	// bit   0	rc  cold reset
	// bit   1	rc  software warm reset
	// bit   2	rc  security violation (triggered by SSM)
	// bit   3	rc  watchdog 0 expired
	// bit   4	rc  watchdog 1 expired
	// bit   5	rc  external warm reset
	// bit   6	z-
	// bit   7	z-
	// bit   8	z-
	// bit   9	rc  icepick warm reset
	//
	// The placement of the icepick warm reset bit makes no sense... unless
	// bit 8 is icepick cold reset?

/*f0c*/	u8  ramldo_pcharge_count;
/*f0d*/	u8  ramldo_vsetup_count;
/*f0e*/	u8  ramldo_sleep_count;
/*f0f*/	u8  ramldo_startup_count;
	// various timing parameters

	struct RamLDO {
		// initialized from eFuse {
		bool disable_rta	: 1;  //rw
		bool abboff_active	: 1;  //rw
		bool abboff_sleep	: 1;  //rw
		bool short_protect_en	: 1;  //rw
		bool no_external_cap	: 1;  //rw
		bool sub_regulation_en	: 1;  //rw
		// }
		bool no_external_clock	: 1;  //rw
		bool two_step_transfer	: 1;  //rw
		bool aipoff_override	: 1;  //rw
		u32			: 0;
		bool auto_retention_en	: 1;  //rw  retention allowed
		u8			: 0;
		bool retention_status	: 1;  //r-  LDO is in retention
		bool in_transition	: 1;  //r-  LDO is transitioning
		u32			: 0;
	};

/*f10*/	RamLDO ramldo_core;
/*f18*/	RamLDO ramldo_mpu;


alignas(0x1000)
	//-------- RTC domain power/reset management -------------------------//

/*000*/	u32 rtc_power_ctrl;
	// bit   2	rw  logic retention-state
	// bit   4	rx  request low power state change (auto-clears)

/*004*/	u32 rtc_power_state;
	// bit   2	r-  logic state
	// bit  20	r-  transition in progress


alignas(0x100)
	//-------- Graphics domain power/reset management --------------------//

/*100*/	u32 gfx_power_ctrl;
	// bits  0- 1	rw  requested power domain state
	// bit   2	rw  logic retention-state
	// bit   4	rx  request low power state change (auto-clears)
	// bit   6	rw  memory retention-state
	// bits 17-18	rw  memory on-state

/*104*/	u32 gfx_reset_active;
	// bit   0	rw  local reset asserted

/*110*/	u32 gfx_power_state alignas(0x10);
	// bits  0- 1	r-  power domain state
	// bit   2	r-  logic state
	// bits  4- 5	r-  memory state
	// bit  20	r-  transition in progress

/*114*/	EvReg<u32> gfx_reset_event;
	// bit   0	rc  local reset occurred


alignas(0x100)
	//-------- Customer eFuse domain power/reset management --------------//

/*200*/	u32 cefuse_power_ctrl;
	// bits  0- 1	rw  requested power domain state
	// bit   4	rx  request low power state change (auto-clears)

/*204*/	u32 cefuse_power_state;
	// bits  0- 1	r-  power domain state
	// bit   2	r-  logic state
	// bit  20	r-  transition in progress
	// bits 24-25	r>  previous low-power domain state
	//	write unchanged value to reset this field to 3 (ON)

};

static_assert( offsetof( Prcm, clk_clk24mhz ) == 0x150, "" );
static_assert( offsetof( Prcm, mod_cm3 ) == 0x4b0, "" );
static_assert( offsetof( Prcm, clksel_io0_dbck ) == 0x53c, "" );


template< PllType type >
struct Pll {
	PllConfig<type> &config;
	PllControl &control;
	PllStatus &status;
	// XXX add remaining fields/info/traits
};

using PllS = Pll< DPLLS >;
using PllLJ = Pll< DPLLLJ >;

let static constexpr pll_core = PllS {
	prcm.pll_core_config, prcm.pll_core_control, prcm.pll_core_status,
};
let static constexpr pll_per = PllLJ {
	prcm.pll_per_config, prcm.pll_per_control, prcm.pll_per_status,
};
let static constexpr pll_mpu = PllS {
	prcm.pll_mpu_config, prcm.pll_mpu_control, prcm.pll_mpu_status,
};
let static constexpr pll_ddr = PllS {
	prcm.pll_ddr_config, prcm.pll_ddr_control, prcm.pll_ddr_status,
};
let static constexpr pll_disp = PllS {
	prcm.pll_disp_config, prcm.pll_disp_control, prcm.pll_disp_status,
};
