/*
 * linux/include/asm-h8300/timex.h
 *
 * H8/300 architecture timex specifications
 */
#ifndef __ASM_RX_TIMEX_H__
#define __ASM_RX_TIMEX_H__

#define CLOCK_TICK_RATE (CONFIG_CPU_CLOCK*1000/8192) /* Timer input freq. */

typedef unsigned long cycles_t;
extern short h8300_timer_count;

static inline cycles_t get_cycles(void)
{
	return 0;
}

#endif
