/*
 *  linux/arch/arm/mm/cache-v7m.S
 *
 *  Copyright (C) 2012 Yoshinori Sato
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  This is the "shell" of the ARMv7m processor support.
 */
#include <linux/linkage.h>
#include <linux/init.h>
#include <asm/assembler.h>

ENTRY(v7m_flush_dcache_all)
	mov	pc, lr
ENDPROC(v7m_flush_dcache_all)

ENTRY(v7m_flush_kern_cache_all)
	mov	pc, lr
ENDPROC(v7m_flush_kern_cache_all)

ENTRY(v7m_flush_user_cache_all)
ENTRY(v7m_flush_user_cache_range)
	mov	pc, lr
ENDPROC(v7m_flush_user_cache_all)
ENDPROC(v7m_flush_user_cache_range)

ENTRY(v7m_coherent_kern_range)
ENTRY(v7m_coherent_user_range)
	mov	pc, lr
ENDPROC(v7m_coherent_kern_range)
ENDPROC(v7m_coherent_user_range)

ENTRY(v7m_flush_kern_dcache_area)
	mov	pc, lr
ENDPROC(v7m_flush_kern_dcache_area)

ENTRY(v7m_dma_map_area)
ENTRY(v7m_dma_unmap_area)	
ENTRY(v7m_dma_flush_range)
	mov	pc, lr
ENDPROC(v7m_dma_map_area)
ENDPROC(v7m_dma_flush_range)
ENDPROC(v7m_dma_unmap_area)

	__INITDATA

	.type	v7m_cache_fns, #object
ENTRY(v7m_cache_fns)
	.long	v7m_flush_kern_cache_all
	.long	v7m_flush_user_cache_all
	.long	v7m_flush_user_cache_range
	.long	v7m_coherent_kern_range
	.long	v7m_coherent_user_range
	.long	v7m_flush_kern_dcache_area
	.long	v7m_dma_map_area
	.long	v7m_dma_unmap_area
	.long	v7m_dma_flush_range
	.size	v7m_cache_fns, . - v7m_cache_fns
