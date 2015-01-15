#
# Makefile for the linux kernel.
#

extra-y := vmlinux.lds

obj-y := process.o traps.o ptrace.o \
	 sys_h8300.o time.o signal.o \
         setup.o syscalls.o irq.o \
	 entry.o clk.o timer/

obj-$(CONFIG_ROMKERNEL) += head_rom.o
obj-$(CONFIG_RAMKERNEL) += head_ram.o
obj-$(CONFIG_CPU_H8300H) += ptrace_h.o irq_h.o
obj-$(CONFIG_CPU_H8S) += ptrace_s.o irq_s.o

obj-$(CONFIG_MODULES) += module.o h8300_ksyms.o
obj-$(CONFIG_H8300H_SIM) += sim-console.o
obj-$(CONFIG_H8S_SIM) += sim-console.o

obj-$(CONFIG_H83069) += cpu/h83069/
obj-$(CONFIG_H8S2678) += cpu/h8s2678/

