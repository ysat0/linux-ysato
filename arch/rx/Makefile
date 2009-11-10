#
# arch/rx/Makefile
#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# (C) Copyright 2009 Yoshinori Sato <ysato@users.sourceforge.jp>
#

KBUILD_CFLAGS += $(cflags-y)
KBUILD_CFLAGS += -g
KBUILD_CFLAGS += -D__linux__
KBUILD_CFLAGS += -DUTS_SYSNAME=\"uClinux\"
KBUILD_AFLAGS += -DPLATFORM=$(PLATFORM) -DMODEL=$(MODEL) $(cflags-y)
LDFLAGS += $(ldflags-y)

LIBGCC := $(shell $(CROSS-COMPILE)$(CC) $(KBUILD_CFLAGS) -print-libgcc-file-name)

head-y := arch/$(ARCH)/platform/$(PLATFORM)/$(BOARD)/crt0_$(MODEL).o

core-y	+= arch/$(ARCH)/kernel/ \
	   arch/$(ARCH)/mm/
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
