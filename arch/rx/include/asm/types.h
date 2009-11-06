#ifndef __ASM_RX_TYPES_H__
#define __ASM_RX_TYPES_H__

#include <asm-generic/int-ll64.h>

#if !defined(__ASSEMBLY__)

/*
 * This file is never included by application software unless
 * explicitly requested (e.g., via linux/types.h) in which case the
 * application is Linux specific so (user-) name space pollution is
 * not a major issue.  However, for interoperability, libraries still
 * need to be careful to avoid a name clashes.
 */

typedef unsigned short umode_t;

/*
 * These aren't exported outside the kernel to avoid name space clashes
 */
#ifdef __KERNEL__

#define BITS_PER_LONG 32

/* Dma addresses are 32-bits wide.  */

typedef u32 dma_addr_t;

#endif /* __KERNEL__ */

#endif /* __ASSEMBLY__ */

#endif /* __ASM_RX_TYPES_H__ */
