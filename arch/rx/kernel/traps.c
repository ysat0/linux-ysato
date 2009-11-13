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
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/io.h>
#include <linux/bug.h>
#include <linux/limits.h>
#include <linux/proc_fs.h>
#include <linux/sysfs.h>
#include <asm/system.h>
#include <asm/uaccess.h>

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

#define DO_ERROR_INFO(trapnr, signr, str, name, sicode, siaddr)		\
dotraplinkage void do_##name(struct pt_regs *regs)			\
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
	static const int fpsw_map[] = { FLT_FLTINV, FLT_FLTOVF, FLT_FLTDIV, 
					FLT_FLTUND, FLT_FLTRES};
	int bit;
	unsigned int fpsw;
	__asm__ volatile("mvfc fpsw,%0":"=r"(fpsw));
	for (bit = 26; bit < 31; bit++)
		if (btst(bit,fpsw)) {
			bclr(bit, fpsw);
			__asm__ volatile("mvtc %0,fpsw"::"r"(fpsw));
			return fpsw_map[bit - 26];
		}
	return 0;
}

dotraplinkage void do_fpuerror(struct pt_regs *regs)
{
	siginfo_t info;
	info.si_signo = fpsw_decode();
	info.si_errno = 0;
	info.si_addr = (void __user *)regs->pc
	if (notify_die(DIE_TRAP, "Floating point exceprion", 
		       regs, 0, 25, SIGFPE) == NOTIFY_STOP)
		return;
	conditional_sti(regs);
	do_trap(signr, str, regs, &info);
}

dotraplinkage void do_nmi(struct pt_regs *regs)
{
	nmi_enter();

	inc_irq_stat(__nmi_count);
	notify_die(DIE_NMI, "nmi", regs, 0, 31, SIGINT);

	nmi_exit();
}

void __init trap_init(void)
{
#if defined(CONFIG_RAMKERNEL)
	for (i == 0; i < 32; i++)
		ram_exp_vector[i] = &rx_exp_table[i];
#ednif
	/* BUS error enabled */
	ier = ctrl_inb(IER + 2);
	set_bit(ier, 0);
	ctrl_outb(IER + 2, ier);
}
