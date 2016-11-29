/***********************************************************************
  ��Ȩ��Ϣ: ��Ȩ����(C) 1988-2005, ��Ϊ�������޹�˾.
  �ļ���: at_sm.h
  ����: lichangqing 45517
  �汾: V500R003
  ��������: 2005-8-8
  �������: 2005-8-10
  ��������: ͷ�ļ�
      
  ��Ҫ�����б�: ��
      
  �޸���ʷ��¼�б�: 
    <��  ��>    <�޸�ʱ��>  <�汾>  <�޸�����>
    l45517      20050816    0.0.1    ��ʼ���
  ��ע: 
************************************************************************/

#ifndef __AT_SM_H
#define __AT_SM_H

// Ϊÿ��AT�����������
#define AT_RSSI_QUERY    			 0	 // RSSI��ѯ��������			 RSSI_QUERY
#define AT_SYSINFO_QUERY			 1	 // ϵͳ��Ϣ��ѯ				 SYSTEM_INFO_QUERY
#define AT_SYSCFG_SET  				 2 	//������������				SYSCFG_SET
#define AT_PDP_SET					 3	//����PDP����					PDP_SET
#define AT_AUTO_SERVICE_REPORT 	 4	//����״̬�仯ָʾ			AUTO_SERVICE_STATUS_REPORT
#define AT_AUTO_SYSTEM_REPORT		 5	//ϵͳģʽ�仯�¼�ָʾ 	AUTO_SYSTEM_MODE_STATUS_REPORT
#if 0
#define AT_0         0 // ��������         AT       <CR><LF>OK<CR><LF>*/
#define AT_GMR_1     1 // ����汾�ţ�     +GMR     <CR><LF>+GMR:<softversion><CR><LF>   ��MS��ش���ʱ��<CR><LF>ERROR<CR><LF>
#define AT_HWVER_2   2 // Ӳ���汾�Ų�ѯ   ^HWVER   <CR><LF>^HWVER:<hardversion><CR><LF> ��MS��ش���ʱ��<CR><LF>ERROR<CR><LF>
#define AT_ESN_3     3 // ESN��ѯ����      +GSN     <CR><LF>+GSN: <ESN><CR><LF>          ��MS��ش���ʱ��<CR><LF>ERROR<CR><LF>
#define AT_SYSINFO_4 4 // ϵͳ��Ϣ��ѯ     ^SYSINFO <CR><LF>^SYSINFO:< srv_status >,< srv_domain >,< roam_status >,< sys_mode >,< sim_state ><CR><LF>
#define AT_CSQ_5     5 // RSSI��ѯ�������� +CSQ     <CR><LF>+CSQ: <rssi>,<ber><CR><LF><CR><LF>OK<CR><LF> ��MS��ش���ʱ��<CR><LF>+CME ERROR: <err><CR><LF>
#define AT_HDRCSQ_6  6 // evdo RSSI ��ѯ   ^HDRCSQ  <CR><LF>^HDRRSSI: <rssi><CR><LF>
#define AT_NETMODE   7//  ����ģʽ����   

/*add by sxg*/
#define AT_PDP_CONTEXT 8//����PDP����
#define AT_OPERATOR    9 //������Ӫ��ѡ��ģʽ: �Զ�/�ֶ�
#define AT_SYSCFG   10 //������������: GPRS/3G,����ͨ����������
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
//#define AT_RCV_MSG_LENGTH  252   //������Ϣ����������
#define AT_RCV_MSG_LENGTH  512   //������Ϣ����������
#endif
#ifndef _AT_SND_MSG_LENGTH_
#define _AT_SND_MSG_LENGTH_
//#define AT_SND_MSG_LENGTH  128    //������Ϣ����������
#define AT_SND_MSG_LENGTH  512    //������Ϣ����������
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
	long int lMsgType;//��������Ӧ�ĺ�	
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
    long lMsgType;                          //��Ϣ���ͣ�����д������������
    int iResult;                            //�������
    int iPid;                               //������Ϣ��PID����Ϊ����Ӧ��Ϣ������ֵ
    int iSerial;                            //��Ϣ���кţ�����ͬһ���̶�ȡ��Ϣ˳�����
    char achName[TMPFILE_MAX_ITEM_NAME];    //��д������
    char achValue[TMPFILE_MAX_ITEM_VALUE];  //��д����ֵ
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

#define MAX_CMD_LEN    9  // �ϱ���������ƥ�䳤��
#define MAX_REPORT_NUM 7  // �����ϱ�����Ŀ
#define READ_SIZE      64 // ÿ����ͼ�Ӵ��ڶ�64���ַ�
#define AT_REPORT_OK   1
#define AT_REPORT_ERR  0
#define PARSE_OK		1
#define PARSE_ERROR		0
// A064D00428 EC506 ADD (by l45517 2005��11��21?) BEGIN
#define AT_REPORT_BUSY AT_REPORT_OK
// A064D00428 EC506 ADD (by l45517 2005��11��21?) END

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

