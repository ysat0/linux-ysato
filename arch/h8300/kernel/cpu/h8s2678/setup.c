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
#include <asm/timer_tmu.h>

static struct plat_sci_port sci_platform_data[] = {
	{
		.mapbase	= 0xffff78,
		.flags		= UPF_BOOT_AUTOCONF,
		.type		= PORT_SCI,
		.irqs		= { 88, 89, 90, 0 },
	}, {
		.mapbase	= 0xffff80,
		.flags		= UPF_BOOT_AUTOCONF,
		.type		= PORT_SCI,
		.irqs		= { 92, 93, 94, 0 },
	}, {
		.mapbase	= 0xffff88,
		.flags		= UPF_BOOT_AUTOCONF,
		.type		= PORT_SCI,
		.irqs		= { 96, 97, 98, 0 },
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
	.name		= "h8300_tm8",
	.id		= 0,
	.dev = {
		.platform_data	= NULL
	},
	.resource	= tm8_unit0_resources,
	.num_resources	= ARRAY_SIZE(tm8_unit0_resources),
};

static struct h8300_tmu_data tmudata = {
	.num		= 6,
	.ch		= {
		{
			.base	= 0xffffd0,
			.irqs	= {40,41,42,43},
			.numdiv	= 4,
			.div	= {1, 4, 16, 64},
		}, {
			.base	= 0xffffe0,
			.irqs	= {48, 49, 50, 51},
			.numdiv	= 5,
			.div	= {1, 4, 16, 64, 256},
		}, {
			.base	= 0xfffff0,
			.irqs	= {52, 53, 54, 55},
			.numdiv	= 5,
			.div	= {1, 4, 16, 64, 256},
		}, {
			.base	= 0xfffe80,
			.irqs	= {56, 57, 58, 59},
			.numdiv	= 7,
			.div	= {1, 4, 16, 64, 256, 1024, 4096},
		}, {
			.base	= 0xfffe90,
			.irqs	= {64, 65, 66, 67},
			.numdiv	= 5,
			.div	= {1, 4, 16, 64, 1024},
		}, {
			.base	= 0xfffea0,
			.irqs	= {68, 69, 70, 71},
			.numdiv	= 5,
			.div	= {1, 4, 16, 64, 256},
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
