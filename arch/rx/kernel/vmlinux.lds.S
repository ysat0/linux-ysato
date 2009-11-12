#define VMLINUX_SYMBOL(_sym_) _##_sym_
#include <asm-generic/vmlinux.lds.h>
#include <asm/page.h>

#define EXCEPTION_FIXED 0xffffff80

#if defined(CONFIG_RX_SIM)
INPUT(romfs.o)
#endif

ENTRY(__start)

SECTIONS
{
        .text :
	{
	VMLINUX_SYMBOL(_text) = .;
	VMLINUX_SYMBOL(__stext) = . ;
	HEAD_TEXT
	TEXT_TEXT
	SCHED_TEXT
	LOCK_TEXT
	VMLINUX_SYMBOL(__etext) = . ;
	}
	EXCEPTION_TABLE(16)

	RODATA
	ROEND = .; 

#if defined(CONFIG_ROMKERNEL)
	VMLINUX_SYMBOL(__data_romtop) = .;
	. = RAMTOP;
	LOAD_OFFSET = RAMTOP - ROEND;
#else
	LOAD_OFFSET = 0
#endif
	VMLINUX_SYMBOL(_sdata) = .;
	RW_DATA_SECTION(L1_CACHE_BYTES, PAGE_SIZE, THREAD_SIZE)

	. = ALIGN(PAGE_SIZE);		/* Init code and data */
	INIT_TEXT_SECTION(PAGE_SIZE)
	INIT_DATA_SECTION(16)
	VMLINUX_SYMBOL(_edata) = .;
	BSS_SECTION(0, PAGE_SIZE, 4)
	VMLINUX_SYMBOL(_ebss) = .;		/* uClinux MTD sucks */
	VMLINUX_SYMBOL(_end) = . ;
        .romfs :	
	{
		*(.romfs*)
	}
	. = RAMTOP+RAMSIZE;
#if defined(CONFIG_ROMKERNEL)
	. = EXCEPTION_VECTOR;
	.vector :
	{
		*(.vector*)
	}
#endif
	DISCARDS
}
