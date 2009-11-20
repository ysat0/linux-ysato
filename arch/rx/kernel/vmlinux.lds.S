#define	LOAD_OFFSET 0
#define VMLINUX_SYMBOL(_sym_) _##_sym_
#include <asm-generic/vmlinux.lds.h>
#include <asm/page.h>
#include <asm/thread_info.h>

#define EXCEPTION_FIXED 0xffffff80

RAMTOP = 0x01000000;
RAMSIZE = 0x00100000;
VMLINUX_SYMBOL(ram_exp_vector) = 0x00000100;

#if defined(CONFIG_RX_GDB_SIM)
/*INPUT(romfs.o)*/
#endif

VMLINUX_SYMBOL(jiffies) = VMLINUX_SYMBOL(jiffies_64);

ENTRY(__start)

SECTIONS
{
        .text :
	{
	VMLINUX_SYMBOL(_text) = .;
	VMLINUX_SYMBOL(_stext) = . ;
	HEAD_TEXT
	TEXT_TEXT
	SCHED_TEXT
	LOCK_TEXT
#if defined(CONFIG_ROMKERNEL)
	INIT_TEXT_SECTION(PAGE_SIZE)
#endif
	VMLINUX_SYMBOL(_etext) = . ;
	}
	EXCEPTION_TABLE(16)

	RODATA
	ROEND = .; 

#if defined(CONFIG_ROMKERNEL)
	VMLINUX_SYMBOL(__data_romtop) = .;
	. = RAMTOP;
#undef LOAD_OFFSET
#define	LOAD_OFFSET (RAMTOP - ROEND)
#endif
	VMLINUX_SYMBOL(_ramstart) = .;
	VMLINUX_SYMBOL(_sdata) = .;
	RW_DATA_SECTION(0, PAGE_SIZE, THREAD_SIZE)

	. = ALIGN(PAGE_SIZE);		/* Init code and data */
#if defined(CONFIG_RAMKERNEL)
	VMLINUX_SYMBOL(__init_begin) = .;
	INIT_TEXT_SECTION(PAGE_SIZE)
#else
	VMLINUX_SYMBOL(__init_begin) = .;
#endif
	INIT_DATA_SECTION(16)
	VMLINUX_SYMBOL(__init_end) = .;
	VMLINUX_SYMBOL(_edata) = .;
	VMLINUX_SYMBOL(_sbss) = .;
	BSS_SECTION(0, PAGE_SIZE, 4)
	VMLINUX_SYMBOL(_ebss) = .;		/* uClinux MTD sucks */
	VMLINUX_SYMBOL(_end) = . ;
        .romfs :	
	{
		*(.romfs*)
	}
	. = RAMTOP + RAMSIZE;
	VMLINUX_SYMBOL(_ramend) = .;
#if defined(CONFIG_ROMKERNEL)
	. = EXCEPTION_VECTOR;
	.vector :
	{
		*(.vector*)
	}
#endif
	DISCARDS
}
