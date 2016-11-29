/* serverpacket.c
 *
 * Constuct and send DHCP server packets
 *
 * Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#include "packet.h"
#include "debug.h"
#include "dhcpd.h"
#include "options.h"
#include "leases.h"


/*start of  增加vdf ipphone dhcp 功能 by s53329 at  20090407*/
#define  BUFSIZE (1024+1)
extern int  bIpphone;
#define DOMAIN_NAME "localdomain"
extern int  ipPhoneipaddr[255];
int g_EnableQosVlan = 0;
/*end of  增加vdf ipphone dhcp 功能 by s53329 at  20090407*/
/* BEGIN: Added by weishi kf33269, 2011/1/11   PN:AU8D04039 : IP phone 带vlan时如配置IP phone的 DHCP reservation,分配地址时响应的地址接口有误*/
extern int  UseIpPhoneVlan;
/* END:   Added by weishi kf33269, 2011/1/11 */
/* send a packet to giaddr using the kernel ip stack */
static int send_packet_to_relay(struct dhcpMessage *payload)
{
	DEBUG(LOG_INFO, "Forwarding packet to relay");

	return kernel_packet(payload, server_config.server, SERVER_PORT,
			payload->giaddr, SERVER_PORT, NULL);
}


/* send a packet to a specific arp address and ip address by creating our own ip packet */
static int send_packet_to_client(struct dhcpMessage *payload, int force_broadcast)
{
	u_int32_t ciaddr;
	char chaddr[6];
	
	if (force_broadcast) {
		DEBUG(LOG_INFO, "broadcasting packet to client (NAK)");
		ciaddr = INADDR_BROADCAST;
		memcpy(chaddr, MAC_BCAST_ADDR, 6);		
	} else if (payload->ciaddr) {
		DEBUG(LOG_INFO, "unicasting packet to client ciaddr");
		ciaddr = payload->ciaddr;
		memcpy(chaddr, payload->chaddr, 6);
	} else if (ntohs(payload->flags) & BROADCAST_FLAG) {
		DEBUG(LOG_INFO, "broadcasting packet to client (requested)");
		ciaddr = INADDR_BROADCAST;
		memcpy(chaddr, MAC_BCAST_ADDR, 6);		
	} else {
		DEBUG(LOG_INFO, "unicasting packet to client yiaddr");
        /*w44771 modify for A36D02260, 应该广播ack报文，begin, 2006-7-7*/
        //ciaddr = payload->yiaddr;
		ciaddr = INADDR_BROADCAST;        
	    /*w44771 modify for A36D02260, 应该广播ack报文，end, 2006-7-7*/

		memcpy(chaddr, payload->chaddr, 6);
	}
	return raw_packet(payload, server_config.server, SERVER_PORT, 
			ciaddr, CLIENT_PORT, chaddr, server_config.ifindex);
}


/* send a dhcp packet, if force broadcast is set, the packet will be broadcast to the client */
static int send_packet(struct dhcpMessage *payload, int force_broadcast)
{
	int ret;

	if (payload->giaddr)
		ret = send_packet_to_relay(payload);
	else ret = send_packet_to_client(payload, force_broadcast);
	return ret;
}


static void init_packet(struct dhcpMessage *packet, struct dhcpMessage *oldpacket, char type)
{
	memset(packet, 0, sizeof(struct dhcpMessage));
	
	packet->op = BOOTREPLY;
	packet->htype = ETH_10MB;
	packet->hlen = ETH_10MB_LEN;
	packet->xid = oldpacket->xid;
	memcpy(packet->chaddr, oldpacket->chaddr, 16);
	packet->cookie = htonl(DHCP_MAGIC);
	packet->options[0] = DHCP_END;
	packet->flags = oldpacket->flags;
	packet->giaddr = oldpacket->giaddr;
	packet->ciaddr = oldpacket->ciaddr;
	add_simple_option(packet->options, DHCP_MESSAGE_TYPE, type);
	add_simple_option(packet->options, DHCP_SERVER_ID, ntohl(server_config.server)); /* expects host order */
}


void fileoption6(char *szOption6, char *dns)
{
    int iLen = 0;
    int i = 0;
    char *pToken =NULL, *pLast=NULL;
    char buf[BUFSIZE];
    long  addr;
    
    if(NULL == szOption6 || NULL == dns )
    {
        return;
    }
    strncpy(buf, dns,COMMONOPTIONLEN_VDF);

    for ( i = 0, pToken = strtok_r(buf, ",", &pLast);
          pToken != NULL ;
          i++, pToken = strtok_r(NULL, ",", &pLast) )
    {
        addr = inet_addr(pToken); 
        memcpy(&szOption6[OPT_VALUE]+i*4, &addr, 4);
    }

    iLen = i*4;
    szOption6[OPT_CODE] = DHCP_DNS_SERVER;
    szOption6[OPT_LEN] = iLen;
	
}

void fileoption120ipaddr(char *szOption120, char *dns)
{
    int iLen = 0;
    int i = 0;
    char *pToken =NULL, *pLast=NULL;
    char buf[BUFSIZE];
    long  addr;
    
    if(NULL == szOption120 || NULL == dns )
    {
        return;
    }
    strncpy(buf, dns,COMMONOPTIONLEN_VDF);

    for ( i = 0, pToken = strtok_r(buf, ",", &pLast);
          pToken != NULL ;
          i++, pToken = strtok_r(NULL, ",", &pLast) )
    {
        addr = inet_addr(pToken); 
        memcpy(&szOption120[OPT_VALUE]+1+i*4, &addr, 4);
    }

    iLen = i*4;
    szOption120[OPT_CODE] = DHCP_OUTBOUND_PROXY;
    szOption120[OPT_LEN] = iLen+1;
    szOption120[OPT_VALUE] = 0x01;
	
}


void fileoption120Domain(char *szOption120, char *dns)
{
    int iLen = 0;
    int TotalLen = 0;
    int iDomainLen = 0;
    int i = 0, j =0;
    char *pToken =NULL, *pLast=NULL;
    char buf[BUFSIZE];
    char domain[BUFSIZE];
    char tmp[BUFSIZE];
    char tmpoption[BUFSIZE];
    char *p = NULL;
    char *ptmp = NULL;
    
    if(NULL == szOption120 || NULL == dns )
    {
        return;
    }
    strncpy(buf, dns,COMMONOPTIONLEN_VDF);

    for ( i = 0, pToken = strtok_r(buf, ",", &pLast);
          pToken != NULL ;
          i++, pToken = strtok_r(NULL, ",", &pLast) )
    {
       memset(tmp, 0, BUFSIZE);
       memset(domain, 0, BUFSIZE);
       strcpy(tmp, pToken);
       iDomainLen = strlen(tmp);
       p = tmp;
       j = 0;
      
       while((ptmp = strchr(p, '.'))!=NULL)
       {
           *ptmp = '\0';
           iLen = strlen(p);
           memset(tmpoption, 0, BUFSIZE);
           tmpoption[0] = iLen;
           memcpy(&(tmpoption[1]),p, iLen);
           memcpy(&(domain[j]), tmpoption, iLen+1);
           j= j+iLen+1;
           p = ptmp+1;


       }

       if((ptmp = strchr(p, '.')) == NULL)
       {
          iLen = strlen(p);
          domain[j] = iLen;
          memcpy(&(domain[j+1]), p, iLen);
       }
       iDomainLen = iDomainLen+2;
          memcpy(&(szOption120[OPT_VALUE+1])+TotalLen, domain, iDomainLen);
       TotalLen+=iDomainLen;
        
    }
    
    szOption120[OPT_CODE] = DHCP_OUTBOUND_PROXY;

	/* BEGIN: Modified by w00135358, 2010/01/13: length 最大程度为0xFF*/
	if(TotalLen+1 > 255)
	{
		szOption120[OPT_LEN] = 255;
	}
	else
	/* END: Modified by w00135358, 2010/01/13: length 最大程度为0xFF*/
	
	{
    	szOption120[OPT_LEN] = TotalLen+1;
	}
    szOption120[OPT_VALUE] = 0x00;
	
}

/* add in the bootp options */
static void add_bootp_options(struct dhcpMessage *packet)
{
	packet->siaddr = server_config.siaddr;
	if (server_config.sname)
		strncpy(packet->sname, server_config.sname, sizeof(packet->sname) - 1);
	if (server_config.boot_file)
		strncpy(packet->file, server_config.boot_file, sizeof(packet->file) - 1);
}
	

/* send a DHCP OFFER to a DHCP DISCOVER */
int sendOffer(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;
	struct dhcpOfferedAddr *lease = NULL;
	u_int32_t req_align, lease_time_align = server_config.lease;
	char *req, *lease_time;
	struct option_set *curr;
	struct in_addr addr;
#ifdef VDF_OPTION
//add for option125
        char VIinfo[VENDOR_IDENTIFYING_INFO_LEN];
        unsigned char StrOption121[OPTION121_LEN];
#endif
	/*start A36D02806, s60000658, 20060906*/
	struct option_set *opt = NULL;
	u_int32_t router_ip = 0;
	
    if(NULL != (opt = find_option(server_config.options, DHCP_ROUTER)))
    {
        router_ip = *(u_int32_t*)(opt->data + 2);
    }
    /*end A36D02806, s60000658, 20060906*/
	init_packet(&packet, oldpacket, DHCPOFFER);
	
#ifdef SUPPORT_MACMATCHIP
/* BEGIN: Modified by weishi kf33269, 2011/1/11   PN:AU8D04039 : IP phone 带vlan时如配置IP phone的 DHCP reservation,分配地址时响应的地址接口有误*/
        if (( !ismacmatch(oldpacket->chaddr))&& (0 == UseIpPhoneVlan) )
        {  
/* END:   Modified by weishi kf33269, 2011/1/11 */
			packet.yiaddr = find_matchip(oldpacket->chaddr);           
		} 
        else
#endif	
	/* the client is in our lease/offered table */
	if ((lease = find_lease_by_chaddr(oldpacket->chaddr))) {
        /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
		if (!lease_expired(lease)) 
			lease_time_align = lease->expires - getSysUpTime();
        /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
		packet.yiaddr = lease->yiaddr;
		
	/* Or the client has a requested ip */
	} else if ((req = get_option(oldpacket, DHCP_REQUESTED_IP)) &&

		   /* Don't look here (ugly hackish thing to do) */
		   memcpy(&req_align, req, 4) &&

		   /* and the ip is in the lease range */
		   ntohl(req_align) >= ntohl(server_config.start) &&
		   ntohl(req_align) <= ntohl(server_config.end) &&
		   #ifdef	VDF_RESERVED
		   //and the ip is not the reserved ip
		   reservedIp(req_align)&&
		   #endif
           #ifdef SUPPORT_MACMATCHIP
/* BEGIN: Modified by weishi kf33269, 2011/1/11   PN:AU8D04039 : IP phone 带vlan时如配置IP phone的 DHCP reservation,分配地址时响应的地址接口有误*/
           isipmatch(req_align) &&(0 == UseIpPhoneVlan)&&
/* END:   Modified by weishi kf33269, 2011/1/11 */
           #endif
		   /* and its not already taken/offered */
		   ((!(lease = find_lease_by_yiaddr(req_align)) ||
		   
		   /* or its taken, but expired */
		   lease_expired(lease)))) {

#ifdef SUPPORT_DHCP_FRAG
                         int tmpNum = 1, i =0;
			      if(1 == inmaster)
				{
				   for(i = 0; i < 5; i++)
				   {
				       if(ntohl(req_align) >= ntohl(ipPool[i].start) &&
		                       ntohl(req_align) <= ntohl(ipPool[i].end))
				       {
				           tmpNum = 0;
				           break;
				       }
				   }
				}
#endif
     				/*start of DHCP 网关会分配自己的维护IP porting by w44771 20060505*/
				/*packet.yiaddr = req_align;			*/
				if((req_align != server_config.server)
#ifdef SUPPORT_DHCP_FRAG
                          && (req_align != router_ip) && (1 == tmpNum))/*A36D02806, s60000658, 20060906*/
#else
                          && (req_align != router_ip))/*A36D02806, s60000658, 20060906*/
#endif
                          {				    
				    packet.yiaddr = req_align; 
				}
			    else
				{					
#ifdef SUPPORT_DHCP_FRAG
				   if(1 == inmaster)
				   {
				    packet.yiaddr = find_address2(0);
				   }
				   else
#endif
				    packet.yiaddr = find_address(0);
				}
     				/*end of DHCP 网关会分配自己的维护IP porting by w44771 20060505*/


	/* otherwise, find a free IP */
	} else {
#ifdef SUPPORT_DHCP_FRAG
		 if(1 == inmaster)
		{
		    packet.yiaddr = find_address2(0);
		}
		else
#endif
		packet.yiaddr = find_address(0);
		
		/* try for an expired lease */
		if (!packet.yiaddr) 
		{
#ifdef SUPPORT_DHCP_FRAG
			 if(1 == inmaster)
			{
			    packet.yiaddr = find_address2(1);
			}
			else
#endif
			packet.yiaddr = find_address(1);
		}
	}
	
	if(!packet.yiaddr) {
		LOG(LOG_WARNING, "no IP addresses to give -- OFFER abandoned");
		return -1;
	}

	#ifdef SUPPORT_CHINATELECOM_DHCP
	if (!add_lease(packet.chaddr, packet.yiaddr, server_config.offer_time, 0)) {
	#else
	if (!add_lease(packet.chaddr, packet.yiaddr, server_config.offer_time)) {
	#endif
		LOG(LOG_WARNING, "lease pool is full -- OFFER abandoned");
		return -1;
	}		

	if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME))) {
		memcpy(&lease_time_align, lease_time, 4);
		lease_time_align = ntohl(lease_time_align);
		if (lease_time_align > server_config.lease) 
			lease_time_align = server_config.lease;
	}

	/* Make sure we aren't just using the lease time from the previous offer */
	if (lease_time_align < server_config.min_lease) 
		lease_time_align = server_config.lease;

	//w44771 add for 全新分配每次下发的租期,begin
#ifdef SUPPORT_DHCP_FRAG
      char *vendorid_tmp = NULL;
	int tmpIndex = 0;
	
	vendorid_tmp = get_option(oldpacket, DHCP_VENDOR);

        /*w44771 add for test CNTEL DHCP, begin*/
        #if 0
        char tmp60[64];
        memset(tmp60, 0, sizeof(tmp60));
        
        if(vendorid_tmp != NULL)
        {
            memcpy(tmp60, vendorid_tmp, *(vendorid_tmp - 1));            
            if(0 == strcmp(tmp60, "MSFT 5.0"))
            {
                memset(tmp60, 0, sizeof(tmp60));
                
                tmp60[0] = 60;
                tmp60[1] = 12;
                tmp60[2] = 'C';
                tmp60[3] = 'T';
                tmp60[4] = 1;
                tmp60[5] = 8;
                strcat(tmp60, "MSFT 5.0");

                vendorid_tmp = tmp60 + 2;                
            }
        }        
        #endif
        /*w44771 add for test CNTEL DHCP, end*/
	
	#ifdef SUPPORT_CHINATELECOM_DHCP
	cOption60Cont[0] = '\0';
	memset(cOption60Cont, 0, sizeof(cOption60Cont));
	if(NULL != vendorid_tmp)
	{
	    memcpy(cOption60Cont, vendorid_tmp - 2, (*(vendorid_tmp-1)) + 2);
	}	
	#endif
	
	if(g_enblSrv2 && NULL != server2_config.vendorid && NULL != vendorid_tmp 
            && strlen(server2_config.vendorid) == *(vendorid_tmp - 1)  && !memcmp(vendorid_tmp, server2_config.vendorid, *(vendorid_tmp - 1)))
	{
		    lease_time_align = server2_config.lease;
		    //printf("===>send offer, use server2_config.lease= 0x%08x\n", server2_config.lease);
	}
	else
	{
		for(tmpIndex = 0; tmpIndex < 5; tmpIndex++)
		{
		  if(ntohl(packet.yiaddr) >= ntohl(ipPool[tmpIndex].start) &&
		     ntohl(packet.yiaddr) <= ntohl(ipPool[tmpIndex].end) )
		  	{
		  	    lease_time_align = ipPool[tmpIndex].lease;
		  	    //printf("===>send offer, use ipPool[%d].lease= 0x%08x\n", tmpIndex, ipPool[tmpIndex].lease);
		  	    break;
		  	}
		}
		if(5 == tmpIndex)
		{
		    lease_time_align = master_lease;
		    //printf("===>send offer, use master lease, master_lease=0x%08x\n",master_lease);
		}
	}
#endif
	//w44771 add for 全新分配每次下发的租期,end
		
	add_simple_option(packet.options, DHCP_LEASE_TIME, lease_time_align);
/*j00100803 Add Begin 2008-04-15 for dhcp option58,59 */
#ifdef SUPPORT_VDF_DHCP
    add_simple_option(packet.options, DHCP_T1, lease_time_align / 2);
    add_simple_option(packet.options, DHCP_T2, (lease_time_align * 0x07) >> 3);
#endif
/*j00100803 Add End 2008-04-15 for dhcp option58,59  */
	curr = server_config.options;
        /* BEGIN: Modified by y67514, 2008/5/22 Vista无法获取地址*/
        if ( flag_option121)
        {
            //option3和邋Option121不能同时下发
            while (curr) 
            {
                /*j00100803 Add Begin 2008-04-15 for dhcp option58,59 */
#ifdef SUPPORT_VDF_DHCP
                if ((curr->data[OPT_CODE] != DHCP_LEASE_TIME) &&
                    (curr->data[OPT_CODE] != DHCP_T1) &&
                    (curr->data[OPT_CODE] != DHCP_T2)&&
                    (curr->data[OPT_CODE] != DHCP_STATIC_ROUTE)&&
                    (curr->data[OPT_CODE] != DHCP_ROUTER))
#else
                if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
#endif
                /*j00100803 Add End 2008-04-15 for dhcp option58,59  */
                {
                    if(!((curr->data[OPT_CODE] == DHCP_DNS_SERVER) &&(bIpphone ==1))
                        && !((curr->data[OPT_CODE] == DHCP_NTP_SERVER) &&(bIpphone ==1)))
                    add_option_string(packet.options, curr->data);
                }
                curr = curr->next;
            }
            StrOption121[OPT_CODE] = DHCP_STATIC_ROUTE;
            StrOption121[OPT_LEN] = 5;
            StrOption121[OPT_VALUE] = 0;
            memcpy(&StrOption121[OPT_VALUE + 1], &server_config.server, 4);
            add_option_string(packet.options, StrOption121);
        }
        else
        {
            while (curr) 
            {
                /*j00100803 Add Begin 2008-04-15 for dhcp option58,59 */
#ifdef SUPPORT_VDF_DHCP
                if ((curr->data[OPT_CODE] != DHCP_LEASE_TIME) &&
                    (curr->data[OPT_CODE] != DHCP_T1) &&
                    (curr->data[OPT_CODE] != DHCP_T2)&&
                    (curr->data[OPT_CODE] != DHCP_STATIC_ROUTE))
#else
                if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
#endif
                /*j00100803 Add End 2008-04-15 for dhcp option58,59  */
                {
                    if(!((curr->data[OPT_CODE] == DHCP_DNS_SERVER) &&(bIpphone ==1))
                        && !((curr->data[OPT_CODE] == DHCP_NTP_SERVER) &&(bIpphone ==1)))
                    add_option_string(packet.options, curr->data);
                }
                curr = curr->next;
            }
        }
        /* END:   Modified by y67514, 2008/5/22 */
	
	#ifdef SUPPORT_CHINATELECOM_DHCP
	if(cOption60Cont[0] != '\0')
	{
	    add_option_string(packet.options, cOption60Cont);
	}
	#endif


	add_bootp_options(&packet);
#ifdef VDF_OPTION
//add for option125
        /* if DHCPDISCOVER from client has device identity, send back gateway identity */
	if ((req = get_option(oldpacket, DHCP_VENDOR_IDENTIFYING))) 
	{
		if (createVIoption(VENDOR_IDENTIFYING_FOR_GATEWAY,VIinfo) != -1)
		{
			add_option_string(packet.options,VIinfo);
		}
	}
#endif
	addr.s_addr = packet.yiaddr;
	/* j00100803 Add Begin 2008-07-21 for dhcp option12,15 request*/
	unsigned char *szRequestList = NULL;
	int k = 0, req_num = 0;
	char szOption12[128], szOption15[128];
	char szHostName[32];
    char vdfoption[BUFSIZE];
	int iLen = 0;
	int m = 0;
	memset(szOption12, 0, sizeof(szOption12));
	memset(szOption15, 0, sizeof(szOption15));
	memset(szHostName, 0, sizeof(szHostName));

	strcpy(szHostName, inet_ntoa(addr));
	szRequestList = get_option(oldpacket, DHCP_PARAM_REQ);
	if(szRequestList)
	{
	    req_num = *(szRequestList -1);
	    for ( k = 0 ; k < req_num; k++ )
	    {
	        if(DHCP_HOST_NAME == szRequestList[k])
	        {
	        	iLen = strlen(szHostName);
				for(m = 0; m < iLen; m++)
				{
					if(szHostName[m] == '.')
					{
						szHostName[m] = '-';
					}
				}
				szOption12[OPT_CODE] = DHCP_HOST_NAME;
				szOption12[OPT_LEN] = iLen;
				memcpy(&szOption12[OPT_VALUE], szHostName, iLen);
				add_option_string(packet.options, szOption12);
	        }
			if(DHCP_DOMAIN_NAME == szRequestList[k] && bIpphone == 0)
			{
				iLen = strlen(DOMAIN_NAME) + 1;
				szOption15[OPT_CODE] = DHCP_DOMAIN_NAME;
				szOption15[OPT_LEN] = iLen;
				memcpy(&szOption15[OPT_VALUE], DOMAIN_NAME, iLen);
				add_option_string(packet.options, szOption15);
			}
	    }
	}
	/* j00100803 Add End 2008-07-21 for dhcp option12,15 request*/
    

    if(bIpphone == 1)
    {
        if(server_config.pIPPhoneDomainName != NULL)
        {
            memset(vdfoption, 0 , BUFSIZE);
            iLen = strlen(server_config.pIPPhoneDomainName) + 1;
            vdfoption[OPT_CODE] = DHCP_DOMAIN_NAME;
            vdfoption[OPT_LEN] = iLen;
			memcpy(&vdfoption[OPT_VALUE], server_config.pIPPhoneDomainName, iLen);
			add_option_string(packet.options, vdfoption);
        }
        if(server_config.pIPPhoneDNSServers != NULL)
        {
            memset(vdfoption, 0 , BUFSIZE);
            fileoption6(vdfoption, server_config.pIPPhoneDNSServers);
            add_option_string(packet.options, vdfoption);
        }

        if(server_config.pIPPhoneTFTPServer != NULL)
        {
            memset(vdfoption, 0 , BUFSIZE);
            iLen = strlen(server_config.pIPPhoneTFTPServer) + 1;
            vdfoption[OPT_CODE] = DHCP_TFTP_SERVER_VDF;
            vdfoption[OPT_LEN] = iLen;
			memcpy(&vdfoption[OPT_VALUE], server_config.pIPPhoneTFTPServer, iLen);
			add_option_string(packet.options, vdfoption);
        }
        else if(server_config.pIPPhoneTFTPServerName != NULL)
        {
            memset(vdfoption, 0 , BUFSIZE);
            iLen = strlen(server_config.pIPPhoneTFTPServerName) + 1;
            vdfoption[OPT_CODE] = DHCP_TFTP_SERVER_VDF;
            vdfoption[OPT_LEN] = iLen;
			memcpy(&vdfoption[OPT_VALUE], server_config.pIPPhoneTFTPServerName, iLen);
			add_option_string(packet.options, vdfoption);
        }

        if(server_config.pOutBoundProxy != NULL)
        {
            memset(vdfoption, 0 , BUFSIZE);
            
            if(ValidateIpAddress(server_config.pOutBoundProxy))
            {
                fileoption120ipaddr(vdfoption, server_config.pOutBoundProxy);
            }
            else
            {
                fileoption120Domain(vdfoption, server_config.pOutBoundProxy);
            }
           
			add_option_string(packet.options, vdfoption);
        }

        if(szRequestList)
        {
            req_num = *(szRequestList -1);
            for ( k = 0 ; k < req_num; k++ )
            {
                if(DHCP_NTP_SERVER == szRequestList[k])
                {
                    if(server_config.iPPhoneNtp1 != 0)
                    {
                        memset(vdfoption, 0 , BUFSIZE);
                        
                        vdfoption[OPT_CODE] = DHCP_NTP_SERVER;
                        vdfoption[OPT_LEN] = 4;
                        memcpy(&vdfoption[OPT_VALUE], &(server_config.iPPhoneNtp1), sizeof(server_config.iPPhoneNtp1));
                        if(server_config.iPPhoneNtp2 != 0)
                        {
                            memcpy(&vdfoption[OPT_VALUE + 4], &(server_config.iPPhoneNtp2), sizeof(server_config.iPPhoneNtp2));
                            vdfoption[OPT_LEN] += 4;
                        }
                        add_option_string(packet.options, vdfoption);
                    }
                    break;
                }
           }
      }
    }
	
	LOG(LOG_INFO, "sending OFFER of %s", inet_ntoa(addr));
	return send_packet(&packet, 0);
}


int sendNAK(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;

	init_packet(&packet, oldpacket, DHCPNAK);
#ifdef SUPPORT_VDF_DHCP
    char szNotifyMsgOption56[128];
    szNotifyMsgOption56[0] = DHCP_MESSAGE;
    szNotifyMsgOption56[1] = 4;
    szNotifyMsgOption56[2] = '0';
    szNotifyMsgOption56[3] = '0';
    szNotifyMsgOption56[4] = '0';
    szNotifyMsgOption56[5] = '\0';
    add_option_string(packet.options, szNotifyMsgOption56);
#endif
/* j00100803 Add End 2008-03-10 */ 
	DEBUG(LOG_INFO, "sending NAK");
	return send_packet(&packet, 1);
}

int ValidateIpAddress(char *pcIpAddr) 
{
    int  i = 0;
    int    bRet = 0;
    char    *pcToken = NULL;
    char    *pcLast = NULL; 
    char    *pcEnd = NULL;
    char    acBuf[16];
    int     lNum = 0;

    if (pcIpAddr == NULL)
    {
        return bRet;
    }

    memset(acBuf, 0, sizeof(acBuf));
    
    strncpy(acBuf, pcIpAddr, 15);
    if((pcToken = strchr(acBuf, ','))!= NULL)
    {
        *pcToken = '\0';
    }

    pcToken = strtok_r(acBuf, ".", &pcLast);
    if ( pcToken == NULL )
    {
        return bRet;
    }
    /* start of 避免使用strtol函数时，将 "+123" 等效为 "123" 转换成整形123 */
    if (*pcToken == '+' || *pcToken == '-')
    {
        return bRet;
    }
    /* end of 避免使用strtol函数时，将 "+123" 等效为 "123" 转换成整形123 */
    lNum = strtol(pcToken, &pcEnd, 10);    

    if((*pcEnd == '\0') && (lNum <= 255) ) 
    {
        for ( i = 0; i < 3; i++ ) 
        {
            pcToken = strtok_r(NULL, ".", &pcLast);
            if ( pcToken == NULL )
            {
                break;
            }
            /* start of 避免使用strtol函数时，将 "+123" 等效为 "123" 转换成整形123 */
            if ((*pcToken == '+') || (*pcToken == '-'))
            {
                break;
            }
            /* end of 避免使用strtol函数时，将 "+123" 等效为 "123" 转换成整形123 */
            lNum = strtol(pcToken, &pcEnd, 10);
            if ( (*pcEnd != '\0') || (lNum > 255) )
            {
                break;
            }
        }
       
        if ( i == 3 )
        {
            bRet = 1;
        }
    }

    return bRet;
}

void addSipAddr(u_int32_t yiaddr)
{
    int i = 0;
    while(ipPhoneipaddr[i]!= 0)
    {
        if(ipPhoneipaddr[i] == yiaddr)
        {
            return;
        }
        i++;
    }
    ipPhoneipaddr[i] = yiaddr;

}

int  findSipaddr(u_int32_t yiaddr)
{
    int i = 0;
    while(ipPhoneipaddr[i]!= 0)
    {
        if(ipPhoneipaddr[i] == yiaddr)
        {
            return  1;
        }
        i++;
    }
    return 0;
}
void BcmMERDataRouteRuleGW(char *srcIp)
{
    char szCmdMark[VENDOR_IDENTIFYING_OPTION_CODE];

    //设置IP phone报文的QoS优先级为最高:nfmark的低8位为0x13
    int iMark = 0x11000013;

    /* start of HG553_protocal : Added by c65985, 2009.12.23  static nat HG553V100R001 */
    char enable = 0;
    FILE *file = NULL;

    file = fopen("/var/staticnatenable", "r");
    if (NULL != file)
    {
        enable = fgetc(file);
        fclose(file);
    }
    /* end of HG553_protocal : Added by c65985, 2009.12.23  static nat HG553V100R001 */
    //ntp报文(dport:123)不走语音PVC, 所以不打mark
    sprintf(szCmdMark, "iptables -t mangle -A PREROUTING -s %s  -j MARK --set-mark %d/0xff0000ff", srcIp, iMark);
    system(szCmdMark);

    /* start of HG553_protocal : Added by c65985, 2009.12.23  static nat HG553V100R001 */
    if ('1' != enable)
    {
        //except ntp
        sprintf(szCmdMark, "iptables -t mangle -A PREROUTING -s %s -p udp --dport  123 -j MARK --set-mark 0x0/0xff0000ff", srcIp);
        system(szCmdMark);

        //except tftp
        sprintf(szCmdMark, "iptables -t mangle -A PREROUTING -s %s -p udp --dport  69 -j MARK --set-mark 0x0/0xff0000ff", srcIp);
        system(szCmdMark);
    }
    /* end of HG553_protocal : Added by c65985, 2009.12.23  static nat HG553V100R001 */

    //清除连接跟踪表，避免语音接口出去的报文使用数据的IP 地址
    system("echo 9 > /proc/sys/net/ipv4/netfilter/ip_conntrack_dns");
}

void bcmEnableQosVlan()
{
    char szCmdMark[VENDOR_IDENTIFYING_OPTION_CODE];
    if(0 == g_EnableQosVlan)
    {
        sprintf(szCmdMark, "ethctl eth0 qos enable");
        system(szCmdMark);
        g_EnableQosVlan = 1;
    }
}

int sendACK(struct dhcpMessage *oldpacket, u_int32_t yiaddr)
{
	struct dhcpMessage packet;
	struct option_set *curr;
	char *lease_time;
	u_int32_t lease_time_align = server_config.lease;
	struct in_addr addr;
    char vdfoption[BUFSIZE];
#ifdef VDF_OPTION
//add for option125
        char VIinfo[VENDOR_IDENTIFYING_INFO_LEN];
        char *req;
        unsigned char StrOption121[OPTION121_LEN];
#endif

	init_packet(&packet, oldpacket, DHCPACK);
	packet.yiaddr = yiaddr;

	//w44771 add for 全新分配每次下发的租期,begin
#ifdef SUPPORT_DHCP_FRAG
      char *vendorid_tmp = NULL;
	int tmpIndex = 0;
	
	vendorid_tmp = get_option(oldpacket, DHCP_VENDOR);

        /*w44771 add for test CNTEL DHCP, begin*/
        #if 0
        char tmp60[64];
        memset(tmp60, 0, sizeof(tmp60));
        
        if(vendorid_tmp != NULL)
        {
            memcpy(tmp60, vendorid_tmp, *(vendorid_tmp - 1));            
            if(0 == strcmp(tmp60, "MSFT 5.0"))
            {
                memset(tmp60, 0, sizeof(tmp60));
                
                tmp60[0] = 60;
                tmp60[1] = 18;
                tmp60[2] = 'C';
                tmp60[3] = 'T';
                tmp60[4] = 1;
                tmp60[5] = 8;
                strcat(tmp60, "MSFT 5.0");

                *(tmp60 + 6 + strlen("MSFT 5.0")) = 5; //Field Type 5
                *(tmp60 + 6 + strlen("MSFT 5.0") + 1) = 4;//Field Len
                *(tmp60 + 6 + strlen("MSFT 5.0") + 1 +1) = 0;//top protocol half
                *(tmp60 + 6 + strlen("MSFT 5.0") + 1 +1 + 1) = 11;//bottom protocol half
                *(tmp60 + 6 + strlen("MSFT 5.0") + 1 + 1 + 1 + 1) = 20;//top port half
                *(tmp60 + 6 + strlen("MSFT 5.0") + 1 + 1 + 1 + 1 + 1) = 6;//top port half

                vendorid_tmp = tmp60 + 2;                
            }
        }        
        #endif
        /*w44771 add for test CNTEL DHCP, end*/
	
	#ifdef SUPPORT_CHINATELECOM_DHCP
	cOption60Cont[0] = '\0';
	memset(cOption60Cont, 0, sizeof(cOption60Cont));
	if(NULL != vendorid_tmp)
	{
	    memcpy(cOption60Cont, vendorid_tmp - 2, (*(vendorid_tmp-1)) + 2);
	}	
	#endif
	
	if(g_enblSrv2 && NULL != server2_config.vendorid && NULL != vendorid_tmp 
            && strlen(server2_config.vendorid) == *(vendorid_tmp - 1)  && !memcmp(vendorid_tmp, server2_config.vendorid, *(vendorid_tmp - 1)))
	{
		    lease_time_align = server2_config.lease;
		    //printf("===>send ack, use server2_config.lease= 0x%08x\n", server2_config.lease);
	}
	else
	{
		for(tmpIndex = 0; tmpIndex < 5; tmpIndex++)
		{
		  if(ntohl(packet.yiaddr) >= ntohl(ipPool[tmpIndex].start) &&
		     ntohl(packet.yiaddr) <= ntohl(ipPool[tmpIndex].end) )
		  	{
		  	    lease_time_align = ipPool[tmpIndex].lease;
		  	    //printf("===>send ack, use ipPool[%d].lease= 0x%08x\n", tmpIndex, ipPool[tmpIndex].lease);
		  	    break;
		  	}
		}
		if(5 == tmpIndex)
		{
		    lease_time_align = master_lease;
		    //printf("===>send ack, use master lease, master_lease=0x%08x\n",master_lease);
		}
	}
#endif
	//w44771 add for 全新分配每次下发的租期,end
	
	if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME))) {
		memcpy(&lease_time_align, lease_time, 4);
		lease_time_align = ntohl(lease_time_align);
		if (lease_time_align > server_config.lease) 
			lease_time_align = server_config.lease;
		else if (lease_time_align < server_config.min_lease) 
			lease_time_align = server_config.lease;
	}
	
	add_simple_option(packet.options, DHCP_LEASE_TIME, lease_time_align);
/*j00100803 Add Begin 2008-04-15 for dhcp option58,59 */
#ifdef SUPPORT_VDF_DHCP
    add_simple_option(packet.options, DHCP_T1, lease_time_align / 2);
    add_simple_option(packet.options, DHCP_T2, (lease_time_align * 0x07) >> 3);
#endif
/*j00100803 Add End 2008-04-15 for dhcp option58,59  */	
	curr = server_config.options;
        /* BEGIN: Modified by y67514, 2008/5/22 Vista无法获取地址*/
        if ( flag_option121)
        {
            //option3和邋Option121不能同时下发
            while (curr) 
            {
                /*j00100803 Add Begin 2008-04-15 for dhcp option58,59 */
#ifdef SUPPORT_VDF_DHCP
                if ((curr->data[OPT_CODE] != DHCP_LEASE_TIME) &&
                    (curr->data[OPT_CODE] != DHCP_T1) &&
                    (curr->data[OPT_CODE] != DHCP_T2)&&
                    (curr->data[OPT_CODE] != DHCP_STATIC_ROUTE)&&
                    (curr->data[OPT_CODE] != DHCP_ROUTER))
#else
                if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
#endif
                /*j00100803 Add End 2008-04-15 for dhcp option58,59  */
                {
                    if(!((curr->data[OPT_CODE] == DHCP_DNS_SERVER) &&(bIpphone ==1))
                        && !((curr->data[OPT_CODE] == DHCP_NTP_SERVER) &&(bIpphone ==1)))
                    add_option_string(packet.options, curr->data);
                }
                curr = curr->next;
            }
            StrOption121[OPT_CODE] = DHCP_STATIC_ROUTE;
            StrOption121[OPT_LEN] = 5;
            StrOption121[OPT_VALUE] = 0;
            memcpy(&StrOption121[OPT_VALUE + 1], &server_config.server, 4);
            add_option_string(packet.options, StrOption121);
        }
        else
        {
            while (curr) 
            {
                /*j00100803 Add Begin 2008-04-15 for dhcp option58,59 */
#ifdef SUPPORT_VDF_DHCP
                if ((curr->data[OPT_CODE] != DHCP_LEASE_TIME) &&
                    (curr->data[OPT_CODE] != DHCP_T1) &&
                    (curr->data[OPT_CODE] != DHCP_T2)&&
                    (curr->data[OPT_CODE] != DHCP_STATIC_ROUTE))
#else
                if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
#endif
                /*j00100803 Add End 2008-04-15 for dhcp option58,59  */
                {

                    if(!((curr->data[OPT_CODE] == DHCP_DNS_SERVER) &&(bIpphone ==1))
                        && !((curr->data[OPT_CODE] == DHCP_NTP_SERVER) &&(bIpphone ==1)))
                    add_option_string(packet.options, curr->data);
                }
                curr = curr->next;
            }
        }
        /* END:   Modified by y67514, 2008/5/22 */

	#ifdef SUPPORT_CHINATELECOM_DHCP
	if(cOption60Cont[0] != '\0')
	{
	    add_option_string(packet.options, cOption60Cont);
	}

	char tmp43[8];
	
	memset(tmp43, 0, sizeof(tmp43));
	tmp43[0] = 43;
	tmp43[1] = strlen(cOption43.configVersion);
	strcat(tmp43, cOption43.configVersion);

	if(tmp43[2] != '\0')
	{
	    add_option_string(packet.options, tmp43);
	}
	#endif

	add_bootp_options(&packet);
#ifdef VDF_OPTION
//add for option125
        /* if DHCPRequest from client has device identity, send back gateway identity,
           and save the device identify */
	if ((req = get_option(oldpacket, DHCP_VENDOR_IDENTIFYING))) 
	{
		if (createVIoption(VENDOR_IDENTIFYING_FOR_GATEWAY,VIinfo) != -1)
			add_option_string(packet.options,VIinfo);
		saveVIoption(req,packet.yiaddr);
	}
#endif

	addr.s_addr = packet.yiaddr;
	/* j00100803 Add Begin 2008-07-21 for dhcp option12,15 request*/
	unsigned char *szRequestList = NULL;
	int k = 0, req_num = 0;
	char szOption12[128], szOption15[128];
	char szHostName[32];

	int iLen = 0;
	int m = 0;
	memset(szOption12, 0, sizeof(szOption12));
	memset(szOption15, 0, sizeof(szOption15));
	memset(szHostName, 0, sizeof(szHostName));

	strcpy(szHostName, inet_ntoa(addr));
	szRequestList = get_option(oldpacket, DHCP_PARAM_REQ);
	if(szRequestList)
	{
	    req_num = *(szRequestList -1);
	    for ( k = 0 ; k < req_num; k++ )
	    {
	        if(DHCP_HOST_NAME == szRequestList[k])
	        {
	        	iLen = strlen(szHostName);
				for(m = 0; m < iLen; m++)
				{
					if(szHostName[m] == '.')
					{
						szHostName[m] = '-';
					}
				}
				szOption12[OPT_CODE] = DHCP_HOST_NAME;
				szOption12[OPT_LEN] = iLen;
				memcpy(&szOption12[OPT_VALUE], szHostName, iLen);
				add_option_string(packet.options, szOption12);
	        }
            
			if(DHCP_DOMAIN_NAME == szRequestList[k] && bIpphone == 0)
			{
				iLen = strlen(DOMAIN_NAME) + 1;
				szOption15[OPT_CODE] = DHCP_DOMAIN_NAME;
				szOption15[OPT_LEN] = iLen;
				memcpy(&szOption15[OPT_VALUE], DOMAIN_NAME, iLen);
				add_option_string(packet.options, szOption15);
			}
	    }
	}
    
    if(bIpphone == 1)
    {
       if(server_config.pIPPhoneDomainName != NULL)
        {
            memset(vdfoption, 0 , BUFSIZE);
            iLen = strlen(server_config.pIPPhoneDomainName) + 1;
            vdfoption[OPT_CODE] = DHCP_DOMAIN_NAME;
            vdfoption[OPT_LEN] = iLen;
			memcpy(&vdfoption[OPT_VALUE], server_config.pIPPhoneDomainName, iLen);
			add_option_string(packet.options, vdfoption);
        }
        if(server_config.pIPPhoneDNSServers != NULL)
        {
            memset(vdfoption, 0 , BUFSIZE);
            fileoption6(vdfoption, server_config.pIPPhoneDNSServers);
            add_option_string(packet.options, vdfoption);
        }
		
        if(server_config.pIPPhoneTFTPServer != NULL)
        {
            memset(vdfoption, 0 , BUFSIZE);
            iLen = strlen(server_config.pIPPhoneTFTPServer) + 1;
            vdfoption[OPT_CODE] = DHCP_TFTP_SERVER_VDF;
            vdfoption[OPT_LEN] = iLen;
			memcpy(&vdfoption[OPT_VALUE], server_config.pIPPhoneTFTPServer, iLen);
			add_option_string(packet.options, vdfoption);
        }else if(server_config.pIPPhoneTFTPServerName != NULL)
        {
            memset(vdfoption, 0 , BUFSIZE);
            iLen = strlen(server_config.pIPPhoneTFTPServerName) + 1;
            vdfoption[OPT_CODE] = DHCP_TFTP_SERVER_VDF;
            vdfoption[OPT_LEN] = iLen;
			memcpy(&vdfoption[OPT_VALUE], server_config.pIPPhoneTFTPServerName, iLen);
			add_option_string(packet.options, vdfoption);
        }
		
        if(server_config.pOutBoundProxy != NULL)
        {
            memset(vdfoption, 0 , BUFSIZE);
            
            if(ValidateIpAddress(server_config.pOutBoundProxy))
            {
                fileoption120ipaddr(vdfoption, server_config.pOutBoundProxy);
            }
            else
            {
                fileoption120Domain(vdfoption, server_config.pOutBoundProxy);
            }
           
			add_option_string(packet.options, vdfoption);
        }
        if(server_config.iIPPhoneVLANQoSEnable == 1)
        {
            bcmEnableQosVlan();   
        }
        if(findSipaddr(yiaddr)== 0)
        {
            BcmMERDataRouteRuleGW(szHostName);
            addSipAddr(yiaddr);
        }

        if(szRequestList)
        {
            req_num = *(szRequestList -1);
            for ( k = 0 ; k < req_num; k++ )
            {
                  if(DHCP_NTP_SERVER == szRequestList[k])
                  {
                    if(server_config.iPPhoneNtp1 != 0)
                    {
                        memset(vdfoption, 0 , BUFSIZE);
                        
                        vdfoption[OPT_CODE] = DHCP_NTP_SERVER;
                        vdfoption[OPT_LEN] = 4;
                        memcpy(&vdfoption[OPT_VALUE], &(server_config.iPPhoneNtp1), sizeof(server_config.iPPhoneNtp1));
                        if(server_config.iPPhoneNtp2 != 0)
                        {
                            memcpy(&vdfoption[OPT_VALUE + 4], &(server_config.iPPhoneNtp2), sizeof(server_config.iPPhoneNtp2));
                            vdfoption[OPT_LEN] += 4;
                        }
                        add_option_string(packet.options, vdfoption);
                    }
                    break;
                }
            }
        }
    }
	/* j00100803 Add End 2008-07-21 for dhcp option12,15 request*/ 
	LOG(LOG_INFO, "sending ACK to %s", inet_ntoa(addr));

	if (send_packet(&packet, 0) < 0) 
		return -1;

       #ifdef SUPPORT_CHINATELECOM_DHCP
       char *vendorid = NULL;
       int foundport = 0;
       unsigned int portvalue = 0;
       int i = 0;
       char portstr[3];

       memset(portstr, 0, sizeof(portstr));
	vendorid = get_option(oldpacket, DHCP_VENDOR);

	/*w44771 for debug test, begin*/
	//vendorid = cOption60Cont + 2;
	/*w44771 for debug test, end*/
	
       if(*(vendorid-1) > 4)//最少有一个Field Type
       {                          
           vendorid += 2;//指向第一个Field Type

           for(i = 0; i < 5; i++)//最多有5个Field
           {
               if(*vendorid == 5)//Type 5, protocol&port
               {
                  vendorid += 4;
                  foundport = 1;
                  //printf("===>CHINA TELECOM Port found!\n");
                  break;
               }
               else
               {
                    vendorid =vendorid + *(vendorid +1) + 2;//指向下一个Field Type 
               }
       	  }
       }

       if(1 == foundport)
       {
           memcpy(portstr, vendorid, 2);
           portvalue = ntohs(*(unsigned short *)(portstr));
           //printf("===>portvalue is %d\n", portvalue);
       }
	add_lease(packet.chaddr, packet.yiaddr, lease_time_align, portvalue);
       #else
	add_lease(packet.chaddr, packet.yiaddr, lease_time_align);
       #endif
      
	return 0;
}


int send_inform(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;
	struct option_set *curr;
        unsigned char StrOption121[OPTION121_LEN];
        u_int32_t lease_time_align = server_config.lease;

    	init_packet(&packet, oldpacket, DHCPACK);

        add_simple_option(packet.options, DHCP_LEASE_TIME, lease_time_align);
        /*j00100803 Add Begin 2008-04-15 for dhcp option58,59 */
#ifdef SUPPORT_VDF_DHCP
        add_simple_option(packet.options, DHCP_T1, lease_time_align / 2);
        add_simple_option(packet.options, DHCP_T2, (lease_time_align * 0x07) >> 3);
#endif
        /*j00100803 Add End 2008-04-15 for dhcp option58,59  */ 
	
	curr = server_config.options;        
        /* BEGIN: Modified by y67514, 2008/5/22 Vista无法获取地址*/
        if ( flag_option121)
        {
            //option3和邋Option121不能同时下发
            while (curr) 
            {
                /*j00100803 Add Begin 2008-04-15 for dhcp option58,59 */
#ifdef SUPPORT_VDF_DHCP
                if ((curr->data[OPT_CODE] != DHCP_LEASE_TIME) &&
                    (curr->data[OPT_CODE] != DHCP_T1) &&
                    (curr->data[OPT_CODE] != DHCP_T2)&&
                    (curr->data[OPT_CODE] != DHCP_STATIC_ROUTE)&&
                    (curr->data[OPT_CODE] != DHCP_ROUTER))
#else
                if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
#endif
                /*j00100803 Add End 2008-04-15 for dhcp option58,59  */
                {
                    add_option_string(packet.options, curr->data);
                }
                curr = curr->next;
            }
            StrOption121[OPT_CODE] = DHCP_STATIC_ROUTE;
            StrOption121[OPT_LEN] = 5;
            StrOption121[OPT_VALUE] = 0;
            memcpy(&StrOption121[OPT_VALUE + 1], &server_config.server, 4);
            add_option_string(packet.options, StrOption121);
        }
        else
        {
            while (curr) 
            {
                /*j00100803 Add Begin 2008-04-15 for dhcp option58,59 */
#ifdef SUPPORT_VDF_DHCP
                if ((curr->data[OPT_CODE] != DHCP_LEASE_TIME) &&
                    (curr->data[OPT_CODE] != DHCP_T1) &&
                    (curr->data[OPT_CODE] != DHCP_T2)&&
                    (curr->data[OPT_CODE] != DHCP_STATIC_ROUTE))
#else
                if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
#endif
                /*j00100803 Add End 2008-04-15 for dhcp option58,59  */
                {
                    add_option_string(packet.options, curr->data);
                }
                curr = curr->next;
            }
        }
        /* END:   Modified by y67514, 2008/5/22 */
        
	add_bootp_options(&packet);

	return send_packet(&packet, 0);
}



