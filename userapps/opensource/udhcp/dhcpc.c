/* dhcpd.c
 *
 * udhcp DHCP client
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
 
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/file.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>

#include "dhcpd.h"
#include "dhcpc.h"
#include "options.h"
#include "clientpacket.h"
#include "packet.h"
#include "script.h"
#include "socket.h"
#include "debug.h"
#include "pidfile.h"
#include <netinet/in.h>
// brcm
#include "board_api.h"

static int state;
static unsigned long requested_ip; /* = 0 */
static unsigned long server_addr;
/* BEGIN: Added by KF19989, 2009/11/19: Automatic ACL */
char gGwIp[ICMP_DHCP_AUTOACL_MAX][32] = {"","","","","","","","","","","","","","","",""};
static unsigned long pre_server_addr[ICMP_DHCP_AUTOACL_MAX];
static int acl_dhcp_flag;
#define DHCP_COMMAND_LEN 255
#define ICMP_ACL_DHCP_FILE_1 "/proc/sys/net/ipv4/icmp_acl_dhcp_1"
#define ICMP_ACL_DHCP_FILE_2 "/proc/sys/net/ipv4/icmp_acl_dhcp_2"
/* END:  Added by KF19989, 2009/11/19: Automatic ACL */
/* BEGIN: Modified by y67514, 2008/6/20   问题单号:time是long型值*/
//static unsigned long timeout;
static long timeout;
/* END:   Modified by y67514, 2008/6/20 */

static int packet_num; /* = 0 */
//w44771 add for A36D02465, begin, 2006-8-14
static int discover_times; /* = 0 */
//w44771 add for A36D02465, end, 2006-8-14

// brcm
char session_path[64];
static char status_path[128]="";
static char pid_path[128]="";
char vendor_class_id[128]="";

#define LISTEN_NONE 0
#define LISTEN_KERNEL 1
#define LISTEN_RAW 2
static int listen_mode = LISTEN_RAW;
// brcm
static int old_mode = LISTEN_RAW;
#define INIT_TIMEOUT 0
/* start of HG553_protocal : Added by c65985, 2010.3.19  DHCP Request mechanism HG553V100R001 */
#define REQ_TIMEOUT_T1 (10+(int)(10 * (lrand48() / (RAND_MAX + 1.0))))
#define REQ_TIMEOUT_T2 (40+(int)(90 * (lrand48() / (RAND_MAX + 1.0))))
/* end of HG553_protocal : Added by c65985, 2010.3.19  DHCP Request mechanism HG553V100R001 */

#define DEFAULT_SCRIPT	"/etc/dhcp/dhcp_getdata"

/*w44771 add for 防止DHCP浪涌,begin, 2006-7-7*/
#define SUPPORT_DHCP_SURGE
/*w44771 add for 防止DHCP浪涌,end, 2006-7-7*/

struct client_config_t client_config = {
	/* Default options. */
	abort_if_no_lease: 0,
	foreground: 0,
	quit_after_lease: 0,
	interface: "eth0",
	Interface: "eth0",  	/*WAN <3.4.5桥使能dhcp> add by s60000658 20060505*/
	pidfile: NULL,
	script: DEFAULT_SCRIPT,
	clientid: NULL,
	hostname: NULL,
	ifindex: 0,
	arp: "\0\0\0\0\0\0",		/* appease gcc-3.0 */
};
/* 表项名的最大长度 */
#define TMPFILE_MAX_ITEM_NAME       16
/* 消息类型，包括写，读，读反馈 */
#define TMPFILE_WRITE_OPT   1
#define TMPFILE_READ_OPT    2
#define TMPFILE_READ_ACK    3
/* 消息队列key值生成需要的参数 */
#define TMPFILE_KEY_PATH    "/var"
#define TMPFILE_KEY_SEED    't'
#define TMPFILE_MAX_ITEM_VALUE      512

typedef struct tag_TMPFILE_MSG
{
    long lMsgType;                          //消息类型，包括写，读，读反馈
    int iResult;                            //操作结果
    int iPid;                               //发送消息的PID，作为读响应消息的类型值
    int iSerial;                            //消息序列号，避免同一进程读取消息顺序错误
    char achName[TMPFILE_MAX_ITEM_NAME];    //读写变量名
    char achValue[TMPFILE_MAX_ITEM_VALUE];  //读写变量值
}S_TMPFILE_MSG;

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

// brcm
void setStatus(int status) {
    char cmd[128] = "";
    int f;
    static int oldStatus  = -1;
    sprintf(cmd, "echo %d > %s", status, status_path);
    system(cmd); 

    /*start of  2008.05.10 HG553V100R001C02B016 AU8D00566 */
    if(oldStatus == status)
    {
        return;
    }
    oldStatus = status;
    /*end of  2008.05.10 HG553V100R001C02B016 AU8D00566 */
    
    if( (f = open( "/dev/brcmboard", O_RDWR )) != -1 )
    {
        ioctl( f, BOARD_IOCTL_WAKEUP_MONITOR_TASK, NULL);
        close(f);
    }
}

void setPid() {
    char cmd[128] = "";
    
    sprintf(cmd, "echo %d > %s", getpid(), pid_path);
    system(cmd); 
}

static void print_usage(void)
{
	printf(
"Usage: udhcpcd [OPTIONS]\n\n"
"  -c, --clientid=CLIENTID         Client identifier\n"
"  -H, --hostname=HOSTNAME         Client hostname\n"
"  -f, --foreground                Do not fork after getting lease\n"
"  -i, --interface=INTERFACE       Interface to use (default: eth0)\n"
"  -n, --now                       Exit with failure if lease cannot be\n"
"                                  immediately negotiated.\n"
"  -p, --pidfile=file              Store process ID of daemon in file\n"
"  -q, --quit                      Quit after obtaining lease\n"
"  -r, --request=IP                IP address to request (default: none)\n"
"  -s, --script=file               Run file at dhcp events (default:\n"
"                                  " DEFAULT_SCRIPT ")\n"
"  -v, --version                   Display version\n"
"  -m,--mode                atm simple bridge or ptm simple bridge or others\n" //w44771 add for simple bridge
	);
}


/* SIGUSR1 handler (renew) */
static void renew_requested(int sig)
{
	sig = 0;
	LOG(LOG_INFO, "Received SIGUSR1");
	/*
	if (state == BOUND || state == RENEWING || state == REBINDING ||
	    state == RELEASED) {
	    */
	if (state == BOUND || state == RENEWING || state == REBINDING)
	{
            listen_mode = LISTEN_KERNEL;
            //SERVER_addr不需要清空，serveraddr的加入或更新只在offer报文的处理中进行
            //server_addr = 0;
            packet_num = 0;
            state = RENEW_REQUESTED;
	}

	if (state == RELEASED) {
		listen_mode = LISTEN_RAW;
		state = INIT_SELECTING;
		packet_num = 0;
	}

	/* Kill any timeouts because the user wants this to hurry along */
	timeout = 0;
}


/* SIGUSR2 handler (release) */
static void release_requested(int sig)
{
	sig = 0;
	LOG(LOG_INFO, "Received SIGUSR2");
	/* send release packet */
	if (state == BOUND || state == RENEWING || state == REBINDING) {
		send_release(server_addr, requested_ip); /* unicast */
		run_script(NULL, "deconfig");
	}

	listen_mode = 0;
	state = RELEASED;
    /* BEGIN: Modified by y67514, 2008/6/20   问题单号:time是long型值*/
	timeout = 0x7fffffff;
    /* END:   Modified by y67514, 2008/6/20 */
}


/* Exit and cleanup */
static void exit_client(int retval)
{
	pidfile_delete(client_config.pidfile);
	CLOSE_LOG();
	exit(retval);
}


/* SIGTERM handler */
static void terminate(int sig)
{
	sig = 0;
	LOG(LOG_INFO, "Received SIGTERM");
    /* BEGIN: Added by KF19989, 2009/12/1: 杀死进程前先去除ICMP规则Automatic ACL DHCP */
    char command[DHCP_COMMAND_LEN];
	int i;
	struct in_addr icmp_addr;

	if(acl_dhcp_flag == 1)
	{
        tmpfile_writevalue("acl_dhcp", "0");
	}

	for(i = 0; i < ICMP_DHCP_AUTOACL_MAX; i++)
	{
		if(pre_server_addr[i] != 0)
		{
			icmp_addr.s_addr = pre_server_addr[i];
	        char *preIP = inet_ntoa(icmp_addr);
			memset(command, 0x00, sizeof(command));
		    sprintf(command, "iptables -D INPUT -s %s -p icmp --icmp-type 8/0 -j ACCEPT", preIP);
		    bcmSystemEx(command, 1);
		}
	}
	/* END:   Added by KF19989, 2009/12/1: 杀死进程前先去除ICMP规则 Automatic ACL DHCP*/
	exit_client(0);
}


static void background(void)
{
	int pid_fd;
	if (client_config.quit_after_lease) {
		exit_client(0);
	} else if (!client_config.foreground) {
		pid_fd = pidfile_acquire(client_config.pidfile); /* hold lock during fork. */
		switch(fork()) {
		case -1:
			perror("fork");
			exit_client(1);
			/*NOTREACHED*/
		case 0:
			// brcm
			setPid();
			break; /* child continues */
		default:
			exit(0); /* parent exits */
			/*NOTREACHED*/
		}
		close(0);
		close(1);
		close(2);
		setsid();
		client_config.foreground = 1; /* Do not fork again. */
		pidfile_write_release(pid_fd);
	}
}

/* j00100803 Add Begin 2008-03-07 */
#if  0 // 暂时屏蔽此功能
typedef enum
{
    TIMER_ENUM_NORMAL,
    TIMER_ENUM_RENEWING,
    TIMER_ENUM_REBINDING
}TimerEnumType;

typedef enum
{
    TIMER_ENUM_RUN,
    TIMER_ENUM_STOP
}TimerEnumState;

int ExecCommandToConfigSNTP(char * pSrvAddr1, char * pSrvAddr2)
{
    char cmd[128];
    memset(cmd, 0, 128);
    if(NULL == pSrvAddr1)
    {
        return -1;
    }
    
    if(NULL == pSrvAddr2)
    {
        sprintf(cmd, "sntp -s %s\n", pSrvAddr1);
    }
    else
    {
        sprintf(cmd, "sntp -s %s -s %s\n", pSrvAddr1, pSrvAddr2);
    }
    system(cmd);
    return 0;
}

#endif
/* j00100803 Add End 2008-03-07 */

/* BEGIN: Added by KF19989, 2009/11/19: Automatic ACL DHCP */
static void Automatic_ACL_DHCP(char *file)
{
    struct in_addr icmp_addr;
    char command[DHCP_COMMAND_LEN];
    struct in_addr gw_addr;
    char fileName[128]="";
	char cmd[512];
	int i,j;

	int cur_server_addr[ICMP_DHCP_AUTOACL_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	int iLen = 32;
	char gw_ip[32]="";

	sprintf(fileName,"gw_%s",client_config.Interface);

	if (0 != tmpfile_getvalue(fileName, gw_ip, &iLen))
	{
		printf("[%s,%d]tmpfile_getvalue failed! \n",__func__,__LINE__);
	}


	printf("[%s,%d]client_config.Interface: %s, router_ip: %s\n",
		__func__,__LINE__,client_config.Interface, gw_ip);


	if(strlen(gw_ip) && (strcmp(gw_ip, "0.0.0.0") != 0))
	{

		inet_aton(gw_ip, &gw_addr);
		/*option 3*/
		printf("[%s,%d]get default gw %s from option 3 to add auto ACL...\n",__func__,__LINE__,gw_ip);
		cur_server_addr[0] = gw_addr.s_addr;
	}
	else 
	{
		for(i = 0; i < ICMP_DHCP_AUTOACL_MAX; i++)
		{
			if(strlen(gGwIp[i]))
			{
				inet_aton(gGwIp[i], &gw_addr);
				/*option 121*/
				printf("[%s,%d]get default gw %s from option 121 to add auto ACL...\n",__func__,__LINE__,gGwIp[i]);
				cur_server_addr[i] = gw_addr.s_addr;
			}
		}
	}
	/*
	else
	{
	
		printf("[%s,%d]no default gw to add auto ACL...\n",__func__,__LINE__);
		return;
	}
	*/
	
    //if(cur_server_addr != pre_server_addr)
    {
		/*
		//允许当前IP ICMP
        icmp_addr.s_addr = cur_server_addr;
        char *currentIP = inet_ntoa(icmp_addr);
        memset(command, 0x00, sizeof(command));
        sprintf(command, "iptables -I INPUT -s %s -p icmp --icmp-type 8/0 -j ACCEPT", currentIP);
        bcmSystemEx(command, 1);
        */
        //删除上一个IP的ICMP
        for(i = 0; i < ICMP_DHCP_AUTOACL_MAX; i++)
        {
	        if(pre_server_addr[i] != 0) {
	            icmp_addr.s_addr = pre_server_addr[i];
	            char *preIP = inet_ntoa(icmp_addr);
	            memset(command, 0x00, sizeof(command));
	            sprintf(command, "iptables -D INPUT -s %s -p icmp --icmp-type 8/0 -j ACCEPT", preIP);
	            bcmSystemEx(command, 1);
	        }
        }

		memset(cmd, 0x00, sizeof(cmd));
		sprintf(cmd, "echo ");

        for(i = 0, j = 0; i < ICMP_DHCP_AUTOACL_MAX; i++)
        {
	        if(cur_server_addr[i] != 0) {
				icmp_addr.s_addr = cur_server_addr[i];
				char *currentIP = inet_ntoa(icmp_addr);
				memset(command, 0x00, sizeof(command));
				sprintf(command, "iptables -I INPUT -s %s -p icmp --icmp-type 8/0 -j ACCEPT", currentIP);
				bcmSystemEx(command, 1);

				memset(command, 0x00, sizeof(command));
				sprintf(command, "%u ", cur_server_addr[i]);
				strcat(cmd, command);	
				j++;
	        }
        }

		for (; j < ICMP_DHCP_AUTOACL_MAX; j++)
		{
			
		
			strcat(cmd, "0 ");
		}
		memset(command, 0x00, sizeof(command));
		sprintf(command, "> %s", file);
		strcat(cmd, command);

		printf("[%s,%d]===cmd:%s========\n",__func__,__LINE__,cmd);

		bcmSystem(cmd);
    

		for(i = 0; i < ICMP_DHCP_AUTOACL_MAX; i++)
		{
        	pre_server_addr[i] = cur_server_addr[i];
		}
    }    
}
/* END:  Added by KF19989, 2009/11/19: Automatic ACL DHCP */
#ifdef COMBINED_BINARY
int udhcpc(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	char *temp, *message;
	unsigned long t1 = 0, t2 = 0, xid = 0;
	unsigned long start = 0, lease;
	fd_set rfds;
	int fd, retval;
	struct timeval tv;
	int c, len;
	struct ifreq ifr;
	struct dhcpMessage packet;
	struct in_addr temp_addr;
	int pid_fd;

	/*w44771 add for 防止DHCP浪涌和169网段ip,begin, 2006-7-7*/
	char cLastMac[24];
	int iTimeRemain, iSeed;
	int dhcp_surge = 0;//w44771 add for simple bridge
	/*w44771 add for 防止DHCP浪涌和169网段ip,end, 2006-7-7*/
	/* BEGIN: Added by y67514, 2008/2/18   PN:AU8D00223*/
	FILE *classidfp;
	/* END:   Added by y67514, 2008/2/18 */
	/* j00100803 Add Begin 2008-03-07 */
	#if 0 //暂时屏蔽此特性
	static int iRenewingTimerOption58 = 0;
	static int iRebindingTimerOption59 = 0;
	TimerEnumType enTimerType = TIMER_ENUM_NORMAL;
	TimerEnumState enTimerOption58 = TIMER_ENUM_STOP;
	TimerEnumState enTimerOption59 = TIMER_ENUM_STOP;
	#endif
	/* j00100803 Add End 2008-03-07 */
	static struct option options[] = {
		{"clientid",	required_argument,	0, 'c'},
		{"foreground",	no_argument,		0, 'f'},
		{"hostname",	required_argument,	0, 'H'},
		{"help",	no_argument,		0, 'h'},
		{"interface",	required_argument,	0, 'i'},
		/*start of WAN <3.4.5桥使能dhcp> porting by s60000658 20060505*/
		{"Interface",	required_argument,	0, 'I'},
		/*end of WAN <3.4.5桥使能dhcp> porting by s60000658 20060505*/
		{"now", 	no_argument,		0, 'n'},
		{"pidfile",	required_argument,	0, 'p'},
		{"quit",	no_argument,		0, 'q'},
		{"request",	required_argument,	0, 'r'},
		{"script",	required_argument,	0, 's'},
		{"version",	no_argument,		0, 'v'},
		{0, 0, 0, 0}
	};
    /* BEGIN: Added by KF19989, 2009/11/19: Automatic ACL DHCP */
    int stat;
    char dhcp_flag = '0';
    int flag_len = sizeof(dhcp_flag);
    char dhcp_file[DHCP_COMMAND_LEN];
    //pre_server_addr = 0;
    memset(pre_server_addr, 0x00, sizeof(pre_server_addr));
	acl_dhcp_flag = 0;
    memset(dhcp_file, 0x00, sizeof(dhcp_file));
	//acl_dhcp为1 表示 ICMP_ACL_DHCP_FILE_1已使用，在此DHCP进程使用ICMP_ACL_DHCP_FILE_2
    stat = tmpfile_getvalue("acl_dhcp", &dhcp_flag, &flag_len);
    if(stat != 0 || (stat == 0 && dhcp_flag ==  '0')) {

		printf("---DEBUG---%s,%d\n",__func__,__LINE__);
		
        strcpy(dhcp_file, ICMP_ACL_DHCP_FILE_1);
		acl_dhcp_flag = 1;
        tmpfile_writevalue("acl_dhcp", "1");
    } else {

		printf("---DEBUG---%s,%d\n",__func__,__LINE__);

		acl_dhcp_flag = 2;
        strcpy(dhcp_file, ICMP_ACL_DHCP_FILE_2);
    }
    /* END:  Added by KF19989, 2009/11/19: Automatic ACL DHCP */
	/* get options */
	while (1) {
		int option_index = 0;
// brcm
		/* BEGIN: Modified by y67514, 2008/2/18   PN:AU8D00223*/
		/*start of WAN <3.4.5桥使能dhcp> porting by s60000658 20060505*/
		//c = getopt_long(argc, argv, "c:fH:hi:np:qr:s:d:v", options, &option_index);
		//c = getopt_long(argc, argv, "c:fH:hi:I:np:qr:s:d:vm", options, &option_index);//w44771 add for simple bridge
		/*end of WAN <3.4.5桥使能dhcp> porting by s60000658 20060505*/
		c = getopt_long(argc, argv, "c:fH:hi:I:np:qr:s:dvm", options, &option_index);
		/* END:   Modified by y67514, 2008/2/18 */
		
		
		if (c == -1) break;
		
		switch (c) {
		case 'c':
			len = strlen(optarg) > 255 ? 255 : strlen(optarg);
			if (client_config.clientid) free(client_config.clientid);
			client_config.clientid = malloc(len + 2);
			client_config.clientid[OPT_CODE] = DHCP_CLIENT_ID;
			client_config.clientid[OPT_LEN] = len;
			strncpy(client_config.clientid + 2, optarg, len);
			break;
		case 'f':
			client_config.foreground = 1;
			break;
		case 'H':
			len = strlen(optarg) > 255 ? 255 : strlen(optarg);
			if (client_config.hostname) free(client_config.hostname);
			client_config.hostname = malloc(len + 2);
			client_config.hostname[OPT_CODE] = DHCP_HOST_NAME;
			client_config.hostname[OPT_LEN] = len;
			strncpy(client_config.hostname + 2, optarg, len);
			break;
		case 'h':
			print_usage();
			return 0;
		case 'i':
			client_config.interface =  optarg;
			client_config.Interface =  optarg; /*WAN <3.4.5桥使能dhcp> add by s60000658 20060505*/
// brcm
			strcpy(session_path, optarg);
			break;
		/*start of WAN <3.4.5桥使能dhcp> porting by s60000658 20060505*/
		case 'I':
			client_config.Interface =  optarg;
			strcpy(session_path, optarg);
			break;
		/*end of WAN <3.4.5桥使能dhcp> porting by s60000658 20060505*/
		case 'n':
			client_config.abort_if_no_lease = 1;
			break;
		case 'p':
			client_config.pidfile = optarg;
			break;
		case 'q':
			client_config.quit_after_lease = 1;
			break;
		case 'r':
			requested_ip = inet_addr(optarg);
			break;
// brcm
		case 'd':
			/* BEGIN: Modified by y67514, 2008/2/18   PN:AU8D00223*/
			/*strcpy(vendor_class_id, optarg);  */
			
			classidfp = fopen("/var/dhcpClassIdentifier","r");
			if ( NULL !=  classidfp)
			{
			    fgets(vendor_class_id,128,classidfp);
			    printf("VDF:%s:%s:%d:vendor_class_id=%s***\n",__FILE__,__FUNCTION__,__LINE__,vendor_class_id);
			    fclose(classidfp);
			}
			/* END:   Modified by y67514, 2008/2/18 */
			break;
		case 's':
			client_config.script = optarg;
			break;
		case 'v':
			printf("udhcpcd, version %s\n\n", VERSION);
			break;
		case 'm'://w44771 add for simple bridge
		         dhcp_surge = 1;
		         break;
		}
	}

	// brcm
        if (strlen(session_path) > 0) {
	    sprintf(status_path, "%s/%s/%s", _PATH_WAN_DIR, session_path, _PATH_MSG);
	    sprintf(pid_path, "%s/%s/%s", _PATH_WAN_DIR, session_path, _PATH_PID);
	}

	OPEN_LOG("udhcpc");
	LOG(LOG_INFO, "udhcp client (v%s) started", VERSION);

	pid_fd = pidfile_acquire(client_config.pidfile);
	pidfile_write_release(pid_fd);

	if ((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) {
		strcpy(ifr.ifr_name, client_config.interface);
		if (ioctl(fd, SIOCGIFINDEX, &ifr) == 0) {
			DEBUG(LOG_INFO, "adapter index %d", ifr.ifr_ifindex);
			client_config.ifindex = ifr.ifr_ifindex;
		} else {
			LOG(LOG_ERR, "SIOCGIFINDEX failed! %s", strerror(errno));
			exit_client(1);
		}
		if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
			memcpy(client_config.arp, ifr.ifr_hwaddr.sa_data, 6);
			DEBUG(LOG_INFO, "adapter hardware address %02x:%02x:%02x:%02x:%02x:%02x",
				client_config.arp[0], client_config.arp[1], client_config.arp[2], 
				client_config.arp[3], client_config.arp[4], client_config.arp[5]);
		} else {
			LOG(LOG_ERR, "SIOCGIFHWADDR failed! %s", strerror(errno));
			exit_client(1);
		}
	} else {
		LOG(LOG_ERR, "socket failed! %s", strerror(errno));
		exit_client(1);
	}
	close(fd);
	fd = -1;

	/* setup signal handlers */
	signal(SIGUSR1, renew_requested);
	signal(SIGUSR2, release_requested);
	signal(SIGTERM, terminate);

	/*w44771 add for 防止DHCP浪涌,begin, 2006-7-7*/
	#ifdef SUPPORT_DHCP_SURGE
	if( 1 == dhcp_surge ) 
	{
	        sprintf(cLastMac, "%02d%02d%02d", 
	        client_config.arp[3], client_config.arp[4], client_config.arp[5]);
	        
	        iSeed = atoi(cLastMac);
	        srand(iSeed);
	        iTimeRemain = rand()%7200;
            /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
	        timeout = iTimeRemain + getSysUpTime();
            /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
	}

        #endif
	/*w44771 add for 防止DHCP浪涌,end, 2006-7-7*/

    /* start of HG553_protocal : Added by c65985, 2010.3.19  DHCP Request mechanism HG553V100R001 */
    gettimeofday(&tv, NULL);  
	srand48((unsigned int)tv.tv_usec);
    /* end of HG553_protocal : Added by c65985, 2010.3.19  DHCP Request mechanism HG553V100R001 */

	state = INIT_SELECTING;
	// brcm
	// run_script(NULL, "deconfig");

	// brcm
	setStatus(0);

	for (;;) {

		// brcm
		if ((old_mode != listen_mode) || (fd == -1)) {
		    old_mode = listen_mode;

            /*START MODIFY:jaffen the fd 0/1/2 will be close when dhcp get ip ADDRESS
            so later file open/socket create may get fd 0,so it need to close fd 0*/
		    if (fd >= 0) {
			    close(fd);
			    fd = -1;
		    }
            /*END MODIFY:jaffen*/
		
		    if (listen_mode == LISTEN_RAW) {
			    if ((fd = raw_socket(client_config.ifindex)) < 0) {
				    LOG(LOG_ERR, "couldn't create raw socket -- au revoir");
				    exit_client(0);
			    }
		    }
		    else if (listen_mode == LISTEN_KERNEL) {
			    if ((fd = listen_socket(INADDR_ANY, CLIENT_PORT, client_config.interface)) < 0) {
				    LOG(LOG_ERR, "couldn't create server socket -- au revoir");
				    exit_client(0);
			    }			
		    } else 
			fd = -1;
		}
/* j00100803 Add Begin 2008-03-07 */
#if 0 //暂时屏蔽此特性
        unsigned long lNow = time(0);
        if(timeout > lNow)
        {
            tv.tv_sec = timeout - lNow;
        }
        else
        {
            tv.tv_sec = 0;
        }
#else
        /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
		tv.tv_sec = timeout - getSysUpTime();
        /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
#endif
/* j00100803 Add End 2008-03-07 */
		tv.tv_usec = 0;
		FD_ZERO(&rfds);
		if (listen_mode) FD_SET(fd, &rfds);
		
		if (tv.tv_sec > 0) {
            /*START MODIFY:jaffen If fd is no file attach to it ,just use select to wait time out
            According to Singtel test ,if use select to wait event of a null fd set ,it may cause 
            network block,may be it is a bug of kernel or poxis ,i don't konw*/
            if(fd == -1)
            {
                retval = select(0, NULL, NULL, NULL, &tv);
            }
            else
            {
    			retval = select(fd + 1, &rfds, NULL, NULL, &tv);
            }
            /*END MOFIFY:jaffen*/
		} else retval = 0; /* If we already timed out, fall through */
		
		if (retval == 0) {
			/* timeout dropped to zero */
			switch (state) {
			case INIT_SELECTING:
				// brcm
				setStatus(0);
				/* start of HG553_protocal : Added by c65985, 2010.3.19  DHCP Request mechanism HG553V100R001 */
				if (packet_num < 2) {
					if (packet_num == 0)
						xid = random_xid();

					/* send discover packet */
					send_discover(xid, requested_ip); /* broadcast */
                    /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					timeout = getSysUpTime() + ((packet_num == 1) ? REQ_TIMEOUT_T2 : REQ_TIMEOUT_T1);
                    /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					packet_num++;
				} else {
				/* end of HG553_protocal : Added by c65985, 2010.3.19  DHCP Request mechanism HG553V100R001 */
					if (client_config.abort_if_no_lease) {
						LOG(LOG_INFO,
						    "No lease, failing.");
						exit_client(1);
				  	}
					/* wait to try again */
                                        //w44771 add for A36D02465, begin, 2006-8-14
                                        discover_times ++;
					if(3 == discover_times)
					{
					    discover_times = 0;
					    
					char cmdfile[128];
   					char cmd[128];
   					
   					int ip3 = 0;
   					int ip4 = 0;
   					char rand_ip[16];

   					memset(rand_ip, 0, sizeof(rand_ip));
   					
				        sprintf(cLastMac, "%02d%02d%02d", 
				        client_config.arp[0], client_config.arp[1], client_config.arp[2]);
				        
				        iSeed = atoi(cLastMac);
				        srand(iSeed);
				        ip3 = rand()%256;//IP3 根据前3个MAC地址确定,范围0~255

				        sprintf(cLastMac, "%02d%02d%02d", 
				        client_config.arp[3], client_config.arp[4], client_config.arp[5]);
				        
				        iSeed = atoi(cLastMac);
				        srand(iSeed);
				        ip4 = rand()%255;//IP4 根据后3个MAC地址确定，范围1~254
				        if(0 == ip4)
				        {
				            ip4 = 1;
				        }   					
   					
					sprintf(cmdfile, "/proc/var/fyi/wan/%s/status", client_config.Interface);
					sprintf(cmd, "echo 4 > %s", cmdfile);
					system(cmd);
					sprintf(cmdfile, "/proc/var/fyi/wan/%s/daemonstatus", client_config.Interface);
					sprintf(cmd, "echo 0 > %s", cmdfile);
					system(cmd);
					sprintf(cmdfile, "/proc/var/fyi/wan/%s/ipaddress", client_config.Interface);
					sprintf(cmd, "echo 169.254.%d.%d > %s", ip3, ip4, cmdfile);
					system(cmd);
					
					if ( strcmp(client_config.interface,"br0") == 0 )
					{
					    sprintf(cmd, "ifconfig %s 169.254.%d.%d", client_config.Interface,ip3, ip4);
					}
					else
					{
					    sprintf(cmd, "ifconfig %s 169.254.%d.%d", client_config.interface,ip3, ip4);
					}
					system(cmd);
					}
                                        //w44771 add for A36D02465, end, 2006-8-14
					packet_num = 0;
                   /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					timeout = getSysUpTime() + INIT_TIMEOUT;
                   /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
				}
				break;
			case RENEW_REQUESTED:
			case REQUESTING:
				if (packet_num < 3) {
					/* send request packet */
					if (state == RENEW_REQUESTED)
						/* BEGIN: Modified by y67514, 2008/2/20   PN:AU8D00203*/
						send_renew_with_request(xid, server_addr, requested_ip); /* unicast */
						/* END:   Modified by y67514, 2008/2/20 */
					else send_selecting(xid, server_addr, requested_ip); /* broadcast */
					/*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					/* start of HG553_protocal : Added by c65985, 2010.3.19  DHCP Request mechanism HG553V100R001 */
					timeout = getSysUpTime() + ((packet_num == 2) ? 4 : 2);
					/* end of HG553_protocal : Added by c65985, 2010.3.19  DHCP Request mechanism HG553V100R001 */
                    /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					packet_num++;
				} else {
					/* timed out, go back to init state */
                                        //w44771 add for A36D02465, begin, 2006-8-14				    
					char cmdfile[128];
   					char cmd[128];
   					
   					int ip3 = 0;
   					int ip4 = 0;
   					char rand_ip[16];

   					memset(rand_ip, 0, sizeof(rand_ip));
   					
				        sprintf(cLastMac, "%02d%02d%02d", 
				        client_config.arp[0], client_config.arp[1], client_config.arp[2]);
				        
				        iSeed = atoi(cLastMac);
				        srand(iSeed);
				        ip3 = rand()%256;//IP3 根据前3个MAC地址确定，范围0~255

				        sprintf(cLastMac, "%02d%02d%02d", 
				        client_config.arp[3], client_config.arp[4], client_config.arp[5]);
				        
				        iSeed = atoi(cLastMac);
				        srand(iSeed);
				        ip4 = rand()%255;//IP4 根据后3个MAC地址确定范围，0~254
				        if(0 == ip4)
				        {
				            ip4 = 1;
				        }   					
   					
					sprintf(cmdfile, "/proc/var/fyi/wan/%s/status", client_config.Interface);
					sprintf(cmd, "echo 4 > %s", cmdfile);
					system(cmd);
					sprintf(cmdfile, "/proc/var/fyi/wan/%s/daemonstatus", client_config.Interface);
					sprintf(cmd, "echo 0 > %s", cmdfile);
					system(cmd);
					sprintf(cmdfile, "/proc/var/fyi/wan/%s/ipaddress", client_config.Interface);
					sprintf(cmd, "echo 169.254.%d.%d > %s", ip3, ip4, cmdfile);
					system(cmd);
					
					if ( strcmp(client_config.interface,"br0") == 0 )
					{
					    sprintf(cmd, "ifconfig %s 169.254.%d.%d", client_config.Interface,ip3, ip4 );
					}
					else
					{
					    sprintf(cmd, "ifconfig %s 169.254.%d.%d", client_config.interface, ip3, ip4);
					}
					system(cmd);
					
                                        //w44771 add for A36D02465, end, 2006-8-14
					state = INIT_SELECTING;
                    /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					timeout = getSysUpTime();
                    /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					packet_num = 0;
					listen_mode = LISTEN_RAW;
					
				}
				break;
			case BOUND:
			    /* j00100803 Add Begin 2008-03-07 */
			    #if 0  //暂时屏蔽此特性
                if(TIMER_ENUM_NORMAL == enTimerType)
                {
                    state = RENEWING;
				    DEBUG(LOG_INFO, "Entering renewing state");
                }
                /* 当前的定时器是renewing定时器 */
                else if(TIMER_ENUM_RENEWING == enTimerType)
                {
                    state = RENEWING;
                    /* renewing定时器已走完 */
                    enTimerOption58 = TIMER_ENUM_STOP;
                    /* 如果还设置了rebinding定时器 */
                    if(TIMER_ENUM_RUN == enTimerOption59)
                    {
                        enTimerType = TIMER_ENUM_REBINDING;
                    }
                    /* 未设置其它定时器了 */
                    else
                    {
                        enTimerType = TIMER_ENUM_NORMAL;
                    }
                    DEBUG(LOG_INFO, "Entering renewing state");
                }
                /* 当前的定时器是rebinding定时器 */
                else if(TIMER_ENUM_REBINDING == enTimerType)
                {
                    state = REBINDING;
                    /* rebinding 定时器已走完 */
                    enTimerOption59 = TIMER_ENUM_STOP;
                    enTimerType = TIMER_ENUM_NORMAL;
                    
				    DEBUG(LOG_INFO, "Entering rebinding state");
				    /* 跳入rebinding状态，直接返回最上面不等待的select */
				    break;
                }
                
			    #else
				/* Lease is starting to run out, time to enter renewing state */
				state = RENEWING;
			    #endif
				/* j00100803 Add End 2008-03-07 */
				listen_mode = LISTEN_KERNEL;
				DEBUG(LOG_INFO, "Entering renew state");
				/* fall right through */
			case RENEWING:
			    /* j00100803 Add Begin 2008-03-07 */
			    #if 0  //暂时屏蔽此特性
			    if(TIMER_ENUM_NORMAL == enTimerType)
			    {
    			    if ((t2 - t1) <= (lease / 14400 + 1))
    			    {
    					/* timed out, enter rebinding state */
    					state = REBINDING;
    					timeout = time(0) + (t2 - t1);
    					DEBUG(LOG_INFO, "Entering rebinding state");
    				}
    				else
    				{
    					/* send a request packet */
    					send_renew(xid, server_addr, requested_ip); /* unicast */
    					
    					t1 = (t2 - t1) / 2 + t1;
    					timeout = t1 + start;
    				}
				}
				else if(TIMER_ENUM_REBINDING == enTimerType)
				{
                    /* send a request packet */
					send_renew(xid, server_addr, requested_ip); /* unicast */
					
					t1 = (t2 - t1) / 2 + t1;
					timeout = t1 + start;
				}
				
			    #else
				/* Either set a new T1, or enter REBINDING state */
				if ((t2 - t1) <= (lease / 14400 + 1)) {
					/* timed out, enter rebinding state */
					state = REBINDING;
                    /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					timeout = getSysUpTime() + (t2 - t1);
                    /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					DEBUG(LOG_INFO, "Entering rebinding state");
				} else {
					/* send a request packet */
					send_renew(xid, server_addr, requested_ip); /* unicast */
					
					t1 = (t2 - t1) / 2 + t1;
					timeout = t1 + start;
				}
				#endif
				/* j00100803 Add End 2008-03--07 */
				break;
			case REBINDING:
				/* Either set a new T2, or enter INIT state */
				if ((lease - t2) <= (lease / 14400 + 1)) {
                                        //w44771 add for A36D02465, begin, 2006-8-14				    
					char cmdfile[128];
   					char cmd[128];
   					
   					int ip3 = 0;
   					int ip4 = 0;
   					char rand_ip[16];

   					memset(rand_ip, 0, sizeof(rand_ip));
   					
				        sprintf(cLastMac, "%02d%02d%02d", 
				        client_config.arp[0], client_config.arp[1], client_config.arp[2]);
				        
				        iSeed = atoi(cLastMac);
				        srand(iSeed);
				        ip3 = rand()%256;//IP3 根据前3个MAC地址确定，范围0~255

				        sprintf(cLastMac, "%02d%02d%02d", 
				        client_config.arp[3], client_config.arp[4], client_config.arp[5]);
				        
				        iSeed = atoi(cLastMac);
				        srand(iSeed);
				        ip4 = rand()%255;//IP4 根据后3个MAC地址确定范围，0~254
				        if(0 == ip4)
				        {
				            ip4 = 1;
				        }   					
   					
					sprintf(cmdfile, "/proc/var/fyi/wan/%s/status", client_config.Interface);
					sprintf(cmd, "echo 4 > %s", cmdfile);
					system(cmd);
					sprintf(cmdfile, "/proc/var/fyi/wan/%s/daemonstatus", client_config.Interface);
					sprintf(cmd, "echo 0 > %s", cmdfile);
					system(cmd);
					sprintf(cmdfile, "/proc/var/fyi/wan/%s/ipaddress", client_config.Interface);
					sprintf(cmd, "echo 169.254.%d.%d > %s", ip3, ip4, cmdfile);
					system(cmd);
					
					if ( strcmp(client_config.interface,"br0") == 0 )
					{
					    sprintf(cmd, "ifconfig %s 169.254.%d.%d", client_config.Interface,ip3, ip4 );
					}
					else
					{
					    sprintf(cmd, "ifconfig %s 169.254.%d.%d", client_config.interface, ip3, ip4);
					}
					system(cmd);
					
                                        //w44771 add for A36D02465, end, 2006-8-14
					
					/* timed out, enter init state */
					state = INIT_SELECTING;
					LOG(LOG_INFO, "Lease lost, entering init state");
					run_script(NULL, "deconfig");
                    /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					timeout = getSysUpTime();
                    /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					packet_num = 0;
					listen_mode = LISTEN_RAW;
				} else {
					/* send a request packet */
					send_renew(xid, 0, requested_ip); /* broadcast */

					t2 = (lease - t2) / 2 + t2;
					timeout = t2 + start;
				}
				break;
			case RELEASED:
				/* yah, I know, *you* say it would never happen */
                 /* BEGIN: Modified by y67514, 2008/6/20   问题单号:time是long型值*/
                timeout = 0x7fffffff;
                /* END:   Modified by y67514, 2008/6/20 */
				break;
			}
		} else if (retval > 0 && listen_mode != LISTEN_NONE && FD_ISSET(fd, &rfds)) {
			/* a packet is ready, read it */
			
			if (listen_mode == LISTEN_KERNEL) {
				if (get_packet(&packet, fd) < 0) continue;
			} else {
				if (get_raw_packet(&packet, fd) < 0) continue;
			} 
			
			if (packet.xid != xid) {
				DEBUG(LOG_INFO, "Ignoring XID %lx (our xid is %lx)",
					(unsigned long) packet.xid, xid);
				continue;
			}
			
			if ((message = get_option(&packet, DHCP_MESSAGE_TYPE)) == NULL) {
				DEBUG(LOG_ERR, "couldnt get option from packet -- ignoring");
				continue;
			}
			
			switch (state) {
			case INIT_SELECTING:
				/* Must be a DHCPOFFER to one of our xid's */
				if (*message == DHCPOFFER) {
					if ((temp = get_option(&packet, DHCP_SERVER_ID))) {
						memcpy(&server_addr, temp, 4);
						xid = packet.xid;
						requested_ip = packet.yiaddr;
						
						/* enter requesting state */
						state = REQUESTING;
                        /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
						timeout = getSysUpTime();
                        /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
						packet_num = 0;
					} else {
						DEBUG(LOG_ERR, "No server ID in message");
					}
				}
				break;
			case RENEW_REQUESTED:
			case REQUESTING:
			case RENEWING:
			case REBINDING:
				if (*message == DHCPACK) {
					if (!(temp = get_option(&packet, DHCP_LEASE_TIME))) {
						LOG(LOG_ERR, "No lease time with ACK, using 1 hour lease");
						lease = 60*60;
					} else {
						memcpy(&lease, temp, 4);
						lease = ntohl(lease);
					}
					/* j00100803 Add Begin 2008-03-07 */
				#if 0  //暂时屏蔽此特性
					char * pDataBegin = NULL;
					if(NULL != (pDataBegin = get_option(&packet, DHCP_T1)))
					{
					    int iOptionLen = (unsigned int)*(pDataBegin - 1);
					    if(iOptionLen == 4)
					    {
    					    int * pTmp = &iRenewingTimerOption58;
    					    memcpy(pTmp, pDataBegin, 4);
					    }
					}
                    pDataBegin = NULL;
					if(NULL != (pDataBegin = get_option(&packet, DHCP_T2)))
					{
					    int iOptionLen = (unsigned int)*(pDataBegin - 1);
					    if(iOptionLen == 4)
					    {   
    					    int * pTmp = &iRebindingTimerOption59;
    					    memcpy(pTmp, pDataBegin, 4);
					    }
					}
					if(iRenewingTimerOption58 > 0)
					{
					    t1 = iRenewingTimerOption58;
					    enTimerOption58 = TIMER_ENUM_RUN;
					    enTimerType = TIMER_ENUM_RENEWING;
					    iRenewingTimerOption58 = 0;
					}
                    else
                    {
                        /* 默认配置 */
                        enTimerOption58 = TIMER_ENUM_STOP;
                        enTimerType = TIMER_ENUM_NORMAL;
                        t1 = lease / 2;
                    }

                    if(iRebindingTimerOption59 > 0)
                    {
                        t2 = iRebindingTimerOption59;
                        enTimerOption59 = TIMER_ENUM_RUN;
                        if(enTimerType != TIMER_ENUM_RENEWING)
                        {
                            enTimerType = TIMER_ENUM_REBINDING;
                        }
                        iRebindingTimerOption59 = 0;
                    }
                    else 
                    {
                        /* 默认配置 */
                        enTimerOption59 = TIMER_ENUM_STOP;
                        t2 = (lease * 0x7) >> 3;
                    }
                    
                    temp_addr.s_addr = packet.yiaddr;
					LOG(LOG_INFO, "Lease of %s obtained, lease time %ld", 
						inet_ntoa(temp_addr), lease);
					/* 用start记录当前计时器开始的时刻 */
					start = time(0);
                    /* 非正常情况 */
                    if(t1 > t2)
                    {
                        /* 恢复缺省值 */
                        t1 = lease / 2;
                        t2 = (lease * 0x7) >> 3;
                        timeout = t1 + start;
                        enTimerOption58 = TIMER_ENUM_STOP;
                        enTimerOption59 = TIMER_ENUM_STOP;
                        enTimerType = TIMER_ENUM_NORMAL;                        
                    }
                    /* 正常情况 */
                    else
                    {
                        timeout = t1 + start; 
                    }
					#else
					/* enter bound state */
					t1 = lease / 2;
					
					/* little fixed point for n * .875 */
					t2 = (lease * 0x7) >> 3;
					temp_addr.s_addr = packet.yiaddr;
					LOG(LOG_INFO, "Lease of %s obtained, lease time %ld", 
						inet_ntoa(temp_addr), lease);
                    /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					start = getSysUpTime();
                    /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					timeout = t1 + start;
					#endif
					/* j00100803 Add Begin 2008-03-07 */
					requested_ip = packet.yiaddr;
					/* 这里解析注册的所有option选项 */
					run_script(&packet,
						   ((state == RENEWING || state == REBINDING) ? "renew" : "bound"));

                               /*start of  2008.05.04 HG553V100R001C02B013 AU8D00566 */
					//添加option121的路由
					run_staticRoute();
                               /*end of  2008.05.04 HG553V100R001C02B013 AU8D00566 */

					state = BOUND;
					//listen_mode = LISTEN_NONE;
					listen_mode = LISTEN_KERNEL;
                    /* BEGIN: Added by KF19989, 2009/11/19: Automatic ACL DHCP */
                    Automatic_ACL_DHCP(dhcp_file);					
                    /* END:  Added by KF19989, 2009/11/19: Automatic ACL DHCP */
					
					// brcm
					setStatus(1);
					background();
					
				} else if (*message == DHCPNAK) {
					/* return to init state */
					LOG(LOG_INFO, "Received DHCP NAK");
					if (state != REQUESTING)
						run_script(NULL, "deconfig");
					state = INIT_SELECTING;
                    /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					timeout = getSysUpTime();
                    /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					requested_ip = 0;
					packet_num = 0;
					listen_mode = LISTEN_RAW;

					// brcm
					setStatus(0);
					 char cmd[128] = "";
					  sprintf(cmd, "echo 4 > /proc/var/fyi/wan/%s/status", session_path);
					system(cmd); 
				}
//#ifdef VDF_FORCERENEW
#if 0
				else if(DHCPFORCERENEW == *message)		//支持RFC3203:forcerenew
				{
					printf("VDF:%s:%s:%d:Received forcerenew***\n",__FILE__,__FUNCTION__,__LINE__);
					renew_requested(0);
				}
#endif
				break;
			case BOUND:
#ifdef VDF_FORCERENEW
				if(DHCPFORCERENEW == *message)		//支持RFC3203:forcerenew
				{
					printf("VDF:%s:%s:%d:Received forcerenew***\n",__FILE__,__FUNCTION__,__LINE__);
					renew_requested(0);
				}
#endif
				break;
			case RELEASED:

				/* ignore all packets */
				break;
			}					
		} else if (retval == -1 && errno == EINTR) {
			/* a signal was caught */
			
		} else {
			/* An error occured */
			DEBUG(LOG_ERR, "Error on select");
		}
		
	}
	return 0;
}

