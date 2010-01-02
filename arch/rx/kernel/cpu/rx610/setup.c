/*
 * RX610 Internal peripheral setup
 *
 *  Copyright (C) 2009  Yoshinori Sato <ysato@users.sourceforge.jp>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/platform_device.h>
#include <linux/serial_sci.h>

static struct plat_sci_port sci_platform_data[] = {
	{
		.mapbase	= 0x00088240,
		.flags		= UPF_BOOT_AUTOCONF,
		.type		= PORT_SCI,
		.irqs		= { 214, 215, 216, 0 },
	}, {
		.mapbase	= 0x00088248,
		.flags		= UPF_BOOT_AUTOCONF,
		.type		= PORT_SCI,
		.irqs		= { 218, 219, 220, 0 },
	}, {
		.mapbase	= 0x00088250,
		.flags		= UPF_BOOT_AUTOCONF,
		.type		= PORT_SCI,
		.irqs		= { 222, 223, 224, 0 },
	}, {
		.mapbase	= 0x00088258,
		.flags		= UPF_BOOT_AUTOCONF,
		.type		= PORT_SCI,
		.irqs		= { 226, 227, 228, 0 },
	}, {
		.mapbase	= 0x00088260,
		.flags		= UPF_BOOT_AUTOCONF,
		.type		= PORT_SCI,
		.irqs		= { 230, 231, 232, 0 },
	}, {
		.mapbase	= 0x00088268,
		.flags		= UPF_BOOT_AUTOCONF,
		.type		= PORT_SCI,
		.irqs		= { 234, 235, 236, 0 },
	}, {
		.mapbase	= 0x00088270,
		.flags		= UPF_BOOT_AUTOCONF,
		.type		= PORT_SCI,
		.irqs		= { 238, 239, 240, 0 },
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

static struct platform_device *devices[] __initdata = {
	&sci_device,
};

static int __init devices_register(void)
{
	return platform_add_devices(devices,
				    ARRAY_SIZE(devices));
}
arch_initcall(devices_register);
