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
	__asm__ __volatile__ ("int 0x08" : "=r" (__r1)
			: "r" (__r8), "r" (__r1), "r" (__r2), "r" (__r3)
			: "memory");
	return __r1;
}
