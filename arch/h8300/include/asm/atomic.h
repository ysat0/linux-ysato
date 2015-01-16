#ifndef __ARCH_H8300_ATOMIC__
#define __ARCH_H8300_ATOMIC__

#include <linux/types.h>
#include <asm/cmpxchg.h>

/*
 * Atomic operations that C can't guarantee us.  Useful for
 * resource counting etc..
 */

#define ATOMIC_INIT(i)	{ (i) }

#define atomic_read(v)		(*(volatile int *)&(v)->counter)
#define atomic_set(v, i)	(((v)->counter) = i)

#include <linux/kernel.h>

static inline int atomic_add_return(int i, atomic_t *v)
{
	unsigned short ccr;
	int ret;

	__asm__ __volatile__ (
		"stc ccr,%w2\n\t"
		"orc #0x80,ccr\n\t"
		"mov.l %1,%0\n\t"
		"add.l %3,%0\n\t"
		"mov.l %0,%1\n\t"
		"ldc %w2,ccr"
		: "=r"(ret), "+m"(v->counter), "=r"(ccr)
		: "ri"(i));
	return ret;
}

#define atomic_add(i, v) atomic_add_return(i, v)
#define atomic_add_negative(a, v)	(atomic_add_return((a), (v)) < 0)

static inline int atomic_sub_return(int i, atomic_t *v)
{
	unsigned short ccr;
	int ret;

	__asm__ __volatile__ (
		"stc ccr,%w2\n\t"
		"orc #0x80,ccr\n\t"
		"mov.l %1,%0\n\t"
		"sub.l %3,%0\n\t"
		"mov.l %0,%1\n\t"
		"ldc %w2,ccr"
		: "=r"(ret), "+m"(v->counter), "=r"(ccr)
		: "ri"(i));
	return ret;
}

#define atomic_sub(i, v) atomic_sub_return(i, v)
#define atomic_sub_and_test(i, v) (atomic_sub_return(i, v) == 0)

static inline int atomic_inc_return(atomic_t *v)
{
	unsigned short ccr;
	int ret;

	__asm__ __volatile__ (
		"stc ccr,%w2\n\t"
		"orc #0x80,ccr\n\t"
		"mov.l %1,%0\n\t"
		"inc.l #1,%0\n\t"
		"mov.l %0,%1\n\t"
		"ldc %w2,ccr"
		: "=r"(ret), "+m"(v->counter), "=r"(ccr));
	return ret;
}

#define atomic_inc(v) atomic_inc_return(v)

/*
 * atomic_inc_and_test - increment and test
 * @v: pointer of type atomic_t
 *
 * Atomically increments @v by 1
 * and returns true if the result is zero, or false for all
 * other cases.
 */
#define atomic_inc_and_test(v) (atomic_inc_return(v) == 0)

static inline int atomic_dec_return(atomic_t *v)
{
	unsigned short ccr;
	int ret;

	__asm__ __volatile__ (
		"stc ccr,%w2\n\t"
		"orc #0x80,ccr\n\t"
		"mov.l %1,%0\n\t"
		"dec.l #1,%0\n\t"
		"mov.l %0,%1\n\t"
		"ldc %w2,ccr"
		: "=r"(ret), "+m"(v->counter), "=r"(ccr));
	return ret;
}

#define atomic_dec(v) atomic_dec_return(v)

static inline int atomic_dec_and_test(atomic_t *v)
{
	unsigned short ccr;
	int ret;

	__asm__ __volatile__ (
		"stc ccr,%w2\n\t"
		"orc #0x80,ccr\n\t"
		"mov.l %1,%0\n\t"
		"dec.l #1,%0\n\t"
		"mov.l %0,%1\n\t"
		"ldc %w2,ccr"
		: "=r"(ret), "+m"(v->counter), "=r"(ccr));
	return ret == 0;
}

static inline int atomic_cmpxchg(atomic_t *v, int old, int new)
{
	int ret;
	unsigned short ccr;

	__asm__ __volatile__ (
		"stc ccr,%w2\n\t"
		"orc #0x80,ccr\n\t"
		"mov.l %1,%0\n\t"
		"cmp.l %3,%0\n\t"
		"bne 1f\n\t"
		"mov.l %4,%1\n"
		"1:\tldc %w2,ccr"
		: "=r"(ret), "+m"(v->counter), "=r"(ccr)
		: "g"(old), "r"(new));
	return ret;
}

static inline int __atomic_add_unless(atomic_t *v, int a, int u)
{
	int ret;
	unsigned char ccr;

	__asm__ __volatile__ (
		"stc ccr,%w2\n\t"
		"orc #0x80,ccr\n\t"
		"mov.l %1,%0\n\t"
		"cmp.l %4,%0\n\t"
		"beq 1f\n\t"
		"add.l %0,%3\n\t"
		"mov.l %3,%1\n"
		"1:\tldc %w2,ccr"
		: "=r"(ret), "+m"(v->counter), "=r"(ccr), "+r"(a)
		: "ri"(u));
	return ret;
}

static inline void atomic_clear_mask(unsigned long mask, unsigned long *v)
{
	unsigned char ccr;
	unsigned long tmp;

	__asm__ __volatile__(
		"stc ccr,%w3\n\t"
		"orc #0x80,ccr\n\t"
		"mov.l %0,%1\n\t"
		"and.l %2,%1\n\t"
		"mov.l %1,%0\n\t"
		"ldc %w3,ccr"
		: "=m"(*v), "=r"(tmp)
		: "g"(~(mask)), "r"(ccr));
}

static inline void atomic_set_mask(unsigned long mask, unsigned long *v)
{
	unsigned char ccr;
	unsigned long tmp;

	__asm__ __volatile__(
		"stc ccr,%w3\n\t"
		"orc #0x80,ccr\n\t"
		"mov.l %0,%1\n\t"
		"or.l %2,%1\n\t"
		"mov.l %1,%0\n\t"
		"ldc %w3,ccr"
		: "=m"(*v), "=r"(tmp)
		: "g"(~(mask)), "r"(ccr));
}

/* Atomic operations are already serializing */
#define smp_mb__before_atomic_dec()    barrier()
#define smp_mb__after_atomic_dec() barrier()
#define smp_mb__before_atomic_inc()    barrier()
#define smp_mb__after_atomic_inc() barrier()

#endif /* __ARCH_H8300_ATOMIC __ */
