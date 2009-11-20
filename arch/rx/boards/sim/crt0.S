#include <linux/linkage.h>

	.global	__start
	.section	.head.text,"ax"
__start:
	mov.l	#0x1000,r0	; initial stack
	bra	startup

	.section	.vector,"a"
no	=	0
	.rept	32
	.long	SYMBOL_NAME(rx_exp_table) + (no * 4)
no	=	no + 1	
	.endr

	.end
