/*
 * arch/rx/kernel/traps.c
 * handles hardware traps and faults.
 * 
 * Copyright (C) 2009 Yoshinori Sato <ysato@users.sourceforge.jp>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/kernel.h>
#include <linux/ptrace.h>
#include <linux/hardirq.h>
#include <linux/irqflags.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/io.h>
#include <linux/bug.h>
#include <linux/limits.h>
#include <linux/proc_fs.h>
#include <linux/sysfs.h>
#include <linux/kdebug.h>
#include <asm/system.h>
#include <asm/uaccess.h>

unsigned long __nmi_count = 0;

void (*exception_table[32 + 20])(void);


static void do_trap(int signr, char *str, struct pt_regs *regs, 
		    siginfo_t *info)
{
	struct task_struct *tsk = current;

	if (user_mode(regs)) {
		if (info)
			force_sig_info(signr, info, tsk);
		else
			force_sig(signr, tsk);
	} else {
		die(str, regs);
	}
	return;
}

static inline void conditional_sti(struct pt_regs *regs)
{
	if (regs->psw & 0x00010000)
		local_irq_enable();
}

#define DO_ERROR_INFO(trapnr, signr, str, name, sicode, siaddr)		\
asmlinkage void do_##name(struct pt_regs *regs)			\
{									\
	siginfo_t info;							\
	info.si_signo = signr;						\
	info.si_errno = 0;						\
	info.si_code = sicode;						\
	info.si_addr = (void __user *)siaddr;				\
	if (notify_die(DIE_TRAP, str, regs, 0, trapnr, signr)		\
							== NOTIFY_STOP)	\
		return;							\
	conditional_sti(regs);						\
	do_trap(signr, str, regs, &info);				\
}

DO_ERROR_INFO(20, SIGILL, "privileged exception", privileged_op, ILL_PRVOPC, regs->pc)
DO_ERROR_INFO(21, SIGSEGV, "access exception", eaccess, SEGV_ACCERR, regs->pc)
DO_ERROR_INFO(23, SIGILL, "invalid opcode", invalid_op, ILL_ILLOPC, regs->pc)
DO_ERROR_INFO(48, SIGBUS, "BUS error", buserr, BUS_ADRERR, regs->pc)

static inline int fpsw_decode(void)
{
	static const int fpsw_map[] = { FPE_FLTINV, FPE_FLTOVF, FPE_FLTDIV, 
					FPE_FLTUND, FPE_FLTRES};
	int bit;
	unsigned int fpsw;
	__asm__ volatile("mvfc fpsw,%0":"=r"(fpsw));
	for (bit = 26; bit < 32; bit++)
		if (fpsw & (1 << bit)) {
			fpsw &= ~(1 << bit);
			__asm__ volatile("mvtc %0,fpsw"::"r"(fpsw));
			return fpsw_map[bit - 26];
		}
	return 0;
}

asmlinkage void do_fpuerror(struct pt_regs *regs)
{
	siginfo_t info;
	info.si_signo = fpsw_decode();
	info.si_errno = 0;
	info.si_addr = (void __user *)regs->pc;
	if (notify_die(DIE_TRAP, "Floating point exceprion", 
		       regs, 0, 25, SIGFPE) == NOTIFY_STOP)
		return;
	conditional_sti(regs);
	do_trap(SIGFPE, "Floating point exceprion", regs, &info);
}

asmlinkage void do_nmi(struct pt_regs *regs)
{
	nmi_enter();

	__nmi_count++;
	notify_die(DIE_NMI, "nmi", regs, 0, 31, SIGINT);

	nmi_exit();
}

static void dump(struct pt_regs *regs)
{
	unsigned long	*sp;
	unsigned char	*tp;
	int		i;

	printk("\nCURRENT PROCESS:\n\n");
	printk("COMM=%s PID=%d\n", current->comm, current->pid);
	if (current->mm) {
		printk("TEXT=%08x-%08x DATA=%08x-%08x BSS=%08x-%08x\n",
			(int) current->mm->start_code,
			(int) current->mm->end_code,
			(int) current->mm->start_data,
			(int) current->mm->end_data,
			(int) current->mm->end_data,
			(int) current->mm->brk);
		printk("USER-STACK=%08x  KERNEL-STACK=%08lx\n\n",
			(int) current->mm->start_stack,
			(int) PAGE_SIZE+(unsigned long)current);
	}

	show_regs(regs);
	printk("\nCODE:");
	tp = ((unsigned char *) regs->pc) - 0x20;
	for (sp = (unsigned long *) tp, i = 0; (i < 0x40);  i += 4) {
		if ((i % 0x10) == 0)
			printk("\n%08x: ", (int) (tp + i));
		printk("%08x ", (int) *sp++);
	}
	printk("\n");

	printk("\nKERNEL STACK:");
	tp = ((unsigned char *) regs) - 0x40;
	for (sp = (unsigned long *) tp, i = 0; (i < 0xc0); i += 4) {
		if ((i % 0x10) == 0)
			printk("\n%08x: ", (int) (tp + i));
		printk("%08x ", (int) *sp++);
	}
	printk("\n");
	if (STACK_MAGIC != *(unsigned long *)((unsigned long)current+PAGE_SIZE))
                printk("(Possibly corrupted stack page??)\n");

	printk("\n\n");
}

static DEFINE_SPINLOCK(die_lock);

void die(const char *str, struct pt_regs *regs)
{
	static int diecount;

	oops_enter();

	console_verbose();
	spin_lock_irq(&die_lock);
	report_bug(regs->pc, regs);
	printk(KERN_EMERG "%s: [#%d] ", str,  ++diecount);
	dump(regs);

	spin_unlock_irq(&die_lock);
	do_exit(SIGSEGV);
}

extern char _start, _etext;
#define check_kernel_text(addr) \
        ((addr >= (unsigned long)(&_start)) && \
         (addr <  (unsigned long)(&_etext))) 

static int kstack_depth_to_print = 24;

void show_stack(struct task_struct *task, unsigned long *esp)
{
	unsigned long *stack,  addr;
	int i;

	if (esp == NULL)
		esp = (unsigned long *) &esp;

	stack = esp;

	printk("Stack from %08lx:", (unsigned long)stack);
	for (i = 0; i < kstack_depth_to_print; i++) {
		if (((unsigned long)stack & (THREAD_SIZE - 1)) == 0)
			break;
		if (i % 8 == 0)
			printk("\n       ");
		printk(" %08lx", *stack++);
	}

	printk("\nCall Trace:");
	i = 0;
	stack = esp;
	while (((unsigned long)stack & (THREAD_SIZE - 1)) != 0) {
		addr = *stack++;
		/*
		 * If the address is either in the text segment of the
		 * kernel, or in the region which contains vmalloc'ed
		 * memory, it *may* be the address of a calling
		 * routine; if so, print it so that someone tracing
		 * down the cause of the crash will be able to figure
		 * out the call path that was taken.
		 */
		if (check_kernel_text(addr)) {
			if (i % 4 == 0)
				printk("\n       ");
			printk(" [<%08lx>]", addr);
			i++;
		}
	}
	printk("\n");
}

void show_trace_task(struct task_struct *tsk)
{
	show_stack(tsk,(unsigned long *)tsk->thread.esp0);
}

void dump_stack(void)
{
	show_stack(NULL,NULL);
}

EXPORT_SYMBOL(dump_stack);

asmlinkage void unhandled_exception(struct pt_regs *regs, unsigned int ex)
{
	if (!user_mode(regs))
		die("exception", regs);
}



#define IER (0x00087200)

extern void *ram_exp_vector[];
extern unsigned int rx_exp_table[];

void __init trap_init(void)
{
	u8 ier;
#if defined(CONFIG_RAMKERNEL)
	int i;
	for (i = 0; i < 32; i++)
		ram_exp_vector[i] = &rx_exp_table[i];
#endif
	/* BUS error enabled */
	ier = __raw_readb((void __iomem*)(IER + 2));
	set_bit(ier, 0);
	__raw_writeb(ier, (void __iomem*)(IER + 2));
}

