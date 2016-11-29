/*
 * H.323 connection tracking helper
 *
 * Copyright (c) 2006 Jing Min Zhao <zhaojingmin@users.sourceforge.net>
 *
 * This source code is licensed under General Public License version 2.
 *
 * Based on the 'brute force' H.323 connection tracking module by
 * Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
 *
 * For more information, please see http://nath323.sourceforge.net/
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/ctype.h>
#include <linux/inet.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/skbuff.h>
#include <net/route.h>
#include <net/ip6_route.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_tuple.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_conntrack_ecache.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <linux/netfilter/nf_conntrack_h323.h>

#include <net/tcp.h>

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

/* Parameters */
static unsigned int default_rrq_ttl __read_mostly = 300;
module_param(default_rrq_ttl, uint, 0600);
MODULE_PARM_DESC(default_rrq_ttl, "use this TTL if it's missing in RRQ");

static int gkrouted_only __read_mostly = 1;
module_param(gkrouted_only, int, 0600);
MODULE_PARM_DESC(gkrouted_only, "only accept calls from gatekeeper");

static int callforward_filter __read_mostly = 1;
module_param(callforward_filter, bool, 0600);
MODULE_PARM_DESC(callforward_filter, "only create call forwarding expectations "
				     "if both endpoints are on different sides "
				     "(determined by routing information)");

/* Hooks for NAT */
int (*set_h245_addr_hook) (struct sk_buff **pskb,
			   unsigned char **data, int dataoff,
			   H245_TransportAddress *taddr,
			   union nf_conntrack_address *addr, __be16 port)
			   __read_mostly;
int (*set_h225_addr_hook) (struct sk_buff **pskb,
			   unsigned char **data, int dataoff,
			   TransportAddress *taddr,
			   union nf_conntrack_address *addr, __be16 port)
			   __read_mostly;
int (*set_sig_addr_hook) (struct sk_buff **pskb,
			  struct nf_conn *ct,
			  enum ip_conntrack_info ctinfo,
			  unsigned char **data,
			  TransportAddress *taddr, int count) __read_mostly;
int (*set_ras_addr_hook) (struct sk_buff **pskb,
			  struct nf_conn *ct,
			  enum ip_conntrack_info ctinfo,
			  unsigned char **data,
			  TransportAddress *taddr, int count) __read_mostly;
#if 0
int (*nat_rtp_rtcp_hook) (struct sk_buff **pskb,
			  struct nf_conn *ct,
			  enum ip_conntrack_info ctinfo,
			  unsigned char **data, int dataoff,
			  H245_TransportAddress *taddr,
			  __be16 port, __be16 rtp_port,
			  struct nf_conntrack_expect *rtp_exp,
			  struct nf_conntrack_expect *rtcp_exp) __read_mostly;
#else
int (*nat_rtp_rtcp_hook) (struct sk_buff **pskb,
			  struct nf_conn *ct,
			  enum ip_conntrack_info ctinfo,
			  unsigned char **data, int dataoff,
			  H245_TransportAddress *taddr,
			  __be16 port, __be16 rtp_port,
			  struct nf_conntrack_expect *rtp_exp,
			  struct nf_conntrack_expect *rtcp_exp,
			  unsigned int mediatype) __read_mostly;
#endif
int (*nat_t120_hook) (struct sk_buff **pskb,
		      struct nf_conn *ct,
		      enum ip_conntrack_info ctinfo,
		      unsigned char **data, int dataoff,
		      H245_TransportAddress *taddr, __be16 port,
		      struct nf_conntrack_expect *exp) __read_mostly;
int (*nat_h245_hook) (struct sk_buff **pskb,
		      struct nf_conn *ct,
		      enum ip_conntrack_info ctinfo,
		      unsigned char **data, int dataoff,
		      TransportAddress *taddr, __be16 port,
		      struct nf_conntrack_expect *exp) __read_mostly;
int (*nat_callforwarding_hook) (struct sk_buff **pskb,
				struct nf_conn *ct,
				enum ip_conntrack_info ctinfo,
				unsigned char **data, int dataoff,
				TransportAddress *taddr, __be16 port,
				struct nf_conntrack_expect *exp) __read_mostly;
int (*nat_q931_hook) (struct sk_buff **pskb,
		      struct nf_conn *ct,
		      enum ip_conntrack_info ctinfo,
		      unsigned char **data, TransportAddress *taddr, int idx,
		      __be16 port, struct nf_conntrack_expect *exp)
		      __read_mostly;

static DEFINE_SPINLOCK(nf_h323_lock);
static char *h323_buffer;

static struct nf_conntrack_helper nf_conntrack_helper_h245;
static struct nf_conntrack_helper nf_conntrack_helper_q931[];
static struct nf_conntrack_helper nf_conntrack_helper_ras[];
/*start of HGW by d37981  2009.4.17 HG522V100R001C08B115  A36D06508 */
extern int h323_enable;
/*e n d of HGW by d37981  2009.4.17 HG522V100R001C08B115  A36D06508 */


/* 这里可以考虑下该怎么处理那种 TCP 分片的报文， 但是这里完成修改之后，后续接下来的解码等都必须要处理好
 * 包括对应的setup中的解码也要完整处理好
 */
static int get_tpkt_data(struct sk_buff **pskb, unsigned int protoff,
			 struct nf_conn *ct, enum ip_conntrack_info ctinfo,
			 unsigned char **data, int *datalen, int *dataoff)
{
	struct nf_ct_h323_master *info = &nfct_help(ct)->help.ct_h323_info;
	int dir = CTINFO2DIR(ctinfo);
	struct tcphdr _tcph, *th;
	int tcpdatalen;
	int tcpdataoff;
	unsigned char *tpkt;
	int tpktlen;
	int tpktoff;

	/* Get TCP header */
	th = skb_header_pointer(*pskb, protoff, sizeof(_tcph), &_tcph);
	if (th == NULL)
		return 0;

	/* Get TCP data offset */
	tcpdataoff = protoff + th->doff * 4;

	/* Get TCP data length */
	tcpdatalen = (*pskb)->len - tcpdataoff;
	if (tcpdatalen <= 0)	/* No TCP data */
		goto clear_out;

	if (*data == NULL) {	/* first TPKT */
		/* Get first TPKT pointer */
		tpkt = skb_header_pointer(*pskb, tcpdataoff, tcpdatalen,
					  h323_buffer);
		BUG_ON(tpkt == NULL);

		/* Validate TPKT identifier 这种情况表示一个TPKT数据包被分为两部分了 */
		if (tcpdatalen < 4 || tpkt[0] != 0x03 || tpkt[1] != 0) {
			/* Netmeeting sends TPKT header and data separately */
			if (info->tpkt_len[dir] > 0) {
				DEBUGP("nf_ct_h323: previous packet "
				       "indicated separate TPKT data of %hu "
				       "bytes\n", info->tpkt_len[dir]);

                /* 一个完整的TPKT报文包含在包中 */
				if (info->tpkt_len[dir] <= tcpdatalen) {
					/* Yes, there was a TPKT header
					 * received */
					*data = tpkt;
					*datalen = info->tpkt_len[dir];
					*dataoff = 0;
					goto out;
				}

				/* Fragmented TPKT */
				{
					DEBUGP("nf_ct_h323: fragmented TPKT [%u.%u.%u.%u:%d]-->[%u.%u.%u.%u:%d], goto clear_out\n",
                        NIPQUAD((*pskb)->nh.iph->saddr),
                        ntohs(th->source),
                        NIPQUAD((*pskb)->nh.iph->daddr),
                        ntohs(th->dest));
				}
				goto clear_out;
			}

			/* It is not even a TPKT 这里表明不是一个完整的TPKT包，也就是一个信令可能封装在多个包中 
                  * 因此是否可以考虑这里进行处理不返回0， 而是给本函数扩展一个参数，然后在这里设置该参数的
                  * 值，同时，设置好对应的 data, datalen以及 dataoff 指针
                  */
			return 0;
		}
		tpktoff = 0;
	} else {		/* Next TPKT */
		tpktoff = *dataoff + *datalen;
		tcpdatalen -= tpktoff;
		if (tcpdatalen <= 4)	/* No more TPKT */
			goto clear_out;
		tpkt = *data + *datalen;

		/* Validate TPKT identifier */
		if (tpkt[0] != 0x03 || tpkt[1] != 0)
			goto clear_out;
	}

	/* Validate TPKT length */
	tpktlen = tpkt[2] * 256 + tpkt[3];
	if (tpktlen < 4)
		goto clear_out;
	if (tpktlen > tcpdatalen) {
		if (tcpdatalen == 4) {	/* Separate TPKT header */
			/* Netmeeting sends TPKT header and data separately */
			DEBUGP("nf_ct_h323: separate TPKT header indicates "
			       "there will be TPKT data of %hu bytes\n",
			       tpktlen - 4);
			info->tpkt_len[dir] = tpktlen - 4;
			return 0;
		}


        #if 0
		if (net_ratelimit())
			printk("nf_ct_h323: incomplete TPKT (fragmented?)\n");
		goto clear_out;
        #else
        DEBUGP("nf_ct_h323: fragmented TPKT [%u.%u.%u.%u:%d]-->[%u.%u.%u.%u:%d], but don't clear it\n",
            NIPQUAD((*pskb)->nh.iph->saddr),
            ntohs(th->source),
            NIPQUAD((*pskb)->nh.iph->daddr),
            ntohs(th->dest));
        #endif
	}

	/* This is the encapsulated data */
	*data = tpkt + 4;
	*datalen = tpktlen - 4;
	*dataoff = tpktoff + 4;

      out:
	/* Clear TPKT length */
	info->tpkt_len[dir] = 0;
	return 1;

      clear_out:
	info->tpkt_len[dir] = 0;
	return 0;
}

/****************************************************************************/

/* end add by d00107688 for tpkt fragement  */
#if 0
static int get_h245_addr(struct nf_conn *ct, unsigned char *data,
			 H245_TransportAddress *taddr,
			 union nf_conntrack_address *addr, __be16 *port)
#else
static int get_h245_addr(struct sk_buff **pskb, struct nf_conn *ct, unsigned char *data,
			 H245_TransportAddress *taddr,
			 union nf_conntrack_address *addr, __be16 *port)
#endif
/* end add by d00107688 for tpkt fragement  */
{
	unsigned char *p;
	int family = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num;
	int len;

	if (taddr->choice != eH245_TransportAddress_unicastAddress)
		return 0;

	switch (taddr->unicastAddress.choice) {
	case eUnicastAddress_iPAddress:
		if (family != AF_INET)
			return 0;
		p = data + taddr->unicastAddress.iPAddress.network;
		len = 4;
		break;
	case eUnicastAddress_iP6Address:
		if (family != AF_INET6)
			return 0;
		p = data + taddr->unicastAddress.iP6Address.network;
		len = 16;
		break;
	default:
		return 0;
	}

    /* start add by d00107688 for tpkt fragement  */
    if (((p + len) - (*pskb)->data) > (*pskb)->len)
    {
        DEBUGP("Get address error, packet has been fragemented");
        return 0;
    }
    /* end add by d00107688 for tpkt fragement  */

	memcpy(addr, p, len);
	memset((void *)addr + len, 0, sizeof(*addr) - len);
    
    /* start add by d00107688 for tpkt fragement */
    if (((p + len + sizeof(__be16)) - (*pskb)->data) > (*pskb)->len)
    {
        DEBUGP("Get port error, packet has been fragemented");
        *port = htons(0);
        return 0;
    }
    /* end add by d00107688 for tpkt fragement  */
    
	memcpy(port, p + len, sizeof(__be16));

	return 1;
}



/****************************************************************************/
#if 0
static int expect_rtp_rtcp(struct sk_buff **pskb, struct nf_conn *ct,
			   enum ip_conntrack_info ctinfo,
			   unsigned char **data, int dataoff,
			   H245_TransportAddress *taddr)
#else
static int expect_rtp_rtcp(struct sk_buff **pskb, struct nf_conn *ct,
			   enum ip_conntrack_info ctinfo,
			   unsigned char **data, int dataoff,
			   H245_TransportAddress *taddr, unsigned int mediatype)
#endif
{
	int dir = CTINFO2DIR(ctinfo);
	int ret = 0;
	__be16 port;
	__be16 rtp_port, rtcp_port;
	union nf_conntrack_address addr;
	struct nf_conntrack_expect *rtp_exp;
	struct nf_conntrack_expect *rtcp_exp;
	typeof(nat_rtp_rtcp_hook) nat_rtp_rtcp;

	/* Read RTP or RTCP address 如果报文中的地址与数据包的源地址一致，直接返回 */
#if  0 
	if (!get_h245_addr(ct, *data, taddr, &addr, &port) ||
	    memcmp(&addr, &ct->tuplehash[dir].tuple.src.u3, sizeof(addr)) ||
	    port == 0)
#else
    /* start add by d00107688 for tpkt fragement  */

    #if 0
	if (!get_h245_addr(ct, *data, taddr, &addr, &port) ||
	     !memcmp(&addr, &ct->tuplehash[!dir].tuple.dst.u3, sizeof(addr)) ||
	    port == 0)
    #else
	if (!get_h245_addr(pskb, ct, *data, taddr, &addr, &port) ||
	     !memcmp(&addr, &ct->tuplehash[!dir].tuple.dst.u3, sizeof(addr)) ||
	    port == 0)
    #endif
    
    /* end add by d00107688 for tpkt fragement  */
#endif
	{
        DEBUGP("src[%u.%u.%u.%u] to [%u.%u.%u.%u] mediaaddr [%u.%u.%u.%u] no need to create exp ",
            NIPQUAD(ct->tuplehash[dir].tuple.src.u3.ip), 
            NIPQUAD(ct->tuplehash[dir].tuple.dst.u3.ip),
            NIPQUAD(addr.ip));
		return 0;
	}

    DEBUGP("\r\n\r\nnow , begin to expect the rtp/rtcp channel......\r\n\r\n");

    /* start of bug fix by d00107688 2009-03-07 */
    #if 0
    if (!is_local_addr(addr))
    {
        DEBUGPC("mediaaddr [%u.%u.%u.%u] no local address, src[%u.%u.%u.%u] to [%u.%u.%u.%u]  no need to create exp ",
            NIPQUAD(addr.ip),
            NIPQUAD(ct->tuplehash[dir].tuple.src.u3.ip), 
            NIPQUAD(ct->tuplehash[dir].tuple.dst.u3.ip));
        return 0;
    }
    #endif
    /* end of bug fix by d00107688 2009-03-07 */

	/* RTP port is even */
	port &= htons(~1);
	rtp_port = port;
#if 1
    #if 0
        rtcp_port = htons(ntohs(port) + 1);
    #else
        rtcp_port = htons(port + 1);
    #endif
#else
    rtcp_port = htons(rtp_port + 1);
#endif

	/* Create expect for RTP */
	if ((rtp_exp = nf_conntrack_expect_alloc(ct)) == NULL)
		return -1;

#if 0
	nf_conntrack_expect_init(rtp_exp, ct->tuplehash[!dir].tuple.src.l3num,
				 &ct->tuplehash[!dir].tuple.src.u3,
				 &ct->tuplehash[!dir].tuple.dst.u3,
				 IPPROTO_UDP, NULL, &rtp_port);
#else
	nf_conntrack_expect_init(rtp_exp, ct->tuplehash[!dir].tuple.src.l3num,
				 NULL,
				 &ct->tuplehash[!dir].tuple.dst.u3,
				 IPPROTO_UDP, NULL, &rtp_port);
#endif

    DEBUGP("create rtp exp src:[%u.%u.%u.%u], dst[%u.%u.%u.%u] addr: [%u.%u.%u.%u] dstport: %d",
        NIPQUAD(ct->tuplehash[!dir].tuple.src.u3.ip), 
        NIPQUAD(ct->tuplehash[!dir].tuple.dst.u3.ip),
        NIPQUAD(addr.ip),
        port);


	/* Create expect for RTCP */
	if ((rtcp_exp = nf_conntrack_expect_alloc(ct)) == NULL) {
		nf_conntrack_expect_put(rtp_exp);
		return -1;
	}

        
#if 0
	nf_conntrack_expect_init(rtcp_exp, ct->tuplehash[!dir].tuple.src.l3num,
				 &ct->tuplehash[!dir].tuple.src.u3,
				 &ct->tuplehash[!dir].tuple.dst.u3,
				 IPPROTO_UDP, NULL, &rtcp_port);
#else
	nf_conntrack_expect_init(rtcp_exp, ct->tuplehash[!dir].tuple.src.l3num,
				 NULL,
				 &ct->tuplehash[!dir].tuple.dst.u3,
				 IPPROTO_UDP, NULL, &rtcp_port);
#endif

    DEBUGP("create rtcp exp src:[%u.%u.%u.%u], dst[%u.%u.%u.%u] addr: [%u.%u.%u.%u] dstport: %d",
        NIPQUAD(ct->tuplehash[!dir].tuple.src.u3.ip), 
        NIPQUAD(ct->tuplehash[!dir].tuple.dst.u3.ip),
        NIPQUAD(addr.ip), 
        rtcp_port);

    /* nat_rtp_rtcp 对应函数 nat_rtp_rtcp */
	if (memcmp(&ct->tuplehash[dir].tuple.src.u3,
		   &ct->tuplehash[!dir].tuple.dst.u3,
		   sizeof(ct->tuplehash[dir].tuple.src.u3)) &&
		   (nat_rtp_rtcp = rcu_dereference(nat_rtp_rtcp_hook)) &&
		   ct->status & IPS_NAT_MASK) {
		/* NAT needed */
#if 0
		ret = nat_rtp_rtcp(pskb, ct, ctinfo, data, dataoff,
				   taddr, port, rtp_port, rtp_exp, rtcp_exp);
#else
		ret = nat_rtp_rtcp(pskb, ct, ctinfo, data, dataoff,
				   taddr, port, rtp_port, rtp_exp, rtcp_exp, mediatype);
#endif
	} else {		/* Conntrack only */
		if (nf_conntrack_expect_related(rtp_exp) == 0) {
			if (nf_conntrack_expect_related(rtcp_exp) == 0) {
				DEBUGP("nf_ct_h323: expect RTP ");
				NF_CT_DUMP_TUPLE(&rtp_exp->tuple);
				DEBUGP("nf_ct_h323: expect RTCP ");
				NF_CT_DUMP_TUPLE(&rtcp_exp->tuple);
			} else {
				nf_conntrack_unexpect_related(rtp_exp);
				ret = -1;
			}
		} else
			ret = -1;
	}

	nf_conntrack_expect_put(rtp_exp);
	nf_conntrack_expect_put(rtcp_exp);

	return ret;
}

/****************************************************************************/
static int expect_t120(struct sk_buff **pskb,
		       struct nf_conn *ct,
		       enum ip_conntrack_info ctinfo,
		       unsigned char **data, int dataoff,
		       H245_TransportAddress *taddr)
{
	int dir = CTINFO2DIR(ctinfo);
	int ret = 0;
	__be16 port;
	union nf_conntrack_address addr;
	struct nf_conntrack_expect *exp;
	typeof(nat_t120_hook) nat_t120;

	/* Read T.120 address */    
    /* start add by d00107688 for tpkt fragement */
#if 0
	if (!get_h245_addr(ct, *data, taddr, &addr, &port) ||
	    memcmp(&addr, &ct->tuplehash[dir].tuple.src.u3, sizeof(addr)) ||
	    port == 0)
#else
    if (!get_h245_addr(pskb, ct, *data, taddr, &addr, &port) ||
    	    memcmp(&addr, &ct->tuplehash[dir].tuple.src.u3, sizeof(addr)) ||
    	    port == 0)
#endif
    /* end add by d00107688 for tpkt fragement  */

    {
		return 0;
	}

	/* Create expect for T.120 connections */
	if ((exp = nf_conntrack_expect_alloc(ct)) == NULL)
		return -1;
	nf_conntrack_expect_init(exp, ct->tuplehash[!dir].tuple.src.l3num,
				 &ct->tuplehash[!dir].tuple.src.u3,
				 &ct->tuplehash[!dir].tuple.dst.u3,
				 IPPROTO_TCP, NULL, &port);
	exp->flags = NF_CT_EXPECT_PERMANENT;	/* Accept multiple channels */

	if (memcmp(&ct->tuplehash[dir].tuple.src.u3,
		   &ct->tuplehash[!dir].tuple.dst.u3,
		   sizeof(ct->tuplehash[dir].tuple.src.u3)) &&
	    (nat_t120 = rcu_dereference(nat_t120_hook)) &&
	    ct->status & IPS_NAT_MASK) {
		/* NAT needed */
		ret = nat_t120(pskb, ct, ctinfo, data, dataoff, taddr,
			       port, exp);
	} else {		/* Conntrack only */
		if (nf_conntrack_expect_related(exp) == 0) {
			DEBUGP("nf_ct_h323: expect T.120 ");
			NF_CT_DUMP_TUPLE(&exp->tuple);
		} else
			ret = -1;
	}

	nf_conntrack_expect_put(exp);

	return ret;
}

/****************************************************************************/
static int process_h245_channel(struct sk_buff **pskb,
				struct nf_conn *ct,
				enum ip_conntrack_info ctinfo,
				unsigned char **data, int dataoff,
				H2250LogicalChannelParameters *channel)
{
	int ret;

    DEBUGP("PROCESS h245 channel");
    
	if (channel->options & eH2250LogicalChannelParameters_mediaChannel) {
		/* RTP */
#if 0
		ret = expect_rtp_rtcp(pskb, ct, ctinfo, data, dataoff,
				      &channel->mediaChannel);
#else
		ret = expect_rtp_rtcp(pskb, ct, ctinfo, data, dataoff,
				      &channel->mediaChannel, MEDIA_CHANNEL);
#endif
		if (ret < 0)
			return -1;
	}

	if (channel->options & eH2250LogicalChannelParameters_mediaControlChannel) {
        DEBUGP("process rtcp media control channel");
		/* RTCP */
#if 0
		ret = expect_rtp_rtcp(pskb, ct, ctinfo, data, dataoff,
				      &channel->mediaControlChannel);
#else
		ret = expect_rtp_rtcp(pskb, ct, ctinfo, data, dataoff,
				      &channel->mediaControlChannel, MEDIA_CONTROL_CHANNEL);
#endif
		if (ret < 0)
			return -1;
	}

	return 0;
}


/* 处理逻辑通道消息 */
static int process_olc(struct sk_buff **pskb, struct nf_conn *ct,
		       enum ip_conntrack_info ctinfo,
		       unsigned char **data, int dataoff,
		       OpenLogicalChannel *olc)
{
	int ret;

	DEBUGP("\r\n\r\nnf_ct_h323: OpenLogicalChannel\n");


    /* 一般来说走到这里 */
	if (olc->forwardLogicalChannelParameters.multiplexParameters.choice ==
	    eOpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters_h2250LogicalChannelParameters)
	{
		ret = process_h245_channel(pskb, ct, ctinfo, data, dataoff,
					   &olc->forwardLogicalChannelParameters.multiplexParameters.h2250LogicalChannelParameters);
		if (ret < 0)
			return -1;
	}

	if ((olc->options &
	     eOpenLogicalChannel_reverseLogicalChannelParameters) &&
	    (olc->reverseLogicalChannelParameters.options &
	     eOpenLogicalChannel_reverseLogicalChannelParameters_multiplexParameters)
	    && (olc->reverseLogicalChannelParameters.multiplexParameters.choice ==
		eOpenLogicalChannel_reverseLogicalChannelParameters_multiplexParameters_h2250LogicalChannelParameters))
	{
		ret =
		    process_h245_channel(pskb, ct, ctinfo, data, dataoff,
					 &olc->reverseLogicalChannelParameters.multiplexParameters.h2250LogicalChannelParameters);
		if (ret < 0)
			return -1;
	}

	if ((olc->options & eOpenLogicalChannel_separateStack) &&
	    olc->forwardLogicalChannelParameters.dataType.choice ==
	    eDataType_data &&
	    olc->forwardLogicalChannelParameters.dataType.data.application.choice == 
	    eDataApplicationCapability_application_t120 &&
	    olc->forwardLogicalChannelParameters.dataType.data.application.t120.choice == 
	    eDataProtocolCapability_separateLANStack &&
	    olc->separateStack.networkAddress.choice ==
	    eNetworkAccessParameters_networkAddress_localAreaAddress) {
		ret = expect_t120(pskb, ct, ctinfo, data, dataoff,
				  &olc->separateStack.networkAddress.localAreaAddress);
		if (ret < 0)
			return -1;
	}

	return 0;
}

/****************************************************************************/
static int process_olca(struct sk_buff **pskb, struct nf_conn *ct,
			enum ip_conntrack_info ctinfo,
			unsigned char **data, int dataoff,
			OpenLogicalChannelAck *olca)
{
	H2250LogicalChannelAckParameters *ack;
	int ret;

	DEBUGP("\r\n\r\nnf_ct_h323: OpenLogicalChannelAck\n");

	if ((olca->options &
	     eOpenLogicalChannelAck_reverseLogicalChannelParameters) &&
	    (olca->reverseLogicalChannelParameters.options &
	     eOpenLogicalChannelAck_reverseLogicalChannelParameters_multiplexParameters)
	    && (olca->reverseLogicalChannelParameters.multiplexParameters.choice ==
		eOpenLogicalChannelAck_reverseLogicalChannelParameters_multiplexParameters_h2250LogicalChannelParameters))
	{
		ret = process_h245_channel(pskb, ct, ctinfo, data, dataoff,
					   &olca->reverseLogicalChannelParameters.multiplexParameters.h2250LogicalChannelParameters);
		if (ret < 0)
			return -1;
	}

	if ((olca->options &
	     eOpenLogicalChannelAck_forwardMultiplexAckParameters) &&
	    (olca->forwardMultiplexAckParameters.choice ==
	     eOpenLogicalChannelAck_forwardMultiplexAckParameters_h2250LogicalChannelAckParameters))
	{
		ack = &olca->forwardMultiplexAckParameters.h2250LogicalChannelAckParameters;
		if (ack->options &
		    eH2250LogicalChannelAckParameters_mediaChannel) {
			/* RTP */
            #if 0
			ret = expect_rtp_rtcp(pskb, ct, ctinfo, data, dataoff,
					      &ack->mediaChannel);
            #else
			ret = expect_rtp_rtcp(pskb, ct, ctinfo, data, dataoff,
					      &ack->mediaChannel, MEDIA_CHANNEL);
            #endif
			if (ret < 0)
				return -1;
		}

		if (ack->options &
		    eH2250LogicalChannelAckParameters_mediaControlChannel) {
			/* RTCP */
            #if 0
			ret = expect_rtp_rtcp(pskb, ct, ctinfo, data, dataoff,
					      &ack->mediaControlChannel);
            #else
			ret = expect_rtp_rtcp(pskb, ct, ctinfo, data, dataoff,
					      &ack->mediaControlChannel, MEDIA_CONTROL_CHANNEL);
            #endif
			if (ret < 0)
				return -1;
		}
	}

	return 0;
}

/* H.245 中一般前面的TCS(能力通报)，MSD(主从关系)等一般都不会携带RTCP/RTP相关信息 */
static int process_h245(struct sk_buff **pskb, struct nf_conn *ct,
			enum ip_conntrack_info ctinfo,
			unsigned char **data, int dataoff,
			MultimediaSystemControlMessage *mscm)
{
	switch (mscm->choice) {
	case eMultimediaSystemControlMessage_request:
		if (mscm->request.choice ==
		    eRequestMessage_openLogicalChannel) {
			return process_olc(pskb, ct, ctinfo, data, dataoff,
					   &mscm->request.openLogicalChannel);
		}
		DEBUGP("nf_ct_h323: H.245 Request %d\n",
		       mscm->request.choice);
		break;
	case eMultimediaSystemControlMessage_response:
		if (mscm->response.choice ==
		    eResponseMessage_openLogicalChannelAck) {
			return process_olca(pskb, ct, ctinfo, data, dataoff,
					    &mscm->response.
					    openLogicalChannelAck);
		}
		DEBUGP("nf_ct_h323: H.245 Response %d\n",
		       mscm->response.choice);
		break;
	default:
		DEBUGP("nf_ct_h323: H.245 signal %d\n", mscm->choice);
		break;
	}

	return 0;
}



/* 处理 H.245 报文 */
static int h245_help(struct sk_buff **pskb, unsigned int protoff,
		     struct nf_conn *ct, enum ip_conntrack_info ctinfo)
{
	static MultimediaSystemControlMessage mscm;
	unsigned char *data = NULL;
	int datalen;
	int dataoff;
	int ret;

    /*start of HGW by d37981  2009.4.17 HG522V100R001C08B115  A36D06508 */
    if (!h323_enable)
    {
        return NF_ACCEPT;
    }
    /*e n d of HGW by d37981  2009.4.17 HG522V100R001C08B115  A36D06508
	/* Until there's been traffic both ways, don't look in packets. */
	if (ctinfo != IP_CT_ESTABLISHED &&
	    ctinfo != IP_CT_ESTABLISHED + IP_CT_IS_REPLY) 
    {
        DEBUGP("Status no right......\r\n");
		return NF_ACCEPT;
	}
    
	DEBUGP("nf_ct_h245: skblen = %u\n", (*pskb)->len);

	spin_lock_bh(&nf_h323_lock);

	/* Process each TPKT */
	while (get_tpkt_data(pskb, protoff, ct, ctinfo,
			     &data, &datalen, &dataoff)) {
			     
		DEBUGP("nf_ct_h245: TPKT len=%d ", datalen);
		NF_CT_DUMP_TUPLE(&ct->tuplehash[CTINFO2DIR(ctinfo)].tuple);

		/* Decode H.245 signal */
		ret = DecodeMultimediaSystemControlMessage(data, datalen,
							   &mscm);
		if (ret < 0) {
            #if 0
			if (net_ratelimit())
				printk("nf_ct_h245: decoding error: %s\n",
				       ret == H323_ERROR_BOUND ?
				       "out of bound" : "out of range");
			/* We don't drop when decoding error */
			break;
            #else
            DEBUGP("nf_ct_h245: decoding error: %s\n",
                   ret == H323_ERROR_BOUND ?
                   "out of bound" : "out of range");
            #endif
		}

		/* Process H.245 signal */
		if (process_h245(pskb, ct, ctinfo, &data, dataoff, &mscm) < 0)
		{
            #if 1
            DEBUGP("process_h245 error, but don't drop it");
            //break;
            #else
			goto drop;
            #endif
		}
	}

	spin_unlock_bh(&nf_h323_lock);
	return NF_ACCEPT;

      drop:
	spin_unlock_bh(&nf_h323_lock);
    #if 0
	if (net_ratelimit())
		printk("nf_ct_h245: packet dropped\n");
    #else
    DEBUGP("nf_ct_h245: packet dropped\n");
    #endif
	return NF_DROP;
}


/* H.245 中打开逻辑通道通告RTP端口信息 */
static struct nf_conntrack_helper nf_conntrack_helper_h245 __read_mostly = {
	.name			= "H.245",
	.me			= THIS_MODULE,
	.max_expected		= H323_RTP_CHANNEL_MAX * 4 + 2 /* T.120 */,
	.timeout		= 240,
	.tuple.dst.protonum	= IPPROTO_UDP,                  /* 这里无所谓因为并没有注册到helper链上所以不会检查 */
	.mask.src.u.udp.port	= __constant_htons(0xFFFF),
	.mask.dst.protonum	= 0xFF,
	.help			= h245_help
};

/****************************************************************************/
int get_h225_addr(struct nf_conn *ct, unsigned char *data,
		  TransportAddress *taddr,
		  union nf_conntrack_address *addr, __be16 *port)
{
	unsigned char *p;
	int family = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num;
	int len;

	switch (taddr->choice) {
	case eTransportAddress_ipAddress:
		if (family != AF_INET)
			return 0;
		p = data + taddr->ipAddress.ip;
		len = 4;
		break;
	case eTransportAddress_ip6Address:
		if (family != AF_INET6)
			return 0;
		p = data + taddr->ip6Address.ip6;
		len = 16;
		break;
	default:
        DEBUGP("Get h225 addr error");
		return 0;
	}


	memcpy(addr, p, len);
	memset((void *)addr + len, 0, sizeof(*addr) - len);
	memcpy(port, p + len, sizeof(__be16));

    

	return 1;
}

/****************************************************************************/
static int expect_h245(struct sk_buff **pskb, struct nf_conn *ct,
		       enum ip_conntrack_info ctinfo,
		       unsigned char **data, int dataoff,
		       TransportAddress *taddr)
{
	int dir = CTINFO2DIR(ctinfo);
	int ret = 0;
	__be16 port;
	union nf_conntrack_address addr;
	struct nf_conntrack_expect *exp;
	typeof(nat_h245_hook) nat_h245;


	/* Read h245Address */
    #if 0
	if (!get_h225_addr(ct, *data, taddr, &addr, &port) ||
	    memcmp(&addr, &ct->tuplehash[dir].tuple.src.u3, sizeof(addr)) ||
	    port == 0)
    #else
	if (!get_h225_addr(ct, *data, taddr, &addr, &port) ||
	    port == 0)
    #endif
	{
        DEBUGP("Get address [%u.%u.%u.%u] tuplehash src[%u.%u.%u.%u]",
            NIPQUAD(addr.ip), NIPQUAD(ct->tuplehash[dir].tuple.src.u3.ip));
		return 0;
	}

	/* Create expect for h245 connection */
	if ((exp = nf_conntrack_expect_alloc(ct)) == NULL)
		return -1;

    memset(exp, 0, sizeof(exp));

#if 0
	nf_conntrack_expect_init(exp, ct->tuplehash[!dir].tuple.src.l3num,
				 &ct->tuplehash[!dir].tuple.src.u3,
				 &ct->tuplehash[!dir].tuple.dst.u3,
				 IPPROTO_TCP, NULL, &port);
#else
nf_conntrack_expect_init(exp, ct->tuplehash[!dir].tuple.src.l3num,
            NULL,
            &ct->tuplehash[!dir].tuple.dst.u3,
            IPPROTO_TCP, NULL, &port);
#endif

	exp->helper = &nf_conntrack_helper_h245;


	if (memcmp(&ct->tuplehash[dir].tuple.src.u3,
		   &ct->tuplehash[!dir].tuple.dst.u3,
		   sizeof(ct->tuplehash[dir].tuple.src.u3)) &&
	    (nat_h245 = rcu_dereference(nat_h245_hook)) &&
	    ct->status & IPS_NAT_MASK) {
		/* NAT needed */
		ret = nat_h245(pskb, ct, ctinfo, data, dataoff, taddr,
			       port, exp);
	} else {		/* Conntrack only */

        nf_conntrack_expect_init(exp, ct->tuplehash[!dir].tuple.src.l3num,
                    &ct->tuplehash[!dir].tuple.src.u3,
                    &addr,
                    IPPROTO_TCP, NULL, &port);
        exp->helper = &nf_conntrack_helper_h245;

        
		if (nf_conntrack_expect_related(exp) == 0) {
			DEBUGP("nf_ct_q931: expect H.245 ");
			NF_CT_DUMP_TUPLE(&exp->tuple);
		} else
			ret = -1;
	}


    DEBUGP("create exp src:[%u.%u.%u.%u], dst[%u.%u.%u.%u] dstport: %d, dir: %0x proto: %0x",
        NIPQUAD(exp->tuple.src.u3.ip),
        NIPQUAD(addr.ip),
        port, exp->dir, exp->tuple.src.l3num);


	nf_conntrack_expect_put(exp);

	return ret;
}

/* If the calling party is on the same side of the forward-to party,
 * we don't need to track the second call */
static int callforward_do_filter(union nf_conntrack_address *src,
				 union nf_conntrack_address *dst,
				 int family)
{
	struct flowi fl1, fl2;
	int ret = 0;

	memset(&fl1, 0, sizeof(fl1));
	memset(&fl2, 0, sizeof(fl2));

	switch (family) {
	case AF_INET: {
		struct rtable *rt1, *rt2;

		fl1.fl4_dst = src->ip;
		fl2.fl4_dst = dst->ip;
		if (ip_route_output_key(&rt1, &fl1) == 0) {
			if (ip_route_output_key(&rt2, &fl2) == 0) {
				if (rt1->rt_gateway == rt2->rt_gateway &&
				    rt1->u.dst.dev  == rt2->u.dst.dev)
					ret = 1;
				dst_release(&rt2->u.dst);
			}
			dst_release(&rt1->u.dst);
		}
		break;
	}
#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
	case AF_INET6: {
		struct rt6_info *rt1, *rt2;

		memcpy(&fl1.fl6_dst, src, sizeof(fl1.fl6_dst));
		memcpy(&fl2.fl6_dst, dst, sizeof(fl2.fl6_dst));
		rt1 = (struct rt6_info *)ip6_route_output(NULL, &fl1);
		if (rt1) {
			rt2 = (struct rt6_info *)ip6_route_output(NULL, &fl2);
			if (rt2) {
				if (!memcmp(&rt1->rt6i_gateway, &rt2->rt6i_gateway,
					    sizeof(rt1->rt6i_gateway)) &&
				    rt1->u.dst.dev == rt2->u.dst.dev)
					ret = 1;
				dst_release(&rt2->u.dst);
			}
			dst_release(&rt1->u.dst);
		}
		break;
	}
#endif
	}
	return ret;

}

/****************************************************************************/
static int expect_callforwarding(struct sk_buff **pskb,
				 struct nf_conn *ct,
				 enum ip_conntrack_info ctinfo,
				 unsigned char **data, int dataoff,
				 TransportAddress *taddr)
{
	int dir = CTINFO2DIR(ctinfo);
	int ret = 0;
	__be16 port;
	union nf_conntrack_address addr;
	struct nf_conntrack_expect *exp;
	typeof(nat_callforwarding_hook) nat_callforwarding;

	/* Read alternativeAddress */
	if (!get_h225_addr(ct, *data, taddr, &addr, &port) || port == 0)
		return 0;

	/* If the calling party is on the same side of the forward-to party,
	 * we don't need to track the second call */
	if (callforward_filter &&
	    callforward_do_filter(&addr, &ct->tuplehash[!dir].tuple.src.u3,
				  ct->tuplehash[!dir].tuple.src.l3num)) {
		DEBUGP("nf_ct_q931: Call Forwarding not tracked\n");
		return 0;
	}

	/* Create expect for the second call leg */
	if ((exp = nf_conntrack_expect_alloc(ct)) == NULL)
		return -1;
	nf_conntrack_expect_init(exp, ct->tuplehash[!dir].tuple.src.l3num,
				 &ct->tuplehash[!dir].tuple.src.u3, &addr,
				 IPPROTO_TCP, NULL, &port);
	exp->helper = nf_conntrack_helper_q931;

	if (memcmp(&ct->tuplehash[dir].tuple.src.u3,
		   &ct->tuplehash[!dir].tuple.dst.u3,
		   sizeof(ct->tuplehash[dir].tuple.src.u3)) &&
	    (nat_callforwarding = rcu_dereference(nat_callforwarding_hook)) &&
	    ct->status & IPS_NAT_MASK) {
		/* Need NAT */
		ret = nat_callforwarding(pskb, ct, ctinfo, data, dataoff,
					 taddr, port, exp);
	} else {		/* Conntrack only */
		if (nf_conntrack_expect_related(exp) == 0) {
			DEBUGP("nf_ct_q931: expect Call Forwarding ");
			NF_CT_DUMP_TUPLE(&exp->tuple);
		} else
			ret = -1;
	}

	nf_conntrack_expect_put(exp);

	return ret;
}


/* 处理 setup 消息，主要就是需要修改 sourceCallSignalAddress 中的IP地址字段 */
static int process_setup(struct sk_buff **pskb, struct nf_conn *ct,
			 enum ip_conntrack_info ctinfo,
			 unsigned char **data, int dataoff,
			 Setup_UUIE *setup)
{
	int dir = CTINFO2DIR(ctinfo);
	int ret;
	int i;
	__be16 port;
	union nf_conntrack_address addr;
	typeof(set_h225_addr_hook) set_h225_addr;

	DEBUGP("nf_ct_q931: Setup\n");

	if (setup->options & eSetup_UUIE_h245Address) {
		ret = expect_h245(pskb, ct, ctinfo, data, dataoff,
				  &setup->h245Address);
		if (ret < 0)
			return -1;
	}



	set_h225_addr = rcu_dereference(set_h225_addr_hook);
	if ((setup->options & eSetup_UUIE_destCallSignalAddress) &&
	    (set_h225_addr) && ct->status && IPS_NAT_MASK &&
	    get_h225_addr(ct, *data, &setup->destCallSignalAddress,
			  &addr, &port) &&
	    memcmp(&addr, &ct->tuplehash[!dir].tuple.src.u3, sizeof(addr))) {
		DEBUGP("nf_ct_q931: set destCallSignalAddress "
		       NIP6_FMT ":%hu->" NIP6_FMT ":%hu\n",
		       NIP6(*(struct in6_addr *)&addr), ntohs(port),
		       NIP6(*(struct in6_addr *)&ct->tuplehash[!dir].tuple.src.u3),
		       ntohs(ct->tuplehash[!dir].tuple.src.u.tcp.port));
		ret = set_h225_addr(pskb, data, dataoff,
				    &setup->destCallSignalAddress,
				    &ct->tuplehash[!dir].tuple.src.u3,
				    ct->tuplehash[!dir].tuple.src.u.tcp.port);
		if (ret < 0)
			return -1;
	}



    /* 对于LAN侧终端做主叫来说，在这种情况下 sourcecallsignaladdress 就是LAN侧地址，因此需要修改成WAN侧地址 */
	if ((setup->options & eSetup_UUIE_sourceCallSignalAddress) &&
	    (set_h225_addr) && ct->status & IPS_NAT_MASK &&
	    get_h225_addr(ct, *data, &setup->sourceCallSignalAddress,
			  &addr, &port) &&
	    memcmp(&addr, &ct->tuplehash[!dir].tuple.dst.u3, sizeof(addr))) {
		DEBUGP("nf_ct_q931: set sourceCallSignalAddress "
		       NIP6_FMT ":%hu->" NIP6_FMT ":%hu\n",
		       NIP6(*(struct in6_addr *)&addr), ntohs(port),
		       NIP6(*(struct in6_addr *)&ct->tuplehash[!dir].tuple.dst.u3),
		       ntohs(ct->tuplehash[!dir].tuple.dst.u.tcp.port));
		ret = set_h225_addr(pskb, data, dataoff,
				    &setup->sourceCallSignalAddress,
				    &ct->tuplehash[!dir].tuple.dst.u3,
				    ct->tuplehash[!dir].tuple.dst.u.tcp.port);
		if (ret < 0)
			return -1;
	}


    /* 如果开启了快启那么可能在setup中直接就携带了OLC消息 */
	if (setup->options & eSetup_UUIE_fastStart) {
        DEBUGP("Now, begin to process fast start message");
		for (i = 0; i < setup->fastStart.count; i++) {
			ret = process_olc(pskb, ct, ctinfo, data, dataoff,
					  &setup->fastStart.item[i]);
			if (ret < 0)
				return -1;
		}
	}


	return 0;
}

/****************************************************************************/
static int process_callproceeding(struct sk_buff **pskb,
				  struct nf_conn *ct,
				  enum ip_conntrack_info ctinfo,
				  unsigned char **data, int dataoff,
				  CallProceeding_UUIE *callproc)
{
	int ret;
	int i;

	DEBUGP("nf_ct_q931: CallProceeding\n");

	if (callproc->options & eCallProceeding_UUIE_h245Address) {
		ret = expect_h245(pskb, ct, ctinfo, data, dataoff,
				  &callproc->h245Address);
		if (ret < 0)
			return -1;
	}

	if (callproc->options & eCallProceeding_UUIE_fastStart) {
		for (i = 0; i < callproc->fastStart.count; i++) {
			ret = process_olc(pskb, ct, ctinfo, data, dataoff,
					  &callproc->fastStart.item[i]);
			if (ret < 0)
				return -1;
		}
	}

	return 0;
}

/* 一般来说 H.245 IP和端口信息都会带在本消息中，但是实际上，允许H.245IP和端口信息带在H.225的其它消息中
 * 如果作为主叫，那么本消息由关守发送过来，告知可以向对端终端的该地址和端口发起TCP连接
 * 如果作为被叫，那么本消息将会由本地终端发给关守，告知关守可以告知对端可以向本地终端的该端口发起连接
 */
static int process_connect(struct sk_buff **pskb, struct nf_conn *ct,
			   enum ip_conntrack_info ctinfo,
			   unsigned char **data, int dataoff,
			   Connect_UUIE *connect)
{
	int ret;
	int i;

	DEBUGP("nf_ct_q931: Connect\n");

    /* 为 H.245 创建预期，如果LAN侧终端做主叫，那么此时可以无需要创建预期，但是如果LAN侧做被叫，那么必须
       * 创建预期，否则，对端主动连接过来的时候数据包将不会被网关转发给对应的LAN侧
       */
	if (connect->options & eConnect_UUIE_h245Address) {
		ret = expect_h245(pskb, ct, ctinfo, data, dataoff,
				  &connect->h245Address);
		if (ret < 0)
		{
            DEBUGP("Error, expect h245");
            return -1;
		}
	}

	if (connect->options & eConnect_UUIE_fastStart) {
		for (i = 0; i < connect->fastStart.count; i++) {
			ret = process_olc(pskb, ct, ctinfo, data, dataoff,
					  &connect->fastStart.item[i]);
			if (ret < 0)
				return -1;
		}
	}

	return 0;
}

/****************************************************************************/
static int process_alerting(struct sk_buff **pskb, struct nf_conn *ct,
			    enum ip_conntrack_info ctinfo,
			    unsigned char **data, int dataoff,
			    Alerting_UUIE *alert)
{
	int ret;
	int i;

	DEBUGP("nf_ct_q931: Alerting\n");

	if (alert->options & eAlerting_UUIE_h245Address) {
		ret = expect_h245(pskb, ct, ctinfo, data, dataoff,
				  &alert->h245Address);
		if (ret < 0)
			return -1;
	}

	if (alert->options & eAlerting_UUIE_fastStart) {
		for (i = 0; i < alert->fastStart.count; i++) {
			ret = process_olc(pskb, ct, ctinfo, data, dataoff,
					  &alert->fastStart.item[i]);
			if (ret < 0)
				return -1;
		}
	}

	return 0;
}

/****************************************************************************/
static int process_information(struct sk_buff **pskb,
			       struct nf_conn *ct,
			       enum ip_conntrack_info ctinfo,
			       unsigned char **data, int dataoff,
			       Information_UUIE *info)
{
	int ret;
	int i;

	DEBUGP("nf_ct_q931: Information\n");

	if (info->options & eInformation_UUIE_fastStart) {
		for (i = 0; i < info->fastStart.count; i++) {
			ret = process_olc(pskb, ct, ctinfo, data, dataoff,
					  &info->fastStart.item[i]);
			if (ret < 0)
				return -1;
		}
	}

	return 0;
}

/****************************************************************************/
static int process_facility(struct sk_buff **pskb, struct nf_conn *ct,
			    enum ip_conntrack_info ctinfo,
			    unsigned char **data, int dataoff,
			    Facility_UUIE *facility)
{
	int ret;
	int i;

	DEBUGP("nf_ct_q931: Facility\n");

	if (facility->reason.choice == eFacilityReason_callForwarded) {
		if (facility->options & eFacility_UUIE_alternativeAddress)
			return expect_callforwarding(pskb, ct, ctinfo, data,
						     dataoff,
						     &facility->
						     alternativeAddress);
		return 0;
	}

	if (facility->options & eFacility_UUIE_h245Address) {
		ret = expect_h245(pskb, ct, ctinfo, data, dataoff,
				  &facility->h245Address);
		if (ret < 0)
			return -1;
	}

	if (facility->options & eFacility_UUIE_fastStart) {
		for (i = 0; i < facility->fastStart.count; i++) {
			ret = process_olc(pskb, ct, ctinfo, data, dataoff,
					  &facility->fastStart.item[i]);
			if (ret < 0)
				return -1;
		}
	}

	return 0;
}

/****************************************************************************/
static int process_progress(struct sk_buff **pskb, struct nf_conn *ct,
			    enum ip_conntrack_info ctinfo,
			    unsigned char **data, int dataoff,
			    Progress_UUIE *progress)
{
	int ret;
	int i;

	DEBUGP("nf_ct_q931: Progress\n");

	if (progress->options & eProgress_UUIE_h245Address) {
		ret = expect_h245(pskb, ct, ctinfo, data, dataoff,
				  &progress->h245Address);
		if (ret < 0)
			return -1;
	}

	if (progress->options & eProgress_UUIE_fastStart) {
		for (i = 0; i < progress->fastStart.count; i++) {
			ret = process_olc(pskb, ct, ctinfo, data, dataoff,
					  &progress->fastStart.item[i]);
			if (ret < 0)
				return -1;
		}
	}

	return 0;
}

/****************************************************************************/
static int process_q931(struct sk_buff **pskb, struct nf_conn *ct,
			enum ip_conntrack_info ctinfo,
			unsigned char **data, int dataoff, Q931 *q931)
{
	H323_UU_PDU *pdu = &q931->UUIE.h323_uu_pdu;
	int i;
	int ret = 0;

	switch (pdu->h323_message_body.choice) {
	case eH323_UU_PDU_h323_message_body_setup:
		ret = process_setup(pskb, ct, ctinfo, data, dataoff,
				    &pdu->h323_message_body.setup);
		break;
	case eH323_UU_PDU_h323_message_body_callProceeding:
		ret = process_callproceeding(pskb, ct, ctinfo, data, dataoff,
					     &pdu->h323_message_body.
					     callProceeding);
		break;
	case eH323_UU_PDU_h323_message_body_connect:
		ret = process_connect(pskb, ct, ctinfo, data, dataoff,
				      &pdu->h323_message_body.connect);
		break;
	case eH323_UU_PDU_h323_message_body_alerting:
		ret = process_alerting(pskb, ct, ctinfo, data, dataoff,
				       &pdu->h323_message_body.alerting);
		break;
	case eH323_UU_PDU_h323_message_body_information:
		ret = process_information(pskb, ct, ctinfo, data, dataoff,
					  &pdu->h323_message_body.
					  information);
		break;
	case eH323_UU_PDU_h323_message_body_facility:
		ret = process_facility(pskb, ct, ctinfo, data, dataoff,
				       &pdu->h323_message_body.facility);
		break;
	case eH323_UU_PDU_h323_message_body_progress:
		ret = process_progress(pskb, ct, ctinfo, data, dataoff,
				       &pdu->h323_message_body.progress);
		break;
	default:
		DEBUGP("nf_ct_q931: Q.931 signal %d\n",
		       pdu->h323_message_body.choice);
		break;
	}

	if (ret < 0)
		return -1;

	if (pdu->options & eH323_UU_PDU_h245Control) {
		for (i = 0; i < pdu->h245Control.count; i++) {
			ret = process_h245(pskb, ct, ctinfo, data, dataoff,
					   &pdu->h245Control.item[i]);
			if (ret < 0)
				return -1;
		}
	}

	return 0;
}


/* START of by d00107688 强制设定MSS值的大小 */
static inline unsigned int tcpoptlen(const u_int8_t *opt, unsigned int offset)
{
	/* Beware zero-length options: make finite progress */
	if (opt[offset] <= TCPOPT_NOP || opt[offset+1] == 0)
		return 1;
	else
		return opt[offset+1];
}


static int tcpmss_set_packet(struct sk_buff **pskb,
		     unsigned int tcphoff, unsigned int mss, 
		     unsigned int minlen)
{
	struct tcphdr *tcph;
	unsigned int tcplen, i;
	u16 newmss;
	u8 *opt;

	if (!skb_make_writable(pskb, (*pskb)->len))
		return -1;

	tcplen = (*pskb)->len - tcphoff;
	tcph = (struct tcphdr *)((*pskb)->nh.raw + tcphoff);

	/* Since it passed flags test in tcp match, we know it is is
	   not a fragment, and has data >= tcp header length.  SYN
	   packets should not contain data: if they did, then we risk
	   running over MTU, sending Frag Needed and breaking things
	   badly. --RR */
	if (tcplen != tcph->doff*4) {
			DEBUGP("xt_TCPMSS: bad length (%u bytes)\n",
			       (*pskb)->len);
		return -1;
	}

    #ifndef XT_TCPMSS_CLAMP_PMTU
    #define XT_TCPMSS_CLAMP_PMTU 0xffff
    #endif

	if (mss == XT_TCPMSS_CLAMP_PMTU) {
		if (dst_mtu((*pskb)->dst) <= minlen) {
				DEBUGP("xt_TCPMSS: "
				       "unknown or invalid path-MTU (%u)\n",
				       dst_mtu((*pskb)->dst));
			return -1;
		}
		newmss = dst_mtu((*pskb)->dst) - minlen;
	} else
		newmss = mss;

	opt = (u_int8_t *)tcph;
	for (i = sizeof(struct tcphdr); i < tcph->doff*4; i += tcpoptlen(opt, i)) {
		if ((opt[i] == TCPOPT_MSS) && (tcph->doff*4 - i >= TCPOLEN_MSS) &&
		    (opt[i+1] == TCPOLEN_MSS)) {
			u_int16_t oldmss;

			oldmss = (opt[i+2] << 8) | opt[i+3];


			if (mss == XT_TCPMSS_CLAMP_PMTU &&
			    oldmss <= newmss)
				return 0;

			opt[i+2] = (newmss & 0xff00) >> 8;
			opt[i+3] = (newmss & 0x00ff);
            
			nf_proto_csum_replace2(&tcph->check, *pskb,
					       htons(oldmss), htons(newmss), 0);

            
			oldmss = (opt[i+2] << 8) | opt[i+3];

			return 0;
		}
	}

    return 0;

#if 0

	/*
	 * MSS Option not found ?! add it..
	 */
	if (skb_tailroom((*pskb)) < TCPOLEN_MSS) {
		struct sk_buff *newskb;

		newskb = skb_copy_expand(*pskb, skb_headroom(*pskb),
					 TCPOLEN_MSS, GFP_ATOMIC);
		if (!newskb)
			return -1;
		kfree_skb(*pskb);
		*pskb = newskb;
		tcph = (struct tcphdr *)((*pskb)->nh.raw + tcphoff);
	}

	skb_put((*pskb), TCPOLEN_MSS);

	opt = (u_int8_t *)tcph + sizeof(struct tcphdr);
	memmove(opt + TCPOLEN_MSS, opt, tcplen - sizeof(struct tcphdr));

	nf_proto_csum_replace2(&tcph->check, *pskb,
			       htons(tcplen), htons(tcplen + TCPOLEN_MSS), 1);
	opt[0] = TCPOPT_MSS;
	opt[1] = TCPOLEN_MSS;
	opt[2] = (newmss & 0xff00) >> 8;
	opt[3] = (newmss & 0x00ff);

	nf_proto_csum_replace4(&tcph->check, *pskb, 0, *((__be32 *)opt), 0);

	oldval = ((__be16 *)tcph)[6];
	tcph->doff += TCPOLEN_MSS/4;
	nf_proto_csum_replace2(&tcph->check, *pskb,
				oldval, ((__be16 *)tcph)[6], 0);
	return TCPOLEN_MSS;
#endif
}


static int tcpmss_modify_packet(struct sk_buff **pskb)
{
	struct iphdr *iph = (*pskb)->nh.iph;
	__be16 newlen;
	int ret;
    
	ret = tcpmss_set_packet(pskb, (iph->ihl * 4), 1460,
				   (sizeof(*iph) + sizeof(struct tcphdr)));
    if (ret > 0)
    {
		iph = (*pskb)->nh.iph;
		newlen = htons(ntohs(iph->tot_len) + ret);
		nf_csum_replace2(&iph->check, iph->tot_len, newlen);
		iph->tot_len = newlen;
        return 1;
    }

    return 0;
    
}
/* end of by d00107688 强制设定MSS值的大小 */


/* 处理 H.225.0 TCP 信令 */
static int q931_help(struct sk_buff **pskb, unsigned int protoff,
		     struct nf_conn *ct, enum ip_conntrack_info ctinfo)
{
	static Q931 q931;
	unsigned char *data = NULL;
	int datalen;
	int dataoff;
	int ret;

    /*start of HGW by d37981  2009.4.17 HG522V100R001C08B115  A36D06508 */
    if (!h323_enable)
    {
        return NF_ACCEPT;
    }
    /*e n d of HGW by d37981  2009.4.17 HG522V100R001C08B115  A36D06508
	/* Until there's been traffic both ways, don't look in packets. */
	if ((ctinfo != IP_CT_ESTABLISHED) &&
	    (ctinfo != IP_CT_ESTABLISHED + IP_CT_IS_REPLY)) 
    {
        #if 0
        /* start of by d00107688 在此修改TCP MSS 大小 */
        tcpmss_modify_packet(pskb);        
        /* end of by d00107688 在此修改TCP MSS 大小 */
        #endif
        
        return NF_ACCEPT;
	}

    /* start of by d00107688 在此修改TCP MSS 大小 */
    if (ctinfo == (IP_CT_ESTABLISHED + IP_CT_IS_REPLY))
    {
        tcpmss_modify_packet(pskb);        
    }
    /* end of by d00107688 在此修改TCP MSS 大小 */
    
	DEBUGP("nf_ct_q931: skblen = %u\n", (*pskb)->len);

	spin_lock_bh(&nf_h323_lock);

	/* Process each TPKT 格式见 RFC 1006,一个消息封装在一个TPKT中，如果有多个消息封在一个报文中，那么就有多个TPKT */
	while (get_tpkt_data(pskb, protoff, ct, ctinfo,
			     &data, &datalen, &dataoff)) {
		DEBUGP("nf_ct_q931: TPKT len=%d ", datalen);
		NF_CT_DUMP_TUPLE(&ct->tuplehash[CTINFO2DIR(ctinfo)].tuple);

		/* Decode Q.931 signal */
		ret = DecodeQ931(data, datalen, &q931);
		if (ret < 0) {
            #if 0
			if (net_ratelimit())
				printk("nf_ct_q931: decoding error: %s\n",
				       ret == H323_ERROR_BOUND ?
				       "out of bound" : "out of range");
			/* We don't drop when decoding error */
			break;
            #else
            DEBUGP("nf_ct_q931: decoding error: %s\n",
				       ret == H323_ERROR_BOUND ?
				       "out of bound" : "out of range");
            #endif
		}

		/* Process Q.931 signal */
		if (process_q931(pskb, ct, ctinfo, &data, dataoff, &q931) < 0)
        #if 0
            goto drop;
        #else
            DEBUGP("notice: process_q931 error");
            //break;
        #endif
	}

	spin_unlock_bh(&nf_h323_lock);
	return NF_ACCEPT;

      drop:
	spin_unlock_bh(&nf_h323_lock);
    #if 0
	if (net_ratelimit())
		printk("nf_ct_q931: packet dropped\n");
    #else
        DEBUGP("packet droped......");
    #endif
	return NF_DROP;
}

/****************************************************************************/
static struct nf_conntrack_helper nf_conntrack_helper_q931[] __read_mostly = {
	{
		.name			= "Q.931",
		.me			= THIS_MODULE,
					  /* T.120 and H.245 */
		.max_expected		= H323_RTP_CHANNEL_MAX * 4 + 4,
		.timeout		= 240,
		.tuple.src.l3num	= AF_INET,
		.tuple.src.u.tcp.port	= __constant_htons(Q931_PORT),
		.tuple.dst.protonum	= IPPROTO_TCP,
		.mask.src.l3num		= 0xFFFF,
		.mask.src.u.tcp.port	= __constant_htons(0xFFFF),
		.mask.dst.protonum	= 0xFF,
		.help			= q931_help
	},
	{
		.name			= "Q.931",
		.me			= THIS_MODULE,
					  /* T.120 and H.245 */
		.max_expected		= H323_RTP_CHANNEL_MAX * 4 + 4,
		.timeout		= 240,
		.tuple.src.l3num	= AF_INET6,
		.tuple.src.u.tcp.port	= __constant_htons(Q931_PORT),
		.tuple.dst.protonum	= IPPROTO_TCP,
		.mask.src.l3num		= 0xFFFF,
		.mask.src.u.tcp.port	= __constant_htons(0xFFFF),
		.mask.dst.protonum	= 0xFF,
		.help			= q931_help
	},
};

/****************************************************************************/
static unsigned char *get_udp_data(struct sk_buff **pskb, unsigned int protoff,
				   int *datalen)
{
	struct udphdr _uh, *uh;
	int dataoff;

	uh = skb_header_pointer(*pskb, protoff, sizeof(_uh), &_uh);
	if (uh == NULL)
		return NULL;
	dataoff = protoff + sizeof(_uh);
	if (dataoff >= (*pskb)->len)
		return NULL;
	*datalen = (*pskb)->len - dataoff;
	return skb_header_pointer(*pskb, dataoff, *datalen, h323_buffer);
}

/****************************************************************************/
static struct nf_conntrack_expect *find_expect(struct nf_conn *ct,
					       union nf_conntrack_address *addr,
					       __be16 port)
{
	struct nf_conntrack_expect *exp;
	struct nf_conntrack_tuple tuple;

	memset(&tuple.src.u3, 0, sizeof(tuple.src.u3));
	tuple.src.u.tcp.port = 0;
	memcpy(&tuple.dst.u3, addr, sizeof(tuple.dst.u3));
	tuple.dst.u.tcp.port = port;
	tuple.dst.protonum = IPPROTO_TCP;

	exp = __nf_conntrack_expect_find(&tuple);
	if (exp && exp->master == ct)
		return exp;
	return NULL;
}

/****************************************************************************/
static int set_expect_timeout(struct nf_conntrack_expect *exp,
			      unsigned timeout)
{
	if (!exp || !del_timer(&exp->timeout))
		return 0;

	exp->timeout.expires = jiffies + timeout * HZ;
	add_timer(&exp->timeout);

	return 1;
}

/****************************************************************************/
static int expect_q931(struct sk_buff **pskb, struct nf_conn *ct,
		       enum ip_conntrack_info ctinfo,
		       unsigned char **data,
		       TransportAddress *taddr, int count)
{
	struct nf_ct_h323_master *info = &nfct_help(ct)->help.ct_h323_info;
	int dir = CTINFO2DIR(ctinfo);
	int ret = 0;
	int i;
	__be16 port;
	union nf_conntrack_address addr;
	struct nf_conntrack_expect *exp;
	typeof(nat_q931_hook) nat_q931;

	/* Look for the first related address */
	for (i = 0; i < count; i++) {
		if (get_h225_addr(ct, *data, &taddr[i], &addr, &port) &&
		    memcmp(&addr, &ct->tuplehash[dir].tuple.src.u3,
			   sizeof(addr)) == 0 && port != 0)
			break;
	}

	if (i >= count)		/* Not found */
		return 0;

	/* Create expect for Q.931 */
	if ((exp = nf_conntrack_expect_alloc(ct)) == NULL)
		return -1;
	nf_conntrack_expect_init(exp, ct->tuplehash[!dir].tuple.src.l3num,
				 gkrouted_only ? /* only accept calls from GK? */
					&ct->tuplehash[!dir].tuple.src.u3 :
					NULL,
				 &ct->tuplehash[!dir].tuple.dst.u3,
				 IPPROTO_TCP, NULL, &port);
	exp->helper = nf_conntrack_helper_q931;
	exp->flags = NF_CT_EXPECT_PERMANENT;	/* Accept multiple calls */

	nat_q931 = rcu_dereference(nat_q931_hook);
	if (nat_q931 && ct->status & IPS_NAT_MASK) {	/* Need NAT */
		ret = nat_q931(pskb, ct, ctinfo, data, taddr, i, port, exp);
	} else {		/* Conntrack only */
		if (nf_conntrack_expect_related(exp) == 0) {
			DEBUGP("nf_ct_ras: expect Q.931 ");
			NF_CT_DUMP_TUPLE(&exp->tuple);

			/* Save port for looking up expect in processing RCF */
			info->sig_port[dir] = port;
		} else
			ret = -1;
	}

	nf_conntrack_expect_put(exp);

	return ret;
}

/****************************************************************************/
static int process_grq(struct sk_buff **pskb, struct nf_conn *ct,
		       enum ip_conntrack_info ctinfo,
		       unsigned char **data, GatekeeperRequest *grq)
{
	typeof(set_ras_addr_hook) set_ras_addr;

	DEBUGP("nf_ct_ras: GRQ\n");

	set_ras_addr = rcu_dereference(set_ras_addr_hook);
	if (set_ras_addr && ct->status & IPS_NAT_MASK)	/* NATed */
		return set_ras_addr(pskb, ct, ctinfo, data,
				    &grq->rasAddress, 1);
	return 0;
}

/****************************************************************************/
static int process_gcf(struct sk_buff **pskb, struct nf_conn *ct,
		       enum ip_conntrack_info ctinfo,
		       unsigned char **data, GatekeeperConfirm *gcf)
{
	int dir = CTINFO2DIR(ctinfo);
	int ret = 0;
	__be16 port;
	union nf_conntrack_address addr;
	struct nf_conntrack_expect *exp;

	DEBUGP("nf_ct_ras: GCF\n");

	if (!get_h225_addr(ct, *data, &gcf->rasAddress, &addr, &port))
		return 0;

	/* Registration port is the same as discovery port */
	if (!memcmp(&addr, &ct->tuplehash[dir].tuple.src.u3, sizeof(addr)) &&
	    port == ct->tuplehash[dir].tuple.src.u.udp.port)
		return 0;

	/* Avoid RAS expectation loops. A GCF is never expected. */
	if (test_bit(IPS_EXPECTED_BIT, &ct->status))
		return 0;

	/* Need new expect */
	if ((exp = nf_conntrack_expect_alloc(ct)) == NULL)
		return -1;
	nf_conntrack_expect_init(exp, ct->tuplehash[!dir].tuple.src.l3num,
				 &ct->tuplehash[!dir].tuple.src.u3, &addr,
				 IPPROTO_UDP, NULL, &port);
	exp->helper = nf_conntrack_helper_ras;

	if (nf_conntrack_expect_related(exp) == 0) {
		DEBUGP("nf_ct_ras: expect RAS ");
		NF_CT_DUMP_TUPLE(&exp->tuple);
	} else
		ret = -1;

	nf_conntrack_expect_put(exp);

	return ret;
}

/****************************************************************************/
static int process_rrq(struct sk_buff **pskb, struct nf_conn *ct,
		       enum ip_conntrack_info ctinfo,
		       unsigned char **data, RegistrationRequest *rrq)
{
	struct nf_ct_h323_master *info = &nfct_help(ct)->help.ct_h323_info;
	int ret;
	typeof(set_ras_addr_hook) set_ras_addr;

	DEBUGP("nf_ct_ras: RRQ\n");

	ret = expect_q931(pskb, ct, ctinfo, data,
			  rrq->callSignalAddress.item,
			  rrq->callSignalAddress.count);
	if (ret < 0)
		return -1;

	set_ras_addr = rcu_dereference(set_ras_addr_hook);
	if (set_ras_addr && ct->status & IPS_NAT_MASK) {
		ret = set_ras_addr(pskb, ct, ctinfo, data,
				   rrq->rasAddress.item,
				   rrq->rasAddress.count);
		if (ret < 0)
			return -1;
	}

	if (rrq->options & eRegistrationRequest_timeToLive) {
		DEBUGP("nf_ct_ras: RRQ TTL = %u seconds\n", rrq->timeToLive);
		info->timeout = rrq->timeToLive;
	} else
		info->timeout = default_rrq_ttl;

	return 0;
}

/****************************************************************************/
static int process_rcf(struct sk_buff **pskb, struct nf_conn *ct,
		       enum ip_conntrack_info ctinfo,
		       unsigned char **data, RegistrationConfirm *rcf)
{
	struct nf_ct_h323_master *info = &nfct_help(ct)->help.ct_h323_info;
	int dir = CTINFO2DIR(ctinfo);
	int ret;
	struct nf_conntrack_expect *exp;
	typeof(set_sig_addr_hook) set_sig_addr;

	DEBUGP("nf_ct_ras: RCF\n");

	set_sig_addr = rcu_dereference(set_sig_addr_hook);
	if (set_sig_addr && ct->status & IPS_NAT_MASK) {
		ret = set_sig_addr(pskb, ct, ctinfo, data,
					rcf->callSignalAddress.item,
					rcf->callSignalAddress.count);
		if (ret < 0)
			return -1;
	}

	if (rcf->options & eRegistrationConfirm_timeToLive) {
		DEBUGP("nf_ct_ras: RCF TTL = %u seconds\n", rcf->timeToLive);
		info->timeout = rcf->timeToLive;
	}

	if (info->timeout > 0) {
		DEBUGP
		    ("nf_ct_ras: set RAS connection timeout to %u seconds\n",
		     info->timeout);
		nf_ct_refresh(ct, *pskb, info->timeout * HZ);

		/* Set expect timeout */
		read_lock_bh(&nf_conntrack_lock);
		exp = find_expect(ct, &ct->tuplehash[dir].tuple.dst.u3,
				  info->sig_port[!dir]);
		if (exp) {
			DEBUGP("nf_ct_ras: set Q.931 expect "
			       "timeout to %u seconds for",
			       info->timeout);
			NF_CT_DUMP_TUPLE(&exp->tuple);
			set_expect_timeout(exp, info->timeout);
		}
		read_unlock_bh(&nf_conntrack_lock);
	}

	return 0;
}

/****************************************************************************/
static int process_urq(struct sk_buff **pskb, struct nf_conn *ct,
		       enum ip_conntrack_info ctinfo,
		       unsigned char **data, UnregistrationRequest *urq)
{
	struct nf_ct_h323_master *info = &nfct_help(ct)->help.ct_h323_info;
	int dir = CTINFO2DIR(ctinfo);
	int ret;
	typeof(set_sig_addr_hook) set_sig_addr;

	DEBUGP("nf_ct_ras: URQ\n");

	set_sig_addr = rcu_dereference(set_sig_addr_hook);
	if (set_sig_addr && ct->status & IPS_NAT_MASK) {
		ret = set_sig_addr(pskb, ct, ctinfo, data,
				   urq->callSignalAddress.item,
				   urq->callSignalAddress.count);
		if (ret < 0)
			return -1;
	}

	/* Clear old expect */
	nf_ct_remove_expectations(ct);
	info->sig_port[dir] = 0;
	info->sig_port[!dir] = 0;

	/* Give it 30 seconds for UCF or URJ */
	nf_ct_refresh(ct, *pskb, 30 * HZ);

	return 0;
}

/****************************************************************************/
/* 处理 ARQ报文，ARQ 是终端发送给服务器的请求报文，服务器回应ACF来表示同意为终端服务
 * 否则回应ARJ来拒绝终端的服务
 */
static int process_arq(struct sk_buff **pskb, struct nf_conn *ct,
		       enum ip_conntrack_info ctinfo,
		       unsigned char **data, AdmissionRequest *arq)
{
	struct nf_ct_h323_master *info = &nfct_help(ct)->help.ct_h323_info;
	int dir = CTINFO2DIR(ctinfo);
	__be16 port;
	union nf_conntrack_address addr;
	typeof(set_h225_addr_hook) set_h225_addr;

	DEBUGP("nf_ct_ras: ARQ\n");

    /* 对应 set_h225_addr */
	set_h225_addr = rcu_dereference(set_h225_addr_hook);

    /* 如果在 ARQ 中指定了呼叫的目的端 */
	if ((arq->options & eAdmissionRequest_destCallSignalAddress) &&
	    get_h225_addr(ct, *data, &arq->destCallSignalAddress,
			  &addr, &port) &&
	    !memcmp(&addr, &ct->tuplehash[dir].tuple.src.u3, sizeof(addr)) &&
	    port == info->sig_port[dir] &&
	    set_h225_addr && ct->status & IPS_NAT_MASK) {
		/* Answering ARQ */
		return set_h225_addr(pskb, data, 0,
				     &arq->destCallSignalAddress,
				     &ct->tuplehash[!dir].tuple.dst.u3,
				     info->sig_port[!dir]);
	}

    /* 获取源信号地址，并修改成对应的WAN侧地址 */
	if ((arq->options & eAdmissionRequest_srcCallSignalAddress) &&
	    get_h225_addr(ct, *data, &arq->srcCallSignalAddress,
			  &addr, &port) &&
	    !memcmp(&addr, &ct->tuplehash[dir].tuple.src.u3, sizeof(addr)) &&
	    set_h225_addr && ct->status & IPS_NAT_MASK) {
		/* Calling ARQ */
		return set_h225_addr(pskb, data, 0,
				     &arq->srcCallSignalAddress,
				     &ct->tuplehash[!dir].tuple.dst.u3,
				     port);
	}

	return 0;
}

/* 处理关守回应的ARQ 确认信息 */
static int process_acf(struct sk_buff **pskb, struct nf_conn *ct,
		       enum ip_conntrack_info ctinfo,
		       unsigned char **data, AdmissionConfirm *acf)
{
	int dir = CTINFO2DIR(ctinfo);
	int ret = 0;
	__be16 port;
	union nf_conntrack_address addr;
	struct nf_conntrack_expect *exp;
	typeof(set_sig_addr_hook) set_sig_addr;

	DEBUGP("nf_ct_ras: ACF\n");

    /* 获取对应的报文格式中的IP地址和端口，也就是说可以向这个地址和端口(一般是1720)发起TCP连接了 */
	if (!get_h225_addr(ct, *data, &acf->destCallSignalAddress,
			   &addr, &port))
		return 0;

    /* 这种情况什么时候会出现呢???? 一般应该很少走到这里吧??? */
	if (!memcmp(&addr, &ct->tuplehash[dir].tuple.dst.u3, sizeof(addr))) {
		/* Answering ACF */
		set_sig_addr = rcu_dereference(set_sig_addr_hook);
		if (set_sig_addr && ct->status & IPS_NAT_MASK)
			return set_sig_addr(pskb, ct, ctinfo, data,
					    &acf->destCallSignalAddress, 1);
		return 0;
	}


	/* Need new expect 因为可能双方都可以发起 H.225.0 的连接 */
	if ((exp = nf_conntrack_expect_alloc(ct)) == NULL)
		return -1;

    /* 预期的源地址为LAN侧终端，预期的目的地址为ACF中给出的地址，端口为ACF中指出的 */
	nf_conntrack_expect_init(exp, ct->tuplehash[!dir].tuple.src.l3num,
				 &ct->tuplehash[!dir].tuple.src.u3, &addr,
				 IPPROTO_TCP, NULL, &port);
	exp->flags = NF_CT_EXPECT_PERMANENT;        /* 这个预期是固定的，至少是在会话过程中不能够拆除,这个连接是由终端主动连接到关守的
	                                                                * 因此，一般来说，不需要做NAT,这里之所以这样做，是因为创建预期然后方便安置
	                                                                * helper函数，实际上，也可以将 helper单独在init中注册，但是这样似乎就破坏了整个
	                                                                * 流程的完整性了
	                                                                */
	exp->helper = nf_conntrack_helper_q931;     /* 对应的这个TCP连接就是 H.225.0 因此也就是处理H.225.0 */

	if (nf_conntrack_expect_related(exp) == 0) {
		DEBUGP("nf_ct_ras: expect Q.931 ");
		NF_CT_DUMP_TUPLE(&exp->tuple);
	} else
		ret = -1;

	nf_conntrack_expect_put(exp);

	return ret;
}

/****************************************************************************/
static int process_lrq(struct sk_buff **pskb, struct nf_conn *ct,
		       enum ip_conntrack_info ctinfo,
		       unsigned char **data, LocationRequest *lrq)
{
	typeof(set_ras_addr_hook) set_ras_addr;

	DEBUGP("nf_ct_ras: LRQ\n");

	set_ras_addr = rcu_dereference(set_ras_addr_hook);
	if (set_ras_addr && ct->status & IPS_NAT_MASK)
		return set_ras_addr(pskb, ct, ctinfo, data,
				    &lrq->replyAddress, 1);
	return 0;
}

/****************************************************************************/
static int process_lcf(struct sk_buff **pskb, struct nf_conn *ct,
		       enum ip_conntrack_info ctinfo,
		       unsigned char **data, LocationConfirm *lcf)
{
	int dir = CTINFO2DIR(ctinfo);
	int ret = 0;
	__be16 port;
	union nf_conntrack_address addr;
	struct nf_conntrack_expect *exp;

	DEBUGP("nf_ct_ras: LCF\n");

	if (!get_h225_addr(ct, *data, &lcf->callSignalAddress,
			   &addr, &port))
		return 0;

	/* Need new expect for call signal */
	if ((exp = nf_conntrack_expect_alloc(ct)) == NULL)
		return -1;
	nf_conntrack_expect_init(exp, ct->tuplehash[!dir].tuple.src.l3num,
				 &ct->tuplehash[!dir].tuple.src.u3, &addr,
				 IPPROTO_TCP, NULL, &port);
	exp->flags = NF_CT_EXPECT_PERMANENT;
	exp->helper = nf_conntrack_helper_q931;

	if (nf_conntrack_expect_related(exp) == 0) {
		DEBUGP("nf_ct_ras: expect Q.931 ");
		NF_CT_DUMP_TUPLE(&exp->tuple);
	} else
		ret = -1;

	nf_conntrack_expect_put(exp);

	/* Ignore rasAddress */

	return ret;
}

/****************************************************************************/
static int process_irr(struct sk_buff **pskb, struct nf_conn *ct,
		       enum ip_conntrack_info ctinfo,
		       unsigned char **data, InfoRequestResponse *irr)
{
	int ret;
	typeof(set_ras_addr_hook) set_ras_addr;
	typeof(set_sig_addr_hook) set_sig_addr;

	DEBUGP("nf_ct_ras: IRR\n");

	set_ras_addr = rcu_dereference(set_ras_addr_hook);
	if (set_ras_addr && ct->status & IPS_NAT_MASK) {
		ret = set_ras_addr(pskb, ct, ctinfo, data,
				   &irr->rasAddress, 1);
		if (ret < 0)
			return -1;
	}

	set_sig_addr = rcu_dereference(set_sig_addr_hook);
	if (set_sig_addr && ct->status & IPS_NAT_MASK) {
		ret = set_sig_addr(pskb, ct, ctinfo, data,
					irr->callSignalAddress.item,
					irr->callSignalAddress.count);
		if (ret < 0)
			return -1;
	}

	return 0;
}

/****************************************************************************/
static int process_ras(struct sk_buff **pskb, struct nf_conn *ct,
		       enum ip_conntrack_info ctinfo,
		       unsigned char **data, RasMessage *ras)
{
	switch (ras->choice) {
	case eRasMessage_gatekeeperRequest:
		return process_grq(pskb, ct, ctinfo, data,
				   &ras->gatekeeperRequest);
	case eRasMessage_gatekeeperConfirm:
		return process_gcf(pskb, ct, ctinfo, data,
				   &ras->gatekeeperConfirm);
	case eRasMessage_registrationRequest:
		return process_rrq(pskb, ct, ctinfo, data,
				   &ras->registrationRequest);
	case eRasMessage_registrationConfirm:
		return process_rcf(pskb, ct, ctinfo, data,
				   &ras->registrationConfirm);
	case eRasMessage_unregistrationRequest:
		return process_urq(pskb, ct, ctinfo, data,
				   &ras->unregistrationRequest);
	case eRasMessage_admissionRequest:
		return process_arq(pskb, ct, ctinfo, data,
				   &ras->admissionRequest);
	case eRasMessage_admissionConfirm:
		return process_acf(pskb, ct, ctinfo, data,
				   &ras->admissionConfirm);
	case eRasMessage_locationRequest:
		return process_lrq(pskb, ct, ctinfo, data,
				   &ras->locationRequest);
	case eRasMessage_locationConfirm:
		return process_lcf(pskb, ct, ctinfo, data,
				   &ras->locationConfirm);
	case eRasMessage_infoRequestResponse:
		return process_irr(pskb, ct, ctinfo, data,
				   &ras->infoRequestResponse);
	default:
		DEBUGP("nf_ct_ras: RAS message %d\n", ras->choice);
		break;
	}

	return 0;
}


/* RAS help */
static int ras_help(struct sk_buff **pskb, unsigned int protoff,
		    struct nf_conn *ct, enum ip_conntrack_info ctinfo)
{
	static RasMessage ras;
	unsigned char *data;
	int datalen = 0;
	int ret;

	DEBUGP("nf_ct_ras: skblen = %u\n", (*pskb)->len);

    /*start of HGW by d37981  2009.4.17 HG522V100R001C08B115  A36D06508 */
    if (!h323_enable)
    {
        return NF_ACCEPT;
    }
    /*e n d of HGW by d37981  2009.4.17 HG522V100R001C08B115  A36D06508*/

	spin_lock_bh(&nf_h323_lock);

	/* Get UDP data */
	data = get_udp_data(pskb, protoff, &datalen);
	if (data == NULL)
		goto accept;
	DEBUGP("nf_ct_ras: RAS message len=%d ", datalen);
	NF_CT_DUMP_TUPLE(&ct->tuplehash[CTINFO2DIR(ctinfo)].tuple);

	/* Decode RAS message */
	ret = DecodeRasMessage(data, datalen, &ras);
	if (ret < 0) {
		if (net_ratelimit())
			printk("nf_ct_ras: decoding error: %s\n",
			       ret == H323_ERROR_BOUND ?
			       "out of bound" : "out of range");
		goto accept;
	}

	/* Process RAS message */
	if (process_ras(pskb, ct, ctinfo, &data, &ras) < 0)
		goto drop;

      accept:
	spin_unlock_bh(&nf_h323_lock);
	return NF_ACCEPT;

      drop:
	spin_unlock_bh(&nf_h323_lock);
	if (net_ratelimit())
		printk("nf_ct_ras: packet dropped\n");
	return NF_DROP;
}

/* h323 流程:
 * 先通过 RAS 协议(UDP, 1719) 与关守进行通信，类似于注册服务
 * 呼叫的时候，首先是 H.225 协议，使用TCP(1720) 先进行会话的 SETUP
 * 然后通过 H.245 进行会话信息以及能力的通告，在 H245中会告诉对方逻辑通道等信息。
 */
static struct nf_conntrack_helper nf_conntrack_helper_ras[] __read_mostly = {
	{
		.name			= "RAS",
		.me			= THIS_MODULE,
		.max_expected		= 32,
		.timeout		= 240,
		.tuple.src.l3num	= AF_INET,
		.tuple.src.u.udp.port	= __constant_htons(RAS_PORT),
		.tuple.dst.protonum	= IPPROTO_UDP,
		.mask.src.l3num		= 0xFFFF,
		.mask.src.u.udp.port	= __constant_htons(0xFFFF),
		.mask.dst.protonum	= 0xFF,
		.help			= ras_help,
	},
	{
		.name			= "RAS",
		.me			= THIS_MODULE,
		.max_expected		= 32,
		.timeout		= 240,
		.tuple.src.l3num	= AF_INET6,
		.tuple.src.u.udp.port	= __constant_htons(RAS_PORT),
		.tuple.dst.protonum	= IPPROTO_UDP,
		.mask.src.l3num		= 0xFFFF,
		.mask.src.u.udp.port	= __constant_htons(0xFFFF),
		.mask.dst.protonum	= 0xFF,
		.help			= ras_help,
	},
};

/****************************************************************************/
static void __exit nf_conntrack_h323_fini(void)
{
	nf_conntrack_helper_unregister(&nf_conntrack_helper_ras[1]);
	nf_conntrack_helper_unregister(&nf_conntrack_helper_ras[0]);
	nf_conntrack_helper_unregister(&nf_conntrack_helper_q931[1]);
	nf_conntrack_helper_unregister(&nf_conntrack_helper_q931[0]);
	kfree(h323_buffer);
	DEBUGP("nf_ct_h323: fini\n");
}

/****************************************************************************/
static int __init nf_conntrack_h323_init(void)
{
	int ret;

	h323_buffer = kmalloc(65536, GFP_KERNEL);
	if (!h323_buffer)
		return -ENOMEM;
	ret = nf_conntrack_helper_register(&nf_conntrack_helper_q931[0]);
	if (ret < 0)
		goto err1;
	ret = nf_conntrack_helper_register(&nf_conntrack_helper_q931[1]);
	if (ret < 0)
		goto err2;
	ret = nf_conntrack_helper_register(&nf_conntrack_helper_ras[0]);
	if (ret < 0)
		goto err3;
	ret = nf_conntrack_helper_register(&nf_conntrack_helper_ras[1]);
	if (ret < 0)
		goto err4;
	DEBUGP("nf_ct_h323: init success\n");
	return 0;

err4:
	nf_conntrack_helper_unregister(&nf_conntrack_helper_ras[0]);
err3:
	nf_conntrack_helper_unregister(&nf_conntrack_helper_q931[1]);
err2:
	nf_conntrack_helper_unregister(&nf_conntrack_helper_q931[0]);
err1:
	return ret;
}

/****************************************************************************/
module_init(nf_conntrack_h323_init);
module_exit(nf_conntrack_h323_fini);

EXPORT_SYMBOL_GPL(get_h225_addr);
EXPORT_SYMBOL_GPL(set_h245_addr_hook);
EXPORT_SYMBOL_GPL(set_h225_addr_hook);
EXPORT_SYMBOL_GPL(set_sig_addr_hook);
EXPORT_SYMBOL_GPL(set_ras_addr_hook);
EXPORT_SYMBOL_GPL(nat_rtp_rtcp_hook);
EXPORT_SYMBOL_GPL(nat_t120_hook);
EXPORT_SYMBOL_GPL(nat_h245_hook);
EXPORT_SYMBOL_GPL(nat_callforwarding_hook);
EXPORT_SYMBOL_GPL(nat_q931_hook);

MODULE_AUTHOR("Jing Min Zhao <zhaojingmin@users.sourceforge.net>");
MODULE_DESCRIPTION("H.323 connection tracking helper");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ip_conntrack_h323");
