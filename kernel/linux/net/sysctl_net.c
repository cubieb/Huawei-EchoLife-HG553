/* -*- linux-c -*-
 * sysctl_net.c: sysctl interface to net subsystem.
 *
 * Begun April 1, 1996, Mike Shaver.
 * Added /proc/sys/net directories for each protocol family. [MS]
 *
 * $Log: sysctl_net.c,v $
 * Revision 1.1  2010/07/06 06:45:08  c65985
 * 【变更分类】
 * 【问题单号】
 * 【问题描述】
 * 问题描述：以太网上行加速版本基线C06B015
 * 【修改说明】
 *
 * Revision 1.1  2008/11/03 11:57:40  y42304
 * *** empty log message ***
 *
 * Revision 1.2  1996/05/08  20:24:40  shaver
 * Added bits for NET_BRIDGE and the NET_IPV4_ARP stuff and
 * NET_IPV4_IP_FORWARD.
 *
 *
 */

#include <linux/mm.h>
#include <linux/sysctl.h>

#include <net/sock.h>

#ifdef CONFIG_INET
#include <net/ip.h>
#endif

#ifdef CONFIG_NET
#include <linux/if_ether.h>
#endif

#ifdef CONFIG_TR
#include <linux/if_tr.h>
#endif

struct ctl_table net_table[] = {
	{
		.ctl_name	= NET_CORE,
		.procname	= "core",
		.mode		= 0555,
		.child		= core_table,
	},
#ifdef CONFIG_INET
	{
		.ctl_name	= NET_IPV4,
		.procname	= "ipv4",
		.mode		= 0555,
		.child		= ipv4_table
	},
#endif
#ifdef CONFIG_TR
	{
		.ctl_name	= NET_TR,
		.procname	= "token-ring",
		.mode		= 0555,
		.child		= tr_table,
	},
#endif
	{ 0 },
};
