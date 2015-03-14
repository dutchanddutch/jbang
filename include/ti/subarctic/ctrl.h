#pragma once
#include "defs.h"
#include "util/device.h"
#include "ti/bandgap.h"

// Control Module

struct Ctrl;

extern Phys< Ctrl > ctrl;  // 0x44e10000 (== l4wk + 0x2'10'000)


struct Pad {
	u32 value;

	enum pull_t : uint {
		pull_down = 0,
		no_pull   = 1,
		pull_up   = 2,
	};

	enum slew_t : uint {
		fast = 0,
		slow = 1,
	};

	enum rx_t : uint {
		rx_dis = 0,
		rx_en = 1,
	};

	let inline mode() const { return value & 7; }
	let inline pull() const { return (pull_t)( value >> 3 & 3 ); }
	let inline rx()   const { return   (rx_t)( value >> 5 & 1 ); }
	let inline slew() const { return (slew_t)( value >> 6 & 1 ); }


	constexpr Pad( Pad const & ) = default;

	constexpr Pad( uint mode, pull_t pull, rx_t rx, slew_t slew = fast )
		: value( mode | pull << 3 | rx << 5 | slew << 6 ) {}


	let static inout( uint mode, pull_t pull, slew_t slew = fast ) {
		return Pad { mode, pull, rx_en, slew };
	}
	let static in( uint mode, pull_t pull, slew_t slew = fast ) {
		return Pad { mode, pull, rx_en, slew };
	}
	let static in( uint mode, slew_t slew = fast ) {
		return Pad { mode, no_pull, rx_en, slew };
	}
	let static out( uint mode, pull_t pull, slew_t slew = fast ) {
		return Pad { mode, pull, rx_dis, slew };
	}
	let static out( uint mode, slew_t slew = fast ) {
		return Pad { mode, no_pull, rx_dis, slew };
	}
};


struct Ctrl {
/*000*/	u32 ident;	//r-
	// 4e8b0100	subarctic 2.0 (XAM3359AZCZ100)
	// 4e8c0001	centaurus 2.0 (DM8148BCYE0)

/*004*/	u32 hwinfo;	//z-

alignas(0x10)
/*010*/	u32 sysconfig;
	// bit   0	z-  (reset)
	// bit   1	1-  (emu-free)
	// bits  2- 3	rw  idlemode
	// bits  4- 6	r-  standbymode: 2  (bogus, ctrl is no initiator)


	//-------- Boot config -----------------------------------------------//

alignas(0x40)
/*040*/	u8  sysboot_lo;
	// bits  0- 4	rw  sysboot 0-4: boot order
	// bit   5	rw  sysboot 5:   clkout 0 enabled
	// bits  6- 7	rw  sysboot 6-7: ethernet: 0=mii, 1=rmii, 2=rgmii
	//
/*041*/	u8  device_type;
	// bits  0- 2	r-  device type:  0=Test, 1=EMU, 2=HS, 3=GP
	//
/*042*/	u8  sysboot_hi;
	// bit   0	rw  sysboot 8:     xip/nand 16-bit
	// bit   1	rw  sysboot 9:     nand internal ECC
	// bits  2- 3	rw  sysboot 10-11: xip mux mode (0 for nand)
	// bits  4- 5	rw  sysboot 12-13: test mode (0 for normal operation)
	// bits  6- 7	rw  sysboot 14-15: osc 0 frequency
	//			0=19.2  1=24  2=25  3=26  MHz

alignas(4)
/*044*/	bool bl_done;	//rw  (free use)
alignas(2)
/*046*/	u8   bl_err;	//rw  (free use, actually u4)


	//-------- Security-related stuff ------------------------------------//

	// Most of the stuff below is from sources of unknown reliability, and
	// read-only (or entirely inaccessible) in public world and/or absent
	// entirely on GP devices.
	//
	// If no access flags (r-/rw etc) are mentioned, assume it's r-/z- or I
	// just haven't dared to try to poke it yet.

alignas(0x100)
/*100*/	u32 sec_ctrl;
/*104*/	u32 sec_sw;		//rw  (free use)
/*108*/	u32 sec_emu_mpu;	//r-

alignas(0x10)
/*110*/	u32 exp_emif_config;	//rw
/*114*/	u32 exp_emif_config2;	//rw

alignas(0x80)
/*180*/	u32 jtag_access;
	// bit   0	rw  enable debug tap 12 (DebugSS DAP)
	// bit   1	z-
	// bit   2	rw  enable debug tap 11 (Wakeup-M3 DAP/debug)
	// bit   3	rw  enable test tap 2
	// bit   4	rc  (unknown)
	// bits  5- 8	z-
	// bit   9	rw  enable debug core 0 (Cortex-A8)
	// bits 10-30	z-
	// bit  31	rs  lock this register (makes all bits read-only)

/*184*/	u32 jtag_cmd_in;
/*188*/	u32 jtag_cmd_out;
/*18c*/	u32 jtag_data_in;
/*190*/	u32 jtag_data_out;

/*194*/	u32 _194;

/*198*/	u32 exp_mreqdomain_0;
/*19c*/	u32 exp_mreqdomain_1;

/*1a0*/	u32 exp_fw_l3_0;	//r-  05dc'0000
/*1a4*/	u32 exp_fw_l3_1;	//r-  7f78'0000
/*1a8*/	u32 exp_fw_l4;		//r-  3133'0000
/*1ac*/	u32 load_exp_fw;	//r-         3c

/*1b0*/	u32 _1b0;
/*1b4*/	u32 sec_ctrl_ro;
/*1b8*/	u32 emif_obfuscation_key;
/*1bc*/	u32 sec_clk_ctrl;

/*1c0*/	u32 _1c0;
/*1c4*/	u32 _1c4;
/*1c8*/	u32 _1c8;
/*1cc*/	u32 _1cc;
/*1d0*/	u32 _1d0;
/*1d4*/	u32 exp_mreqdomain_2;
/*1d8*/	u32 _1d8;
/*1dc*/	u32 _1dc;

/*1e0*/	u32 mpu_l2_config;  //rw  (aegis)

alignas(0x200)
/*200*/	u8  cek[16];
	// more keying stuff here, but layout unclear, and all-zero anyway.

alignas(0x40)
/*240*/	u32 sec_status;
	// bit   0	r-  set during wait-in-reset, clear after secrom
/*244*/	u32 secmem_status;
/*248*/	u64 sec_err_stat_app;
/*250*/	u64 sec_err_stat_dbg;


	//-------- Secure State Machine (SSM) config -------------------------//

	using a8_pa_t = u32;	// cortex-a8 physical address
	using a8_va_t = u32;	// cortex-a8 virtual address (secure-world)

	// before secrom runs,
	//	firewall_control = 0x200
	//	cpsr_mode_enforcement = 0x1cb
	//	everything else = 0
	// secrom fills in various addresses and sets firewall_control to 0x41

alignas(0x100)
/*300*/	a8_pa_t	fast_secram_end;

/*304*/	u32	firewall_control;

	// secure stacked mode
/*308*/	a8_pa_t stacked_ram_start;
/*30c*/	a8_pa_t stacked_ram_end;

	// secure privileged mode
/*310*/	a8_pa_t spm_stack_start;
/*314*/	a8_pa_t spm_stack_end;

	// secure monitor mode (pa)
/*318*/	a8_pa_t mon_ram_code_start;
/*31c*/	a8_pa_t mon_ram_code_end;
/*320*/	a8_pa_t mon_ram_data_end;
/*324*/	a8_pa_t mon_rom_code_start;
/*328*/	a8_pa_t mon_rom_code_end;
/*32c*/	a8_pa_t mon_periph_start;
/*330*/	a8_pa_t mon_periph_end;
/*334*/	a8_pa_t mon_stack_start;
/*338*/	a8_pa_t mon_stack_end;

	// secure monitor mode (va)
/*33c*/	a8_va_t	mon_ram_code_vstart;
/*340*/	a8_va_t	mon_ram_code_vend;
/*344*/	a8_va_t	mon_ram_data_vend;
/*348*/	a8_va_t	mon_rom_code_vstart;
/*34c*/	a8_va_t	mon_rom_code_vend;
/*350*/	a8_va_t	mon_stack_vstart;
/*354*/	a8_va_t	mon_stack_vend;
/*358*/	a8_va_t	mon_shared_vstart;
/*35c*/	a8_va_t	mon_shared_vend;
/*360*/	a8_va_t	mon_periph_vstart;
/*364*/	a8_va_t	mon_periph_vend;

/*368*/	u32	cpsr_mode_enforcement;

/*36c*/	a8_pa_t	l3_secram_end;


	//-------- LDO and PLL controls --------------------------------------//

alignas(0x400)
/*400*/	u32 vbb_ldo[10];
	// 7 = mpu

/*428*/	u32 ram_ldo[6];
	// 0 = core
	// 1 = mpu

/*440*/	u32 _refclk_ljcb_ldo;

/*444*/	bool clk32khz_opp50;	//rw  set when per-m2 is 96 rather than 192 MHz

/*448*/	Bandgap bandgap[2];
	// only one present in subarctic and aegis

/*458*/	uint mpu_pll_ulow_sel	: 1;
	uint disp_pll_ulow_sel	: 1;
	uint ddr_pll_ulow_sel	: 1;
	// 0 = core-m6
	// 1 = per-m2

/*45c*/	u32 _45c;
/*460*/	u32 _460;
/*464*/	u32 _464;

/*468*/	u32 main_osc_ctrl;
	// bit   0	rw  disconnect internal bias resistor

/*46c*/	u32 rc_osc_ctrl;

/*470*/	u32 deepsleep_ctrl;

alignas(0x20)
/*480*/	u32 _pcie_config;

alignas(0x100)
/*500*/	u32 _pe_scratchpad[3];

/*50c*/	u8  pll_mpu_pwrstatus;  // (aegis)
/*50d*/	u8  pll_per_pwrstatus;
/*50e*/	u8  pll_disp_pwrstatus;
/*50f*/	u8  pll_ddr_pwrstatus;
	// 0 = power off
	// 1 = precharge ack (pon)
	// 3 = full power ack (pgood)


	//-------- Device configuration --------------------------------------//

alignas(0x200)
/*600*/	u32 jtag_id;	//r-  x'b944'02f  where x is:
	// 0 = revision 1.0
	// 1 = revision 2.0
	// 2 = revision 2.1

/*604*/	u32 features;	//r-  20ff0383
	// bit   0	pruss
	// bit   1	ethernet
	// bit   7	can
	// bits  8- 9	?
	// bits 16-23	pruss features:
	// bit  16	  ethercat / ODD_NIBBLE
	// bit  17	  TX_AUTO_SEQUENCE
	// bit  25	dss (aegis)
	// bit  29	sgx

/*608*/	u64 init_pressure;  //rw
	// bits  0- 1	mpu
	// bits  2- 3	pruss 0 (aegis)
	// bits  4- 5	pruss 1
	// bits  6- 7	?
	// bits  8- 9	-
	// bits 10-11	-
	// bits 12-13	-
	// bits 14-15	dft
	// bits 16-17	tc 0 read
	// bits 18-19	tc 0 write
	// bits 20-21	tc 1 read
	// bits 22-23	tc 1 write
	// bits 24-25	tc 2 read
	// bits 26-27	tc 2 write
	// bits 28-29	-
	// bits 30-31	-
	// bits 32-33	ethernet
	// bits 34-35	dss (aegis)
	// bits 36-37	usb dma (subarctic)
	// bits 38-39	usb qmgr (subarctic)
	// bits 40-41	vpfe 0 (aegis)
	// bits 42-43	vpfe 1 (aegis)
	// bits 44-45	-
	// bits 46-47	-
	// bits 48-49	(pcie)
	// bits 50-51	-
	// bits 52-53	sgx
	// bits 54-55	lcdc (subarctic)
	// bits 56-57	dap
	// bits 58-59	-
	// bits 60-61	-
	// bits 62-63	-

/*610*/	u32 dev_attr_aegis;

/*614*/	u32 edma_max_burst;  //rw  u2[3]  max burst length per TC
	// 0 =  16 bytes (2 cycles)
	// 1 =  32 bytes (3 cycles)
	// 2 =  64 bytes (5 cycles)
	// 3 = 128 bytes (9 cycles)

/*618*/	u32 _618;

/*61c*/	u32 _dsp_sysconfig;

	struct alignas(8) UsbPhy {
	/*0*/	bool cm_pwrdn	:  1;
		bool otg_pwrdn	:  1;
		bool chgdet_dis	:  1;
		bool chgdet_rst	:  1;
		bool src_on_dm	:  1;  // instead of dp
		bool sink_on_dp	:  1;  // instead of dm
		bool vsrc_en	:  1;
		bool isink_en	:  1;
	/*1*/	bool dm_pullup	:  1;
		bool dp_pullup	:  1;
		bool chgdet_ext	:  1;
		bool _ctl_11	:  1;

		bool gpio_en	:  1;
		bool gpio_inv	:  1;
		bool gpio_cross	:  1;
		bool _ctl_15	:  1;
	/*2*/	bool _ctl_16	:  1;
		bool dp_pulldn	:  1;  // gpio-mode only
		bool dm_pulldn	:  1;  // gpio-mode only

		bool vdet_en	:  1;  // VBUS detection (except Session End)
		bool sedet_en	:  1;  // Session End detection enabled
		bool _ctl_21	:  1;
		bool _ctl_22	:  1;
		bool dp_dm_swap	:  1;

	/*3*/	u8 _ctl_24_31;	//rw  must be kept at 0x3c

	/*4*/	u8 chgdet_status;

	/*5*/	bool wakeup_ev	:  1;
	};

/*620*/	UsbPhy usbphy[2];

	// First and second word are swapped.
	// Same format is used by MII module (but not by ALE).
	struct alignas(8) EthAddr {
		u32 hhi;  // bytes 4-5 (and two null-bytes)
		u32 wlo;  // bytes 0-3
	};

/*630*/	EthAddr ethaddr[2];

/*640*/	u32 _sw_revision;

/*644*/	u8 can_raminit_start;	//rw  triggers on rising edge
/*645*/	u8 can_raminit_done;	//rc
	// bit   0	can 0
	// bit   1	can 1

alignas(4)
/*648*/	bool usb_wakeup_en[2];

/*64c*/	u16 usbphy_filter[2];  // (aegis)

/*650*/	u32 gmii_config;
	// bits  0- 1	rw  mii 0 (port 1) mode
	// bits  2- 3	rw  mii 1 (port 2) mode
	//	0 = mii/gmii,  1=rmii,  2=rgmii
	// bit   4	rw  rgmii 0 internal delay disabled
	// bit   5	rw  rgmii 1 internal delay disabled
	//	when using rgmii these must be set, no internal delay supported
	// bit   6	rw  rmii 0 use external refclk
	// bit   7	rw  rmii 1 use external refclk
	//	when using rmii these must be set, internal refclk too jittery

/*654*/	u32 hass_config;  // (aegis)
/*658*/	u32 timer_cascade;  // (aegis)
/*65c*/	u32 _65c;
/*660*/	u32 _660;

/*664*/	u32 pwmss_config;
	// bit   0	rw  enable timebase clock for pwmss 0
	// bit   1	rw  enable timebase clock for pwmss 1
	// bit   2	rw  enable timebase clock for pwmss 2

/*668*/	u32 _668, _66c;

/*670*/	u64 mreqprio;  //rw  u4[16]
	// bits  0- 1	mpu port 0 (subarctic)
	// bits  2- 3	mpu port 1
	// bits  4- 5	pruss 1 pru 0
	// bits  6- 7	pruss 1 pru 1
	// bits  8- 9	ethernet
	// bits 10-11	usb dma (subarctic)
	// bits 12-13	usb qmgr (subarctic)
	// bits 14-15	sgx
	// bits 16-17	expansion
	// bits 18-19	dss (aegis)
	// bits 20-21	vpfe 0 (aegis)
	// bits 22-23	vpfe 1 (aegis)
	// bits 24-25	-
	// bits 26-27	-
	// bits 28-29	-
	// bits 30-31	-

/*678*/	u32 _678, _67c;
/*680*/	u32 _680, _684, _688, _68c;

/*690*/	u8 hw_event_sel[16];

/*6a0*/	u32 sr_enable;
	// bit   0	rw  enable smartreflex 0
	// bit   1	rw  enable smartreflex 1

/*6a4*/	u8 mpu_dbg_sel;  //rw  group 0-7
	uint		: 1;
	bool mpu_dbg_en : 1;  //rw

/*6a8*/	u32 mpu_dbg_data;  //r-

/*6ac*/	u32 _6ac;

/*6b0*/	u32 prcm_debug[2];

/*6b8*/	u32 _6b8, _6bc;

/*6c0*/	u64 mrgn_mode;

alignas(0x100)
/*700*/	u8 _700[ 0x70 ];

	using Opp = u32[6];

/*770*/	Opp mpu_opp;
/*788*/	Opp _dsp_opp;
/*7a0*/	Opp _iva_opp;
/*7b8*/	Opp core_opp;
/*7d0*/	u32 bb_scale;

/*7d4*/	u8 _7d4[ 0x7f4 - 0x7d4 ];

/*7f4*/	u16 usb_pid;
/*7f6*/	u16 usb_vid;
/*7f8*/	u16 _pcie_pid;
/*7fa*/	u16 _pcie_vid;

/*7fc*/	u32 dev_attr_subarctic;
	// in binary:  (x = may vary)
	//
	// xxxxxxxxxxxxxx'01'xxx'1111111'10'1111	ZCZ  300 MHz
	// xxxxxxxxxxxxxx'01'xxx'1111110'10'1111	ZCZ  600 MHz
	// xxxxxxxxxxxxxx'01'xxx'1111100'10'1111	ZCZ  720 MHz
	// xxxxxxxxxxxxxx'01'xxx'1111000'10'1111	ZCZ  800 MHz
	// xxxxxxxxxxxxxx'01'xxx'1110000'10'1111	ZCZ 1000 MHz
	//
	// xxxxxxxxxxxxxx'10'xxx'1111111'01'1111	ZCE  300 MHz
	// xxxxxxxxxxxxxx'10'xxx'1111110'01'1111	ZCE  600 MHz


	//-------- Pad configuration -----------------------------------------//

alignas(0x800)
/*800*/	Pad pad[ 384 ];  // 211 entries present
	// After reset, input_en is true and slow_slew is false for all pads,
	// function is 7 (gpio) for pads 0-109
	//	except pad 108 gets function 3 if sysboot 5 is high
	//	pull is disabled only for the sysboot inputs (pads 40-55)
	//	pull up/down varies for the remaining pins
	// function is 0 for pads 110-210
	//	these are all special-purpose pads and only four of them (121,
	//		122, 135, 141) alternatively support gpio function.
	//	pull up/down/disabled varies


/*e00*/	u8 vddhv_level;  //r-  0=1.8V  1=3.3V
/*e01*/	u8 vddhv_detect_ok;
/*e02*/	u8 _vddhv_something;
	// bit   0	vddhv1, gpmc
	// bit   1	mmc a
	// bit   2	mmc b
	// bit   3	eth a
	// bit   4	eth b
	// bit   5	vddhv6, general
	// mmc are vddhv2 and vddhv4
	// eth are vddhv3 and vddhv5

/*e04*/	u32 ddr_io_ctrl[2];
/*e0c*/	u32 ddr_vtp_ctrl[2];
/*e14*/	u32 ddr_vref_ctrl;
	// note: only one emif present on subarctic

/*e18*/	u32 _mlbp_sig_io_ctrl;
/*e1c*/	u32 _mlbp_dat_io_ctrl;
/*e20*/	u32 _mlbp_clk_io_ctrl;

/*e24*/	u32 _serdes_refclk_ctl;


	//-------- Event multiplexing ----------------------------------------//

alignas(0x100)
/*f00*/	u8 _dsp_irq_src[81];	// irq 15-95
alignas(4)
/*f54*/	u8 _mc_irq_src[57];	// irq 7-63
alignas(4)
/*f90*/	u8 edma_ev_src[64];	// channel 0-63
alignas(4)
/*fd0*/	u8 timer_ev_src[3];	// timer 5-7
alignas(4)
/*fd4*/	u8 ecap_ev_src[3];	// ecap 0-2
alignas(4)
/*fd8*/	u32 adc_ev_src[2];	// adc 0
	// 0 = pruss host irq 2
	// 1 = timer 4 event
	// 2 = timer 5 event
	// 3 = timer 6 event
	// 4 = timer 7 event
	// 5 = ext_hw_trigger (aegis)


	//-------- Reset isolation -------------------------------------------//

alignas(0x1000)
/*000*/	u32 reset_iso;
	// bit   0	rw  ethernet switch
	// bit   1	rw  jtag (aegis)

alignas(0x200)
/*200*/	u8 _1200[ 0x100 ];


	//-------- Misc power management -------------------------------------//

/*300*/	u8 _1300[ 0x18 ];

	enum class PllPower : u8 {
		off	= 0b1'0'1111'00,
		pon	= 0b1'0'1111'01,  // precharge (limited power)
		pgood	= 0b1'0'1111'11,  // full power, reset asserted
		ret	= 0b1'0'1101'11,  // reset released
		iso	= 0b1'0'0001'11,
		active	= 0b1'0'0000'11,

		hw	= 0b0'0'0000'11,  // auto-managed
	};

/*318*/	PllPower pll_mpu_pwrctl;  // (aegis)
/*319*/	PllPower pll_per_pwrctl;
/*31a*/	PllPower pll_disp_pwrctl;
/*31b*/	PllPower pll_ddr_pwrctl;
	// Default state is normal.
	// To power down, move in reverse sequence from hw to off.
	// To power up, move in forward sequence from off to hw, but wait
	// after moving to pon and pgood for confirmation in corresponding
	// pwrstatus register.

/*31c*/	u32 ddr_cke_ctrl;

/*320*/	uint pin_25_f3_sel	: 1;  // sub-mux for pin 25 ("GPMC_A9") mode 3
	// 0 = mmc 2 d7
	// 1 = rmii 1 crs/rxdv

	bool vsldo_core_auto_ramp : 1;

	struct {
/*324*/		u32 _clear;

		let operator () () -> void {
			_clear = 1;
			write_barrier( _clear );
			_clear = 0;
		}
	} m3_txev_eoi;

 union {
/*328*/	a8_pa_t mpu_resume_addr;
/*328*/	u32 ipc_msg[8];
 };

/*348*/	u32 rtc_idle;

	//-------- DDR PHY pad config ----------------------------------------//

alignas(0x100)
/*400*/	u32 _1400;

	template< uint n >
	struct DDRPadconf {
		static_assert( n >= 1 && n <= 11, "" );

		uint drive	:  3;
		uint slew	:  2;
		uint drive_clk	:  3;  // cmd macro 0 and data macros only
		uint slew_clk	:  2;  // cmd macro 0 and data macros only
		uint pullup	:  n;
		uint pulldown	:  n;
		// pull up/down are one bit per I/O line.
		// setting both bits selects bus keeper.
	};

	// argh
	using DDRCmdPadconf = DDRPadconf< 11 >;
	using DDRDataPadconf = DDRPadconf< 10 >;

/*404*/	DDRCmdPadconf ddr_cmd_ioctl[3];

alignas(0x40)
/*440*/	DDRDataPadconf ddr_data_ioctl[5];
	// dunno why 5 seem to be present, only first two are documented
};

static_assert( offsetof( Ctrl, mrgn_mode ) == 0x6c0, "" );
static_assert( offsetof( Ctrl, dev_attr_subarctic ) == 0x7fc, "" );
static_assert( offsetof( Ctrl, ddr_vref_ctrl ) == 0xe14, "" );
static_assert( offsetof( Ctrl, adc_ev_src ) == 0xfd8, "" );
static_assert( offsetof( Ctrl, rtc_idle ) == 0x1348, "" );
