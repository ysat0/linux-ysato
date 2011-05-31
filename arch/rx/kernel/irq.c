/*
 * arch/rx/kernel/irq.c
 *
 * Copyright (C) 2009 Yoshinori Sato <ysato@users.sourceforge.jp>
 *
 */

#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/seq_file.h>
#include <asm/processor.h>
#include <asm/io.h>

void setup_rx_irq_desc(struct irq_chip *chip);

extern unsigned long rx_int_table[NR_IRQS];
extern unsigned long rx_exp_table[32];
static unsigned long *interrupt_vector[NR_IRQS];

#define EXT_IRQ0 64
#define EXT_IRQS 16
#define IR (0x00087000)
#define IER (0x00087200)
#define IRQER (0x0008c300)

static void rx_ack_irq(unsigned int no)
{
	unsigned int offset;
	unsigned int bit;
	u8 ier;
	if (no > RX_MIN_IRQ) {
		offset = no / 8;
		bit = no % 8;
		ier = __raw_readb((void __iomem *)(IER + offset));
		ier &= ~(1 << bit);		/* disable IRQ on ICU */
		__raw_writeb(ier, (void __iomem *)(IER + offset));
	}
}

static void rx_end_irq(unsigned int no)
{
	unsigned int offset;
	unsigned int bit;
	u8 ier;
	if (no > RX_MIN_IRQ) {
		offset = no / 8;
		bit = no % 8;
		__raw_writeb(0, (void __iomem *)(IR + no));
		ier = __raw_readb((void __iomem *)(IER + offset));
		ier |= (1 << bit);		/* enable IRQ on ICU */
		__raw_writeb(ier, (void __iomem *)(IER + offset));
	}
}

struct irq_chip rx_icu = {
	.name		= "RX-ICU",
	.ack		= rx_ack_irq,
	.end		= rx_end_irq,
};

void __init setup_vector(void)
{
	int i;
	for (i = 0; i < NR_IRQS; i++)
		interrupt_vector[i] = &rx_int_table[i];
	__asm__ volatile("mvtc %0,intb"::"i"(interrupt_vector));
}


void __init init_IRQ(void)
{
	setup_vector();
	setup_rx_irq_desc(&rx_icu);
}

asmlinkage int do_IRQ(unsigned int irq, struct pt_regs *regs)
{
	struct pt_regs *old_regs = set_irq_regs(regs);

	irq_enter();
	generic_handle_irq(irq);
	irq_exit();

	set_irq_regs(old_regs);
	return 1;
}

#if defined(CONFIG_PROC_FS)
int show_interrupts(struct seq_file *p, void *v)
{
	int i = *(loff_t *) v;
	struct irqaction * action;
	unsigned long flags;
	struct irq_desc *desc;

	if (i == 0)
		seq_puts(p, "           CPU0");

	if (i < MIN_IRQ || i >= NR_IRQS)
		return 0;

	desc = irq_to_desc(i);
	if (desc == NULL)
		return 0;
	spin_lock_irqsave(&desc->lock, flags);
	action = desc->action;
	if (action) {
		seq_printf(p, "%3d: ",i);
		seq_printf(p, "%10u ", kstat_irqs(i));
		seq_printf(p, " %14s", desc->chip->name);
		seq_printf(p, "-%-8s", desc->.name);
		seq_printf(p, "  %s", action->name);

		for (action = action->next; action; action = action->next)
			seq_printf(p, ", %s", action->name);
		seq_putc(p, '\n');
	}
	spin_unlock_irqrestore(&desc->lock, flags);
	return 0;
}
#endif
