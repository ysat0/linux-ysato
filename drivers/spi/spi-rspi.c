/*
 * Renesas RSPI controller 
 *
 * Copyright (c) 2011 Yoshinori Sato

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/notifier.h>
#include <linux/cpufreq.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_bitbang.h>
#include <linux/spi/spi_rspi.h>
#include <asm/io.h>

struct spi_rspi {
	/* bitbang has to be first */
	struct spi_bitbang bitbang;
	struct completion done;
	struct resource mem;
	struct device *dev;
	void __iomem *reg;
	u8 *rx_ptr;
	const u8 *tx_ptr;
	int irqs[4];
	int remaining_bytes;
	int bits_per_word;
	struct clk *pclk;
};

static int rspi_setup_transfer(struct spi_device *spi,
		struct spi_transfer *t)
{
	struct spi_rspi *rspi = spi_master_get_devdata(spi->master);
	int brr;
	unsigned short cmd0;
	u8 mode = spi->mode;
	int div;

	brr = clk_get_rate(rspi->pclk) / t->speed_hz;
	div = 1;
	while (brr >= 256) {
		brr /= 2;
		div++;
	}
	brr = brr + 1;

	cmd0 = ((div - 1) << 2) | (mode & 3);
	cmd0 |= (mode & SPI_LSB_FIRST) ? 0x1000 : 0x0000;
	if (t->bits_per_word >= 8 && t->bits_per_word <= 16) {
		cmd0 |= (t->bits_per_word - 1) << 8;
		rspi->bits_per_word = t->bits_per_word;
	} else {
		switch (t->bits_per_word) {
		case 0:
			/* assume 8bit */
			cmd0 |= 0x700;
			rspi->bits_per_word = 8;
			break;
		case 20:
			rspi->bits_per_word = 32;
			break;
		case 24:
			cmd0 |= 0x100;
			rspi->bits_per_word = 32;
			break;
		case 32:
			cmd0 |= 0x200;
			rspi->bits_per_word = 32;
			break;
		default:
			return -EINVAL;
		}
	}

	writeb(0x00, rspi->reg);
	writeb(0x00, rspi->reg + 0x02);
	writeb(0x00, rspi->reg + 0x08);
	writeb(0x20, rspi->reg + 0x0b);
	writew(cmd0, rspi->reg + 0x10);
	writeb(brr, rspi->reg + 0x0a);
	return 0;
}

static int rspi_setup(struct spi_device *spi)
{
	/* nothing do. */
	return 0;
}

int rspi_txrx_bufs(struct spi_device *spi, struct spi_transfer *t)
{
	struct spi_rspi *rspi = spi_master_get_devdata(spi->master);
	int i;
	rspi->tx_ptr = t->tx_buf;
	rspi->rx_ptr = t->rx_buf;
	rspi->remaining_bytes = t->len;
	INIT_COMPLETION(rspi->done);
	writeb(0xf9, rspi->reg);
	wait_for_completion(&rspi->done);
	return t->len - rspi->remaining_bytes;
}


static irqreturn_t rspi_eio(int irq, void *p)
{
	struct spi_rspi *rspi = p;
	const char *msg[] = {"overrun", NULL, "mode fault", "parity"};
	unsigned char ssr;
	int i;

	ssr = readb(rspi->reg + 3);
	for (i = 0; i < ARRAY_SIZE(msg); i++) {
		if (!msg[i])
			continue;
		if (ssr & (1 << i)) {
			dev_err(rspi->dev, "%s error.", msg[i]);
			ssr &= ~(1 << i);
		}
	}
	writeb(ssr, rspi->reg + 3);

	return IRQ_HANDLED;
}

irqreturn_t rspi_rio(int irq, void *p)
{
	struct spi_rspi *rspi = p;
	int rx_size;
	u32 rxd;
	if (rspi->bits_per_word == 8) {
		rx_size = 1;
	} else if (rspi->bits_per_word <= 16) {
		rx_size = 2;
	} else {
		rx_size=4;
	}
	rxd = readl(rspi->reg + 4);
	if (rspi->rx_ptr) {
		switch (rx_size) {
		case 1:
			*(u8 *)rspi->rx_ptr = rxd;
			break;
		case 2:
			*(u16 *)rspi->rx_ptr = rxd;
			break;
		case 4:
			*(u32 *)rspi->rx_ptr = rxd;
			break;
		}
		rspi->rx_ptr += rx_size;
	}
	rspi->remaining_bytes -= rx_size;
	if (rspi->remaining_bytes <= 0) {
		  complete(&rspi->done);
		  writeb(0x00, rspi->reg);
	}
	return IRQ_HANDLED;
}

irqreturn_t rspi_tio(int irq, void *p)
{
	struct spi_rspi *rspi = p;
	if (rspi->remaining_bytes > 0 && rspi->tx_ptr) {
		if (rspi->bits_per_word == 8) {
			writel(*(unsigned char *)rspi->tx_ptr, rspi->reg + 4);
			rspi->tx_ptr += 1;
		} else if (rspi->bits_per_word <= 16) {
			writel(*(unsigned short *)rspi->tx_ptr, rspi->reg + 4);
			rspi->tx_ptr += 2;
		} else {
			writel(*(unsigned long *)rspi->tx_ptr, rspi->reg + 4);
			rspi->tx_ptr += 4;
		}
	} else
		writel(0, rspi->reg + 4);
	return IRQ_HANDLED;
}

static struct spi_master *rspi_init(struct device *dev, struct resource *mem,
				    struct resource *irq, int bus_num, int num_cs, 
				    void (*csfunc)(struct spi_device *spi, int is_on))
{
	struct spi_master *master;
	struct spi_rspi *rspi;
	int ret;
	int i;
	int step;
	int irqno;
	irqreturn_t (*rspi_irq_handler[])(int irq, void *dev_id) = {
		rspi_eio, rspi_rio, rspi_tio, NULL,
	};

	master = spi_alloc_master(dev, sizeof(struct spi_rspi));
	if (!master)
		return NULL;

	/* the spi->mode bits understood by this driver: */
	master->mode_bits = SPI_CPOL | SPI_CPHA;

	rspi = spi_master_get_devdata(master);
	rspi->bitbang.master = spi_master_get(master);
	rspi->bitbang.chipselect = csfunc;
	rspi->bitbang.setup_transfer = rspi_setup_transfer;
	rspi->bitbang.txrx_bufs = rspi_txrx_bufs;
	rspi->bitbang.master->setup = rspi_setup;
	rspi->dev = dev;

	init_completion(&rspi->done);

	
	rspi->pclk = clk_get(&dev, "rspi_clk");
	if (IS_ERR(rspi->pclk)) {
		rspi->pclk = clk_get(&dev, "peripheral_clk");
		if (IS_ERR(rspi->pclk)) {
			dev_err(dev, "can't get iclk\n");
			return NULL;
		}
	}

	if (!request_mem_region(mem->start, resource_size(mem),"rspi"))
		goto put_master;

	rspi->reg = ioremap(mem->start, resource_size(mem));
	if (rspi->reg == NULL) {
		dev_warn(dev, "ioremap failure\n");
		goto map_failed;
	}

	master->bus_num = bus_num;
	master->num_chipselect = num_cs;

	rspi->mem = *mem;

	step = ((irq->end + 1) - irq->start) / ARRAY_SIZE(rspi_irq_handler);
	irqno = irq->start;
	for (i = 0; i < ARRAY_SIZE(rspi_irq_handler); i++) {
		if (!rspi_irq_handler[i])
			continue;
		ret = request_irq(irqno, rspi_irq_handler[i], 0, "rspi", rspi);
		if (ret)
			goto free_irq;
		rspi->irqs[i] = irqno;
		irqno += step;
	}

	ret = spi_bitbang_start(&rspi->bitbang);
	if (ret) {
		dev_err(dev, "spi_bitbang_start FAILED\n");
		goto free_irq;
	}
	dev_notice(dev, "SPI bus %d at %p, irq = %d\n", bus_num, rspi->reg, irq->start);
	return master;

free_irq:
	for (i = 0; i < ARRAY_SIZE(rspi_irq_handler); i++)
		if (rspi->irqs[i] > 0)
			free_irq(rspi->irqs[i], rspi);
	iounmap(rspi->reg);
map_failed:
	release_mem_region(mem->start, resource_size(mem));
put_master:
	spi_master_put(master);
	return NULL;
}

static void rspi_shutdown(struct spi_master *master)
{
	struct spi_rspi *rspi;
	int i;

	rspi = spi_master_get_devdata(master);

	spi_bitbang_stop(&rspi->bitbang);
	for(i = 0; i < 8; i++)
		free_irq(rspi->irqs[i], rspi);
	iounmap(rspi->reg);

	release_mem_region(rspi->mem.start, resource_size(&rspi->mem));
	spi_master_put(rspi->bitbang.master);
}

static int __devinit rspi_probe(struct platform_device *dev)
{
	struct rspi_platform_data *pdata;
	struct resource *memr;
	struct resource *irqr;
	int num_cs = 0;
	void (*csfunc)(struct spi_device *spi, int is_on) = NULL;
	struct spi_master *master;
	int i;
	int irqstep;
	int irq;

	pdata = dev->dev.platform_data;
	if (pdata) {
		num_cs = pdata->num_cs;
		csfunc = pdata->csfunc;
	}

	if (!num_cs || !csfunc) {
		dev_err(&dev->dev, "Missing slave select configuration data\n");
		return -EINVAL;
	}


	memr = platform_get_resource(dev, IORESOURCE_MEM, 0);
	if (!memr)
		return -ENODEV;

	irqr = platform_get_resource(dev, IORESOURCE_IRQ, 0);
	if (!irqr)
		return -ENXIO;

	master = rspi_init(&dev->dev, memr, irqr, dev->id, num_cs, csfunc);
	if (!master)
		return -ENODEV;

	for (i = 0; i < pdata->num_devices; i++)
		spi_new_device(master, &pdata->devices[i]);

	platform_set_drvdata(dev, master);
	return 0;
}

static int __devexit rspi_remove(struct platform_device *dev)
{
	rspi_shutdown(platform_get_drvdata(dev));
	platform_set_drvdata(dev, 0);

	return 0;
}

static struct platform_driver rspi_driver = {
	.probe = rspi_probe,
	.remove = __devexit_p(rspi_remove),
	.driver = {
		.name = "rspi",
		.owner = THIS_MODULE,
	},
};

static int __init rspi_platform_init(void)
{
	return platform_driver_register(&rspi_driver);
}
module_init(rspi_platform_init);

static void __exit rspi_platform_exit(void)
{
	platform_driver_unregister(&rspi_driver);
}
module_exit(rspi_platform_exit);

MODULE_ALIAS("platform:spi_rspi");
MODULE_AUTHOR("Yoshinori Sato <ysato@users.sourceforge.jp>");
MODULE_DESCRIPTION("RSPI SPI driver");
MODULE_LICENSE("GPL");
