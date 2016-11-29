/*
 *	Userspace interface
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_if.c,v 1.1 2010/07/06 06:45:11 c65985 Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/ethtool.h>
#include <linux/if_arp.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/rtnetlink.h>
#include <linux/if_ether.h>
#include <net/sock.h>

#include "br_private.h"

#ifdef CONFIG_IGMP_SNOOPING
#include "br_igmp_snooping.h"
#endif

/*
 * Determine initial path cost based on speed.
 * using recommendations from 802.1d standard
 *
 * Need to simulate user ioctl because not all device's that support
 * ethtool, use ethtool_ops.  Also, since driver might sleep need to
 * not be holding any locks.
 */
static int port_cost(struct net_device *dev)
{
	struct ethtool_cmd ecmd = { ETHTOOL_GSET };
	struct ifreq ifr;
	mm_segment_t old_fs;
	int err;

	strncpy(ifr.ifr_name, dev->name, IFNAMSIZ);
	ifr.ifr_data = (void __user *) &ecmd;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	err = dev_ethtool(&ifr);
	set_fs(old_fs);
	
	if (!err) {
		switch(ecmd.speed) {
		case SPEED_100:
			return 19;
		case SPEED_1000:
			return 4;
		case SPEED_10000:
			return 2;
		case SPEED_10:
			return 100;
		default:
			pr_info("bridge: can't decode speed from %s: %d\n",
				dev->name, ecmd.speed);
			return 100;
		}
	}

	/* Old silly heuristics based on name */
	if (!strncmp(dev->name, "lec", 3))
		return 7;

	if (!strncmp(dev->name, "plip", 4))
		return 2500;

	return 100;	/* assume old 10Mbps */
}


/*
 * Check for port carrier transistions.
 * Called from work queue to allow for calling functions that
 * might sleep (such as speed check), and to debounce.
 */
void br_port_carrier_check(struct net_bridge_port *p)
{
	struct net_device *dev = p->dev;
	struct net_bridge *br = p->br;

	if (netif_carrier_ok(dev))
		p->path_cost = port_cost(dev);

	if (netif_running(br->dev)) {
		spin_lock_bh(&br->lock);
		if (netif_carrier_ok(dev)) {
			if (p->state == BR_STATE_DISABLED)
				br_stp_enable_port(p);
		} else {
			if (p->state != BR_STATE_DISABLED)
				br_stp_disable_port(p);
		}
		spin_unlock_bh(&br->lock);
	}
}

static void release_nbp(struct kobject *kobj)
{
	struct net_bridge_port *p
		= container_of(kobj, struct net_bridge_port, kobj);
	kfree(p);
}

static struct kobj_type brport_ktype = {
#ifdef CONFIG_SYSFS
	.sysfs_ops = &brport_sysfs_ops,
#endif
	.release = release_nbp,
};

static void destroy_nbp(struct net_bridge_port *p)
{
	struct net_device *dev = p->dev;

	p->br = NULL;
	p->dev = NULL;
	dev_put(dev);

	kobject_put(&p->kobj);
}

static void destroy_nbp_rcu(struct rcu_head *head)
{
	struct net_bridge_port *p =
			container_of(head, struct net_bridge_port, rcu);
	destroy_nbp(p);
}

/* Delete port(interface) from bridge is done in two steps.
 * via RCU. First step, marks device as down. That deletes
 * all the timers and stops new packets from flowing through.
 *
 * Final cleanup doesn't occur until after all CPU's finished
 * processing packets.
 *
 * Protected from multiple admin operations by RTNL mutex
 */
static void del_nbp(struct net_bridge_port *p)
{
	struct net_bridge *br = p->br;
	struct net_device *dev = p->dev;

	sysfs_remove_link(&br->ifobj, dev->name);

	dev_set_promiscuity(dev, -1);

	spin_lock_bh(&br->lock);
	br_stp_disable_port(p);
	spin_unlock_bh(&br->lock);

	br_fdb_delete_by_port(br, p, 1);

	list_del_rcu(&p->list);

	rcu_assign_pointer(dev->br_port, NULL);

	kobject_uevent(&p->kobj, KOBJ_REMOVE);
	kobject_del(&p->kobj);
	
	call_rcu(&p->rcu, destroy_nbp_rcu);
}

/* called with RTNL */
static void del_br(struct net_bridge *br)
{
	struct net_bridge_port *p, *n;

 #ifdef CONFIG_IGMP_SNOOPING
	br_igmp_snooping_clear();
 #endif
/*start of ATP bridge bind, z37589, 20070814*/
#ifdef CONFIG_ATP_BRIDGE_BIND
    struct bridgebind_group *gp, *gn;
	int j;

	for (j = 0; j < BR_BRIDGEBIND_GROUP_MAX; j++)
    {
        list_for_each_entry_safe(gp, gn, &br->group_table[j], list) {
    		if(atomic_read(&(gp->__refcnt)))
            {
                atomic_set(&(gp->dirty), 1);
            }
            else
            {
                spin_lock_bh(&br->glock);
                list_del_rcu(&gp->list);
                kfree((void*)gp);
                spin_unlock_bh(&br->glock);
            }
    	}
    }
#endif
/*end of ATP bridge bind, z37589, 20070814*/
    
	list_for_each_entry_safe(p, n, &br->port_list, list) {
		del_nbp(p);
	}

	del_timer_sync(&br->gc_timer);

	br_sysfs_delbr(br->dev);
 	unregister_netdevice(br->dev);
}

static struct net_device *new_bridge_dev(const char *name)
{
	struct net_bridge *br;
	struct net_device *dev;
    /*start bridge bind, z37589, 20070814*/
#ifdef CONFIG_ATP_BRIDGE_BIND
    int i;
#endif
    /*end bridge bind, z37589, 20070814*/

	dev = alloc_netdev(sizeof(struct net_bridge), name,
			   br_dev_setup);
	
	if (!dev)
		return NULL;

	br = netdev_priv(dev);
	br->dev = dev;

	spin_lock_init(&br->lock);
	INIT_LIST_HEAD(&br->port_list);
	spin_lock_init(&br->hash_lock);

	br->bridge_id.prio[0] = 0x80;
	br->bridge_id.prio[1] = 0x00;

	memcpy(br->group_addr, br_group_address, ETH_ALEN);

	br->feature_mask = dev->features;
	br->stp_enabled = 0;
	br->designated_root = br->bridge_id;
	br->root_path_cost = 0;
	br->root_port = 0;
	br->bridge_max_age = br->max_age = 20 * HZ;
	br->bridge_hello_time = br->hello_time = 2 * HZ;
	br->bridge_forward_delay = br->forward_delay = 15 * HZ;
	br->topology_change = 0;
	br->topology_change_detected = 0;
	br->ageing_time = 300 * HZ;
	INIT_LIST_HEAD(&br->age_list);

	br_stp_timer_init(br);

/*start of ATP bridge bind, z37589, 20070814*/
#ifdef CONFIG_ATP_BRIDGE_BIND
    for(i = 0; i < BR_BRIDGEBIND_GROUP_MAX; i++)
    {
        INIT_LIST_HEAD(&br->group_table[i]);
    }
    /*组功能只有某一个WAN启用绑定才开启，没有任何一个WAN启用绑定则关闭组功能以免影响性能*/
	br->group_enable  = 0; 
#endif
/*end of ATP bridge bind, z37589, 20070814*/
	
#ifdef CONFIG_ATP_LAN_VLAN
    br->vlan_enable = 0;
#endif

	return dev;
}

/* find an available port number */
static int find_portno(struct net_bridge *br)
{
	int index;
	struct net_bridge_port *p;
	unsigned long *inuse;

	inuse = kcalloc(BITS_TO_LONGS(BR_MAX_PORTS), sizeof(unsigned long),
			GFP_KERNEL);
	if (!inuse)
		return -ENOMEM;

	set_bit(0, inuse);	/* zero is reserved */
	list_for_each_entry(p, &br->port_list, list) {
		set_bit(p->port_no, inuse);
	}
	index = find_first_zero_bit(inuse, BR_MAX_PORTS);
	kfree(inuse);

	return (index >= BR_MAX_PORTS) ? -EXFULL : index;
}

/* called with RTNL but without bridge lock */
static struct net_bridge_port *new_nbp(struct net_bridge *br, 
				       struct net_device *dev)
{
	int index;
	struct net_bridge_port *p;
	
	index = find_portno(br);
	if (index < 0)
		return ERR_PTR(index);

	p = kzalloc(sizeof(*p), GFP_KERNEL);
	if (p == NULL)
		return ERR_PTR(-ENOMEM);

	p->br = br;
	dev_hold(dev);
	p->dev = dev;
	p->path_cost = port_cost(dev);
 	p->priority = 0x8000 >> BR_PORT_BITS;
	p->port_no = index;
	br_init_port(p);
	p->state = BR_STATE_DISABLED;
	br_stp_port_timer_init(p);

	kobject_init(&p->kobj);
	kobject_set_name(&p->kobj, SYSFS_BRIDGE_PORT_ATTR);
	p->kobj.ktype = &brport_ktype;
	p->kobj.parent = &(dev->dev.kobj);
	p->kobj.kset = NULL;

	return p;
}

int br_add_bridge(const char *name)
{
	struct net_device *dev;
	int ret;

	dev = new_bridge_dev(name);
	if (!dev) 
		return -ENOMEM;

	rtnl_lock();
	if (strchr(dev->name, '%')) {
		ret = dev_alloc_name(dev, dev->name);
		if (ret < 0) {
			free_netdev(dev);
			goto out;
		}
	}

	ret = register_netdevice(dev);
	if (ret)
		goto out;

	ret = br_sysfs_addbr(dev);
	if (ret)
		unregister_netdevice(dev);
 out:
	rtnl_unlock();
	return ret;
}

int br_del_bridge(const char *name)
{
	struct net_device *dev;
	int ret = 0;

	rtnl_lock();
	dev = __dev_get_by_name(name);
	if (dev == NULL) 
		ret =  -ENXIO; 	/* Could not find device */

	else if (!(dev->priv_flags & IFF_EBRIDGE)) {
		/* Attempt to delete non bridge device! */
		ret = -EPERM;
	}

	else if (dev->flags & IFF_UP) {
		/* Not shutdown yet. */
		ret = -EBUSY;
	} 

	else 
		del_br(netdev_priv(dev));

	rtnl_unlock();
	return ret;
}

/* MTU of the bridge pseudo-device: ETH_DATA_LEN or the minimum of the ports */
int br_min_mtu(const struct net_bridge *br)
{
	const struct net_bridge_port *p;
	int mtu = 0;

	ASSERT_RTNL();

	if (list_empty(&br->port_list))
		mtu = ETH_DATA_LEN;
	else {
		list_for_each_entry(p, &br->port_list, list) {
			if (!mtu  || p->dev->mtu < mtu)
				mtu = p->dev->mtu;
		}
	}
	return mtu;
}

/*
 * Recomputes features using slave's features
 */
void br_features_recompute(struct net_bridge *br)
{
	struct net_bridge_port *p;
	unsigned long features, checksum;

	checksum = br->feature_mask & NETIF_F_ALL_CSUM ? NETIF_F_NO_CSUM : 0;
	features = br->feature_mask & ~NETIF_F_ALL_CSUM;

	list_for_each_entry(p, &br->port_list, list) {
		unsigned long feature = p->dev->features;

		if (checksum & NETIF_F_NO_CSUM && !(feature & NETIF_F_NO_CSUM))
			checksum ^= NETIF_F_NO_CSUM | NETIF_F_HW_CSUM;
		if (checksum & NETIF_F_HW_CSUM && !(feature & NETIF_F_HW_CSUM))
			checksum ^= NETIF_F_HW_CSUM | NETIF_F_IP_CSUM;
		if (!(feature & NETIF_F_IP_CSUM))
			checksum = 0;

		if (feature & NETIF_F_GSO)
			feature |= NETIF_F_GSO_SOFTWARE;
		feature |= NETIF_F_GSO;

		features &= feature;
	}

	if (!(checksum & NETIF_F_ALL_CSUM))
		features &= ~NETIF_F_SG;
	if (!(features & NETIF_F_SG))
		features &= ~NETIF_F_GSO_MASK;

	br->dev->features = features | checksum | NETIF_F_LLTX |
			    NETIF_F_GSO_ROBUST;
}

/* called with RTNL */
int br_add_if(struct net_bridge *br, struct net_device *dev)
{
	struct net_bridge_port *p;
	int err = 0;

	if (dev->flags & IFF_LOOPBACK || dev->type != ARPHRD_ETHER)
		return -EINVAL;

	if (dev->hard_start_xmit == br_dev_xmit)
		return -ELOOP;

	if (dev->br_port != NULL)
		return -EBUSY;

	p = new_nbp(br, dev);
	if (IS_ERR(p))
		return PTR_ERR(p);

	err = kobject_add(&p->kobj);
	if (err)
		goto err0;

	err = br_fdb_insert(br, p, dev->dev_addr);
	if (err)
		goto err1;

	err = br_sysfs_addif(p);
	if (err)
		goto err2;

	rcu_assign_pointer(dev->br_port, p);
		dev_set_promiscuity(dev, 1);

		list_add_rcu(&p->list, &br->port_list);

		spin_lock_bh(&br->lock);
		br_stp_recalculate_bridge_id(br);
	br_features_recompute(br);

	if ((dev->flags & IFF_UP) && netif_carrier_ok(dev) &&
	    (br->dev->flags & IFF_UP))
			br_stp_enable_port(p);
		spin_unlock_bh(&br->lock);

		dev_set_mtu(br->dev, br_min_mtu(br));

	kobject_uevent(&p->kobj, KOBJ_ADD);

	return 0;
err2:
	br_fdb_delete_by_port(br, p, 1);
err1:
	kobject_del(&p->kobj);
err0:
	kobject_put(&p->kobj);
	return err;
}

/* called with RTNL */
int br_del_if(struct net_bridge *br, struct net_device *dev)
{
	struct net_bridge_port *p = dev->br_port;
	
	if (!p || p->br != br) 
		return -EINVAL;

	del_nbp(p);

	spin_lock_bh(&br->lock);
	br_stp_recalculate_bridge_id(br);
	br_features_recompute(br);
	spin_unlock_bh(&br->lock);

	return 0;
}

/*start bridge bind, z37589, 20070814*/
#ifdef CONFIG_ATP_BRIDGE_BIND
/*如果get成功, 使用完毕后一定要调用br_release_group*/
__inline__ struct bridgebind_group* br_get_group(struct net_bridge *br, unsigned short groupid)
{
    struct bridgebind_group *pgroup;
    
    list_for_each_entry_rcu(pgroup, &br->group_table[GROUP_HASH_KEY(groupid)], list)
    {
        if(pgroup->groupid == groupid)
        {
            atomic_inc(&(pgroup->__refcnt));
            return pgroup;
        }
    }

    return NULL;
}

__inline__ void br_release_group(struct bridgebind_group *pgroup)
{    
    atomic_dec(&(pgroup->__refcnt));
    if(atomic_read( &(pgroup->__refcnt) ) <= 0 && atomic_read( &(pgroup->dirty) ))
    {
        spin_lock_bh(&pgroup->lock);
        kfree((void*)pgroup);
        spin_unlock_bh(&pgroup->lock);
    }
    
}

int br_add_group(struct net_bridge *br, unsigned short groupid)
{
    struct bridgebind_group *pgroup;
    
    if(NULL != (pgroup = br_get_group(br, groupid)))
    {
        br_release_group(pgroup);
        return -EINVAL;
    }

    pgroup = (struct bridgebind_group *)kmalloc(sizeof(*pgroup), GFP_KERNEL);
    
    if(NULL != pgroup)
    {
        if(DEBUG_VBR)
            printk("BRGROUP(kenl): add group %d, br %s, file %s, line %d\n",
               groupid, br->dev->name, __FILE__, __LINE__);
        
        memset(pgroup, 0, sizeof(struct bridgebind_group));
        pgroup->groupid = groupid;
        spin_lock_bh(&br->glock);
        list_add_rcu(&pgroup->list, &br->group_table[GROUP_HASH_KEY(groupid)]);
        spin_unlock_bh(&br->glock);
        return 0;
    }
    
    return -EINVAL;
}

int br_del_group(struct net_bridge *br, unsigned short groupid)
{
    struct bridgebind_group *pgroup = NULL;
    
    list_for_each_entry_rcu(pgroup, &br->group_table[GROUP_HASH_KEY(groupid)], list)
    {
        if(pgroup->groupid == groupid)
        {   
            spin_lock_bh(&br->glock);
            list_del_rcu(&pgroup->list);
            INIT_LIST_HEAD(&pgroup->list);
            spin_unlock_bh(&br->glock);
            if(atomic_read(&(pgroup->__refcnt)) > 0)
            {
                if(DEBUG_VBR)
                    printk("BRGROUP(kenl): del group %d to dirty, br %s, file %s, line %d\n",
                           groupid, br->dev->name, __FILE__, __LINE__);
                atomic_set(&(pgroup->dirty), 1);
            }
            else
            {
                if(DEBUG_VBR)
                    printk("BRGROUP(kenl): del group %d drastically, br %s, file %s, line %d\n",
                           groupid, br->dev->name, __FILE__, __LINE__);
                spin_lock_bh(&br->glock);                
                kfree((void*)pgroup);
                spin_unlock_bh(&br->glock);
            }

            break;
        }
    }
    return 0;
}

int br_add_group_ports(struct net_bridge *br, unsigned short groupid, unsigned short pts_mask)
{
    struct bridgebind_group *pgroup = NULL;

    if(NULL == (pgroup = br_get_group(br, groupid)) )
    {
        return -EINVAL;
    }
    
    if(DEBUG_VBR)
        printk("BRGROUP(kenl): add group %d ports 0x%x, br %s, file %s, line %d\n",
               groupid, pts_mask, br->dev->name, __FILE__, __LINE__);
    
    pgroup->ports |= pts_mask;
    br_release_group(pgroup);   

    return 0;
}

int br_del_group_ports(struct net_bridge *br, unsigned short groupid, unsigned short pts_mask)
{
    struct bridgebind_group *pgroup = NULL;

    if(NULL == (pgroup = br_get_group(br, groupid)) )
    {
        return -EINVAL;
    }

    if(DEBUG_VBR)
        printk("BRGROUP(kenl): del group %d ports 0x%x, br %s, file %s, line %d\n",
               groupid, pts_mask, br->dev->name, __FILE__, __LINE__);
    
    pgroup->ports &= ~pts_mask;
    br_release_group(pgroup);

    return 0;
}

int br_set_groupflag(struct net_bridge *br, unsigned char groupflag)
{
    if (!br) 
		return -EINVAL;
    
	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

    if(DEBUG_VBR)
            printk("BRGROUP(kenl): set group %s  in %s,  file %s, line %d\n",
                   (groupflag==1)?"enable":"disable", br->dev->name, __FILE__, __LINE__);
    
    br->group_enable = groupflag;
    
	return 0;
}

int br_set_groupmacln(struct net_bridge *br, unsigned short groupid, unsigned char enable)
{
    struct bridgebind_group *pgroup = NULL;
    
    if (!br) 
		return -EINVAL;
    
	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

    pgroup = br_get_group(br, groupid);

    if(NULL == pgroup)
        return -EINVAL;

    if(DEBUG_VBR)
            printk("BRGROUP(kenl): set group %d macln %d, in %s,  file %s, line %d\n",
                   groupid, enable, br->dev->name, __FILE__, __LINE__);  
    
    pgroup->macln_enable = enable;
    br_release_group(pgroup);
    
	return 0;
}

/*得到报文接收或者发送端口属于的组号*/
unsigned short brgetgroup(struct net_bridge *br, struct sk_buff *skb)
{
    struct bridgebind_group *pgroup = NULL;
    int port_no = -1;
    unsigned short mask = 0;      
    unsigned short port_mask = 0; //根据MASK转换出来后移位得出
    struct net_bridge_port *p;
    int i = 0;

    /*先找出收或者发报文的端口设备名*/
    list_for_each_entry_rcu(p, &br->port_list, list)
    {
        if(!strcmp(skb->dev->name, p->dev->name))
        {
            port_no = p->port_no;
            break;
        }
    }

    /*查找失败直接返回*/
    if(port_no == -1)
    {
        return 0;//找不到则默认在0组中；因为一个端口只要UP则肯定在某个组中
    }

    mask |= (unsigned short)1 << port_no;
    port_mask = mask;

    for(i = 0; i < BR_BRIDGEBIND_GROUP_MAX; i++)
    {
        list_for_each_entry_rcu(pgroup, &br->group_table[GROUP_HASH_KEY(i)], list)
        {
            if(pgroup->ports & port_mask) //不为0说明在一个组中
            {
                return pgroup->groupid;
            }
        }
    }

    return 0; //找不到则默认在0组中；因为一个端口只要UP则肯定在某个组中
}

/*得到报文接收属于的组号*/
unsigned short brgetgroupbyname(struct net_bridge *br, char *pcName)
{
    struct bridgebind_group *pgroup = NULL;
    int port_no = -1;
    unsigned short mask = 0;      
    unsigned short port_mask = 0; //根据MASK转换出来后移位得出
    struct net_bridge_port *p;
    int i = 0;

    /*先找出收报文的端口设备名*/
    list_for_each_entry_rcu(p, &br->port_list, list)
    {
        if(!strcmp(pcName, p->dev->name))
        {
            port_no = p->port_no;
            break;
        }
    }

    /*查找失败直接返回*/
    if(port_no == -1)
    {
        return 0;//找不到则默认在0组中；因为一个端口只要UP则肯定在某个组中
    }

    mask |= (unsigned short)1 << port_no;
    port_mask = mask;

    for(i = 0; i < BR_BRIDGEBIND_GROUP_MAX; i++)
    {
        list_for_each_entry_rcu(pgroup, &br->group_table[GROUP_HASH_KEY(i)], list)
        {
            if(pgroup->ports & port_mask) //不为0说明在一个组中
            {
                return pgroup->groupid;
            }
        }
    }

    return 0; //找不到则默认在0组中；因为一个端口只要UP则肯定在某个组中
}
#endif
/*end bridge bind, z37589, 20070814*/

void __exit br_cleanup_bridges(void)
{
	struct net_device *dev, *nxt;

	rtnl_lock();
	for (dev = dev_base; dev; dev = nxt) {
		nxt = dev->next;
		if (dev->priv_flags & IFF_EBRIDGE)
			del_br(dev->priv);
	}
	rtnl_unlock();

}
