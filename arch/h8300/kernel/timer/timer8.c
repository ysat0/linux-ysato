/*
 *  linux/arch/h8300/kernel/cpu/timer/timer8.c
 *
 *  Yoshinori Sato <ysato@users.sourcefoge.jp>
 *
 *  8bit Timer Handler
 *
 */

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/timer.h>

/* 8bit timer x2 */
#define CMFA	6

#define _8TCR	0
#define _8TCSR	2
#define TCORA	4
#define TCORB	6
#define _8TCNT	8

#define CMIEA	0x40
#define CCLR_CMA 0x08
#define CKS2	0x04

struct timer8_priv {
	struct platform_device *pdev;
	struct irqaction irqaction;
	unsigned long mapbase;
};

/*
 * timer_interrupt() needs to keep up the real-time clock,
 * as well as call the "xtime_update()" routine every clocktick
 */

static irqreturn_t timer_interrupt(int irq, void *dev_id)
{
	struct timer8_priv *p = (struct timer8_priv *)dev_id;
	h8300_timer_tick();
	ctrl_bclr(CMFA, p->mapbase + _8TCSR);
	return IRQ_HANDLED;
}

static const int __initconst divide_rate[] = {8, 64, 8192};

static int __init timer8_setup(struct timer8_priv *p, struct platform_device *pdev)
{
	struct resource *res;
	unsigned int div;
	unsigned int cnt;
	int irq;

	memset(p, 0, sizeof(*p));
	p->pdev = pdev;

	res = platform_get_resource(p->pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&p->pdev->dev, "failed to get I/O memory\n");
		return -ENXIO;
	}

	irq = platform_get_irq(p->pdev, 0);
	if (irq < 0) {
		dev_err(&p->pdev->dev, "failed to get irq\n");
		return -ENXIO;
	}

	p->mapbase = res->start;

	p->irqaction.name = dev_name(&p->pdev->dev);
	p->irqaction.handler = timer_interrupt;
	p->irqaction.dev_id = p;
	p->irqaction.flags = IRQF_DISABLED | IRQF_TIMER;

	calc_param(cnt, div, divide_rate, 0x10000);
	div++;

	setup_irq(irq, &p->irqaction);

	/* initialize timer */
	ctrl_outw(cnt, p->mapbase + TCORA);
	ctrl_outw(0x0000, p->mapbase + _8TCSR);
	ctrl_outw((CMIEA|CCLR_CMA|CKS2) << 8 | div,
		  p->mapbase + _8TCR);

	platform_set_drvdata(pdev, p);

	return 0;
}

static int __init timer8_probe(struct platform_device *pdev)
{
	struct timer8_priv *p = platform_get_drvdata(pdev);

	if (p) {
		dev_info(&pdev->dev, "kept as earlytimer\n");
		return 0;
	}

	p = kmalloc(sizeof(*p), GFP_KERNEL);
	if (p == NULL) {
		dev_err(&pdev->dev, "failed to allocate driver data\n");
		return -ENOMEM;
	}

	return timer8_setup(p, pdev);
}

static int timer8_remove(struct platform_device *pdev)
{
	return -EBUSY;
}

static struct platform_driver __initdata timer8_driver = {
	.probe		= timer8_probe,
	.remove		= timer8_remove,
	.driver		= {
		.name	= "h8300_8timer",
	}
};

early_platform_init("earlytimer", &timer8_driver);
