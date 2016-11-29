#ifndef _IPT_TOS_H
#define _IPT_TOS_H


#define IPT_TOS_MAX_VALUE   255

struct ipt_tos_range_info {
    u_int8_t tos_min;
    u_int8_t tos_max;
    u_int8_t invert;
};

#ifndef IPTOS_NORMALSVC
#define IPTOS_NORMALSVC 0
#endif

#endif /*_IPT_TOS_H*/
