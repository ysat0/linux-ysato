#ifndef _H8300_PAGE_H
#define _H8300_PAGE_H

#include <asm-generic/page.h>
#include <linux/types.h>

#define MAP_NR(addr) (((uintptr_t)(addr)-PAGE_OFFSET) >> PAGE_SHIFT)

#endif
