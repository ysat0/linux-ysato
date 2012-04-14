/*
 * arch/arm/mach-fm3/include/mach/irqs.h
 *
 * Copyright (C) 2012 Yoshinori Sato
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __ASM_ARCH_IRQS_H
#define __ASM_ARCH_IRQS_H

/*
 * FM3 interrupt sources
 */
#define IRQ_FM3_WDOG		1	/* Watchdog timer */
#define IRQ_FM3_RTC		24	/* Real Time Clock */
#define IRQ_FM3_DUAL_TIMER	6	/* Dual Timer */

#define IRQ_FM3_UART0_RX	7
#define IRQ_FM3_UART0_TX	8
#define IRQ_FM3_UART1_RX	9
#define IRQ_FM3_UART1_TX	10
#define IRQ_FM3_UART2_RX	11
#define IRQ_FM3_UART2_TX	12
#define IRQ_FM3_UART3_RX	13
#define IRQ_FM3_UART3_TX	14
#define IRQ_FM3_UART4_RX	15
#define IRQ_FM3_UART4_TX	16
#define IRQ_FM3_UART5_RX	17
#define IRQ_FM3_UART5_TX	18
#define IRQ_FM3_UART6_RX	19
#define IRQ_FM3_UART6_TX	20
#define IRQ_FM3_UART7_RX	21
#define IRQ_FM3_UART7_TX	22

#define NR_IRQS			32

#endif
