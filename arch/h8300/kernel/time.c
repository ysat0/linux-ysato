/*
 *  linux/arch/h8300/kernel/time.c
 *
 *  Yoshinori Sato <ysato@users.sourceforge.jp>
 *
 *  Copyright (C) 1991, 1992, 1995  Linus Torvalds
 *
 * This file contains the m68k-specific time handling details.
 * Most of the stuff is located in the machine specific files.
 *
 * 1997-09-10	Updated NTP code according to technical memorandum Jan '96
 *		"A Kernel Model for Precision Timekeeping" by Dave Mills
 */

#include <linux/errno.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/param.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/timex.h>
#include <linux/profile.h>
#include <linux/platform_device.h>

#include <asm/io.h>
#include <asm/irq_regs.h>
#include <asm/timer.h>
#include <asm/irq_regs.h>

#define	TICK_SIZE (tick_nsec / 1000)

void h8300_timer_tick(void)
{
	if (current->pid)
		profile_tick(CPU_PROFILING);
	xtime_update(1);
	update_process_times(user_mode(get_irq_regs()));
}

void __init timer_setup(void)
{
	early_platform_driver_register_all("earlytimer");
	early_platform_driver_probe("earlytimer", 1, 0);
}

void __init time_init(void)
{
	late_time_init = timer_setup;
}
