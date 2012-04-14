/*
 * arch/arm/mach-fm3/include/mach/uncompress.h
 *
 * Copyright (C) 2012 Yoshinori Sato
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include <mach/hardware.h>
#include <asm/mach-types.h>

#include <mach/platform.h>

#define FM3_UART_BASE FM3_UART0_BASE
#define FM3_UART_SSR *(volatile unsigned char *)(FM3_UART_BASE +  0x05)
#define FM3_UART_TDR *(volatile unsigned short *)(FM3_UART_BASE +  0x08)

/*
 * This does not append a newline
 */
static inline void putc(int c)
{
	while ((FM3_UART_SSR & (1 << 1)) == 0)
		barrier();

	FM3_UART_TDR = c;
}

static inline void flush(void)
{

	while ((FM3_UART_SSR & (1 << 0)) == 0)
		barrier();
}

/*
 * nothing to do
 */
#define arch_decomp_setup()
#define arch_decomp_wdog()
