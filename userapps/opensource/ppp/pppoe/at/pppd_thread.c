/***********************************************************************
  ��Ȩ��Ϣ : ��Ȩ����(C) 1988-2005, ��Ϊ�������޹�˾.
  �ļ���   : pppd_thread.c
  ����     : lichangqing 45517
  �汾     : V500R003
  �������� : 2005-8-8
  ������� : 2005-8-10
  �������� : 

  ��Ҫ�����б�: 
      
  �޸���ʷ��¼�б�: 
    <��  ��>    <�޸�ʱ��>  <�汾>  <�޸�����>
    l45517      20050816    0.0.1    ��ʼ���
  ��ע: 
************************************************************************/

#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>

#include "pppd.h"
#include "utils_lib.h"
#include "at_sm.h"
#include "at_thread.h"

extern struct tagATConfig at_config;
/*------------------------------------------------------------
  ����ԭ�� : int pppd_thread_at_dail(int modem_fd)
  ����     : AT���Ž�������
  ����     : ��
  ���     : ��
  ����ֵ   : 1:����ʧ�ܣ�0:���ųɹ�
-------------------------------------------------------------*/
int pppd_thread_at_dail(int modem_fd)
{
    int sys_mode;
    int sys_rssi;
    int tmp;
    char buf[64];//���AT����


    memset(buf, 0, sizeof(buf));
    sprintf(buf, "ATD%s\r", at_config.phone_number ? at_config.phone_number:"*99#" );
    
    sys_mode = at_thread_cm500_sysmode();
    sys_rssi = at_thread_cm500_rssi();

    if (SYSMODE_WCDMA == sys_mode)
    {
		//printf("VDF:%s:%s:%d:SYSMODE_WCDMA == sys_mode***\n",__FILE__,__FUNCTION__,__LINE__);
        //if (at_sm_modem_submit_at(modem_fd, "ATD*99#\r"))
        if (at_sm_modem_submit_at(modem_fd, buf))		/*��Modem���·�ATָ��*/
        {
            __msleep(500);
            WARN("at dial : retrying ...");
            //if (at_sm_modem_submit_at(modem_fd, "ATD*99#\r"))
            if (at_sm_modem_submit_at(modem_fd, buf))
            {
                WARN("at dial : failed");
                return AT_TIMEOUT;
            }
        }
    }
    else if (SYSMODE_NO_SERVICES != sys_mode)
    {
		//printf("VDF:%s:%s:%d:SYSMODE_NO_SERVICES != sys_mode***\n",__FILE__,__FUNCTION__,__LINE__);
        WARN("AT dial %s ... ...", buf);
        if (!sys_rssi)
        {
            WARN("at dial failed rssi<%d>", sys_rssi);
            //return AT_TIMEOUT; //modify by sxg, ignore rssi value
        }
        //if (at_sm_modem_submit_at(modem_fd, "ATD#777\r"))
        if (at_sm_modem_submit_at(modem_fd, buf))
        {
            __msleep(500);
            // A064D00300 EC506 ADD (by l45517 2005��11��3�� ) BEGIN
            write(modem_fd, "\r", 1);
            // A064D00300 EC506 ADD (by l45517 2005��11��3�� ) END
            WARN("at dial failed, writing abort");
            
            // A064D00300 EC506 MOD (by l45517 2005��11��3�� ) BEGIN
            return AT_TIMEOUT;
            // A064D00300 EC506 MOD (by l45517 2005��11��3�� ) END
        }
    }
    else
    {
    		printf("VDF:%s:%s:%d:SYSMODE_NO_SERVICES***\n",__FILE__,__FUNCTION__,__LINE__);
		int at_res = 0;
		if (at_sm_submit_at(AT_SYSINFO_QUERY,  &at_res, NULL))
		{
			printf("VDF:%s:%s:%d:SYSINFO��ʱ***\n",__FILE__,__FUNCTION__,__LINE__);
			ERROR("cm500 timeout");
			return AT_TIMEOUT;
		}
		if (at_res)
		{
			printf("VDF:%s:%s:%d:SYSINFO��������***\n",__FILE__,__FUNCTION__,__LINE__);
			WARN("got sysinfo error msg");
		}
	//printf("VDF:%s:%s:%d:sys_mode == %d***\n",__FILE__,__FUNCTION__,__LINE__,sys_mode);
        WARN("illegal sysmode type : NO SERVICES\n");
        return AT_TIMEOUT;
    }
    PPPD_THREAD_DEBUG("at dial successful");
    return AT_OK;
}


/*------------------------------------------------------------
  ����ԭ�� : int pppd_thread_ath(int modem_fd)
  ����     : AT���Ź�������
  ����     : ��
  ���     : ��
  ����ֵ   : 1:����ʧ�ܣ�0:���ųɹ�
-------------------------------------------------------------*/
int pppd_thread_ath(int modem_fd)
{
    int sys_mode;
    int sys_rssi;
    int tmp;
    if (at_sm_modem_submit_at(modem_fd, "ATH\r")) 
    {
        // A064D00428 EC506 MOD (by l45517 2005��11��21?) BEGIN
        /*
        if (at_sm_modem_submit_at(modem_fd, "ATH\r")) 
        {
            return AT_TIMEOUT;
        }
        */
        return AT_TIMEOUT;
        // A064D00428 EC506 MOD (by l45517 2005��11��21?) END
    }
    return AT_OK;
}


/*------------------------------------------------------------
  ����ԭ�� : static int pppd_thread_entry(void* args)
  ����     : pppd�߳��������
  ����     : args������
  ���     : ��
  ����ֵ   : 0 �ɹ�
-------------------------------------------------------------*/
static int     g_thread_args = 0;
static char**  g_thread_argv = NULL;

static int pppd_thread_entry(void* args)
{
 
    pppd_main(g_thread_args, g_thread_argv);
    return SUCCESS;
}


/*------------------------------------------------------------
  ����ԭ�� : int pppd_thread_initialize(int args, char** argv)
  ����     : pppd�̳߳�ʼ��
  ����     : args��argvΪ�����в���
  ���     : ��
  ����ֵ   : 0 �ɹ�
-------------------------------------------------------------*/
THREAD_S g_ppp_thread;

//A064D00348 qinzhiyuan begin
//int pppd_thread_initialize(int args,  char** argv)
int pppd_thread_initialize(int args, const char** argv)
//A064D00348 qinzhiyuan end
{

    g_thread_args = args;
    g_thread_argv = argv;
	printf("***YP:%s:%s:%d:pppd thread initialized***\n",__FILE__,__FUNCTION__,__LINE__);
    /// pppd_main(g_thread_args, g_thread_argv);
    
    if (__init_thread_t(&g_ppp_thread, pppd_thread_entry, NULL))
    {
        return FAILED;
    }
    
    PPPD_DEBUG("pppd thread initialized");
    if (__start_thread(&g_ppp_thread) < 0)
    {
        ERROR("pppd thread start failed");
        return FAILED;
    }
    
    PPPD_DEBUG("pppd thread started thread_id = %d", g_ppp_thread.thread_id);
    return SUCCESS;
}


/*------------------------------------------------------------
  ����ԭ�� : int pppd_thread_destroy()
  ����     : pppd�߳�ע��
  ����     : ��
  ���     : ��
  ����ֵ   : 0 �ɹ�
-------------------------------------------------------------*/
int pppd_thread_destroy()
{
    int i_pid;
    int tmp;
    int status;

    // A64D01038 ADD (by l45517 2006��4��10�� ) BEGIN
    // ���ppp ���������Ƴ���
    INFO("kill : pppd thread not exist");
    if (g_ppp_thread.thread_id > 0)
    {
        tmp = kill(g_ppp_thread.thread_id, SIGTERM);
        if (tmp)
        {
            if (ESRCH == errno)
            {
                INFO("kill : pppd thread not exist");
            }
            else
            {
                ERROR("kill : pppd error");
            }
        }
        else
        {
            INFO("pppd thread killed, pid = %d", g_ppp_thread.thread_id);
            while(1)
            {
                i_pid = waitpid(g_ppp_thread.thread_id, &status, __WALL);
                if (i_pid > 0)
                {
                    INFO("waitpid : thread %d exited\n", i_pid);
                }
                else if (i_pid == 0)
                {
                    INFO("waitpid : no child was available\n");
                }
                else
                {
                    if (EINTR == errno)
                    {
                        continue;
                    }
                    ERROR("waitpid : ");
                }
                break;
            }
        }
    }
    // A64D01038 ADD (by l45517 2006��4��10�� ) END

    __destroy_thread_t(&g_ppp_thread);
    return 0;
}
