#ifndef __ASM_RX_LINKAGE_H__
#define __ASM_RX_LINKAGE_H__

#undef SYMBOL_NAME_LABEL
#undef SYMBOL_NAME
#define SYMBOL_NAME_LABEL(_name_) _##_name_##:
#define SYMBOL_NAME(_name_) _##_name_
#endif
