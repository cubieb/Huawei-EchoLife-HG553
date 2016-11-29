#ifndef __BR_VLAN_H
#define __BR_VLAN_H

#include <linux/netdevice.h>
#include <linux/miscdevice.h>
#include <linux/if_bridge.h>

#define VLAN_ID_NUM_MAX 8

enum
{
    /*��ʼ��VLAN ID*/
    LAN_VLAN_ID_INIT = 1,

    /*���͵ı�����Ҫ����VLAN ID*/
    LAN_VLAN_ID_TAG,

    /*���͵ı�����Ҫȥ��VLAN ID*/
    LAN_VLAN_ID_REMOVE,

    /*���ط��͵ı��ģ�����ҪVLAN ID*/
    LAN_VLAN_ID_LOCAL_OUT,

    /*�������VLAN ID*/
    LAN_VLAN_ID_OTERH
    
};

enum
{
    /*tag ���*/
    TYPE_VLAN_TAG,

    /*untag ���*/
    TYPE_VLAN_UNTAG,

    /*pvid ���*/
    TYPE_VLAN_PVID,

    /*����*/
    TYPE_VLAN_OTHER
};

struct port_vlanInfo
{
    unsigned short tag[VLAN_ID_NUM_MAX];         //VLAN TAG
    unsigned short untag[VLAN_ID_NUM_MAX];       // VLAN UNTAG
    unsigned short pvid;                         //VLAN PVID
    unsigned short reserved;                     // Ԥ����Ϊ���ֽڶ���
};

void br_vlan_init_port(struct net_bridge_port *port);

void br_vlan_set_by_port(struct net_bridge_port *port,unsigned int ulOpt,unsigned int value);

void br_vlan_del_by_port(struct net_bridge_port *port,unsigned int ulOpt,unsigned int value);

void br_vlan_init(void);

void br_vlan_fini(void);

void br_vlan_debug(void);

int br_vlan_xmit_in(struct sk_buff *skb);

#endif
