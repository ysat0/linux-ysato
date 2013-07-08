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

static struct plat_sci_port sci_platform_data[] = {
	{
		.mapbase	= 0xffffb0,
		.flags		= UPF_BOOT_AUTOCONF,
		.type		= PORT_SCI,
		.irqs		= { 52, 53, 54, 0 },
	}, {
		.mapbase	= 0xffffb8,
		.flags		= UPF_BOOT_AUTOCONF,
		.type		= PORT_SCI,
		.irqs		= { 56, 57, 58, 0 },
	}, {
		.mapbase	= 0xffffc0,
		.flags		= UPF_BOOT_AUTOCONF,
		.type		= PORT_SCI,
		.irqs		= { 60, 61, 62, 0 },
	}, {
		.flags = 0,
	}
};

static struct platform_device sci_device = {
	.name		= "sh-sci",
	.id		= -1,
	.dev		= {
		.platform_data	= sci_platform_data,
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
	.name		= "h8300_tm8",
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
	.name		= "h8300_tm8",
	.id		= 1,
	.dev = {
		.platform_data	= NULL
	},
	.resource	= tm8_unit1_resources,
	.num_resources	= ARRAY_SIZE(tm8_unit1_resources),
};


static struct h8300_timer16_data timer16data = {
	.base		= 0xffff60,
	.num		= 3,
	.ch		= {
		{
			.base	= 0xffff68,
			.irqs	= {24, 25, 26},
			.enable	= 0,
			.imf	= 0,
			.imie	= 4,
		}, {
			.base	= 0xffff70,
			.irqs	= {28, 29, 30},
			.enable	= 1,
			.imf	= 1,
			.imie	= 5,
		}, {
			.base	= 0xffff78,
			.irqs	= {32, 33, 34},
			.enable	= 2,
			.imf	= 2,
			.imie	= 6,
		},
	},
};

static struct platform_device timer16_device = {
	.name		= "h8300-16bitimer",
	.id		= -1,
	.dev		= {
		.platform_data	= &timer16data,
	},
};

static struct platform_device *devices[] __initdata = {
	&tm8_unit0_device,
	&tm8_unit1_device,
	&timer16_device,
	&sci_device,
};

static int __init devices_register(void)
{
	return platform_add_devices(devices,
				    ARRAY_SIZE(devices));
}
arch_initcall(devices_register);
