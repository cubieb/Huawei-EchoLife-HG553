/*
 * iptables module to match MAC address ranges
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/etherdevice.h>

#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <linux/netfilter/xt_mac_range.h>
#include <linux/netfilter/x_tables.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kevert  <Kevert@huawei.com>");
MODULE_DESCRIPTION("iptables arbitrary MAC range match module");

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

#if 0
#define DEBUGPC(str, args...)        \
      do                             \
      {                              \
          printk("[%s %s:%d]  ",__FILE__, __FUNCTION__, __LINE__); \
          printk("\033[1;33;40m");   \
          printk(str, ## args);      \
          printk("\033[0;37;40m\n"); \
      }while(0)
#else
#define DEBUGPC(str, args...)
#endif


static int
match(const struct sk_buff *skb,
      const struct net_device *in,
      const struct net_device *out,
      const struct xt_match *match,
      const void *matchinfo,
      int offset,
      unsigned int protoff,
      int *hotdrop)
{
	const struct xt_macrange_info *info = matchinfo;

    if ((skb->mac.raw < skb->head)
        || ((skb->mac.raw + ETH_HLEN) > skb->data))
    {
        //DEBUGPC("error, unknown thing......\r\n");
        return 0;
    }




    if (info->flags & XT_MACRANGE_SRC)
    {


        #if 0
                if (((memcmp(eth_hdr(skb)->h_source, info->src.min_mac, ETH_ALEN) < 0)
                    || (memcmp(eth_hdr(skb)->h_source, info->src.max_mac, ETH_ALEN) > 0))
                    ^ !!(info->flags & XT_MACRANGE_SRC_INV))
        #endif

         if ((memcmp(eth_hdr(skb)->h_source, info->src.min_mac, ETH_ALEN) < 0)
            || (memcmp(eth_hdr(skb)->h_source, info->src.max_mac, ETH_ALEN) > 0))
        {
            return 0;
        }

        /*
        return ((skb->mac.raw >= skb->head)
	     && ((skb->mac.raw + ETH_HLEN) <= skb->data)
	        // If so, compare...
	     && (((memcmp(skb->mac.ethernet->h_source, info->src.min_mac, ETH_ALEN)
	    	>= 0)&&(memcmp(skb->mac.ethernet->h_source, info->src.max_mac, ETH_ALEN) <= 0)) ^ (!!!(info->flags & MACRANGE_SRC_INV))));
         */
    }

    if (info->flags & XT_MACRANGE_DST)
    {

        #if 0
                if (((memcmp(eth_hdr(skb)->h_dest, info->dst.min_mac, ETH_ALEN) < 0)
                    || (memcmp(eth_hdr(skb)->h_dest, info->dst.max_mac, ETH_ALEN) > 0))
                    ^ !!(info->flags & XT_MACRANGE_DST_INV))
        #endif
        if ((memcmp(eth_hdr(skb)->h_dest, info->dst.min_mac, ETH_ALEN) < 0)
            || (memcmp(eth_hdr(skb)->h_dest, info->dst.max_mac, ETH_ALEN) > 0))
        {
            return 0;
        }

    }

#if 0
    printk("src: %02x:%02x:%02x:%02x:%02x:%02x dst: %02x:%02x:%02x:%02x:%02x:%02x matched\r\n",
        skb->mac.ethernet->h_source[0], skb->mac.ethernet->h_source[1],
        skb->mac.ethernet->h_source[2], skb->mac.ethernet->h_source[3],
        skb->mac.ethernet->h_source[4], skb->mac.ethernet->h_source[5],
        skb->mac.ethernet->h_dest[0], skb->mac.ethernet->h_dest[1],
        skb->mac.ethernet->h_dest[2], skb->mac.ethernet->h_dest[3],
        skb->mac.ethernet->h_dest[4], skb->mac.ethernet->h_dest[5]
        );
#endif

    return 1;

}

#if 0
static int check(const char *tablename,
		 const struct ipt_ip *ip,
		 void *matchinfo,
		 unsigned int matchsize,
		 unsigned int hook_mask)
{
	/* verify size */
	if (matchsize != IPT_ALIGN(sizeof(struct ipt_macrange_info)))
		return 0;

	return 1;
}
#endif

static struct xt_match xt_macrange_match[] =
{
	{
		.name = "macrange",
		.family = AF_INET,
		.match = &match,
		.matchsize = sizeof(struct xt_macrange_info),
		.hooks = (1 << NF_IP_PRE_ROUTING) |
				  (1 << NF_IP_LOCAL_IN) |
				  (1 << NF_IP_FORWARD) |
				  (1 << NF_IP_POST_ROUTING) |
				  (1 << NF_IP_LOCAL_OUT),
		.me = THIS_MODULE,
	},

	{
		.name = "macrange",
		.family = AF_INET6,
		.match = &match,
		.matchsize = sizeof(struct xt_macrange_info),
		.hooks = (1 << NF_IP6_PRE_ROUTING) |
				  (1 << NF_IP6_LOCAL_IN) |
				  (1 << NF_IP6_FORWARD) |
				  (1 << NF_IP6_LOCAL_OUT) |
				  (1 << NF_IP6_POST_ROUTING),
		.me = THIS_MODULE,
	},
};

static int __init xt_macrange_init(void)
{
	return xt_register_matches(xt_macrange_match, ARRAY_SIZE(xt_macrange_match));
}

static void __exit xt_macrange_fini(void)
{
	xt_unregister_matches(xt_macrange_match, ARRAY_SIZE(xt_macrange_match));
}

module_init(xt_macrange_init);
module_exit(xt_macrange_fini);
