/*
 *	Handle incoming frames
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_input.c,v 1.2 2010/07/10 12:36:08 l43571 Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/netfilter_bridge.h>
#if defined(CONFIG_MIPS_BRCM)
#include <linux/if_vlan.h>
#include <linux/timer.h>
#include <linux/igmp.h>
#include <linux/ip.h>
#include <linux/blog.h>
#endif
#include "br_private.h"
#ifdef CONFIG_IGMP_SNOOPING
#include "br_igmp_snooping.h"
#endif
#ifdef CONFIG_ATP_BRIDGE_BIND
#include <net/wan_bind.h>
#endif

#ifdef CONFIG_SUPPORT_ATP
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#endif

#ifdef CONFIG_ATP_SUPPORT_HG553
#define DHCP_OPTION         1024
#define DHCP_OPTION_LEN   268
extern char dhcpoption[DHCP_OPTION]; 
/* BEGIN: Modified by w00135358, 2010/01/27: bug112 of ip phone arp HG553V100R001*/
extern int dhcpIpphoneVlan;
/* END: Modified by w00135358, 2010/01/27: bug112 of ip phone arp HG553V100R001*/
#define VLAN_GET_VID(tag) (((unsigned short)(tag))&VLAN_VID_MASK)
#endif /* CONFIG_ATP_SUPPORT_HG553 */

#ifdef CONFIG_ATP_BRIDGE_BIND
extern struct WanBindMap g_stBindInfo[8];
extern struct list_head g_wanDevs;

extern spinlock_t g_wanLock;
extern int ip_check_wan_init();
#endif

#ifdef CONFIG_ATP_SUPPORT_HG553
#define OPTION_FIELD		0
#define FILE_FIELD		1
#define SNAME_FIELD		2

#define OPT_CODE 0
#define OPT_LEN 1
#define OPT_DATA 2
#define DHCP_VENDOR 0x3c

#define DHCP_PADDING	0x00
#define DHCP_OPTION_OVER 0x34
#define DHCP_END		0xFF
static unsigned char *get_option60(unsigned char* data, int code)
{
	int i, length;
	static char err[] = "bogus packet, option fields too long."; /* save a few bytes */
	unsigned char *optionptr;
	int over = 0, done = 0, curr = OPTION_FIELD;
	
	optionptr = data;
	i = 0;
	length = 308;
	while (!done) {
		if (i >= length) {
			return NULL;
		}
		if (optionptr[i + OPT_CODE] == code) {
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				return NULL;
			}
			return optionptr + i ;
		}			
		switch (optionptr[i + OPT_CODE]) {
		case DHCP_PADDING:
			i++;
			break;
		case DHCP_OPTION_OVER:
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				return NULL;
			}
			over = optionptr[i + 3];
			i += optionptr[OPT_LEN] + 2;
			break;
		case DHCP_END:
			if (curr == OPTION_FIELD && over & FILE_FIELD) {
				//optionptr = packet->file;
				i = 0;
				length = 128;
				curr = FILE_FIELD;
			} else if (curr == FILE_FIELD && over & SNAME_FIELD) {
				//optionptr = packet->sname;
				i = 0;
				length = 64;
				curr = SNAME_FIELD;
			} else done = 1;
			break;
		default:
			i += optionptr[OPT_LEN + i] + 2;
		}
	}
	return NULL;
}

int check_option60(unsigned char *optionstr, unsigned char *vendorstr)
{
    char r1, r2;
    char *l;
    if (optionstr == NULL || vendorstr == NULL)
    {
        return 0;
    }
     if (!strlen(vendorstr))
	  return 0;
    if(!strlen(optionstr))
	  return 0;
    char *end = optionstr + strlen(optionstr)-1;
    l = strstr(optionstr, vendorstr);
    for (;;) {
        r1 =0; r2=0;
        if (l != NULL) {
            if (l == optionstr) {
                r1 = 1;
            }
            else { // need to see | if not the first one
                if (*(l-1) == '|') {
                    r1 = 1;
                }
            }
            
            if (*(l + strlen(vendorstr)) == 0 || *(l + strlen(vendorstr)) == '|' ) { //need to see |if not last one
                r2 = 1;
            }

            if (r1 && r2) {
                /*if (loc != NULL) {
                    *loc = l;
                }*/
                return 1;
            }
            else {
                if (l < end) {
                    l = strstr(l+1, vendorstr);
                }
            }
        }
        else {
            return 0;
        }
    }
}

/* BEGIN: Modified by w00135358, 2010/01/27: bug112 of ip phone arp HG553V100R001*/
int isOption60Correct(char *server_config_ipt60, char *pUserVenderID, int opt60_len)
{
    char *pOp60 = NULL;
    char szVendID[128] = {0};
	
	//printk("pUserVenderID:%s,opt60_len:%d\n",pUserVenderID,opt60_len);
    if (server_config_ipt60 != NULL && pUserVenderID != NULL)
    {
        if ((opt60_len > 0) && (opt60_len < 128))
        {
        	memset(szVendID, 0, sizeof(szVendID));
            memcpy(szVendID, pUserVenderID, opt60_len);
        }
        else
        {
            return 0;
        }
        
        pOp60 = strstr(server_config_ipt60, szVendID);
        
        if (pOp60 != NULL)
        {
            int nLen = strlen(szVendID);
            
            if (*(pOp60 + nLen ) == '%' || *(pOp60 + nLen ) == '\0')
            {
                if (pOp60 > server_config_ipt60)
                {
                    if (*(pOp60 - 1) == '%')
                    {
                        return 1;
                    }
                }
                else
                {
                    return 1;
                }
            }
        }
    }

    return 0;
}
/* END: Modified by w00135358, 2010/01/27: bug112 of ip phone arp HG553V100R001*/

#endif /* CONFIG_ATP_SUPPORT_HG553 */
/*start of ATP 2008.01.16 增加 for DHCP设备类型二层透传 by c60023298 */
#ifdef CONFIG_ATP_BR_DEV_RELAY
#define OPTION_FIELD		0
#define FILE_FIELD		1
#define SNAME_FIELD		2

#define OPT_CODE 0
#define OPT_LEN 1
#define OPT_DATA 2
#define DHCP_VENDOR 0x3c

#define DHCP_PADDING	0x00
#define DHCP_OPTION_OVER 0x34
#define DHCP_END		0xFF

static unsigned char *get_option_china(unsigned char* data, int code)
{
	int i, length;
	static char err[] = "bogus packet, option fields too long."; /* save a few bytes */
	unsigned char *optionptr;
	int over = 0, done = 0, curr = OPTION_FIELD;
	
	optionptr = data;
	i = 0;
	length = 308;
	while (!done) {
		if (i >= length) {
			return NULL;
		}
		if (optionptr[i + OPT_CODE] == code) {
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				return NULL;
			}
			return optionptr + i + 2;
		}			
		switch (optionptr[i + OPT_CODE]) {
		case DHCP_PADDING:
			i++;
			break;
		case DHCP_OPTION_OVER:
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				return NULL;
			}
			over = optionptr[i + 3];
			i += optionptr[OPT_LEN] + 2;
			break;
		case DHCP_END:
			if (curr == OPTION_FIELD && over & FILE_FIELD) {
				//optionptr = packet->file;
				i = 0;
				length = 128;
				curr = FILE_FIELD;
			} else if (curr == FILE_FIELD && over & SNAME_FIELD) {
				//optionptr = packet->sname;
				i = 0;
				length = 64;
				curr = SNAME_FIELD;
			} else done = 1;
			break;
		default:
			i += optionptr[OPT_LEN + i] + 2;
		}
	}
	return NULL;
}
static int find_relay_index(unsigned char *buff)
{
   int index;
    if(!memcmp(buff,"STB",3))
    {
       index = 0;
    }
    else if(!memcmp(buff,"Phone",5))
    {
       index = 1;
    }
    else if(!memcmp(buff,"Camera",6))
    {
       index = 2;
    }
    else
    {
      index = 3;
    }    
    
    return index;
}
#endif
/*end of ATP 2008.01.16 增加 for DHCP设备类型二层透传 by c60023298 */

/*start of ATP 2008.01.16 增加 for DHCP端口绑定二层透传 by c60023298 */
#ifdef CONFIG_ATP_BRIDGE_BIND
#ifdef CONFIG_ATP_BR_BIND_RELAY
static int Br_bind_dhcp_relay(struct net_bridge *br,struct sk_buff *skb)
{
	int iEnable = 0;
	unsigned short usWanGroupid = 0;	
	unsigned short usLanGroupid  = 0;	
	int i = 0;
	
	if(NULL == skb || NULL == br)
	{
		return iEnable;
	}
              
   usLanGroupid = brgetgroup(br,skb);

   if(usLanGroupid == 0)
   {
         return iEnable; 
   }

   for( i = 0; i < BIND_RELAY_NUM; i++)
   {
			if(g_bindrelay.iEnable[i] && g_bindrelay.acWanName[i][0] != 'e' && g_bindrelay.acWanName[i][0] != 'w')
         {
               usWanGroupid = brgetgroupbyname(br,g_bindrelay.acWanName[i]);
    	     if(usWanGroupid == usLanGroupid)
		     {
		    		iEnable = 1;
		    		break;
   		     }
         }
    }
    return iEnable;    
}
#endif
#endif
/*end of ATP 2008.01.16 增加 for DHCP端口绑定二层透传 by c60023298 */
/* Bridge group multicast address 802.1d (pg 51). */
const u8 br_group_address[ETH_ALEN] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x00 };
#ifdef CONFIG_ATP_BR_DEV_RELAY
/* start of edit by c60023298 for dhcp relay by port, 2008-03-12 */
static int Br_port_dhcp_relay(struct sk_buff *skb)
{
    int iEnable = 0;
	unsigned short usWanGroupid = 0;	
	unsigned short usLanGroupid  = 0;	
	int i = 0;
	
	if(NULL == skb)
	{
		return iEnable;
	}
              
       for( i = 0; i < BIND_RELAY_NUM; i++)
       {
            if (g_bindrelay.iEnable[i] && (g_bindrelay.acWanName[i][0] == 'e' || g_bindrelay.acWanName[i][0] == 'w'))
            {
                if(!strcmp(g_bindrelay.acWanName[i],skb->dev->name))
                {
//printk("\n *************the g_bindrelay.acWanName[%d] is %s\n",i,g_bindrelay.acWanName[i]);
                    iEnable = 1;
                    break;
                }
            }
    }
    return iEnable;    
}
/* end of edit by c60023298 for dhcp relay by port, 2008-03-12 */
#endif


static void br_pass_frame_up(struct net_bridge *br, struct sk_buff *skb)
{
	struct net_device *indev;

#ifdef CONFIG_ATP_BRIDGE_BIND
    int i = 0;
    char devname[32] = "";

        if (skb->protocol == __constant_htons(ETH_P_PPP_SES)
		||skb->protocol == __constant_htons(ETH_P_PPP_DISC))
    {
        //printk("skb is from %s\n",skb->dev->name);
        if(skb->mark & MARK_LAN_MASK)
        {
           i = mark_name(skb->mark);
        }

        if (-1 != i)
        {
        //绑定了，然后绑定的wan类型为桥，丢弃
        if(g_stBindInfo[i].ulBindFlag )
        {
            /* start of zhuhui 2007-11-19: 修改问题单AU4D00202:条pvc的一条会话开启pppoe proxy，另一个会话绑订的Lan口下所连的pc发起pppoe拨号抓包显示认证流程 */
            if((!strcmp(g_stBindInfo[i].acConnType,"IP_Bridged")) ||
               (!strcmp(g_stBindInfo[i].acConnType,"PPPoE_Bridged")) ||
               ((!strcmp(g_stBindInfo[i].acConnType,"IP_Routed")) && !(g_stBindInfo[i].ulPppxFlag)))
            /* end of zhuhui 2007-11-19:　修改问题单AU4D00202*/
            {
                printk("conntype is %s, not bind ,so drop\n",g_stBindInfo[i].acConnType);
                kfree_skb(skb);
                //printk("skb is from bridge %s\n",skb->dev->name);
                return;
            }
        }
        //没有绑定，如果不存在没绑定的pppx，则丢弃
        else
        {
            struct WanDev *wandev = NULL;
            int lFind = 0;
            /*Jaffen modify start: 解决没有WAN接口时死机问题*/
            if(ip_check_wan_init())
            {
                spin_lock_bh(&g_wanLock);
                list_for_each_entry_rcu(wandev, &g_wanDevs, list)
                {
                    if((wandev->lPppx == 1) && (wandev->lBinded == 0))
                    {
                        lFind = 1;
                        break;
                    }
                }
                spin_unlock_bh(&g_wanLock);
            }
            /*Jaffen modify end*/
            if(!lFind)
            {
                kfree_skb(skb);
                //printk("skb is from bridge %s\n",skb->dev->name);
                return;
            }
        }
    }
        }
#endif
	br->statistics.rx_packets++;
	br->statistics.rx_bytes += skb->len;

	indev = skb->dev;
    /* add lan interface (eth0.5) in iptables 20090217 */
        skb->lanindev = indev;
        skb->dev = br->dev;
        skb->mark |= MARK_ROUTE_MASK;//走路由


	NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_IN, skb, indev, NULL,
		netif_receive_skb);
}

void addr_debug(unsigned char *dest)
{
#define NUM2PRINT 50
	char buf[NUM2PRINT * 3 + 1];	/* 3 chars per byte */
	int i = 0;
	for (i = 0; i < 6 && i < NUM2PRINT; i++) {
		sprintf(buf + i * 3, "%2.2x ", 0xff & dest[i]);
	}
	printk("%s ", buf);
}

#ifdef CONFIG_SUPPORT_ATP
/*start of ATP 2008.05.05 获取lan侧发送组播报文其IP和MAC地址对应关系以便协议栈记录日志使用 add by h00101845*/
struct mc_ip_mac
{
    struct list_head     ipmac_list;
    struct timer_list    ipmac_timer;
    spinlock_t           ipmac_lock;
    int                  ipmac_timer_start;
    int                  count;
};

struct ip_mac_entry
{
    struct list_head    list;
    char ip[16];
    unsigned char mac[ETH_ALEN];
    long   time;
};

struct mc_ip_mac  mim_list;

#define IP_MAC_TIMEOUT        135
#define IP_MAC_CHECK_TIMEOUT  2

static void ip_mac_timeout(unsigned long data)
{
    struct ip_mac_entry  *mim_entry;
    struct mc_ip_mac     *im_list;

    im_list = (struct mc_ip_mac*)data;

    spin_lock_bh(&im_list->ipmac_lock);
	list_for_each_entry_rcu(mim_entry, &im_list->ipmac_list, list) 
    {
	    if ((jiffies > mim_entry->time) && ((jiffies - mim_entry->time) < (IP_MAC_TIMEOUT*HZ)))
	    {
    		list_del_rcu(&mim_entry->list);
            mim_list.count--;
    		kfree(mim_entry);
	    }
	}
	spin_unlock_bh(&im_list->ipmac_lock);
		
	mod_timer(&im_list->ipmac_timer, jiffies + IP_MAC_CHECK_TIMEOUT*HZ);	
}

static int ip_mac_add(unsigned char *host_mac, unsigned char *host_ip)
{
    struct ip_mac_entry *im_entry;

    list_for_each_entry_rcu(im_entry, &mim_list.ipmac_list, list)
    {
        if (!memcmp(im_entry->mac, host_mac, ETH_ALEN))
        {
            if (!memcmp(im_entry->ip, host_ip, 16))
            {
                im_entry->time = jiffies + IP_MAC_TIMEOUT*HZ;
                return 0;
            }
            else
            {
                spin_lock_bh(&mim_list.ipmac_lock);
                list_del_rcu(&im_entry->list);
                mim_list.count--;
        		kfree(im_entry);
                spin_unlock_bh(&mim_list.ipmac_lock);
            }
        }
    }

    im_entry = kmalloc(sizeof(struct ip_mac_entry), GFP_KERNEL);
    if (!im_entry)
	{
	    return -ENOMEM;
	}

    memcpy(im_entry->mac, host_mac, ETH_ALEN);	
	memcpy(im_entry->ip, host_ip, 16);
	im_entry->time = jiffies + IP_MAC_TIMEOUT*HZ;

    spin_lock_bh(&mim_list.ipmac_lock);
	list_add_rcu(&im_entry->list, &mim_list.ipmac_list);
	spin_unlock_bh(&mim_list.ipmac_lock);

	if (!mim_list.ipmac_timer_start) 
	{
    	init_timer(&mim_list.ipmac_timer);
	    mim_list.ipmac_timer.expires = jiffies + IP_MAC_CHECK_TIMEOUT*HZ;
	    mim_list.ipmac_timer.function = ip_mac_timeout;
	    mim_list.ipmac_timer.data = (unsigned long)(&mim_list);
	    add_timer(&mim_list.ipmac_timer);
	    mim_list.ipmac_timer_start = 1;
        mim_list.count++;
	}

    return 0;
}

static void ip_match_mac(struct sk_buff *skb)
{
    unsigned char *data = skb->data;
    unsigned short protocol ;
    unsigned char ipaddr[16] = {0};
    protocol = ((u16 *) data)[-1];
    sprintf(ipaddr, "%u.%u.%u.%u", data[12], data[13], data[14], data[15]);

    if (data[9] == IPPROTO_IGMP && protocol == __constant_htons(ETH_P_IP))
    {
#if 0        
        printk("************%s, ipaddr: [%s], macaddr: [%02x:%02x:%02x:%02x:%02x:%02x]**********\r\n", __FUNCTION__, ipaddr, 
            skb->mac.ethernet->h_source[0],
            skb->mac.ethernet->h_source[1],
            skb->mac.ethernet->h_source[2],
            skb->mac.ethernet->h_source[3],
            skb->mac.ethernet->h_source[4],
            skb->mac.ethernet->h_source[5]);
#endif
        ip_mac_add(eth_hdr(skb)->h_source, ipaddr);              
    }

    return ;
}

#ifdef CONFIG_PROC_FS

static rwlock_t ip_mac_lock = RW_LOCK_UNLOCKED;

struct ip_mac_iter
{
    int ct;
};

static struct ip_mac_entry *ip_mac_seq_idx(struct ip_mac_iter *iter,
					   loff_t pos)
{
    struct ip_mac_entry *im_entry = NULL;
    struct list_head *lh;

    list_for_each_entry_rcu(im_entry, &mim_list.ipmac_list, list)
    {
        if (pos-- == 0)
        {
            return im_entry;
        }
    }

	return NULL;   
}

static int ip_mac_flg = 0;

static void *mac_mc_seq_start(struct seq_file *seq, loff_t *pos)
{
    read_lock(&ip_mac_lock);    
    return *pos ? ip_mac_seq_idx(seq->private, *pos - 1) 
		: SEQ_START_TOKEN;
}

static void *mac_mc_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
    struct ip_mac_entry *im_entry;
    struct ip_mac_iter *iter = seq->private;
    
    ++*pos;
	if (v == SEQ_START_TOKEN)
	{
		return ip_mac_seq_idx(iter, 0);
	}
    
    if (ip_mac_flg < (mim_list.count - 1))
    {
        ip_mac_flg++;
        im_entry = (struct ip_mac_entry*)(((struct ip_mac_entry*)v)->list.next);
        return im_entry;
    }
    
	return NULL;
}

static void mac_mc_seq_stop(struct seq_file *seq, void *v)
{
    ip_mac_flg = 0;
	read_unlock(&ip_mac_lock);
}


static int mac_mc_seq_show(struct seq_file *seq, void *v)
{
    struct mc_ip_mac    *mim;

	spin_lock_bh(&mim->ipmac_lock);
    if (v == SEQ_START_TOKEN) 
    {
		seq_puts(seq, 
			 "IP Address            HW Address\n");
	}
    else if (v != NULL)
    {
        struct ip_mac_entry *im_entry = v;
        seq_printf(seq, "%-16s      %02x:%02x:%02x:%02x:%02x:%02x", 
               im_entry->ip,
        	   im_entry->mac[0],
        	   im_entry->mac[1],
        	   im_entry->mac[2],
        	   im_entry->mac[3],
        	   im_entry->mac[4],
        	   im_entry->mac[5]);
        seq_putc(seq, '\n');
    }
        
	spin_unlock_bh(&mim->ipmac_lock);
	return 0;
}

static struct seq_operations mac_mc_seq_ops = {
	.start = mac_mc_seq_start,
	.next  = mac_mc_seq_next,
	.stop  = mac_mc_seq_stop,
	.show  = mac_mc_seq_show,
};

static int mac_mc_seq_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &mac_mc_seq_ops);
}

static struct file_operations mac_mc_seq_fops = {
	.owner	 = THIS_MODULE,
	.open    = mac_mc_seq_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release,
};

#endif
void __init br_ip_mac_mc_init(void)
{
    INIT_LIST_HEAD(&mim_list.ipmac_list);
	mim_list.ipmac_lock = SPIN_LOCK_UNLOCKED;
    mim_list.ipmac_timer_start = 0;
    mim_list.count = 0;

    proc_net_fops_create("mac_mcast", 0, &mac_mc_seq_fops);
}
/*end of ATP 2008.05.05 获取lan侧发送组播报文其IP和MAC地址对应关系以便协议栈记录日志使用 add by h00101845*/
#endif

/* note: already called with rcu_read_lock (preempt_disabled) */
int br_handle_frame_finish(struct sk_buff *skb)
{
	const unsigned char *dest = eth_hdr(skb)->h_dest;
	struct net_bridge_port *p = rcu_dereference(skb->dev->br_port);
	struct net_bridge *br;
	struct net_bridge_fdb_entry *dst;
	int passedup = 0;
	unsigned short usIpHeadLen = 0;
/*start of ATP bridge bind, z37589, 20070814*/
#ifdef CONFIG_ATP_BRIDGE_BIND
        struct bridgebind_group *pgroup = NULL;
#endif
/*end of ATP bridge bind, z37589, 20070814*/

	if (!p || p->state == BR_STATE_DISABLED)
		goto drop;

	/* insert into forwarding database after filtering to avoid spoofing */
	br = p->br;
/*start of ATP bridge bind, z37589, 20070814*/
#ifdef CONFIG_ATP_BRIDGE_BIND
        if(p->br->group_enable)
        {
           do{
                pgroup = br_get_group(p->br, brgetgroup(p->br, skb));
                
                if(NULL == pgroup)
                    break;

                /*学习时候应该不论什么组号都要学习，组限制应该在转发和发送出去时候做*/
                /*目前虽然转发处理时也判断了报文是否在同一个组中，但是学习时候也应该做一下限制*/
                if(pgroup->macln_enable && ISIN_GROUP(pgroup->ports, p->port_no))
                {
                    br_fdb_update(br, p, eth_hdr(skb)->h_source);                      
                }
                br_release_group(pgroup);
               
            }while(0);
        }
        else
#endif
/*end of ATP bridge bind, z37589, 20070814*/
	    br_fdb_update(br, p, eth_hdr(skb)->h_source);

	if (p->state == BR_STATE_LEARNING)
		goto drop;

	if (br->dev->flags & IFF_PROMISC) {
		struct sk_buff *skb2;

		skb2 = skb_clone(skb, GFP_ATOMIC);
		if (skb2 != NULL) {
			passedup = 1;
			br_pass_frame_up(br, skb2);
		}
	}
    
#ifdef CONFIG_ATP_SUPPORT_HG553
        if (dest[0] & 1) 
        {
            /* BEGIN: Modified by w00135358, 2010/01/27: bug112 of ip phone arp HG553V100R001*/
            if(ntohs(skb->protocol) == 0x0800)
            {
                if((dest[0]== 0xff))
                {
                     if(skb->data[9] == IPPROTO_UDP)
                     {
                           //判断是否溢出，或者用宏来表示
                           if(skb->data[20] == 0x00 && skb->data[21] == 0x44 
                               && skb->data[22] == 0x00 && skb->data[23] == 0x43)
                           {
                               if(dhcpIpphoneVlan != 0 && skb->len > DHCP_OPTION_LEN)
                               {
                                   unsigned char * option = get_option60(&skb->data[DHCP_OPTION_LEN], DHCP_VENDOR);
                                   unsigned char tmpbuff[256];
                                   memset(tmpbuff, 0, 256);
                                  
                                   if(NULL != option)
                                   {
                                      char *tmppoint = option;
                                      memcpy(tmpbuff,tmppoint + 2,*(tmppoint + 1));
                                      if(isOption60Correct(dhcpoption, tmpbuff,*(option + 1)) == 1)
                                      {
                                           //TODO:check vlan!
                                           int optionLen = *(option+1);
                                           memset(option+2, 0xFF, optionLen);
                                           //udp checksum??
                                      }
                                   }
                               }
                           }
                     }
                }                                        
            }
            else if (skb->protocol == __constant_htons(ETH_P_8021Q)) {
                unsigned short proto;
                struct vlan_hdr *vhdr = (struct vlan_hdr *)(skb->data);
                if(vhdr->h_vlan_encapsulated_proto  == 0x0800)
                {
                    if((dest[0]== 0xff))
                    {
                          if(skb->data[9+4] == IPPROTO_UDP)
                          {
                                //判断是否溢出，或者用宏来表示
                                if(skb->data[20+4] == 0x00 && skb->data[21+4] == 0x44 
                                    && skb->data[22+4] == 0x00 && skb->data[23+4] == 0x43)
                                {
                                    printk("!!Get dhcp packet with vlan:%d,dhcpIpphoneVlan:%d!!\n",VLAN_GET_VID(vhdr->h_vlan_TCI),dhcpIpphoneVlan);
                                    if (dhcpIpphoneVlan == VLAN_GET_VID(vhdr->h_vlan_TCI) && skb->len > DHCP_OPTION_LEN+4)
                                    {
                                        unsigned char * option = get_option60(&skb->data[DHCP_OPTION_LEN+4], DHCP_VENDOR);
                                        unsigned char tmpbuff[256];
                                        memset(tmpbuff, 0, 256);
                                        
                                        if(NULL != option)
                                        {
                                            char *tmppoint = option;
                                            memcpy(tmpbuff,tmppoint + 2,*(tmppoint + 1));
                                            if(isOption60Correct(dhcpoption, tmpbuff,*(option + 1)) == 0)
                                            {
                                                 //TODO:check vlan!
                                                 kfree_skb(skb);
                                                 goto out;
                                            }
                                            else
                                            {    
                                                 //TODO:check vlan!
                                            }
                                        }
                                        else
                                        {
                                            //TODO:check vlan!
                                            kfree_skb(skb);
                                            goto out;
                                        }
                                    }
                                }
                          }
                    }
                }
            }
            /* END: Modified by w00135358, 2010/01/27: bug112 of ip phone arp HG553V100R001*/
        }
#endif /* CONFIG_ATP_SUPPORT_HG553 */

/*start of ATP 2008.01.16 增加 for DHCP二层透传 by c60023298 */
#if defined(CONFIG_ATP_BR_DEV_RELAY) || defined(CONFIG_ATP_BRIDGE_BIND)

    usIpHeadLen = (skb->data[0] & 0x0f) * 4;

    if(dest[0]== 0xff)
    {
        if(skb->data[9] == IPPROTO_UDP)
        {
            if(skb->data[usIpHeadLen] == 0x00 && skb->data[usIpHeadLen + 1] == 0x44 
            && skb->data[usIpHeadLen + 2] == 0x00 && skb->data[usIpHeadLen + 3] == 0x43)
            {
/*start of ATP 2008.01.16 增加 for DHCP端口绑定二层透传 by c60023298 */               
			               if(Br_port_dhcp_relay(skb))
			               {
                                struct net_device *pdev1 = dev_get_by_name(skb->dev->name);
//printk("\n &&&&&&&&&&&&&&&&&&&&&&&&&&&&& usIpHeadLen is %d\n",usIpHeadLen);                                                                
                                if( NULL != pdev1 && NULL != pdev1->br_port)
                                {
                                    /*start of 问题单:AU4D01221,dhcp relay 导致网口不发包,by c00126165 2008-12-30*/
                                     //br_flood_forward(br,skb,!passedup);
                                     br_flood_forward(br, skb, 0);  
                                    /*end of 问题单:AU4D01221,dhcp relay 导致网口不发包,by c00126165 2008-12-30*/
                                    goto out;
                                }
			               	}
#ifdef CONFIG_ATP_BRIDGE_BIND
#ifdef CONFIG_ATP_BR_BIND_RELAY
                if(Br_bind_dhcp_relay(br,skb))
                {   
                    struct net_device *pdev = dev_get_by_name(skb->dev->name);

                    if( NULL != pdev && NULL != pdev->br_port)
                    {
                                     /*start of 问题单:AU4D01221,dhcp relay 导致网口不发包,by c00126165 2008-12-30*/
                                     //br_flood_forward(br,skb,!passedup);
                                     br_flood_forward(br, skb, 0);  
                                     /*end of 问题单:AU4D01221,dhcp relay 导致网口不发包,by c00126165 2008-12-30*/
                                    goto out;
                    }
                }
#endif
#endif	
/*end of ATP 2008.01.16 增加 for DHCP端口绑定二层透传 by c60023298 */

/*start of ATP 2008.01.16 增加 for DHCP设备类型二层透传 by c60023298 */
#ifdef CONFIG_ATP_BR_DEV_RELAY
                unsigned char * option = get_option_china(&skb->data[usIpHeadLen + 248], DHCP_VENDOR);
                unsigned char tmpbuff[255];
                int relay_index;
                int index;
                int lOptLength = 0;
                if(NULL != option)
                {
                    unsigned char *tmppoint = option;

                    tmppoint += 2; 
                    for(index = 0; index < 5; index++)
                    {
                        if(NULL != tmppoint && lOptLength < *(option - 1))
                        {
                            if(*tmppoint == 2)
                            {
                                memcpy(tmpbuff,tmppoint + 2,*(tmppoint + 1));
                                break;
                            }
                        }
                        else
                        {
                            index = 5;
                            break;
                        }
                        tmppoint = tmppoint + *(tmppoint + 1) + 2;
                        lOptLength = tmppoint - option;
                    }
                    if(index == 5)
                    {
                        memcpy(tmpbuff, option, *(option -1));
                    }
                        relay_index = find_relay_index(tmpbuff);                              
                }
                else
                {
                    relay_index = 3;
                }
                if(!g_devrelay.iEnable[relay_index])
                {
                    br_pass_frame_up(br,skb);
                    goto out;
                }
                else
                {
    	          /*start of 问题单:AU4D01221,dhcp relay 导致网口不发包,by c00126165 2008-12-30*/
                  //br_flood_forward(br,skb,!passedup);
    	          br_flood_forward(br, skb, 0);
    	          /*end of 问题单:AU4D01221,dhcp relay 导致网口不发包,by c00126165 2008-12-30*/
    	           goto out;
                }
#endif
/*end of ATP 2008.01.16 增加 for DHCP设备类型二层透传 by c60023298 */
                }
            }
        }
#endif
/*end of ATP 2008.01.16 增加 for DHCP二层透传 by c60023298 */
	if (is_multicast_ether_addr(dest)) {
#ifdef CONFIG_SUPPORT_ATP
/*add by h00101845 for igmp log 2008-05-05*/
        ip_match_mac(skb);
/*add by h00101845 for igmp log 2008-05-05*/
#endif
#ifdef CONFIG_IGMP_SNOOPING
          if (!br_igmp_snooping_forward(skb,br,dest, 1))   
#endif
		br_flood_forward(br, skb, !passedup);
		if (!passedup)  
			br_pass_frame_up(br, skb);
		goto out;
	}
	dst = __br_fdb_get(br, dest);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
	blog_br_fdb(skb, __br_fdb_get(br, eth_hdr(skb)->h_source), dst);
#endif
	if (dst != NULL && dst->is_local) {
		if (!passedup)
			br_pass_frame_up(br, skb);
		else
			kfree_skb(skb);
		goto out;
	}
	if (dst != NULL) {
/*start of ATP bridge bind, z37589, 20070814*/
#ifdef CONFIG_ATP_BRIDGE_BIND
        if (br->group_enable) {
    		struct bridgebind_group *pgroup = br_get_group(br, brgetgroup(br, skb));
                
    		if( NULL != pgroup )
		    {
		        if( (((*(skb->dev->name) == 'e') && (*(skb->dev->name+1) == 't') && (*(skb->dev->name+2) == 'h'))
		            || ((*(skb->dev->name) == 'w') && (*(skb->dev->name+1) == 'l')))
		            && (((*(dst->dst->dev->name) == 'e') && (*(dst->dst->dev->name+1) == 't') && (*(dst->dst->dev->name+2) == 'h'))
		            || ((*(dst->dst->dev->name) == 'w') && (*(dst->dst->dev->name+1) == 'l'))) )
	            {
	                br_forward(dst->dst, skb);
			        br_release_group(pgroup);    
    		        goto out;
	            }
		        
		        if(pgroup->macln_enable && ISIN_GROUP(pgroup->ports , dst->dst->port_no))
	            {
	                br_forward(dst->dst, skb);
			        br_release_group(pgroup);    
    		        goto out;
	            }		        
                br_release_group(pgroup);    
		    }
    		else
		    {
    		    kfree_skb(skb);
		        goto out;
		    }
    		
	    }
	    else
#endif
/*end of ATP bridge bind, z37589, 20070814*/
        {
		br_forward(dst->dst, skb);
		goto out;
	}
	}

	br_flood_forward(br, skb, 0);

out:
	return 0;
drop:
	kfree_skb(skb);
	goto out;
}

/* note: already called with rcu_read_lock (preempt_disabled) */
static int br_handle_local_finish(struct sk_buff *skb)
{
	struct net_bridge_port *p = rcu_dereference(skb->dev->br_port);

	if (p && p->state != BR_STATE_DISABLED)
		br_fdb_update(p->br, p, eth_hdr(skb)->h_source);

	return 0;	 /* process further */
}

/* Does address match the link local multicast address.
 * 01:80:c2:00:00:0X
 */
static inline int is_link_local(const unsigned char *dest)
{
	return memcmp(dest, br_group_address, 5) == 0 && (dest[5] & 0xf0) == 0;
}

/*
 * Called via br_handle_frame_hook.
 * Return 0 if *pskb should be processed furthur
 *	  1 if *pskb is handled
 * note: already called with rcu_read_lock (preempt_disabled)
 */
int br_handle_frame(struct net_bridge_port *p, struct sk_buff **pskb)
{
	struct sk_buff *skb = *pskb;
	const unsigned char *dest = eth_hdr(skb)->h_dest;

	if (!is_valid_ether_addr(eth_hdr(skb)->h_source))
		goto err;

	if (unlikely(is_link_local(dest))) {
		skb->pkt_type = PACKET_HOST;
		return NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_IN, skb, skb->dev,
			       NULL, br_handle_local_finish) != 0;
	}

	if (p->state == BR_STATE_FORWARDING || p->state == BR_STATE_LEARNING) {
		if (br_should_route_hook) {
			if (br_should_route_hook(pskb))
				return 0;
			skb = *pskb;
			dest = eth_hdr(skb)->h_dest;
		}

		if (!compare_ether_addr(p->br->dev->dev_addr, dest))
			skb->pkt_type = PACKET_HOST;
        /* start 64 byte can not reach 15M in vdsl A36D06384 by f00110348 20090331 */
        if (sysctl_fast_routing)
        {
            br_handle_frame_finish(skb);
        }
        else
        {
    		NF_HOOK(PF_BRIDGE, NF_BR_PRE_ROUTING, skb, skb->dev, NULL, br_handle_frame_finish);
        }
        /* end 64 byte can not reach 15M in vdsl A36D06384 by f00110348 20090331 */
        return 1;
	}

err:
	kfree_skb(skb);
	return 1;
}
