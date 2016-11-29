/***********************************************************************
  ��Ȩ��Ϣ : ��Ȩ����(C) 1988-2005, ��Ϊ�������޹�˾.
  �ļ���   : at_thread.h
  ����     : lichangqing 45517
  �汾     : V500R003
  �������� : 2005-8-8
  ������� : 2005-8-10
  �������� : ͷ�ļ�
      
  ��Ҫ�����б�: ��
      
  �޸���ʷ��¼�б�: 
    <��  ��>    <�޸�ʱ��>  <�汾>  <�޸�����>
    l45517      20050816    0.0.1    ��ʼ���
  ��ע: 
************************************************************************/

#ifndef __AT_THREAD_H
#define __AT_THREAD_H

typedef int RSSI_S;
typedef struct
{
    int srv_status;
    int srv_domain;
    int roam_status;
    int sys_mode;
    int sim_state;
} SYS_INFO_S;

#define USIM_1           1        // ��ȡusim��״̬
#define RSSI_2           2        // ��ȡRSSI
#define EVDO_VERSION_3   3        // ��ȡEVDO�汾
#define PPP_IP_STATE_4   4        // ��ȡPPP����״̬��IP��ַ
#define PPP_LINKTIME_5   5        // ��ȡPPP����ʱ��
#define PPP_STATS_6      6        // ��ȡPPP�ۼ������͵�ǰ����
#define ERROR_11         11       // �ӿ�ȡ��
#define ERROR_12         12       // �ӿ�ȡ��
#define CM500_NETTYPE_13 13       // ��ȡ��������

#define RESPONSE_SUCCESSE 1       // ��Ӧ�û���ȡ��Ϣ�ɹ�
#define RESPONSE_FAILED   0       // ��Ӧ�û���ȡ��Ϣʧ��
#define BASE_SIZE         8       // ���ص�������ret_val��cmd�ĳ���

#define EVDO_VERSION_LEN    32    // EVDO�汾�ַ����ĳ���
#define ESN_LEN             32    // ESN�ַ����ĳ���
#define CM500_START_TIME_15 15    // 20��

#define AT_TIMEOUT          1     // AT�����ѯʧ��
#define AT_OK               0     // AT�����ѯ�ɹ�
#define AT_CHLD_EXITED      1     // û�����߳��˳�
#define AT_CHLD_NOT_EXITED  0     // �����߳��˳�
#define AT_MAIN_EXITED      1     // ���߳��˳�
#define AT_SM_RCV_EXITED    1     // AT�ϱ��߳��˳�


#define  SERVPORT           8765  //�����������˿ں�
#define  BACKLOG            10    // ���ͬʱ����������
#define  REQUEST_SIZE       4     // �ύ�������󳤶�

#define AT_CMD_PARAM_LEN    8     //AT�����������

struct evdo_version_3
{
    char soft[EVDO_VERSION_LEN];  // ���Ȳ�����31���ַ���
    char hard[EVDO_VERSION_LEN];  // ���Ȳ�����31���ַ���
};

// ppp����״̬
#define CONNECTED     1
#define CONNECTING    0
#define DISCONNECTED -1

// ppp IP��ַ������״̬
struct ppp_info_4
{
    unsigned int ip_addr;
    unsigned int ppp_state;
};

// ppp ��ǰ����ʱ�����ۼ�����ʱ��
struct ppp_time_stat_5 
{
    unsigned int cur_time_len;
    unsigned int total_time_len;
};

// ppp ��ǰ�������ۼ�����
// A064D00313 EC506 MOD (by l45517 2005��10��25?) BEGIN
/*
struct ppp_stat_6 
{
    unsigned int cur_ppp_ibytes;
    unsigned int cur_ppp_obytes;
    unsigned int total_ppp_ibytes;
    unsigned int total_ppp_obytes;
};
*/
struct ppp_stat_6
{
    long long cur_ppp_ibytes;
    long long cur_ppp_obytes;
    long long total_ppp_ibytes;
    long long total_ppp_obytes;
};
// A064D00313 EC506 MOD (by l45517 2005��10��25?) END


//���ظ�client����Ľṹ��
struct request_result 
{
    int ret_val;     // ��ʾ����ִ���Ƿ�ɹ�
    int cmd;         // �����������
    union 
    {
        int    uim_1;
        int    evdo_rssi_2;
        struct evdo_version_3   evdo_version_3;
        struct ppp_info_4       ppp_info_4;
        struct ppp_time_stat_5  ppp_time_5;

        struct ppp_stat_6       ppp_stat_6;
        int    net_type_13;
    } ret_data;
};

/*AT���ò����ṹ��, add by sxg*/
typedef struct tagATConfig
{
	char *profile;
	char *phone_number;
	char *ap_name;
	unsigned int operator;
	unsigned int conn_type;
	char * channel;
}AT_CONFIG_S;

#define SIGNAL(s, handler)	do { \
	sa.sa_handler = handler; \
	if (sigaction(s, &sa, NULL) < 0) \
	    FATAL("Couldn't establish signal handler (%d): %m", s); \
    } while (0);

#endif //AT_THREAD_H

