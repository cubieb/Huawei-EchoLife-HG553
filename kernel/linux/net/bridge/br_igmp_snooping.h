
#ifndef __BR_IGMP_SNOOPING_H
#define __BR_IGMP_SNOOPING_H

#include <linux/netdevice.h>
#include <linux/miscdevice.h>
#include <linux/if_bridge.h>

#define IGMP_TIMER_CHECK_TIMEOUT 10
#define IGMP_QUERY_TIMEOUT 150

#ifndef ETH_ALEN
#define ETH_ALEN	6		/* Octets in one ethernet addr	 */
#endif

#define BR_IGMP_SNOOPING_DISABLE 0
#define BR_IGMP_SNOOPING_ENABLE 1

typedef union ipv4
{
    unsigned int ulIpAddr;
    unsigned char acIpAddr[4];
}igmp_ipaddr;

struct net_bridge_igmp_snooping_entry
{
    struct list_head 		  list;
	struct net_bridge_port	  *dev_dst;             /*Ŀ���Ŷ˿ڶ�Ӧ�ĵ�ַ����eth0.**/
	unsigned char			  grp_mac[ETH_ALEN];  /*�鲥��ַ��Ӧ��MAC��ַ����01:00:5e:xx:xx:xx*/
	unsigned char			  host_mac[ETH_ALEN];   /*PC����MAC��ַ*/
	unsigned long			  time;                  /*��ʱ��*/
};

struct net_bridge_igmp_snooping
{
	struct list_head		igmp_list;         /*����ͷ*/
	struct timer_list 		igmp_timer;        /*��ʱ��*/
	int		                igmp_snooping_enable;         /*0:��ʾIGMP Snooping����*/
	spinlock_t		        igmp_lock;         /*������*/
	int		 	            igmp_start_timer; /*��ʱ���Ƿ����ı�־*/
};

extern  void  br_igmp_snooping_clear(void);
extern  int   br_igmp_snooping_forward(struct sk_buff *skb, struct net_bridge *br,unsigned char *dest,int forward);
extern  void  br_igmp_snooping_set_enable(int enable);
extern  void  br_igmp_snooping_show(void);
extern  void  br_igmp_snooping_init(void);

#endif
