#ifndef __H8300_TIMER_H
#define __H8300_TIMER_H

unsigned long get_cpu_clock(void);

#define TIMER_FREQ get_cpu_clock() /* Timer input freq. */

struct h8300_timer8_config {
	int rating;
};

struct h8300_timer16_config {
	int rating;
	__u8 enable;
	__u8 imfa;
	__u8 imiea;
};

#endif
