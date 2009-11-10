#ifndef __ASM_RX_SYSTEM_H__
#define __ASM_RX_SYSTEM_H__

#include <linux/linkage.h>

asmlinkage extern struct task_struct *__switch_to(struct task_struct *prev,
						  struct task_struct *next);
#define switch_to(prev,next,last)		\
do {						\
	(last) = __switch_to(prev, next);	\
} while(0)

#define	irqs_disabled()				\
({						\
	unsigned long iflag;			\
	__asm__ volatile ("mvfc psw, %0\n\t"	\
			  "btst #16, %0\n\t"	\
			  "sz %0"		\
			  : "=r"(iflag));	\
	(iflag);				\
})

#define local_irq_disable()			\
do {						\
	unsigned long psw;			\
	__asm__ volatile ("mvfc psw, %0\n\t"	\
			  "bclr #16, %0\n\t"	\
			  "mvtc %0, psw"	\
			  : "=r"(psw));		\
}
#define local_irq_enable()			\
do {						\
	unsigned long psw;			\
	__asm__ volatile ("mvfc psw, %0\n\t"	\
			  "bset #16, %0\n\t"	\
			  "mvtc %0, psw"	\
			  : "=r"(psw));		\
} while(0)

#define local_save_flags(psw)			\
do {						\
	__asm__ volatile ("mvfc psw, %0"	\
			  : "=r"(psw));		\
} while(0)

#define local_irq_save(psw)	\
do {				\
	local_save_flags(psw);	\
	local_irq_disable();	\
} while(0)

#define local_irq_restore(psw)		\
do {					\
	__asm__ volatile("mvtc %0, psw"	\
			 ::"r"(psw));	\
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
		__asm__ volatile("xchg [%0].l %1"
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

void die(char *str, struct pt_regs *fp, unsigned long err);

#endif /* __ASM_RX_SYSTEM_H__ */
