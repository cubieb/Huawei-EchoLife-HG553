/*
 *	Forwarding decision
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_forward.c,v 1.1 2010/07/06 06:45:11 c65985 Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/netfilter_bridge.h>
#include "br_private.h"
#if defined(CONFIG_MIPS_BRCM)
#include <linux/blog.h>
#endif

#ifdef CONFIG_ATP_LAN_VLAN
#include "br_vlan.h"
#endif

 /*start of LAN 广播包不能回流 by l39225 2006-7-15*/
static inline int should_eth_deliver(const struct net_bridge_port *p, 
				 const struct sk_buff *skb)
{
     unsigned char *dest;
     dest = NULL;
	
    dest = eth_hdr(skb)->h_dest;

    if (dest != NULL)
    {        
        if ( (dest[0] == 0xff) && (dest[1] ==0xff) && (dest[2] == 0xff)
              &&(dest[3] == 0xff) && (dest[4] ==0xff) && (dest[5] == 0xff)
            )
       {
            return  1;
       }

    }
    
    if (  (*(p->dev->name) == 'e')  &&  (*(p->dev->name+1) == 't')
           && (*(p->dev->name+2) == 'h')  && (*(skb->dev->name) == 'e')
           && (*(skb->dev->name+1) == 't') &&  (*(skb->dev->name+2) == 'h')
           && (*(skb->dev->name +3) == *(p->dev->name+3)) )
      {
            return 0;
      }
    
     return 1;
}
    /*end of LAN 广播包不能回流 by l39225 2006-7-15*/
/* Don't forward packets to originating port or forwarding diasabled */
static inline int should_deliver(const struct net_bridge_port *p,
				 const struct sk_buff *skb)
{
 	if (skb->dev == p->dev ||
	    p->state != BR_STATE_FORWARDING)
		return 0;
      /*start of  修改wan 接口回流给其他wan接口问题 by s53329 at  20071105
	if ((skb->nfmark & FROM_WAN)  &&
	    ((*(p->dev->name)  == 'n') &&
	    (*(p->dev->name + 1) == 'a') &&
	    (*(p->dev->name + 2) == 's')))
	*/
	if ((*(p->dev->name)    == 'n') &&
	    (*(skb->dev->name)  == 'n')) 
	  /*  end of  修改wan 接口回流给其他wan接口问题 by s53329 at  20071105  */
		return 0;

	return 1;
}

static inline unsigned packet_length(const struct sk_buff *skb)
{
	return skb->len - (skb->protocol == htons(ETH_P_8021Q) ? VLAN_HLEN : 0);
}

int br_dev_queue_push_xmit(struct sk_buff *skb)
{
#if defined(CONFIG_MIPS_BRCM_DISABLED)
	// Just to make it consistent with 2.4 so it will not surprise the customers.(Should be more intelligent.)
#ifdef CONFIG_BRIDGE_NETFILTER
	/* ip_refrag calls ip_fragment, doesn't copy the MAC header. */
	nf_bridge_maybe_copy_header(skb);
#endif
	skb_push(skb, ETH_HLEN);

    L2PT_LOG(skb, skb->dev, ETH_802x, HDR_INSERT, DIR_TX,
             0, ETH_HLEN, skb->data);   /* CONFIG_L2PT */

	dev_queue_xmit(skb);
#else
	/* drop mtu oversized packets except gso */
	if (packet_length(skb) > skb->dev->mtu && !skb_is_gso(skb))
		kfree_skb(skb);
	else {
		/* ip_refrag calls ip_fragment, doesn't copy the MAC header. */
		if (nf_bridge_maybe_copy_header(skb))
			kfree_skb(skb);
		else 
		{
			skb_push(skb, ETH_HLEN);

#ifdef CONFIG_ATP_LAN_VLAN
            int lRet = 0;
            lRet = br_vlan_xmit_in(skb);
            
            if (lRet)
            {
                if (NULL != skb)
                {
                    kfree_skb(skb);
                }
                return 0;
            }
            else
#endif
            {
    			dev_queue_xmit(skb);
            }
		}
	}
#endif

	return 0;
}

int br_forward_finish(struct sk_buff *skb)
{
    /* start 64 byte can not reach 15M in vdsl A36D06384 by f00110348 20090331 */
    if (sysctl_fast_routing)
    {
        return br_dev_queue_push_xmit(skb);
    }
    else
    {
    	return NF_HOOK(PF_BRIDGE, NF_BR_POST_ROUTING, skb, NULL, skb->dev,
    		       br_dev_queue_push_xmit);
    }
    /* end 64 byte can not reach 15M in vdsl A36D06384 by f00110348 20090331 */

}

static void __br_deliver(const struct net_bridge_port *to, struct sk_buff *skb)
{
	skb->dev = to->dev;
	NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_OUT, skb, NULL, skb->dev,
			br_forward_finish);
}

static void __br_forward(const struct net_bridge_port *to, struct sk_buff *skb)
{
	struct net_device *indev;

	indev = skb->dev;
	skb->dev = to->dev;
	skb->ip_summed = CHECKSUM_NONE;

#ifdef CONFIG_ATP_QOS_NETFILTER
    /* if qos enable, all packet to IMQ */
    skb->mark |= QOS_DEFAULT_MARK;
#endif
	NF_HOOK(PF_BRIDGE, NF_BR_FORWARD, skb, indev, skb->dev,
			br_forward_finish);
}

/* called with rcu_read_lock */
void br_deliver(const struct net_bridge_port *to, struct sk_buff *skb)
{
	if (should_deliver(to, skb)) {
		__br_deliver(to, skb);
		return;
	}

	kfree_skb(skb);
}

/* called with rcu_read_lock */
void br_forward(const struct net_bridge_port *to, struct sk_buff *skb)
{
	if (should_deliver(to, skb)) {
		__br_forward(to, skb);
		return;
	}

	kfree_skb(skb);
}

#ifdef CONFIG_ATP_BRIDGE_BIND
/*Start of ATP 2008-11-26 for AU4D01075 by c47036: 绑定的LAN从绑定的WAN走，不绑定的LAN从任意的WAN走 */
/* 桥绑定判断，pgroup为接收报文的端口所在的组，name为接收报文的端口的名字，p为待转发的端口 */
static inline int bind_check(struct net_bridge *br, const struct bridgebind_group *pgroup, 
    const char* name, const struct net_bridge_port *p)
{
    int ret = 0;    
    struct bridgebind_group *pdstgroup = NULL;
    
    /* 待转发的端口所在的组 */
    pdstgroup = br_get_group(br, brgetgroupbyname(br, p->dev->name));

    /* 同一个组里的端口可以互相转发，默认组里的LAN口可以向任意WAN口转发，任意WAN口可以向默认组里的LAN口转发 */
    if ((ISIN_GROUP(pgroup->ports , p->port_no))
        || ((name[0] == 'e' || name[0] == 'w') && (pgroup->groupid == 0))
        || ((name[0] == 'n') && (NULL != pdstgroup) && (pdstgroup->groupid == 0))
        || (name[0] != 'n' && p->dev->name[0] != 'n'))
    {
        ret = 1;
    }

    if (NULL != pdstgroup)
    {
        br_release_group(pdstgroup);
    }
    
    return ret;    
}
/*End of ATP 2008-11-26 for AU4D01075 by c47036 */  
#endif

/* called under bridge lock */
static void br_flood(struct net_bridge *br, struct sk_buff *skb, int clone,
	void (*__packet_hook)(const struct net_bridge_port *p,
			      struct sk_buff *skb))
{
    /*start of 桥组播不开snooping时两pc不能同时组播by l129990,2009-10-14*/
    unsigned char *dest;
    dest = NULL;
    dest = eth_hdr(skb)->h_dest;
    /*end of 桥组播不开snooping时两pc不能同时组播by l129990,2009-10-14*/
    
	struct net_bridge_port *p;
	struct net_bridge_port *prev;

/*start of 桥组播不开snooping时两pc不能同时组播by l129990,2009-10-14*/
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
	if ( (skb->blog_p && !skb->blog_p->rx.info.multicast)||(is_multicast_ether_addr(dest)))
    {   
		blog_skip(skb);
    }
#endif
/*end of 桥组播不开snooping时两pc不能同时组播by l129990,2009-10-14*/
	if (clone) {
		struct sk_buff *skb2;

		if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
			br->statistics.tx_dropped++;
			return;
		}

		skb = skb2;
	}

	prev = NULL;

/*start of ATP bridge bind, z37589, 20070814*/
#ifdef CONFIG_ATP_BRIDGE_BIND
    //if(br->group_enable && (skb->protocol != __constant_htons(ETH_P_ARP)) && !(skb->data[21] == 0x43 && skb->data[23] == 0x44)) //DHCP、ARP包不考虑组在整个桥内转发
    if(br->group_enable && !(skb->data[21] == 0x43 && skb->data[23] == 0x44)) //只有DHCP不考虑组在整个桥内转发
    {
        struct bridgebind_group *pgroup = br_get_group(br, brgetgroup(br, skb));

		if(NULL != pgroup)
	    {
    		list_for_each_entry_rcu(p, &br->port_list, list) {
            /*Start of ATP 2008-11-26 for AU4D01075 by c47036: 绑定的LAN从绑定的WAN走，不绑定的LAN从任意的WAN走 */    
    		//if (should_deliver(p, skb) && should_eth_deliver(p,skb) && (ISIN_GROUP(pgroup->ports , p->port_no))) { 
    		if (should_deliver(p, skb) && should_eth_deliver(p,skb) && (ISIN_GROUP(pgroup->ports , p->port_no))) { 
            /*End of ATP 2008-11-26 for AU4D01075 by c47036 */      
    			if (prev != NULL)
                {
    				struct sk_buff *skb2;

    				if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
    					br->statistics.tx_dropped++;
    					kfree_skb(skb);
    					br_release_group(pgroup);
    					return;
    				}
                    /* start A36D07088 two pc multicast by f00110348 20090724 */
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
    				blog_clone(skb, skb2->blog_p);
#endif
                    /* end A36D07088 two pc multicast by f00110348 20090724 */
    				__packet_hook(prev, skb2);
    			}

    			prev = p;
    		}
        	}
    		br_release_group(pgroup);
        }
    }
    else
#endif
/*end of ATP bridge bind, z37589, 20070814*/

	list_for_each_entry_rcu(p, &br->port_list, list) {
		if (should_deliver(p, skb) &&  should_eth_deliver(p,skb)) {
			if (prev != NULL) {
				struct sk_buff *skb2;

				if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
					br->statistics.tx_dropped++;
					kfree_skb(skb);
					return;
				}
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
				blog_clone(skb, skb2->blog_p);
#endif
				__packet_hook(prev, skb2);
			}

			prev = p;
		}
	}

	if (prev != NULL) {
		__packet_hook(prev, skb);
		return;
	}

	kfree_skb(skb);
}

#ifdef CONFIG_SUPPORT_ATP
/*start of ATP 2008.01.11 防止组播数据在br下nas口回流 add by h00101845*/
#define ATP_IPPROTO_UDP    17
static void br_deliver_nonas(const struct net_bridge_port * to, struct sk_buff * skb)
{
   /*
    *解决br0下存在nas接口三层组播(IGMP Proxy)出现马赛克问题
    *原因: 三层udp数据报文向二层转发广播进入nas接口, 数据流处理不及时
    *将进入桥下nas口的数据包直接丢弃不做任何处理
    *
    */
    const unsigned char *dest = eth_hdr(skb)->h_dest;
   
    if( skb->data[9] == ATP_IPPROTO_UDP &&
        dest[0] == 0x01 && 
        dest[1] == 0x00 &&
        dest[2] == 0x5e &&
        *(to->dev->name) == 'n' && *(to->dev->name + 1) == 'a' &&
        *(to->dev->name + 2) == 's')
    {
        kfree_skb(skb);
        return ;
    }
    __br_deliver(to,skb);
}

/* called with rcu_read_lock */
void br_flood_deliver(struct net_bridge *br, struct sk_buff *skb, int clone)
{
	br_flood(br, skb, clone, br_deliver_nonas);
}
#else
/* called with rcu_read_lock */
void br_flood_deliver(struct net_bridge *br, struct sk_buff *skb, int clone)
{
	br_flood(br, skb, clone, __br_deliver);
}
#endif
/*end of ATP 2008.01.11 防止组播数据在br下nas口回流 add by h00101845*/

/* called under bridge lock */
void br_flood_forward(struct net_bridge *br, struct sk_buff *skb, int clone)
{
	br_flood(br, skb, clone, __br_forward);
}
