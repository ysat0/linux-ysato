#ifndef __ASM_RX_PROCESSOR_H__
#define __ASM_RX_PROCESSOR_H__

/*
 * Default implementation of macro that returns current
 * instruction pointer ("program counter").
 */
#define current_text_addr() ({ __label__ _l; _l: &&_l;})

#include <linux/compiler.h>
#include <asm/segment.h>
#include <asm/fpu.h>
#include <asm/ptrace.h>
#include <asm/current.h>

/*
 * User space process size: 3.75GB. This is hardcoded into a few places,
 * so don't change it unless you know what you are doing.
 */
#define TASK_SIZE	(0xFFFFFFFFUL)

#ifdef __KERNEL__
#define STACK_TOP	TASK_SIZE
#define STACK_TOP_MAX	STACK_TOP
#endif

/*
 * This decides where the kernel will search for a free chunk of vm
 * space during mmap's. We won't be using it
 */
#define TASK_UNMAPPED_BASE	0

struct thread_struct {
	unsigned long  ksp;		/* kernel stack pointer */
	unsigned long  usp;		/* user stack pointer */
	unsigned long  psw;		/* saved status register */
	unsigned long  esp0;            /* points to SR of stack frame */
	struct {
		unsigned short *addr;
		unsigned char inst;
	} breakinfo;
};

#define INIT_THREAD  {						\
	.ksp  = sizeof(init_stack) + (unsigned long)init_stack, \
	.usp  = 0,						\
	.ccr  = 0x00100000,					\
	.esp0 = 0,						\
	.breakinfo = {						\
		.addr = (unsigned short *)-1,			\
		.inst = 0					\
	}							\
}

/*
 * Do necessary setup to start up a newly executed thread.
 *
 * pass the data segment into user programs if it exists,
 * it can't hurt anything as far as I can tell
 */
#define start_thread(_regs, _pc, _usp)			        \
do {							        \
	set_fs(USER_DS);           /* reads from user space */  \
	(_regs)->pc = (_pc);				        \
	(_regs)->psw = 0x00020000; /* USP */			\
	wrusp(((unsigned long)(_usp)));		                \
} while(0)
#endif

/* Forward declaration, a strange C thing */
struct task_struct;

/* Free all resources held by a thread. */
static inline void release_thread(struct task_struct *dead_task)
{
}

extern int kernel_thread(int (*fn)(void *), void * arg, unsigned long flags);

static inline void prepare_to_copy(tsk)
{
}

/*
 * Free current thread data structures etc..
 */
static inline void exit_thread(void)
{
}

/*
 * Return saved PC of a blocked thread.
 */
unsigned long thread_saved_pc(struct task_struct *tsk);
unsigned long get_wchan(struct task_struct *p);

#define task_pt_regs(tsk) ((struct pt_regs *)(tsk)->thread.sp0 - 1)
#define	KSTK_EIP(tsk) (task_pt_regs(task)->pc)
#define	KSTK_ESP(tsk) (task_pt_regs(task)->r[0])

#define cpu_relax()    barrier()

#endif
