#ifndef __ASM_RX_MODULE_H__
#define __ASM_RX_MODULE_H__
/*
 * This file contains the H8/300 architecture specific module code.
 */
struct mod_arch_specific { };
#define Elf_Shdr Elf32_Shdr
#define Elf_Sym Elf32_Sym
#define Elf_Ehdr Elf32_Ehdr

#define MODULE_SYMBOL_PREFIX "_"

#endif /* _ASM_H8/300_MODULE_H */
