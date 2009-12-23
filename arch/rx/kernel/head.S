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
#if !defined(CONFIG_RX_GDB_SIM)
#if defined(CONFIG_ROMKERNEL)
	mov.l	#CONFIG_INITIAL_STACK,r0
	mov.l	setup_bsc,r1
	jsr	r1

	/* copy rw data for ram */
	mov.l	__data_romtop,r2
	mov.l	_sdata,r1
	mov.l	_edata,r3
	sub	r1,r3
	smovf
#endif
#else
	/* setup args */
	mov.l	#12, r5
	int	#255
	mov.l	r1,r2
	mov.l	#0x400,r1
	mov.l	#13, r5
	int	#255
	mov.l	#0x400,r2
1:	
	movu.b	[r2+],r3
	tst	r3,r3
	bne	1b
	mov.l	#' ',r3
	mov.b	r3,[-r2]
	add	#1,r2
	sub	#1,r1
	bne	1b
	mov.l	#0,r3
	mov.b	r3,[-r2]
#endif
	/* clear BSS */
	mov.l	#_sbss,r1
	mov.l	#_ebss,r2
	sub	r1,r2
	shlr	#2,r2
	sub	r3,r3
1:
	mov.l	r3,[r1+]
	sub	#1, r2
	bne	1b
	/* start kernel */
	mov.l	#init_thread_union, r1
	add	#PAGE_SIZE, r1, r0
	mov.l	#start_kernel, r1
	jmp	r1

	.weak	setup_bsc
setup_bsc:	
	rts
	
	.end
