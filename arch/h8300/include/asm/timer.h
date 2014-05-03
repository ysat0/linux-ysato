#ifndef __H8300_TIMER_H
#define __H8300_TIMER_H

unsigned long get_cpu_clock(void);

#define TIMER_FREQ get_cpu_clock() /* Timer input freq. */

#define MODE_CS 0
#define MODE_CED 1

#define DIV_8 0
#define DIV_64 1
#define DIC_8192 2

struct h8300_timer8_config {
	int mode;
	int div;
	int rating;
};

struct h8300_timer16_config {
	int rating;
	__u8 enable;
	__u8 imfa;
	__u8 imiea;
};

struct h8300_tpu_config {
	int rating;
};
#endif
