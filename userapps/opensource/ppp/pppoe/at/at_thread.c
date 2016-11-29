/***********************************************************************
  ��Ȩ��Ϣ : ��Ȩ����(C) 1988-2005, ��Ϊ�������޹�˾.
  �ļ���   : at_thread.c
  ����     : lichangqing 45517
  �汾     : V500R003
  �������� : 2005-8-8
  ������� : 2005-8-10
  �������� : ʵ��AT�����߳�
      
  ��Ҫ�����б�: 
      
  �޸���ʷ��¼�б�: 
    <��  ��>    <�޸�ʱ��>  <�汾>  <�޸�����>
    l45517      20050816    0.0.1    ��ʼ���
  ��ע: 
************************************************************************/

#include <stdio.h>
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <sys/types.h> 
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// A064D00358 EC506 ADD (by l45517 2005��11��2�� ) BEGIN
#include <setjmp.h>
#include <linux/termios.h>
// A064D00358 EC506 ADD (by l45517 2005��11��2�� ) END

#include "pppd.h"
#include "at_sm.h"
#include "utils_lib.h"
#include "pppd_thread.h"
#include "at_thread.h"
#include "files.h"    //add by sxg

#define HSPA_SHARE_FILE_PATH "/var/HspaStatus"
#define	HSPA_IMEI_LENGTH	16
#define	HSPA_PRODUCT_NAME_LENGTH	60

struct stDataFlow
{
	unsigned long int ulCurrDsTime;
	unsigned long int ulTxRate;
	unsigned long int ulRxRate;
	unsigned long int ulQosTxRate;
	unsigned long int ulQosRxRate;	
};
struct stHspaInfo
{
	int lPid;
	short int sDeviceCount;
	short int sDeviceStatus;
	short int sSimExist;
	short int sSimStatus;
	short int sSysMode;
	short int sSubSysMode;
	short int sSrvStatus;
	short int sPinPukNeed;
	char acIMEI[HSPA_IMEI_LENGTH]; 
      	char acIMSI[HSPA_IMEI_LENGTH];
	char acProductName[HSPA_PRODUCT_NAME_LENGTH];
      char acPinCode[HSPA_IMEI_LENGTH];
	char acOldIMEI[HSPA_IMEI_LENGTH];
	short int Hspadetectfinish;
	struct stDataFlow stDataFlowInfo;
 };

int               g_log_fd = -1;
//static RSSI_S     g_cm500_rssi;                          // cm500 RSSI
//static RSSI_S     g_cm500_hdrrssi;                       // cm500 evdo RSSI
//static SYS_INFO_S g_cm500_sysinfo = {0, 0, 0, 0, 0};     // cm500ϵͳ��Ϣ
//static char       g_cm500_sw_version[EVDO_VERSION_LEN];  // ����汾��,���Ȳ�����31���ַ��� ������\0��
//static char       g_cm500_hw_version[EVDO_VERSION_LEN];  // Ӳ���汾�š�
//static char       g_cm500_esn[ESN_LEN];//A064D00348 qinzhiyuan            

extern struct stMsg_RSSI_Query			rssi_get ;
extern struct stMsg_System_Info_Query	sys_info_get;
extern struct stMsg_Syscfg_Set			cfg_set;
extern struct stMsg_PDP_Set				pdp_set;
extern struct stMsg_Service_Status		rp_service_sts;
extern struct stMsg_System_Mode			rp_sys_mode;


int               g_main_thread_killed = 0;              // �����źŴ�����������ã�
int               g_shake_hand_failed  = 0;              // ����ʧ�ܱ�־
int               g_child_exited       = 0;
AT_TIMER_S        g_shankehand_timer;                    // cm500���ֵĶ�ʱ��
//MUTEX_S           g_rcv_start_mutex;
//MUTEX_S           g_pppd_start_mutex;

extern char* nvram_get(const char *name);

//A064D00348 qinzhiyuan begin
extern char* nvram_set(const char* name,const char* value);
extern char* nvram_commit(void);
//A064D00348 qinzhiyuan end

#define MODEM_TIMEOUT 50
static int g_no_upstats_times = 0;
static int g_no_downstats_times = 0;
static int g_has_set_network = 0;

int g_modem_stub = 0;
int g_modem_restore_ldisc_stub = 0;
int g_stub = 0; // A064D00429 EC506 ADD (by L45517 2005��11��25?) 

/*����ATģ������ò��� add by sxg*/
AT_CONFIG_S at_config = {NULL, NULL, NULL, 0, 0, 0};

/*------------------------------------------------------------
  ����ԭ��: int at_thread_cm500_sysmode()
  ����    : ��ȡcm500ϵͳģʽ
  ����    : ��
  ���    : ��
  ����ֵ  : ����ϵͳģʽ
-------------------------------------------------------------*/
int at_thread_cm500_sysmode()
{
    return sys_info_get.ucSysMode;
}


/*------------------------------------------------------------
  ����ԭ��: int at_thread_cm500_rssi()
  ����    : ��ȡcm500ϵͳģʽ
  ����    : ��
  ���    : ��
  ����ֵ  : ����ϵͳģʽ
-------------------------------------------------------------*/
int at_thread_cm500_rssi()
{
    return rssi_get.ucRSSI;
}


/*------------------------------------------------------------
  ����ԭ��: void at_thread_reset_sysmode()
  ����    : ��ȡcm500ϵͳģʽ
  ����    : ��
  ���    : ��
  ����ֵ  : ��
-------------------------------------------------------------*/
void at_thread_reset_sysmode()
{
    sys_info_get.ucSysMode= 0;
}

/*------------------------------------------------------------
  ����ԭ�� : static void toggle_wan_led()
  ����     : wan �ڵ�ƺ���
  ����     : ��
  ���     : ��
  ����ֵ   : ��
-------------------------------------------------------------*/
static void toggle_wan_led()
{
	static int i_is_wan_ledon = 0;
	// A064D00429 EC506 MOD (by L45517 2005��12��1�� ) BEGIN
	static struct ppp_his_stat history_stats = {0, 0};
    struct ppp_his_stat cur_stats = {0, 0};
	// A064D00429 EC506 MOD (by L45517 2005��12��1�� ) END

	int language = 0;//CUSTOMID;  /* A64D01304: chenjindong add 2006-7-11 */
	
	if (PHASE_RUNNING != phase)
	{
		//wan_ledoff();
		return;
	}
    
    // A064D00358 EC506 ADD (by l45517 2005��11��2�� ) BEGIN
    // ��Ӽ��dcd�ź��ߵĹ��ܡ�
    if (real_ttyfd != -1)
    {
        int modembits;
        
        if (ioctl(real_ttyfd, TIOCMGET, &modembits))
        {
            ERROR("get DCD error");
        }
        else
        {
            if (!(modembits & TIOCM_CD))
            {
                kill(g_ppp_thread.thread_id, SIGHUP);
                return;
            }
            /* A64D01304: chenjindong begin 2006-7-11 */
            /* --------------------------------------- 
                ��ͨ�汾���DSR ������״̬    
                OFF:�Ͽ�ppp ����  
                --------------------------------------- */
            else if(!(modembits & TIOCM_DSR))
            {                
                if(1 == language)
                {
                    kill(g_ppp_thread.thread_id, SIGHUP);
                    return;
                }
            }
            /* A64D01304: chenjindong end 2006-7-11 */        
            //INFO("modembits <%x>", modembits);
        }
    }
    
	if (!get_cur_link_stats(&cur_stats))
    {
		//wan_ledoff();
		return;
    }
    // A064D00358 EC506 ADD (by l45517 2005��11��2�� ) END


  	if (history_stats.bytes_out == cur_stats.bytes_out)
  	{
      	g_no_upstats_times++;
  	}
    else
    {
        history_stats.bytes_out = cur_stats.bytes_out;
        
        if (g_no_upstats_times > 20)
        {
            g_no_downstats_times = 0;
        }
            
        g_no_upstats_times = 0;
    }

	if (history_stats.bytes_in == cur_stats.bytes_in)
	{
    	g_no_downstats_times++;
        
        if( g_no_downstats_times > 10 )
        {
		   // wan_ledon();
            return;
        }
	}
    else
    {
        if (g_no_downstats_times > 20)
        {
            g_no_upstats_times = 0;
        }
        g_no_downstats_times = 0;
    }
    
	history_stats.bytes_in = cur_stats.bytes_in;
	if (i_is_wan_ledon)
	{
		i_is_wan_ledon = 0;
		//wan_ledoff();
		return;
	}
	else
	{
    	i_is_wan_ledon = 1;
    	//wan_ledon();
    	return;
	}
}


/*------------------------------------------------------------
  ����ԭ�� : void at_thread_shakehand_callback(int arg)
  ����     : ��cm500��ʱ���ֻص�����
  ����     : arg������
  ���     : ��
  ����ֵ   : ��
-------------------------------------------------------------*/
static int g_shakehand_times = 0; // ��ʱ�������Ĵ���
void at_thread_shakehand_callback(int arg)
{
    int at_res;
	int   errno_tmp = errno;
    int fd_tmp_pcui;
    //char* pch = "ERROR";//A064D00348 qinzhiyuan begin
    char* tmp;
    char buf[AT_CMD_PARAM_LEN];
	struct timeval cur_time;
    struct ppp_his_stat cur_stats;

    toggle_wan_led();

    // A64D01229 ADD (by l45517 2006��5��30�� ) BEGIN
    {
        
        static int block_phase_authenticate = 0;
        static int block_phase_serialconn = 0;

        /*
         *  �����ⵥ�Ǵ�����Ϊppp״̬���������������⣬��ǰ�Ĳ��Է�����ppp ״̬
         *  ������PHASE_AUTHENTICATE��PHASE_SERIALCONNʱ����ָ�����
         *  ������Ƚ��Ѹ��֣����ö�λ������ͨ�����µķ������: ����״̬������
         *  ǰ�������״̬��ʱ�䳬��5��ʱ��ͨ�������ź�SIGHUP����ppp״̬��������
         *  �л��ѡ�
         *  ��ʱ��ÿ100������һ�Σ�block_phase_authenticate��block_phase_serialconn����100 ����ppp��ĳ
         *  ��״̬ͣ����ʱ�䡣
         */

        if (PHASE_AUTHENTICATE == phase)
        {
            block_phase_authenticate++;
        }
        else
        {
            block_phase_authenticate = 0;
        }
        
        if (PHASE_SERIALCONN == phase)
        {
            block_phase_serialconn++;
        }
        else
        {
            block_phase_serialconn = 0;
        }

        if ((block_phase_authenticate > 1800) || (block_phase_serialconn > 1800))
        {
            kill(g_ppp_thread.thread_id, SIGHUP);
        }
    }

    /* ����5��������5 ��������CM500�����֣�����ִ���κβ�����*/
    if (g_shakehand_times < 50) 
    {
        g_shakehand_times++;
        return;
    }
    g_shakehand_times = 0;

    // A64D01229 ADD (by l45517 2006��5��30�� ) END


    // ���״̬�����ڻ״̬
    if (at_sm_is_active())
    {
        return;
    }
    
    /* HUAWEI HGW qinzhiyuan 2005��10��9��
       ����ϵͳģʽ*/

    if (1) 
    {
        if (at_sm_submit_at(AT_SYSINFO_QUERY, &at_res, NULL))
        {
            INFO("cm500 shakehand timeout");
            g_shake_hand_failed = AT_TIMEOUT;
            __stop_timer(&g_shankehand_timer);
            errno = errno_tmp;
            return;
        }
        if (at_res)
        {
            WARN("got sysinfo error msg");
        }
    }

    // ��ѯRSSI
    if (at_sm_submit_at(AT_RSSI_QUERY,  &at_res, NULL))
    {
        _DEBUG("cm500 shakehand timeout");
        g_shake_hand_failed = AT_TIMEOUT;
        __stop_timer(&g_shankehand_timer);
        errno = errno_tmp;
        return;
    }
    if (at_res)
    {
        WARN("got rssi error msg");
        // A064D00428 EC506 DEL (by l45517 2005��11��21?) BEGIN
        //  g_cm500_rssi = 0;
        // A064D00428 EC506 DEL (by l45517 2005��11��21?) END
    }

    INFO("hdrrssi, rssi<%d>", rssi_get.ucRSSI); 

#if 1
    // A064D00429 EC506 ADD (by L45517 2005��12��2�� ) BEGIN
    get_cur_link_time(&cur_time);
    get_cur_link_stats(&cur_stats);

    _DEBUG("timer shakehand rssi <%d>, sysmode <%d>, stubs <%d, %d, %d>\n" 

        "    srv_status      = %d\n"
        "    srv_domain      = %d\n"
        "    roam_status     = %d\n"
        "    sys_mode        = %d\n"
        "    sim_state       = %d\n"
        "    ppp link state  = %d\n"
        "    ppp ip addr     = %I\n"
        //"    evdo sw version = %s\n"
        //"    evdo hw version = %s\n"
        //"    ppp link time        cur = %d, history = %d\n"
        //"    ppp link status-up   cur = %d, history = %d\n"
        //"    ppp link status-down cur = %d, history = %d"

        , (int)rssi_get.ucRSSI, sys_info_get.ucSysMode, g_modem_stub, g_modem_restore_ldisc_stub, g_stub  

        , sys_info_get.ucSrvStatus
        , sys_info_get.ucSrvDomain
        , sys_info_get.ucRoamStatus
        , sys_info_get.ucSysMode
        , sys_info_get.ucSimState
        , phase
        , ipcp_get_ppp_addr()
        //, g_cm500_sw_version
        //, g_cm500_hw_version
        //, cur_time.tv_sec,     (cur_time.tv_sec     + g_pppd_history_time.tv_sec)
        //, cur_stats.bytes_out, (cur_stats.bytes_out + g_pppd_history_stats.bytes_out)
        //, cur_stats.bytes_in,  (cur_stats.bytes_in  + g_pppd_history_stats.bytes_in)
        );
	
    // A064D00429 EC506 ADD (by L45517 2005��12��2�� ) END

#else
    _DEBUG("timer shakehand rssi <%d>, sysmode <%d>\n"
        , (int)g_cm500_rssi, g_cm500_sysinfo.sys_mode);
#endif

   // rssi_ledshow((int)g_cm500_rssi);

    // ���ϵͳģʽ����ȷ�����ѯϵͳģʽ
    if (SYSMODE_NO_SERVICES == sys_info_get.ucSysMode)
    {
        if (at_sm_submit_at(AT_SYSINFO_QUERY, &at_res, NULL))
        {
            _DEBUG("cm500 shakehand timeout");
            g_shake_hand_failed = AT_TIMEOUT;
            __stop_timer(&g_shankehand_timer);
            errno = errno_tmp;
            return;
        }
        if (at_res)
        {
            WARN("got sysinfo error msg");
        }
        else
        {
            _DEBUG("timer shakehand sysinfo <srv_status = %d, srv_domain = %d,"
                "roam_status = %d, sys_mode = %d, sim_state = %d>",
                sys_info_get.ucRoamStatus,
                sys_info_get.ucSimState,
                sys_info_get.ucSrvDomain,
                sys_info_get.ucSrvStatus,
                sys_info_get.ucSysMode);
        }
    }
    /* end modify */
    
    errno = errno_tmp;
    return;

}


/*------------------------------------------------------------
  ����ԭ��: int open_log(void)
  ����    : ��ϵͳ��־�ļ�
  ����    : �����в���
  ���    : ��
  ����ֵ  : 0 �ɹ���1 ʧ��
-------------------------------------------------------------*/
int open_log(void)
{
    g_log_fd = open("/var/log/ppp.log", O_CREAT | O_RDWR | O_TRUNC);
    if (g_log_fd < 0)
    {
        return FAILED;
    }
    return SUCCESS;
}


/*------------------------------------------------------------
  ����ԭ��: void close_log(void)
  ����    : ��ϵͳ��־�ļ�
  ����    : �����в���
  ���    : ��
  ����ֵ  : ��
-------------------------------------------------------------*/
void close_log(void)
{
    if (g_log_fd < 0)
    {
        return;
    }
    close(g_log_fd);
}

/*------------------------------------------------------------
  ����ԭ��: int main(int args, const char** argv)
  ����    : AT�����߳�Ҳ�ǽ��̵�������
  ����    : �����в���
  ���    : ��
  ����ֵ  : ��
-------------------------------------------------------------*/
int main(int args, const char** argv)
{
    int i_time; // cm500�����ȴ�ʱ��
    int i_ret;
    int sin_size;
    int request_num;
    int recvbytes;
    //A064D00363 yangjianping begin
    int fd_pcui = -1;
    //A064D00363 yangjianping end
    //system("ttyUSB");
	//__msleep(100000);
    int    sock_fd,     client_fd;   // sock_fd������socket; client_fd�����ݴ���socket
    struct sockaddr_in  my_addr;     // ������ַ��Ϣ
    struct sockaddr_in  remote_addr; // �ͻ��˵�ַ��Ϣ

    // A64D01229 ADD (by l45517 2006��5��30�� ) BEGIN
    int i_cm500_start_time[3] = {15, 30, 45}; // cm500�������ȴ�ʱ��
    // A64D01229 ADD (by l45517 2006��5��30�� ) END

    int i_cm500_start_times   = 0;

    int i_args = 0;
    //g_pppd_history_time.tv_sec     = 0;
    //g_pppd_history_time.tv_usec    = 0;
    //g_pppd_history_stats.bytes_in  = 0;
    //g_pppd_history_stats.bytes_out = 0;

    // A064D00428 EC506 ADD (by l45517 2005��11��21?) BEGIN
    //g_cm500_sw_version[0] = '\0';
    //g_cm500_hw_version[0] = '\0';
    // A064D00428 EC506 ADD (by l45517 2005��11��21?) END
    
    daemon(1, 1);

    sigaction(SIGALRM, NULL, &(g_shankehand_timer.old_sigalrm));

    reopen_log();
    if (open_log())
    {
        printf("[%16s:%4d] open log failed, errno = %d <%s>\n", 
            __FILE__, __LINE__, errno, strerror(errno));
        return 0;
    }

    debug = 0;
//    __init_mutex(&g_rcv_start_mutex, "/usr/sbin/pppd",  3);
//    __init_mutex(&g_pppd_start_mutex, "/usr/sbin/pppd", 4);
	//__p(&g_pppd_start_mutex);
	
    /*start, modify by sxg*/
	for(i_args = 0; i_args < args; i_args++)
	{
		if(!strstr(argv[i_args], "-U"))
		{
			continue;
		}
		/*PPP Over USB--��HSPA����*/
//		__p(&g_pppd_start_mutex);
		/*
	    if (pppd_thread_initialize(args, argv))
	    {
	        ERROR("pppd initialization failed");
	        exit(1);
	    }*/
	    //��ȡ������Ϣ
		if(!read_config(ATCFG_PROFILE))
		{
			ERROR("at read config fail <%s>\n", ATCFG_PROFILE);
		}
		_DEBUG("at config: at_profile=%s, phone_number=%s, ap_name=%s, operator=%d, conn_type=%d, channel=%s\n", 
		                at_config.profile ? at_config.profile:"<NULL>", 
		                at_config.phone_number ? at_config.phone_number:"<NULL>" ,
		                at_config.ap_name ? at_config.ap_name:"<NULL>" ,
		                at_config.operator,
		                at_config.conn_type,
		                at_config.channel);

	    _DEBUG("VDF: start ppp over usb\n");
		return pppd_main(args, argv);
	    //break;
	}

	/*ppp over ADSL--��ADSL����*/
	if(i_args == args)
	{
		_DEBUG("VDF: start ppp over ADSL\n");
		return pppd_main(args, argv);
	}
	/*end, modify by sxg*/
	printf("VDF:%s:%s:%d:erro!!! ppp can't start up***\n",__FILE__,__FUNCTION__,__LINE__);
	return 0;
}

/*------------------------------------------------------------
  ����ԭ��: int at_init()
  ����    : ���ݿ���ʼ״̬�Ĳ�ѯ������
  ����    : ��
  ���    : ��
  ����ֵ  : 0����ʼ���ɹ���1�����ɹ�
-------------------------------------------------------------*/

int at_init()
{
	int   at_res;
	char* pch = "ERROR";
	char* tmp;
	char buf[20];	
	struct stHspaInfo HspaInfo;
	FILE *fp = NULL;
	
	VDF_DBG("VDF:%s:%s:%d:��ʼ��ѯ***\n",__FILE__,__FUNCTION__,__LINE__);

	memset(&HspaInfo,0,sizeof(HspaInfo));
	while (1)
	{
		//ͨ����hspa״̬�ļ��ж����ݿ��Ƿ����
		if ( (fp = fopen(HSPA_SHARE_FILE_PATH, "r")) == NULL )
		{
			printf("Error:HspaStatus read failed.\n");
		}
		else
		{
			fread(&HspaInfo, sizeof(struct stHspaInfo), 1, fp);
			if(0 != HspaInfo.sPinPukNeed)
			{
				//pin������֤
				fclose(fp);
				VDF_DBG("VDF:%s:%s:%d:pin������֤***\n",__FILE__,__FUNCTION__,__LINE__);
				continue;
			}
			if(255 == HspaInfo.sSimExist)
			{
				//sim��������
				VDF_DBG("VDF:%s:%s:%d:sim��������***\n",__FILE__,__FUNCTION__,__LINE__);
				fclose(fp);
				continue;
			}
			//sim��״̬����
			fclose(fp);
			VDF_DBG("VDF:%s:%s:%d:sSysMode=%d ��ʼ״̬ok***\n",__FILE__,__FUNCTION__,__LINE__,HspaInfo.sSysMode);
			break;
        	}
              __msleep(5000);
	}
	
	// ��ȡcm500 ϵͳ��Ϣ
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
		return AT_TIMEOUT;
	}
	else
	{
		VDF_DBG("***VDF:cm500 sysinfo <srv_status = %d, srv_domain = %d,"
			"roam_status = %d, sys_mode = %d, sim_state = %d>***\n",
				sys_info_get.ucSrvStatus,
				sys_info_get.ucSrvDomain,
				sys_info_get.ucRoamStatus,
				sys_info_get.ucSysMode,
				sys_info_get.ucSimState);

	}

	// ���ϵͳģʽ����ȷ�����²�һ��
	if (SYSMODE_NO_SERVICES == sys_info_get.ucSysMode)
	{
		__msleep(5000);
		if (at_sm_submit_at(AT_SYSINFO_QUERY,  &at_res, NULL))
		{
			return AT_TIMEOUT;
		}
		if (at_res)
		{
			WARN("got sysinfo error msg");
			return AT_TIMEOUT;
		}
		else
		{
			VDF_DBG("***VDF:cm500 sysinfo <srv_status = %d, srv_domain = %d,"
				"roam_status = %d, sys_mode = %d, sim_state = %d>***\n",
					sys_info_get.ucSrvStatus,
					sys_info_get.ucSrvDomain,
					sys_info_get.ucRoamStatus,
					sys_info_get.ucSysMode,
					sys_info_get.ucSimState);

		}
	}
	
	// ��ѯcm500 RSSIֵ
	if (at_sm_submit_at(AT_RSSI_QUERY, &at_res, NULL))
	{
		ERROR("cm500 timeout");
		return AT_TIMEOUT;
	}
	if (at_res)
	{
		WARN("got rssi error msg");
	}
	VDF_DBG("VDF:%s:%s:%d:rssi=%d***\n",__FILE__,__FUNCTION__,__LINE__,rssi_get.ucRSSI);
	/*���� APN,�Զ�ģʽap_nam=NULL,�ֶ�ģʽʹ������ֵ*/  
	if(at_config.ap_name)
	{
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "1,\"IP\",\"%s\"", at_config.ap_name);
		if(at_sm_submit_at(AT_PDP_SET, &at_res, buf))
		{
			ERROR("cm500 timeout");
			return AT_TIMEOUT;
		}
		if (at_res)
		{
			//����ʧ��
			WARN("set APN error msg");
			return AT_TIMEOUT;
		}
	}
	
    #if 0
	/*������������: GPRS/3G*/
	if(at_config.conn_type == ATCFG_CONNTYPE_GPRS_FIRST
		|| at_config.conn_type == ATCFG_CONNTYPE_3G_FIRST
		|| at_config.conn_type == ATCFG_CONNTYPE_GPRS_ONLY
		|| at_config.conn_type == ATCFG_CONNTYPE_3G_ONLY
		|| at_config.conn_type == ATCFG_CONNTYPE_AUTO)
	{
		memset(buf, 0, sizeof(buf));
		if(at_config.conn_type == ATCFG_CONNTYPE_GPRS_FIRST)
		{
			sprintf(buf, "2,1");
		}
		else if(at_config.conn_type == ATCFG_CONNTYPE_3G_FIRST)
		{
			sprintf(buf,"2,2");
		}
		else if(at_config.conn_type == ATCFG_CONNTYPE_GPRS_ONLY)
		{
			sprintf(buf,"13,3");
		}
		else if(at_config.conn_type == ATCFG_CONNTYPE_3G_ONLY)
		{
			sprintf(buf,"14,3");			
		}
        else if(at_config.conn_type == ATCFG_CONNTYPE_AUTO)
		{
			sprintf(buf,"2,0");			
		}

		/*����ͨ����������*/
		/*
		if(at_config.channel == ATCFG_CHANNEL_UNLIMITED)
		{
			sprintf(buf, "%s,3FFFFFFF", buf);
		}
		else if(at_config.channel == ATCFG_CHANNEL_GSM900_GSM1800_WCDMA2100)
		{
			sprintf(buf, "%s,00400380", buf);			 
		}
		else if(at_config.channel == ATCFG_CHANNEL_GSM1900)
		{
			sprintf(buf, "%s,00200000", buf);			  
		}
		else
		{
			sprintf(buf, "%s,40000000",buf);
		}
		*/
		
		sprintf(buf, "%s,%s", buf,at_config.channel);
		sprintf(buf, "%s,2,4", buf);
		VDF_DBG("AT is %s.........\n",buf);
		if(at_sm_submit_at(AT_SYSCFG_SET, &at_res, buf))
		{
			ERROR("cm500 timeout");
			return AT_TIMEOUT;
		}
		if (at_res)
		{
			WARN("set Conn Type error msg");
			return AT_TIMEOUT;
		}
	}
    #endif
	VDF_DBG("VDF:%s:%s:%d:�������***\n",__FILE__,__FUNCTION__,__LINE__);
	return AT_OK;
}


