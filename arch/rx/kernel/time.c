#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/profile.h>
#include <linux/timex.h>
#include <linux/sched.h>
#include <linux/clockchips.h>
#include <linux/platform_device.h>
#include <linux/rtc.h>
#include <asm/clock.h>

void rx_clk_init(void);

/* common tick interrupt handler */
void rx_timer_tick(void)
{
	if (current->pid)
		profile_tick(CPU_PROFILING);
	write_seqlock(&xtime_lock);
	do_timer(1);
	write_sequnlock(&xtime_lock);
	update_process_times(user_mode(get_irq_regs()));
}

/* dummy RTC access */
void read_persistent_clock(struct timespec *tv)
{
	tv->tv_sec = mktime(2000, 1, 1, 0, 0, 0);
	tv->tv_nsec = 0;
}

int update_persistent_clock(struct timespec now)
{
	return 0;
}

void __init time_init(void)
{
	rx_clk_init();
	clk_init();

	set_normalized_timespec(&wall_to_monotonic,
				-xtime.tv_sec, -xtime.tv_nsec);
}
