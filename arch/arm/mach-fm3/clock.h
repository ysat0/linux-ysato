/*
 *  linux/arch/arm/mach-fm3/clock.h
 *
 *  Copyright (C) 2012 Yoshinori Sato
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

struct clk {
	struct list_head node;
	const char	*name;
	struct device	*dev;
	unsigned long	rate_hz;
	struct clk	*parent;
	void		(*mode)(struct clk *, int);
	unsigned int	id;
};
