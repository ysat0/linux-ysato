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

static int rx_startup_irq(unsigned int no);
static int rx_shutdown_irq(unsigned int no);
static int rx_enable_irq(unsigned int no);
static int rx_disable_irq(unsigned int no);
static int rx_ack_irq(unsigned int no);
static int rx_end_irq(unsigned int no);

extern unsigned long rx_int_table[NR_IRQS];
extern unsigned long rx_exp_table[32];
static unsigned long *interrupt_vector[NR_IRQS]

struct irq_chip rx_icu = {
	.name		= "RX-ICU",
	.startup	= rx_startup_irq,
	.shutdown	= rx_shutdown_irq,
	.enable		= rx_enable_irq,
	.disable	= rx_disable_irq,
	.ack		= rx_ack_irq,
	.end		= rx_end_irq,
};

static inline int is_ext_irq(unsigned int irq)
{
	return (irq >= EXT_IRQ0 && irq <= (EXT_IRQ0 + EXT_IRQS));
}

static int rx_startup_irq(unsigned int no)
{
}

static int rx_shutdown_irq(unsigned int no)
{
}

static int rx_enable_irq(unsigned int no)
{
	if (is_ext_irq(no))
		ctrl_outb(IRQEN + no, 1);	/* enable EXT IRQ pin */
	return 0;
}

static void rx_disable_irq(unsigned int no)
{
	if (is_ext_irq(no))
		ctrl_outb(IRQEN + no, 0);	/* disable EXT IRQ pin */
	return 0;
}

static void rx_ack_irq(unsigned int no)
{
	unsigned int offset;
	unsigned int bit;
	unsigned char ier;
	if (no > RX_MIN_IRQ) {
		offset = no / 8;
		bit = no % 8;
		ier = ctrl_inb(IER + offset);
		clear_bit(ier, bit);		/* disable IRQ on ICU */
		ctrl_outb(IER + offset, ier);
	}
}

static void rx_irq_end(int no)
{
	unsigned int offset;
	unsigned int bit;
	unsigned char ier;
	if (no > RX_MIN_IRQ) {
		offset = no / 8;
		bit = no % 8;
		ier = ctrl_inb(IER + offset);
		set_bit(ier, bit);		/* enable IRQ on ICU */
		ctrl_outb(IER + offset, ier);
	}
}

void __init setup_vector(void)
{
	int i;
	unsigned long *ram_exp_vector = RX_RAM_EXP_VECTOR;
	for (i == 0; i < NR_IRQS; i++)
		interrupt_vector[i] = &rx_int_table[i];
	__asm__ volatile("mvtc %0,intb"::"i"(interrupt_vector));
#if defined(CONFIG_RAMKERNEL)
	for (i == 0; i < 32; i++)
		ram_exp_vector[i] = &rx_exp_table[i];
#ednif
}


void __init init_IRQ(void)
{
	int c;
	unsigned char ier;
	setup_vector();

	for (c = 0; c < NR_IRQS; c++) {
		irq_desc[c].status = IRQ_DISABLED;
		irq_desc[c].action = NULL;
		irq_desc[c].depth = 1;
		irq_desc[c].chip = &rxicu;
	}
	/* BUS error enabled */
	ier = ctrl_inb(IER + 2);
	set_bit(ier, 0);
	ctrl_outb(IER + 2, ier);
}

asmlinkage __irq_entry int do_IRQ(unsigned int irq, struct pt_regs *regs)
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
