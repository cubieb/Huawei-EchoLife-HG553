/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1997, 1999, 2000, 2001 Ralf Baechle
 * Copyright (C) 2000, 2001 Silicon Graphics, Inc.
 */
#ifndef _ASM_SOCKET_H
#define _ASM_SOCKET_H

#include <asm/sockios.h>

/*
 * For setsockopt(2)
 *
 * This defines are ABI conformant as far as Linux supports these ...
 */
#define SOL_SOCKET	0xffff

#define SO_DEBUG	0x0001	/* Record debugging information.  */
#define SO_REUSEADDR	0x0004	/* Allow reuse of local addresses.  */
#define SO_KEEPALIVE	0x0008	/* Keep connections alive and send
				   SIGPIPE when they die.  */
#define SO_DONTROUTE	0x0010	/* Don't do local routing.  */
#define SO_BROADCAST	0x0020	/* Allow transmission of
				   broadcast messages.  */
#define SO_LINGER	0x0080	/* Block on close of a reliable
				   socket to transmit pending data.  */
#define SO_OOBINLINE 0x0100	/* Receive out-of-band data in-band.  */
#if 0
To add: #define SO_REUSEPORT 0x0200	/* Allow local address and port reuse.  */
#endif

#define SO_TYPE		0x1008	/* Compatible name for SO_STYLE.  */
#define SO_STYLE	SO_TYPE	/* Synonym */
#define SO_ERROR	0x1007	/* get error status and clear */
#define SO_SNDBUF	0x1001	/* Send buffer size. */
#define SO_RCVBUF	0x1002	/* Receive buffer. */
#define SO_SNDLOWAT	0x1003	/* send low-water mark */
#define SO_RCVLOWAT	0x1004	/* receive low-water mark */
#define SO_SNDTIMEO	0x1005	/* send timeout */
#define SO_RCVTIMEO 	0x1006	/* receive timeout */
#define SO_ACCEPTCONN	0x1009

/* linux-specific, might as well be the same as on i386 */
#define SO_NO_CHECK	11
#define SO_PRIORITY	12
#define SO_BSDCOMPAT	14

#define SO_PASSCRED	17
#define SO_PEERCRED	18

/* Security levels - as per NRL IPv6 - don't actually do anything */
#define SO_SECURITY_AUTHENTICATION		22
#define SO_SECURITY_ENCRYPTION_TRANSPORT	23
#define SO_SECURITY_ENCRYPTION_NETWORK		24

#define SO_BINDTODEVICE		25

/* Socket filtering */
#define SO_ATTACH_FILTER        26
#define SO_DETACH_FILTER        27

#define SO_PEERNAME             28
#define SO_TIMESTAMP		29
#define SCM_TIMESTAMP		SO_TIMESTAMP

#define SO_PEERSEC		30

/* start of 2008-11-24 add by d00107688 to QoS */
/* 注意: 这里内核默认本来是 31 的， 但是在 2.6.8 中并没有这个宏的存在，但是在2.6.8中QoS打mark
 * 已经用了这个值，因此这里将标准内核的该值进行修改成66，从而使得QoS打标记和2.6.8全部一致
 * 因此组件不需要做任何修改。
 */
#if 0
#define SO_SNDBUFFORCE		31
#else
#define SO_SNDBUFFORCE      66
#endif
/* end of 2008-11-24 add by d00107688 to QoS */

#define SO_RCVBUFFORCE		33
#define SO_PASSSEC		34


/* start of 2008-11-24 add by d00107688 to QoS */
#if defined(CONFIG_NETFILTER) && defined(CONFIG_ATP_QOS_NETFILTER)
#define SO_NFMARK       31
#else
#define SO_NFMARK       31
#endif

/* 目前暂时语音全部打上同一个mark，后续如果需要针对具体某种协议进行不同的mark，那么可以在
 * 这里修改，先预留出各种协议的宏，到代码中各种不同协议使用这个里面定义的不同的宏就好了
 */
#define NFMARK_VOIP     0xA0
#define NFMARK_SIP      0xA0
#define NFMARK_SIPALG   0xA0
#define NFMARK_RTP      0xA0
#define NFMARK_RTCP     0xA0
#define NFMARK_H323     0xA0
/* TR069 */
#define NFMARK_TR069    0x18
/* IPTV */
#define NFMARK_IPTV     0xB0
#define NFMARK_RTSP     0xB0
/* PPP */
#define NFMARK_PPP      0xB5
/* IGMP */
#define NFMARK_IGMP     0xB6
/* DNS */
#define NFMARK_DNS      0xB7
/* DHCP */
#define NFMARK_DHCP     0xB8
/* FTP */
#define NFMARK_FTP      0xB9

/* end of 2008-11-24 add by d00107688 to QoS */



#ifdef __KERNEL__

/** sock_type - Socket types
 *
 * Please notice that for binary compat reasons MIPS has to
 * override the enum sock_type in include/linux/net.h, so
 * we define ARCH_HAS_SOCKET_TYPES here.
 *
 * @SOCK_DGRAM - datagram (conn.less) socket
 * @SOCK_STREAM - stream (connection) socket
 * @SOCK_RAW - raw socket
 * @SOCK_RDM - reliably-delivered message
 * @SOCK_SEQPACKET - sequential packet socket
 * @SOCK_PACKET - linux specific way of getting packets at the dev level.
 *		  For writing rarp and other similar things on the user level.
 */
enum sock_type {
	SOCK_DGRAM	= 1,
	SOCK_STREAM	= 2,
	SOCK_RAW	= 3,
	SOCK_RDM	= 4,
	SOCK_SEQPACKET	= 5,
	SOCK_DCCP	= 6,
	SOCK_PACKET	= 10,
};

#define SOCK_MAX (SOCK_PACKET + 1)

#define ARCH_HAS_SOCKET_TYPES 1

#endif /* __KERNEL__ */

#endif /* _ASM_SOCKET_H */
