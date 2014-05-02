/*
 *  linux/arch/h8300/kernel/cpu/timer/timer8.c
 *
 *  Yoshinori Sato <ysato@users.sourcefoge.jp>
 *
 *  8bit Timer clock source
 *
 */

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/clocksource.h>
#include <linux/module.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/timer.h>

#define _8TCR	0
#define _8TCSR	2
#define TCORA	4
#define TCORB	6
#define _8TCNT	8

struct timer8_priv {
	struct platform_device *pdev;
	struct clocksource cs;
	struct irqaction irqaction;
	unsigned long mapbase;
	raw_spinlock_t lock;
	unsigned long total_cycles;
	unsigned int cs_enabled;
};

static unsigned long timer8_get_counter(struct timer8_priv *p)
{
	unsigned long v1, v2, v3;
	int o1, o2;

	o1 = ctrl_inb(p->mapbase + _8TCSR) & 0x20;

	/* Make sure the timer value is stable. Stolen from acpi_pm.c */
	do {
		o2 = o1;
		v1 = ctrl_inw(p->mapbase + _8TCNT);
		v2 = ctrl_inw(p->mapbase + _8TCNT);
		v3 = ctrl_inw(p->mapbase + _8TCNT);
		o1 = ctrl_inb(p->mapbase + _8TCSR) & 0x20;
	} while (unlikely((o1 != o2) || (v1 > v2 && v1 < v3)
			  || (v2 > v3 && v2 < v1) || (v3 > v1 && v3 < v2)));

	v2 |= o1 << 10;
	return v2;
}

static irqreturn_t timer8_interrupt(int irq, void *dev_id)
{
	struct timer8_priv *p = dev_id;

	p->total_cycles += 0x10000;
	ctrl_outb(ctrl_inb(p->mapbase + _8TCSR) & ~0x20,
		  p->mapbase + _8TCSR);
	return IRQ_HANDLED;
}
	
static inline struct timer8_priv *cs_to_priv(struct clocksource *cs)
{
	return container_of(cs, struct timer8_priv, cs);
}

static cycle_t timer8_clocksource_read(struct clocksource *cs)
{
	struct timer8_priv *p = cs_to_priv(cs);
	unsigned long flags, raw;
	unsigned long value;

	raw_spin_lock_irqsave(&p->lock, flags);
	value = p->total_cycles;
	raw = timer8_get_counter(p);
	raw_spin_unlock_irqrestore(&p->lock, flags);

	return value + raw;
}

static int timer8_clocksource_enable(struct clocksource *cs)
{
	struct timer8_priv *p = cs_to_priv(cs);

	WARN_ON(p->cs_enabled);

	p->total_cycles = 0;
	ctrl_outw(0, p->mapbase + _8TCNT);
	ctrl_outw(0x2402, p->mapbase + _8TCR);
	
	p->cs_enabled = true;
	return 0;
}

static void timer8_clocksource_disable(struct clocksource *cs)
{
	struct timer8_priv *p = cs_to_priv(cs);

	WARN_ON(!p->cs_enabled);

	ctrl_outb(0, p->mapbase + _8TCR); 
	p->cs_enabled = false;
}

static int __init timer8_setup(struct timer8_priv *p, struct platform_device *pdev)
{
	struct h8300_timer8_config *cfg = dev_get_platdata(&pdev->dev);
	struct resource *res;
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
	p->irqaction.handler = timer8_interrupt;
	p->irqaction.dev_id = p;
	p->irqaction.flags = IRQF_DISABLED | IRQF_TIMER;

	p->cs.name = pdev->name;
	p->cs.rating = cfg->rating;
	p->cs.read = timer8_clocksource_read;
	p->cs.enable = timer8_clocksource_enable;
	p->cs.disable = timer8_clocksource_disable;
	p->cs.mask = CLOCKSOURCE_MASK(sizeof(unsigned long) * 8);
	p->cs.flags = CLOCK_SOURCE_IS_CONTINUOUS;

	setup_irq(irq + 3, &p->irqaction);
	clocksource_register_hz(&p->cs, get_cpu_clock() / 64 / HZ);

	platform_set_drvdata(pdev, p);

	return 0;
}

static int timer8_probe(struct platform_device *pdev)
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

static struct platform_driver timer8_driver = {
	.probe		= timer8_probe,
	.remove		= timer8_remove,
	.driver		= {
		.name	= "h8300_8timer",
	}
};

static int __init timer8_init(void)
{
	return platform_driver_register(&timer8_driver);
}

static void __exit timer8_exit(void)
{
	platform_driver_unregister(&timer8_driver);
}

subsys_initcall(timer8_init);
module_exit(timer8_exit);

MODULE_AUTHOR("Yoshinori Sato");
MODULE_DESCRIPTION("H8/300H Timer Driver");
MODULE_LICENSE("GPL v2");
