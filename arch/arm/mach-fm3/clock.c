/*
 *  linux/arch/arm/mach-fm3/clock.c
 *
 *  Copyright (C) 2012 Yoshinori Sato
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <asm/io.h>
#include <asm/clkdev.h>
#include <mach/platform.h>

#include "clock.h"

static DEFINE_SPINLOCK(clk_lock);

static struct clk *master;
static struct clk clkmo = {
	.name		= "clkmo",
	.rate_hz	= 0,
	.id		= 0,
};
static struct clk clkso = {
	.name		= "clkso",
	.rate_hz	= 0,
	.id		= 1,
};
static struct clk clkhc = {
	.name		= "clkhc",
	.rate_hz	= 4000000,
	.id		= 2,
};
static struct clk clklc = {
	.name		= "clklc",
	.rate_hz	= 100000,
	.id		= 3,
};
static struct clk clkpll = {
	.name		= "clkpll",
	.parent		= &clkmo,
	.id		= 4,
};
static struct clk hclk = {
	.name		= "hclk",
	.id		= 5,
};
static struct clk tpiuclk = {
	.name		= "tpiuclk",
	.parent		= &hclk,
	.id		= 6,
};
static struct clk pclk0 = {
	.name		= "pclk0",
	.parent		= &hclk,
	.id		= 7,
};
static struct clk pclk1 = {
	.name		= "pclk1",
	.parent		= &hclk,
	.id		= 8,
};
static struct clk pclk2 = {
	.name		= "pclk2",
	.parent		= &hclk,
	.id		= 9,
};
static struct clk swdogclk = {
	.name		= "swdogclk",
	.parent		= &pclk0,
	.id		= 10,
};

int clk_enable(struct clk *clk)
{
	return 0;
}
EXPORT_SYMBOL(clk_enable);

void clk_disable(struct clk *clk)
{
}
EXPORT_SYMBOL(clk_disable);

unsigned long clk_get_rate(struct clk *clk)
{
	return clk->rate_hz;
}
EXPORT_SYMBOL(clk_get_rate);

long clk_round_rate(struct clk *clk, unsigned long rate)
{
	unsigned long base_rate;
	const static int div[] = {1, 2, 4, 8};
	int i;
	if (clk->parent) {
		base_rate = clk_get_rate(clk->parent);
	} else {
		base_rate = clk->rate_hz;
	}
	for (i = 0; i < ARRAY_SIZE(div); i++) {
		if (rate < (base_rate / div[i]))
			return base_rate / div[i];
	}
	return -1;
}
EXPORT_SYMBOL(clk_round_rate);

int clk_set_rate(struct clk *clk, unsigned long rate)
{
	int div;
	unsigned char psr;
	static const unsigned long psr_addr[] = {FM3_TTC_PSR, FM3_APBC0_PSR, FM3_APBC1_PSR, FM3_APBC2_PSR };
	unsigned long flags;
	spin_lock_irqsave(&clk_lock, flags);
	switch (clk->id) {
	case 5: /* hclk */
		div = master->rate_hz / rate;
		if (div < 5) 
			writeb(div - 1, FM3_BSC_PSR);
		else
			switch(div) {
			case 6:
				writeb(4, FM3_BSC_PSR);
				break;
			case 8:
				writeb(5, FM3_BSC_PSR);
				break;
			case 16:
				writeb(6, FM3_BSC_PSR);
				break;
			}
		break;
	case 6: /* tpuiclk */
	case 7: /* pclk0 */
	case 8: /* pclk1 */
	case 9: /* pclk2 */
		psr = readb(psr_addr[clk->id - 6]);
		div = clk_get_rate(clk->parent) / rate;
		div /= 2;
		psr &= ~0x03;
		psr |= div;
		writeb(psr, psr_addr[clk->id - 6]);
		break;
	case 10:
		div = clk_get_rate(clk->parent) / rate;
		div /= 2;
		writeb(0x80 | div, FM3_SWC_PSR);
		break;
	}
	clk->rate_hz = rate;
	spin_unlock_irqrestore(&clk_lock, flags);
	return 0;
}
EXPORT_SYMBOL(clk_set_rate);

static struct  clk_lookup lookups[] = {
	{
		.con_id	= "hcr",
		.clk	= &clkhc,
	}, {
		.con_id	= "main",
		.clk	= &clkmo,
	}, {
		.con_id	= "lcr",
		.clk	= &clklc,
	}, {
		.con_id = "sub",
		.clk	= &clkso,
	}, {
		.con_id	= "pll",
		.clk	= &clkpll,
	}, {
		.con_id = "hclk",
		.clk	= &hclk,
	}, {
		.con_id	= "apb0",
		.clk	= &pclk0,
	}, {
		.con_id	= "apb1",
		.clk	= &pclk1,
	}, {
		.con_id	= "apb2",
		.clk	= &pclk2,
	}, {
		.con_id	= "tpiu",
		.clk	= &tpiuclk,
	}, {
		.con_id	= "swdog",
		.clk	= &swdogclk,
	},
};

int __init fm3_clock_init(unsigned int mo_hz, unsigned int so_hz)
{
	static struct clk *rcs[] = { &clkhc, &clkmo, &clkpll, NULL, 
				     &clklc, &clkso, NULL, NULL };
	static const int div[] = { 1, 2, 3, 4, 6, 8, 16, -1 };
	u8 scm_ctl = readb(FM3_SCM_CTL);
	u8 bsc_psr = readb(FM3_BSC_PSR);
#if 0
	u8 pll_ctl1 = readb(FM3_PLL_CTL1);
	u8 pll_ctl2 = readb(FM3_PLL_CTL2);
#else
	u8 pll_ctl1 = 1;
	u8 pll_ctl2 = 35;
#endif
	unsigned int pllin;
	unsigned int pllout;
	int i;

	clkmo.rate_hz = mo_hz;
	clkso.rate_hz = so_hz;
	master = rcs[scm_ctl >> 5];
	if (master == NULL)
		return -EINVAL;
	pllin = clkmo.rate_hz / ((pll_ctl1 >> 4) + 1);
	pllout = pllin * ((pll_ctl1 & 0x0f) + 1 * (pll_ctl2 & 0x3f) + 1);
	clkpll.rate_hz = pllout / ((pll_ctl1 & 0x0f) + 1);
	hclk.parent = master;
	hclk.rate_hz = master->rate_hz / div[bsc_psr & 7];
	for(i = 0; i < ARRAY_SIZE(lookups); i++)
		clkdev_add(&lookups[i]);
	return 0;
}
