#ifndef _XT_MAC_RANGE_H
#define _XT_MAC_RANGE_H

#define XT_MACRANGE_SRC  0x100
#define XT_MACRANGE_DST  0x200
#define XT_MACRANGE_SRC_INV 0x1000
#define XT_MACRANGE_DST_INV 0x2000


struct xt_macrange {
    unsigned char min_mac[ETH_ALEN];
    unsigned char max_mac[ETH_ALEN];
};

struct xt_macrange_info
{
    struct xt_macrange src;
    struct xt_macrange dst;

    u_int8_t flags;
};
#endif /*_IPT_MAC_H*/
