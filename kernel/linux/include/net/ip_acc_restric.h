
#ifndef _IP_ACC_RESTRIC_H_
#define  _IP_ACC_RESTRIC_H_
#include <linux/skbuff.h>
#include <linux/timer.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <asm/atomic.h>
#include <linux/if.h>

#define ACC_DEVADDR_HASH_KEY(addr) (((addr)^((addr)>>8)^((addr)>>16)^((addr)>>24)) & 0xF)
#define ACC_DEVADDR_BUF_MAX         16

#define ACC_DEV_LIVE_ALWAYS         0
#define ACC_DEV_LIVE_DEFAULT        60 //s
#define ACC_TRUE                      (1 == 1)
#define ACC_FALSE                     (1 == 0)

#define ACC_ARP_PROBE_TIMES_MAX     3
#define ACC_ARP_PROBE_INTERVAL      2 //s

/* start static ip restric A36D04768 20080623 */
#define ACC_STATIC_IP      0
#define ACC_DYNAMIC_IP     1
/* end static ip restric A36D04768 20080623 */

enum
{
    ACC_DUMP_RESTRIC = 0,
    ACC_DUMP_ALLOW,
    ACC_DUMP_CRNT,
    ACC_DUMP_ALL,
    /* more add here */
    ACC_DUMP_BUTT        
};

enum
{
    ACC_DEVTYPE_STB = 0,
    ACC_DEVTYPE_PC,    
    ACC_DEVTYPE_WIFI,
    ACC_DEVTYPE_CAMERA,
    /* more add here */
    ACC_DEVTYPE_OTHER,
    ACC_DEVTYPE_BUTT
};

enum
{
	ACC_MODE_DISABLE = 0,
	ACC_MODE_BYTOTAL,
	ACC_MODE_BYTYPE,
	/* more add here */
	ACC_MODE_BUTT
};

#define  ACC_WAN_BRIDGE     0
#define  ACC_WAN_ROUTE      1
struct acc_restric_hdr;
struct acc_devtype_hdr;
struct acc_device;

struct acc_arp_probe
{
    spinlock_t         lock;
    unsigned int       times;
    struct timer_list timer;  
    struct acc_device *parent;
};

struct acc_rstric_wantype
{
    struct list_head  list;
    char             wan_name[IFNAMSIZ];
    char             bindlist[256];
    int              wan_type;
};

struct acc_wan
{
    char             wan_name[IFNAMSIZ];
    char             bindlist[256];
    int              wan_type;
};


struct acc_device
{
    struct list_head	 list_allow;
    struct list_head	 list_crnt;
    struct timer_list	 timer;
	atomic_t		     refcnt;
	atomic_t             dirty;
    unsigned int         addr;
    unsigned int         live_time;
    /* start static ip restric A36D04768 20080623 */    
    unsigned int         ip_flg;  /* static 0, dynamic 1 */
    /* end static ip restric A36D04768 20080623 */
    struct acc_arp_probe    arp_probe; 
    struct acc_devtype_hdr *dev_type;
};

struct acc_dev_type    //user space for config
{
    unsigned int enable;
    unsigned int type;
    unsigned int total_allow;
};

struct acc_dev_desc     //user space for config
{
    unsigned int type;
    unsigned int addr;
    unsigned int live_time;  //s
};

struct acc_restric_desc //user space for config
{
    unsigned int            enable;
    unsigned int            total_allow;
    char                    lan_name[IFNAMSIZ];
    unsigned int            lan_host;
    unsigned int            lan_mask;
    struct acc_dev_type    acc_devtype[ACC_DEVTYPE_BUTT];
};

struct acc_devtype_hdr
{
    unsigned int            enable;
    unsigned int            type;
    unsigned int            total_allow;
    unsigned int            total_crnt; 
    struct acc_restric_hdr *parent_hdr;
};

struct acc_restric_hdr
{
    unsigned int            inited;
    unsigned int            enable;
    unsigned int            total_allow;
    unsigned int            total_crnt;
    char                    lan_name[IFNAMSIZ];
    unsigned int            lan_host;
    unsigned int            lan_mask;
    spinlock_t              lock_allow;
    spinlock_t              lock_crnt;
    struct list_head       allow_devs[ACC_DEVADDR_BUF_MAX];    
    struct list_head       crnt_devs[ACC_DEVADDR_BUF_MAX]; 
    struct acc_devtype_hdr acc_devtype[ACC_DEVTYPE_BUTT];
};

extern void ipacc_restric_init(void);
#endif
