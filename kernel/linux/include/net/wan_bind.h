
#ifndef _WAN_BIND_H_
#define _WAN_BIND_H_
#include <linux/skbuff.h>
#include <linux/timer.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <asm/atomic.h>
#include <linux/if.h>
#include <linux/netdevice.h>

/* WAN�������ͳ��� */
#define WAN_CONNTYPE_LEN 16

/* WAN�����б��� */
#define WAN_SERVICE_LEN 256

/* WAN���ֳ��� */
#define WAN_NAME_LEN  256

/* LAN���ֳ��� */
#define LAN_NAME_LEN  64

/* LAN��Ŀ */
#define WAN_LAN_NUM 8

/*WAN�˿ڰ󶨼�¼�ṹ*/
struct WanBindMap
{
	char   acLanName[LAN_NAME_LEN];  //LAN��������������һ��LAN��ֻ�ܰ󶨵�һ��PVC
	char   acPvcName[WAN_NAME_LEN];  //LAN�ڰ󶨵�PVC���֣����û�а���Ϊ�գ�NAS�ӿ�
	unsigned int ulPppxFlag;  //�󶨵�PVC�Ƿ������˴��� 
	unsigned int ulBindFlag;  //�󶨱�ʶ������ΪBIND_TRUE��û�а���ΪBIND_FALSE
	char   acConnType[WAN_CONNTYPE_LEN];
	char   acServiceList[WAN_SERVICE_LEN];
};

/*���е�wan��Ϣ*/
struct WanDev
{
	struct list_head list;
	char acName[WAN_NAME_LEN];	//WAN������������
	unsigned int ulBind_lans;   //�󶨵�LAN
	int lBinded;                //�Ƿ����ð�
	int lPppx;                  //�Ƿ�����pppoe����
    int lWan_type;              //wan����
    //add by z67625 for CT
    unsigned int ulVlanMode;    //vlan mode
    int lIfidx;                 //Ifidx
    
};

//add by z67625 for CT
struct LanVlan
{
    int vlanid;
    int priority;        
};

struct bindinfo
{
	char acName[WAN_NAME_LEN];	//WAN������������
	unsigned int ulBind_lans;   //�󶨵�LAN
	int lBinded;                //�Ƿ����ð�
	int lPppx;                  //�Ƿ�����pppoe����
    int lWan_type;              //wan����
    //add by z67625 for CT
	unsigned int ulVlanMode;   //vlan mode
};


extern struct WanDev *find_wanbyname(char *pname);

//add by z67625
struct WanMode
{
    char acName[WAN_NAME_LEN];   //WAN������
    int lMode;                   //vlanmode
    int lVid;                    //vlanid��
    int lifindex;                 //�ӿڶ�Ӧ�豸index��
};
#endif
