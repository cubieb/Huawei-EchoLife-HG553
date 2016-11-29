#ifndef __SYS_CALL_H__
#define __SYS_CALL_H__
#include <netinet/in.h>


#ifndef _UINT32_
#define _UINT32_
typedef unsigned long   UINT32;
#endif
typedef unsigned char   byte;
#define XML_PSI_START_TAG      "<psitree>"
#define XML_PSI_END_TAG        "</psitree>"
typedef enum {
   CLI_ACCESS_DISABLED = 0,
   CLI_ACCESS_LOCAL,
   CLI_ACCESS_REMOTE,
   CLI_ACCESS_CONSOLE,
   CLI_ACCESS_REMOTE_SSH,     
   CLI_ACCESS_REMOTE_TELNET,
} CLI_ACCESS_WHERE;

#define RENAME_BFTPD      0x20c

#define SYS_CMD_LEN        512
#define CLI_MAX_BUF_SZ       512
#define TRUE 1
#define FALSE 0
typedef enum _psiStatus {
   PSI_STS_OK = 0,
   PSI_STS_ERR_GENERAL,
   PSI_STS_ERR_MEMORY,
   PSI_STS_ERR_OBJECT_NOT_FOUND,
   PSI_STS_ERR_APP_NOT_FOUND,
   PSI_STS_ERR_OBJECT_EXIST,
   PSI_STS_ERR_APP_EXIST,
   PSI_STS_ERR_INVALID_LENGTH,
   PSI_STS_ERR_INVALID_VALUE,
   PSI_STS_ERR_INVALID_CRC,
   PSI_STS_ERR_INVALID_MAGIC_VALUE,
   PSI_STS_ERR_INVALID_PROFILE_NUM,
   PSI_STS_ERR_PROFILE_NAME,
   PSI_STS_ERR_PROFILE_TOO_LARGE,
   PSI_STS_ERR_CONFIGFILE_UNKNOWN,
   PSI_STS_ERR_CONFIGFILE_SIZE
} PSI_STATUS;


#if defined(__cplusplus)
extern "C" {
#endif
#define bcmSystem(cmd)		bcmSystemEx (cmd,1)
int bcmSystemEx (char *command, int printFlag);

int bcmCheckInterfaceUp(char *devname);

int bcmIsModuleInserted(char *modName);

int bcmGetLanInfo(char *lan_ifname, struct in_addr *lan_ip, struct in_addr *lan_subnetmask);
int isAccessFromLan(struct in_addr clntAddr);
int checkChipId(char *strTagChipId, char *sig2);
void bcmRemoveModules(int lanIf);

#define BCM_PPPOE_CLIENT_STATE_DOWN          3   /* totally down */
#define BCM_PPPOE_CLIENT_STATE_UP            4   /* totally up */
#define BCM_PPPOE_SERVICE_AVAILABLE          5   /* ppp service is available on the remote */

#define BCM_PPPOE_AUTH_FAILED                7

#define TMPFILE_MAX_ITEM_NAME       16
/* 表项值的最大长度 */
#define TMPFILE_MAX_ITEM_VALUE      512



#define TMPFILE_WRITE_OPT   1
#define TMPFILE_READ_OPT    2
#define TMPFILE_READ_ACK    3
/* 消息队列key值生成需要的参数 */
#define TMPFILE_KEY_PATH    "/var"
#define TMPFILE_KEY_SEED    't'


/* var目录下文件读写消息 */
typedef struct tag_TMPFILE_MSG
{
    long lMsgType;                          //消息类型，包括写，读，读反馈
    int iResult;                            //操作结果
    int iPid;                               //发送消息的PID，作为读响应消息的类型值
    int iSerial;                            //消息序列号，避免同一进程读取消息顺序错误
    char achName[TMPFILE_MAX_ITEM_NAME];    //读写变量名
    char achValue[TMPFILE_MAX_ITEM_VALUE];  //读写变量值
}S_TMPFILE_MSG;
extern int tmpfile_writevalue(char *pchName, char *pchValue);
extern int tmpfile_getvalue(char *pchName, char *pchValue, int *piLen);




 /*发送往HSPA控制进程的消息结构体*/
struct stAtSndMsg
{
	long int lMsgType;//命令所对应的宏	
	char acParam[AT_SND_MSG_LENGTH];
};

/*接收来自HSPA控制进程的消息结构体*/
struct stAtRcvMsg
{
	long int lMsgType; 
	char acText[AT_RCV_MSG_LENGTH];
};
 int AtRcv( int lMsgKey, struct stAtRcvMsg *pstAtRcvMsg, int lMsgType );
 int AtSend(int lMsgKey, struct stAtSndMsg *pstAtSndMsg, char cFrom,  int lMsgType,  const char *pszParam);
 
#if defined(__cplusplus)
}
#endif

#endif
