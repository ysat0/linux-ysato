/*
 *  linux/arch/h8300/board/h8300h_sim/target.S
 *
 *  Yoshinori Sato <ysato@users.sourceforge.jp>
 *
 *  Platform depend startup
 *  Target: GDB simulator
 */

#define ASSEMBLY

#include <asm/linkage.h>

	.global lowlevel_init
	.global _platform_gpio_table
	.global _target_name
	
	.h8300s
lowlevel_init:
	rts
	
	/* I/O port assign information */
_platform_gpio_table:	
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
	;; PCDDR
	.byte	0x00,0x00
	;; PDDDR
	.byte	0x00,0x00
	;; PEDDR
	.byte	0x00,0x00
	;; PFDDR
	.byte	0x00,0x00
	;; PGDDR
	.byte	0x00,0x00

#if defined(CONFIG_ROMKERNEL)
	/* interrupt vector */
	.section .vectors,"ax"
	.long	_start
	.long	_start
vector	=	2
	.rept	128-2
	.long	_interrupt_redirect_table+vector*4
vector	=	vector + 1
	.endr
#endif
