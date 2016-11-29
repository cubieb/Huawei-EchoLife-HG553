/***********************************************************************
  ��Ȩ��Ϣ : ��Ȩ����(C) 1988-2005, ��Ϊ�������޹�˾.
  �ļ���   : at_sm.c
  ����     : lichangqing 45517
  �汾     : V500R003
  �������� : 2005-8-8
  ������� : 2005-8-10
  �������� : at״̬����ʵ��
      
  ��Ҫ�����б�: 
      
  �޸���ʷ��¼�б�: 
    <��  ��>    <�޸�ʱ��>  <�汾>  <�޸�����>
    l45517      20050816    0.0.1    ��ʼ���
  ��ע: 
************************************************************************/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/wait.h>

#include <termios.h>
#include <unistd.h>
#include <setjmp.h>

#include "pppd.h"
#include "at_sm.h"
#include "at_thread.h"
#include "utils_lib.h"

#include <sys/ipc.h>
#include <sys/msg.h>


static int at_report_handler_rssi (struct stAtRcvMsg *pstAtRcvMsg);
static int at_report_handler_sysinfo (struct stAtRcvMsg *pstAtRcvMsg);
static int at_report_handler_cfg_set (struct stAtRcvMsg *pstAtRcvMsg);
static int at_report_handler_pdp_set (struct stAtRcvMsg *pstAtRcvMsg);
static int at_report_handler_service_sts (struct stAtRcvMsg *pstAtRcvMsg);
static int at_report_handler_sys_mode (struct stAtRcvMsg *pstAtRcvMsg);

static void  at_create_pdp_context_cmd(char* buf, char* param);
static void  at_create_syscfg_cmd(char* buf, char* param);


struct stMsg_RSSI_Query			rssi_get = {0,0,0,0};
struct stMsg_System_Info_Query	sys_info_get = {0,0,0,0,0,0,0,0,0};
struct stMsg_Syscfg_Set			cfg_set = {0,0};
struct stMsg_PDP_Set				pdp_set = {0,0};
struct stMsg_Service_Status		rp_service_sts = {0,0,0};
struct stMsg_System_Mode			rp_sys_mode = {0,0,0,0};

int AtSndMsgId = -1;
int AtRcvMsgId = -1;

AT_HANDLE_T  g_at_handles[] =
{
	{RSSI_QUERY,&rssi_get,at_report_handler_rssi,"AT+CSQ",NULL},
	{SYSTEM_INFO_QUERY,&sys_info_get,at_report_handler_sysinfo,"AT^SYSINFO",NULL},
	{SYSCFG_SET,&cfg_set,at_report_handler_cfg_set,"AT^SYSCFG=",at_create_syscfg_cmd},
	{PDP_SET,&pdp_set,at_report_handler_pdp_set,"AT+CGDCONT=",at_create_pdp_context_cmd},
	{AUTO_SERVICE_STATUS_REPORT,&rp_service_sts,at_report_handler_service_sts,NULL,NULL},
	{AUTO_SYSTEM_MODE_STATUS_REPORT,&rp_sys_mode,at_report_handler_sys_mode,NULL,NULL},
	{0,NULL,NULL,NULL,NULL},
};

/*
// Ϊÿ��AT�����������
#define AT_RSSI_QUERY    				 0	 // RSSI��ѯ��������			RSSI_QUERY
#define AT_SYSINFO_QUERY			 1	 // ϵͳ��Ϣ��ѯ				SYSTEM_INFO_QUERY
#define AT_SYSCFG_SET  				 2 	//������������				SYSCFG_SET
#define AT_PDP_SET					 3	//����PDP����					PDP_SET
#define AT_AUTO_SERVICE_REPORT 		 4	//����״̬�仯ָʾ		AUTO_SERVICE_STATUS_REPORT
#define AT_AUTO_SYSTEM_REPORT		 5	//ϵͳģʽ�仯�¼�ָʾ 	AUTO_SYSTEM_MODE_STATUS_REPORT

*/
int g_i_at_rcv_thread_killed = 0; // �ñ���Ŀǰδʵ�֣�����signal���������ʵ��
//int g_pcui_fd  = -1;

THREAD_S         g_st_at_report_thread;  // �߳������ṹ
static WAIT_S    g_st_submit_at_wait;    // �ȴ��ϱ������̻߳��ѵ��¼�
static int       g_i_is_sm_active;       // �Ƿ����߳��������·�AT����;
//MUTEX_S          g_st_cur_at_num_mutex;  // ���ϱ��߳�ͬ����mutex;
//MUTEX_S          g_st_pcui_wr_mutex;     // ���ϱ��߳�ͬ����mutex;

//static int       g_i_cur_at_num        = -1;    // �ݴ� at_sm_submit_at�ύ��at�������
void*            g_at_result           = NULL;  // �ݴ� at_sm_submit_at�ύ�ķ��ؽ��ָ��
int              g_i_is_report_ok;              // ��AT�ϱ�error���ϱ���ʽ����ֵΪ0������Ϊ1
//static           sigjmp_buf g_sigjmp;
static int       rcv_waiting = 0;               // �ϱ������̴߳��ڵȴ��ϱ�״̬

extern int g_main_thread_killed;

/*------------------------------------------------------------
  ����ԭ��: static int  at_create_pdp_context_cmd(char* buf, char* parameters)
  ����: ��������AT����,
  ����: AT������Ҫ�Ĳ���
  ���: ��
  ����ֵ:���ɵ�AT����
-------------------------------------------------------------*/
static void  at_create_pdp_context_cmd(char* buf, char* param)
{
    sprintf(buf,"AT+CGDCONT=%s", param);
}


/*------------------------------------------------------------
  ����ԭ��: static int  at_create_syscfg_cmd(char* buf, char* parameters)
  ����: ��������AT����,
  ����: AT������Ҫ�Ĳ���
  ���: ��
  ����ֵ:���ɵ�AT����
-------------------------------------------------------------*/
static void  at_create_syscfg_cmd(char* buf, char* param)
{
    sprintf(buf,"AT^SYSCFG=%s", param);
}

#if 1
/*------------------------------------------------------------
  ����ԭ��: static inline int AtRcv(int lMsgKey, struct stAtRcvMsg *pstAtRcvMsg, int lMsgType)  
  ����: ����HSPA�ӿ�ģ�����Ϣ
  ����: lMsgId:��ϢID��pstAtRcvMsg :��Ϣ�ṹ��ָ��;lMsgType:��Ϣ����
  ���: ��
  ����ֵ:
-------------------------------------------------------------*/

int AtRcv(int lMsgId, struct stAtRcvMsg *pstAtRcvMsg, int lMsgType)
{
    /* BEGIN: Modified by y67514, 2008/6/20   ���ⵥ��:�Ż�AT�շ�����*/
    int iRetryTimes = 0;
    struct stAtRcvMsg AtRcvMsg;

    memset(pstAtRcvMsg, 0, sizeof(*pstAtRcvMsg));
    memset(&AtRcvMsg, 0, sizeof(AtRcvMsg));
  
    /* ��������Ϣʧ�ܣ�����10�� */
    while (-1 == msgrcv(lMsgId,(void*)pstAtRcvMsg, AT_RCV_MSG_LENGTH, lMsgType, IPC_NOWAIT))
    {
        iRetryTimes++;
        if (15 <= iRetryTimes)
        {
            printf("\n! ! !NO RESPONSE FOR AT TYPE %d ! ! !\n",lMsgType);
            return -1;
        }
        __msleep(1000);
    }
    
    while (-1 != msgrcv(lMsgId,(void*)&AtRcvMsg, AT_RCV_MSG_LENGTH, lMsgType, IPC_NOWAIT))
    {
        memcpy(pstAtRcvMsg,&AtRcvMsg,sizeof(*pstAtRcvMsg));
        __msleep(1000);
    }
    return 0;
    /* END:   Modified by y67514, 2008/6/20 */
}


/*------------------------------------------------------------
  ����ԭ��:static inline int AtSend(int lMsgKey, struct stAtSndMsg *pstAtSndMsg, char cFrom,  int lMsgType,  const char *pszParam)
  ����: ��HSPA�ӿ�ģ�鷢����Ϣ
  ����: lMsgId:��ϢID��pstAtSndMsg :��Ϣ�ṹ��ָ��;cFrom:��Ϣ��Դ;lMsgType:��Ϣ����;pszParam:ATָ�����
  ���: ��
  ����ֵ:
-------------------------------------------------------------*/

int AtSend(int lMsgId, struct stAtSndMsg *pstAtSndMsg, char cFrom,  int lMsgType,  const char *pszParam)
{
	memset(pstAtSndMsg, 0, sizeof(*pstAtSndMsg));	
	pstAtSndMsg->lMsgType = lMsgType;
	pstAtSndMsg->acParam[0] = cFrom;
	//strcpy(pstAtSndMsg->acText+1, pszParam);
	if(NULL!= pszParam)
		strcpy(pstAtSndMsg->acParam+1, pszParam);
       //������Ϣʱʹ�÷�����ʽ��
	return msgsnd(lMsgId, (void*)pstAtSndMsg, AT_SND_MSG_LENGTH, IPC_NOWAIT);
}
#endif

/*------------------------------------------------------------
  ����ԭ��: static int at_report_handler_rssi (struct stAtRcvMsg *pstAtRcvMsg)
  ����: �����������ص�rssi����
  ����: ���յ�����Ϣ�ṹ
  ���: ��
  ����ֵ:PARSE_OK �����ɹ���PARSE_ERROR����ʧ��
-------------------------------------------------------------*/

static int at_report_handler_rssi (struct stAtRcvMsg *pstAtRcvMsg)
{
	int i = 0;
	if(RSSI_QUERY != pstAtRcvMsg->lMsgType)
	{
		return PARSE_ERROR;
	}


	rssi_get.ucCmdFrom = (unsigned char)pstAtRcvMsg->acText[i++];
	
	rssi_get.ucResult=  (unsigned char)pstAtRcvMsg->acText[i++];
	if(rssi_get.ucResult)
	{
		return PARSE_ERROR;
	}
	rssi_get.ucRSSI=  (unsigned char)pstAtRcvMsg->acText[i++];
	
	return PARSE_OK;

}

/*------------------------------------------------------------
  ����ԭ��: static int at_report_handler_sysinfo (struct stAtRcvMsg *pstAtRcvMsg)
  ����: �����������ص�sysinfo����
  ����: ���յ�����Ϣ�ṹ
  ���: ��
  ����ֵ:PARSE_OK �����ɹ���PARSE_ERROR����ʧ��
-------------------------------------------------------------*/

static int at_report_handler_sysinfo (struct stAtRcvMsg *pstAtRcvMsg)
{
	int i = 0;
	if(SYSTEM_INFO_QUERY != pstAtRcvMsg->lMsgType)
	{
		return PARSE_ERROR;
	}

	sys_info_get.ucCmdFrom = (unsigned char)pstAtRcvMsg->acText[i++];
	sys_info_get.ucResult= (unsigned char)pstAtRcvMsg->acText[i++];
	if(sys_info_get.ucResult)
	{
		return PARSE_ERROR;
	}
	sys_info_get.ucSrvStatus = (unsigned char)pstAtRcvMsg->acText[i++];
	sys_info_get.ucSrvDomain = (unsigned char)pstAtRcvMsg->acText[i++];
	sys_info_get.ucRoamStatus = (unsigned char)pstAtRcvMsg->acText[i++];
	sys_info_get.ucSysMode = (unsigned char)pstAtRcvMsg->acText[i++];
	sys_info_get.ucSimState = (unsigned char)pstAtRcvMsg->acText[i++];
	sys_info_get.ucLockState = (unsigned char)pstAtRcvMsg->acText[i++];
	sys_info_get.ucSysSubMode = (unsigned char)pstAtRcvMsg->acText[i++];
	VDF_DBG("VDF:%s:%s:%d:sysmode��%d***\n",__FILE__,__FUNCTION__,__LINE__,sys_info_get.ucSysMode);
	return PARSE_OK;

}

/*------------------------------------------------------------
  ����ԭ��: static int at_report_handler_cfg_set (struct stAtRcvMsg *pstAtRcvMsg)
  ����: �����������ص�cfg_set����
  ����: ���յ�����Ϣ�ṹ
  ���: ��
  ����ֵ:PARSE_OK �����ɹ���PARSE_ERROR����ʧ��
-------------------------------------------------------------*/

static int at_report_handler_cfg_set (struct stAtRcvMsg *pstAtRcvMsg)
{
	int i = 0;
	if(SYSCFG_SET != pstAtRcvMsg->lMsgType)
	{
		return PARSE_ERROR;
	}

	cfg_set.ucCmdFrom = (unsigned char)pstAtRcvMsg->acText[i++];
	cfg_set.ucResult= (unsigned char)pstAtRcvMsg->acText[i++];
	if(cfg_set.ucResult)
	{
		return PARSE_ERROR;
	}
	
	return PARSE_OK;

}

/*------------------------------------------------------------
  ����ԭ��: static int at_report_handler_pdp_set (struct stAtRcvMsg *pstAtRcvMsg)
  ����: �����������ص�pdp_set����
  ����: ���յ�����Ϣ�ṹ
  ���: ��
  ����ֵ:PARSE_OK �����ɹ���PARSE_ERROR����ʧ��
-------------------------------------------------------------*/

static int at_report_handler_pdp_set (struct stAtRcvMsg *pstAtRcvMsg)
{
	int i = 0;
	if(PDP_SET != pstAtRcvMsg->lMsgType)
	{
		return PARSE_ERROR;
	}

	pdp_set.ucCmdFrom = (unsigned char)pstAtRcvMsg->acText[i++];
	pdp_set.ucResult= (unsigned char)pstAtRcvMsg->acText[i++];
	if(pdp_set.ucResult)
	{
		return PARSE_ERROR;
	}
	
	return PARSE_OK;

}

/*------------------------------------------------------------
  ����ԭ��: static int at_report_handler_service_sts (struct stAtRcvMsg *pstAtRcvMsg)
  ����: �����������ص�service_sts����
  ����: ���յ�����Ϣ�ṹ
  ���: ��
  ����ֵ:PARSE_OK �����ɹ���PARSE_ERROR����ʧ��
-------------------------------------------------------------*/

static int at_report_handler_service_sts (struct stAtRcvMsg *pstAtRcvMsg)
{
	int i = 0;
	if(AUTO_SERVICE_STATUS_REPORT != pstAtRcvMsg->lMsgType)
	{
		return PARSE_ERROR;
	}


	rp_service_sts.ucCmdFrom = (unsigned char)pstAtRcvMsg->acText[i++];
	
	rp_service_sts.ucResult=  (unsigned char)pstAtRcvMsg->acText[i++];
	if(rp_service_sts.ucResult)
	{
		return PARSE_ERROR;
	}
	rp_service_sts.ucSrvStatus=  (unsigned char)pstAtRcvMsg->acText[i++];
	
	return PARSE_OK;

}

/*------------------------------------------------------------
  ����ԭ��: static int at_report_handler_sys_mode (struct stAtRcvMsg *pstAtRcvMsg)
  ����: �����������ص�rssi����
  ����: ���յ�����Ϣ�ṹ
  ���: ��
  ����ֵ:PARSE_OK �����ɹ���PARSE_ERROR����ʧ��
-------------------------------------------------------------*/

static int at_report_handler_sys_mode (struct stAtRcvMsg *pstAtRcvMsg)
{
	int i = 0;
	if(AUTO_SYSTEM_MODE_STATUS_REPORT != pstAtRcvMsg->lMsgType)
	{
		return PARSE_ERROR;
	}

	rp_sys_mode.ucCmdFrom = (unsigned char)pstAtRcvMsg->acText[i++];
	
	rp_sys_mode.ucResult=  (unsigned char)pstAtRcvMsg->acText[i++];
	if(rp_sys_mode.ucResult)
	{
		return PARSE_ERROR;
	}
	rp_sys_mode.ucSysMode=  (unsigned char)pstAtRcvMsg->acText[i++];
	rp_sys_mode.ucSubSysMode=  (unsigned char)pstAtRcvMsg->acText[i++];
	
	return PARSE_OK;

}

/*------------------------------------------------------------
  ����ԭ�� : int at_sm_error_handler(char* report_buf, int len, int err_num)
  ����     : ��CM500���ϱ�������Ϣ�Ĵ���
  ����     : report_buf��lenͬreport_handler_0, err_numΪ�������ͺ�
  ���     : ��
  ����ֵ   : 0 �ɹ�
-------------------------------------------------------------*/
void at_sm_error_handler()
{
    
    g_i_is_report_ok = AT_REPORT_ERR;
    __wakeup(&g_st_submit_at_wait);
}


// A064D00428 EC506 ADD (by l45517 2005��11��21?) BEGIN
void at_sm_modem_busy_handler()
{
    
    g_i_is_report_ok = AT_REPORT_BUSY;
    __wakeup(&g_st_submit_at_wait);
}
// A064D00428 EC506 ADD (by l45517 2005��11��21?) END


/*------------------------------------------------------------
  ����ԭ�� : int at_sm_is_active()
  ����     : �Ƿ����߳����·�AT����
  ����     : ��
  ���     : ��
  ����ֵ   : 0(��ʾAT״̬������); 1(���߳����·�AT����)
-------------------------------------------------------------*/
int at_sm_is_active()
{
    return g_i_is_sm_active;
}

/*------------------------------------------------------------
  ����ԭ�� : int at_sm_submit_at(int at_num, void* res, int* is_report_err,char* parameters)
  ����     : ͨ��pcui���·�һ��AT����
  ����     : at_num AT�����;parameters AT������Ҫ�õ��Ĳ���
  ���     : res : ����AT�ϱ��Ľ��; is_report_err: ����cm500���Ƿ��ϱ���error��Ϣ���ϱ���ʽ����
  ����ֵ   : 1 : ��ʾ��ʱ;��0 : AT�����·��ɹ�
-------------------------------------------------------------*/
int at_sm_submit_at(int at_num, int* is_report_err,char* param)
{
	struct stAtSndMsg AtSndMsg;
	struct stAtRcvMsg AtRcvMsg;
	char atbuf[AT_SND_MSG_LENGTH];
	int ret = 1;
	int iRetryTimes = 0;
	//char buf[64];//���AT����

	g_i_is_sm_active = 1;
	g_i_is_report_ok = AT_REPORT_OK;
	
	memset(atbuf,0,AT_SND_MSG_LENGTH);
	if (g_at_handles[at_num].create_at_cmd == NULL)
	{
		/*AT���ɺ���Ϊ�գ�����AT����Ϊһ����ʽ*/
		strcpy(atbuf, g_at_handles[at_num].at_cmd_str); //, strlen(g_at_handles[at_num].at_cmd_str));   
	}
	else
	{ 
		/*AT���ɺ�����Ϊ�գ�����AT�����ʽ���⣬��Ҫ���⺯������*/
		g_at_handles[at_num].create_at_cmd(atbuf, param);
	}
	
	AtSndMsgId = msgget(MSG_AT_QUEUE, 0666);
	while(-1 == AtSndMsgId)
	{
		VDF_DBG("VDF:%s:%s:%d:can't get the AtSndMsgId***\n",__FILE__,__FUNCTION__,__LINE__);
		__msleep(5*1000);
		AtSndMsgId = msgget(MSG_AT_QUEUE, 0666);
	}

	while(-1 ==AtSend(AtSndMsgId,&AtSndMsg,MODEM_MODULE,g_at_handles[at_num].at_cmd_id,atbuf))
	{
		VDF_DBG("VDF:%s:%s:%d:AtSend ERRO:msgid=%d,atnum=%d,param=%s***\n",__FILE__,__FUNCTION__,__LINE__,AtSndMsgId,g_at_handles[at_num].at_cmd_id,atbuf);
		//g_i_is_sm_active = 0;
		//return AT_TIMEOUT;
		__msleep(5*1000);
	}

	//g_i_cur_at_num = at_num;
	//ret = __sleep_wait(&g_st_submit_at_wait, 7 * 1000);
	if (g_main_thread_killed) 
	{
		ret = 0;
	}

	AtRcvMsgId = msgget(MSG_MODEM_QUEUE, 0666);
	while(-1 == AtRcvMsgId)
	{
		VDF_DBG("VDF:%s:%s:%d:Can't get the AtRcvMsgId***\n",__FILE__,__FUNCTION__,__LINE__);
		__msleep(5*1000);
		AtRcvMsgId = msgget(MSG_MODEM_QUEUE, 0666);
	}
	VDF_DBG("VDF:%s:%s:%d:AtRcvMsgId=%d***\n",__FILE__,__FUNCTION__,__LINE__,AtRcvMsgId);
        __msleep(200);
        /* BEGIN: Modified by y67514, 2008/6/20   ���ⵥ��:�Ż�AT�շ�����*/
        if(-1 == AtRcv(AtRcvMsgId,&AtRcvMsg,g_at_handles[at_num].at_cmd_id))
        {
            g_i_is_sm_active = 0;
            return AT_TIMEOUT;
        }   
        /* END:   Modified by y67514, 2008/6/20 */

	if(PARSE_ERROR == g_at_handles[at_num].at_cmd_report_handler(&AtRcvMsg))
	{
		*is_report_err = 1;
	}
	else
	{
		*is_report_err = 0;
	}
	

	// ���ϱ������߳�ͬ�������⵱ǰ�̷߳��غ��ϱ��߳�ʹ��g_at_result����ɶ�ջ����
	//__p(&g_st_cur_at_num_mutex);
	//g_i_cur_at_num = -1;    
	//g_at_result  = NULL;
	//__v(&g_st_cur_at_num_mutex);

	g_i_is_sm_active = 0;
	return (ret > 0) ? AT_OK : AT_TIMEOUT;
}


/*------------------------------------------------------------
  ����ԭ�� : int at_sm_modem_submit_at(char* at_dial_str)
  ����     : ͨ��modem�ڲ���
  ����     : at_dial_strΪ�·��Ĳ����ַ���, modem_fd modem���ļ����
  ���     : ��
  ����ֵ   : 1 : ��ʾʧ��; 0 : ��ʾ�ɹ�
-------------------------------------------------------------*/
int at_sm_modem_submit_at(int modem_fd, char* at_dial_str)
{
    int    ii = LOOP_TIMES;
    int    i_len;
    int    ret;
	    // A064D00411 EC506 ADD (by l45517 2005��11��18?) BEGIN
    char   ch[512], *pch = NULL;    // �����������ݵĻ���������СΪ512�ֽ�
    const char *report_msg;  // �ϱ���ƥ����Ϣ
    const char *report_no_carrier = NULL;
    fd_set read_set;
    struct timeval tvSelect;
	memset(ch,0,512);
	//printf("***YP:%s:%s:%d:pid=%d,at_dial_str=%s***\n",__FILE__,__FUNCTION__,__LINE__,getpid(),at_dial_str);
    
    if (strcmp(at_dial_str, "ATH\r"))
    {
       /*star 2009-04-06 w45260:���ݿ����ø�ͨ�»��ߺ�,��ATX��Ĭ��ֵ��0�޸�Ϊ1�����������ϱ�������չ�ֶ�
       ��������Ϣ,����:CONNECT 7200000 ).��������»���ֲ��ܽ������ݲ��ŵ�����.
        ԭ��:���ض����ݿ�����Ӧ��Ϣƥ��̫�ϸ�,�������������жϲ���.
        �������:���طſ�ƥ���������ݴ���չ�ֶε����*/
        //report_msg = "\r\nCONNECT\r\n";
        report_msg = "CONNECT";
		/*end 2009-04-06*/
		
        report_no_carrier = "NO CARRIER";
    }
    else
    {
        report_msg = "\r\nOK\r\n";
    }
    // A064D00411 EC506 ADD (by l45517 2005��11��18?) END

    i_len = strlen(at_dial_str);
    while (ii)
    {
        ret = write(modem_fd, at_dial_str, i_len);
        if (-1 == ret)
        {
            if (EINTR == errno)
            {
                continue;
            }
            ERROR("modem : write AT dial command(%s) error", at_dial_str);
            VDF_DBG("AT:%s:%s:%d:write AT erro***\n",__FILE__,__FUNCTION__,__LINE__);
            return AT_TIMEOUT;
        }
        ii--;
        if (ret != i_len)
        {
            if (0 == ii) // ����·�5�ζ�ʧ��
            {
                ERROR("modem : submit AT dial command(%s) failed\n", at_dial_str);
                VDF_DBG("AT:%s:%s:%d:submit AT dial  failed***\n",__FILE__,__FUNCTION__,__LINE__);
                return AT_TIMEOUT;
            }
            WARN("modem : have not submit whole at command for %d times", ii);
            continue;
        }
        break;
    }
    AT_SM_DEBUG("modem dial : write at cmd successful");

    // A064D00300 EC506 MOD (by l45517 2005��11��3�� ) BEGIN
    tvSelect.tv_sec  = 20; // ��ȴ�20��
    // A064D00300 EC506 MOD (by l45517 2005��11��3�� ) END

    tvSelect.tv_usec = 0;

    pch = &ch[0];

    // A064D00411 EC506 ADD (by l45517 2005��11��18?) BEGIN
    while (1)
    {
        if (hungup)
        {
            VDF_DBG("AT:%s:%s:%d:hungup***\n",__FILE__,__FUNCTION__,__LINE__);
            return AT_TIMEOUT;
        }
        
        while (1)
        {
            FD_ZERO(&read_set);
            FD_SET(modem_fd, &read_set);

            ret = select(modem_fd + 1, &read_set, NULL, NULL, &tvSelect);
            if (-1 == ret)
            {
                if (EINTR == errno)
                {
                    // A64D01038 ADD (by l45517 2006��4��11�� ) BEGIN
                    if (!persist)
                    {
                        break;
                    }
                    // A64D01038 ADD (by l45517 2006��4��11�� ) END
                    continue;
                }
                ERROR("modem : select modem error");
                VDF_DBG("AT:%s:%s:%d:select modem error***\n",__FILE__,__FUNCTION__,__LINE__);
                return AT_TIMEOUT;
            }
            if (!ret)
            {
                //A64D01427 yangjianping begin
                WARN("modem : wait for report msg timeout \n");
                //A64D01427 yangjianping end
                VDF_DBG("AT:%s:%s:%d:wait for report msg timeout***\n",__FILE__,__FUNCTION__,__LINE__);
                return AT_TIMEOUT;
            }
            break;
        }
        // A064D00503 EC506 ADD (by L45517 2005��12��2�� ) BEGIN
        INFO("select time is : sec = %d, ms = %d, us = %d , hungup <%d>", 
            tvSelect.tv_sec, (tvSelect.tv_usec / 1000), (tvSelect.tv_usec % 1000), hungup);
        // A064D00503 EC506 ADD (by L45517 2005��12��2�� ) END

        while (1)
        {
            ret = read(modem_fd, pch, 64); // ÿ��������64�ֽ�
            if (-1 == ret)
            {
                if (EINTR == errno)
                {
                    // A64D01038 ADD (by l45517 2006��4��11�� ) BEGIN
                    if (!persist)
                    {
                        break;
                    }
                    // A64D01038 ADD (by l45517 2006��4��11�� ) END
                    continue;
                }
                ERROR("read modem_fd error\n");
                VDF_DBG("AT:%s:%s:%d:read modem_fd error***\n",__FILE__,__FUNCTION__,__LINE__);
                return AT_TIMEOUT;
            }
            /* BEGIN: Added by y67514, 2008/11/29   PN:AU8D01263:HSPA����PIN��δ��������£�HSAP���Ų��ɹ���*/
            if ( 0 == ret )
            {
                /*��������Ϊ�ջ�����ļ�β*/
                VDF_DBG("AT:%s:%s:%d:modem_fd read nothing!!!\n",__FILE__,__FUNCTION__,__LINE__);
                return AT_TIMEOUT;
            }
            /* END:   Added by y67514, 2008/11/29 */
            break;
        }
        // A64D01038 ADD (by l45517 2006��4��11�� ) BEGIN
        if (!persist)
        {
            VDF_DBG("AT:%s:%s:%d:***\n",__FILE__,__FUNCTION__,__LINE__);
            return AT_TIMEOUT;
        }
        // A64D01038 ADD (by l45517 2006��4��11�� ) END

        pch[ret] = '\0';
        pch += ret;

	//printf("***YP:%s:%s:%d:MODEM REPORT=%s***\n",__FILE__,__FUNCTION__,__LINE__,ch);
        if (strstr(ch, report_msg)) 
        {
            return AT_OK;
        }
        
       // printf("(sxg)%s %d, %s %s %s\n", __FILE__, __LINE__, ch, report_no_carrier, pch);
        
        if (report_no_carrier)
        {
            if (strstr(ch, report_no_carrier))
            {
                VDF_DBG("AT:%s:%s:%d:***\n",__FILE__,__FUNCTION__,__LINE__);
                return AT_TIMEOUT;
            }
        }

        if (pch > &ch[512-64]) 
        {
            VDF_DBG("AT:%s:%s:%d:***\n",__FILE__,__FUNCTION__,__LINE__);
            return AT_TIMEOUT;
        }
    }
    // A064D00411 EC506 ADD (by l45517 2005��11��18?) END

}


