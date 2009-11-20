#ifndef __ASM_RX_SYSTEM_H__
#define __ASM_RX_SYSTEM_H__

#include <linux/linkage.h>

#define switch_to(prev,next,last)			\
do {							\
	__asm__ volatile("pushm r6-r13\n\t"		\
			 "mov.l #1f,[%0]\n\t"		\
			 "mov.l r0,[%1]\n\t"		\
			 "mov.l %3,r0\n\t"		\
			 "jmp %2\n"			\
			 "1:\n\t"			\
			 "popm r6-r11\n\t"		\
			 ::"r"(&prev->thread.pc),	\
				 "r"(&prev->thread.sp),	\
				 "r"(next->thread.pc),	\
				 "r"(next->thread.sp));	\
	last = prev;					\
} while(0)

#define	irqs_disabled()				\
({						\
	unsigned long iflag;			\
	__asm__ volatile ("mvfc psw, %0\n\t"	\
			  "btst #16, %0\n\t"	\
			  "scz.l %0"		\
			  : "=r"(iflag));	\
	(iflag);				\
})

#define local_irq_disable()			\
	__asm__ volatile ("clrpsw i")
#define local_irq_enable()			\
	__asm__ volatile ("setpsw i")
	
#define local_save_flags(iflag)			\
do {						\
	__asm__ volatile ("mvfc psw, %0\n\t"	\
	                  "btst #16, %0\n\t"	\
			  "scnz.l %0"		\
			  : "=r"(iflag));	\
} while(0)

#define local_irq_save(iflag)	\
do {				\
	local_save_flags(iflag);	\
	local_irq_disable();	\
} while(0)

#define local_irq_restore(iflag)		\
do {						\
	if (iflag)				\
		__asm__ volatile("setpsw i");	\
	else					\
		__asm__ volatile("clrpsw i");	\
} while(0)

#define nop()  asm volatile ("nop"::)
#define mb()   asm volatile (""   : : :"memory")
#define rmb()  asm volatile (""   : : :"memory")
#define wmb()  asm volatile (""   : : :"memory")
#define set_mb(var, value) do { xchg(&var, value); } while (0)

#ifdef CONFIG_SMP
#define smp_mb()	mb()
#define smp_rmb()	rmb()
#define smp_wmb()	wmb()
#define smp_read_barrier_depends()	read_barrier_depends()
#else
#define smp_mb()	barrier()
#define smp_rmb()	barrier()
#define smp_wmb()	barrier()
#define smp_read_barrier_depends()	do { } while(0)
#endif

#define xchg(ptr,x) ((__typeof__(*(ptr)))__xchg((unsigned long)(x),(ptr),sizeof(*(ptr))))

struct __xchg_dummy { unsigned long a[100]; };
#define __xg(x) ((volatile struct __xchg_dummy *)(x))

static inline unsigned long __xchg(unsigned long x, volatile void * ptr, int size)
{
	unsigned long tmp, flags;

	switch (size) {
	case 1:
		local_irq_save(flags);
		tmp = *(u8 *)__xg(ptr);
		*(u8 *)__xg(ptr) = x;
		local_irq_restore(flags);
		break;
	case 2:
		local_irq_save(flags);
		tmp = *(u16 *)__xg(ptr);
		*(u16 *)__xg(ptr) = x;
		local_irq_restore(flags);
		break;
	case 4:
		__asm__ volatile("xchg %0, %1"
				 :"=m"(__xg(ptr)),"=r"(x));
		break;
	default:
		tmp = 0;	  
	}
	return tmp;
}

#include <asm-generic/cmpxchg-local.h>

/*
 * cmpxchg_local and cmpxchg64_local are atomic wrt current CPU. Always make
 * them available.
 */
#define cmpxchg_local(ptr, o, n)				  	       \
	((__typeof__(*(ptr)))__cmpxchg_local_generic((ptr), (unsigned long)(o),\
			(unsigned long)(n), sizeof(*(ptr))))
#define cmpxchg64_local(ptr, o, n) __cmpxchg64_local_generic((ptr), (o), (n))

#ifndef CONFIG_SMP
#include <asm-generic/cmpxchg.h>
#endif

#define arch_align_stack(x) (x)

#endif /* __ASM_RX_SYSTEM_H__ */
