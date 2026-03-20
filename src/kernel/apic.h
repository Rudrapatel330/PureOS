#ifndef APIC_H
#define APIC_H

#include "types.h"

// Local APIC Registers (Offsets from Local APIC Base)
#define LAPIC_ID 0x0020
#define LAPIC_VER 0x0030
#define LAPIC_TPR 0x0080
#define LAPIC_APR 0x0090
#define LAPIC_PPR 0x00A0
#define LAPIC_EOI 0x00B0
#define LAPIC_RRD 0x00C0
#define LAPIC_LDR 0x00D0
#define LAPIC_DFR 0x00E0
#define LAPIC_SIVR 0x00F0 // Spurious Interrupt Vector Register
#define LAPIC_ISR 0x0100
#define LAPIC_TMR 0x0180
#define LAPIC_IRR 0x0200
#define LAPIC_ESR 0x0280
#define LAPIC_ICR_LOW 0x0300   // Interrupt Command Register Low
#define LAPIC_ICR_HIGH 0x0310  // Interrupt Command Register High
#define LAPIC_LVT_TIMER 0x0320 // LVT Timer Register
#define LAPIC_LVT_THERMAL 0x0330
#define LAPIC_LVT_PERF 0x0340
#define LAPIC_LVT_LINT0 0x0350
#define LAPIC_LVT_LINT1 0x0360
#define LAPIC_LVT_ERROR 0x0370
#define LAPIC_TIMER_INITCNT 0x0380
#define LAPIC_TIMER_CURCNT 0x0390
#define LAPIC_TIMER_DIV 0x03E0

// ICR Command Delivery Modes
#define ICR_FIXED 0x0000
#define ICR_LOWEST_PRIORITY 0x0100
#define ICR_SMI 0x0200
#define ICR_NMI 0x0400
#define ICR_INIT 0x0500
#define ICR_STARTUP 0x0600

// ICR Command Levels / Triggers
#define ICR_DEASSERT 0x0000
#define ICR_ASSERT 0x4000
#define ICR_EDGE 0x0000
#define ICR_LEVEL 0x8000

// ICR Dest Shorthands
#define ICR_NO_SHORTHAND 0x00000
#define ICR_SELF 0x40000
#define ICR_ALL_INCL_SELF 0x80000
#define ICR_ALL_EXCL_SELF 0xC0000

void lapic_init(void);
void lapic_eoi(void);
uint32_t lapic_read(uint32_t reg);
void lapic_write(uint32_t reg, uint32_t data);
void lapic_send_ipi(uint8_t target_apic_id, uint32_t flags);

#endif // APIC_H
