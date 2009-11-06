#ifndef __ASM_RX_UNALIGNED_H__
#define __ASM_RX_UNALIGNED_H__

#include <linux/unaligned/be_memmove.h>
#include <linux/unaligned/le_byteshift.h>
#include <linux/unaligned/generic.h>

#define get_unaligned	__get_unaligned_be
#define put_unaligned	__put_unaligned_be

#endif /* __ASM_RX_UNALIGNED_H__ */
