/*
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: if_bridge.h,v 1.1 2010/07/06 06:18:40 c65985 Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#ifndef _LINUX_IF_BRIDGE_H
#define _LINUX_IF_BRIDGE_H

#include <linux/types.h>

#define SYSFS_BRIDGE_ATTR	"bridge"
#define SYSFS_BRIDGE_FDB	"brforward"
#define SYSFS_BRIDGE_PORT_SUBDIR "brif"
#define SYSFS_BRIDGE_PORT_ATTR	"brport"
#define SYSFS_BRIDGE_PORT_LINK	"bridge"

#define BRCTL_VERSION 1

#define BRCTL_GET_VERSION 0
#define BRCTL_GET_BRIDGES 1
#define BRCTL_ADD_BRIDGE 2
#define BRCTL_DEL_BRIDGE 3
#define BRCTL_ADD_IF 4
#define BRCTL_DEL_IF 5
#define BRCTL_GET_BRIDGE_INFO 6
#define BRCTL_GET_PORT_LIST 7
#define BRCTL_SET_BRIDGE_FORWARD_DELAY 8
#define BRCTL_SET_BRIDGE_HELLO_TIME 9
#define BRCTL_SET_BRIDGE_MAX_AGE 10
#define BRCTL_SET_AGEING_TIME 11
#define BRCTL_SET_GC_INTERVAL 12
#define BRCTL_GET_PORT_INFO 13
#define BRCTL_SET_BRIDGE_STP_STATE 14
#define BRCTL_SET_BRIDGE_PRIORITY 15
#define BRCTL_SET_PORT_PRIORITY 16
#define BRCTL_SET_PATH_COST 17
#define BRCTL_GET_FDB_ENTRIES 18
/*start vlan bridge, s60000658, 20060627*/
#define BRCTL_ADD_VLAN 25
#define BRCTL_DEL_VLAN 26
#define BRCTL_GET_VLAN 27
#define BRCTL_ADD_VLAN_PORTS 28
#define BRCTL_DEL_VLAN_PORTS 29
#define BRCTL_SET_PORT_MODE  30
#define BRCTL_SET_PVID 31
#define BRCTL_SET_PRIO 32
#define BRCTL_SET_VLANFLAG 33
#define BRCTL_SET_MNGRVID 34
#define BRCTL_SET_MACLN 35
/*end vlan bridge, s60000658, 20060627*/
/*start of 增加组播mac控制功能 by s53329  at  20070802*/
#define BRCTL_ADD_MCCHECK   36
#define BRCTL_DEL_MCCHECK    37
/*end  of 增加组播mac控制功能 by s53329  at  20070802*/
/* 桥接性能优化 */
#define BRCTL_FAST_BRIDGING    20

#define BRCTL_SET_IGMP_SNOOPING 23
#define BRCTL_SHOW_IGMP_SNOOPING 24

#ifdef CONFIG_ATP_LAN_VLAN
#define BRCTL_SET_VLAN_ID      30
#define BRCTL_DEL_VLAN_ID      31
#define BRCTL_SHOW_VLAN_ID     32
#endif

/*start of ATP bridge bind, z37589, 20070814*/
#if defined(CONFIG_ATP_BRIDGE_BIND) || defined(SUPPORT_BRIDGE_BIND)
#define BRCTL_ADD_GROUP 50
#define BRCTL_DEL_GROUP 51
#define BRCTL_GET_GROUP 52
#define BRCTL_ADD_GROUP_PORTS 53
#define BRCTL_DEL_GROUP_PORTS 54
#define BRCTL_SET_GROUPFLAG 55
#define BRCTL_SET_GROUPMACLN 56
#define BRCTL_GET_LAN_NAME 78
#endif
/*end of ATP bridge bind, z37589, 20070814*/


#define BR_STATE_DISABLED 0
#define BR_STATE_LISTENING 1
#define BR_STATE_LEARNING 2
#define BR_STATE_FORWARDING 3
#define BR_STATE_BLOCKING 4

/*start of ATP 2008.01.16 增加 for DHCP端口绑定二层透传 by c60023298 */       
#if defined(CONFIG_ATP_BRIDGE_BIND) || defined(SUPPORT_BRIDGE_BIND)
#if defined(CONFIG_ATP_BR_BIND_RELAY) || defined(SUPPORT_BR_BIND_RELAY)
#define BRCTL_SET_BIND_RELAY 38
#define BRCTL_DEL_BIND_RELAY 39
#endif
#endif
/*end of ATP 2008.01.16 增加 for DHCP端口绑定二层透传 by c60023298 */      

/*start of ATP 2008.01.16 增加 for DHCP设备类型二层透传 by c60023298 */
#if defined(CONFIG_ATP_BR_DEV_RELAY) || defined(SUPPORT_BR_DEV_RELAY)
#define BRCTL_SET_DEV_RELAY 40
#endif
/*end of ATP 2008.01.16 增加 for DHCP设备类型二层透传 by c60023298 */

/*start of ATP 2008.01.16 增加 for 显示二层透传内核信息 by c60023298 */
#if (defined(CONFIG_ATP_BR_DEV_RELAY) || defined(CONFIG_ATP_BRIDGE_BIND)) \
    || (defined(SUPPORT_BR_DEV_RELAY) || defined(SUPPORT_BRIDGE_BIND))
#define BRCTL_SHOW_RELAY 42
#endif
/*end of ATP 2008.01.16 增加 for 显示二层透传内核信息 by c60023298 */

struct __bridge_info
{
	__u64 designated_root;
	__u64 bridge_id;
	__u32 root_path_cost;
	__u32 max_age;
	__u32 hello_time;
	__u32 forward_delay;
	__u32 bridge_max_age;
	__u32 bridge_hello_time;
	__u32 bridge_forward_delay;
	__u8 topology_change;
	__u8 topology_change_detected;
	__u8 root_port;
	__u8 stp_enabled;
	__u32 ageing_time;
	__u32 gc_interval;
	__u32 hello_timer_value;
	__u32 tcn_timer_value;
	__u32 topology_change_timer_value;
	__u32 gc_timer_value;
};

struct __port_info
{
	__u64 designated_root;
	__u64 designated_bridge;
	__u16 port_id;
	__u16 designated_port;
	__u32 path_cost;
	__u32 designated_cost;
	__u8 state;
	__u8 top_change_ack;
	__u8 config_pending;
	__u8 unused0;
	__u32 message_age_timer_value;
	__u32 forward_delay_timer_value;
	__u32 hold_timer_value;
};

struct __fdb_entry
{
	__u8 mac_addr[6];
	__u8 port_no;
	__u8 is_local;
	__u32 ageing_timer_value;
	__u32 unused;
};

/*start of ATP add by z67625*/
#if defined(CONFIG_ATP_BRIDGE_BIND) || defined(SUPPORT_BRIDGE_BIND)
struct __fdb_lan
{
    __u8 mac_addr[6];
    __u8 port_no;
    char lanname[16];
};
#endif
/*end of ATP add by z67625*/


#ifdef __KERNEL__

#include <linux/netdevice.h>

extern void brioctl_set(int (*ioctl_hook)(unsigned int, void __user *));
extern int (*br_handle_frame_hook)(struct net_bridge_port *p, struct sk_buff **pskb);
extern int (*br_should_route_hook)(struct sk_buff **pskb);

#endif

#endif
