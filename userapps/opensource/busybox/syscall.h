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
/* ����ֵ����󳤶� */
#define TMPFILE_MAX_ITEM_VALUE      512



#define TMPFILE_WRITE_OPT   1
#define TMPFILE_READ_OPT    2
#define TMPFILE_READ_ACK    3
/* ��Ϣ����keyֵ������Ҫ�Ĳ��� */
#define TMPFILE_KEY_PATH    "/var"
#define TMPFILE_KEY_SEED    't'


/* varĿ¼���ļ���д��Ϣ */
typedef struct tag_TMPFILE_MSG
{
    long lMsgType;                          //��Ϣ���ͣ�����д������������
    int iResult;                            //�������
    int iPid;                               //������Ϣ��PID����Ϊ����Ӧ��Ϣ������ֵ
    int iSerial;                            //��Ϣ���кţ�����ͬһ���̶�ȡ��Ϣ˳�����
    char achName[TMPFILE_MAX_ITEM_NAME];    //��д������
    char achValue[TMPFILE_MAX_ITEM_VALUE];  //��д����ֵ
}S_TMPFILE_MSG;
extern int tmpfile_writevalue(char *pchName, char *pchValue);
extern int tmpfile_getvalue(char *pchName, char *pchValue, int *piLen);




 /*������HSPA���ƽ��̵���Ϣ�ṹ��*/
struct stAtSndMsg
{
	long int lMsgType;//��������Ӧ�ĺ�	
	char acParam[AT_SND_MSG_LENGTH];
};

/*��������HSPA���ƽ��̵���Ϣ�ṹ��*/
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
