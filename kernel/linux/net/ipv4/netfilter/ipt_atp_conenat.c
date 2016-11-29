/* ConeNat.  Simple mapping which alters range to a local IP address
   (depending on route). */

/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2006 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/inetdevice.h>
#include <linux/ip.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <net/protocol.h>
#include <net/ip.h>
#include <net/checksum.h>
#include <net/route.h>
#include <linux/netfilter_ipv4.h>
#ifdef CONFIG_NF_NAT_NEEDED
#include <net/netfilter/nf_nat_rule.h>
#else
#include <linux/netfilter_ipv4/ip_nat_rule.h>
#endif

#include <linux/netfilter/x_tables.h>
//#include <linux/netfilter_ipv4/ip_tables.h>

/* start for cone nat */
//  #define ASSERT_READ_LOCK(x) MUST_BE_READ_LOCKED(&nf_conntrack_lock)
//  #define ASSERT_WRITE_LOCK(x) MUST_BE_WRITE_LOCKED(&nf_conntrack_lock)
#include <linux/netdevice.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_tuple.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_helper.h>
// #include <linux/list.h>
/* start for cone nat */
#include <net/netfilter/nf_nat_rule.h>


/* start of AU4D01414 by d00107688 2009-03-14 Cone nat情况下，网关LAN侧的H323的ALG功能失效  */
#include <linux/netfilter/nf_conntrack_h323.h>
/* start of AU4D01414 by d00107688 2009-03-14 Cone nat情况下，网关LAN侧的H323的ALG功能失效  */


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Netfilter Core Team <coreteam@netfilter.org>");
MODULE_DESCRIPTION("iptables CONENAT target module");

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

/* Lock protects conat region inside conntrack */
static DEFINE_RWLOCK(conat_lock);

/* start for cone nat */
/* 此处不用大改，在预期到的 init_conntrack 中实现 */
static int atp_nat_expect(struct nf_conn *ct, struct nf_conntrack_expect *exp)   // struct nf_conn *new, struct nf_conntrack_expect *this
{
 //   printk("cone nat: yeah exp is coming...... \n");
    DEBUGP("cone nat: atp_nat_expect() to recive the first reply pkt change ct .\n");
    DEBUGP("cone nat: ct = %p, exp = %p .\n", ct , exp);
//	struct ip_conntrack_expect *exp = ct->master;
	struct nf_nat_range mr;   // = struct nf_nat_range
	unsigned int ulRet;

    /* 将其他预期到这个exp 的报文源ip 都改成 原ct 的正向目的ip??  这段应该可以去掉 
	// Change src to where master sends to 
	mr.flags = IP_NAT_RANGE_MAP_IPS;
	mr.min_ip = ct->master->tuplehash[!exp->dir].tuple.dst.u3.ip;
	mr.max_ip = ct->master->tuplehash[!exp->dir].tuple.dst.u3.ip;

	// hook doesn't matter, but it has to do source manip 
	ulRet = ip_nat_setup_info(ct, &mr, NF_IP_POST_ROUTING);
	if (ulRet != NF_ACCEPT)
		return ulRet;
    */
    
    /* 将其他预期到这个exp 的报文目的ip,port 都改成 原ct 的正向源ip, 源端口 */
	/* For DST manip, map port here to where it's expected. */
	mr.flags = IP_NAT_RANGE_MAP_IPS | IP_NAT_RANGE_PROTO_SPECIFIED;
	mr.min = exp->saved_proto;
	mr.max = exp->saved_proto;
	mr.min_ip = ct->master->tuplehash[!exp->dir].tuple.src.u3.ip;
	mr.max_ip = ct->master->tuplehash[!exp->dir].tuple.src.u3.ip;
	/* hook doesn't matter, but it has to do destination manip */

	DEBUGP("cone nat: atp_nat_expect() out .\n");
	return nf_nat_setup_info(ct, &mr, NF_IP_PRE_ROUTING);
}

static int atp_nat_help2(struct sk_buff **pskb, unsigned int protoff, struct nf_conn *ct,
	                     enum ip_conntrack_info ctinfo)
{
    DEBUGP("\n cone nat: atp_nat_help2() to create exp.\n");
    /* dir = 正向  or  反向 */
	int dir = CTINFO2DIR(ctinfo);
	struct nf_conntrack_expect *exp;

	/* 只对正向的第一个报文做处理，在此保证对每个conntrack仅做一个exp */
	if (ctinfo == IP_CT_ESTABLISHED || dir != IP_CT_DIR_ORIGINAL)
		return NF_ACCEPT;

    DEBUGP("cone nat: ctinfo = %d , dir = %d  \n", ctinfo, dir);
	DEBUGP("cone nat: help: packet[%d bytes] "
	       "%u.%u.%u.%u:%hu->%u.%u.%u.%u:%hu, "
	       "reply: %u.%u.%u.%u:%hu->%u.%u.%u.%u:%hu\n",
	       (*pskb)->len,
	       NIPQUAD(ct->tuplehash[dir].tuple.src.u3.ip),
	       ntohs(ct->tuplehash[dir].tuple.src.u.udp.port),
	       NIPQUAD(ct->tuplehash[dir].tuple.dst.u3.ip),
	       ntohs(ct->tuplehash[dir].tuple.dst.u.udp.port),
	       NIPQUAD(ct->tuplehash[!dir].tuple.src.u3.ip),
	       ntohs(ct->tuplehash[!dir].tuple.src.u.udp.port),
	       NIPQUAD(ct->tuplehash[!dir].tuple.dst.u3.ip),
	       ntohs(ct->tuplehash[!dir].tuple.dst.u.udp.port));
	/* 192.168.1.2:10000 -> 129.102.100.80:20000
	        189.1.1.9:10000 <- 129.102.100.80:20000   UDP */
	/* Create expect */
	exp = nf_conntrack_expect_alloc(ct);
	if (exp == NULL)
	{
	    DEBUGP("cone nat: exp = NULL \n");
		return NF_ACCEPT;
	}
	else
	{
	    DEBUGP("cone nat: ct = %p  exp = %p \n", ct , exp);
	}         

//  int count = 0;

    memset(&(exp->mask), 0 , sizeof(struct nf_conntrack_tuple));

    /*
    for (count = 0; count < NF_CT_TUPLE_L3SIZE; count++)
    {
        exp->mask.src.u3.all[count] = 0;
    }	
	exp->mask.src.u.all= 0;
	exp->mask.src.l3num = 0;
	*/
	exp->tuple.dst.u3.ip = ct->tuplehash[!dir].tuple.dst.u3.ip;
	exp->tuple.dst.u.udp.port = ct->tuplehash[!dir].tuple.dst.u.udp.port;
	exp->tuple.dst.protonum = ct->tuplehash[!dir].tuple.dst.protonum;
	exp->mask.dst.u3.ip = 0xFFFFFFFF;
	exp->mask.dst.u.udp.port= 0xFFFF;
	exp->mask.dst.protonum = 0xFF;
	exp->expectfn = atp_nat_expect;
	exp->helper = NULL;
	exp->dir = !dir;
	exp->flags = NF_CT_EXPECT_PERMANENT;
	exp->saved_ip = ct->tuplehash[dir].tuple.src.u3.ip;
	exp->saved_proto.udp.port = ct->tuplehash[dir].tuple.src.u.udp.port;

    DEBUGP("cone nat: create exp.dst: %u.%u.%u.%u:%hu  change to: %u.%u.%u.%u:%hu \n",
	       NIPQUAD(exp->tuple.dst.u3.ip),
	       ntohs(exp->tuple.dst.u.udp.port),
	       NIPQUAD(exp->saved_ip),
	       ntohs(exp->saved_proto.udp.port));
    
	/* Setup expect */
	nf_conntrack_expect_related(exp);

	nf_conntrack_expect_put(exp);  // 这个是后来加的，确定一下
	
	DEBUGP("cone nat: expect setup\n");

	return NF_ACCEPT;
}


/* 用来处理报文中的alg，创建  一个预期连接 exp */
static struct nf_conntrack_helper ip_conntrack_helper_atp_nat __read_mostly = {
	.name = "CONE_NAT",
	.me = THIS_MODULE,
	.tuple.src.l3num = AF_INET,
	.max_expected = 0,
	.timeout = 10 * 60 ,
	.help2 = atp_nat_help2,
};

static inline int exp_cmp(const struct nf_conntrack_expect * exp, 
                          u_int32_t ip, u_int16_t port,	u_int8_t proto)
{
    DEBUGP("cone: exp_cmp() exp.dst.ip = %u.%u.%u.%u, ip = %u.%u.%u.%u"
        "exp.dst.port = %hu, Port = %hu "
        "exp.proto = %hu, proto = %hu \n", 
	    NIPQUAD(exp->tuple.dst.u3.ip), NIPQUAD(ip),
	    ntohs(exp->tuple.dst.u.udp.port), ntohs(port),
	    ntohs(exp->tuple.dst.protonum), ntohs(proto));
    
	return exp->tuple.dst.u3.ip == ip && exp->tuple.dst.u.udp.port == port &&
	       exp->tuple.dst.protonum == proto;
}

/* saved_addr  用exp 结构中的自行扩展的变量，可以将影响减小到最小 */
static inline int exp_src_cmp(const struct nf_conntrack_expect * exp, 
	                          const struct nf_conntrack_tuple * tp)
{
    DEBUGP("cone: exp_src_cmp() exp.saved_ip = %u.%u.%u.%u, ip = %u.%u.%u.%u"
        "exp.saved_port = %hu, Port = %hu "
        "exp.proto = %hu, proto = %hu \n", 
	    NIPQUAD(exp->saved_ip), NIPQUAD(tp->src.u3.ip),
	    ntohs(exp->saved_proto.udp.port), ntohs(tp->src.u.udp.port),
	    ntohs(exp->tuple.dst.protonum), ntohs(tp->dst.protonum));
    /* Cone NAT 的目的就是将   src.ip  src.port 和 WAN 侧的 port绑定
       此处就是查找有没有这种对应关系的 exp 存在 */
	return exp->saved_ip == tp->src.u3.ip &&
	       exp->saved_proto.udp.port == tp->src.u.udp.port &&
	       exp->tuple.dst.protonum == tp->dst.protonum;
}

/* end for cone nat */

/* FIXME: Multiple targets. --RR */
static int
conenat_check(const char *tablename,
		 const void *e,
		 const struct xt_target *target,
		 void *targinfo,
		 unsigned int hook_mask)
{
	const struct ip_nat_multi_range_compat *mr = targinfo;

	if (mr->range[0].flags & IP_NAT_RANGE_MAP_IPS) {
		DEBUGP("conenat_check: bad MAP_IPS.\n");
		return 0;
	}
	if (mr->rangesize != 1) {
		DEBUGP("conenat_check: bad rangesize %u.\n", mr->rangesize);
		return 0;
	}
	return 1;
}

static unsigned int
conenat_target(struct sk_buff **pskb,
		  const struct net_device *in,
		  const struct net_device *out,
		  unsigned int hooknum,
		  const struct xt_target *target,
		  const void *targinfo)
{
    DEBUGP("\n\n cone nat: input \n");
#ifdef CONFIG_NF_NAT_NEEDED
	struct nf_conn_nat *nat;
#endif
	struct ip_conntrack *ct;
	enum ip_conntrack_info ctinfo;
	struct nf_nat_range newrange;
	const struct ip_nat_multi_range_compat *mr;
	struct rtable *rt;
	__be32 newsrc;

    /* start for cone nat */
    unsigned int iflag = 0;
	unsigned int ulRet;
    u_int16_t usMinPort, usMaxPort;
    u_int16_t usNewPort, usTmpPort;
    struct nf_conntrack_expect *exp;
    /* end for cone nat */

	IP_NF_ASSERT(hooknum == NF_IP_POST_ROUTING);

	ct = ip_conntrack_get(*pskb, &ctinfo);
#ifdef CONFIG_NF_NAT_NEEDED
	nat = nfct_nat(ct);
#endif
	IP_NF_ASSERT(ct && (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED
	                    || ctinfo == IP_CT_RELATED + IP_CT_IS_REPLY));

	/* Source address is 0.0.0.0 - locally generated packet that is
	 * probably not supposed to be conenated.
	 */
#ifdef CONFIG_NF_NAT_NEEDED
	if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip == 0)
#else
	if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip == 0)
#endif
		return NF_ACCEPT;

	mr = targinfo;
	rt = (struct rtable *)(*pskb)->dst;
	newsrc = inet_select_addr(out, rt->rt_gateway, RT_SCOPE_UNIVERSE);
	if (!newsrc) {
		printk("Cone nat: %s ate my IP address\n", out->name);
		return NF_DROP;
	}

    DEBUGP("cone nat: newsrc = %u.%u.%u.%u\n", NIPQUAD(newsrc));
    
	write_lock_bh(&conat_lock);
#ifdef CONFIG_NF_NAT_NEEDED
	nat->masq_index = out->ifindex;
#else
	ct->nat.masq_index = out->ifindex;
#endif
	write_unlock_bh(&conat_lock);

    /* start for cone nat */
	DEBUGP("cone nat: CT=%p  SrcIp = %u.%u.%u.%u, SrcPort = %hu, DstIp = %u.%u.%u.%u, DstPort = %hu \n", 
        ct,
        NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip),
	    ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.udp.port),
	    NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip),
	    ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.udp.port));
	DEBUGP("cone nat:  targInfo  MinPort = %hu, MaxPort = %hu \n", mr->range[0].min.udp.port, mr->range[0].max.udp.port);
	DEBUGP("cone nat:  proto:[%d] \n", ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum));
	/* 前向组的信息，tuple结构的src可操作，目的部分不可操作 */

    /* start of AU4D01414 by d00107688 2009-03-14 Cone nat情况下，网关LAN侧的H323的ALG功能失效 
       * 注意，如果仅仅只是为了修改H323在CONE NAT条件下无法使用，那么值需要将这个if 0 打开即可，而不需要修改
       * 下面 405 行的代码
       */
    #if 0
	if ((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum == IPPROTO_UDP)
        && (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.udp.port != __constant_htons(RAS_PORT))) 
    #else
	if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum == IPPROTO_UDP)
    #endif
    /* end of AU4D01414 by d00107688 2009-03-14 Cone nat情况下，网关LAN侧的H323的ALG功能失效  */
	{
	    DEBUGP("cone nat: process udp pkt. \n");
		/* Choose port */
		read_lock_bh(&nf_conntrack_lock);
//		read_lock_bh(&nf_conntrack_expect_tuple_lock);

        /* 参考  init_conntrack() L751 在预期列表中寻找预期 */
		/* 在预期列表中匹配 tuple 的前向预期 */
        list_for_each_entry(exp, &nf_conntrack_expect_list, list)
        {
            if (exp_src_cmp(exp, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple))
            {
                iflag = 1;
                break;
            }
        }
        
        /* 找到了这种对应关系，就要将这种绑定应用到当前的ct */
		if (iflag) 
		{
		    iflag = 0;
			usMinPort = usMaxPort = exp->tuple.dst.u.udp.port;
			/*
			printk("cone nat: existing mapped port = %hu\n",
			       ntohs(usMinPort));
			*/
		}
		else
		{		
			usMinPort = mr->range[0].min.udp.port == 0? 
				ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.udp.port : mr->range[0].min.udp.port;
			usMaxPort = mr->range[0].max.udp.port == 0? 
				htons(65535) : mr->range[0].max.udp.port;
            /* 从当前报文的端口到 65535 设置转换范围 */
			for (usNewPort = ntohs(usMinPort),usTmpPort = ntohs(usMaxPort); 
			     usNewPort <= usTmpPort; 
			     usNewPort++) 
			{
			    iflag = 0;
			    /* 查询WAN 侧的该端口  有没有被WAN侧入口的预期 */
			    list_for_each_entry(exp, &nf_conntrack_expect_list, list)
                {
                    if (exp_cmp(exp, newsrc, htons(usNewPort), ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum))
                    {
                        iflag = 1;
                        break;
                    }
                }

                if (!iflag)
                {
               //     printk("cone nat: new mapped port = %hu\n", usNewPort);
					usMinPort = usMaxPort = htons(usNewPort);
					break;
                }
			}
		}
//		read_unlock_bh(&nf_conntrack_expect_tuple_lock);
		read_unlock_bh(&nf_conntrack_lock);

		newrange = ((struct nf_nat_range)  // 稍后改回 ip_nat_range
    				{mr->range[0].flags | IP_NAT_RANGE_MAP_IPS | IP_NAT_RANGE_PROTO_SPECIFIED,
    				 newsrc, 
    				 newsrc,
    				 {.udp = {usMinPort}}, 
    				 {.udp = {usMaxPort}}
    				});
		
	    DEBUGP("cone nat: ip_nat_setup_info(cone_nat). \n");
		/* Set ct helper */
		ulRet = nf_nat_setup_info(ct, &newrange, hooknum);
		/* 设置完地址转换后，设置当前ct 的 helper, 在发包之后再建一个exp
		   why???? */
		if (ulRet == NF_ACCEPT)
		{
            /* start of AU4D01414 by d00107688 2009-03-14 Cone nat情况下，网关LAN侧的H323的ALG功能失效  */
		    /* 这儿得改，在ct 的 指针后面挂上 help 指针 */
            if (nfct_help(ct) && nfct_help(ct)->helper)
            {
                DEBUGP("Already have help %s, so won't repleace with %s", 
                    nfct_help(ct)->helper->name, ip_conntrack_helper_atp_nat.name);
            }
            else
            {
                if (nfct_help(ct))
                {
		            nfct_help(ct)->helper = &ip_conntrack_helper_atp_nat;
                }
                else
                {
                    if (net_ratelimit())
                    {
                        printk("no help for ct, so won't set cone nat helper\r\n");
                    }
                }
            }
            /* end of AU4D01414 by d00107688 2009-03-14 Cone nat情况下，网关LAN侧的H323的ALG功能失效  */
		}
		return ulRet;
	}
/* end for cone nat */

    DEBUGP("cone nat: process non-udp pkt, just masquerade. \n");
	/* Transfer from original range. */
	newrange = ((struct ip_nat_range)
		{ mr->range[0].flags | IP_NAT_RANGE_MAP_IPS,
		  newsrc, newsrc,
		  mr->range[0].min, mr->range[0].max });

	/* Hand modified range to generic setup. */
	return nf_nat_setup_info(ct, &newrange, hooknum);
}

static inline int
device_cmp(struct ip_conntrack *i, void *ifindex)
{
	int ret;
#ifdef CONFIG_NF_NAT_NEEDED
	struct nf_conn_nat *nat = nfct_nat(i);

	if (!nat)
		return 0;
#endif

	read_lock_bh(&conat_lock);
#ifdef CONFIG_NF_NAT_NEEDED
	ret = (nat->masq_index == (int)(long)ifindex);
#else
	ret = (i->nat.masq_index == (int)(long)ifindex);
#endif
/*start of 问题单:AU4D00925，连接跟踪信息清空。by 00126165 2009-10-25*/
	if (i->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum == htons(1)) //icmp
	{
	    ret = 1;
	}
/*end of 问题单:AU4D00925，连接跟踪信息清空。by 00126165 2009-10-25*/
	read_unlock_bh(&conat_lock);

	return ret;
}

static int conat_device_event(struct notifier_block *this,
			     unsigned long event,
			     void *ptr)
{
	struct net_device *dev = ptr;

	if (event == NETDEV_DOWN) {
		/* Device was downed.  Search entire table for
		   conntracks which were associated with that device,
		   and forget them. */
		IP_NF_ASSERT(dev->ifindex != 0);

		ip_ct_iterate_cleanup(device_cmp, (void *)(long)dev->ifindex);
	}

	return NOTIFY_DONE;
}

static int conat_inet_event(struct notifier_block *this,
			   unsigned long event,
			   void *ptr)
{
	struct net_device *dev = ((struct in_ifaddr *)ptr)->ifa_dev->dev;

#ifdef CONFIG_SUPPORT_ATP
    // When device is down, delete conntracks too.
	if ((event == NETDEV_UP) || (event == NETDEV_DOWN)) {
#else
	if (event == NETDEV_DOWN) {
#endif
		/* IP address was deleted.  Search entire table for
		   conntracks which were associated with that device,
		   and forget them. */
		IP_NF_ASSERT(dev->ifindex != 0);

		ip_ct_iterate_cleanup(device_cmp, (void *)(long)dev->ifindex);
	}

	return NOTIFY_DONE;
}

static struct notifier_block conat_dev_notifier = {
	.notifier_call	= conat_device_event,
};

static struct notifier_block conat_inet_notifier = {
	.notifier_call	= conat_inet_event,
};

static struct xt_target conenat = {
	.name		= "CONE_NAT",
	.family		= AF_INET,
	.target		= conenat_target,
	.targetsize	= sizeof(struct ip_nat_multi_range_compat),
	.table		= "nat",
	.hooks		= 1 << NF_IP_POST_ROUTING,
	.checkentry	= conenat_check,
	.me		= THIS_MODULE,
};

static int __init ipt_conenat_init(void)
{
	int ret;

	ret = xt_register_target(&conenat);

	if (ret == 0) {
		/* Register for device down reports */
		register_netdevice_notifier(&conat_dev_notifier);
		/* Register IP address change reports */
		register_inetaddr_notifier(&conat_inet_notifier);
	}

	return ret;
}

static void __exit ipt_conenat_fini(void)
{
	xt_unregister_target(&conenat);
	unregister_netdevice_notifier(&conat_dev_notifier);
	unregister_inetaddr_notifier(&conat_inet_notifier);	
}

module_init(ipt_conenat_init);
module_exit(ipt_conenat_fini);
