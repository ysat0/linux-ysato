#ifndef __ASM_RX_PARAM_H__
#define __ASM_RX_PARAM_H__

#ifdef __KERNEL__
#define HZ		CONFIG_HZ
#define	USER_HZ		HZ
#define	CLOCKS_PER_SEC	(USER_HZ)
#else
#define HZ		100
#endif

#define EXEC_PAGESIZE	4096

#ifndef NOGROUP
#define NOGROUP		(-1)
#endif

#define MAXHOSTNAMELEN	64	/* max length of hostname */

#endif /* __ASM_RX_PARAM_H__ */
