
#ifndef _WAN_BIND_H_
#define _WAN_BIND_H_
#include <linux/skbuff.h>
#include <linux/timer.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <asm/atomic.h>
#include <linux/if.h>
#include <linux/netdevice.h>

/* WAN连接类型长度 */
#define WAN_CONNTYPE_LEN 16

/* WAN服务列表长度 */
#define WAN_SERVICE_LEN 256

/* WAN名字长度 */
#define WAN_NAME_LEN  256

/* LAN名字长度 */
#define LAN_NAME_LEN  64

/* LAN数目 */
#define WAN_LAN_NUM 8

/*WAN端口绑定记录结构*/
struct WanBindMap
{
	char   acLanName[LAN_NAME_LEN];  //LAN口名字做索引，一个LAN口只能绑定到一个PVC
	char   acPvcName[WAN_NAME_LEN];  //LAN口绑定的PVC名字，如果没有绑定则为空，NAS接口
	unsigned int ulPppxFlag;  //绑定的PVC是否启用了代理 
	unsigned int ulBindFlag;  //绑定标识。绑定了为BIND_TRUE；没有绑定则为BIND_FALSE
	char   acConnType[WAN_CONNTYPE_LEN];
	char   acServiceList[WAN_SERVICE_LEN];
};

/*所有的wan信息*/
struct WanDev
{
	struct list_head list;
	char acName[WAN_NAME_LEN];	//WAN口名字做索引
	unsigned int ulBind_lans;   //绑定的LAN
	int lBinded;                //是否启用绑定
	int lPppx;                  //是否启用pppoe代理
    int lWan_type;              //wan类型
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
	char acName[WAN_NAME_LEN];	//WAN口名字做索引
	unsigned int ulBind_lans;   //绑定的LAN
	int lBinded;                //是否启用绑定
	int lPppx;                  //是否启用pppoe代理
    int lWan_type;              //wan类型
    //add by z67625 for CT
	unsigned int ulVlanMode;   //vlan mode
};


extern struct WanDev *find_wanbyname(char *pname);

//add by z67625
struct WanMode
{
    char acName[WAN_NAME_LEN];   //WAN口名字
    int lMode;                   //vlanmode
    int lVid;                    //vlanid号
    int lifindex;                 //接口对应设备index号
};
#endif
