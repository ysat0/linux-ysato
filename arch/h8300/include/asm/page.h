#ifndef _H8300_PAGE_H
#define _H8300_PAGE_H
#include <asm-generic/page.h>
#define MAP_NR(addr)		(((unsigned long)(addr)-PAGE_OFFSET) >> PAGE_SHIFT)
#endif
