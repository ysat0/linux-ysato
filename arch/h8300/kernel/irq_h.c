/*
 * linux/arch/h8300/kernel/irq_h.c
 *
 * Copyright 2014 Yoshinori Sato <ysato@users.sourceforge.jp>
 */

#include <linux/init.h>
#include <linux/irq.h>
#include <asm/io.h>

static const char ipr_bit[] = {
	 7,  6,  5,  5,
	 4,  4,  4,  4,  3,  3,  3,  3,
	 2,  2,  2,  2,  1,  1,  1,  1,
	 0,  0,  0,  0, 15, 15, 15, 15,
	14, 14, 14, 14, 13, 13, 13, 13,
	-1, -1, -1, -1, 11, 11, 11, 11,
	10, 10, 10, 10,  9,  9,  9,  9,
};

#define IPR 0xffee18

static void h8300h_disable_irq(struct irq_data *data)
{
	int bit;
	unsigned int addr;
	int irq = data->irq - 12;

	bit = ipr_bit[irq];
	if (bit >= 0) {
		addr = IPR + (irq >> 3);
		ctrl_bclr(bit & 7, addr);
	}
}

static void h8300h_enable_irq(struct irq_data *data)
{
	int bit;
	unsigned int addr;
	int irq = data->irq - 12;

	bit = ipr_bit[irq];
	if (bit >= 0) {
		addr = IPR + (irq >> 3);
		ctrl_bset(bit & 7, addr);
	}
}

struct irq_chip h8300h_irq_chip = {
	.name		= "H8/300H-INTC",
	.irq_enable	= h8300h_enable_irq,
	.irq_disable	= h8300h_disable_irq,
};

void __init h8300_init_ipr(void)
{
	/* All interrupt priority high */
	ctrl_outb(0xff, IPR + 0);
	ctrl_outb(0xee, IPR + 1);
}
