
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include <crypt.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <ctype.h>
#include <net/if.h>
#include <net/route.h>
#include <string.h>
#include <syslog.h>
#include <fcntl.h>



//**************************************************************************
// Function Name: bcmcheck_enable
// Description  : check the appName with ip address against the psi
//                for access mode
// Parameters   : appName - application name in the acl.conf (telnet, ssh, etc.)
//                clntAddr - incoming ip address
// Returns      : access mode - CLI_ACCESS_LOCAL, CLI_REMOTE_LOCAL, CLI_ACCESS_DISABLED
//**************************************************************************
int bcmCheckEnable(char *appName, struct in_addr clntAddr)
{
   // is client address in Access Control List ?
   /*del by sxg(60000658) ACL IP 仅对wan侧进行限制, 2006/02/22, begin*/
   /*if ( BcmScm_isInAccessControlList(inet_ntoa(clntAddr)) == FALSE )
      return CLI_ACCESS_DISABLED;
      */
   /*del by sxg(60000658) ACL IP 仅对wan侧进行限制, 2006/02/22, end*/

   if ( isAccessFromLan(clntAddr) == TRUE ) {
      // is enabled from lan ?
      if ( BcmScm_isServiceEnabled(appName, CLI_ACCESS_LOCAL) == FALSE )
         return CLI_ACCESS_DISABLED;
      else
         return CLI_ACCESS_LOCAL;
   } else {
      // is enabled from wan ?
      if ( BcmScm_isServiceEnabled(appName, CLI_ACCESS_REMOTE) == FALSE )
         return CLI_ACCESS_DISABLED;
      /*start of HG553 2008.08.07 V100R001C02B022 AU8D00877  对FTP不需要使用IP限制*/
      /*add by sxg(60000658) ACL IP 仅对wan侧进行限制, 2006/02/22, begin*/
      else if ( BcmScm_isInAccessControlList(inet_ntoa(clntAddr)) == FALSE )
      {
        if (strcmp(appName, "ftp") != 0)
        {
            return CLI_ACCESS_DISABLED;
        }
        else
        {
            return CLI_ACCESS_REMOTE;
        }
      }
      /*add by sxg(60000658) ACL IP 仅对wan侧进行限制, 2006/02/22, end*/ 
      /*end of HG553 2008.08.07 V100R001C02B022 AU8D00877 对FTP不需要使用IP限制*/
  #ifdef   SUPPORT_RADIUS
      else
      {
            if (strcmp(appName, "telnet") == 0)
            {
                return  CLI_ACCESS_REMOTE_TELNET;
            }
    #endif
            else
            {
                return CLI_ACCESS_REMOTE;
            }
    #ifdef    SUPPORT_RADIUS
      }
    #endif  
    }
}


/***************************************************************************
// Function Name: bcmIsModuleInserted.
// Description  : verify the given module name is already inserted or not.
// Parameters   : modName -- the given module name.
// Returns      : TRUE or FALSE.
****************************************************************************/
int bcmIsModuleInserted(char *modName) {
   int ret = FALSE;
   char buf[SYS_CMD_LEN];
   FILE* fs = fopen("/proc/modules", "r");

   if ( fs != NULL ) {
      while ( fgets(buf, SYS_CMD_LEN, fs) > 0 )
         if ( strstr(buf, modName) != NULL ) {
            ret = TRUE;
            break;
         }
      fclose(fs);
   }

   return ret;
}



void bcmHidePassword(char *command) {
   char *ptr = NULL;
   char * begin, *end;
   int len = 0;

   /* pppd -i .....  -p password */
   if ((ptr = strstr(command,"pppd")) != NULL) {
     if (!strstr(ptr, "-p")) 
        return;
     begin = strstr(ptr,"-p") + 3;
     end = strchr(begin,' ');
     if (end == NULL) 
       len = strlen(begin);
     else 
       len = end - begin;
   }

   while (len > 0) {
      *begin = '*';
      begin++; len--;
   }
}

/***************************************************************************
// Function Name: bcmSystem().
// Description  : launch shell command in the child process.
// Parameters   : command - shell command to launch.
// Returns      : status 0 - OK, -1 - ERROR.
****************************************************************************/
int bcmSystemEx (char *command, int printFlag) {
   int pid = 0, status = 0;
   char *newCommand = NULL;

   if ( command == 0 )
      return 1;

   pid = fork();
   if ( pid == -1 )
      return -1;

   if ( pid == 0 ) {
      char *argv[4];
      argv[0] = "sh";
      argv[1] = "-c";
      argv[2] = command;
      argv[3] = 0;
#ifdef BRCM_DEBUG
      if (printFlag)
         printf("app: %s\r\n", command);
#endif
      if (printFlag) {
        if ((newCommand = strdup(command)) != NULL) {
           bcmHidePassword(newCommand);
           syslog(LOG_DEBUG, newCommand);
           free(newCommand);
        }
      }
      execvp("/bin/sh", argv);
      exit(127);
   }

   /* wait for child process return */
   do {
      if ( waitpid(pid, &status, 0) == -1 ) {
         if ( errno != EINTR )
            return -1;
      } else
         return status;
   } while ( 1 );

   return status;
}


static int getLanInfo(char *lan_ifname, struct in_addr *lan_ip, struct in_addr *lan_subnetmask)
{
   int socketfd;
   struct ifreq lan;

   if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
     printf("app: Error openning socket when getting LAN info\n");
     return -1;
   }

   strcpy(lan.ifr_name,lan_ifname);
   if (ioctl(socketfd,SIOCGIFADDR,&lan) < 0) {
     printf("app: Error getting LAN IP address\n");
     close(socketfd);
     return -1;
   }
   *lan_ip = ((struct sockaddr_in *)&(lan.ifr_addr))->sin_addr;

   if (ioctl(socketfd,SIOCGIFNETMASK,&lan) < 0) {
     printf("app: Error getting LAN subnet address\n");
     close(socketfd);
     return -1;
   }

   *lan_subnetmask = ((struct sockaddr_in *)&(lan.ifr_netmask))->sin_addr;

   close(socketfd);
   return 0;
}

static int isIpExtension(void)
{
   FILE *fp;
   int ipextension = 0;

   if ((fp=fopen("/var/ipextension","r")) != NULL) {
      fscanf(fp,"%d",&ipextension);
      fclose(fp);
   }

   return ipextension;
}

static void getIpExtIp(char *buf)
{
   FILE* fs;
   char wan[64], gateway[64], dns[64], str[256];

   if ( buf == NULL ) return;

   buf[0] = '\0';
   fs = fopen("/var/ipextinfo", "r");
   if ( fs != NULL ) {
      fgets(str, 256, fs);
      fclose(fs);
      sscanf(str, "%s %s %s\n", wan, gateway, dns);
      strcpy(buf, wan);
   }
}



int isAccessFromLan(struct in_addr clntAddr)
{
   int ret = 0;
   struct in_addr inAddr, inMask;
   char wan[64];

   getLanInfo("br0", &inAddr, &inMask);
   /* check ip address of support user to see it is in LAN or not */
   if ( (clntAddr.s_addr & inMask.s_addr) == (inAddr.s_addr & inMask.s_addr) )
      ret = 1;
   else {
      /* check ip address of support user to see if it is from secondary LAN */
      if (bcmCheckInterfaceUp("br0:0")) {
         getLanInfo("br0:0", &inAddr, &inMask);
         if ( (clntAddr.s_addr & inMask.s_addr) == (inAddr.s_addr & inMask.s_addr) )
            ret = 1;
      }

      /* Last option it must be from WAN side */
      if (isIpExtension()) {
         getIpExtIp(wan);
      if ( clntAddr.s_addr == inet_addr(wan) )
         ret = 1;
      }
   }

   return ret;
}

// return 0, ok. return -1 = wrong chip
// used by upload.c and ftpd, tftpd, tftp utilities.
int checkChipId(char *strTagChipId, char *sig2)
{
    int tagChipId = 0;
    unsigned int chipId = (int) sysGetChipId();
    int result;

    tagChipId = strtol(strTagChipId, NULL, 16);

    if (tagChipId == chipId)
        result = 0;
    else {
        printf("Chip Id error.  Image Chip Id = %04x, Board Chip Id = %04x.\n", tagChipId, chipId);
        result = -1;
    }

    return result;
}

/***************************************************************************
// Function Name: bcmCheckForRedirect(void)
// Description  : check for nat redirect for .
// Parameters   : none
// Returns      : 0 --> tcp port 21, 22, 23, 80 is redirected. -1 --> not redirected
****************************************************************************/
int bcmCheckForRedirect(void)
{
   char col[11][32];
   char line[512];
   FILE* fs;  
   int count = 0;

   if (bcmIsModuleInserted("iptable_nat") == FALSE)
      return FALSE;

   bcmSystem("iptables -t nat -L PREROUTING_UTILITY > /var/nat_redirect");

   fs = fopen("/var/nat_redirect", "r");
   if ( fs != NULL ) {
      while ( fgets(line, sizeof(line), fs) ) {
         // read pass 3 header lines
         if ( count++ < 3 )
            continue;
         sscanf(line, "%s %s %s %s %s %s %s %s %s %s %s",
               col[0], col[1], col[2], col[3], col[4], col[5],
               col[6], col[7], col[8], col[9], col[10]);
        if ((strcmp(col[0], "REDIRECT") == 0) && (strcmp(col[1], "tcp") == 0) && (strcmp(col[8], "ports") == 0))
          if (strcmp(col[9], "80") == 0 || strcmp(col[9], "23") == 0 || strcmp(col[9], "21") == 0 ||
              strcmp(col[9], "22") == 0 || strcmp(col[9], "69") == 0) {
              return TRUE;
          }
      }
      fclose(fs);
   }
   unlink("/var/nat_redirect");
   return FALSE;
}

/***************************************************************************
// Function Name: bcmRemoveModules(int lanIf)
// Description  : remove not used modules to free memory.
// Parameters   : none
// Returns      : none.
****************************************************************************/
void bcmRemoveModules(int lanIf)
{
   char *modList[]=
   {
      "bcm_enet",
      "bcm_usb",
      "ipt_state",
      "ipt_mark",
      "ipt_limit",
      "ipt_tcpmss_j",
      "ipt_REDIRECT",
      "ipt_MASQUERADE",
      "ipt_mark_j",
      "ipt_LOG",
      "ipt_FTOS",
      "ip_nat_tftp",
      "ip_nat_irc",
      "ip_nat_ftp",
      "ip_nat_h323",
      "ip_nat_pptp",
      "ip_nat_gre",
      "ip_nat_rtsp",
      "ip_nat_ipsec",
      "ip_conntrack_tftp",
      "ip_conntrack_irc",
      "ip_conntrack_ftp",
      "ip_conntrack_h323",
      "ip_conntrack_pptp",
      "ip_conntrack_gre",
      "ip_conntrack_rtsp",
      "ip_conntrack_ipsec",
      "iptable_mangle",
      "ip_conntrack",
      "ip_tables",
      NULL,
   };

   char cmd[SYS_CMD_LEN];
   int i = 0;
   int saveNat = FALSE;

   if (lanIf == 0)         // if lan, do not kill bcm_usb and bcm_enet
      i = 2;
   else // if in ipow mode, leave bcm_enet out   
   {
      FILE *fs = fopen("/proc/var/fyi/wan/eth0/pid", "r");
      if (fs != NULL) 
      {
         i = 1;        
         fclose(fs);
      }
   }

   saveNat = bcmCheckForRedirect();
   if (bcmIsModuleInserted("iptable_filter") == TRUE)
   {  
       strncpy(cmd, "iptables -t filter -F", SYS_CMD_LEN-1);
       bcmSystem(cmd);
   }
   if (bcmIsModuleInserted("iptable_nat") == TRUE)
   {  
       strncpy(cmd, "iptables -t nat -F", SYS_CMD_LEN-1);
       bcmSystem(cmd);
   }
   if (bcmIsModuleInserted("iptable_mangle") == TRUE)
   {  
       strncpy(cmd, "iptables -t mangle -F", SYS_CMD_LEN-1);
       bcmSystem(cmd);
   }

   while (modList[i] != NULL)
   {
      if (bcmIsModuleInserted(modList[i]) == TRUE) 
      {
         if (!(saveNat && strcmp(modList[i], "iptable_nat") == 0))
         {
            sprintf(cmd, "rmmod %s", modList[i]);
            bcmSystem(cmd);
         }
      }
      i++;
   }
   printf("\nRemaining modules:\n");
   bcmSystemMute("cat /proc/modules");
   printf("\nMemory info:\n");
   bcmSystemMute("sysinfo");
   sleep(1);
}


//**************************************************************************
// Function Name: bcmMacStrToNum
// Description  : convert MAC address from string to array of 6 bytes.
//                Ex: 0a:0b:0c:0d:0e:0f -> 0a0b0c0d0e0f
// Returns      : status.
//**************************************************************************
int bcmMacStrToNum(char *macAddr, char *str) {
   char *pToken = NULL, *pLast = NULL;
   char *buf;
   UINT16 i = 1;
   int len;
   
   if ( macAddr == NULL ) return FALSE;
   if ( str == NULL ) return FALSE;

   len = strlen(str) + 1;
   if (len > 20)
     len = 20;
   buf = (char*)malloc(len);
   memset(buf,0,len);

   if ( buf == NULL ) return FALSE;

   /* need to copy since strtok_r updates string */
   strncpy(buf, str,len-1);

   /* Mac address has the following format
       xx:xx:xx:xx:xx:xx where x is hex number */
   pToken = strtok_r(buf, ":", &pLast);
   macAddr[0] = (char) strtol(pToken, (char **)NULL, 16);
   for ( i = 1; i < 6; i++ ) {
      pToken = strtok_r(NULL, ":", &pLast);
      macAddr[i] = (char) strtol(pToken, (char **)NULL, 16);
   }

   free(buf);

   return TRUE;
}

//**************************************************************************
// Function Name: bcmMacNumToStr
// Description  : convert MAC address from array of 6 bytes to string.
//                Ex: 0a0b0c0d0e0f -> 0a:0b:0c:0d:0e:0f
// Returns      : status.
//**************************************************************************
int bcmMacNumToStr(char *macAddr, char *str) {
   if ( macAddr == NULL ) return FALSE;
   if ( str == NULL ) return FALSE;

   sprintf(str, "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
           (UINT8) macAddr[0], (UINT8) macAddr[1], (UINT8) macAddr[2],
           (UINT8) macAddr[3], (UINT8) macAddr[4], (UINT8) macAddr[5]);

   return TRUE;
}
/*start of HG553 2008.08.09 V100R001C02B022 AU8D00864 by c65985 */
int bcmOption61EthToStr(char *macAddr, char *str) {
   if ( macAddr == NULL ) return FALSE;
   if ( str == NULL ) return FALSE;

   sprintf(str, "Hardware type:Ethernet  Client MAC address: %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
           (UINT8) macAddr[1], (UINT8) macAddr[2], (UINT8) macAddr[3],
           (UINT8) macAddr[4], (UINT8) macAddr[5], (UINT8) macAddr[6] );

   return TRUE;
}


/***********************************************************
Function:       int tmpfile_writevalue(PS_TMPFILE_CONTENT)
Description:    写文件
Calls:          NULL
Called By:      需要写临时文件的函数
Input:          需要写入的数据名称、长度、值
Output:         NULL
Return:         0		成功
                其他    失败
Others:         NULL

------------------------------------------------------------
实现：
使用消息队列实现，消息处理优先处理写消息
************************************************************/
int tmpfile_writevalue(char *pchName, char *pchValue)
{
    int msgid;
    int iKey;
    S_TMPFILE_MSG sMsg;

    if ((TMPFILE_MAX_ITEM_NAME <= strlen(pchName))
        || (TMPFILE_MAX_ITEM_VALUE <= strlen(pchValue)))
    {
        printf("Input string is too long.\n");
        return -1;
    }

    memset(&sMsg, 0, sizeof(S_TMPFILE_MSG));
    sMsg.lMsgType = TMPFILE_WRITE_OPT;
    strcpy(sMsg.achName, pchName);
    strcpy(sMsg.achValue, pchValue);

    iKey = ftok(TMPFILE_KEY_PATH, TMPFILE_KEY_SEED);
    msgid = msgget(iKey, IPC_CREAT | 0660);
    if (-1 == msgid) 
    {
        printf("Get message queue failed.\n");
        return -1;
    }

    if (-1 == msgsnd(msgid, &sMsg, sizeof(sMsg) - sizeof(long), 0))
    {
        return -1;
    }

    return 0;
}

/***********************************************************
Function:       int tmpfile_getvalue(PS_TMPFILE_CONTENT)
Description:    读文件
Calls:          NULL
Called By:      需要从临时文件读取数据的函数
Input:          需要读出的数据名称、预计长度
Output:         返回的数据值，实际长度
Return:         0		成功
                其他	失败
Others:         NULL

----------------------------------------------------------------------------
实现：
使用消息队列实现，消息处理优先处理写消息
************************************************************/
int tmpfile_getvalue(char *pchName, char *pchValue, int *piLen)
{
    int msgid;
    int iKey;
    int iRetryTimes;
    int iPid;
    S_TMPFILE_MSG sMsg;
    static int iSerial = 0;

    if (TMPFILE_MAX_ITEM_NAME <= strlen(pchName))
    {
        printf("Input string is too long.\n");
        return -1;
    }

    iKey = ftok(TMPFILE_KEY_PATH, TMPFILE_KEY_SEED);
    msgid = msgget(iKey, IPC_CREAT | 0660);
    if (-1 == msgid) 
    {
        printf("Get message queue failed.\n");
        return -1;
    }

    iPid = getpid();
    
    memset(&sMsg, 0, sizeof(S_TMPFILE_MSG));
    sMsg.lMsgType = TMPFILE_READ_OPT;
    sMsg.iPid = iPid;
    sMsg.iSerial = iSerial;
    iSerial++;
    strcpy(sMsg.achName, pchName);
    if (-1 == msgsnd(msgid, &sMsg, sizeof(sMsg) - sizeof(long), 0))
    {
        return -1;
    }

    /* 接收返回消息 */
    memset(&sMsg, 0, sizeof(S_TMPFILE_MSG));
    iRetryTimes = 0;
    /* 若消息体中序列号与要读取的不一致，再次读取消息 */
    while (1)
    {
        /* 若接收消息失败，重试3次 */
        while (-1 == msgrcv(msgid, &sMsg, sizeof(sMsg) - sizeof(long), iPid, IPC_NOWAIT))
        {
            iRetryTimes++;
            if (10 <= iRetryTimes)
            {
                //perror("tmpfile_getvalue");
                return -1;
            }
            usleep(100000);
        }
        if (sMsg.iSerial == iSerial - 1)
        {
            break;
        }
    }

    if (0 != sMsg.iResult)
    {
        return -1;
    }

    if (*piLen < strlen(sMsg.achValue))
    {
        printf("Value string is too long.\n");
        return -1;
    }
    
    strcpy(pchValue, sMsg.achValue);
    *piLen = strlen(pchValue);

#if (TMPFILE_DEBUG == 1)
    printf("<Lazyjack> serial %d get %s value %s\n", iSerial - 1, pchName, pchValue);
#endif
    return 0;
}


int TR069_SendMessage(long msg_type, void *msg_content, int content_len)
{
    int msgid;
    int iKey;
    int iPid;
    S_TR069_MSG sMsg;
  
    if (content_len > TR069_MAX_CONTENT)
    {
        return -1;
    }
    /* start of maintain 不启动tr69c进程时多次从网页上设置tr069相关配置信息会导致网页不能正常访问 by xujunxia 43813 2006年9月13日*/
    FILE *pidfile;
    int  tr69c_pid = 0;
    pidfile = fopen(TR69_PID_FILE, "r");
    if (pidfile == NULL) 
    {
        return -1;
    }  
    else
    {
        if (fscanf(pidfile, "%d", &tr69c_pid) != 1) 
        {
           fclose(pidfile);
    	    return -1;
        }
        fclose(pidfile);
    }
    /* end of maintain 不启动tr69c进程时多次从网页上设置tr069相关配置信息会导致网页不能正常访问 by xujunxia 43813 2006年9月13日 */
    
    iKey = ftok(TR069_KEY_PATH, TR069_KEY_SEED);
    msgid = msgget(iKey, IPC_CREAT | 0660);
    if (-1 == msgid) 
    {
        printf("Get message queue failed.\n");
        return -1;
    }

    iPid = getpid();
    
    memset(&sMsg, 0, sizeof(S_TR069_MSG));
    sMsg.lMsgType = msg_type;
    if (msg_content != NULL)
    {
        memcpy(sMsg.achContent, msg_content, content_len);
    }
    if (-1 == msgsnd(msgid, &sMsg, sizeof(sMsg) - sizeof(long), 0))
    {
        printf("TR069_SendMessage send error  %d\n");
        return -1;
    }

    return 0;
}    

int TR069_ReceiveMessage(long *msg_type, void *msg_content, int content_len)
{
    int msgid;
    int iKey;
    S_TR069_MSG sMsg;
    int result;

    if (content_len > TR069_MAX_CONTENT)
    {
        return -1;
    }

    iKey = ftok(TR069_KEY_PATH, TR069_KEY_SEED);
    msgid = msgget(iKey, IPC_CREAT | 0660);
    if (-1 == msgid) 
    {
        printf("Get message queue failed.\n");
        return -1;
    }

    while (-1 == msgrcv(msgid, &sMsg, sizeof(sMsg) - sizeof(long), 0, 0))
    {
    }
    *msg_type = sMsg.lMsgType;
    memcpy(msg_content, sMsg.achContent, content_len);

    return 0;
}

int AtRcv( int lMsgKey, struct stAtRcvMsg *pstAtRcvMsg, int lMsgType )
{
    /*start of 修改hspa配置参数阻塞问题 by s53329 at  20080328*/
    int iRetryTimes = 0;

    memset(pstAtRcvMsg, 0, sizeof(*pstAtRcvMsg));
  
    /* 若接收消息失败，重试15次 */
    while (-1 == msgrcv(lMsgKey, (void*)pstAtRcvMsg, AT_RCV_MSG_LENGTH, lMsgType, IPC_NOWAIT))
    {
        iRetryTimes++;
        if (20 <= iRetryTimes)
        {
            return -1;
        }
        usleep(200000);
    }
    /*end of 修改hspa配置参数阻塞问题 by s53329 at  20080328*/
    return 0;
}

int AtSend(int lMsgKey, struct stAtSndMsg *pstAtSndMsg, char cFrom,  int lMsgType,  const char *pszParam)
{
	memset(pstAtSndMsg, 0, sizeof(*pstAtSndMsg));	
	pstAtSndMsg->lMsgType = lMsgType;
	pstAtSndMsg->acParam[0] = cFrom;
	if(NULL!= pszParam)
		strcpy(pstAtSndMsg->acParam+1, pszParam);
	return msgsnd(lMsgKey, (void*)pstAtSndMsg, AT_SND_MSG_LENGTH, 0);
}

