#ifndef __IPT_VLAN_H
#define __IPT_VLAN_H



#ifndef __KERNEL__

#define ATP_VLAN_ORIG           0x1
#define ATP_VLAN_SET_ID         0x10
#define ATP_VLAN_SET_PRIO       0x20
#define ATP_VLAN_SET_ALL        (ATP_VLAN_SET_ID | ATP_VLAN_SET_PRIO)
#define ATP_HAVE_VLAN_TAG   (ATP_VLAN_ORIG | ATP_VLAN_SET_ALL)

#endif



struct ipt_vlan_j_info {
    unsigned short usVlanid;
    unsigned char  ucVlanprio;
    unsigned short ulVlanTag;
    unsigned short vlan_flags;
};

#endif
