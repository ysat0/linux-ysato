#
# Makefile for the linux kernel.
#

extra-y := vmlinux.lds

obj-y := process.o traps.o ptrace.o irq.o \
	 sys_h8300.o time.o signal.o \
         setup.o gpio.o syscalls.o \
	 entry.o timer/

obj-$(CONFIG_MODULES) += module.o h8300_ksyms.o 
obj-$(CONFIG_EARLY_PRINTK) += early_printk.o

obj-$(CONFIG_H83007) += cpu/h83069/	# have same peripherals
obj-$(CONFIG_H83069) += cpu/h83069/
