/*
 * linux/arch/h8300/kernel/sys_h8300.c
 *
 * This file contains various random system calls that
 * have a non-standard calling sequence on the H8/300
 * platform.
 */

#include <asm/setup.h>
#include <asm/uaccess.h>

/* sys_cacheflush -- no support.  */
asmlinkage int
sys_cacheflush (unsigned long addr, int scope, int cache, unsigned long len)
{
	return -EINVAL;
}

asmlinkage int sys_getpagesize(void)
{
	return PAGE_SIZE;
}
