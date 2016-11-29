/*
 *	Ioctl handler
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_ioctl.c,v 1.2 2010/07/10 12:36:08 l43571 Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/capability.h>
#include <linux/kernel.h>
#include <linux/if_bridge.h>
#include <linux/netdevice.h>
#include <linux/times.h>
#include <asm/uaccess.h>
#include "br_private.h"
#ifdef CONFIG_IGMP_SNOOPING
#include "br_igmp_snooping.h"
#endif
#ifdef CONFIG_ATP_LAN_VLAN
#include "br_vlan.h"
#endif

int g_FastBridging = 0;

#ifdef CONFIG_ATP_SUPPORT_HG553
/*start of 增加添加内核brctl option60的值 by s53329 at  20070731*/
#define    DHCP_OPTION    1024
char dhcpoption[DHCP_OPTION];   
/*end  of 增加添加内核brctl option60的值 by s53329 at  20070731*/
/* BEGIN: Modified by w00135358, 2010/01/27: bug112 of ip phone arp HG553V100R001*/
int dhcpIpphoneVlan;
/* END: Modified by w00135358, 2010/01/27: bug112 of ip phone arp HG553V100R001*/
#endif /* CONFIG_ATP_SUPPORT_HG553 */

/*start of ATP 2008.01.16 增加 for DHCP设备类型二层透传 by c60023298 */
#ifdef CONFIG_ATP_BR_DEV_RELAY
struct dev_relay g_devrelay = {
	.iEnable = {0},
};
EXPORT_SYMBOL(g_devrelay);
#endif
/*end of ATP 2008.01.16 增加 for DHCP设备类型二层透传 by c60023298 */

/*start of ATP 2008.01.16 增加 for DHCP端口绑定二层透传 by c60023298 */       
#ifdef CONFIG_ATP_BRIDGE_BIND
#ifdef CONFIG_ATP_BR_BIND_RELAY
struct bind_relay g_bindrelay = {
	.acWanName = {0},
    .iEnable = {0},
};

EXPORT_SYMBOL(g_bindrelay);
static int br_bind_relay_exist(const char *value, int flag)
{
   int i;
   for(i = 0; i < BIND_RELAY_NUM; i++)
   	{
   	  	if(!memcmp(g_bindrelay.acWanName[i],value,IFNAMSIZ))
   	  	{
	   	  	g_bindrelay.iEnable[i] = flag;
	   	  	return 1;
   	  	}
   	}
   return 0;
}
static void br_add_bind_relay(const char *value,int flag)
{
    int i;
    unsigned char buff[IFNAMSIZ +1]={0};
    if(br_bind_relay_exist(value,flag))
    {
  	   return;
    }
    for(i = 0; i < BIND_RELAY_NUM; i++)
    {
       if(!memcmp(g_bindrelay.acWanName[i],buff,IFNAMSIZ))
       {
       	  memcpy(g_bindrelay.acWanName[i],value,IFNAMSIZ);       	
          g_bindrelay.iEnable[i] = flag;
          break;
       }
    }
       return;
}

static void br_del_bind_relay(const char *value)
{
    int i;
    for(i = 0; i < BIND_RELAY_NUM; i++)
    {
       if(!memcmp(g_bindrelay.acWanName[i], value,IFNAMSIZ))
       {     	  
          memset(g_bindrelay.acWanName[i],0,IFNAMSIZ); 
          g_bindrelay.iEnable[i] = 0;
          break;
       }
    }
    return ;
}
#endif
#endif
/*end of ATP 2008.01.16 增加 for DHCP端口绑定二层透传 by c60023298 */    

/*start of ATP 2008.01.16 增加 for 显示二层透传内核信息 by c60023298 */
#if defined(CONFIG_ATP_BR_DEV_RELAY) || defined(CONFIG_ATP_BRIDGE_BIND)
static void br_get_showrelay(const char *name)
{
#ifdef CONFIG_ATP_BR_DEV_RELAY
     if(memcmp(name ,"device",6) == 0)
     {
          printk("\n Category \t\t Enable\n");
          printk("  STB     \t\t  %d \n",g_devrelay.iEnable[0]);
          printk(" Phone    \t\t  %d \n",g_devrelay.iEnable[1]);
          printk(" Camera   \t\t  %d \n",g_devrelay.iEnable[2]);
          printk(" Computer \t\t  %d \n",g_devrelay.iEnable[3]);
     }
#ifdef CONFIG_ATP_BRIDGE_BIND
#ifdef CONFIG_ATP_BR_BIND_RELAY     
     else if(memcmp(name,"bind",4) == 0)
     {
          printk("\n Interface \t\t Enable\n");
          int i;
          unsigned char buff[IFNAMSIZ + 1]={0};
          for(i = 0; i < BIND_RELAY_NUM; i++)
          {
             if(memcmp(g_bindrelay.acWanName[i], buff,IFNAMSIZ)!= 0)
             {
                     printk(" %s    \t\t  %d \n",g_bindrelay.acWanName[i],g_bindrelay.iEnable[i]);
             }
          }
     }
#endif
#endif
#else
#ifdef CONFIG_ATP_BRIDGE_BIND
#ifdef CONFIG_ATP_BR_BIND_RELAY
     if(memcmp(name,"bind",4) == 0)
     {
          printk("\n Interface \t\t Enable\n");
          int i;
          unsigned char buff[IFNAMSIZ + 1]={0};
          for(i = 0; i < BIND_RELAY_NUM; i++)
          {
             if(memcmp(g_bindrelay.acWanName[i], buff,IFNAMSIZ)!= 0)
             {
                     printk(" %s    \t\t  %d \n",g_bindrelay.acWanName[i],g_bindrelay.iEnable[i]);
             }
          }
     }
#endif
#endif
#endif
     return;
}
#endif
/*end of ATP 2008.01.16 增加 for 显示二层透传内核信息 by c60023298 */
/* called with RTNL */
static int get_bridge_ifindices(int *indices, int num)
{
	struct net_device *dev;
	int i = 0;

	for (dev = dev_base; dev && i < num; dev = dev->next) {
		if (dev->priv_flags & IFF_EBRIDGE)
			indices[i++] = dev->ifindex;
	}

	return i;
}

/* called with RTNL */
static void get_port_ifindices(struct net_bridge *br, int *ifindices, int num)
{
	struct net_bridge_port *p;

	list_for_each_entry(p, &br->port_list, list) {
		if (p->port_no < num)
			ifindices[p->port_no] = p->dev->ifindex;
	}
}

/*
 * Format up to a page worth of forwarding table entries
 * userbuf -- where to copy result
 * maxnum  -- maximum number of entries desired
 *            (limited to a page for sanity)
 * offset  -- number of records to skip
 */
static int get_fdb_entries(struct net_bridge *br, void __user *userbuf,
			   unsigned long maxnum, unsigned long offset)
{
	int num;
	void *buf;
	size_t size;

	/* Clamp size to PAGE_SIZE, test maxnum to avoid overflow */
	if (maxnum > PAGE_SIZE/sizeof(struct __fdb_entry))
		maxnum = PAGE_SIZE/sizeof(struct __fdb_entry);

	size = maxnum * sizeof(struct __fdb_entry);

	buf = kmalloc(size, GFP_USER);
	if (!buf)
		return -ENOMEM;

	num = br_fdb_fillbuf(br, buf, maxnum, offset);
	if (num > 0) {
		if (copy_to_user(userbuf, buf, num*sizeof(struct __fdb_entry)))
			num = -EFAULT;
	}
	kfree(buf);

	return num;
}
#ifdef CONFIG_ATP_BRIDGE_BIND
static int get_br_lan_name(struct net_bridge *br,void __user *userbuf,
                           unsigned long maxnum,unsigned long offset)
{
        int num;
	void *buf;
	size_t size = maxnum * sizeof(struct __fdb_lan);

	if (size > PAGE_SIZE) {
		size = PAGE_SIZE;
		maxnum = PAGE_SIZE/sizeof(struct __fdb_lan);
	}

	buf = kmalloc(size, GFP_USER);
	if (!buf)
		return -ENOMEM;
	
	num = br_fdb_filllanbuf(br, buf, maxnum, offset);
	if (num > 0) {
		if (copy_to_user(userbuf, buf, num*sizeof(struct __fdb_lan)))
			num = -EFAULT;
	}
	kfree(buf);

	return num;
}
#endif

static int add_del_if(struct net_bridge *br, int ifindex, int isadd)
{
	struct net_device *dev;
	int ret;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	dev = dev_get_by_index(ifindex);
	if (dev == NULL)
		return -EINVAL;

	if (isadd)
		ret = br_add_if(br, dev);
	else
		ret = br_del_if(br, dev);

	dev_put(dev);
	return ret;
}

/*start of ATP bridge bind, z37589, 20070814*/
#ifdef CONFIG_ATP_BRIDGE_BIND
static int add_del_group(struct net_bridge *br, unsigned short groupid, int isadd)
{
	int ret;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;
	
	if (isadd)
		ret = br_add_group(br, groupid);
	else
		ret = br_del_group(br, groupid);

	return ret;
}
static int add_del_group_ports(struct net_bridge *br,  unsigned short groupid,
                                      unsigned short pts_mask, int isadd)
{
	int ret;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;
	
	if (isadd)
		ret = br_add_group_ports(br, groupid, pts_mask);
	else
		ret = br_del_group_ports(br, groupid, pts_mask);

	return ret;
}
#endif
/*end bridge bind, z37589, 20070814*/

/*
 * Legacy ioctl's through SIOCDEVPRIVATE
 * This interface is deprecated because it was too difficult to
 * to do the translation for 32/64bit ioctl compatability.
 */
static int old_dev_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct net_bridge *br = netdev_priv(dev);
	unsigned long args[4];

	if (copy_from_user(args, rq->ifr_data, sizeof(args)))
		return -EFAULT;

	switch (args[0]) {
	case BRCTL_ADD_IF:
	case BRCTL_DEL_IF:
		return add_del_if(br, args[1], args[0] == BRCTL_ADD_IF);
	/*start bridge bind, z37589, 20070814*/
#ifdef CONFIG_ATP_BRIDGE_BIND
	case BRCTL_ADD_GROUP :
    case BRCTL_DEL_GROUP :
        return add_del_group(br, (unsigned short)args[1], args[0] == BRCTL_ADD_GROUP);
    case BRCTL_ADD_GROUP_PORTS :
    case BRCTL_DEL_GROUP_PORTS :
        return add_del_group_ports(br, (unsigned short)args[1], (unsigned short)args[2], 
                               args[0] == BRCTL_ADD_GROUP_PORTS);
    case BRCTL_SET_GROUPFLAG:
        return br_set_groupflag(br, (unsigned char)args[1]);
    case BRCTL_SET_GROUPMACLN: 
        return br_set_groupmacln(br, (unsigned short)args[1], (unsigned char)args[2]);
    case BRCTL_GET_GROUP:
    {
        struct bridgebind_group *pgroup;
        struct net_bridge_port *p;
        
        pgroup = br_get_group(br, (unsigned short)args[1]);
        
        if(NULL == pgroup)
        {
            int i, isnull = 1;
            
            /*如果找不到指定的group,就将所有的group显示出来*/
            printk("\n=========bridge %s============\n", br->dev->name);
            printk("group_enable      : %d\n", br->group_enable);
            printk("===============================\n");
            for (i = 0; i < BR_BRIDGEBIND_GROUP_MAX; i++)
            {
                list_for_each_entry_rcu(pgroup, &br->group_table[i], list)
                {
                    isnull = 1; //重新对一个组遍历端口要把标识位置为1
                    
                    printk("group %d, mac_learn %d: ", pgroup->groupid, pgroup->macln_enable);
                    list_for_each_entry_rcu(p, &br->port_list, list)
                    {
                        if(pgroup->ports & ((unsigned short)1 << p->port_no))
                        {
                            isnull = 0;
                            break; //只要发现组里面至少有一个port就可以                       
                        }                        
                    }
                    
                    if(isnull) //组里没有端口号
                    {
                        printk("\n   (null)\n");
                        printk("*******************************\n");
                        continue;
                    }
                    else //组里有端口号
                    {
                        printk("\n");
                    }
                    
                    list_for_each_entry_rcu(p, &br->port_list, list)
                    {
                        if(pgroup->ports & ((unsigned short)1 << p->port_no))
                        {
                            printk("   port_name: %s, ", p->dev->name); 
                            printk("port_no: %d\n", p->port_no);                            
                        }
                    }
                    printk("*******************************\n");
                }
            }
            return -ENXIO; /*ENXIO表示是内核态来打印*/
        }
        else //有对应的组，则只打印对应组信息
        {
            int i, isnull = 1;
            
            /*如果找不到指定的group,就将所有的group显示出来*/
            printk("\n=========bridge %s============\n", br->dev->name);
            printk("group_enable      : %d\n", br->group_enable);
            printk("===============================\n");
            
            printk("group %d, mac_learn %d: ", pgroup->groupid, pgroup->macln_enable);
            list_for_each_entry_rcu(p, &br->port_list, list)
            {
                if(pgroup->ports & ((unsigned short)1 << p->port_no))
                {
                    isnull = 0;
                    break; //只要发现组里面至少有一个port就可以                      
                }                        
            }
            
            if(isnull) //组里没有端口号
            {
                printk("\n   (null)\n");
                printk("*******************************\n");

                br_release_group(pgroup);
                
                return -ENXIO; /*ENXIO表示是内核态来打印*/
            }
            else //组里有端口号
            {
                printk("\n");
            }
            
            list_for_each_entry_rcu(p, &br->port_list, list)
            {
                if(pgroup->ports & ((unsigned short)1 << p->port_no))
                {
                    printk("   port_name: %s, ", p->dev->name); 
                    printk("port_no: %d\n", p->port_no);                            
                }                        
            }
            printk("*******************************\n");

            br_release_group(pgroup);
            
            return -ENXIO; /*ENXIO表示是内核态来打印*/
        }
#if 0   /*目前由内核态来打印，后续改成用户态打印要把组下的端口号传回来*/
		if (copy_to_user((void __user *)args[2], &(pgroup->ports), sizeof(pgroup->ports)))
	    {
	        br_release_group( pgroup );
			return -EFAULT;
	    }
#endif        
        br_release_group(pgroup);
        
        return 0; /*0表示是内核态传值回来，由用户态来打印*/
    } 
#endif
	/*end bridge bind, z37589, 20070814*/


	case BRCTL_GET_BRIDGE_INFO:
	{
		struct __bridge_info b;

		memset(&b, 0, sizeof(struct __bridge_info));
		rcu_read_lock();
		memcpy(&b.designated_root, &br->designated_root, 8);
		memcpy(&b.bridge_id, &br->bridge_id, 8);
		b.root_path_cost = br->root_path_cost;
		b.max_age = jiffies_to_clock_t(br->max_age);
		b.hello_time = jiffies_to_clock_t(br->hello_time);
		b.forward_delay = br->forward_delay;
		b.bridge_max_age = br->bridge_max_age;
		b.bridge_hello_time = br->bridge_hello_time;
		b.bridge_forward_delay = jiffies_to_clock_t(br->bridge_forward_delay);
		b.topology_change = br->topology_change;
		b.topology_change_detected = br->topology_change_detected;
		b.root_port = br->root_port;
		b.stp_enabled = br->stp_enabled;
		b.ageing_time = jiffies_to_clock_t(br->ageing_time);
		b.hello_timer_value = br_timer_value(&br->hello_timer);
		b.tcn_timer_value = br_timer_value(&br->tcn_timer);
		b.topology_change_timer_value = br_timer_value(&br->topology_change_timer);
		b.gc_timer_value = br_timer_value(&br->gc_timer);
		rcu_read_unlock();

		if (copy_to_user((void __user *)args[1], &b, sizeof(b)))
			return -EFAULT;

		return 0;
	}

	case BRCTL_GET_PORT_LIST:
	{
		int num, *indices;

		num = args[2];
		if (num < 0)
			return -EINVAL;
		if (num == 0)
			num = 256;
		if (num > BR_MAX_PORTS)
			num = BR_MAX_PORTS;

		indices = kcalloc(num, sizeof(int), GFP_KERNEL);
		if (indices == NULL)
			return -ENOMEM;

		get_port_ifindices(br, indices, num);
		if (copy_to_user((void __user *)args[1], indices, num*sizeof(int)))
			num =  -EFAULT;
		kfree(indices);
		return num;
	}

	case BRCTL_SET_BRIDGE_FORWARD_DELAY:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		spin_lock_bh(&br->lock);
		br->bridge_forward_delay = clock_t_to_jiffies(args[1]);
		if (br_is_root_bridge(br))
			br->forward_delay = br->bridge_forward_delay;
		spin_unlock_bh(&br->lock);
		return 0;

	case BRCTL_SET_BRIDGE_HELLO_TIME:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		spin_lock_bh(&br->lock);
		br->bridge_hello_time = clock_t_to_jiffies(args[1]);
		if (br_is_root_bridge(br))
			br->hello_time = br->bridge_hello_time;
		spin_unlock_bh(&br->lock);
		return 0;

	case BRCTL_SET_BRIDGE_MAX_AGE:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		spin_lock_bh(&br->lock);
		br->bridge_max_age = clock_t_to_jiffies(args[1]);
		if (br_is_root_bridge(br))
			br->max_age = br->bridge_max_age;
		spin_unlock_bh(&br->lock);
		return 0;

	case BRCTL_SET_AGEING_TIME:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		br->ageing_time = clock_t_to_jiffies(args[1]);
		return 0;

	case BRCTL_GET_PORT_INFO:
	{
		struct __port_info p;
		struct net_bridge_port *pt;

		rcu_read_lock();
		if ((pt = br_get_port(br, args[2])) == NULL) {
			rcu_read_unlock();
			return -EINVAL;
		}

		memset(&p, 0, sizeof(struct __port_info));
		memcpy(&p.designated_root, &pt->designated_root, 8);
		memcpy(&p.designated_bridge, &pt->designated_bridge, 8);
		p.port_id = pt->port_id;
		p.designated_port = pt->designated_port;
		p.path_cost = pt->path_cost;
		p.designated_cost = pt->designated_cost;
		p.state = pt->state;
		p.top_change_ack = pt->topology_change_ack;
		p.config_pending = pt->config_pending;
		p.message_age_timer_value = br_timer_value(&pt->message_age_timer);
		p.forward_delay_timer_value = br_timer_value(&pt->forward_delay_timer);
		p.hold_timer_value = br_timer_value(&pt->hold_timer);

		rcu_read_unlock();

		if (copy_to_user((void __user *)args[1], &p, sizeof(p)))
			return -EFAULT;

		return 0;
	}

	case BRCTL_SET_BRIDGE_STP_STATE:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		br->stp_enabled = args[1]?1:0;
		return 0;

	case BRCTL_SET_BRIDGE_PRIORITY:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		spin_lock_bh(&br->lock);
		br_stp_set_bridge_priority(br, args[1]);
		spin_unlock_bh(&br->lock);
		return 0;

	case BRCTL_SET_PORT_PRIORITY:
	{
		struct net_bridge_port *p;
		int ret = 0;

		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		if (args[2] >= (1<<(16-BR_PORT_BITS)))
			return -ERANGE;

		spin_lock_bh(&br->lock);
		if ((p = br_get_port(br, args[1])) == NULL)
			ret = -EINVAL;
		else
			br_stp_set_port_priority(p, args[2]);
		spin_unlock_bh(&br->lock);
		return ret;
	}

#ifdef CONFIG_ATP_LAN_VLAN
    case BRCTL_SET_VLAN_ID:
    {
       if (!capable(CAP_NET_ADMIN))
       {
           return -EPERM;
       }
       unsigned int ulOpt = 0;
       int ret = 0;
       struct net_bridge_port *p;
       
       if (!strcmp(args[2],"tag"))
       {
           ulOpt = TYPE_VLAN_TAG;
       }
       else if (!strcmp(args[2],"untag"))
       {
           ulOpt = TYPE_VLAN_UNTAG;
       }
       else if (!strcmp(args[2],"pvid"))
       {
           ulOpt = TYPE_VLAN_PVID;
       }
       else
       {
        //   printk("Get args[2]:%s====>Error.....\r\n",args[2]);
           return -EPERM;
       }

       spin_lock_bh(&br->lock);
       if ((p = br_get_port(br, args[1])) == NULL)
        {
         //   printk("Get Port is Error..........\r\n");
			ret = -EINVAL;
        }
		else
		{
			br_vlan_set_by_port(p, ulOpt,args[3]);
		}
		spin_unlock_bh(&br->lock);  
		
		return ret;
    }
    case BRCTL_DEL_VLAN_ID:
    {
       if (!capable(CAP_NET_ADMIN))
       {
           return -EPERM;
       }
       unsigned int ulOpt = 0;
       int ret = 0;
       struct net_bridge_port *p;
       
       if (!strcmp(args[2],"tag"))
       {
           ulOpt = TYPE_VLAN_TAG;
       }
       else if (!strcmp(args[2],"untag"))
       {
           ulOpt = TYPE_VLAN_UNTAG;
       }
       else if (!strcmp(args[2],"pvid"))
       {
           ulOpt = TYPE_VLAN_PVID;
       }
       else
       {
           return -EPERM;
       }
       
       spin_lock_bh(&br->lock);
       if ((p = br_get_port(br, args[1])) == NULL)
        {
			ret = -EINVAL;
        }
		else
		{
			br_vlan_del_by_port(p, ulOpt,args[3]);
		}
		spin_unlock_bh(&br->lock);  
		
		return ret;
    }
    case BRCTL_SHOW_VLAN_ID:
    {
        spin_lock_bh(&br->lock);
        if (args[1] == 0)
        {
            br->vlan_enable = 0;
        }
        else if (args[1] == 1)
        {
            br->vlan_enable = 1;
        }
        else if (args[1] == 2)
        {
            br_vlan_show(br);
        }
        else if (args[1] == 3)
        {
            br_vlan_debug();
        }
        spin_unlock_bh(&br->lock);  
        return 0;
    }    
#endif
/*start of ATP 2008.01.16 增加 for DHCP端口绑定二层透传 by c60023298 */       
#ifdef CONFIG_ATP_BRIDGE_BIND
#ifdef CONFIG_ATP_BR_BIND_RELAY
     case  BRCTL_SET_BIND_RELAY:
    {
       if (!capable(CAP_NET_ADMIN))
       {
               return -EPERM;
       }
        int iEnable = args[2];
        br_add_bind_relay(args[1],iEnable);
          return 0;
    }
    case  BRCTL_DEL_BIND_RELAY:
    {
       if (!capable(CAP_NET_ADMIN))
       {
               return -EPERM;
       }
        int flag = args[2];
        br_del_bind_relay(args[1]);
          return 0;
    }
#endif
#endif
/*end of ATP 2008.01.16 增加 for DHCP端口绑定二层透传 by c60023298 */       
/*start of ATP 2008.01.16 增加 for 显示二层透传内核信息 by c60023298 */
#if defined(CONFIG_ATP_BR_DEV_RELAY) || defined(CONFIG_ATP_BRIDGE_BIND)
    case BRCTL_SHOW_RELAY:
    {
        if (!capable(CAP_NET_ADMIN))
        {
               return -EPERM;
        }
        br_get_showrelay(args[1]);
        return 0;
    }
#endif
/*end of ATP 2008.01.16 增加 for 显示二层透传内核信息 by c60023298 */
	case BRCTL_SET_PATH_COST:
	{
		struct net_bridge_port *p;
		int ret = 0;

		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		spin_lock_bh(&br->lock);
		if ((p = br_get_port(br, args[1])) == NULL)
			ret = -EINVAL;
		else
			br_stp_set_path_cost(p, args[2]);
		spin_unlock_bh(&br->lock);
		return ret;
	}

	case BRCTL_GET_FDB_ENTRIES:
		return get_fdb_entries(br, (void __user *)args[1], 
				       args[2], args[3]);
#ifdef CONFIG_ATP_BRIDGE_BIND
    case BRCTL_GET_LAN_NAME:
        return get_br_lan_name(br,(void __user *)args[1], 
				       args[2], args[3]);
#endif
	}

	return -EOPNOTSUPP;
}

static int old_deviceless(void __user *uarg)
{
	unsigned long args[3];

	if (copy_from_user(args, uarg, sizeof(args)))
		return -EFAULT;

	switch (args[0]) {
	case BRCTL_GET_VERSION:
		return BRCTL_VERSION;

	case BRCTL_GET_BRIDGES:
	{
		int *indices;
		int ret = 0;

		if (args[2] >= 2048)
			return -ENOMEM;
		indices = kcalloc(args[2], sizeof(int), GFP_KERNEL);
		if (indices == NULL)
			return -ENOMEM;

		args[2] = get_bridge_ifindices(indices, args[2]);

		ret = copy_to_user((void __user *)args[1], indices, args[2]*sizeof(int))
			? -EFAULT : args[2];

		kfree(indices);
		return ret;
	}

	case BRCTL_ADD_BRIDGE:
	case BRCTL_DEL_BRIDGE:
	{
		char buf[IFNAMSIZ];

		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		if (copy_from_user(buf, (void __user *)args[1], IFNAMSIZ))
			return -EFAULT;

		buf[IFNAMSIZ-1] = 0;

		if (args[0] == BRCTL_ADD_BRIDGE)
			return br_add_bridge(buf);

		return br_del_bridge(buf);
	}
#ifdef CONFIG_IGMP_SNOOPING
	case BRCTL_SHOW_IGMP_SNOOPING:
	{
		br_igmp_snooping_show();
		return 0;
	}
	case BRCTL_SET_IGMP_SNOOPING:
	{
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		br_igmp_snooping_set_enable((int)args[1]);
		return 0;
	}
    case BRCTL_FAST_BRIDGING:
    {
        if (!capable(CAP_NET_ADMIN))
        {
			return -EPERM;
        }
        /* 查看状态 */
        if (((int)args[1] != 0) && ((int)args[1] != 1))
        {
            printk("\nFast bridging status %d.\n", g_FastBridging);
            return 0;
        }

        /* 设置状态 */
        g_FastBridging = (int)args[1];
        printk("\nFast bridging set %d.\n", g_FastBridging); 
#ifdef CONFIG_BRIDGE_NETFILTER
        set_br_netfilter_call_iptables((int)args[1]);
#endif
        return 0;
    }
#endif
/*start of ATP 2008.01.16 增加 for DHCP设备类型二层透传 by c60023298 */
#ifdef CONFIG_ATP_BR_DEV_RELAY
    case BRCTL_SET_DEV_RELAY:
    {
    	if (!capable(CAP_NET_ADMIN))
			return -EPERM;
       int iIndex = args[1];
       int iEnable = args[2];
        g_devrelay.iEnable[iIndex] = iEnable ;
		return 0;
    }
#endif
/*end of ATP 2008.01.16 增加 for DHCP设备类型二层透传 by c60023298 */
	}

	return -EOPNOTSUPP;
}

int br_ioctl_deviceless_stub(unsigned int cmd, void __user *uarg)
{
	switch (cmd) {
	case SIOCGIFBR:
	case SIOCSIFBR:
		return old_deviceless(uarg);
		
	case SIOCBRADDBR:
	case SIOCBRDELBR:
	{
		char buf[IFNAMSIZ];

		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		if (copy_from_user(buf, uarg, IFNAMSIZ))
			return -EFAULT;

		buf[IFNAMSIZ-1] = 0;
		if (cmd == SIOCBRADDBR)
			return br_add_bridge(buf);

		return br_del_bridge(buf);
	}
#ifdef CONFIG_ATP_SUPPORT_HG553
           /*start of 增加添加内核brctl option60的值 by s53329 at  20070731*/
        case SIOCBRADDDHCPOPTION:
         {
            if (copy_from_user(dhcpoption, uarg, DHCP_OPTION))
                return -EFAULT;
              return 0;
          }
            /*end  of 增加添加内核brctl option60的值 by s53329 at  20070731*/
        /* BEGIN: Modified by w00135358, 2010/01/27: bug112 of ip phone arp HG553V100R001*/
        case SIOCBRADDDHCPVLAN:
        {
            char buf[IFNAMSIZ];
            if (copy_from_user(buf, uarg, sizeof(buf)))
                return -EFAULT;
              dhcpIpphoneVlan = simple_strtoul(buf, NULL, 10);
              return 0;
        }
        /* END: Modified by w00135358, 2010/01/27: bug112 of ip phone arp HG553V100R001*/
#endif /* CONFIG_ATP_SUPPORT_HG553 */
	}
	return -EOPNOTSUPP;
}

int br_dev_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct net_bridge *br = netdev_priv(dev);

	switch(cmd) {
	case SIOCDEVPRIVATE:
		return old_dev_ioctl(dev, rq, cmd);

	case SIOCBRADDIF:
	case SIOCBRDELIF:
		return add_del_if(br, rq->ifr_ifindex, cmd == SIOCBRADDIF);

	}

	pr_debug("Bridge does not support ioctl 0x%x\n", cmd);
	return -EOPNOTSUPP;
}
