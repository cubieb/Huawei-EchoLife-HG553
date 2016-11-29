#ifndef _IPT_MAC_H
#define _IPT_MAC_H

#include <linux/netfilter/xt_mac_range.h>


#define MACRANGE_SRC  		XT_MACRANGE_SRC
#define MACRANGE_DST  		XT_MACRANGE_DST
#define MACRANGE_SRC_INV 	XT_MACRANGE_SRC_INV
#define MACRANGE_DST_INV 	XT_MACRANGE_DST_INV

#define ipt_macrange xt_macrange
#define ipt_macrange_info xt_macrange_info


#endif /*_IPT_MAC_H*/
