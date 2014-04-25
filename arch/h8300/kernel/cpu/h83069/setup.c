/*
 * H8/3069 Internal peripheral setup
 *
 *  Copyright (C) 2009  Yoshinori Sato <ysato@users.sourceforge.jp>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/platform_device.h>
#include <linux/serial_sci.h>
#include <asm/timer16.h>

static struct plat_sci_port sci_platform_data0 = {
	.mapbase	= 0xffffb0,
	.flags		= UPF_BOOT_AUTOCONF,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_5,
	.type		= PORT_SCI,
	.irqs		= { 52, 53, 54, 0 },
};

static struct plat_sci_port sci_platform_data1 = {
	.mapbase	= 0xffffb8,
	.flags		= UPF_BOOT_AUTOCONF,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_5,
	.type		= PORT_SCI,
	.irqs		= { 56, 57, 58, 0 },
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

static struct resource tm8_unit0_resources[] = {
	[0] = {
		.name	= "8bit timer 0",
		.start	= 0xffff80,
		.end	= 0xffff89,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= 36,
		.end	= 43,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device tm8_unit0_device = {
	.name		= "h8300_8timer",
	.id		= 0,
	.dev = {
		.platform_data	= NULL
	},
	.resource	= tm8_unit0_resources,
	.num_resources	= ARRAY_SIZE(tm8_unit0_resources),
};

static struct resource tm8_unit1_resources[] = {
	[0] = {
		.name	= "8bit timer 1",
		.start	= 0xffff90,
		.end	= 0xffff99,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= 44,
		.end	= 51,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device tm8_unit1_device = {
	.name		= "h8300_8timer",
	.id		= 1,
	.dev = {
		.platform_data	= NULL
	},
	.resource	= tm8_unit1_resources,
	.num_resources	= ARRAY_SIZE(tm8_unit1_resources),
};


static struct h8300_timer16_data timer16data0 = {
	.common	= 0xffff60,
	.base	= 0xffff68,
	.irqs	= {24, 25, 26},
	.enable	= 0,
	.imfa	= 0,
	.imie	= 4,
};

static struct h8300_timer16_data timer16data1 = {
	.common	= 0xffff60,
	.base	= 0xffff70,
	.irqs	= {28, 29, 30},
	.enable	= 1,
	.imfa	= 1,
	.imie	= 5,
};

static struct h8300_timer16_data timer16data2 = {
	.common	= 0xffff60,
	.base	= 0xffff78,
	.irqs	= {32, 33, 34},
	.enable	= 2,
	.imfa	= 2,
	.imie	= 6,
};

static struct platform_device timer16_ch0_device = {
	.name	= "h8300h-16timer",
	.id		= 0,
	.dev		= {
		.platform_data	= &timer16data0,
	},
};

static struct platform_device timer16_ch1_device = {
	.name	= "h8300h-16timer",
	.id		= 1,
	.dev		= {
		.platform_data	= &timer16data1,
	},
};

static struct platform_device timer16_ch2_device = {
	.name	= "h8300h-16timer",
	.id		= 2,
	.dev		= {
		.platform_data	= &timer16data2,
	},
};

static struct platform_device *devices[] __initdata = {
	&tm8_unit0_device,
	&tm8_unit1_device,
	&timer16_ch0_device,
	&timer16_ch1_device,
	&timer16_ch2_device,
	&sci0_device,
	&sci1_device,
};

static int __init devices_register(void)
{
	return platform_add_devices(devices,
				    ARRAY_SIZE(devices));
}

arch_initcall(devices_register);

void __init early_device_register(void)
{
	early_platform_add_devices(devices,
				   ARRAY_SIZE(devices));
}
