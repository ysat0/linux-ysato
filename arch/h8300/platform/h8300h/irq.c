/*
 * Interrupt handling H8/300H depend.
 * Yoshinori Sato <ysato@users.sourceforge.jp>
 *
 */

#include <linux/init.h>
#include <linux/errno.h>

#include <asm/ptrace.h>
#include <asm/traps.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/gpio-internal.h>
#include <asm/regs306x.h>

const int __initconst h8300_saved_vectors[] = {
#if defined(CONFIG_GDB_DEBUG)
	TRAP3_VEC,	/* TRAPA #3 is GDB breakpoint */
#endif
	-1,
};

const h8300_vector __initconst h8300_trap_table[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	_system_call,
	0,
	0,
	_trace_break,
};

int h8300_enable_irq_pin(unsigned int irq)
{
	int bitmask;
	if (irq < EXT_IRQ0 || irq > EXT_IRQ5)
		return 0;


	return 0;
}

void h8300_disable_irq_pin(unsigned int irq)
{
	int bitmask;
	if (irq < EXT_IRQ0 || irq > EXT_IRQ5)
		return;

}
