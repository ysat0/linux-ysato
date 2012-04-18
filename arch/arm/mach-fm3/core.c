/*
 *  linux/arch/arm/mach-fm3/core.c
 *
 *  Copyright (C) 2012 Yoshinori Sato
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/sysdev.h>
#include <linux/interrupt.h>
#include <linux/amba/bus.h>
#include <linux/amba/clcd.h>
#include <linux/amba/mmci.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/io.h>
#include <linux/ata_platform.h>
#include <linux/delay.h>
#include <linux/clk.h>

#include <asm/clkdev.h>
#include <asm/system.h>
#include <asm/irq.h>
#include <asm/leds.h>
#include <asm/mach-types.h>
#include <asm/hardware/arm_timer.h>
#include <asm/mach/arch.h>
#include <asm/mach/flash.h>
#include <asm/mach/irq.h>
#include <asm/mach/map.h>

#include <mach/platform.h>
#include <mach/hardware.h>

#include <asm/hardware/gic.h>

#include "clock.h"
#include "core.h"

#define SYSTICK 0xe000e010
#define CONTROL 0x00000000
#define RELOAD  0x00000004
#define CURRENT 0x00000008

#define TICKINT 0x00000002
#define ENABLE  0x00000001

static void timer_set_mode(enum clock_event_mode mode,
			   struct clock_event_device *clk)
{
	unsigned long ctrl;
	unsigned long rate;
	unsigned long count;
	struct clk *input;

	switch(mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		input = clk_get(NULL, "hclk");
		rate = clk_get_rate(input) / 8;
		count = rate / 10000;
		writel(count, SYSTICK + RELOAD);
		/* fall through */
	case CLOCK_EVT_MODE_ONESHOT:
		/* period set, and timer enabled in 'next_event' hook */
		ctrl |= TICKINT | ENABLE;
		break;
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
	default:
		ctrl = 0;
	}

	writel(ctrl, SYSTICK + CONTROL);
}

static int timer_set_next_event(unsigned long evt,
				struct clock_event_device *unused)
{
	unsigned long ctrl = readl(SYSTICK + CONTROL);

	writel(evt, SYSTICK + RELOAD);
	writel(ctrl | TICKINT, SYSTICK + CONTROL);

	return 0;
}

static struct clock_event_device systick_clockevent =	 {
	.name		= "systick",
	.shift		= 24,
	.features       = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.set_mode	= timer_set_mode,
	.set_next_event	= timer_set_next_event,
	.rating		= 300,
	.cpumask	= cpu_all_mask,
};

static void __init systick_clockevents_init(unsigned int timer_irq)
{
	systick_clockevent.irq = timer_irq;
	systick_clockevent.mult =
		div_sc(1000000, NSEC_PER_SEC, systick_clockevent.shift);
	systick_clockevent.max_delta_ns =
		clockevent_delta2ns(0xffffff, &systick_clockevent);
	systick_clockevent.min_delta_ns =
		clockevent_delta2ns(0x1, &systick_clockevent);

	clockevents_register_device(&systick_clockevent);
}

/*
 * IRQ handler for the timer
 */
static irqreturn_t systick_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = &systick_clockevent;

	evt->event_handler(evt);
	if (evt->mode == CLOCK_EVT_MODE_ONESHOT) {
		u32 control;
		control = readl(SYSTICK);
		control &= ~TICKINT;
		writel(control, SYSTICK);
	}
	return IRQ_HANDLED;
}

static struct irqaction systick_irq = {
	.name		= "Cortex-M  SysTick",
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= systick_interrupt,
};

/*
 * Set up the clock events devices
 */
void __init fm3_timer_init(unsigned int timer_irq)
{
	fm3_clock_init(4000000, 32768);
	/* 
	 * Make irqs happen for the system timer
	 */
	setup_irq(timer_irq, &systick_irq);

	systick_clockevents_init(timer_irq);
}
