
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/times.h>
#include <linux/if_vlan.h>
#include <asm/atomic.h>
#include <linux/igmp.h>
#include "br_private.h"
#include "br_igmp_snooping.h"

#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif
struct net_bridge_igmp_snooping igmp_snooping_list;

static unsigned char upnp_mac[6] = {0x01, 0x00, 0x5e, 0x7f, 0xff, 0xfa};

/* start of AU4D01311 by d00107688 网关开启igmp snooping功能后，动态路由：br0的rip v2功能出现异常 */
static unsigned char ripv2_mac[6] = {0x01, 0x00, 0x5e, 0x00, 0x00, 0x09};
/* end of AU4D01311 by d00107688 网关开启igmp snooping功能后，动态路由：br0的rip v2功能出现异常 */

#if 0
#define IS_DEBUGP(format, args...) printk(format,## args)
#else
#define PRSITE_DEBUGP(format, args...)
#endif

#if 0
unsigned char extra_mac[][6] = {  
    {0x01, 0x00, 0x5e, 0x7f, 0xff, 0xfa},  /*upnp*/
    {0x01, 0x00, 0x5e, 0x00, 0x00, 0x01},  /*sys1*/ 
    {0x01, 0x00, 0x5e, 0x00, 0x00, 0x02},  /*sys2*/
    {0x01, 0x00, 0x5e, 0x00, 0x00, 0x05},  /*ospf1*/
    {0x01, 0x00, 0x5e, 0x00, 0x00, 0x06},  /*ospf2*/
    {0x01, 0x00, 0x5e, 0x00, 0x00, 0x09},  /*ripv2*/
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},  /*sys*/
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}   /*end*/
};

int br_igmp_snooping_filter_mac(unsigned char *grp_mac)
{
    int i = 0;

    for( i = 0; extra_mac[i][0] != 0x00; i++ )
    {
        if ((!memcmp(grp_mac,extra_mac[i],ETH_ALEN))
        {
            return 0;
        }
    }

    return 1;
}

void br_igmp_snooping_del_by_grp_mac(struct net_bridge_port *dev_port, unsigned char *grp_mac)
{
	struct net_bridge_igmp_snooping_entry *dst_entry;

	spin_lock_bh(&igmp_snooping_list->igmp_lock);
	list_for_each_entry_rcu(dst_entry, &igmp_snooping_list.igmp_list, list)
	{
	    if ((!memcmp(&dst_entry->grp_mac, grp_mac, ETH_ALEN)) && (dst_entry->dev_dst == dev_port)) 
	    {
    		list_del_rcu(&dst_entry->list);
    		kfree(dst_entry);
	    }
	}
	spin_unlock_bh(&igmp_snooping_list->igmp_lock);	
}
#endif

void br_igmp_snooping_init(void)
{
	INIT_LIST_HEAD(&igmp_snooping_list.igmp_list);
	igmp_snooping_list.igmp_lock = SPIN_LOCK_UNLOCKED;
	igmp_snooping_list.igmp_start_timer = 0;
	igmp_snooping_list.igmp_snooping_enable = 0;
}

static void br_igmp_snooping_query_timeout(unsigned long ptr)
{
	struct net_bridge_igmp_snooping_entry *dst_entry;
	struct net_bridge_igmp_snooping *igmp_list;

	igmp_list = (struct net_bridge_igmp_snooping *) ptr;

	spin_lock_bh(&igmp_list->igmp_lock);
	list_for_each_entry_rcu(dst_entry, &igmp_list->igmp_list, list) 
    {
	    if ((jiffies > dst_entry->time) && ((jiffies - dst_entry->time) < (IGMP_QUERY_TIMEOUT*HZ)))
	    {
    		list_del_rcu(&dst_entry->list);
    		kfree(dst_entry);
	    }
	}
	spin_unlock_bh(&igmp_list->igmp_lock);
		
	mod_timer(&igmp_list->igmp_timer, jiffies + IGMP_TIMER_CHECK_TIMEOUT*HZ);		
}

static int br_igmp_snooping_update(struct net_bridge_port *dev_port, unsigned char *grp_mac, unsigned char *host_mac)
{
	struct net_bridge_igmp_snooping_entry *dst_entry;
	int ret = 0;
    
	list_for_each_entry_rcu(dst_entry, &igmp_snooping_list.igmp_list, list)
	{
	    if (!memcmp(&dst_entry->grp_mac, grp_mac, ETH_ALEN))
	    {
    		if ((!memcmp(&dst_entry->host_mac, host_mac, ETH_ALEN)) && (dst_entry->dev_dst == dev_port))
    		{    		
                dst_entry->time = jiffies + IGMP_QUERY_TIMEOUT*HZ;
    		    ret = 1;
    		}
	    }
	}
	
	return ret;
}

static struct net_bridge_igmp_snooping_entry *br_igmp_snooping_get(struct net_bridge_port *dev_port, unsigned char *grp_mac, unsigned char *host_mac)
{
	struct net_bridge_igmp_snooping_entry *dst_entry;

	list_for_each_entry_rcu(dst_entry, &igmp_snooping_list.igmp_list, list)
	{
	    if ((!memcmp(&dst_entry->grp_mac, grp_mac, ETH_ALEN)) && (!memcmp(&dst_entry->host_mac, host_mac, ETH_ALEN))) 
	    {
		    if (dst_entry->dev_dst == dev_port)
		    {
    		    return dst_entry;
		    }
	    }
	}
	
	return NULL;
}

static int br_igmp_snooping_add(struct net_bridge_port *dev_port, unsigned char *grp_mac, unsigned char *host_mac)
{
	struct net_bridge_igmp_snooping_entry *igmp_entry;


    /* start of AU4D01311 by d00107688 网关开启igmp snooping功能后，动态路由：br0的rip v2功能出现异常 */
	/*if grp_mac is upnp_mac,please return */
	if ((!memcmp(grp_mac, upnp_mac, ETH_ALEN))
        || (!memcmp(grp_mac, ripv2_mac, ETH_ALEN)))
	{
	    return 0;
	}
    /* end of AU4D01311 by d00107688 网关开启igmp snooping功能后，动态路由：br0的rip v2功能出现异常 */
		    
	if (br_igmp_snooping_update(dev_port, grp_mac, host_mac))
	{
    	return 0;
	}
	
	igmp_entry = kmalloc(sizeof(struct net_bridge_igmp_snooping_entry), GFP_KERNEL);
	if (!igmp_entry)
	{
	    return ENOMEM;
	}	
	memcpy(igmp_entry->host_mac, host_mac, ETH_ALEN);	
	memcpy(igmp_entry->grp_mac, grp_mac, ETH_ALEN);
	igmp_entry->time = jiffies + IGMP_QUERY_TIMEOUT*HZ;
    igmp_entry->dev_dst = dev_port;
    
	spin_lock_bh(&igmp_snooping_list.igmp_lock);
	list_add_rcu(&igmp_entry->list, &igmp_snooping_list.igmp_list);
	spin_unlock_bh(&igmp_snooping_list.igmp_lock);
    /* start A36D07088 two pc multicast by f00110348 20090724 */
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
	blog_stop(NULL, NULL);
#endif
    /* end A36D07088 two pc multicast by f00110348 20090724 */

	if (!igmp_snooping_list.igmp_start_timer) 
	{
    	init_timer(&igmp_snooping_list.igmp_timer);
	    igmp_snooping_list.igmp_timer.expires = jiffies + IGMP_TIMER_CHECK_TIMEOUT*HZ;
	    igmp_snooping_list.igmp_timer.function = br_igmp_snooping_query_timeout;
	    igmp_snooping_list.igmp_timer.data = (unsigned long)(&igmp_snooping_list);
	    add_timer(&igmp_snooping_list.igmp_timer);
	    igmp_snooping_list.igmp_start_timer = 1;
	}

	return 1;
}

static int br_igmp_snooping_del(struct net_bridge_port *dev_port, unsigned char *grp_mac, unsigned char *host_mac)
{
	struct net_bridge_igmp_snooping_entry *dst_entry;
    
	if ((dst_entry = br_igmp_snooping_get(dev_port, grp_mac, host_mac))) 
	{
    	spin_lock_bh(&igmp_snooping_list.igmp_lock);
	    list_del_rcu(&dst_entry->list);
	    kfree(dst_entry);
    #if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
	    blog_stop(NULL, NULL);
    #endif	    
    	spin_unlock_bh(&igmp_snooping_list.igmp_lock);

	    return 1;
	}
	
	return 0;
}

static int br_igmp_snooping_empty(unsigned char *grp_mac)
{
	struct net_bridge_igmp_snooping_entry *dst_entry;
    
	spin_lock_bh(&igmp_snooping_list.igmp_lock);
	list_for_each_entry_rcu(dst_entry, &igmp_snooping_list.igmp_list, list)
	{
	    if ((!memcmp(&dst_entry->grp_mac, grp_mac, ETH_ALEN))) 
	    {
			spin_unlock_bh(&igmp_snooping_list.igmp_lock);
	    	return 0;
	    }
	}
	spin_unlock_bh(&igmp_snooping_list.igmp_lock);
	return 1;
}

void br_igmp_snooping_show(void)
{
	struct net_bridge_igmp_snooping_entry *dst_entry;

	printk("\nigmp snooping: <%s>\n",(igmp_snooping_list.igmp_snooping_enable?"Enable":"Disable"));

	printk("\nbridge	device	group			source			timeout\n");
	list_for_each_entry_rcu(dst_entry, &igmp_snooping_list.igmp_list, list)
	{
	    if (NULL == dst_entry->dev_dst)
	    {
	        continue;
	    }
	    printk("%s   %s  	",dst_entry->dev_dst->br->dev->name, dst_entry->dev_dst->dev->name);
        printk("%02x:%02x:%02x:%02x:%02x:%02x",dst_entry->grp_mac[0],
                                               dst_entry->grp_mac[1],
                                               dst_entry->grp_mac[2],
                                               dst_entry->grp_mac[3],
                                               dst_entry->grp_mac[4],
                                               dst_entry->grp_mac[5]);
	    printk("	");
        printk("%02x:%02x:%02x:%02x:%02x:%02x",dst_entry->host_mac[0],
                                               dst_entry->host_mac[1],
                                               dst_entry->host_mac[2],
                                               dst_entry->host_mac[3],
                                               dst_entry->host_mac[4],
                                               dst_entry->host_mac[5]);
	    printk("	%d\n", (int) (dst_entry->time - jiffies)/HZ);
	}
}


void br_igmp_snooping_clear(void)
{
	struct net_bridge_igmp_snooping_entry *dst_entry;
   
	spin_lock_bh(&igmp_snooping_list.igmp_lock);
	list_for_each_entry_rcu(dst_entry, &igmp_snooping_list.igmp_list, list)
	{
	    list_del_rcu(&dst_entry->list);
	    kfree(dst_entry);
	}
	del_timer(&(igmp_snooping_list.igmp_timer));
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
	blog_stop(NULL, NULL);
#endif	
	spin_unlock_bh(&igmp_snooping_list.igmp_lock);
}

int br_igmp_snooping_forward(struct sk_buff *skb, struct net_bridge *br,unsigned char *dest,int forward)
{
    int k;
    int status = 0;
    unsigned char mc_mac[6];
    unsigned char *data = skb->data;
    unsigned short protocol ;
    unsigned short usIpHeadLen = 0;
    igmp_ipaddr ipaddr;
    struct sk_buff *skb2;
    struct net_bridge_igmp_snooping_entry *dst_entry;    
    struct igmpv3_report *pstRepV3;
    struct igmpv3_grec   *pstGrec;

#ifdef CONFIG_ATP_BRIDGE_BIND
    struct bridgebind_group *pgroup = NULL;
#endif

    protocol = ((u16 *) data)[-1];

    if (!igmp_snooping_list.igmp_snooping_enable || protocol != __constant_htons(ETH_P_IP))
    {
        return 0;
    }
    
    /* start of AU4D01311 by d00107688 网关开启igmp snooping功能后，动态路由：br0的rip v2功能出现异常 */
    if((1 == igmp_snooping_list.igmp_snooping_enable) 
        && (!memcmp(dest, upnp_mac, ETH_ALEN)
            || !memcmp(dest, ripv2_mac, ETH_ALEN)))
    {
        return 0;
    }
    /* end of AU4D01311 by d00107688 网关开启igmp snooping功能后，动态路由：br0的rip v2功能出现异常 */
    
    if (data[9] == IPPROTO_IGMP) 
    {
        /* AU4D00944 AU4D00955: 增加skb->dev->br_port指针合法性判断，否则br_nf_forward_ip会出错 */
        if (NULL == skb->dev->br_port)
        {
            return 0;
        }
        usIpHeadLen = (skb->data[0] & 0x0f) * 4;
        if ((data[usIpHeadLen] == IGMPV2_HOST_MEMBERSHIP_REPORT 
            || data[usIpHeadLen] == IGMP_HOST_MEMBERSHIP_REPORT)
             && protocol == __constant_htons(ETH_P_IP))
        {
            br_igmp_snooping_add(skb->dev->br_port, dest, eth_hdr(skb)->h_source);
        }
        else if (data[usIpHeadLen] == IGMPV3_HOST_MEMBERSHIP_REPORT && 
            protocol == __constant_htons(ETH_P_IP))
        {
            pstRepV3 = (struct igmpv3_report *)&data[usIpHeadLen];
            pstGrec = &pstRepV3->grec[0];
            for (k = 0; k < pstRepV3->ngrec; k++)
            {
                ipaddr.ulIpAddr = pstGrec->grec_mca;
                mc_mac[0] = 0x01;
                mc_mac[1] = 0x00;
                mc_mac[2] = 0x5e;
                mc_mac[3] = 0x7f & ipaddr.acIpAddr[1];
                mc_mac[4] = ipaddr.acIpAddr[2];
                mc_mac[5] = ipaddr.acIpAddr[3];
                if ((IGMPV3_MODE_IS_EXCLUDE == pstGrec->grec_type) || 
                    (IGMPV3_CHANGE_TO_EXCLUDE == pstGrec->grec_type) || 
                    (pstGrec->grec_nsrcs != 0 && IGMPV3_MODE_IS_INCLUDE == pstGrec->grec_type) || 
                    (pstGrec->grec_nsrcs != 0 && IGMPV3_CHANGE_TO_INCLUDE == pstGrec->grec_type) ||
                    (IGMPV3_ALLOW_NEW_SOURCES == pstGrec->grec_type)) 
                {
        		    br_igmp_snooping_add(skb->dev->br_port, mc_mac, eth_hdr(skb)->h_source);
        		}
        		else if ((IGMPV3_CHANGE_TO_INCLUDE == pstGrec->grec_type && pstGrec->grec_nsrcs == 0) || 
                         (pstGrec->grec_nsrcs == 0 && IGMPV3_MODE_IS_INCLUDE == pstGrec->grec_type) || 
                         (IGMPV3_BLOCK_OLD_SOURCES == pstGrec->grec_type)) 
                {
        		    br_igmp_snooping_del(skb->dev->br_port, mc_mac, eth_hdr(skb)->h_source);
        		}
                pstGrec = (struct igmpv3_grec *)((char *)pstGrec + sizeof(struct igmpv3_grec) + pstGrec->grec_nsrcs * sizeof(struct in_addr)); 
            }
        }
        else if (data[usIpHeadLen] == IGMP_HOST_LEAVE_MESSAGE) 
        {
            mc_mac[0] = 0x01;
            mc_mac[1] = 0x00;
            mc_mac[2] = 0x5e;
            mc_mac[3] = 0x7f & data[usIpHeadLen + 5];
            mc_mac[4] = data[usIpHeadLen + 6];
            mc_mac[5] = data[usIpHeadLen + 7];
            br_igmp_snooping_del(skb->dev->br_port, mc_mac, eth_hdr(skb)->h_source);
            if (br_igmp_snooping_empty(mc_mac))
            {
                skb2 = skb_clone(skb, GFP_ATOMIC);
	        #if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
		        blog_clone(skb, skb2->blog_p);
            #endif
                /* start of AU4D01376 by d00107688 2009-02-16  存在一条PPPoE的WAN,同时开启IGMP Snooping与IGMP Proxy,且将该wan与IGMP Proxy绑定,再将IGMP Proxy去使能,网关会打印cpu 0重启 */
                if (forward)
                {
                    br_flood_forward(br, skb2, 0);
                }
                else
                {
                    br_flood_deliver(br, skb2, 0);
                }
                /* end of AU4D01376 by d00107688 2009-02-16  存在一条PPPoE的WAN,同时开启IGMP Snooping与IGMP Proxy,且将该wan与IGMP Proxy绑定,再将IGMP Proxy去使能,网关会打印cpu 0重启 */
            }		    
            status = 1;
        }
        return status;
    }

    list_for_each_entry_rcu(dst_entry, &igmp_snooping_list.igmp_list, list)
    {
        if (!memcmp(&dst_entry->grp_mac, dest, ETH_ALEN)) 
        {
            if (NULL == dst_entry->dev_dst)
            {
                continue;
            }
#ifdef CONFIG_ATP_BRIDGE_BIND
            if(br->group_enable)
            {
                pgroup = br_get_group(br, brgetgroup(br, skb));

                if(((NULL != pgroup)
                && (ISIN_GROUP(pgroup->ports, dst_entry->dev_dst->port_no)))
                || (!forward)) //路由WAN的组播流不判断绑定组
                {
                    skb2 = skb_clone(skb, GFP_ATOMIC);
                    /* start A36D07088 two pc multicast by f00110348 20090724 */
                #if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
		            blog_clone(skb, skb2->blog_p);
                #endif
                    /* end A36D07088 two pc multicast by f00110348 20090724 */
                    if (forward)
                    {
                        br_forward(dst_entry->dev_dst, skb2);
                    }
                    else
                    {
                        br_deliver(dst_entry->dev_dst, skb2);
                    }
                }

                if( NULL != pgroup ) 
                {
                    br_release_group(pgroup);
                }
            }
            else
            {
#endif
                skb2 = skb_clone(skb, GFP_ATOMIC);
                /* start A36D07088 two pc multicast by f00110348 20090724 */
            #if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
		        blog_clone(skb, skb2->blog_p);
            #endif
                /* end A36D07088 two pc multicast by f00110348 20090724 */
                if (forward)
                {
                    br_forward(dst_entry->dev_dst, skb2);
                }
                else
                {
                    br_deliver(dst_entry->dev_dst, skb2);
                }
#ifdef CONFIG_ATP_BRIDGE_BIND                
            }
#endif            
            status = 1;
        }	    
    }
    
    memset(mc_mac, 0xff, 6);
    if((1 == igmp_snooping_list.igmp_snooping_enable) 
          && memcmp(mc_mac, eth_hdr(skb)->h_dest, ETH_ALEN))
    {
        status = 1;
    }
    
    if ((!forward) && (status))
    {
        kfree_skb(skb);
    }

    return status;
}

void br_igmp_snooping_set_enable(int enable)
{
    igmp_snooping_list.igmp_snooping_enable = enable;
}

int br_igmp_snooping_get_enable(void)
{
    return igmp_snooping_list.igmp_snooping_enable;
}


