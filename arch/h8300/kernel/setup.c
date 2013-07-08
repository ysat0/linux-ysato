/*
 *  linux/arch/h8300/kernel/setup.c
 *
 *  Copyleft  ()) 2000       James D. Schettine {james@telos-systems.com}
 *  Copyright (C) 1999,2000  Greg Ungerer (gerg@snapgear.com)
 *  Copyright (C) 1998,1999  D. Jeff Dionne <jeff@lineo.ca>
 *  Copyright (C) 1998       Kenneth Albanowski <kjahds@kjahds.com>
 *  Copyright (C) 1995       Hamish Macdonald
 *  Copyright (C) 2000       Lineo Inc. (www.lineo.com) 
 *  Copyright (C) 2001 	     Lineo, Inc. <www.lineo.com>
 *
 *  H8/300 porting Yoshinori Sato <ysato@users.sourceforge.jp>
 */

/*
 * This file handles the architecture-dependent parts of system setup
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/fb.h>
#include <linux/console.h>
#include <linux/genhd.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/major.h>
#include <linux/bootmem.h>
#include <linux/seq_file.h>
#include <linux/init.h>

#include <asm/setup.h>
#include <asm/irq.h>
#include <asm/pgtable.h>

#if defined(__H8300H__)
#define CPU "H8/300H"
#include <asm/regs306x.h>
#endif

#if defined(__H8300S__)
#define CPU "H8S"
#include <asm/regs267x.h>
#endif

#define STUBSIZE 0xc000

unsigned long rom_length;
unsigned long memory_start;
unsigned long memory_end;

char __initdata command_line[COMMAND_LINE_SIZE];

extern int _stext, _etext, _sdata, _edata, _sbss, _ebss, _end;
extern unsigned long _ramstart, _ramend;
extern char _target_name[];
extern char command_start[];
void h8300_early_devices_register(void);

void __init setup_arch(char **cmdline_p)
{
	int bootmap_size;

	memory_start = (unsigned long) &_ramstart;

	/* allow for ROMFS on the end of the kernel */
	if (memcmp((void *)memory_start, "-rom1fs-", 8) == 0) {
#if defined(CONFIG_BLK_DEV_INITRD)
		initrd_start = memory_start;
		initrd_end = memory_start += be32_to_cpu(((unsigned long *) (memory_start))[2]);
#else
		memory_start += be32_to_cpu(((unsigned long *) memory_start)[2]);
#endif
	}
	memory_start = PAGE_ALIGN(memory_start);
#if !defined(CONFIG_BLKDEV_RESERVE)
	memory_end = (unsigned long) &_ramend; /* by now the stack is part of the init task */
#if defined(CONFIG_GDB_DEBUG)
	memory_end -= STUBSIZE;
#endif
#else
	if ((memory_end < CONFIG_BLKDEV_RESERVE_ADDRESS) && 
	    (memory_end > CONFIG_BLKDEV_RESERVE_ADDRESS))
	    /* overlap userarea */
	    memory_end = CONFIG_BLKDEV_RESERVE_ADDRESS; 
#endif

	init_mm.start_code = (unsigned long) &_stext;
	init_mm.end_code = (unsigned long) &_etext;
	init_mm.end_data = (unsigned long) &_edata;
	init_mm.brk = (unsigned long) 0; 

	printk(KERN_INFO "\r\n\nuClinux " CPU "\n");
	printk(KERN_INFO "Target Hardware: %s\n",_target_name);
	printk(KERN_INFO "Flat model support (C) 1998,1999 Kenneth Albanowski, D. Jeff Dionne\n");
	printk(KERN_INFO "H8/300 series support by Yoshinori Sato <ysato@users.sourceforge.jp>\n");

#ifdef DEBUG
	printk(KERN_DEBUG "KERNEL -> TEXT=0x%06x-0x%06x DATA=0x%06x-0x%06x "
		"BSS=0x%06x-0x%06x\n", (int) &_stext, (int) &_etext,
		(int) &_sdata, (int) &_edata,
		(int) &_sbss, (int) &_ebss);
	printk(KERN_DEBUG "KERNEL -> ROMFS=0x%06x-0x%06x MEM=0x%06x-0x%06x "
		"STACK=0x%06x-0x%06x\n",
	       (int) &_ebss, (int) memory_start,
		(int) memory_start, (int) memory_end,
		(int) memory_end, (int) &_ramend);
#endif

#ifdef CONFIG_DEFAULT_CMDLINE
	/* set from default command line */
	if (*command_start == '\0')
		memcpy(command_line, CONFIG_KERNEL_COMMAND, sizeof(command_line));
#endif
	if (*command_line == '\0')
		memcpy(command_line, command_start, sizeof(command_line));
	*cmdline_p = command_line;
	memcpy(boot_command_line, command_line, COMMAND_LINE_SIZE);
	boot_command_line[COMMAND_LINE_SIZE-1] = 0;

#ifdef DEBUG
	if (strlen(*cmdline_p)) 
		printk(KERN_DEBUG "Command line: '%s'\n", *cmdline_p);
#endif

	/*
	 * give all the memory to the bootmap allocator,  tell it to put the
	 * boot mem_map at the start of memory
	 */
	bootmap_size = init_bootmem_node(
			NODE_DATA(0),
			memory_start >> PAGE_SHIFT, /* map goes here */
			PAGE_OFFSET >> PAGE_SHIFT,	/* 0 on coldfire */
			memory_end >> PAGE_SHIFT);
	/*
	 * free the usable memory,  we have to make sure we do not free
	 * the bootmem bitmap so we then reserve it after freeing it :-)
	 */
	free_bootmem(memory_start, memory_end - memory_start);
	reserve_bootmem(memory_start, bootmap_size, BOOTMEM_DEFAULT);
	/*
	 * get kmalloc into gear
	 */
	paging_init();
#ifdef DEBUG
	printk(KERN_DEBUG "Done setup_arch\n");
#endif
}

/*
 *	Get CPU information for use by the procfs.
 */

static int show_cpuinfo(struct seq_file *m, void *v)
{
    char *cpu;
    int mode;
    u_long clockfreq;

    cpu = CPU;
    mode = *(volatile unsigned char *)MDCR & 0x07;

    clockfreq = CONFIG_CPU_CLOCK;

    seq_printf(m,  "CPU:\t\t%s (mode:%d)\n"
		   "Clock:\t\t%lu.%1luMHz\n"
		   "BogoMips:\t%lu.%02lu\n"
		   "Calibration:\t%lu loops\n",
	           cpu,mode,
		   clockfreq/1000,clockfreq%1000,
		   (loops_per_jiffy*HZ)/500000,((loops_per_jiffy*HZ)/5000)%100,
		   (loops_per_jiffy*HZ));

    return 0;
}

static void *c_start(struct seq_file *m, loff_t *pos)
{
	return *pos < NR_CPUS ? ((void *) 0x12345678) : NULL;
}

static void *c_next(struct seq_file *m, void *v, loff_t *pos)
{
	++*pos;
	return c_start(m, pos);
}

static void c_stop(struct seq_file *m, void *v)
{
}

const struct seq_operations cpuinfo_op = {
	.start	= c_start,
	.next	= c_next,
	.stop	= c_stop,
	.show	= show_cpuinfo,
};
