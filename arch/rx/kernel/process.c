/*
 * arch/sh/kernel/process.c
 *
 * This file handles the architecture-dependent parts of process handling..
 *
 *  Copyright (C) 1995  Linus Torvalds
 *
 *  RX version:  Copyright (C) 2009 Yoshinori Sato
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/elfcore.h>
#include <linux/pm.h>
#include <linux/kallsyms.h>
#include <linux/kexec.h>
#include <linux/kdebug.h>
#include <linux/tick.h>
#include <linux/reboot.h>
#include <linux/fs.h>
#include <linux/ftrace.h>
#include <linux/preempt.h>
#include <asm/uaccess.h>
#include <asm/mmu_context.h>
#include <asm/pgalloc.h>
#include <asm/system.h>
#include <asm/ubc.h>
#include <asm/fpu.h>
#include <asm/syscalls.h>
#include <asm/watchdog.h>

void __noreturn machine_restart(char * __unused)
{
	unsigned long tmp;
	local_irq_disable();
	__asm__ voaltile("mov.l #0xfffffffc,%0\n\t"
			 "mov.l [%0],%0\n\t"
			 "jmp %0"
			 ::"r"(tmp));
}

void __noreturn machine_halt(void)
{
	local_irq_disable();

	while (1)
		cpu_sleep();
}

void machine_power_off(void)
{
	/* Nothing do. */
}

void show_regs(struct pt_regs * regs)
{
	printk("\n");
	printk("Pid : %d, Comm: \t\t%s\n", task_pid_nr(current), current->comm);
	print_symbol("PC is at %s\n", instruction_pointer(regs));

	printk("PC  : %08lx SP  : %08lx PSW  : %08lx\n",
	       regs->pc, regs->regs[0], regs->psw);
	printk("R1  : %08lx R2  : %08lx R3  : %08lx\n",
	       regs->regs[1],
	       regs->regs[2],regs->regs[3]);
	printk("R4  : %08lx R5  : %08lx R6  : %08lx R7  : %08lx\n",
	       regs->regs[4],regs->regs[5],
	       regs->regs[6],regs->regs[7]);
	printk("R8  : %08lx R9  : %08lx R10 : %08lx R11 : %08lx\n",
	       regs->regs[8],regs->regs[9],
	       regs->regs[10],regs->regs[11]);
	printk("R12 : %08lx R13 : %08lx R14 : %08lx R15 : %08lx\n",
	       regs->regs[12],regs->regs[13],
	       regs->regs[14],regs->regs[15]);

	show_trace(NULL, (unsigned long *)regs->regs[0], regs);
}

/*
 * Create a kernel thread
 */
static void __noreturn kernel_thread_helper(void *nouse, int (*fn)(void *), void *arg)
{
	fn(arg);
	do_exit(-1);
}

int kernel_thread(int (*fn)(void *), void * arg, unsigned long flags)
{
	struct pt_regs regs;

	memset(&regs, 0, sizeof(regs));
	regs.regs[1] = (unsigned long)fn;
	regs.regs[2] = (unsigned long)arg;

	regs.pc = (unsigned long)kernel_thread_helper;
	regs.sr = 0;

	/* Ok, create the new process.. */
	return do_fork(flags | CLONE_VM | CLONE_UNTRACED, 0,
		      &regs, 0, NULL, NULL);
}

/*
 * Free current thread data structures etc..
 */
void exit_thread(void)
{
}

void flush_thread(void)
{
}

void release_thread(struct task_struct *dead_task)
{
	/* do nothing */
}

asmlinkage void ret_from_fork(void);

int copy_thread(unsigned long clone_flags,
                unsigned long usp, unsigned long topstk,
		 struct task_struct * p, struct pt_regs * regs)
{
	struct pt_regs * childregs;

	childregs = (struct pt_regs *) (THREAD_SIZE + task_stack_page(p)) - 1;

	*childregs = *regs;
	childregs->pc = (unsigned long) ret_from_fork;
	childregs->r[1] = 0;

	p->thread.usp = usp;
	p->thread.ksp = (unsigned long)childregs;

	return 0;
}

asmlinkage int sys_fork(void)
{
	return -EINVAL;
}

asmlinkage int rx_clone(struct pt_regs *regs)
{
	unsigned long clone_flags = regs->r[1];
	unsigned long newsp = regs->r[2];
	unsigned long parent_tidptr = regs->r[3];
	unsigned long child_tidptr = regs->r[4];

	if (!newsp)
		newsp = regs->usp;
	return do_fork(clone_flags, newsp, regs, 0,
			(int __user *)parent_tidptr,
			(int __user *)child_tidptr);
}

asmlinkage int rx_vfork(struct pt_regs regs)
{
	return do_fork(CLONE_VFORK | CLONE_VM | SIGCHLD, regs->usp, regs,
		       0, NULL, NULL);
}

/*
 * sys_execve() executes a new program.
 */
asmlinkage int sys_execve(char __user *ufilename, char __user * __user *uargv,
			  char __user * __user *uenvp, int dummy, ...)
{
	struct pt_regs *regs = (struct pt_regs *)
		((unsigned char *)&dummy - 4)
	int error;
	char *filename;

	filename = getname(ufilename);
	error = PTR_ERR(filename);
	if (IS_ERR(filename))
		goto out;

	error = do_execve(filename, uargv, uenvp, regs);
	putname(filename);
out:
	return error;
}

unsigned long get_wchan(struct task_struct *p)
{
	unsigned long pc;

	if (!p || p == current || p->state == TASK_RUNNING)
		return 0;

	stack_page = (unsigned long)p;
	fp = ((struct pt_regs *)p->thread.ksp)->r6;
	do {
		if (fp < stack_page+sizeof(struct thread_info) ||
		    fp >= (THREAD_SIZE - sizeof(pt_regs) +stack_page)
			return 0;
		pc = ((unsigned long *)fp)[1];
		if (!in_sched_functions(pc))
			return pc;
		fp = *(unsigned long *) fp;
	} while (count++ < 16);
	return 0;
}
