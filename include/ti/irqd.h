#pragma once
#include "defs.h"

// Standard irq aggregators
//	One or more level and/or edge-triggered sources.
//	Level output.  Sometimes also pulse-output, in which case an `eoi'
//	register will be nearby, normally immediately before or after it.
//
// multiple outputs are achieved by
//	Irqd4<T>[n]
// or
//	Irqd4<T[n]>
// (varies)


// old version

template< typename T >
struct Irqd {
	using TR = T;
	using TW = T volatile;

 union {
	TR pending;	//r-
	TW clear;	//-c  edge-triggered sources only
 };
	T enabled;	//rw
};

// old version with wakeup-enable

template< typename T >
struct Irqdw : public Irqd<T> {
	T wakeup;	//rw
};


// highlander version

template< typename T >
struct Irqd4 {
	using TR = T;
	using TW = T volatile;

 union {
	TR pending;	//r-
	TW set;		//-s  edge-triggered sources only, for debug
 };
 union {
	TW clear;	//-c  edge-triggered sources only
	TR active;	//r-  = pending & enabled
 };
 union {
	TR enabled;	//r-
	TW enable;	//-s
 };
	TW disable;	//-c
};

// highlander version with wakeup-enable

template< typename T >
struct Irqd4w : public Irqd4<T> {
	T wakeup;	//rw
};


// dma version (no status, only enables)

template< typename T >
struct Dmad4 {
	using TR = T;
	using TW = T volatile;

 union {
	TR enabled;	//r-
	TW enable;	//-s
 };
	TW disable;	//-c
};

// dma version with wakeup-enable

template< typename T, typename W >
struct Dmad4w : public Dmad4<T> {
	W wakeup;	//rw
};
