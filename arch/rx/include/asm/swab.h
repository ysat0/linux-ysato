#ifndef __ASM_RX_SWAB_H__
#define __ASM_RX_SWAB_H__

#include <linux/types.h>

#if defined(__GNUC__) && !defined(__STRICT_ANSI__) || defined(__KERNEL__)
#  define __SWAB_64_THRU_32__
#endif

#endif /* __ASM_RX_SWAB_H__ */
