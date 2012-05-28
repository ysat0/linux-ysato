/*
 *  linux/arch/arm/mach-fm3/board-frk_fm3.c
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
#include <linux/sysdev.h>
#include <linux/amba/bus.h>
#include <linux/amba/mmci.h>

#include <mach/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/leds.h>
#include <asm/mach-types.h>
#include <asm/hardware/gic.h>
#include <asm/hardware/nvic.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>

#include <mach/platform.h>
#include <mach/irqs.h>

#include "core.h"
#include "clock.h"

#if PAGE_OFFSET != PHYS_OFFSET
#error "PAGE_OFFSET != PHYS_OFFSET"
#endif

#if defined(CONFIG_SERIAL_FM3_CONSOLE)
struct resource early_resources[] = {
	[0] = {
		.start = 0x40038400,
	}
};

struct platform_device early_console_device = {
	.name = "FM3 early console port",
	.id = 0,
	.resource = early_resources,
};
#endif

static void __init fm3_map_io(void)
{
}

/*
 * FM3 AMBA devices
 */

#define WATCHDOG_IRQ	{ IRQ_FM3_WDOG, NO_IRQ }
#define RTC_IRQ		{ IRQ_FM3_RTC, NO_IRQ }

#define UART0_IRQ	{ IRQ_FM3_UART0_RX, IRQ_FM3_UART0_TX }
#define UART3_IRQ	{ IRQ_FM3_UART3_RX, IRQ_FM3_UART3_TX }
#define UART4_IRQ	{ IRQ_FM3_UART4_RX, IRQ_FM3_UART4_RX }

AMBA_DEVICE(wdog,  "dev:e1",		WATCHDOG, 0x1000, NULL, 0x0001);
AMBA_DEVICE(rtc,   "dev:e8",		RTC,	0x400, NULL, 0x0002);
AMBA_DEVICE(uart0, "dev:uart0",		UART0,	0x100, NULL, 0x0010);
AMBA_DEVICE(uart3, "dev:uart3",		UART3,	0x100, NULL, 0x0013);
AMBA_DEVICE(uart4, "dev:uart4",		UART4,	0x100, NULL, 0x0014);

static struct amba_device *amba_devs[] __initdata = {
	&uart4_device,
	&uart0_device,
	&uart3_device,
	&wdog_device,
	&rtc_device,
};

static void __init gic_init_irq(void)
{
	nvic_init();
}

static void __init timer_init(void)
{
	unsigned int timer_irq;

	timer_irq = IRQ_FM3_DUAL_TIMER;

	fm3_timer_init(0);
}

static struct sys_timer fm3_timer = {
	.init		= timer_init,
};

static void __init fm3_init(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(amba_devs); i++) {
		struct amba_device *d = amba_devs[i];
		amba_device_register(d, &iomem_resource);
	}
#define GPIO_PFR2	((volatile unsigned int       *)(0x40033008))
#define GPIO_ADE	((volatile unsigned int       *)(0x40033500))
#define GPIO_EPFR07	((volatile unsigned int       *)(0x4003361C))

	*GPIO_EPFR07 = (*GPIO_EPFR07 & 0xFFFFFF00) | 0x00000050;
	*GPIO_PFR2 = *GPIO_PFR2 | 0x00000006;
	*GPIO_ADE  = *GPIO_ADE & 0x7FFFFFFF;
}

MACHINE_START(FM3, "FUJITSU FM3")
	.phys_io	= 0x4000000,
	.io_pg_offst	= (IO_ADDRESS(0x40000000) >> 18) & 0xfffc,
	.boot_params	= PHYS_OFFSET + 0x100,
	.map_io		= fm3_map_io,
	.init_irq	= gic_init_irq,
	.timer		= &fm3_timer,
	.init_machine	= fm3_init,
MACHINE_END
