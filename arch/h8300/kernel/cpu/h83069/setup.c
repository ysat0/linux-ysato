/*
 * H8/3069 Internal peripheral setup
 *
 *  Copyright (C) 2009,2014 Yoshinori Sato <ysato@users.sourceforge.jp>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/platform_device.h>
#include <linux/serial_sci.h>
#include <linux/init.h>
#include <linux/io.h>
#include <asm/timer.h>

static struct resource sci0_resources[] = {
	DEFINE_RES_MEM(0xffffb0, 8),
	DEFINE_RES_IRQ(52),
};


static struct plat_sci_port sci_platform_data0 = {
	.flags		= UPF_BOOT_AUTOCONF,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.type		= PORT_SCI,
};

static struct resource sci1_resources[] = {
	DEFINE_RES_MEM(0xffffb8, 8),
	DEFINE_RES_IRQ(56),
};

static struct plat_sci_port sci_platform_data1 = {
	.flags		= UPF_BOOT_AUTOCONF,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.type		= PORT_SCI,
};

static struct platform_device sci0_device = {
	.name		= "sh-sci",
	.id		= 0,
	.resource	= sci0_resources,
	.dev		= {
		.platform_data	= &sci_platform_data0,
	},
};

static struct platform_device sci1_device = {
	.name		= "sh-sci",
	.id		= 1,
	.resource	= sci1_resources,
	.dev		= {
		.platform_data	= &sci_platform_data1,
	},
};

static struct h8300_timer8_config timer8_platform_data = {
	.mode	= MODE_CS,
	.div	= DIV_64,
	.rating = 200,
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
		.platform_data	= &timer8_platform_data,
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
		.platform_data	= &timer8_platform_data,
	},
	.resource	= tm8_unit1_resources,
	.num_resources	= ARRAY_SIZE(tm8_unit1_resources),
};


static struct h8300_timer16_config timer16data0 = {
	.rating = 200,
	.enable	= 0,
	.imfa	= 0,
	.imiea	= 4,
};

static struct h8300_timer16_config timer16data1 = {
	.rating = 200,
	.enable	= 1,
	.imfa	= 1,
	.imiea	= 5,
};

static struct h8300_timer16_config timer16data2 = {
	.rating = 200,
	.enable	= 2,
	.imfa	= 2,
	.imiea	= 6,
};

static struct resource tm16ch0_resources[] = {
	[0] = {
		.name	= "16bit timer 0",
		.start	= 0xffff68,
		.end	= 0xffff6f,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= 0xffff60,
		.end	= 0xffff66,
		.flags	= IORESOURCE_MEM,
	},
	[2] = {
		.start	= 24,
		.end	= 26,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct resource tm16ch1_resources[] = {
	[0] = {
		.name	= "16bit timer 1",
		.start	= 0xffff70,
		.end	= 0xffff77,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= 0xffff60,
		.end	= 0xffff66,
		.flags	= IORESOURCE_MEM,
	},
	[2] = {
		.start	= 28,
		.end	= 30,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct resource tm16ch2_resources[] = {
	[0] = {
		.name	= "16bit timer 2",
		.start	= 0xffff78,
		.end	= 0xffff7f,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= 0xffff60,
		.end	= 0xffff66,
		.flags	= IORESOURCE_MEM,
	},
	[2] = {
		.start	= 32,
		.end	= 34,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device timer16_ch0_device = {
	.name	= "h8300h-16timer",
	.id		= 0,
	.dev		= {
		.platform_data	= &timer16data0,
	},
	.resource	= tm16ch0_resources,
	.num_resources	= ARRAY_SIZE(tm16ch0_resources),
};

static struct platform_device timer16_ch1_device = {
	.name	= "h8300h-16timer",
	.id		= 1,
	.dev		= {
		.platform_data	= &timer16data1,
	},
	.resource	= tm16ch1_resources,
	.num_resources	= ARRAY_SIZE(tm16ch1_resources),
};

static struct platform_device timer16_ch2_device = {
	.name	= "h8300h-16timer",
	.id		= 2,
	.dev		= {
		.platform_data	= &timer16data2,
	},
	.resource	= tm16ch2_resources,
	.num_resources	= ARRAY_SIZE(tm16ch2_resources),
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

static struct platform_device *early_devices[] __initdata = {
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
	early_platform_add_devices(early_devices,
				   ARRAY_SIZE(devices));
}
