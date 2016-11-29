/*
 * RTSP extension for IP connection tracking.
 * (C) 2003 by Tom Marshall <tmarshall at real.com>
 * based on ip_conntrack_irc.h
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 */
#ifndef _IP_CONNTRACK_RTSP_H
#define _IP_CONNTRACK_RTSP_H

//#define IP_NF_RTSP_DEBUG 1
#define IP_NF_RTSP_VERSION "0.6.21"

#ifdef __KERNEL__
/* port block types */
typedef enum {
    pb_single,  /* client_port=x */
    pb_range,   /* client_port=x-y */
    pb_discon   /* client_port=x/y (rtspbis) */
} portblock_t;

/* We record seq number and length of rtsp headers here, all in host order. */

/*
 * This structure is per expected connection.  It is a member of struct
 * ip_conntrack_expect.  The TCP SEQ for the conntrack expect is stored
 * there and we are expected to only store the length of the data which
 * needs replaced.  If a packet contains multiple RTSP messages, we create
 * one expected connection per message.
 *
 * We use these variables to mark the entire header block.  This may seem
 * like overkill, but the nature of RTSP requires it.  A header may appear
 * multiple times in a message.  We must treat two Transport headers the
 * same as one Transport header with two entries.
 */
struct ip_ct_rtsp_expect
{
    u_int32_t   len;        /* length of header block */
    portblock_t pbtype;     /* Type of port block that was requested */
    u_int16_t   loport;     /* Port that was requested, low or first */
    u_int16_t   hiport;     /* Port that was requested, high or second */
#if 0
    uint        method;     /* RTSP method */
    uint        cseq;       /* CSeq from request */
#endif
    // TODO: add src/destination address 
};

extern unsigned int (*nf_nat_rtsp_hook)(struct sk_buff **pskb,
				 enum ip_conntrack_info ctinfo,
				 unsigned int matchoff, unsigned int matchlen,
				 struct ip_ct_rtsp_expect *prtspexp,
				 struct nf_conntrack_expect *exp);

/* This structure exists only once per master */
struct nf_ct_rtsp_master {
	/* The client has sent PAUSE message and not replied */
	int paused;
};

/* Single data channel */
extern int (*nat_rtsp_channel_hook) (struct sk_buff **pskb,
				     struct nf_conn *ct,
				     enum ip_conntrack_info ctinfo,
				     unsigned int matchoff,
				     unsigned int matchlen,
				     struct nf_conntrack_expect *exp,
				     int *delta);

/* A pair of data channels (RTP/RTCP) */
extern int (*nat_rtsp_channel2_hook) (struct sk_buff **pskb,
				      struct nf_conn *ct,
				      enum ip_conntrack_info ctinfo,
				      unsigned int matchoff,
				      unsigned int matchlen,
				      struct nf_conntrack_expect *rtp_exp,
				      struct nf_conntrack_expect *rtcp_exp,
				      char dash, int *delta);

/* Modify parameters like client_port in Transport for single data channel */
extern int (*nat_rtsp_modify_port_hook) (struct sk_buff **pskb,
					 struct nf_conn *ct,
			      	  	 enum ip_conntrack_info ctinfo,
			      	  	 unsigned int matchoff,
					 unsigned int matchlen,
			      	  	 __be16 rtpport, int *delta);

/* Modify parameters like client_port in Transport for multiple data channels*/
extern int (*nat_rtsp_modify_port2_hook) (struct sk_buff **pskb,
					  struct nf_conn *ct,
			       	   	  enum ip_conntrack_info ctinfo,
			       	   	  unsigned int matchoff,
					  unsigned int matchlen,
			       	   	  __be16 rtpport, __be16 rtcpport,
				   	  char dash, int *delta);

/* Modify parameters like destination in Transport */
extern int (*nat_rtsp_modify_addr_hook) (struct sk_buff **pskb,
					 struct nf_conn *ct,
				 	 enum ip_conntrack_info ctinfo,
					 int matchoff, int matchlen,
					 int *delta);

extern void (*nf_nat_rtsp_hook_expectfn)(struct nf_conn *ct, struct nf_conntrack_expect *exp);

#define RTSP_PORT   554

#endif /* __KERNEL__ */

#endif /* _IP_CONNTRACK_RTSP_H */
