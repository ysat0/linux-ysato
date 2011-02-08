#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/param.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/profile.h>

#include <asm/io.h>
#include <asm/irq.h>

static const u32 base[] = {0x00088002, 0x00088008, 0x00088012, 0x00088018};
static void __iomem *str[] = {(void __iomem *)0x00088000, 
			      (void __iomem *)0x00088010};
static const int irqno[] = {28, 29, 30, 31};

enum {CMCR = 0, CMCNT = 2, CMCOR = 4};

/*
 * timer_interrupt() needs to keep up the real-time clock,
 * as well as call the "do_timer()" routine every clocktick
 */

static void (*tick_handler)(void);

static irqreturn_t timer_interrupt(int irq, void *dev_id)
{
	__raw_writeb(0, (void __iomem *)(0x0008701c + irq - 28));
	tick_handler();
	return IRQ_HANDLED;
}

static struct irqaction cmt_irq = {
	.name		= "rx-cmt",
	.handler	= timer_interrupt,
	.flags		= IRQF_TIMER,
};

static const int __initdata divide_rate[] = {8, 32, 128, 512};

void __init rx_clk_init(void (*tick)(void))
{
	unsigned int div;
	unsigned int cnt;
	unsigned long p_freq;
	u16 str_r;
	u16 mstpcra;
	u8 ier;

	tick_handler = tick;

	p_freq = CONFIG_INPUT_CLOCK_FREQ * CONFIG_PCLK_MULT;
	for (div = 0; div < 4; div++) {
		cnt = p_freq / HZ / divide_rate[div];
		if (cnt < 0x00010000)
			break;
	}

	mstpcra = __raw_readw((void __iomem *)(0x00080010));
	mstpcra |= 1 << (14 + (CONFIG_RX_CMT_CH % 2));
	__raw_writew(mstpcra, (void __iomem *)(0x00080010));

	setup_irq(irqno[CONFIG_RX_CMT_CH], &cmt_irq);

	/* initalize timer */
	__raw_writew(cnt, (void __iomem *)(base[CONFIG_RX_CMT_CH] + CMCOR));
	__raw_writew(0x0000, (void __iomem *)(base[CONFIG_RX_CMT_CH] + CMCNT));
	__raw_writew(0x0040 | div, (void __iomem *)(base[CONFIG_RX_CMT_CH] + CMCR));
	str_r = __raw_readw(str[CONFIG_RX_CMT_CH/2]);
	str_r |= 1 << (CONFIG_RX_CMT_CH % 2);
	__raw_writew(str_r, str[CONFIG_RX_CMT_CH/2]);
	ier = __raw_readb((void __iomem *)0x00087203);
	ier |= 0x10 << CONFIG_RX_CMT_CH;
	__raw_writeb(ier, (void __iomem *)0x00087203);
}
