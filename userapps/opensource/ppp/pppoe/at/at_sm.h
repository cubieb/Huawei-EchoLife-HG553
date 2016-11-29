/***********************************************************************
  版权信息: 版权所有(C) 1988-2005, 华为技术有限公司.
  文件名: at_sm.h
  作者: lichangqing 45517
  版本: V500R003
  创建日期: 2005-8-8
  完成日期: 2005-8-10
  功能描述: 头文件
      
  主要函数列表: 无
      
  修改历史记录列表: 
    <作  者>    <修改时间>  <版本>  <修改描述>
    l45517      20050816    0.0.1    初始完成
  备注: 
************************************************************************/

#ifndef __AT_SM_H
#define __AT_SM_H

// 为每条AT命令分配的序号
#define AT_RSSI_QUERY    			 0	 // RSSI查询功能命令			 RSSI_QUERY
#define AT_SYSINFO_QUERY			 1	 // 系统信息查询				 SYSTEM_INFO_QUERY
#define AT_SYSCFG_SET  				 2 	//设置连接类型				SYSCFG_SET
#define AT_PDP_SET					 3	//定义PDP关联					PDP_SET
#define AT_AUTO_SERVICE_REPORT 	 4	//服务状态变化指示			AUTO_SERVICE_STATUS_REPORT
#define AT_AUTO_SYSTEM_REPORT		 5	//系统模式变化事件指示 	AUTO_SYSTEM_MODE_STATUS_REPORT
#if 0
#define AT_0         0 // 测试命令         AT       <CR><LF>OK<CR><LF>*/
#define AT_GMR_1     1 // 软件版本号，     +GMR     <CR><LF>+GMR:<softversion><CR><LF>   有MS相关错误时：<CR><LF>ERROR<CR><LF>
#define AT_HWVER_2   2 // 硬件版本号查询   ^HWVER   <CR><LF>^HWVER:<hardversion><CR><LF> 有MS相关错误时：<CR><LF>ERROR<CR><LF>
#define AT_ESN_3     3 // ESN查询命令      +GSN     <CR><LF>+GSN: <ESN><CR><LF>          有MS相关错误时：<CR><LF>ERROR<CR><LF>
#define AT_SYSINFO_4 4 // 系统信息查询     ^SYSINFO <CR><LF>^SYSINFO:< srv_status >,< srv_domain >,< roam_status >,< sys_mode >,< sim_state ><CR><LF>
#define AT_CSQ_5     5 // RSSI查询功能命令 +CSQ     <CR><LF>+CSQ: <rssi>,<ber><CR><LF><CR><LF>OK<CR><LF> 有MS相关错误时：<CR><LF>+CME ERROR: <err><CR><LF>
#define AT_HDRCSQ_6  6 // evdo RSSI 查询   ^HDRCSQ  <CR><LF>^HDRRSSI: <rssi><CR><LF>
#define AT_NETMODE   7//  网络模式设置   

/*add by sxg*/
#define AT_PDP_CONTEXT 8//定义PDP关联
#define AT_OPERATOR    9 //设置运营商选择模式: 自动/手动
#define AT_SYSCFG   10 //设置连接类型: GPRS/3G,设置通道限制类型
#endif


#define MODEM_MODULE 3
#define MSG_AT_QUEUE  100
#define MSG_MODEM_QUEUE  102
#define	SYSTEM_INFO_QUERY	34
#define	RSSI_QUERY	31
#define	SYSCFG_SET	39
#define	PDP_SET	51
#define	AUTO_SERVICE_STATUS_REPORT	67
#define	AUTO_SYSTEM_MODE_STATUS_REPORT	68

/* Bits in auth_pending[] */
#define PAP_WITHPEER	1
#define PAP_PEER	2
#define CHAP_WITHPEER	4
#define CHAP_PEER	8
#define TMPFILE_MAX_ITEM_VALUE      512
#define TMPFILE_MAX_ITEM_NAME       16

#ifndef _AT_RCV_MSG_LENGTH_
#define _AT_RCV_MSG_LENGTH_
//#define AT_RCV_MSG_LENGTH  252   //接收消息缓冲区长度
#define AT_RCV_MSG_LENGTH  512   //接收消息缓冲区长度
#endif
#ifndef _AT_SND_MSG_LENGTH_
#define _AT_SND_MSG_LENGTH_
//#define AT_SND_MSG_LENGTH  128    //发送消息缓冲区长度
#define AT_SND_MSG_LENGTH  512    //发送消息缓冲区长度
#endif
#ifndef _stAtRcvMsg_
#define _stAtRcvMsg_
struct stAtRcvMsg
{
	long int lMsgType; 
	char acText[AT_RCV_MSG_LENGTH];
};
#endif
#ifndef _stAtSndMsg_
#define _stAtSndMsg_
struct stAtSndMsg
{
	long int lMsgType;//命令所对应的宏	
	char acParam[AT_SND_MSG_LENGTH];
};
#endif
struct stMsg_RSSI_Query
{
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucRSSI;
	unsigned char ucBer;
};
struct stMsg_System_Info_Query
{
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucSrvStatus;
	unsigned char ucSrvDomain;
	unsigned char ucRoamStatus;
	unsigned char ucSysMode;
	unsigned char ucSimState; 
	unsigned char ucLockState;
	unsigned char ucSysSubMode;
	unsigned char ucExtern1;
	unsigned char ucExtern2;
	unsigned char ucExtern3;
};
struct stMsg_Syscfg_Set
{
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucExtern1;
	unsigned char ucExtern2;
};
struct stMsg_PDP_Set
{
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucExtern1;
	unsigned char ucExtern2;
};
struct stMsg_Service_Status
{ 
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucSrvStatus;
	unsigned char ucExtern1;
};
struct stMsg_System_Mode
{ 
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucSysMode;
	unsigned char ucSubSysMode;
};

typedef struct tag_TMPFILE_MSG
{
    long lMsgType;                          //消息类型，包括写，读，读反馈
    int iResult;                            //操作结果
    int iPid;                               //发送消息的PID，作为读响应消息的类型值
    int iSerial;                            //消息序列号，避免同一进程读取消息顺序错误
    char achName[TMPFILE_MAX_ITEM_NAME];    //读写变量名
    char achValue[TMPFILE_MAX_ITEM_VALUE];  //读写变量值
}S_TMPFILE_MSG;
#define BCM_PPPOE_AUTH_FAILED                7
#define TMPFILE_KEY_PATH    "/var"
#define TMPFILE_KEY_SEED    't'
#define TMPFILE_READ_OPT    2

int at_sm_is_active();
int at_sm_submit_at(int at_num,  int* is_report_err, char* param);
//int at_sm_initialize();
//int at_sm_destroy();
int at_sm_modem_submit_at(int modem_fd, char* at_dial_str);

#define SYSMODE_WCDMA       5
#define SYSMODE_CDMA        2
#define SYSMODE_NO_SERVICES 0

#define LOOP_TIMES 5

#define MAX_CMD_LEN    9  // 上报命令的最大匹配长度
#define MAX_REPORT_NUM 7  // 允许上报的数目
#define READ_SIZE      64 // 每此试图从串口读64个字符
#define AT_REPORT_OK   1
#define AT_REPORT_ERR  0
#define PARSE_OK		1
#define PARSE_ERROR		0
// A064D00428 EC506 ADD (by l45517 2005年11月21?) BEGIN
#define AT_REPORT_BUSY AT_REPORT_OK
// A064D00428 EC506 ADD (by l45517 2005年11月21?) END

#if 0
typedef struct AT_HANDLE_S
{
    char* at_cmd_str;
    char* at_report_str;
    char* at_report_err_str;
    void  (*create_at_cmd)(char* buf, char* param);
    int  (*at_sm_report_handler)(char* report_buf);
}AT_HANDLE_T;
#endif
typedef struct AT_HANDLE_S
{
	int at_cmd_id;
	void* at_cmd_struct;
	int  (*at_cmd_report_handler)(struct stAtRcvMsg *pstAtRcvMsg);
	char* at_cmd_str;
	void  (*create_at_cmd)(char* buf, char* param);
}AT_HANDLE_T;

 
#endif // __AT_SM_H

