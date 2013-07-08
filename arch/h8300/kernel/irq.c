/*
 * linux/arch/h8300/kernel/irq.c
 *
 * Copyright 2007 Yoshinori Sato <ysato@users.sourceforge.jp>
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/kernel_stat.h>
#include <linux/seq_file.h>
#include <linux/init.h>
#include <linux/random.h>
#include <linux/bootmem.h>
#include <linux/irq.h>
#include <linux/interrupt.h>

#include <asm/traps.h>
#include <asm/io.h>
#include <asm/setup.h>
#include <asm/errno.h>

/*#define DEBUG*/

extern unsigned long *interrupt_redirect_table;
extern const int h8300_saved_vectors[];
extern const h8300_vector h8300_trap_table[];
int h8300_enable_irq_pin(unsigned int irq);
void h8300_disable_irq_pin(unsigned int irq);

#define CPU_VECTOR ((unsigned long *)0x000000)
#define ADDR_MASK (0xffffff)

#if defined(CONFIG_CPU_H8300H)
const static char ipr_bit[] = {
	 7,  6,  5,  5,
	 4,  4,  4,  4,  3,  3,  3,  3,
	 2,  2,  2,  2,  1,  1,  1,  1,
	 0,  0,  0,  0, 15, 15, 15, 15,
	14, 14, 14, 14, 13, 13, 13, 13,
	-1, -1, -1, -1, 11, 11, 11, 11,
	10, 10, 10, 10,  9,  9,  9,  9,
};
#endif

#if defined(CONFIG_CPU_H8S)
const static unsigned char ipr_table[] = {
	0x03, 0x02, 0x01, 0x00, 0x13, 0x12, 0x11, 0x10, /* 16 - 23 */
	0x23, 0x22, 0x21, 0x20, 0x33, 0x32, 0x31, 0x30, /* 24 - 31 */
	0x43, 0x42, 0x41, 0x40, 0x53, 0x53, 0x52, 0x52, /* 32 - 39 */
	0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, /* 40 - 47 */
	0x50, 0x50, 0x50, 0x50, 0x63, 0x63, 0x63, 0x63, /* 48 - 55 */
	0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, /* 56 - 63 */
	0x61, 0x61, 0x61, 0x61, 0x60, 0x60, 0x60, 0x60, /* 64 - 71 */
	0x73, 0x73, 0x73, 0x73, 0x72, 0x72, 0x72, 0x72, /* 72 - 79 */
	0x71, 0x71, 0x71, 0x71, 0x70, 0x83, 0x82, 0x81, /* 80 - 87 */
	0x80, 0x80, 0x80, 0x80, 0x93, 0x93, 0x93, 0x93, /* 88 - 95 */
	0x92, 0x92, 0x92, 0x92, 0x91, 0x91, 0x91, 0x91, /* 96 - 103 */
	0x90, 0x90, 0x90, 0x90, 0xa3, 0xa3, 0xa3, 0xa3, /* 104 - 111 */
	0xa2, 0xa2, 0xa2, 0xa2, 0xa1, 0xa1, 0xa1, 0xa1, /* 112 - 119 */
	0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, /* 120 - 127 */
};
#endif

static inline int is_ext_irq(unsigned int irq)
{
	return (irq >= EXT_IRQ0 && irq <= (EXT_IRQ0 + EXT_IRQS));
}

#if defined(CONFIG_CPU_H8300H)
static void h8300_disable_irq(struct irq_data *data)
{
	int bit;
	unsigned int addr;
	int irq = data->irq;
	if ((bit = ipr_bit[irq - 12]) >= 0) {
		addr = (bit < 8)?IPRA:IPRB;
		ctrl_bclr(bit & 7, addr);
	}
}

static void h8300_enable_irq(struct irq_data *data)
{
	int bit;
	unsigned int addr;
	int irq = data->irq;
	if ((bit = ipr_bit[irq]) >= 0) {
		addr = (bit < 8)?IPRA:IPRB;
		ctrl_bset(bit & 7, addr);
	}
	if (is_ext_irq(irq))
		ctrl_bclr((irq - EXT_IRQ0), ISR);
}
#endif
#if defined(CONFIG_CPU_H8S)
static void h8300_disable_irq(struct irq_data *data)
{
	int pos;
	unsigned int addr;
	unsigned short pri;
	int irq = data->irq;
	addr = IPRA + ((ipr_table[irq - 16] & 0xf0) >> 3);
	pos = (ipr_table[irq - 16] & 0x0f) * 4;
	pri = ~(0x000f << pos);
	pri &= ctrl_inw(addr);
	ctrl_outw(pri, addr);
	if (is_ext_irq(irq))
		ISR_REGS &= ~(1 << (irq - EXT_IRQ0));
}

static void h8300_enable_irq(struct irq_data *data)
{
	int pos;
	unsigned int addr;
	unsigned short pri;
	int irq = data->irq;
	addr = IPRA + ((ipr_table[irq - 16] & 0xf0) >> 3);
	pos = (ipr_table[irq - 16] & 0x0f) * 4;
	pri = ~(0x000f << pos);
	pri &= ctrl_inw(addr);
	pri |= 1 << pos;
	ctrl_outw(pri, addr);
	if (is_ext_irq(irq))
		ISR_REGS &= ~(1 << (irq - EXT_IRQ0));
}
#endif

static unsigned int h8300_startup_irq(struct irq_data *data)
{
	if (is_ext_irq(data->irq))
		return h8300_enable_irq_pin(data->irq);
	else
		return 0;
}

static void h8300_shutdown_irq(struct irq_data *data)
{
	if (is_ext_irq(data->irq))
		h8300_disable_irq_pin(data->irq);
}

/*
 * h8300 interrupt controller implementation
 */
struct irq_chip h8300irq_chip = {
	.name		= "H8300-INTC",
	.irq_startup	= h8300_startup_irq,
	.irq_shutdown	= h8300_shutdown_irq,
	.irq_enable	= h8300_enable_irq,
	.irq_disable	= h8300_disable_irq,
};

#if defined(CONFIG_RAMKERNEL)
static unsigned long __init *get_vector_address(void)
{
	unsigned long *rom_vector = CPU_VECTOR;
	unsigned long base,tmp;
	int vec_no;

	base = rom_vector[EXT_IRQ0] & ADDR_MASK;

	/* check romvector format */
	for (vec_no = EXT_IRQ1; vec_no <= EXT_IRQ0+EXT_IRQS; vec_no++) {
		if ((base+(vec_no - EXT_IRQ0)*4) != (rom_vector[vec_no] & ADDR_MASK))
			return NULL;
	}

	/* ramvector base address */
	base -= EXT_IRQ0*4;

	/* writerble check */
	tmp = ~(*(volatile unsigned long *)base);
	(*(volatile unsigned long *)base) = tmp;
	if ((*(volatile unsigned long *)base) != tmp)
		return NULL;
	return (unsigned long *)base;
}

static void __init setup_vector(void)
{
	int i;
	unsigned long *ramvec,*ramvec_p;
	const h8300_vector *trap_entry;
	const int *saved_vector;

	ramvec = get_vector_address();
	if (ramvec == NULL)
		panic("interrupt vector serup failed.");
	else
		printk(KERN_INFO "virtual vector at 0x%08lx\n",(unsigned long)ramvec);

	/* create redirect table */
	ramvec_p = ramvec;
	trap_entry = h8300_trap_table;
	saved_vector = h8300_saved_vectors;
	for ( i = 0; i < NR_IRQS; i++) {
		if (i == *saved_vector) {
			ramvec_p++;
			saved_vector++;
		} else {
			if ( i < NR_TRAPS ) {
				if (*trap_entry)
					*ramvec_p = VECTOR(*trap_entry);
				ramvec_p++;
				trap_entry++;
			} else
				*ramvec_p++ = REDIRECT(interrupt_entry);
		}
	}
	interrupt_redirect_table = ramvec;
#ifdef DEBUG
	ramvec_p = ramvec;
	for (i = 0; i < NR_IRQS; i++) {
		if ((i % 8) == 0)
			printk(KERN_DEBUG "\n%p: ",ramvec_p);
		printk(KERN_DEBUG "%p ",*ramvec_p);
		ramvec_p++;
	}
	printk(KERN_DEBUG "\n");
#endif
}
#else
#define setup_vector() do { } while(0)
#endif

/* Set all interrupt level 1 */
#if defined(CONFIG_CPU_H8300H)
void __init init_ipr(void)
{
	ctrl_outb(0xff, IPRA);
	ctrl_outb(0xee, IPRB);
}
#endif
#if defined(CONFIG_CPU_H8S)
void __init init_ipr(void)
{
	int n;
	/* IPRA to IPRK */
	for (n = 0; n <= 'k' - 'a'; n++)
		ctrl_outw(0x1111, IPRA + (n * 2));
}
#endif

void __init init_IRQ(void)
{
	int c;

	setup_vector();

	for (c = 0; c < NR_IRQS; c++)
		irq_set_chip_and_handler(c, &h8300irq_chip, handle_simple_irq);
}

asmlinkage void do_IRQ(int irq)
{
	irq_enter();
	generic_handle_irq(irq);
	irq_exit();
}
