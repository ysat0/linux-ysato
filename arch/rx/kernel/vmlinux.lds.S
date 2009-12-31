#define	LOAD_OFFSET 0
#include <asm-generic/vmlinux.lds.h>
#include <asm/page.h>
#include <asm/thread_info.h>

#define EXCEPTION_FIXED 0xffffff80

ROMTOP = 0xff000000;
RAMTOP = 0x01000000;
RAMSIZE = 0x01000000;
ram_exp_vector = 0x00000000;

#if defined(CONFIG_RX_GDB_SIM)
/*INPUT(romfs.o)*/
#endif

jiffies = jiffies_64;

ENTRY(__start)

SECTIONS
{
	. = ROMTOP;
        .text :
	{
	_text = .;
	_stext = . ;
	HEAD_TEXT
	TEXT_TEXT
	SCHED_TEXT
	LOCK_TEXT
	_etext = . ;
	}
	EXCEPTION_TABLE(16)
#if defined(CONFIG_ROMKERNEL)
	INIT_TEXT_SECTION(PAGE_SIZE)
#endif

	RODATA
	ROEND = .; 

#if defined(CONFIG_ROMKERNEL)
	__data_romtop = .;
#endif
	. = RAMTOP;
	_sdata = .;
	RW_DATA_SECTION(0, PAGE_SIZE, THREAD_SIZE)

	. = ALIGN(PAGE_SIZE);		/* Init code and data */
#if defined(CONFIG_RAMKERNEL)
	__init_begin = .;
	INIT_TEXT_SECTION(PAGE_SIZE)
#else
	__init_begin = .;
#endif
	INIT_DATA_SECTION(16)
	__init_end = .;
	_edata = .;
	_sbss = .;
	BSS_SECTION(0, PAGE_SIZE, 4)
	_ebss = .;		/* uClinux MTD sucks */
	_end = . ;
	_ramstart = .;
        .romfs :	
	{
		*(.romfs*)
	}
	. = RAMTOP + RAMSIZE;
	_ramend = .;
#if defined(CONFIG_ROMKERNEL)
	. = EXCEPTION_FIXED;
	.vector :
	{
		*(.vector*)
	}
#endif
	DISCARDS
}
