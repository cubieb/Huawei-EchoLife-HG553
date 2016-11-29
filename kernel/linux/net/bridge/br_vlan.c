
#include <linux/kernel.h>
#include <linux/times.h>
#include <linux/if_vlan.h>
#include <asm/atomic.h>
#include <linux/module.h>
#include <linux/ip.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/netfilter_bridge.h>

#include "br_private.h"
#include "br_vlan.h"

static int debug_flag = 0;

#if 1
#define VLAN_DEBUGP(format, args...) do { if (debug_flag){printk(format,## args);}}while(0)
#else
#define VLAN_DEBUGP(format, args...)
#endif

static unsigned int br_vlan_get_type(struct net_bridge_port *port,unsigned short vlan_id)
{
    int i = 0;

    /*TAG*/
    for (i = 0; i < VLAN_ID_NUM_MAX; i++)
    {
        if (port->vlaninfo.tag[i] != 0
            && port->vlaninfo.tag[i] == vlan_id)
        {
            return TYPE_VLAN_TAG;
        }
    }

    /*UNTAG*/
    for (i = 0; i < VLAN_ID_NUM_MAX; i++)
    {
        if (port->vlaninfo.untag[i] != 0
            && port->vlaninfo.untag[i] == vlan_id)
        {
            return TYPE_VLAN_UNTAG;
        }
    }

    if (port->vlaninfo.pvid != 0
        && port->vlaninfo.pvid == vlan_id)
    {
        return TYPE_VLAN_PVID;
    }
    
    return TYPE_VLAN_OTHER;
}

static int br_vlan_id_tag_exist(struct net_bridge_port *port,unsigned short vlan_id)
{
    int i = 0;
    int ret = 0;
    
    for (i = 0; i < VLAN_ID_NUM_MAX; i++)
    {
        if (port->vlaninfo.tag[i] != 0
            && port->vlaninfo.tag[i] == vlan_id)
        {
            ret = 1;
            break;
        }
    }
    
    return ret;
}

static int br_vlan_port_tag_or_untag_exist(struct net_bridge_port *port)
{
    int i = 0;
    int ret = 0;
    
    for (i = 0; i < VLAN_ID_NUM_MAX; i++)
    {
        if (port->vlaninfo.tag[i] != 0
            || port->vlaninfo.untag[i] != 0)
        {
            ret = 1;
            break;
        }
    }

    return ret;
}

void br_vlan_init_port(struct net_bridge_port *port)
{
    int i = 0;

 //   printk("br_vlan_init_port=================>\r\n");
    for (i = 0; i < VLAN_ID_NUM_MAX; i++)
    {
        port->vlaninfo.tag[i] = 0;
        port->vlaninfo.untag[i] = 0;
    }

    port->vlaninfo.pvid = 0;

    port->vlaninfo.reserved = 0;

    return ;
}

void br_vlan_set_by_port(struct net_bridge_port *port,unsigned int ulOpt,unsigned int value)
{
    int i = 0;

    switch(ulOpt)
    {
        case TYPE_VLAN_TAG:
            {
                for (i = 0; i < VLAN_ID_NUM_MAX; i++)
                {
                    if (port->vlaninfo.tag[i] != 0
                        && port->vlaninfo.tag[i] == value)
                    {
                        return;
                    }
                }

                for (i = 0; i <VLAN_ID_NUM_MAX; i++)
                {
                    if (port->vlaninfo.tag[i] == 0)
                    {
                        port->vlaninfo.tag[i] = value;
                        break;
                    }
                }
            }
            break;
        case TYPE_VLAN_UNTAG:
            {
                for (i = 0; i < VLAN_ID_NUM_MAX; i++)
                {
                    if (port->vlaninfo.untag[i] != 0
                        && port->vlaninfo.untag[i] == value)
                    {
                        return;
                    }
                }

                for (i = 0; i <VLAN_ID_NUM_MAX; i++)
                {
                    if (port->vlaninfo.untag[i] == 0)
                    {
                        port->vlaninfo.untag[i] = value;
                        break;
                    }
                }
                
            }
            break;
        case TYPE_VLAN_PVID:
            {
                port->vlaninfo.pvid = value;
            }
            break;
         default:
            break;
    }
    
    return;
}

void br_vlan_del_by_port(struct net_bridge_port *port,unsigned int ulOpt,unsigned int value)
{
    int i = 0;
    
    switch(ulOpt)
    {
        case TYPE_VLAN_TAG:
            {
                for (i = 0; i < VLAN_ID_NUM_MAX; i++)
                {
                    if (port->vlaninfo.tag[i] != 0
                        && port->vlaninfo.tag[i] == value)
                    {
                        port->vlaninfo.tag[i] = 0;
                        break;
                    }
                }
            }
            break;
        case TYPE_VLAN_UNTAG:
            {
                for (i = 0; i < VLAN_ID_NUM_MAX; i++)
                {
                    if (port->vlaninfo.untag[i] != 0
                        && port->vlaninfo.untag[i] == value)
                    {
                        port->vlaninfo.untag[i] = 0;
                        break;
                    }
                }                
            }
            break;
        case TYPE_VLAN_PVID:
            {
                port->vlaninfo.pvid = 0;
            }
            break;
         default:
            break;
    }
    
    return;
}

void br_vlan_show(struct net_bridge *br)
{
    struct net_bridge_port *p;
    int flag = 0;
    int i = 0;
    
    printk("\n<**************** bridge %s    ****************>\r\n", br->dev->name);
    printk("vlan enable      : %d\r\n", br->vlan_enable);

    if (br->vlan_enable)
    {
        list_for_each_entry_rcu(p, &br->port_list, list)
        {
            printk("\n<**************** device %s ****************>\r\n", p->dev->name);
            printk("%7s","tag:");
            flag = 0;
            for (i = 0; i < VLAN_ID_NUM_MAX; i++)
            {
                if (p->vlaninfo.tag[i] != 0)
                {
                    if(flag == 0)
                    {
                        printk("%d",p->vlaninfo.tag[i]);
                        flag = 1;
                    }
                    else
                    {
                        printk(",%d",p->vlaninfo.tag[i]);
                        
                    }
                }
            }
            
            printk("\r\n");
            printk("%7s","untag:");
            flag = 0;
            for (i = 0; i < VLAN_ID_NUM_MAX; i++)
            {
                if (p->vlaninfo.untag[i] != 0)
                {
                    if(flag == 0)
                    {
                        printk("%d",p->vlaninfo.untag[i]);
                        flag = 1;
                    }
                    else
                    {
                        printk(",%d",p->vlaninfo.untag[i]);
                        
                    }
                }
            }
            
            printk("\r\n");
            printk("%7s","pvid:");
            if (p->vlaninfo.pvid != 0)
            {
                printk("%d",p->vlaninfo.pvid);
            }

            printk("\r\n");

        }
    }

    return;
}

void br_vlan_debug(void)
{
    debug_flag = !debug_flag;
}

/*
该函数在br_handle_frame_finish之前调用，被放在钩子点NF_BR_PRE_ROUTING上。
indev: not NULL,
outdev: is NULL
*/
static unsigned int br_vlan_pre_routing(unsigned int hook, struct sk_buff **pskb,
   const struct net_device *indev, const struct net_device *outdev,
   int (*okfn)(struct sk_buff *))
{
    struct sk_buff *skb = *pskb;
    struct vlan_hdr   *vlhdr = NULL;
    int ret = NF_ACCEPT;
    struct net_bridge_port	*inport = rcu_dereference(indev->br_port);

    skb->vlan_id_flags = TYPE_VLAN_OTHER;
    skb->h_vlan_id     = 0;  
    skb->h_vlan_nook   = LAN_VLAN_ID_INIT;

    if (inport->br->vlan_enable)
    {
        /*对LAN侧 eth0.*或者wl0.*的包进行VLAN判断*/   
        if (indev->name[0] == 'e'
            || indev->name[0] == 'w')
        {
            /*报文中带了VLAN ID*/
            if (__constant_htons(ETH_P_8021Q) == skb->protocol)
            {
                vlhdr = (struct vlan_hdr*)skb->data;
                unsigned short vlan_id = ntohs(vlhdr->h_vlan_TCI);

                /* 
                   报文中的VLAN ID与该端口配置的 VLAN ID相等
                   动作:去掉VLAN头，并将VLAN ID记录在SKB中。
                */
                if (br_vlan_id_tag_exist(inport,vlan_id))
                {
                    
                    
                    /*去掉VLAN 头，然后把VLAN标记到skb中*/
                    skb->vlan_id_flags = TYPE_VLAN_TAG;
                    skb->h_vlan_id     = vlan_id;  
                    
                    VLAN_DEBUGP("<br_vlan_pre_routing>:coming in lan vlan_id:%d,<port_name:%s>\r\n",vlan_id,indev->name);

                    ret = NF_ACCEPT;
                    
                }
                /*
                报文中的VLAN ID 与 该端口配置的VLAN ID 不相等
                动作:丢弃该报文.  
                */
                else
                {
                    VLAN_DEBUGP("<br_vlan_pre_routing>:coming in lan vlan_id:%d,<but port_name:%s not this tag:drop>\r\n",vlan_id,indev->name);

                    ret = NF_DROP;
                }
            }
            /*报文中没有带VLAN ID*/
            else
            {
                /*该端口配置了tag或者untag值*/
                if (br_vlan_port_tag_or_untag_exist(inport))
                {
                    /*该端口又配置了PVID，则使用它，并标记在skb中*/
                    if (inport->vlaninfo.pvid != 0)
                    {
                        skb->vlan_id_flags = TYPE_VLAN_PVID;
                        skb->h_vlan_id     = inport->vlaninfo.pvid;        
                        
                        VLAN_DEBUGP("<br_vlan_pre_routing>:coming in lan NOT vlan_id,indev<%s>,pvid:<%d>\r\n",indev->name,indev->br_port->vlaninfo.pvid);

                        ret = NF_ACCEPT;
                        
                    }
                    /*该端口没有配置PVID，则丢弃*/
                    else
                    {
                                          
                        VLAN_DEBUGP("<br_vlan_pre_routing>:packet not vlan_id,but indev<%s>,have tag or untag,not pvid:drop\r\n",indev->name);
                        
                        ret = NF_DROP;
                    }
                }
                /*该端口没有tag和untag*/
                else
                {
                    /*但该端口又配置了PVID，则使用它，并标记在skb中*/
                    if (inport->vlaninfo.pvid != 0)
                    {
                        skb->vlan_id_flags = TYPE_VLAN_PVID;
                        skb->h_vlan_id     = inport->vlaninfo.pvid;        
                        
                        VLAN_DEBUGP("<br_vlan_pre_routing>:indev<%s>,not tag and untag,but pvid:<%d>\r\n",indev->name,indev->br_port->vlaninfo.pvid);

                        ret = NF_ACCEPT;
                        
                    }
                    /*该端口没有配置PVID*/
                    else
                    {
                        
                        skb->vlan_id_flags = TYPE_VLAN_UNTAG;
                        skb->h_vlan_id     = 0;
                        
                        VLAN_DEBUGP("<br_vlan_pre_routing>:indev<%s>,not pvid and tag and untag\r\n",indev->name);
                        
                        ret = NF_ACCEPT;
                    }
                }
            }
        }
        /*WAN 侧进来的包*/
        else
        {
            VLAN_DEBUGP("<br_vlan_pre_routing>:coming in wan interface<%s>\r\n",indev->name);
            ret = NF_ACCEPT;
        }
    }
    else
    {
        VLAN_DEBUGP("<br_vlan_pre_routing>:vlan don't enable\r\n");

        ret = NF_ACCEPT;
    }
    
    return ret;
}

/*
该函数在转发之前调用，被放在钩子点NF_BR_FORWARD上。
indev: not NULL,
outdev: not NULL
*/
static unsigned int br_vlan_forward(unsigned int hook, struct sk_buff **pskb,
   const struct net_device *indev, const struct net_device *outdev,
   int (*okfn)(struct sk_buff *))
{
    struct sk_buff *skb = *pskb;
    int ret = NF_ACCEPT;
    unsigned int ulType = TYPE_VLAN_OTHER;
    struct net_bridge_port	*inport = rcu_dereference(indev->br_port);
    struct net_bridge_port	*outport = rcu_dereference(outdev->br_port);

    if (inport->br->vlan_enable)
    {
        /*对LAN侧 eth0.*或者wl0.*的包进行VLAN判断*/   
        if (indev->name[0] == 'e' || indev->name[0] == 'w')
        {
            /*转发给LAN 侧eth0.*或者wl0.*的包*/
            if (outdev->name[0] == 'e' || outdev->name[0] == 'w')
            {
                if (skb->vlan_id_flags != TYPE_VLAN_UNTAG
                    && skb->h_vlan_id != 0)
                {
                    ulType = br_vlan_get_type(outport,skb->h_vlan_id);

                    if (TYPE_VLAN_TAG == ulType)
                    {
                        /*添加802.1p的报文头*/
                        if (__constant_htons(ETH_P_8021Q) != skb->protocol)
                        {
                            skb->h_vlan_nook   = LAN_VLAN_ID_TAG;
                        }
                        
                        VLAN_DEBUGP("<br_vlan_forward>:indev<%s>,outdev<%s> tag:%d !\r\n",indev->name,outdev->name,skb->h_vlan_id);

                        ret = NF_ACCEPT;
                    }
                    else if (TYPE_VLAN_UNTAG == ulType)
                    {
                        /*移走802.1p的报文头*/
                        if (__constant_htons(ETH_P_8021Q) == skb->protocol)
                        {
                            skb->h_vlan_nook   = LAN_VLAN_ID_REMOVE;
                        }
                       
                        VLAN_DEBUGP("<br_vlan_forward>:indev<%s>,outdev<%s> untag :%d!\r\n",indev->name,outdev->name,skb->h_vlan_id);

                        ret = NF_ACCEPT;
                    }
                    else if (TYPE_VLAN_PVID == ulType)
                    {
                        VLAN_DEBUGP("<br_vlan_forward>:indev<%s>,outdev<%s> pvid:%d !\r\n",indev->name,outdev->name,skb->h_vlan_id);

                        ret = NF_ACCEPT;
                    }
                    else
                    {
                        VLAN_DEBUGP("<br_vlan_forward>:indev<%s>,outdev<%s> snoopf,drop\r\n",indev->name,outdev->name);

                        ret = NF_DROP;
                    }                    
                }
                /*进来的包没有带VLAN ID 并且 indev也没有配置PVID*/
                else
                {
                    /*判断出去的outdev是否和indev一样*/
                    if (br_vlan_port_tag_or_untag_exist(outport))
                    {
                        VLAN_DEBUGP("<br_vlan_forward>:indev<%s> not tag and untag,but outdev<%s> have,drop\r\n",indev->name,outdev->name);

                        ret = NF_DROP;
                    }
                    else
                    {
                        if (outport->vlaninfo.pvid != 0)
                        {
                            VLAN_DEBUGP("indev<%s> not pvid and tag and untag,but outdev<%s> have,drop\r\n",indev->name,outdev->name);

                            ret = NF_DROP;
                        }
                        else
                        {
                            VLAN_DEBUGP("indev<%s> not pvid and tag and untag,but outdev<%s> also have not!\r\n",indev->name,outdev->name);

                            ret = NF_ACCEPT;
                        }
                    }
                    
                }
            }
            /*转发给WAN 侧的包*/
            else
            {
                VLAN_DEBUGP("<br_vlan_forward>:indev<%s>,outdev<%s>\r\n",indev->name,outdev->name);
                
                ret = NF_ACCEPT;
            }
        }
        /*WAN 侧进来的包*/
        else
        {
            VLAN_DEBUGP("<br_vlan_forward>:coming in wan interface<%s>\r\n",indev->name);

            ret = NF_ACCEPT;
        }
    }
    else
    {
        VLAN_DEBUGP("<br_vlan_forward>:vlan don't enable\r\n");

        ret = NF_ACCEPT;
    }
    
    return ret;
}

/*标记该包由网关本身发出*/
static unsigned int br_vlan_local_out(unsigned int hook, struct sk_buff **pskb,
				    const struct net_device *in,
				    const struct net_device *out,
				    int (*okfn)(struct sk_buff *))
{
    struct sk_buff *skb = *pskb;

    skb->vlan_id_flags = TYPE_VLAN_OTHER;
    skb->h_vlan_id     = 0;  
    skb->h_vlan_nook   = LAN_VLAN_ID_LOCAL_OUT;
    
    return NF_ACCEPT;
}

static inline struct sk_buff *br_vlan_put_tag(struct sk_buff *skb, unsigned short tag)
{
	struct vlan_ethhdr *veth;

	if (skb_headroom(skb) < VLAN_HLEN) 
	{
		struct sk_buff *sk_tmp = skb;
		skb = skb_realloc_headroom(sk_tmp, VLAN_HLEN);
		kfree_skb(sk_tmp);
		if (!skb) {
			printk(KERN_ERR "vlan: failed to realloc headroom\n");
			return NULL;
		}
	} 
    
	veth = (struct vlan_ethhdr *)skb_push(skb, VLAN_HLEN);

	/* Move the mac addresses to the beginning of the new header. */
	memmove(skb->data, skb->data + VLAN_HLEN, 2 * VLAN_ETH_ALEN);

	/* first, the ethernet type */
	veth->h_vlan_proto = __constant_htons(ETH_P_8021Q);

	/* now, the tag */
	veth->h_vlan_TCI = htons(tag);

	skb->protocol = __constant_htons(ETH_P_8021Q);
	skb->mac.raw -= VLAN_HLEN;
	skb->nh.raw -= VLAN_HLEN;

	return skb;
}


int br_vlan_xmit_in(struct sk_buff *skb)
{
    unsigned short reserved = 0; //字节对齐
    unsigned short vlan_id = 0;
    struct net_bridge_port *port = NULL; 

    rcu_read_lock_bh();

    vlan_id = skb->h_vlan_id;
    
    port = rcu_dereference(skb->dev->br_port);
    
    if (port->br->vlan_enable)
    {        
        switch(skb->h_vlan_nook)
        {
            case LAN_VLAN_ID_TAG:
                {
                    if (__constant_htons(ETH_P_8021Q) != skb->protocol)
                    {
                        skb = br_vlan_put_tag(skb, vlan_id);

                        if (!skb)
                        {
                            printk("BRVLAN: br vlan xmit routing set vlan id error\r\n");
                            return 1;
                        }
                        
                        VLAN_DEBUGP("<br_vlan_xmit_in>:set vlan id success :%d,outdev:<%s>\r\n",vlan_id,skb->dev->name);
                    }
                }
                break;
            case LAN_VLAN_ID_REMOVE:
                {
                    if (__constant_htons(ETH_P_8021Q) == skb->protocol)
                    {
                        unsigned tmp[VLAN_ETH_ALEN * 2];
                		struct vlan_ethhdr *vhdr = (struct vlan_ethhdr *)(skb->data);
                        unsigned short vlan_id = ntohs(vhdr->h_vlan_TCI);
                        unsigned short proto = vhdr->h_vlan_encapsulated_proto;
#if 0                    
                        skb = skb_share_check(skb, GFP_ATOMIC);
                        if (!skb)
                        {
                            printk("BRVLAN: unshare skb fail ,maybe memery not enough or other reasons, %s, %d\n", __FUNCTION__, __LINE__);
                            return 1;
                        }
#endif 
                        /*remove vlan tag*/
                		memcpy(tmp, skb->data, VLAN_ETH_ALEN * 2);
                		skb_pull(skb, VLAN_HLEN);
                		memcpy(skb->data, tmp, VLAN_ETH_ALEN * 2);
                		skb->nh.raw += VLAN_HLEN;
                		
                		skb->protocol = proto;
                   
                		VLAN_DEBUGP("<br_vlan_xmit_in>:remove vlan id success :%d,outdev:<%s>\r\n",vlan_id,skb->dev->name);
                    }
                }
                break;
            default:
                break;
        }
    }

    rcu_read_unlock_bh();
    
    return 0;
}

static unsigned int br_vlan_local_in(unsigned int hook, struct sk_buff **pskb,
				   const struct net_device *in,
				   const struct net_device *out,
				   int (*okfn)(struct sk_buff *))
{
    struct sk_buff *skb = *pskb;
    
	/* If pass up to IP, remove VLAN header */
	if (skb->protocol == __constant_htons(ETH_P_8021Q)) 
	{
		unsigned short proto;
		struct vlan_hdr *vhdr = (struct vlan_hdr *)(skb->data);
        unsigned short vlan_id = ntohs(vhdr->h_vlan_TCI);
        
		skb = skb_share_check(skb, GFP_ATOMIC);
		if (skb) 
		{
			memmove(skb->data - ETH_HLEN + VLAN_HLEN,
				skb->data - ETH_HLEN, VLAN_ETH_ALEN * 2);
			skb_pull(skb, VLAN_HLEN);
			skb->mac.raw += VLAN_HLEN;
			skb->nh.raw += VLAN_HLEN;
			skb->h.raw += VLAN_HLEN;
		}
		/* make sure protocol is correct before passing up */
		proto = vhdr->h_vlan_encapsulated_proto;
		skb->protocol = proto;
		/* TODO: do we need to assign skb->priority? */

		VLAN_DEBUGP("<br_vlan_local_in>:remove vlan id success :%d,indev:<%s>\r\n",vlan_id,in->name);
	}
	
	return NF_ACCEPT;
}

static struct nf_hook_ops br_vlan_ops[] = {
	{ .hook = br_vlan_pre_routing, 
	  .owner = THIS_MODULE, 
	  .pf = PF_BRIDGE, 
	  .hooknum = NF_BR_PRE_ROUTING, 
	  .priority = NF_BR_PRI_BRNF - 100, },   //优先级最高 -100
	{ .hook = br_vlan_forward,
	  .owner = THIS_MODULE,
	  .pf = PF_BRIDGE,
	  .hooknum = NF_BR_FORWARD,
	  .priority = NF_BR_PRI_BRNF - 100, }, // 优先级最高 -100
	{ .hook = br_vlan_local_out,
	  .owner = THIS_MODULE,
	  .pf = PF_BRIDGE,
	  .hooknum = NF_BR_LOCAL_OUT,
	  .priority = NF_BR_PRI_BRNF - 100, },       //在此链上优先级低一点点 -100	
	{ .hook = br_vlan_local_in,
	  .owner = THIS_MODULE,
	  .pf = PF_BRIDGE,
	  .hooknum = NF_BR_LOCAL_IN,
	  .priority = NF_BR_PRI_BRNF - 100, },	       //优先级高一点点了 -100  
};

void br_vlan_init(void)
{
	int i;
    
	for (i = 0; i < ARRAY_SIZE(br_vlan_ops); i++) {
		int ret;

		if ((ret = nf_register_hook(&br_vlan_ops[i])) >= 0)
			continue;

		while (i--)
			nf_unregister_hook(&br_vlan_ops[i]);

		return ;
	}

	printk(KERN_NOTICE "Bridge LAN vlan registered\n");

	return ;
}

void br_vlan_fini(void)
{
	int i;

	for (i = ARRAY_SIZE(br_vlan_ops) - 1; i >= 0; i--)
		nf_unregister_hook(&br_vlan_ops[i]);

	return ;
}
