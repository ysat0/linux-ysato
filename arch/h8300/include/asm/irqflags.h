#ifndef _H8300_IRQFLAGS_H
#define _H8300_IRQFLAGS_H

#ifdef CONFIG_CPU_H8300H
static inline unsigned long arch_local_save_flags(void)
{
	unsigned long flags;
	asm volatile ("stc ccr,%w0" : "=r" (flags));
	return flags;
}

static inline void arch_local_irq_disable(void)
{
	asm volatile ("orc  #0x80,ccr" : : : "memory");
}

static inline void arch_local_irq_enable(void)
{
	asm volatile ("andc #0x7f,ccr" : : : "memory");
}

static inline unsigned long arch_local_irq_save(void)
{
	unsigned long flags = arch_local_save_flags();
	arch_local_irq_disable();
	return flags;
}

static inline void arch_local_irq_restore(unsigned long flags)
{
	asm volatile ("ldc %w0,ccr" : : "r" (flags) : "memory");
}
#endif
#ifdef CONFIG_CPU_H8S
static inline unsigned long arch_local_save_flags(void)
{
	unsigned long flags;
	asm volatile ("stc ccr,%w0\n\tstc exr,%x0" : "=r" (flags));
	return flags;
}

static inline void arch_local_irq_disable(void)
{
	asm volatile ("orc  #0x80,ccr\n\torc #0x07,exr" : : : "memory");
}

static inline void arch_local_irq_enable(void)
{
	asm volatile ("andc #0x7f,ccr\n\tandc #0xf0,exr\n\t": : : "memory");
}

static inline unsigned long arch_local_irq_save(void)
{
	unsigned long flags = arch_local_save_flags();
	arch_local_irq_disable();
	return flags;
}

static inline void arch_local_irq_restore(unsigned long flags)
{
	asm volatile ("ldc %w0,ccr\n\tldc %x0,exr" : : "r" (flags) : "memory");
}
#endif

static inline int arch_irqs_disabled_flags(unsigned long flags)
{
	return (flags & 0x80) == 0x80;
}

static inline int arch_irqs_disabled(void)
{
	return arch_irqs_disabled_flags(arch_local_save_flags());
}

#endif /* _H8300_IRQFLAGS_H */
