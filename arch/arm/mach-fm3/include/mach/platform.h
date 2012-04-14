/*
 * arch/arm/mach-fm3/include/mach/platform.h
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

#ifndef __ASM_ARCH_PLATFORM_H
#define __ASM_ARCH_PLATFORM_H

/*
 * FM3 peripheral addresses
 */
#define FM3_WATCHDOG_BASE	0x40012000
#define FM3_DUAL_TIMER1		0x40015000
#define FM3_DUAL_TIMER2		0x40015020
#define FM3_UART0_BASE		0x40038000
#define FM3_UART1_BASE		0x40038100
#define FM3_UART2_BASE		0x40038200
#define FM3_UART3_BASE		0x40038300
#define FM3_UART4_BASE		0x40038400
#define FM3_UART5_BASE		0x40038500
#define FM3_UART6_BASE		0x40038600
#define FM3_UART7_BASE		0x40038700
#define FM3_RTC_BASE		0x4003b000

#define FM3_SCM_CTL		0x40010000
#define FM3_BSC_PSR		0x40010010
#define FM3_APBC0_PSR		0x40010014
#define FM3_APBC1_PSR		0x40010018
#define FM3_APBC2_PSR		0x4001001c
#define FM3_SWC_PSR		0x40010020
#define FM3_TTC_PSR		0x40010028
#define FM3_PLL_CTL1		0x40010038
#define FM3_PLL_CTL2		0x4001003c

extern int fm3_clock_init(unsigned int mo_hz, unsigned int so_hz);

#endif	/* __ASM_ARCH_PLATFORM_H */
