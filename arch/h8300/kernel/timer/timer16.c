/*
 *  linux/arch/h8300/kernel/timer/timer16.c
 *
 *  Yoshinori Sato <ysato@users.sourcefoge.jp>
 *
 *  16bit Timer Handler
 *
 */

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/param.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#include <asm/segment.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/timer.h>
#include <asm/timer16.h>

#define TSTR	0
#define TSNC	1
#define TMDR	2
#define TOLR	3
#define TISRA	4
#define TISRB	5
#define TISRC	6

#define TCR	0
#define TIOR	1
#define TCNT	2
#define GRA	4
#define GRB	6

#define CCLR0 (1 << 5)

struct timer16_priv {
	struct platform_device *pdev;
	struct irqaction irqaction;
	unsigned long mapbase;
	unsigned long mapcommon;
	unsigned char imfa;
};

void h8300_timer_tick(void);

static irqreturn_t timer_interrupt(int irq, void *dev_id)
{
	struct timer16_priv *p = (struct timer16_priv *)dev_id;
	h8300_timer_tick();
	ctrl_bclr(p->imfa, p->mapcommon + TISRA);
	return IRQ_HANDLED;
}

static const int __initconst divide_rate[] = {1, 2, 4, 8};

static int __init timer16_setup(struct timer16_priv *p, struct platform_device *pdev)
{
	unsigned int div;
	unsigned int cnt;
	struct h8300_timer16_data *cfg = dev_get_platdata(&pdev->dev);

	memset(p, 0, sizeof(*p));
	p->pdev = pdev;
	p->mapbase =cfg->base;
	p->mapcommon = cfg->common;
	p->imfa = cfg->imfa;

	p->irqaction.name = dev_name(&p->pdev->dev);
	p->irqaction.handler = timer_interrupt;
	p->irqaction.dev_id = p;
	p->irqaction.flags = IRQF_DISABLED | IRQF_TIMER;
	
	calc_param(cnt, div, divide_rate, 0x10000);

	setup_irq(cfg->irqs[0], &p->irqaction);

	/* initialize timer */
	ctrl_bclr(cfg->enable, p->mapcommon + TSTR);
	ctrl_outb(CCLR0 | div, p->mapbase + TCR);
	ctrl_outw(cnt, p->mapbase + GRA);
	ctrl_bset(cfg->imie, p->mapcommon + TISRA);
	ctrl_bset(cfg->enable, p->mapcommon + TSTR);

	return 0;
}

static int __init timer16_probe(struct platform_device *pdev)
{
	struct timer16_priv *p =dev_get_platdata(&pdev->dev);

	if (p) {
		dev_info(&pdev->dev, "kept as earlytimer\n");
		return 0;
	}

	p = kmalloc(sizeof(*p), GFP_KERNEL);
	if (p == NULL) {
		dev_err(&pdev->dev, "failed to allocate driver data\n");
		return -ENOMEM;
	}

	return timer16_setup(p, pdev);
}

static int timer16_remove(struct platform_device *pdev)
{
	return -EBUSY;
}

static struct platform_driver __initdata timer16_driver = {
	.probe		= timer16_probe,
	.remove		= timer16_remove,
	.driver		= {
		.name	= "h8300h-16timer",
	}
};

early_platform_init("earlytimer", &timer16_driver);
