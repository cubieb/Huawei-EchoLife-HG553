/*
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_private.h,v 1.1 2010/07/06 06:45:25 c65985 Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#ifndef _BR_PRIVATE_H
#define _BR_PRIVATE_H

#include <linux/netdevice.h>
#include <linux/if_bridge.h>
#include <linux/igmp.h>
#include <linux/in.h>

#define BR_HASH_BITS 8
#define BR_HASH_SIZE (1 << BR_HASH_BITS)

#define BR_HOLD_TIME (1*HZ)

#define BR_PORT_BITS	10
#define BR_MAX_PORTS	(1<<BR_PORT_BITS)

#define BR_PORT_DEBOUNCE (HZ/10)
#ifdef CONFIG_ATP_LAN_VLAN
#include "br_vlan.h"
#endif
#define BR_VERSION	"2.2"

/*start of ATP bridge bind, z37589, 20070814*/
#ifdef CONFIG_ATP_BRIDGE_BIND
#ifndef DEBUG_VBR
#define  DEBUG_VBR 0
#endif
#define BR_BRIDGEBIND_GROUP_MAX 16 
#define GROUP_HASH_KEY(groupid) ((groupid)%BR_BRIDGEBIND_GROUP_MAX)
#define ISVALID_GROUPID(groupid)   (((groupid) < 0xF) && ((groupid) >= 0))
#define ISIN_GROUP(group_ports, port) ((group_ports) & ((unsigned short)1 << (port)))
#endif
/*end of ATP bridge bind, z37589, 20070814*/

/* start 64 byte can not reach 15M in vdsl A36D06384 by f00110348 20090331 */
extern int g_FastBridging;
/* end 64 byte can not reach 15M in vdsl A36D06384 by f00110348 20090331 */
extern int sysctl_fast_routing;

typedef struct bridge_id bridge_id;
typedef struct mac_addr mac_addr;
typedef __u16 port_id;

/*start of ATP bridge bind, z37589, 20070814*/
#ifdef CONFIG_ATP_BRIDGE_BIND
struct bridgebind_group
{
	struct list_head	list;
    spinlock_t			lock;
	atomic_t		    __refcnt;
	atomic_t            dirty;
	unsigned short      groupid;
	unsigned short      ports; /*端口索引port_no(低位起)对应的
	                            位是1表示端口在group_table中*/ 
	unsigned char       macln_enable;/*mac learning*/
	unsigned char       reserved1;
	unsigned short      reserved2;
};
#endif
/*end bridge bind, z37589, 20070814*/

struct bridge_id
{
	unsigned char	prio[2];
	unsigned char	addr[6];
};

struct mac_addr
{
	unsigned char	addr[6];
};

struct net_bridge_fdb_entry
{
	struct hlist_node		hlist;
	struct net_bridge_port		*dst;

	struct rcu_head			rcu;
	atomic_t			use_count;
	unsigned long			ageing_timer;
	mac_addr			addr;
	unsigned char			is_local;
	unsigned char			is_static;
};

struct net_bridge_port
{
	struct net_bridge		*br;
	struct net_device		*dev;
	struct list_head		list;
    
	/* STP */
	u8				priority;
	u8				state;
	u16				port_no;
	unsigned char			topology_change_ack;
	unsigned char			config_pending;
	port_id				port_id;
	port_id				designated_port;
	bridge_id			designated_root;
	bridge_id			designated_bridge;
	u32				path_cost;
	u32				designated_cost;
	int		 		dirty;

	struct timer_list		forward_delay_timer;
	struct timer_list		hold_timer;
	struct timer_list		message_age_timer;
	struct kobject			kobj;
	struct rcu_head			rcu;
	
#ifdef CONFIG_ATP_LAN_VLAN
    struct port_vlanInfo       vlaninfo;
#endif

};

struct net_bridge
{
	spinlock_t			lock;
	struct list_head		port_list;
	struct net_device		*dev;
	struct net_device_stats		statistics;
	spinlock_t			hash_lock;
	struct hlist_head		hash[BR_HASH_SIZE];
	struct list_head		age_list;
	unsigned long			feature_mask;


#ifdef CONFIG_ATP_LAN_VLAN
    int         vlan_enable;     //VLAN ID 全局开关
#endif
/*start of ATP bridge bind, z37589, 20070814*/
#ifdef CONFIG_ATP_BRIDGE_BIND
    spinlock_t        glock;
    /*group_table以BR_BRIDGEBIND_GROUP_MAX求余算出来相同的值就存在一个表中，
    以list形式串起来，比如1和17求余BR_BRIDGEBIND_GROUP_MAX后值相同就都存放
    在group_table[1]中，由于目前组号只支持0-15，因此每个组表里其实只有一个
    组号，后续可以扩展组号超出16个*/
    struct list_head  group_table[BR_BRIDGEBIND_GROUP_MAX];
    unsigned char     group_enable;
    unsigned char     reserved2;
    unsigned short    reserved1;
#endif
/*end of ATP bridge bind, z37589, 20070814*/

	/* STP */
	bridge_id			designated_root;
	bridge_id			bridge_id;
	u32				root_path_cost;
	unsigned long			max_age;
	unsigned long			hello_time;
	unsigned long			forward_delay;
	unsigned long			bridge_max_age;
	unsigned long			ageing_time;
	unsigned long			bridge_hello_time;
	unsigned long			bridge_forward_delay;

	u8				group_addr[ETH_ALEN];
	u16				root_port;
	unsigned char			stp_enabled;
	unsigned char			topology_change;
	unsigned char			topology_change_detected;

	struct timer_list		hello_timer;
	struct timer_list		tcn_timer;
	struct timer_list		topology_change_timer;
	struct timer_list		gc_timer;
	struct kobject			ifobj;
};

extern struct notifier_block br_device_notifier;
extern const u8 br_group_address[ETH_ALEN];

/* called under bridge lock */
static inline int br_is_root_bridge(const struct net_bridge *br)
{
	return !memcmp(&br->bridge_id, &br->designated_root, 8);
}


/* br_device.c */
extern void br_dev_setup(struct net_device *dev);
extern int br_dev_xmit(struct sk_buff *skb, struct net_device *dev);

/* br_fdb.c */
extern void br_fdb_init(void);
extern void br_fdb_fini(void);
extern void br_fdb_changeaddr(struct net_bridge_port *p,
			      const unsigned char *newaddr);
extern void br_fdb_cleanup(unsigned long arg);
extern void br_fdb_delete_by_port(struct net_bridge *br,
				  const struct net_bridge_port *p, int do_all);
extern struct net_bridge_fdb_entry *__br_fdb_get(struct net_bridge *br,
						 const unsigned char *addr);
extern struct net_bridge_fdb_entry *br_fdb_get(struct net_bridge *br,
					       unsigned char *addr);
extern void br_fdb_put(struct net_bridge_fdb_entry *ent);
extern int br_fdb_fillbuf(struct net_bridge *br, void *buf, 
			  unsigned long count, unsigned long off);
#ifdef CONFIG_ATP_BRIDGE_BIND
extern int br_fdb_filllanbuf(struct net_bridge *br, void *buf, 
			  unsigned long count, unsigned long off);
#endif
extern int br_fdb_insert(struct net_bridge *br,
			 struct net_bridge_port *source,
			 const unsigned char *addr);
extern void br_fdb_update(struct net_bridge *br,
			  struct net_bridge_port *source,
			  const unsigned char *addr);

/* br_forward.c */
extern void br_deliver(const struct net_bridge_port *to,
		struct sk_buff *skb);
extern int br_dev_queue_push_xmit(struct sk_buff *skb);
extern void br_forward(const struct net_bridge_port *to,
		struct sk_buff *skb);
extern int br_forward_finish(struct sk_buff *skb);
extern void br_flood_deliver(struct net_bridge *br,
		      struct sk_buff *skb,
		      int clone);
extern void br_flood_forward(struct net_bridge *br,
		      struct sk_buff *skb,
		      int clone);

/* br_if.c */
extern void br_port_carrier_check(struct net_bridge_port *p);
extern int br_add_bridge(const char *name);
extern int br_del_bridge(const char *name);
extern void br_cleanup_bridges(void);
extern int br_add_if(struct net_bridge *br,
	      struct net_device *dev);
extern int br_del_if(struct net_bridge *br,
	      struct net_device *dev);
extern int br_min_mtu(const struct net_bridge *br);
extern void br_features_recompute(struct net_bridge *br);

/*start of ATP bridge bind, z37589, 20070814*/
#ifdef CONFIG_ATP_BRIDGE_BIND
extern struct bridgebind_group* br_get_group(struct net_bridge *br, unsigned short groupid);
extern void br_release_group(struct bridgebind_group *pgroup);
extern int br_add_group(struct net_bridge *br, unsigned short groupid);
extern int br_del_group(struct net_bridge *br, unsigned short groupid);
extern int br_add_group_ports(struct net_bridge *br, unsigned short groupid, unsigned short pts_mask);
extern int br_del_group_ports(struct net_bridge *br, unsigned short groupid, unsigned short pts_mask);
extern int br_set_groupflag(struct net_bridge *br, unsigned char groupflag);
extern int br_set_groupmacln(struct net_bridge *br, unsigned short groupid, unsigned char enable);
extern unsigned short brgetgroup(struct net_bridge *br, struct sk_buff *skb);
extern unsigned short brgetgroupbyname(struct net_bridge *br, char *pcName);
#endif
/*end of ATP bridge bind, z37589, 20070814*/

/* br_input.c */
extern int br_handle_frame_finish(struct sk_buff *skb);
extern int br_handle_frame(struct net_bridge_port *p, struct sk_buff **pskb);

/* br_ioctl.c */
extern int br_dev_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
extern int br_ioctl_deviceless_stub(unsigned int cmd, void __user *arg);

/* br_netfilter.c */
#ifdef CONFIG_BRIDGE_NETFILTER
extern int br_netfilter_init(void);
extern void br_netfilter_fini(void);
extern void set_br_netfilter_call_iptables(int lEnable);
#else
#define br_netfilter_init()	(0)
#define br_netfilter_fini()	do { } while(0)
#endif

/* br_stp.c */
extern void br_log_state(const struct net_bridge_port *p);
extern struct net_bridge_port *br_get_port(struct net_bridge *br,
				    	   u16 port_no);
extern void br_init_port(struct net_bridge_port *p);
extern void br_become_designated_port(struct net_bridge_port *p);

/* br_stp_if.c */
extern void br_stp_enable_bridge(struct net_bridge *br);
extern void br_stp_disable_bridge(struct net_bridge *br);
extern void br_stp_enable_port(struct net_bridge_port *p);
extern void br_stp_disable_port(struct net_bridge_port *p);
extern void br_stp_recalculate_bridge_id(struct net_bridge *br);
extern void br_stp_change_bridge_id(struct net_bridge *br, const unsigned char *a);
extern void br_stp_set_bridge_priority(struct net_bridge *br,
				       u16 newprio);
extern void br_stp_set_port_priority(struct net_bridge_port *p,
				     u8 newprio);
extern void br_stp_set_path_cost(struct net_bridge_port *p,
				 u32 path_cost);
extern ssize_t br_show_bridge_id(char *buf, const struct bridge_id *id);

/* br_stp_bpdu.c */
extern int br_stp_rcv(struct sk_buff *skb, struct net_device *dev,
		      struct packet_type *pt, struct net_device *orig_dev);

/* br_stp_timer.c */
extern void br_stp_timer_init(struct net_bridge *br);
extern void br_stp_port_timer_init(struct net_bridge_port *p);
extern unsigned long br_timer_value(const struct timer_list *timer);

/* br.c */
extern struct net_bridge_fdb_entry *(*br_fdb_get_hook)(struct net_bridge *br,
						       unsigned char *addr);
extern void (*br_fdb_put_hook)(struct net_bridge_fdb_entry *ent);


/* br_netlink.c */
extern void br_netlink_init(void);
extern void br_netlink_fini(void);
extern void br_ifinfo_notify(int event, struct net_bridge_port *port);

#ifdef CONFIG_SYSFS
/* br_sysfs_if.c */
extern struct sysfs_ops brport_sysfs_ops;
extern int br_sysfs_addif(struct net_bridge_port *p);

/* br_sysfs_br.c */
extern int br_sysfs_addbr(struct net_device *dev);
extern void br_sysfs_delbr(struct net_device *dev);

#else

#define br_sysfs_addif(p)	(0)
#define br_sysfs_addbr(dev)	(0)
#define br_sysfs_delbr(dev)	do { } while(0)
#endif /* CONFIG_SYSFS */

#endif 
