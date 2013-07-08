/*
 *  linux/arch/h8300/platform/h8300h/generic/crt0_rom.S
 *
 *  Yoshinori Sato <ysato@users.sourceforge.jp>
 *
 *  Platform depend startup
 *  Target Archtecture:	generic
 *  Memory Layout     :	ROM
 */

#define ASSEMBLY

#include <asm/linkage.h>
#include <asm/regs306x.h>
	
	.global __start
	.global __command_line
	.global __platform_gpio_table
	.global __target_name
	
	.h8300h
	.section .head.text,"ax"
	.file	"crt0_rom.S"

	/* CPU Reset entry */
__start:
	mov.l	#__ramend,sp
	ldc	#0xc0,ccr
	mov.l	#SYSCR,er0
	bclr	#3,@er0
	
	/* Peripheral Setup */
	
#if !defined(CONFIG_H8300H_SIM)
	/* .bss clear */
	mov.l	#__sbss,er5
	mov.l	#__ebss,er4
	sub.l	er5,er4
	shlr	er4
	shlr	er4
	sub.l	er0,er0
1:	
	mov.l	er0,@er5
	adds	#4,er5
	dec.l	#1,er4
	bne	1b

	/* copy .data */
	mov.l	#__begin_data,er5
	mov.l	#__sdata,er6
	mov.l	#__edata,er4
	sub.l	er6,er4
	shlr.l	er4
	shlr.l	er4
1:	
	mov.l	@er5+,er0
	mov.l	er0,@er6
	adds	#4,er6
	dec.l	#1,er4
	bne	1b	

#else
	/* get cmdline from gdb */
	jsr	@0xcc
	;; er0 - argc
	;; er1 - argv
	mov.l	#_command_start,er3
	adds	#4,er1
	dec.l	#1,er0
	beq	4f
1:
	mov.l	@er1+,er2
2:	
	mov.b	@er2+,r4l
	beq	3f
	mov.b	r4l,@er3
	adds	#1,er3
	bra	2b
3:
	mov.b	#' ',r4l
	mov.b	r4l,@er3
	adds	#1,er3
	dec.l	#1,er0
	bne	1b
	subs	#1,er3
	mov.b	#0,r4l
	mov.b	r4l,@er3
4:	
#endif

	/* linux kernel start */
	ldc	#0xd0,ccr	/* running kernel */
	mov.l	#_init_thread_union,sp
	add.l	#0x2000,sp
	jsr	@_start_kernel
_exit:

	jmp	_exit

	rts

	/* I/O port assign information */
__platform_gpio_table:	
	mov.l	#gpio_table,er0
	rts

gpio_table:
	;; P1DDR
	.byte	0x00,0x00
	;; P2DDR
	.byte	0x00,0x00
	;; P3DDR
	.byte	0x00,0x00
	;; P4DDR
	.byte	0x00,0x00
	;; P5DDR
	.byte	0x00,0x00
	;; P6DDR
	.byte	0x00,0x00
	;; dummy
	.byte	0x00,0x00
	;; P8DDR
	.byte	0x00,0x00
	;; P9DDR
	.byte	0x00,0x00
	;; PADDR
	.byte	0x00,0x00
	;; PBDDR
	.byte	0x00,0x00

	.section .rodata
__target_name:	
	.asciz	"generic"
	
	/* interrupt vector */
	.section .vectors,"ax"
	.long	__start
vector	=	1
	.rept	64-1
	.long	_interrupt_redirect_table+vector*4
vector	=	vector + 1
	.endr
