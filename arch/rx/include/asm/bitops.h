#ifndef __ASM_RX_BITOPS_H__
#define __ASM_RX_BITOPS_H__

#include <linux/compiler.h>
#include <asm/system.h>

#ifdef __KERNEL__

#ifndef _LINUX_BITOPS_H
#error only <linux/bitops.h> can be included directly
#endif

/*
 * Function prototypes to keep gcc -Wall happy
 */

/*
 * ffz = Find First Zero in word. Undefined if no zero exists,
 * so code should check against ~0UL first..
 */
static __inline__ unsigned long ffz(unsigned long word)
{
	unsigned long result = -1;

	__asm__("1:\n\t"
		"add #1,%0\n\t"
		"shlr #1,%1\n\t"
		"bc 1b"
		: "=r" (result)
		: "r" (word), "0"(result));
	return result;
}

#define RX_GEN_BITOP(FNAME,OP)						\
static __inline__ void FNAME(int nr, volatile unsigned long* addr) 	\
{									\
	volatile unsigned char *b_addr;					\
	b_addr = (volatile unsigned char *)addr + ((nr >> 3));		\
	__asm__ volatile (OP " %1,%0.b":"=m"(*b_addr):"ri"(nr & 7));	\
}
/*
 * clear_bit() doesn't provide any barrier for the compiler.
 */
#define smp_mb__before_clear_bit()	barrier()
#define smp_mb__after_clear_bit()	barrier()

RX_GEN_BITOP(set_bit   ,"bset")
RX_GEN_BITOP(clear_bit ,"bclr")
RX_GEN_BITOP(change_bit,"bnot")
#define __set_bit(nr,addr)    set_bit((nr),(addr))
#define __clear_bit(nr,addr)  clear_bit((nr),(addr))
#define __change_bit(nr,addr) change_bit((nr),(addr))

#undef RX_GEN_BITOP
#undef RX_GEN_BITOP_CONST

static __inline__ int test_bit(int nr, const unsigned long* addr)
{
	volatile unsigned char *b_addr;
	int result;
	b_addr = (volatile unsigned char *)addr + ((nr >> 3));
	__asm__ volatile ("btst %1, %2.b\n\t"
			  "scnz.l %0"
			  :"=r"(result):"ri"(nr & 7), "Q"(*b_addr));
	return result;
}

#define __test_bit(nr, addr) test_bit(nr, addr)

#define RX_GEN_TEST_BITOP(FNNAME,OP)				\
static __inline__ int FNNAME(int nr, volatile void * addr)	\
{								\
	int result;						\
	unsigned long psw;					\
	volatile unsigned char *b_addr;				\
	b_addr = (volatile unsigned char *)addr + ((nr >> 3));	\
	__asm__ volatile("mvfc psw, %2\n\t"			\
			 "clrpsw i\n\t"				\
			 "btst %3, %1.b\n\t"			\
			 OP " %3, %1.b\n\t"			\
			 "scnz.l %0\n\t"			\
			 "mvtc %2, psw"				\
			 : "=r"(result),"+m"(*b_addr),"=r"(psw)	\
			 :"ri"(nr & 7):"cc");			\
	return result;						\
}								\
								\
static __inline__ int __ ## FNNAME(int nr, volatile void * addr)\
{								\
	int result;						\
	volatile unsigned char *b_addr;				\
	b_addr = (volatile unsigned char *)addr + ((nr >> 3));  \
	__asm__ volatile("btst %2, %1.b\n\t"			\
			 OP " %2, %1.b\n\t"			\
			 "scnz.l %0"				\
			 :"=r"(result),"+m"(*b_addr)		\
			 :"ri"(nr & 7));			\
	return result;						\
}

RX_GEN_TEST_BITOP(test_and_set_bit,   "bset")
RX_GEN_TEST_BITOP(test_and_clear_bit, "bclr")
RX_GEN_TEST_BITOP(test_and_change_bit,"bnot")
#undef RX_GEN_TEST_BITOP_CONST
#undef RX_GEN_TEST_BITOP

#include <asm-generic/bitops/ffs.h>

static __inline__ unsigned long __ffs(unsigned long word)
{
	unsigned long result;

	result = -1;
	__asm__ volatile("1:\n\t"
			 "add #1,%0\n\t"
			 "shlr #1,%1\n\t"
			 "bnc 1b"
			 : "=r" (result)
			 : "r"(word),"0"(result));
	return result;
}

#include <asm-generic/bitops/find.h>
#include <asm-generic/bitops/sched.h>
#include <asm-generic/bitops/hweight.h>
#include <asm-generic/bitops/lock.h>
#include <asm-generic/bitops/ext2-atomic.h>

#endif /* __KERNEL__ */

#include <asm-generic/bitops/fls.h>
#include <asm-generic/bitops/__fls.h>
#include <asm-generic/bitops/fls64.h>

#endif /* __ASM_RX_BITOPS_H__ */
