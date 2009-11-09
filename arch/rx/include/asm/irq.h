#ifndef __ASM_RX_IRQ_H__
#define __ASM_RX_IRQ_H__

#define NR_IRQS 256

static __inline__ int irq_canonicalize(int irq)
{
	return irq;
}

#endif /* __ASM_RX_IRQ_H__ */
