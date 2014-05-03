/*
 * H8S2678 Internal peripheral setup
 *
 *  Copyright (C) 2014  Yoshinori Sato <ysato@users.sourceforge.jp>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/platform_device.h>
#include <linux/serial_sci.h>
#include <asm/timer.h>

static struct plat_sci_port sci_platform_data0 = {
	.mapbase	= 0xffff78,
	.flags		= UPF_BOOT_AUTOCONF,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_5,
	.type		= PORT_SCI,
	.irqs		= { 88, 89, 90, 0 },
};

static struct plat_sci_port sci_platform_data1 = {
	.mapbase	= 0xffff80,
	.flags		= UPF_BOOT_AUTOCONF,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_5,
	.type		= PORT_SCI,
	.irqs		= { 92, 93, 94, 0 },
};

static struct plat_sci_port sci_platform_data2 = {
	.mapbase	= 0xffff88,
	.flags		= UPF_BOOT_AUTOCONF,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_5,
	.type		= PORT_SCI,
	.irqs		= { 96, 97, 98, 0 },
};

static struct platform_device sci0_device = {
	.name		= "sh-sci",
	.id		= 0,
	.dev		= {
		.platform_data	= &sci_platform_data0,
	},
};

static struct platform_device sci1_device = {
	.name		= "sh-sci",
	.id		= 1,
	.dev		= {
		.platform_data	= &sci_platform_data1,
	},
};

static struct platform_device sci2_device = {
	.name		= "sh-sci",
	.id		= 2,
	.dev		= {
		.platform_data	= &sci_platform_data2,
	},
};

static struct h8300_timer8_config timer8_platform_data = {
	.mode	= MODE_CED,
	.div	= DIV_8,
	.rating = 200,
};

static struct resource tm8_unit0_resources[] = {
	[0] = {
		.name	= "8bit timer 0",
		.start	= 0xffffb0,
		.end	= 0xffffb9,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= 72,
		.end	= 79,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device tm8_unit0_device = {
	.name		= "h8300_8timer",
	.id		= 0,
	.dev = {
		.platform_data	= &timer8_platform_data,
	},
	.resource	= tm8_unit0_resources,
	.num_resources	= ARRAY_SIZE(tm8_unit0_resources),
};

static struct h8300_tpu_config tpu12data = {
	.rating = 200,
};

static struct h8300_tpu_config tpu45data = {
	.rating = 200,
};

static struct resource tpu12_resources[] = {
	[0] = {
		.name	= "tpu ch1-2",
		.start	= 0xffffe0,
		.end	= 0xffffef,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= 0xfffff0,
		.end	= 0xfffffb,
		.flags	= IORESOURCE_MEM,
	},
};

static struct resource tpu45_resources[] = {
	[0] = {
		.name	= "tpu ch4-5",
		.start	= 0xfffe90,
		.end	= 0xfffe9f,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= 0xfffea0,
		.end	= 0xfffeab,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device tpu12_device = {
	.name	= "h8s tpu",
	.id		= 0,
	.dev		= {
		.platform_data	= &tpu12data,
	},
	.resource	= tpu12_resources,
	.num_resources	= ARRAY_SIZE(tpu12_resources),
};

static struct platform_device tpu45_device = {
	.name	= "h8s tpu",
	.id		= 1,
	.dev		= {
		.platform_data	= &tpu45data,
	},
	.resource	= tpu45_resources,
	.num_resources	= ARRAY_SIZE(tpu45_resources),
};

static struct platform_device *devices[] __initdata = {
	&tm8_unit0_device,
	&tpu12_device,
	&tpu45_device,
	&sci0_device,
	&sci1_device,
	&sci2_device,
};

static struct platform_device *early_devices[] __initdata = {
	&tm8_unit0_device,
	&sci0_device,
	&sci1_device,
	&sci2_device,
};

static int __init devices_register(void)
{
	return platform_add_devices(devices,
				    ARRAY_SIZE(devices));
}
arch_initcall(devices_register);

void __init early_device_register(void)
{
	early_platform_add_devices(early_devices,
				   ARRAY_SIZE(devices));
}
