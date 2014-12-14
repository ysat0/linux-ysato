#
# arch/h8300/Makefile
#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# (C) Copyright 2002,2003 Yoshinori Sato <ysato@users.sourceforge.jp>
#

cflags-$(CONFIG_CPU_H8300H)	:= -mh
ldflags-$(CONFIG_CPU_H8300H)	:= -mh8300helf_linux
cflags-$(CONFIG_CPU_H8S)	:= -ms
ldflags-$(CONFIG_CPU_H8S)	:= -mh8300self_linux

KBUILD_CFLAGS += $(cflags-y)
KBUILD_CFLAGS += -mint32 -fno-builtin
KBUILD_CFLAGS += -g
KBUILD_CFLAGS += -D__linux__
KBUILD_CFLAGS += -DUTS_SYSNAME=\"uClinux\"
KBUILD_AFLAGS += $(cflags-y)
LDFLAGS += $(ldflags-y)

CROSS_COMPILE := h8300-unknown-linux-
LIBGCC := $(shell $(CROSS-COMPILE)$(CC) $(KBUILD_CFLAGS) -print-libgcc-file-name)

core-y	+= arch/$(ARCH)/kernel/ arch/$(ARCH)/mm/ arch/$(ARCH)/boards/

libs-y	+= arch/$(ARCH)/lib/ $(LIBGCC)

boot := arch/h8300/boot

archmrproper:

archclean:
	$(Q)$(MAKE) $(clean)=$(boot)

vmlinux.srec vmlinux.bin zImage: vmlinux
	$(Q)$(MAKE) $(build)=$(boot) $(boot)/$@

define archhelp
  @echo  'vmlinux.bin  - Create raw binary'
  @echo  'vmlinux.srec - Create srec binary'
  @echo  'zImage       - Compressed kernel image'
endef
