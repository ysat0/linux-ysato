/*
 * linux/arch/h8300/kernel/clk.c
 *
 * Copyright 2015 Yoshinori Sato <ysato@users.sourceforge.jp>
 */

#include <linux/clk.h>
#include <linux/module.h>
#include <linux/compiler.h>
#include <linux/clkdev.h>
#include <asm/clk.h>

unsigned long clk_get_rate(struct clk *clk)
{
	return clk->parent->rate;
}
EXPORT_SYMBOL(clk_get_rate)

int clk_enable(struct clk *clk)
{
	return 0;
}
EXPORT_SYMBOL(clk_enable)

void clk_disable(struct clk *clk)
{
}
EXPORT_SYMBOL(clk_disable)

static struct clk master_clk;

static struct clk peripheral_clk = {
	.parent = &master_clk,
};

static struct clk_lookup lookups[] = {
	{
		.con_id = "master_clk",
		.clk = &master_clk,
	},
	{
		.con_id = "peripheral_clk",
		.clk = &peripheral_clk,
	},
};

int __init h8300_clk_init(unsigned long hz)
{
	master_clk.rate = hz;
	clkdev_add_table(lookups, ARRAY_SIZE(lookups));
	return 0;
}
