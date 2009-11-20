/*
 * Automatically generated C config: don't edit
 * Linux kernel version: 2.6.32-rc6
 * Thu Nov 19 17:15:32 2009
 */
/*
 * Helper macros to support writing architecture specific
 * linker scripts.
 *
 * A minimal linker scripts has following content:
 * [This is a sample, architectures may have special requiriements]
 *
 * OUTPUT_FORMAT(...)
 * OUTPUT_ARCH(...)
 * ENTRY(...)
 * SECTIONS
 * {
 *	. = START;
 *	__init_begin = .;
 *	HEAD_TEXT_SECTION
 *	INIT_TEXT_SECTION(PAGE_SIZE)
 *	INIT_DATA_SECTION(...)
 *	PERCPU(PAGE_SIZE)
 *	__init_end = .;
 *
 *	_stext = .;
 *	TEXT_SECTION = 0
 *	_etext = .;
 *
 *      _sdata = .;
 *	RO_DATA_SECTION(PAGE_SIZE)
 *	RW_DATA_SECTION(...)
 *	_edata = .;
 *
 *	EXCEPTION_TABLE(...)
 *	NOTES
 *
 *	BSS_SECTION(0, 0, 0)
 *	_end = .;
 *
 *	STABS_DEBUG
 *	DWARF_DEBUG
 *
 *	DISCARDS		// must be the last
 * }
 *
 * [__init_begin, __init_end] is the init section that may be freed after init
 * [_stext, _etext] is the text section
 * [_sdata, _edata] is the data section
 *
 * Some of the included output section have their own set of constants.
 * Examples are: [__initramfs_start, __initramfs_end] for initramfs and
 *               [__nosave_begin, __nosave_end] for the nosave data
 */




/* Align . to a 8 byte boundary equals to maximum function alignment. */


/* The actual configuration determine if the init/exit sections
 * are handled as text/data or they can be discarded (which
 * often happens at runtime)
 */
/* .data section */
/*
 * Data section helpers
 */
/*
 * Read only Data
 */
/* RODATA & RO_DATA provided for backward compatibility.
 * All archs are supposed to use RO_DATA() */
/* .text section. Map to function alignment to avoid address changes
 * during second ld run in second ld pass when generating System.map */
/* sched.text is aling to function alignment to secure we have same
 * address even at second ld pass when generating System.map */
/* spinlock.text is aling to function alignment to secure we have same
 * address even at second ld pass when generating System.map */
/* Section used for early init (in .S files) */
/*
 * Exception table
 */
/*
 * Init task
 */
/* init and exit section handling */
/*
 * bss (Block Started by Symbol) - uninitialized data
 * zeroed during startup
 */
/*
 * DWARF debug sections.
 * Symbols in the DWARF debugging sections are relative to
 * the beginning of the section so we begin them at 0.
 */
  /* Stabs debugging sections.  */
/*
 * Default discarded sections.
 *
 * Some archs want to discard exit text/data at runtime rather than
 * link time due to cross-section references such as alt instructions,
 * bug table, eh_frame, etc.  DISCARDS must be the last of output
 * section definitions so that such archs put those in earlier section
 * definitions.
 */
/**
 * PERCPU_VADDR - define output section for percpu area
 * @vaddr: explicit base address (optional)
 * @phdr: destination PHDR (optional)
 *
 * Macro which expands to output section for percpu area.  If @vaddr
 * is not blank, it specifies explicit base address and all percpu
 * symbols will be offset from the given address.  If blank, @vaddr
 * always equals @laddr + LOAD_OFFSET.
 *
 * @phdr defines the output PHDR to use if not blank.  Be warned that
 * output PHDR is sticky.  If @phdr is specified, the next output
 * section in the linker script will go there too.  @phdr should have
 * a leading colon.
 *
 * Note that this macros defines __per_cpu_load as an absolute symbol.
 * If there is no need to put the percpu section at a predetermined
 * address, use PERCPU().
 */
/**
 * PERCPU - define output section for percpu area, simple version
 * @align: required alignment
 *
 * Align to @align and outputs output section for percpu area.  This
 * macro doesn't maniuplate @vaddr or @phdr and __per_cpu_load and
 * __per_cpu_start will be identical.
 *
 * This macro is equivalent to ALIGN(align); PERCPU_VADDR( , ) except
 * that __per_cpu_load is defined as a relative symbol against
 * .data.percpu which is required for relocatable x86_32
 * configuration.
 */
/*
 * Definition of the high level *_SECTION macros
 * They will fit only a subset of the architectures
 */
/*
 * Writeable data.
 * All sections are combined in a single .data section.
 * The sections following CONSTRUCTORS are arranged so their
 * typical alignment matches.
 * A cacheline is typical/always less than a PAGE_SIZE so
 * the sections that has this restriction (or similar)
 * is located before the ones requiring PAGE_SIZE alignment.
 * NOSAVE_DATA starts and ends with a PAGE_SIZE alignment which
 * matches the requirment of PAGE_ALIGNED_DATA.
 *
 * use 0 as page_align if page_aligned data is not used */
/*
 * Generic page.h implementation, for NOMMU architectures.
 * This provides the dummy definitions for the memory management.
 */
/* PAGE_SHIFT determines the page size */
/*
 * Size of kernel stack for each process. This must be a power of 2...
 */
/*
 * Offsets in thread_info structure, used in assembly code
 */
/*
 * thread information flag bit numbers
 */
/* as above, but as bit values */
/* work to do in syscall trace */
/* work to do on any return to u-space */
/* work to do on interrupt/exception return */
RAMTOP = 0x01000000;
RAMSIZE = 0x00100000;
_ram_exp_vector = 0x00000100;
/*INPUT(romfs.o)*/
_jiffies = _jiffies_64;
ENTRY(__start)
SECTIONS
{
        .text :
 {
 __text = .;
 __stext = . ;
 *(.head.text)
 . = ALIGN(8); *(.text.hot) *(.text) *(.ref.text) *(.text.unlikely)
 . = ALIGN(8); ___sched_text_start = .; *(.sched.text) ___sched_text_end = .;
 . = ALIGN(8); ___lock_text_start = .; *(.spinlock.text) ___lock_text_end = .;
 __etext = . ;
 }
 . = ALIGN(16); __ex_table : AT(ADDR(__ex_table) - 0) { ___start___ex_table = .; *(__ex_table) ___stop___ex_table = .; }
 . = ALIGN((4096)); .rodata : AT(ADDR(.rodata) - 0) { ___start_rodata = .; *(.rodata) *(.rodata.*) *(__vermagic) *(__markers_strings) *(__tracepoints_strings) } .rodata1 : AT(ADDR(.rodata1) - 0) { *(.rodata1) } .pci_fixup : AT(ADDR(.pci_fixup) - 0) { ___start_pci_fixups_early = .; *(.pci_fixup_early) ___end_pci_fixups_early = .; ___start_pci_fixups_header = .; *(.pci_fixup_header) ___end_pci_fixups_header = .; ___start_pci_fixups_final = .; *(.pci_fixup_final) ___end_pci_fixups_final = .; ___start_pci_fixups_enable = .; *(.pci_fixup_enable) ___end_pci_fixups_enable = .; ___start_pci_fixups_resume = .; *(.pci_fixup_resume) ___end_pci_fixups_resume = .; ___start_pci_fixups_resume_early = .; *(.pci_fixup_resume_early) ___end_pci_fixups_resume_early = .; ___start_pci_fixups_suspend = .; *(.pci_fixup_suspend) ___end_pci_fixups_suspend = .; } .builtin_fw : AT(ADDR(.builtin_fw) - 0) { ___start_builtin_fw = .; *(.builtin_fw) ___end_builtin_fw = .; } .rio_route : AT(ADDR(.rio_route) - 0) { ___start_rio_route_ops = .; *(.rio_route_ops) ___end_rio_route_ops = .; } __ksymtab : AT(ADDR(__ksymtab) - 0) { ___start___ksymtab = .; *(__ksymtab) ___stop___ksymtab = .; } __ksymtab_gpl : AT(ADDR(__ksymtab_gpl) - 0) { ___start___ksymtab_gpl = .; *(__ksymtab_gpl) ___stop___ksymtab_gpl = .; } __ksymtab_unused : AT(ADDR(__ksymtab_unused) - 0) { ___start___ksymtab_unused = .; *(__ksymtab_unused) ___stop___ksymtab_unused = .; } __ksymtab_unused_gpl : AT(ADDR(__ksymtab_unused_gpl) - 0) { ___start___ksymtab_unused_gpl = .; *(__ksymtab_unused_gpl) ___stop___ksymtab_unused_gpl = .; } __ksymtab_gpl_future : AT(ADDR(__ksymtab_gpl_future) - 0) { ___start___ksymtab_gpl_future = .; *(__ksymtab_gpl_future) ___stop___ksymtab_gpl_future = .; } __kcrctab : AT(ADDR(__kcrctab) - 0) { ___start___kcrctab = .; *(__kcrctab) ___stop___kcrctab = .; } __kcrctab_gpl : AT(ADDR(__kcrctab_gpl) - 0) { ___start___kcrctab_gpl = .; *(__kcrctab_gpl) ___stop___kcrctab_gpl = .; } __kcrctab_unused : AT(ADDR(__kcrctab_unused) - 0) { ___start___kcrctab_unused = .; *(__kcrctab_unused) ___stop___kcrctab_unused = .; } __kcrctab_unused_gpl : AT(ADDR(__kcrctab_unused_gpl) - 0) { ___start___kcrctab_unused_gpl = .; *(__kcrctab_unused_gpl) ___stop___kcrctab_unused_gpl = .; } __kcrctab_gpl_future : AT(ADDR(__kcrctab_gpl_future) - 0) { ___start___kcrctab_gpl_future = .; *(__kcrctab_gpl_future) ___stop___kcrctab_gpl_future = .; } __ksymtab_strings : AT(ADDR(__ksymtab_strings) - 0) { *(__ksymtab_strings) } __init_rodata : AT(ADDR(__init_rodata) - 0) { *(.ref.rodata) } __param : AT(ADDR(__param) - 0) { ___start___param = .; *(__param) ___stop___param = .; . = ALIGN((4096)); ___end_rodata = .; } . = ALIGN((4096));
 ROEND = .;
 __ramstart = .;
 __sdata = .;
 . = ALIGN((1 << 12)); .data : AT(ADDR(.data) - 0) { . = ALIGN(8192); *(.data.init_task) . = ALIGN((1 << 12)); ___nosave_begin = .; *(.data.nosave) . = ALIGN((1 << 12)); ___nosave_end = .; . = ALIGN((1 << 12)); *(.data.page_aligned) . = ALIGN(0); *(.data.cacheline_aligned) . = ALIGN(0); *(.data.read_mostly) *(.data) *(.ref.data) . = ALIGN(8); ___start___markers = .; *(__markers) ___stop___markers = .; . = ALIGN(32); ___start___tracepoints = .; *(__tracepoints) ___stop___tracepoints = .; . = ALIGN(8); ___start___verbose = .; *(__verbose) ___stop___verbose = .; CONSTRUCTORS }
 . = ALIGN((1 << 12)); /* Init code and data */
 ___init_begin = .;
 . = ALIGN((1 << 12)); .init.text : AT(ADDR(.init.text) - 0) { __sinittext = .; *(.init.text) *(.devinit.text) *(.cpuinit.text) *(.meminit.text) __einittext = .; }
 .init.data : AT(ADDR(.init.data) - 0) { *(.init.data) *(.devinit.data) *(.cpuinit.data) *(.meminit.data) . = ALIGN(8); ___ctors_start = .; *(.ctors) ___ctors_end = .; *(.init.rodata) *(.devinit.rodata) *(.cpuinit.rodata) *(.meminit.rodata) . = ALIGN(16); ___setup_start = .; *(.init.setup) ___setup_end = .; ___initcall_start = .; *(.initcallearly.init) ___early_initcall_end = .; *(.initcall0.init) *(.initcall0s.init) *(.initcall1.init) *(.initcall1s.init) *(.initcall2.init) *(.initcall2s.init) *(.initcall3.init) *(.initcall3s.init) *(.initcall4.init) *(.initcall4s.init) *(.initcall5.init) *(.initcall5s.init) *(.initcallrootfs.init) *(.initcall6.init) *(.initcall6s.init) *(.initcall7.init) *(.initcall7s.init) ___initcall_end = .; ___con_initcall_start = .; *(.con_initcall.init) ___con_initcall_end = .; ___security_initcall_start = .; *(.security_initcall.init) ___security_initcall_end = .; }
 ___init_end = .;
 __edata = .;
 __sbss = .;
 . = ALIGN(0); ___bss_start = .; . = ALIGN(0); .sbss : AT(ADDR(.sbss) - 0) { *(.sbss) *(.scommon) } . = ALIGN((1 << 12)); .bss : AT(ADDR(.bss) - 0) { *(.bss.page_aligned) *(.dynbss) *(.bss) *(COMMON) } . = ALIGN(4); ___bss_stop = .;
 __ebss = .; /* uClinux MTD sucks */
 __end = . ;
        .romfs :
 {
  *(.romfs*)
 }
 . = RAMTOP + RAMSIZE;
 __ramend = .;
 /DISCARD/ : { *(.exit.text) *(.devexit.text) *(.cpuexit.text) *(.memexit.text) *(.exit.data) *(.devexit.data) *(.devexit.rodata) *(.cpuexit.data) *(.cpuexit.rodata) *(.memexit.data) *(.memexit.rodata) *(.exitcall.exit) *(.discard) }
}
