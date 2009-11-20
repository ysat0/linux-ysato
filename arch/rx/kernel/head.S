/*
 * arch/rx/kernel/head.S
 *  Copyright (C) 2009 Yoshinori Sato
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */
	

#include <linux/init.h>
#include <linux/linkage.h>
#include <asm/thread_info.h>
#include <asm/vmlinux.lds.h>

	.global	startup
	.section	.head.text,"ax"
startup:	
#if defined(CONFIG_ROMKERNEL) || !defined(CONFIG_RX_GDB_SIM)
	/* copy rw data for ram */
	mov.l	#SYMBOL_NAME(__data_romtop), r2
	mov.l	#SYMBOL_NAME(_sdata), r1
	mov.l	#SYMBOL_NAME(_edata), r3
	sub	r1, r3
	smovf
#endif

	/* clear BSS */
	mov.l	#SYMBOL_NAME(_sbss), r1
	mov.l	#SYMBOL_NAME(_ebss), r2
	sub	r1, r2
	shlr	#2, r2
	sub	r3,r3
1:
	mov.l	r3,[r1+]
	sub	#1, r2
	bne	1b
	/* start kernel */
	mov.l	#SYMBOL_NAME(init_thread_union), r1
	add	#PAGE_SIZE, r1, r0
	mov.l	#SYMBOL_NAME(start_kernel), r1
	jmp	r1

	.end
