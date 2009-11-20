/*
 * arch/rx/kernel/entry.S
 *
 * The RX exception entry
 *
 * Copyright (C) 2009 Yoshinori Sato
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/linkage.h>
#include <asm/asm-offsets.h>
#include <asm/thread_info.h>
#include <asm/unistd.h>
#include <asm/errno.h>
#include <asm/page.h>

#define	OFF_R1	(1*4)
#define	OFF_R2	(2*4)
#define	OFF_R3	(3*4)
#define	OFF_R4	(4*4)
#define	OFF_R5	(5*4)
#define	OFF_R7	(7*4)
#define OFF_USP (16*4)
#define OFF_VEC (17*4)
#define OFF_PSW (19*4)

#if !defined(CONFIG_PREEMPT)
#define resume_kernel restore_all
#endif

#define MIN_IRQ 20
#define MAX_EXCEPTION (32 + 20)
	
.global	SYMBOL_NAME(rx_int_table), SYMBOL_NAME(rx_exp_table)
.global SYMBOL_NAME(ret_from_fork)
	
	.section	.text
	.align	4
SYMBOL_NAME_LABEL(rx_int_table)
	.rept	256
	bsr.a	interrupt_entry
	.endr
SYMBOL_NAME_LABEL(rx_exp_table)
	.rept	32
	bsr.a	exception_entry
	.endr

interrupt_entry:
	sub	#4,r0
	pushm	r1-r15
	mov.l	r0,r1
	add	#-(19*4),r1
	mov.l	r1,[-r0]
	mvfc	usp,r1
	mov.l	r1,OFF_USP[r0]
	mov.l	OFF_VEC[r0],r1
	mov.l	#SYMBOL_NAME(rx_int_table),r2
	sub	r2,r1
	shlr	#2,r1
	sub	#1,r1
	mov.l	r1,OFF_VEC[r0]
	cmp	#MIN_IRQ,r1
	ble	1f
	bsr	SYMBOL_NAME(do_IRQ)
	bra	ret_from_interrupt
1:
	add	#32,r1
	bra	exception_handler

exception_entry:
	sub	#4,r0
	pushm	r1-r15
	mov.l	r0,r15
	add	#-(19*4),r15
	mov.l	r15,[-r0]
	mvfc	usp,r15
	mov.l	r15,OFF_USP[r0]
	mov.l	OFF_VEC[r0],r15
	mov.l	#SYMBOL_NAME(rx_exp_table),r14
	sub	r14,r15
	shlr	#2,r15
	sub	#1,r15
exception_handler:
	cmp	#MAX_EXCEPTION,r15
	bge	1f
	add	#0x1000,r15,r14
	mov.l	r14,OFF_VEC[r0]
	shll	#2,r15
	add	#SYMBOL_NAME(exception_table),r15
	mov.l	[r15],r15
	mov.l	r0,r1
	bsr	r15
	bra	ret_from_exception
1:
	mov.l	r0,r1
	mov.l	r15,r2
	bsr	SYMBOL_NAME(unhandled_exception)
ret_from_exception:	
#if defined(CONFIG_PREEMPT)
	clrpsw	i
#endif
ret_from_interrupt:	
	mov.l	OFF_PSW[r0],r1
	btst	#20,r1
	beq	resume_kernel
resume_userspace:
	clrpsw	i
	mov.l	#~(THREAD_SIZE - 1),r1
	and	r0, r1
	mov.l	TI_FLAGS[r1],r2
	tst	#_TIF_WORK_MASK,r2
	beq	restore_all
work_pending:
	btst	#TIF_NEED_RESCHED,r2
	bne	work_resched
	mov.l	r0,r1			; pt_regs
	mov	r8,r3
	bsr	SYMBOL_NAME(do_notify_resume)	; do_notify_resume(pt_regs *, ti_flags, saved_r1)
	bra	resume_userspace
work_resched:
	bsr	SYMBOL_NAME(schedule)
	clrpsw	i
	mov	TI_FLAGS[r1],r2
	tst	#_TIF_WORK_MASK,r2
	bne	work_pending
restore_all:
	mov.l	OFF_USP[r0],r1
	mvtc	r1,usp
	add	#4,r0
	popm	r1-r15
	add	#8,r0
	rte
#if defined(CONFIG_PREEMPT)
resume_kernel:
	clrpsw	i
	mov.l	#~(THREAD_SIZE - 1),r1
	and	r0,r1
	mov.l	TI_PRECOUNT[r1],r2
	tst	r2,r2
	bne	restore_all
1:	
	mov.l	TI_FLAGS[r1],r2
	btst	#TIF_NEED_RESCHED,r2
	beq	restore_all
	mov.l	OFF_PSW[r0],r2
	btst	#16,r2		; I flag check
	beq	restore_all	; intterupt disable (exception cause)
	bsr	SYMBOL_NAME(preempt_schedule_irq)
	bra	1b
#endif
syscall_entry:
	setpsw	i
	mov.l	#~(THREAD_SIZE - 1),r15
	and	r0,r15
	mov.l	TI_FLAGS[r15],r14
	tst	#_TIF_WORK_SYSCALL_MASK,r14
	bnz	4f
1:	
	cmp	#NR_syscalls,r8
	blt	2f
	mov.l	#-ENOSYS,r1	; invalid no
	bra	3f
2:	
	shll	#2,r8
	add	#SYMBOL_NAME(syscall_table),r8
	mov.l	r7,[-r0]
	mov.l	r5,[-r0]
	jsr	r8
	add	#8,r0
3:	
	mov.l	OFF_R1[r0],r8	; r8 = saved r1
	mov.l	r1,OFF_R1[r0]
	clrpsw	i
	mov.l	#~(THREAD_SIZE - 1),r15
	and	r0,r15
	mov.l	TI_FLAGS[r15],r14
	tst	#_TIF_ALLWORK_MASK,r14
	bne	5f
	bra	restore_all
4:
	;; syscall trace enter
	mov.l	r0,r1
	bsr	SYMBOL_NAME(syscall_trace_enter) ; syscall_trace_enter(pt_regs *)
	mov.l	OFF_R1[r0],r1
	mov.l	OFF_R2[r0],r2
	mov.l	OFF_R3[r0],r3
	mov.l	OFF_R4[r0],r4
	mov.l	OFF_R5[r0],r5
	bra	1b
5:
	;; syscall trace leave
	btst	#TIF_SYSCALL_TRACE,r14
	beq	work_pending
	setpsw	i
	mov.l	r0,r1
	bsr	SYMBOL_NAME(syscall_trace_leave) ; syscall_trace_leave(pt_regs *)
	bra	resume_userspace

SYMBOL_NAME_LABEL(ret_from_fork)
	bsr	SYMBOL_NAME(schedule_tail)
	bra	ret_from_exception

	.end
