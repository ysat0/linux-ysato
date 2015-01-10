#ifndef __H8300_CLK_H
#define __H8300_CLK_H

struct clk {
	struct clk		*parent;
	unsigned long		rate;
	unsigned long		flags;
};

#endif
