/*
 * drivers/serial/fm3-uart.c
 *
 * Fujitsu FM3 On-chip UART driver
 *
 *  Copyright (C) 2012 Yoshinori Sato
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/major.h>
#include <linux/string.h>
#include <linux/sysrq.h>
#include <linux/ioport.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/console.h>
#include <linux/platform_device.h>
#include <linux/notifier.h>
#include <linux/cpufreq.h>
#include <linux/clk.h>
#include <linux/ctype.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/serial_core.h>
#include <linux/amba/bus.h>
#include <asm/io.h>

#define FIFOSIZE 16

#define SMR	0x00
#define SCR	0x01
#define ESCR	0x04
#define SSR	0x05
#define TDR	0x08
#define RDR	0x08
#define BGR0	0x0c
#define BGR1	0x0d
#define FCR0	0x14
#define FCR1	0x15
#define FBYTE1	0x18
#define FBYTE2	0x19

#define SSR_TBI		(1 << 0)
#define SSR_TDRE	(1 << 1)
#define SSR_RDRF	(1 << 2)
#define SSR_ORE		(1 << 3)
#define SSR_FRE		(1 << 4)
#define SSR_PE		(1 << 5)
#define SSR_REC		(1 << 7)

#define SCR_TIE		(1 << 3)
#define SCR_RIE		(1 << 4)
#define SCR_UPCL	(1 << 7)

#define FCR_FTIE	(1 << 1)

#define SMR_SOE		(1 << 0)

#define FCR_FDRQ	(1 << 2)

struct fm3_port {
	struct uart_port	port;
	unsigned int		irqs[2];

	/* Port enable callback */
	void			(*enable)(struct uart_port *port);

	/* Port disable callback */
	void			(*disable)(struct uart_port *port);

	/* Break timer */
	struct timer_list	break_timer;
	int			break_flag;

	struct clk		*clk;

	struct list_head	node;
};

struct fm3_uart_priv {
	spinlock_t lock;
	struct list_head ports;
	struct notifier_block clk_nb;
};

struct plat_fm3_port {
	unsigned int mapbase;
	unsigned int membase;
};

/* Function prototypes */
static void stop_tx(struct uart_port *port);

static struct fm3_port fm3_ports[8];
static struct uart_driver fm3_uart_driver;

static inline struct fm3_port *
to_fm3_port(struct uart_port *uart)
{
	return container_of(uart, struct fm3_port, port);
}

static inline void uart_out(struct uart_port *port, int reg, int val)
{
	writeb(val, port->membase + reg);
}

static inline void uart_outw(struct uart_port *port, int reg, int val)
{
	writew(val, port->membase + reg);
}

static inline int uart_in(struct uart_port *port, int reg)
{
	return readb(port->membase + reg);
}

static inline int uart_inw(struct uart_port *port, int reg)
{
	return readw(port->membase + reg);
}

static int rxd_in(struct uart_port *port)
{
	/* TODO */
	return 0;
}

#if defined(CONFIG_CONSOLE_POLL) || defined(CONFIG_SERIAL_FM3_CONSOLE)

#ifdef CONFIG_CONSOLE_POLL
static inline void handle_error(struct uart_port *port)
{
	/* Clear error flags */
	uart_out(port, SSR, 0x80);
}

static int fm3_poll_get_char(struct uart_port *port)
{
	unsigned short status;
	int c;

	do {
		status = uart_in(port, SSR);
		if (status & 0x38) {
			handle_error(port);
			continue;
		}
	} while (!(status & 0x04));

	c = uart_in(port, RDR);

	return c;
}
#endif

static void fm3_poll_put_char(struct uart_port *port, unsigned char c)
{
	unsigned short status;

	do {
		status = uart_in(port, SSR);
	} while (!(status & 0x02));

	uart_out(port, TDR, c);
}
#endif

/* ********************************************************************** *
 *                   the interrupt related routines                       *
 * ********************************************************************** */

static void transmit_chars(struct uart_port *port)
{
	struct circ_buf *xmit = &port->state->xmit;
	unsigned int stopped = uart_tx_stopped(port);
	u8 ctrl = 0;
	int count;

	if (port->type == PORT_FM3_FIFO)
		count = FIFOSIZE - uart_in(port, FBYTE1);
	else
		count = 1;
	do {
		unsigned char c;

		if (port->x_char) {
			c = port->x_char;
			port->x_char = 0;
		} else if (!uart_circ_empty(xmit) && !stopped) {
			c = xmit->buf[xmit->tail];
			xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		} else {
			break;
		}

		uart_out(port, TDR, c);

		port->icount.tx++;
	} while (--count > 0);

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(port);
	if (uart_circ_empty(xmit)) {
		stop_tx(port);
	} else {
		if (port->type == PORT_FM3_FIFO) {
			ctrl = uart_in(port, FCR1);
			ctrl &= ~0x02;
			uart_out(port, FCR1, ctrl);
		}
	}
}

static inline void receive_chars(struct uart_port *port)
{
	struct tty_struct *tty = port->state->port.tty;
	int i, count, copied = 0;
	unsigned short status;


	if (port->type == PORT_FM3_FIFO) {
		count = uart_in(port, FBYTE1);
		if (count == 0)
			count = 1;
	} else
		count = 1;
	while (1) {
		status = uart_in(port, SSR);
		if (!(status & 0x04))
			break;

		/* Don't copy more bytes than there is room for in the buffer */
		count = tty_buffer_request_room(tty, count);

		/* If for any reason we can't copy more data, we're done! */
		if (count == 0)
			break;

		for (i = 0; i < count; i++) {
			char c = uart_in(port, RDR);
			if (uart_handle_sysrq_char(port, c)) {
				count--; i--;
				continue;
			}
			tty_insert_flip_char(tty, c, TTY_NORMAL);
		}
		copied += count;
		port->icount.rx += count;
	}

	if (copied)
		/* Tell the rest of the system the news. New characters! */
		tty_flip_buffer_push(tty);
}

#define BREAK_JIFFIES (HZ/20)
static void schedule_break_timer(struct fm3_port *port)
{
	port->break_timer.expires = jiffies + BREAK_JIFFIES;
	add_timer(&port->break_timer);
}
/* Ensure that two consecutive samples find the break over. */
static void break_timer(unsigned long data)
{
	struct fm3_port *port = (struct fm3_port *)data;

	if (rxd_in(&port->port) == 0) {
		port->break_flag = 1;
		schedule_break_timer(port);
	} else if (port->break_flag == 1) {
		/* break is over. */
		port->break_flag = 2;
		schedule_break_timer(port);
	} else
		port->break_flag = 0;
}

static int handle_errors(struct uart_port *port)
{
	int copied = 0;
	u8 status = uart_in(port, SSR);
	struct tty_struct *tty = port->state->port.tty;

	if (status & SSR_ORE) {
		if (tty_insert_flip_char(tty, 0, TTY_OVERRUN))
			copied++;

		dev_notice(port->dev, "overrun error");
	}

	if (status & SSR_FRE) {
		if (rxd_in(port) == 0) {
			/* Notify of BREAK */
			struct fm3_port *fm3port = to_fm3_port(port);

			if (!fm3port->break_flag) {
				fm3port->break_flag = 1;
				schedule_break_timer(fm3port);

				/* Do sysrq handling. */
				if (uart_handle_break(port))
					return 0;

				dev_dbg(port->dev, "BREAK detected\n");

				if (tty_insert_flip_char(tty, 0, TTY_BREAK))
					copied++;
			}

		} else {
			/* frame error */
			if (tty_insert_flip_char(tty, 0, TTY_FRAME))
				copied++;

			dev_notice(port->dev, "frame error\n");
		}
	}

	if (status & SSR_PE) {
		if (tty_insert_flip_char(tty, 0, TTY_PARITY))
			copied++;

		dev_notice(port->dev, "parity error");
	}

	if (copied)
		tty_flip_buffer_push(tty);
	uart_out(port, SSR, SSR_REC);

	return copied;
}

static irqreturn_t rx_interrupt(int irq, void *port)
{
	u8 status = uart_in(port, SSR);
	if (status & (SSR_PE|SSR_FRE|SSR_ORE))
		handle_errors(port);
	else
		receive_chars(port);

	return IRQ_HANDLED;
}

static irqreturn_t tx_interrupt(int irq, void *ptr)
{
	struct uart_port *port = ptr;
	unsigned long flags;

	spin_lock_irqsave(&port->lock, flags);
	transmit_chars(port);
	spin_unlock_irqrestore(&port->lock, flags);

	return IRQ_HANDLED;
}

static int uart_clock_notifier(struct notifier_block *self,
			       unsigned long phase, void *p)
{
	struct fm3_uart_priv *priv = container_of(self,
						struct fm3_uart_priv, clk_nb);
	struct fm3_port *port;
	unsigned long flags;

	if ((phase == CPUFREQ_POSTCHANGE) ||
	    (phase == CPUFREQ_RESUMECHANGE)) {
		spin_lock_irqsave(&priv->lock, flags);
		list_for_each_entry(port, &priv->ports, node)
			port->port.uartclk = clk_get_rate(port->clk);
		spin_unlock_irqrestore(&priv->lock, flags);
	}

	return NOTIFY_OK;
}

static void uart_clk_enable(struct uart_port *port)
{
	struct fm3_port *fm3_port = to_fm3_port(port);

	clk_enable(fm3_port->clk);
	fm3_port->port.uartclk = clk_get_rate(fm3_port->clk);
}

static void uart_clk_disable(struct uart_port *port)
{
	struct fm3_port *fm3_port = to_fm3_port(port);

	clk_disable(fm3_port->clk);
}

static int register_interrupt(struct uart_port *port)
{
	int i;
	struct fm3_port *fm3_port= to_fm3_port(port);
	irqreturn_t (*handlers[2])(int irq, void *ptr) = {
		rx_interrupt, tx_interrupt,
	};
	const char *desc[] = { "UART Transmit", "UART Receive" };
	for (i = 0; i < ARRAY_SIZE(handlers); i++) {
		if (request_irq(port->irq + i, handlers[i],
				IRQF_DISABLED, desc[i], fm3_port)) {
			dev_err(fm3_port->port.dev, "Can't allocate IRQ\n");
			return -ENODEV;
		}
		fm3_port->irqs[i] = port->irq + i;
	}

	return 0;
}

static void unregister_interrupt(struct uart_port *port)
{
	int i;
	struct fm3_port *fm3_port= to_fm3_port(port);

	for (i = 0; i < ARRAY_SIZE(fm3_port->irqs); i++) {
		if (!fm3_port->irqs[i])
			continue;
		
		free_irq(fm3_port->irqs[i], port);
	}
}

static unsigned int tx_empty(struct uart_port *port)
{
	unsigned short status = uart_in(port, SSR);
	return status & SSR_TDRE;
}

static void set_mctrl(struct uart_port *port, unsigned int mctrl)
{
}

static unsigned int get_mctrl(struct uart_port *port)
{
	return TIOCM_DTR | TIOCM_RTS | TIOCM_DSR;
}

static void start_tx(struct uart_port *port)
{
	u8 ctrl;

	ctrl = uart_in(port, SCR);
	ctrl |= SCR_TIE;
	uart_out(port, SCR, ctrl);
}

static void stop_tx(struct uart_port *port)
{
	u8 ctrl;

	ctrl = uart_in(port, SCR);
	ctrl &= ~SCR_TIE;
	uart_out(port, SCR, ctrl);
}

static void start_rx(struct uart_port *port, unsigned int tty_start)
{
	u8 ctrl;

	ctrl = uart_in(port, SCR);
	ctrl |= SCR_RIE;
	uart_out(port, SCR, ctrl);
}

static void stop_rx(struct uart_port *port)
{
        u8 ctrl;

	ctrl = uart_in(port, SCR);
	ctrl &= ~SCR_RIE;
	uart_out(port, SCR, ctrl);
}

static void enable_ms(struct uart_port *port)
{
}

static void break_ctl(struct uart_port *port, int break_state)
{
}

static int uart_startup(struct uart_port *port)
{
	struct fm3_port *p = to_fm3_port(port);

	if (p->enable)
		p->enable(port);

	register_interrupt(port);
	start_tx(port);
	start_rx(port, 1);

	return 0;
}

static void uart_shutdown(struct uart_port *port)
{
	struct fm3_port *p = to_fm3_port(port);

	stop_rx(port);
	stop_tx(port);
	unregister_interrupt(port);

	if (p->disable)
		p->disable(port);
}

static int gbgr_value(int baud, int clk)
{
	return clk / baud - 1;
}

static void init_pins(struct uart_port *port, int cflag)
{
	/* TODO */
}

static void set_termios(struct uart_port *port, struct ktermios *termios,
			    struct ktermios *old)
{
	u16 baud, smr_val, escr_val;
	int max_baud;
	int t = -1;

	/*
	 * earlyprintk comes here early on with port->uartclk set to zero.
	 * the clock framework is not up and running at this point so here
	 * we assume that 115200 is the maximum baud rate. please note that
	 * the baud rate is not programmed during earlyprintk - it is assumed
	 * that the previous boot loader has enabled required clocks and
	 * setup the baud rate generator hardware for us already.
	 */
	max_baud = port->uartclk ? port->uartclk / 16 : 115200;

	baud = uart_get_baud_rate(port, termios, old, 0, max_baud);
	if (likely(baud && port->uartclk))
		t = gbgr_value(baud, port->uartclk);

	uart_out(port, SCR, SCR_UPCL); 

	smr_val = uart_in(port, SMR) & (0x08 | SMR_SOE);
	escr_val = uart_in(port, ESCR) & ~0x1b;
	if ((termios->c_cflag & CSIZE) == CS7)
		escr_val |= 0x03;
	if (termios->c_cflag & PARENB)
		escr_val |= 0x10;
	if (termios->c_cflag & PARODD)
		escr_val |= 0x08;
	if (termios->c_cflag & CSTOPB)
		smr_val |= 0x08;

	uart_update_timeout(port, termios->c_cflag, baud);

	uart_out(port, SMR, smr_val);
	uart_out(port, ESCR, escr_val);

	if (t > 0) {
		uart_outw(port, BGR0, t);
		udelay((1000000+(baud-1)) / baud); /* Wait one bit interval */
	}

	init_pins(port, termios->c_cflag);

	uart_out(port, SCR, 0x03);
	if (port->type == PORT_FM3_FIFO)
		uart_out(port, FCR1, 0x04);
	if ((termios->c_cflag & CREAD) != 0)
		start_rx(port, 0);
}

static const char *type(struct uart_port *port)
{
	return "FM3-UART";
}

static void release_port(struct uart_port *port)
{
	/* Nothing here yet .. */
}

static int request_port(struct uart_port *port)
{
	/* Nothing here yet .. */
	return 0;
}

static void config_port(struct uart_port *port, int flags)
{
	if (flags & UART_CONFIG_TYPE)
		switch(port->line) {
		case 0:
		case 1:
		case 2:
		case 3:
			/* Ch0 to 3 w/o FIFO */
			port->type = PORT_FM3;
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			/* Ch4 to 7 with FIFO */
			port->type = PORT_FM3_FIFO;
			break;
		}

	if (port->membase)
		return;

	port->membase = ioremap_nocache(port->mapbase, 0x40);
	
	if (IS_ERR(port->membase))
		dev_err(port->dev, "can't remap port#%d\n", port->line);
}

static int verify_port(struct uart_port *port, struct serial_struct *ser)
{
	return 0;
}

static struct uart_ops fm3_uart_ops = {
	.tx_empty	= tx_empty,
	.set_mctrl	= set_mctrl,
	.get_mctrl	= get_mctrl,
	.start_tx	= start_tx,
	.stop_tx	= stop_tx,
	.stop_rx	= stop_rx,
	.enable_ms	= enable_ms,
	.break_ctl	= break_ctl,
	.startup	= uart_startup,
	.shutdown	= uart_shutdown,
	.set_termios	= set_termios,
	.type		= type,
	.release_port	= release_port,
	.request_port	= request_port,
	.config_port	= config_port,
	.verify_port	= verify_port,
#ifdef CONFIG_CONSOLE_POLL
	.poll_get_char	= fm3_poll_get_char,
	.poll_put_char	= fm3_poll_put_char,
#endif
};

static void __devinit init_single_port(struct amba_device *dev,
				       struct fm3_port *fm3_port,
				       unsigned int index)
{
	fm3_port->port.ops	= &fm3_uart_ops;
	fm3_port->port.iotype	= UPIO_MEM;
	fm3_port->port.line	= index;

	if (dev) {
		fm3_port->clk = clk_get(&dev->dev, "apb1");
		fm3_port->enable = uart_clk_enable;
		fm3_port->disable = uart_clk_disable;
		fm3_port->port.dev = &dev->dev;
	}

	fm3_port->break_timer.data = (unsigned long)fm3_port;
	fm3_port->break_timer.function = break_timer;
	init_timer(&fm3_port->break_timer);

	fm3_port->port.flags	= UPF_BOOT_AUTOCONF;
	fm3_port->port.mapbase	= dev->res.start;
	fm3_port->port.irq	= dev->irq[0];
}

#ifdef CONFIG_SERIAL_FM3_CONSOLE
static struct tty_driver *serial_console_device(struct console *co, int *index)
{
	struct uart_driver *p = &fm3_uart_driver;
	*index = co->index;
	return p->tty_driver;
}

static void serial_console_putchar(struct uart_port *port, int ch)
{
	fm3_poll_put_char(port, ch);
}

/*
 *	Print a string to the serial port trying not to disturb
 *	any possible real use of the port...
 */
static void serial_console_write(struct console *co, const char *s,
				 unsigned count)
{
	struct uart_port *port = co->data;
	struct fm3_port *fm3_port = to_fm3_port(port);

	if (fm3_port->enable)
		fm3_port->enable(port);

	uart_console_write(port, s, count, serial_console_putchar);

	/* wait until fifo is empty and last bit has been transmitted */
	if (port->type == PORT_FM3_FIFO) {
		while (!(uart_in(port, FCR1) & FCR_FDRQ))
			cpu_relax();
	} else {
		while (!(uart_in(port, SSR) & SSR_TDRE))
			cpu_relax();
	}
	if (fm3_port->disable)
		fm3_port->disable(port);
}

#define GPIO_PFR2	((volatile unsigned int       *)(0x40033008))
#define GPIO_ADE	((volatile unsigned int       *)(0x40033500))
#define GPIO_EPFR07	((volatile unsigned int       *)(0x4003361C))

static int __devinit serial_console_setup(struct console *co, char *options)
{
	struct fm3_port *fm3_port;
	struct uart_port *port;
	int baud = 115200;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';
	int ret;

	/*
	 * Check whether an invalid uart number has been specified, and
	 * if so, search for the first available port that does have
	 * console support.
	 */
	if (co->index >= 8)
		co->index = 0;

	if (co->data) {
		port = co->data;
		fm3_port = to_fm3_port(port);
	} else {
		fm3_port = &fm3_ports[co->index];
		port = &(fm3_port->port);
		co->data = port;
	}

	/*
	 * Also need to check port->type, we don't actually have any
	 * UPIO_PORT ports, but uart_report_port() handily misreports
	 * it anyways if we don't have a port available by the time this is
	 * called.
	 */
	config_port(port, 0);

	*GPIO_EPFR07 = (*GPIO_EPFR07 & 0xFFFFFF00) | 0x00000050;
	*GPIO_PFR2 = *GPIO_PFR2 | 0x00000006;
	*GPIO_ADE  = *GPIO_ADE & 0x7FFFFFFF;

	if (fm3_port->enable)
		fm3_port->enable(port);
	uart_out(port, SCR, 0x80);
	uart_out(port, SCR, 0x00);

	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);

	ret = uart_set_options(port, co, baud, parity, bits, flow);
	return ret;
}

static struct console serial_console = {
	.name		= "ttyS",
	.device		= serial_console_device,
	.write		= serial_console_write,
	.setup		= serial_console_setup,
	.flags		= CON_PRINTBUFFER,
	.index		= -1,
};
#endif

#if defined(CONFIG_SERIAL_FM3_CONSOLE)
#define SERIAL_FM3_CONSOLE	(&serial_console)
#else
#define SERIAL_FM3_CONSOLE	0
#endif

static struct uart_driver fm3_uart_driver = {
	.owner		= THIS_MODULE,
	.driver_name	= "fm3-uart",
	.dev_name	= "ttyS",
	.major		= TTY_MAJOR,
	.minor		= 64,
	.nr		= 8,
	.cons		= SERIAL_FM3_CONSOLE,
};


static int remove(struct amba_device *dev)
{
	struct fm3_uart_priv *priv = platform_get_drvdata(dev);
	struct fm3_port *p;
	unsigned long flags;

	cpufreq_unregister_notifier(&(priv->clk_nb), CPUFREQ_TRANSITION_NOTIFIER);

	spin_lock_irqsave(&(priv->lock), flags);
	list_for_each_entry(p, &(priv->ports), node)
		uart_remove_one_port(&fm3_uart_driver, &p->port);
	spin_unlock_irqrestore(&(priv->lock), flags);

	kfree(priv);
	return 0;
}

static int __devinit probe_single_port(struct amba_device *dev,
				       unsigned int index,
				       struct fm3_port *port)
{
	struct fm3_uart_priv *priv = amba_get_drvdata(dev);
	unsigned long flags;
	int ret;

	/* Sanity check */
	if (unlikely(index >= 8)) {
		dev_notice(&dev->dev, "Attempting to register port "
			   "%d when only %d are available.\n",
			   index+1, 8);
		return 0;
	}

	init_single_port(dev, port, index);

	ret = uart_add_one_port(&fm3_uart_driver, &port->port);
	if (ret)
		return ret;

	INIT_LIST_HEAD(&port->node);

	spin_lock_irqsave(&priv->lock, flags);
	list_add(&port->node, &priv->ports);
	spin_unlock_irqrestore(&priv->lock, flags);

	return 0;
}

static int __devinit fm3_serial_probe(struct amba_device *dev, struct amba_id *id)
{
	struct fm3_uart_priv *priv;
	int ret = -EINVAL;

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	INIT_LIST_HEAD(&priv->ports);
	spin_lock_init(&priv->lock);
	amba_set_drvdata(dev, priv);

	priv->clk_nb.notifier_call = uart_clock_notifier;
	cpufreq_register_notifier(&priv->clk_nb, CPUFREQ_TRANSITION_NOTIFIER);

	ret = probe_single_port(dev, dev->periphid & 0x0f, &fm3_ports[dev->periphid & 0x0f]);
	if (ret)
		goto err_unreg;

	return 0;

err_unreg:
	remove(dev);
	return ret;
}

static struct amba_id uart_ids[] __initdata = {
	{
		.id	= 0x00000010,
		.mask	= 0x000ffff0,
	},
	{ 0, 0 },
};

static struct amba_driver fm3_driver = {
	.drv = {
		.name	= "fm3-uart",
	},
	.id_table	= uart_ids,
	.probe		= fm3_serial_probe,
	.remove		= remove,
	.probe		= fm3_serial_probe,
};

static int __init fm3_uart_init(void)
{
	int ret;

	printk(KERN_INFO "Serial: Fujitsu FM3 UART driver\n");
	ret = uart_register_driver(&fm3_uart_driver);
	if (likely(ret == 0)) {
		ret = amba_driver_register(&fm3_driver);
		if (unlikely(ret))
			uart_unregister_driver(&fm3_uart_driver);
	}

	return ret;
}

static void __exit fm3_uart_exit(void)
{
	amba_driver_unregister(&fm3_driver);
	uart_unregister_driver(&fm3_uart_driver);
}

module_init(fm3_uart_init);
module_exit(fm3_uart_exit);

MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:fm3_uart");
