#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/smp.h>
#include <linux/sem.h>
#include <linux/msg.h>
#include <linux/shm.h>
#include <linux/stat.h>
#include <linux/syscalls.h>
#include <linux/mman.h>
#include <linux/file.h>
#include <linux/utsname.h>
#include <linux/ipc.h>
#include <linux/linkage.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>

/*
 * Do a system call from kernel instead of calling sys_execve so we
 * end up with proper pt_regs.
 */
int kernel_execve(const char *filename, char *const argv[], char *const envp[])
{
	register long __r8 __asm__ ("r8") = __NR_execve;
	register long __r1 __asm__ ("r1") = (long) filename;
	register long __r2 __asm__ ("r2") = (long) argv;
	register long __r3 __asm__ ("r3") = (long) envp;
	__asm__ __volatile__ ("int #0x08" : "=r" (__r1)
			: "r" (__r8), "r" (__r1), "r" (__r2), "r" (__r3)
			: "memory");
	return __r1;
}

asmlinkage int sys_cacheflush(unsigned long addr, unsigned long len, int op)
{
	/* Not affected and always success */
	return 0;
}

asmlinkage long sys_mmap2(unsigned long addr, unsigned long len,
			  unsigned long prot, unsigned long flags,
			  unsigned long fd, unsigned long pgoff)
{
	int error = -EBADF;
	struct file * file = NULL;

	/*
	 * The shift for mmap2 is constant, regardless of PAGE_SIZE
	 * setting.
	 */
	if (pgoff & ((1 << (PAGE_SHIFT - 12)) - 1))
		return -EINVAL;

	pgoff >>= PAGE_SHIFT - 12;

	flags &= ~(MAP_EXECUTABLE | MAP_DENYWRITE);
	if (!(flags & MAP_ANONYMOUS)) {
		file = fget(fd);
		if (!file)
			goto out;
	}

	down_write(&current->mm->mmap_sem);
	error = do_mmap_pgoff(file, addr, len, prot, flags, pgoff);
	up_write(&current->mm->mmap_sem);

	if (file)
		fput(file);
out:
	return error;
}

/*
 * sys_ipc() is the de-multiplexer for the SysV IPC calls..
 *
 * This is really horribly ugly.
 */
asmlinkage int sys_ipc(uint call, int first, int second,
		       int third, void __user *ptr, long fifth)
{
	int version, ret;

	version = call >> 16; /* hack for backward compatibility */
	call &= 0xffff;

	if (call <= SEMTIMEDOP)
		switch (call) {
		case SEMOP:
			return sys_semtimedop(first,
					      (struct sembuf __user *)ptr,
					      second, NULL);
		case SEMGET:
			return sys_semget (first, second, third);
		case SEMCTL: {
			union semun fourth;
			if (!ptr)
				return -EINVAL;
			if (get_user(fourth.__pad, (void __user * __user *) ptr))
				return -EFAULT;
			return sys_semctl (first, second, third, fourth);
			}
		case SEMTIMEDOP:
			return sys_semtimedop(first,
				(struct sembuf __user *)ptr, second,
			        (const struct timespec __user *)fifth);
		default:
			return -EINVAL;
		}

	if (call <= MSGCTL)
		switch (call) {
		case MSGSND:
			return sys_msgsnd (first, (struct msgbuf __user *) ptr,
					  second, third);
		case MSGRCV:
			switch (version) {
			case 0:
			{
				struct ipc_kludge tmp;

				if (!ptr)
					return -EINVAL;

				if (copy_from_user(&tmp,
					(struct ipc_kludge __user *) ptr,
						   sizeof (tmp)))
					return -EFAULT;

				return sys_msgrcv (first, tmp.msgp, second,
						   tmp.msgtyp, third);
			}
			default:
				return sys_msgrcv (first,
						   (struct msgbuf __user *) ptr,
						   second, fifth, third);
			}
		case MSGGET:
			return sys_msgget ((key_t) first, second);
		case MSGCTL:
			return sys_msgctl (first, second,
					   (struct msqid_ds __user *) ptr);
		default:
			return -EINVAL;
		}
	if (call <= SHMCTL)
		switch (call) {
		case SHMAT: {
			ulong raddr;
			ret = do_shmat (first, (char __user *) ptr,
					second, &raddr);
			if (ret)
				return ret;
			return put_user (raddr, (ulong __user *) third);
		}
		case SHMDT:
			return sys_shmdt ((char __user *)ptr);
		case SHMGET:
			return sys_shmget (first, second, third);
		case SHMCTL:
			return sys_shmctl (first, second,
					   (struct shmid_ds __user *) ptr);
		default:
			return -EINVAL;
		}

	return -EINVAL;
}
