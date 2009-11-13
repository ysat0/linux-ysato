#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <linux/unistd.h>
#include <linux/stddef.h>
#include <linux/personality.h>
#include <linux/freezer.h>
#include <linux/tracehook.h>
#include <asm/cacheflush.h>
#include <asm/ucontext.h>
#include <asm/uaccess.h>

#define _BLOCKABLE (~(sigmask(SIGKILL) | sigmask(SIGSTOP)))

int do_signal(struct pt_regs *, sigset_t *);

static int
restore_sigcontext(struct pt_regs *regs, struct sigcontext __user *sc,
		   int *r0_p)
{
	unsigned int err = 0;

	/* Always make any pending restarted system calls return -EINTR */
	current_thread_info()->restart_block.fn = do_no_restart_syscall;

#define COPY(x)		err |= __get_user(regs->x, &sc->sc_##x)
	COPY(r[0]); COPY(r[1]);
	COPY(r[2]); COPY(r[3]);
	COPY(r[4]); COPY(r[5]);
	COPY(r[6]); COPY(r[7]);
	COPY(r[8]); COPY(r[9]);
	COPY(r[10]); COPY(r[11]);
	COPY(r[12]); COPY(r[13]);
	COPY(r[14]); COPY(r[15]);
	COPY(psw); COPY(pc);
#undef COPY
	regs->vec = 0;
	return err;
}


static int
setup_sigcontext(struct sigcontext __user *sc, struct pt_regs *regs,
	         unsigned long mask)
{
	int err = 0;

#define COPY(x)	err |= __put_user(regs->x, &sc->sc_##x)
	COPY(r[0]); COPY(r[1]);
	COPY(r[2]); COPY(r[3]);
	COPY(r[4]); COPY(r[5]);
	COPY(r[6]); COPY(r[7]);
	COPY(r[8]); COPY(r[9]);
	COPY(r[10]); COPY(r[11]);
	COPY(r[12]); COPY(r[13]);
	COPY(r[14]); COPY(r[15]);
	COPY(psw); COPY(pc);
#undef COPY

	err |= __put_user(mask, &sc->oldmask);

	return err;
}

/*
 * Determine which stack to use..
 */
static inline void __user *
get_sigframe(struct k_sigaction *ka, unsigned long sp, size_t frame_size)
{
	/* This is the X/Open sanctioned signal stack switching.  */
	if (ka->sa.sa_flags & SA_ONSTACK) {
		if (sas_ss_flags(sp) == 0)
			sp = current->sas_ss_sp + current->sas_ss_size;
	}

	return (void __user *)((sp - frame_size) & -8ul);
}

#define SIMM16(n) (n) & 0x00ff, ((n) >> 8) & 0x00ff
static const u8 __retcode[8] = {
	0xfb, 0x8a, SIMM16(_NR_sigreturn),	/* mov.l #_NR_sigreturn,r8 */
	0x75, 0x60, 0x08, 			/* int 0x08 */
	0x03};					/* nop (padding) */

static const u8 __rt_retcode[8] = {
	0xfb, 0x8a, SIMM16(_NR_rt_sigreturn),	/* mov.l #_NR_rt_sigreturn,r8 */
	0x75, 0x60, 0x08, 			/* int 0x08 */
	0x03};					/* nop (padding) */

struct sigframe
{
	void __user *pretcode;
	struct sigcontext sc;
	unsigned long extramask[_NSIG_WORDS-1];
	u8 retcode[8];
};

struct rt_sigframe
{
	void __user *pretcode;
	struct siginfo info;
	struct ucontext uc;
	u8 retcode[8];
};

static void setup_frame(int sig, struct k_sigaction *ka,
			sigset_t *set, struct pt_regs *regs)
{
	struct sigframe __user *frame;
	int err = 0;

	frame = get_sigframe(ka, regs->usp, sizeof(*frame));

	if (!access_ok(VERIFY_WRITE, frame, sizeof(*frame)))
		goto give_sigsegv;

	signal = current_thread_info()->exec_domain
		&& current_thread_info()->exec_domain->signal_invmap
		&& sig < 32
		? current_thread_info()->exec_domain->signal_invmap[sig]
		: sig;

	err |= setup_sigcontext(&frame->sc, regs, set->sig[0]);

	if (_NSIG_WORDS > 1)
		err |= __copy_to_user(frame->extramask, &set->sig[1],
				      sizeof(frame->extramask));

	/* Set up to return from userspace.  If provided, use a stub
	   already in userspace.  */
	if (ka->sa.sa_flags & SA_RESTORER) {
		regs->pr = (unsigned long) ka->sa.sa_restorer;
	} else {
		/* setup retcode */
		err |= __put_user(*((u64 *)&__rt_retcode), (u64 *)frame->retcode);
		err |= __put_user(frame->retcode, frame->pretcode);
		if (err)
			goto give_sigsegv;
	}

	/* Set up registers for signal handler */
	regs->usp = (unsigned long)frame;
	regs->r[1] = signal;	/* Arg for signal handler */
	regs->r[2] = 0;
	regs->r[3] = (unsigned long)&frame->sc;
	regs->pc = (unsigned long)ka->sa.sa_handler;

	set_fs(USER_DS);

#if DEBUG_SIG
	printk("SIG deliver (%s:%d): sp=%p pc=%p\n",
		current->comm, current->pid, frame, regs->pc);
#endif

	return;

give_sigsegv:
	force_sigsegv(sig, current);
}

static void setup_rt_frame(int sig, struct k_sigaction *ka, siginfo_t *info,
			   sigset_t *set, struct pt_regs *regs)
{
	struct rt_sigframe __user *frame;
	int err = 0;
	int signal;

	frame = get_sigframe(ka, regs->spu, sizeof(*frame));

	if (!access_ok(VERIFY_WRITE, frame, sizeof(*frame)))
		goto give_sigsegv;

	signal = current_thread_info()->exec_domain
		&& current_thread_info()->exec_domain->signal_invmap
		&& sig < 32
		? current_thread_info()->exec_domain->signal_invmap[sig]
		: sig;

	err |= copy_siginfo_to_user(&frame->info, info);
	if (err)
		goto give_sigsegv;

	/* Create the ucontext.  */
	err |= __put_user(0, &frame->uc.uc_flags);
	err |= __put_user(0, &frame->uc.uc_link);
	err |= __put_user(current->sas_ss_sp, &frame->uc.uc_stack.ss_sp);
	err |= __put_user(sas_ss_flags(regs->spu),
			  &frame->uc.uc_stack.ss_flags);
	err |= __put_user(current->sas_ss_size, &frame->uc.uc_stack.ss_size);
	err |= setup_sigcontext(&frame->uc.uc_mcontext, regs, set->sig[0]);
	err |= __copy_to_user(&frame->uc.uc_sigmask, set, sizeof(*set));
	if (err)
		goto give_sigsegv;

	/* setup retcode */
	err |= __put_user(*((u64 *)&__rt_retcode), (u64 *)frame->retcode);
	err |= __put_user(frame->retcode, frame->pretcode);
	if (err)
		goto give_sigsegv;

	/* Set up registers for signal handler */
	regs->usp = (unsigned long)frame;
	regs->r[1] = signal;	/* Arg for signal handler */
	regs->r[2] = (unsigned long)&frame->info;
	regs->r[3] = (unsigned long)&frame->uc;
	regs->pc = (unsigned long)ka->sa.sa_handler;

	set_fs(USER_DS);

#if DEBUG_SIG
	printk("SIG deliver (%s:%d): sp=%p pc=%p\n",
		current->comm, current->pid, frame, regs->pc);
#endif

	return;

give_sigsegv:
	force_sigsegv(sig, current);
}

/*
 * OK, we're invoking a handler
 */

static inline void
handle_syscall_restart(unsigned long save_r1, struct pt_regs *regs,
		       struct sigaction *sa)
{
}

static int 
handle_signal(unsigned long sig, struct k_sigaction *ka, siginfo_t *info,
	      sigset_t *oldset, struct pt_regs *regs, unsigned long saved_r1)
{
	unsigned short inst;

	/* Are we from a system call? */
	if (regs->vec >= 0x1000) {
		/* check for system call restart.. */
		switch (regs->r[1]) {
		case -ERESTART_RESTARTBLOCK:
		case -ERESTARTNOHAND:
			regs->regs[1] = -EINTR;
			break;
			
		case -ERESTARTSYS:
			if (!(sa->sa_flags & SA_RESTART)) {
				regs->regs[1] = -EINTR;
				break;
			}
			/* fallthrough */
		case -ERESTARTNOINTR:
			regs->regs[1] = saved_r1;
			regs->pc -= 3;
			break;
		}
	}
	/* Set up the stack frame */
	if (ka->sa.sa_flags & SA_SIGINFO)
		ret = setup_rt_frame(sig, ka, info, oldset, regs);
	else
		ret = setup_frame(sig, ka, oldset, regs);
	if (ret)
		return ret;

	if (ka->sa.sa_flags & SA_ONESHOT)
		ka->sa.sa_handler = SIG_DFL;

	spin_lock_irq(&current->sighand->siglock);
	sigorsets(&current->blocked,&current->blocked,&ka->sa.sa_mask);
	if (!(ka->sa.sa_flags & SA_NODEFER))
		sigaddset(&current->blocked,sig);
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);
	return 0;
}

/*
 * Note that 'init' is a special process: it doesn't get signals it doesn't
 * want to handle. Thus you cannot kill init even with a SIGKILL even by
 * mistake.
 */
static int do_signal(struct pt_regs *regs, sigset_t *oldset, unsigned long saved_r1)
{
	siginfo_t info;
	int signr;
	sigset_t *oldset;
	struct k_sigaction ka;

	/*
	 * We want the common case to go fast, which
	 * is why we may in certain cases get here from
	 * kernel mode. Just return without doing anything
	 * if so.
	 */
	if (!user_mode(regs))
		return 1;

	if (try_to_freeze()) 
		goto no_signal;

	if (test_thread_flag(TIF_RESTORE_SIGMASK))
		oldset = &current->saved_sigmask;
	else
		oldset = &current->blocked;

	signr = get_signal_to_deliver(&info, &ka, regs, NULL);
	if (signr > 0) {
		/* Re-enable any watchpoints before delivering the
		 * signal to user space. The processor register will
		 * have been cleared if the watchpoint triggered
		 * inside the kernel.
		 */

		/* Whee!  Actually deliver the signal.  */
		handle_signal(signr, &ka, &info, oldset, regs, saved_r1);
		return 1;
	}

 no_signal:
	/* Did we come from a system call? */
	if (regs->vec >= 0x1000) {
		/* Restart the system call - no handlers present */
		if (regs->r[1] == -ERESTARTNOHAND ||
		    regs->r[1] == -ERESTARTSYS ||
		    regs->r[1] == -ERESTARTNOINTR) {
			regs->r[1] = saved_r1;
			regs->pc -= 3;
		}
		if (regs->r1 == -ERESTART_RESTARTBLOCK){
			regs->r[1] = saved_r1;
			regs->r[8] = __NR_restart_syscall;
			regs->pc -=3;
		}
	}
	return 0;
}

/*
 * notification of userspace execution resumption
 * - triggered by current->work.notify_resume
 */
void do_notify_resume(struct pt_regs *regs, unsigned long thread_info_flags,
		      unsigned long saved_r1)
{

	/* Pending single-step? */
	if (thread_info_flags & _TIF_SINGLESTEP)
		clear_thread_flag(TIF_SINGLESTEP);

	/* deal with pending signal delivery */
	if (thread_info_flags & _TIF_SIGPENDING)
		do_signal(regs, saved_r1);

	if (thread_info_flags & _TIF_NOTIFY_RESUME) {
		clear_thread_flag(TIF_NOTIFY_RESUME);
		tracehook_notify_resume(regs);
		if (current->replacement_session_keyring)
			key_replace_session_keyring();
	}

	clear_thread_flag(TIF_IRET);
}
