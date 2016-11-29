#ifndef __IPT_VLAN_H
#define __IPT_VLAN_H

//#define VLAN_MATCH_VLAN_ID_VALUE   0x1
#define VLAN_MATCH_VLAN_ID_RANGE   0x10
//#define VLAN_MATCH_VLAN_PRIO_VALUE  0x10
#define VLAN_MATCH_VLAN_PRIO_RANGE  0x20

#define IPT_VLAN_ID_INV     0x01
#define IPT_VLAN_PRIO_INV     0x02

struct ipt_vlan_info {
	u_int16_t ulVlanId[2];
	u_int16_t  ulVlanPrio[2];
    u_int8_t  ulinvflags;
};

#endif
