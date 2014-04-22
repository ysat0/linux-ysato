#include <asm-generic/vmlinux.lds.h>
#include <asm/page.h>

/* target memory map */
#ifdef CONFIG_H8300H_GENERIC
#define ROMTOP  0x000000
#define ROMSIZE 0x400000
#define RAMTOP  0x400000
#define RAMSIZE 0x400000
#endif

#ifdef CONFIG_H8300H_AKI3068NET
#define ROMTOP  0x000000
#define ROMSIZE 0x080000
#define RAMTOP  0x400000
#define RAMSIZE 0x200000
#endif

#ifdef CONFIG_H8300H_H8MAX
#define ROMTOP  0x000000
#define ROMSIZE 0x080000
#define RAMTOP  0x400000
#define RAMSIZE 0x200000
#endif

#ifdef CONFIG_H8300H_SIM
#define ROMTOP  0x000000
#define ROMSIZE 0x100000
#define RAMTOP  0x100000
#define RAMSIZE 0x200000
#endif

#ifdef CONFIG_H8S_SIM
#define ROMTOP  0x000000
#define ROMSIZE 0x400000
#define RAMTOP  0x400000
#define RAMSIZE 0x800000
#endif

#ifdef CONFIG_H8S_EDOSK2674
#define ROMTOP  0x000000
#define ROMSIZE 0x400000
#define RAMTOP  0x400000
#define RAMSIZE 0x800000
#endif

#if defined(CONFIG_H8300H_SIM) || defined(CONFIG_H8S_SIM)
/*INPUT(romfs.o)*/
#endif

jiffies = jiffies_64 + 4;

ENTRY(_start)

SECTIONS
{
#if defined(CONFIG_ROMKERNEL)
	. = ROMTOP; 
	.vectors :
	{
	_vector = . ;
		*(.vector*)
	}
#else
	. = RAMTOP; 
	.bootvec :	
	{
		*(.bootvec)
	}
#endif
        .text :
	{
	_text = .;
#if defined(CONFIG_ROMKERNEL)
	*(.int_redirect)
#endif
	_stext = . ;
	TEXT_TEXT
	SCHED_TEXT
	LOCK_TEXT
	_etext = . ;
	}
	EXCEPTION_TABLE(16)
	NOTES
	RODATA
#if defined(CONFIG_ROMKERNEL)
	SECURITY_INIT
#endif
	ROEND = .; 
#if defined(CONFIG_ROMKERNEL)
	. = RAMTOP;
	.data :	AT(ROEND)
#else
	.data :	
#endif
	{
	_sdata = . ;
	__data_start = . ;

	INIT_TASK_DATA(0x2000)
	. = ALIGN(0x4) ;
		DATA_DATA
	. = ALIGN(0x4) ;
		*(.data.*)	

	. = ALIGN(0x4) ;
	__init_begin = .;
	_sinittext = .;
		INIT_TEXT
	_einittext = .;
		INIT_DATA
	INIT_SETUP(0x4)
	INIT_CALLS
	CON_INITCALL
		EXIT_TEXT
		EXIT_DATA
	INIT_RAM_FS
	. = ALIGN(0x4) ;
	__init_end = .;
	_edata = . ;
	}
#if defined(CONFIG_RAMKERNEL)
	SECURITY_INIT
#endif
	_begin_data = LOADADDR(.data);
        .bss : 
        {
	. = ALIGN(0x4) ;
	__sbss = . ;
	__bss_start = . ;
		*(.bss*)
	. = ALIGN(0x4) ;
		*(COMMON)
	. = ALIGN(0x4) ;
	__bss_stop = . ;
	_ebss = . ;
	_end = . ;
	_ramstart = .;
	}
        .romfs :	
	{
		*(.romfs*)
	}
	. = RAMTOP+RAMSIZE;
        .dummy :
        {
	command_start = . - 0x200 ;
	_ramend = . ;
	}

	DISCARDS
}
