#ifndef _H8300_IRQFLAGS_H
#define _H8300_IRQFLAGS_H

#ifdef CONFIG_CPU_H8300H
static inline unsigned char arch_local_save_flags(void)
{
	unsigned char flags;
	asm volatile ("stc ccr,%w0" : "=r" (flags));
	return flags;
}

static inline void arch_local_irq_disable(void)
{
	asm volatile ("orc  #0xc0,ccr" : : : "cc", "memory");
}

static inline void arch_local_irq_enable(void)
{
	asm volatile ("andc #0x3f,ccr" : : : "cc", "memory");
}

static inline unsigned char arch_local_irq_save(void)
{
	unsigned char flags;
	asm volatile ("stc ccr,%w0\n\t"
		      "orc  #0xc0,ccr" :"=r" (flags) : : "cc", "memory");
	return flags;
}

static inline void arch_local_irq_restore(unsigned char flags)
{
	asm volatile ("ldc %w0,ccr" : : "r" (flags) : "cc","memory");
}

static inline int arch_irqs_disabled_flags(unsigned long flags)
{
	return (flags & 0xc0) == 0xc0;
}
#endif
#ifdef CONFIG_CPU_H8S
static inline unsigned long arch_local_save_flags(void)
{
	unsigned short flags;
	asm volatile ("stc ccr,%w0\n\tstc exr,%x0" : "=r" (flags));
	return flags;
}

static inline void arch_local_irq_disable(void)
{
	asm volatile ("orc  #0x80,ccr\n\t"
		      "orc #0x07,exr" : : : "cc", "memory");
}

static inline void arch_local_irq_enable(void)
{
	asm volatile ("andc #0x7f,ccr\n\t"
		      "andc #0xf0,exr\n\t": : : "cc", "memory");
}

static inline unsigned long arch_local_irq_save(void)
{
	unsigned short flags;
	asm volatile ("stc ccr,%w0\n\t"
		      "stc exr,%x0\n\t"
		      "orc  #0x80,ccr\n\t"
		      "orc #0x07,exr"
		      : "=r" (flags) : : "cc", "memory");
	return flags;
}

static inline void arch_local_irq_restore(unsigned long flags)
{
	asm volatile ("ldc %w0,ccr\n\t"
		      "ldc %x0,exr"
		      : : "r" (flags) : "cc", "memory");
}

static inline int arch_irqs_disabled_flags(unsigned short flags)
{
	return (flags & 0x0780) == 0x0780;
}

#endif

static inline int arch_irqs_disabled(void)
{
	return arch_irqs_disabled_flags(arch_local_save_flags());
}

#endif /* _H8300_IRQFLAGS_H */
