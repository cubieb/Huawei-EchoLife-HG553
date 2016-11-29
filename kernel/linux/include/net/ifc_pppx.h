
#ifndef _IFC_PPPX_H_
#define _IFC_PPPX_H_
#include <linux/skbuff.h>
#include <linux/timer.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <asm/atomic.h>
#include <linux/if.h>
#include <linux/netdevice.h>

#define PPPX_MAC_TRUE    (1 == 1)
#define PPPX_MAC_FALSE   (1 == 0)

struct pppx_macmap
{
	char *name;
	unsigned char rmt_mac[ETH_ALEN];/* ppp trace */
};
struct pppx_ppptrace //for user space
{
 	char lan_name[IFNAMSIZ];
 	unsigned char rmt_mac[ETH_ALEN];
};


extern int pppx_get_lanname( void **trace_info, int *len);
extern void pppx_bindmac(struct sk_buff *skb, int ifc);

#endif
