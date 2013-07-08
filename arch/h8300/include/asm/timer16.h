#ifndef _ASM_H8300_TIMER16_H_
#define _ASM_H8300_TIMER16_H_

struct timer16ch {
	__u32 base;
	__u8 irqs[3];
	__u8 enable;
	__u8 imf;
	__u8 imie;
};

struct h8300_timer16_data {
	__u32 base;
	int num;
	struct timer16ch ch[];
};

#endif
