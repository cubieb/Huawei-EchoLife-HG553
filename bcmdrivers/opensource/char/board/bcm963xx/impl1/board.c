/*
<:copyright-gpl 
 Copyright 2002 Broadcom Corp. All Rights Reserved. 
 
 This program is free software; you can distribute it and/or modify it 
 under the terms of the GNU General Public License (Version 2) as 
 published by the Free Software Foundation. 
 
 This program is distributed in the hope it will be useful, but WITHOUT 
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 for more details. 
 
 You should have received a copy of the GNU General Public License along 
 with this program; if not, write to the Free Software Foundation, Inc., 
 59 Temple Place - Suite 330, Boston MA 02111-1307, USA. 
:>
*/
/***************************************************************************
 * File Name  : board.c
 *
 * Description: This file contains Linux character device driver entry 
 *              for the board related ioctl calls: flash, get free kernel
 *              page and dump kernel memory, etc.
 *
 *1.增加FLASH VA分区by l39225 2006-5-8
  2.增加对RESET BUTTON的处理by l39225 2006-5-8
 ***************************************************************************/
/* Includes. */
#include <linux/version.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/capability.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/pagemap.h>
#include <asm/uaccess.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/if.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

#include <bcm_map_part.h>
#include <board.h>
#include <bcmTag.h>
#include "boardparms.h"
#include "flash_api.h"
#include "bcm_intr.h"
#include "board.h"
#include "bcm_map_part.h"
#include <asm/delay.h>

/* Typedefs. */

#if defined (WIRELESS)
#if 1 /* 2008/01/28 Jiajun Weng : New code from 3.12L.01 */
#define SES_EVENT_BTN_PRESSED      0x00000001
#define SES_EVENTS                 SES_EVENT_BTN_PRESSED /*OR all values if any*/
#endif
#define SES_LED_OFF     0
#define SES_LED_ON      1
#define SES_LED_BLINK   2
#endif

typedef struct
{
    unsigned long ulId;
    char chInUse;
    char chReserved[3];
} MAC_ADDR_INFO, *PMAC_ADDR_INFO;

typedef struct
{
    unsigned long ulSdramSize;
    unsigned long ulPsiSize;
    unsigned long ulNumMacAddrs;
    unsigned char ucaBaseMacAddr[NVRAM_MAC_ADDRESS_LEN];
    char szSerialNumber[NVRAM_SERIAL_NUMBER_LEN];     /* y42304 added:支持序列号 */
    MAC_ADDR_INFO MacAddrs[1];
} NVRAM_INFO, *PNVRAM_INFO;

typedef struct
{
    unsigned long eventmask;    
} BOARD_IOC, *PBOARD_IOC;

#define POLLCNT_1SEC        HZ
#define BP_ACTIVE_MASK   0x8000
#define FLASH_WRITING      1
#define FLASH_IDLE             0
static struct timer_list   poll_timer;
int s_iFlashWriteFlg = 0;
EXPORT_SYMBOL(s_iFlashWriteFlg);


static struct timer_list   poll_dataCard;
int g_iReset = 1;
EXPORT_SYMBOL(g_iReset);


#if defined (WIRELESS)
static int WPS_FLAG = 0 ;
static struct timer_list   switch_wlan_timer;
#endif

/*Dyinggasp callback*/
typedef void (*cb_dgasp_t)(void *arg);
typedef struct _CB_DGASP__LIST
{
    struct list_head list;
    char name[IFNAMSIZ];
    cb_dgasp_t cb_dgasp_fn;
    void *context;
}CB_DGASP_LIST , *PCB_DGASP_LIST;

static struct workqueue_struct *board_workqueue;
static struct work_struct board_work;


static LED_MAP_PAIR LedMapping[MAX_VIRT_LEDS] =
{   // led name     Initial state       physical pin (ledMask)
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},   
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},    
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},   
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},            
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32} // NOTE: kLedEnd has to be at the end.
};

/* Externs. */
extern struct file fastcall *fget_light(unsigned int fd, int *fput_needed);
//extern unsigned int nr_free_pages (void);
#define nr_free_pages() global_page_state(NR_FREE_PAGES)
extern const char *get_system_type(void);
extern void kerSysFlashInit(void);
extern unsigned long get_nvram_start_addr(void);
extern unsigned long get_scratch_pad_start_addr(void);
extern unsigned long getMemorySize(void);
extern void __init boardLedInit(PLED_MAP_PAIR);
extern void boardLedCtrl(BOARD_LED_NAME, BOARD_LED_STATE);
extern void kerSysLedRegisterHandler( BOARD_LED_NAME ledName,
    HANDLE_LED_FUNC ledHwFunc, int ledFailType );
extern UINT32 getCrc32(byte *pdata, UINT32 size, UINT32 crc);

/* Prototypes. */
void __init InitNvramInfo( void );
static int board_open( struct inode *inode, struct file *filp );
static int board_ioctl( struct inode *inode, struct file *flip, unsigned int command, unsigned long arg );
static ssize_t board_read(struct file *filp,  char __user *buffer, size_t count, loff_t *ppos); 
static unsigned int board_poll(struct file *filp, struct poll_table_struct *wait);
static int board_release(struct inode *inode, struct file *filp);                        

static BOARD_IOC* borad_ioc_alloc(void);
static void borad_ioc_free(BOARD_IOC* board_ioc);

/* DyingGasp function prototype */
static void __init kerSysDyingGaspMapIntr(void);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
static irqreturn_t kerSysDyingGaspIsr(int irq, void * dev_id, struct pt_regs * regs);
#else
static unsigned int kerSysDyingGaspIsr(void);
#endif
static void __init kerSysInitDyingGaspHandler( void );
static void __exit kerSysDeinitDyingGaspHandler( void );
/* -DyingGasp function prototype - */

static int ConfigCs(BOARD_IOCTL_PARMS *parms);
static void SetPll(int pll_mask, int pll_value);
static void SetGpio(int gpio, GPIO_STATE_t state);
static int kerSysGetResetStatus(void);
static void bcm63xx_poll_reset_button(void);
static void bcm63xx_poll_switch_wlan_key(unsigned long ulData);

#if defined (WIRELESS)
static irqreturn_t sesBtn_isr(int irq, void *dev_id, struct pt_regs *ptregs);
static void __init sesBtn_mapGpio(void);
static void __init sesBtn_mapIntr(int context);
static Bool sesBtn_pressed(void); //for WPS feature by yiheng
static unsigned int sesBtn_poll(struct file *file, struct poll_table_struct *wait);
static ssize_t sesBtn_read(struct file *file,  char __user *buffer, size_t count, loff_t *ppos);
static void __init sesLed_mapGpio(void);
static void sesLed_ctrl(int action);
static void __init ses_board_init(void);
static void __exit ses_board_deinit(void);
#endif

static PNVRAM_INFO g_pNvramInfo = NULL;
static int g_ledInitialized = 0;
static wait_queue_head_t g_board_wait_queue;
static CB_DGASP_LIST *g_cb_dgasp_list_head = NULL;

static int g_wakeup_monitor = 0;
static struct file *g_monitor_file = NULL;
static struct task_struct *g_monitor_task = NULL;
static unsigned int (*g_orig_fop_poll)
    (struct file *, struct poll_table_struct *) = NULL;

static struct file_operations board_fops =
{
  open:       board_open,
  ioctl:      board_ioctl,
  poll:       board_poll,
  read:       board_read,
  release:    board_release,
};

uint32 board_major = 0;

static int g_sWlanFlag = 0;

static int g_sVoipServiceStatus = 0;  /*1: 正在进行VOIP业务; 0: 没有进行VOIP业务*/

#if defined (WIRELESS)
static unsigned short sesBtn_irq = BP_NOT_DEFINED;
static unsigned short sesBtn_gpio = BP_NOT_DEFINED;
static unsigned short sesLed_gpio = BP_NOT_DEFINED;
#endif


/* HUAWEI HGW s48571 2008年1月19日 Hardware Porting add begin:*/
int g_nHspaTrafficMode = MODE_NONE;
EXPORT_SYMBOL(g_nHspaTrafficMode);
/* HUAWEI HGW s48571 2008年1月19日 Hardware Porting add end.*/

/* HUAWEI HGW s48571 2008年2月20日 装备测试状态标志添加 add begin:*/
int g_nEquipTestMode = FALSE;
EXPORT_SYMBOL(g_nEquipTestMode);
/* HUAWEI HGW s48571 2008年2月20日 装备测试状态标志添加 add end.*/

/* HUAWEI HGW s48571 2008年8月16日 规避特定业务情况下WAN地址丢失 add begin:*/
int g_iAtmReset = 0;
EXPORT_SYMBOL(g_iAtmReset);
/* HUAWEI HGW s48571 2008年8月16日 规避特定业务情况下WAN地址丢失 add end.*/

extern atomic_t g_iDataCardIn;
extern int iUsbTtyState;


#if defined(MODULE)
int init_module(void)
{
    return( brcm_board_init() );              
}

void cleanup_module(void)
{
    if (MOD_IN_USE)
        printk("brcm flash: cleanup_module failed because module is in use\n");
    else
        brcm_board_cleanup();
}
#endif //MODULE 


static void clear_port_register(int port)
{
#define EHCI_PORT_SC_0_ADDR		0xfffe1354
#define EHCI_PORT_SC_1_ADDR		0xfffe1358
#define EHCI_PORT_STAT_MASK		0x00000F00
#define EHCI_PORT_J_STAT			0x00000900
#define EHCI_PORT_K_STAT			0x00000500
#define EHCI_BAD_LIMIT                      0x01


	static unsigned int siMulAdd[2] = {0, 0};
	volatile unsigned int *ulReg = port ? EHCI_PORT_SC_1_ADDR : EHCI_PORT_SC_0_ADDR;
	int iRegVal = 0;

	if (port > 1)
	{
		return;
	}

	iRegVal = *ulReg & EHCI_PORT_STAT_MASK;

	if (iRegVal == EHCI_PORT_J_STAT || iRegVal == EHCI_PORT_K_STAT)
	{
		siMulAdd[port]++;
	}

	if (siMulAdd[port] > EHCI_BAD_LIMIT)
	{
		*ulReg = 0x00001002;
		siMulAdd[port] = 0;
		printk("ly: ......clear usb. \n");
	}

	return;
}

#define  ATM_QUEUE_CHECK_INTERVAL   15   //ATM 队列丢包检测周期，单位为s
#define  ATM_QUEUE_CHECK_COUNT       5   //ATM 队列丢包检测次数
/*****************************************************************************
 函 数 名  : fault_detect
 功能描述  : 检测数据卡是否识别正确
 输入参数  : unsigned long ulData  
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年4月18日
    作    者   : liuyang 65130
    修改内容   : 新生成函数

*****************************************************************************/
void fault_detect(unsigned long ulData)
{
    static int iDo = 0;
    static int iOldUsbState = 3;
    
    /* HUAWEI HGW s48571 2008年8月16日 规避特定业务情况下WAN地址丢失 add begin:*/
    static int iTime = 0;
    static int iCount = 0;
    /* HUAWEI HGW s48571 2008年8月16日 规避特定业务情况下WAN地址丢失 add end.*/
    int i;
    if (atomic_read(&g_iDataCardIn))
    {
        if (iUsbTtyState < 3 && (!iDo))
        {  
            if (iOldUsbState != iUsbTtyState)
            {
            	   for (i = 0; i < 2; i++)
		    {
				clear_port_register(i);
		    }
                iOldUsbState = iUsbTtyState;
/* BEGIN: Modified by weishi kf33269, 2011/3/28   PN:hspa led turn red for a while, when hspa card insert*/
                poll_dataCard.expires = jiffies + 5*HZ;
/* END:   Modified by weishi kf33269, 2011/3/28 */
                add_timer(&poll_dataCard);
                return;
            }
            
            kerSysLedCtrl(kLedHspa, kLedStateFail);
        }
        iDo = 1;
    }
    else
    {
        if (iDo)
        {
            kerSysLedCtrl(kLedHspa, kLedStateOff);
            iDo = 0;
	  iOldUsbState = 3;
        }
    }

    for (i = 0; i < 2; i++)
    {
		clear_port_register(i);
    }

    /* HUAWEI HGW s48571 2008年8月16日 规避特定业务情况下WAN地址丢失 add begin:*/
    iTime++;
    if( ATM_QUEUE_CHECK_INTERVAL == iTime)
    {
        iTime = 0;
        if ( 0 ==  g_iAtmReset )
        {
            iCount = 0;
        }
        else if( 1 ==  g_iAtmReset )//处于记录态，下面转入待启动态
        {
            iCount = 0;
            g_iAtmReset = 2;
        }
        else if ( 2 ==  g_iAtmReset )//处于待启动态，继续检测，
        {
            iCount++;
            printk("\nATM Queue full count: %d (check interval: %d s)\n",iCount, ATM_QUEUE_CHECK_INTERVAL);
        }
    }

    if ( ATM_QUEUE_CHECK_COUNT == iCount )
    {   
        printk("\nATM queue full, System will reboot! \n");
		
        kerSysMipsSoftReset();
    }
    /* HUAWEI HGW s48571 2008年8月16日 规避特定业务情况下WAN地址丢失 add end.*/
    
    
    poll_dataCard.expires = jiffies +  HZ;
    add_timer(&poll_dataCard);
    
    return;
}


/* 2008/01/28 Jiajun Weng : New code from 3.12L.01 */
static int map_external_irq (int irq)
{
   int map_irq ;

   switch (irq) {
      case BP_EXT_INTR_0   :
         map_irq = INTERRUPT_ID_EXTERNAL_0;
         break ;
      case BP_EXT_INTR_1   :
         map_irq = INTERRUPT_ID_EXTERNAL_1;
         break ;
      case BP_EXT_INTR_2   :
         map_irq = INTERRUPT_ID_EXTERNAL_2;
         break ;
      case BP_EXT_INTR_3   :
         map_irq = INTERRUPT_ID_EXTERNAL_3;
         break ;
#if defined(CONFIG_BCM96358) || defined(CONFIG_BCM96368)
      case BP_EXT_INTR_4   :
         map_irq = INTERRUPT_ID_EXTERNAL_4;
         break ;
      case BP_EXT_INTR_5   :
         map_irq = INTERRUPT_ID_EXTERNAL_5;
         break ;
#endif
      default           :
         printk ("Invalid External Interrupt definition \n") ;
         map_irq = 0 ;
         break ;
   }
     return (map_irq) ;
}

static int __init brcm_board_init( void )
{
    typedef int (*BP_LED_FUNC) (unsigned short *);
    static struct BpLedInformation
    {
        BOARD_LED_NAME ledName;
        BP_LED_FUNC bpFunc;
        BP_LED_FUNC bpFuncFail;
    } bpLedInfo[] =
    {{kLedAdsl, BpGetAdslLedGpio, BpGetAdslFailLedGpio},
    /* HUAWEI HGW s48571 2008年1月18日 hardware porting modify begin:*/
    {kLedWireless, NULL, BpGetWirelessFailLedGpio},
    /* HUAWEI HGW s48571 2008年1月18日 hardware porting modify end. */
     {kLedUsb, BpGetUsbLedGpio, NULL},
     {kLedHpna, BpGetHpnaLedGpio, NULL},
     {kLedWanData, BpGetWanDataLedGpio, NULL},
     {kLedWanDataFail, BpGetWanDataFailLedGpio, NULL},             
     {kLedPPP, BpGetPppLedGpio, BpGetPppFailLedGpio},
     {kLedVoip, BpGetVoipLedGpio, NULL},
     {kLedSes, BpGetWirelessSesLedGpio, NULL}, 
     /*start of  GPIO 3.4.2 porting by l39225 20060504*/
/*s48571 modify  for VDF HG553 hardware porting begin*/
//     {kLedPower,BpGetBootloaderPowerOnLedGpio,NULL},
     {kLedPower,BpGetBootloaderPowerOnLedGpio,BpGetBootloaderStopLedGpio},
/*s48571 modify  for VDF HG553 hardware porting end*/
     /*end of  GPIO 3.4.2 porting by l39225 20060504*/
     /*start of y42304 added 20061109: 支持2路电话线路等的支持 */
     {kLedLine0,BpGetTelLine0LedGpio,NULL},
     {kLedLine1,BpGetTelLine1LedGpio,NULL},
     {kLedPSTN, BpGetPstnLedGpio,    NULL}, 
    /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add begin:*/
    {kLedInternet, BpGetInternetLedGpio, BpGetInternetFailLedGpio}, 
    {kLedHspa, BpGetHspaLedGpio, BpGetHspaFailLedGpio}, 
    /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add end.*/
     /*end of y42304 added 20061109: 支持2路电话线路等的支持 */
     {kLedEnd, NULL, NULL}
    };

    int ret;
        
    ret = register_chrdev(BOARD_DRV_MAJOR, "bcrmboard", &board_fops );
    if (ret < 0)
    {
        printk( "brcm_board_init(major %d): fail to register device.\n",BOARD_DRV_MAJOR);
    }
    else 
    {
        PLED_MAP_PAIR pLedMap = LedMapping;
        unsigned short gpio;
        struct BpLedInformation *pInfo;

        printk("brcmboard: brcm_board_init entry\n");
        board_major = BOARD_DRV_MAJOR;
        InitNvramInfo();

        for( pInfo = bpLedInfo; pInfo->ledName != kLedEnd; pInfo++ )
        {
            if( pInfo->bpFunc && (*pInfo->bpFunc) (&gpio) == BP_SUCCESS )
            {
                pLedMap->ledName = pInfo->ledName;
                pLedMap->ledActiveLow = (gpio & BP_ACTIVE_LOW) ? 1 : 0;
                pLedMap->ledSerial = (gpio & BP_GPIO_SERIAL) ? 1 : 0;

                /* start of board added by y42304 20060517: 解决能用GPIO32或以上点灯的问题 */
                if ((gpio & GPIO_NUM_TOTAL_BITS_MASK) >= 32)
                {                                        
                    pLedMap->ledMask = GPIO_NUM_TO_MASK_HIGH(gpio);
                    pLedMap->ledlow32GPIO = GPIO_HIGH; /* 所用的GPIO号为32或以上 */
                }
                else
                {                    
                    pLedMap->ledMask = GPIO_NUM_TO_MASK(gpio);
                }
                /* end of board added by y42304 20060517 */
            }
            if( pInfo->bpFuncFail && (*pInfo->bpFuncFail) (&gpio) == BP_SUCCESS )
            {
                pLedMap->ledName = pInfo->ledName;
                pLedMap->ledActiveLowFail = (gpio & BP_ACTIVE_LOW) ? 1 : 0;
                pLedMap->ledSerialFail = (gpio & BP_GPIO_SERIAL) ? 1 : 0;
                
                /* start of board added by y42304 20060517: 解决能用GPIO32或以上点灯的问题 */
                if ((gpio & GPIO_NUM_TOTAL_BITS_MASK) >= 32)
                {                                        
                    pLedMap->ledMaskFail = GPIO_NUM_TO_MASK_HIGH(gpio);                    
                    pLedMap->ledlow32GPIO = GPIO_HIGH; /* 所用的GPIO号为32或以上 */
                }
                else
                {                    
                    pLedMap->ledMaskFail = GPIO_NUM_TO_MASK(gpio);
                }
                /* end of board added by y42304 20060517 */
            }
            if( pLedMap->ledName != kLedEnd )
            {
                pLedMap++;
            }
        }
        
        init_waitqueue_head(&g_board_wait_queue);

        board_workqueue = create_singlethread_workqueue("board");
        
#if defined (WIRELESS)
        ses_board_init();
#endif        
        kerSysInitDyingGaspHandler();
        kerSysDyingGaspMapIntr();

        boardLedInit(LedMapping);
        g_ledInitialized = 1;
        /* start of y42304 added 20061230: 系统启动时先关闭Voip, 线路LED */    
         kerSysLedCtrl(kLedVoip, kLedStateOff);        
         kerSysLedCtrl(kLedLine0,kLedStateOff);        
         kerSysLedCtrl(kLedLine1,kLedStateOff);
        /* end of y42304 added 20061230 */     
        /*START ADD : liujianfeng 37298 2007-01-14 for [A36D03423]*/
         kerSysLedCtrl(kLedPSTN,kLedStateOn);
        /*END ADD : liujianfeng 37298 2007-01-14 for [A36D03423]*/

        /*start of  GPIO 3.4.3 porting by l39225 20060504*/
        init_timer(&poll_timer);
        poll_timer.function = bcm63xx_poll_reset_button;
        poll_timer.expires = jiffies + 10*POLLCNT_1SEC;
        add_timer(&poll_timer);
        /*end of  GPIO 3.4.3 porting by l39225 20060504*/

    /* Start of y42304 added 20061223: 支持HG550i ADM6996+Si3215单板wlan开关按键功能 */
    #if defined (WIRELESS)    	
        init_timer(&switch_wlan_timer);
        switch_wlan_timer.function = bcm63xx_poll_switch_wlan_key;
        switch_wlan_timer.expires = jiffies + 10*POLLCNT_1SEC;
        add_timer(&switch_wlan_timer);
    #endif        
    /* end of y42304 added 20061223 */


        /*start of l65130 hspa故障检测*/
        init_timer(&poll_dataCard);
        poll_dataCard.function = fault_detect;
        poll_dataCard.expires = jiffies + 30*HZ;
        add_timer(&poll_dataCard);
        /* end of l65130 */
    
    
        kerSysLedCtrl(kLedPower,kLedStateOn);    
    }
    return ret;
} 

#if defined (WIRELESS)      
/*************************************************
  Function:       kerSysGetWlanButtonStatus
  Description:    获取wlan 开关按键按键的状态
  Input:          无
  Output:         无
  Return:          0:  低电平
                   >0: 高电平
                   -1: 无wlan开关按键
*************************************************/
static int kerSysGetWlanButtonStatus(void)
{
	unsigned short unGpio;
	unsigned long ulGpioMask;
	volatile unsigned long *pulGpioReg;
	int iRet = -1;

    /* 得到wlan 开关按键GPIO*/
	//if( BpGetWirelessLedGpio( &unGpio ) == BP_SUCCESS )
	//modify by y68191 无线状态按钮变更
    if( BpGetWirelessSesBtnGpio( &unGpio ) == BP_SUCCESS )
	{
		ulGpioMask = GPIO_NUM_TO_MASK(unGpio);
		pulGpioReg = &GPIO->GPIOio;

	#if !defined(CONFIG_BCM96338)
		if( (unGpio & BP_GPIO_NUM_MASK) >= 32 )
		{
			ulGpioMask = GPIO_NUM_TO_MASK_HIGH(unGpio);
			pulGpioReg = &GPIO->GPIOio_high;
		}
	#endif

		iRet = (int) *pulGpioReg & ulGpioMask;
	}
	else 
	{
		//printk(" Hardware no wlan button\n");
	}

	return iRet;
}

/* start of vdf HG553V100R001B016 w45260 added 20080421: 解决问题单AU8D00593 .
 WiFi 按键，如果长按 <3s: no action ;3-10s: Wifi on/off ;>10s: WPS
*/
/* Start of vdf HG553V100R001B020 l68693 modify 20080703: RESET功能下线改为1秒 */
#define REVERSE_WLAN_STATE_TIME       1   
/*end of of vdf HG553V100R001B020 l68693 modify 20080703: */

#define WPS_WLAN_TIME                 10   
/*end of of vdf HG553V100R001B016 w45260 added 20080421: */

/*注意:太大会浪费CPU资源,太小会降低按键检测精度给使用带来不便*/
/* Start of vdf HG553V100R001B020 l68693 modify 20080703: 改为每秒检测4次 */
#define COUNTS_PSEC                   4  
/*end of of vdf HG553V100R001B020 l68693 modify 20080703: */

static void bcm63xx_poll_switch_wlan_key(unsigned long ulData)
{
    static unsigned char s_ucWlanButtonCnt = 0;
    static unsigned char s_ucIsChangeWlanStatus = 0;
    /* Start of vdf HG553V100R001B020 l68693 modify 20080703: RESET功能下线改为1秒 */    
    /* -1 确保最多在REVERSE_WLAN_STATE_TIME * COUNTS_PSEC个周期内能够触发按钮事件*/
    static int  iStateReverseCounts = REVERSE_WLAN_STATE_TIME * COUNTS_PSEC - 1;
    /*end of of vdf HG553V100R001B020 l68693 modify 20080703: */
    
    static int  iWPSCounts      = WPS_WLAN_TIME * COUNTS_PSEC;
    static int  iJiffiesCounts  = POLLCNT_1SEC/COUNTS_PSEC;
    int  iRet;
	
    iRet = kerSysGetWlanButtonStatus();
    if (iRet == 0) 
    {            
        s_ucWlanButtonCnt++;
    }
	else if (iRet > 0)
	{
        if (s_ucWlanButtonCnt > iWPSCounts)
        {
            printk(" wps button has been pressed!\n");
            WPS_FLAG = 1;
            wake_up_interruptible(&g_board_wait_queue); 
        }
        else if (s_ucWlanButtonCnt >= iStateReverseCounts)
		{    
		    printk(" wlan reset  button has been pressed!!\n");
            g_sWlanFlag = (0 == g_sWlanFlag)?1:0;
            //s_ucIsChangeWlanStatus = 1;
            kerSysWakeupMonitorTask();   	
		}
        s_ucWlanButtonCnt = 0;
	}

    if (iRet >= 0)
    {
        /* Start of w45260 added 20080417: AU8D00593 定时间隔设定为 (1/COUNTS_PSEC) S */
        switch_wlan_timer.expires = jiffies + iJiffiesCounts;
		/* end of w45260 added 20080417*/
        add_timer(&switch_wlan_timer);
    }    
}
#endif

/*start of  GPIO 3.4.3 porting by l39225 20060504*/
/*************************************************
  Function:        kerSysGetResetStatus
  Description:    获取复位按键的状态
  Input:            无
  Output:          无
  Return:          0: 低电平
                       >0 :高电平
                       -1:无复位按键
  Others:         无
*************************************************/

static int kerSysGetResetStatus(void)
{
	unsigned short unGpio;
	unsigned long ulGpioMask;
	volatile unsigned long *pulGpioReg;
	int iRet = -1;

	if( BpGetPressAndHoldResetGpio( &unGpio ) == BP_SUCCESS )
	{
		ulGpioMask = GPIO_NUM_TO_MASK(unGpio);
		pulGpioReg = &GPIO->GPIOio;

	#if !defined(CONFIG_BCM96338)
		if( (unGpio & BP_GPIO_NUM_MASK) >= 32 )
		{
			ulGpioMask = GPIO_NUM_TO_MASK_HIGH(unGpio);
			pulGpioReg = &GPIO->GPIOio_high;
		}
	#endif

		iRet = (int) *pulGpioReg & ulGpioMask;
	}
	else 
	{
		printk(" Hardware no  reset button\n");
	}

	return iRet;
}





/*************************************************
  Function:        bcm63xx_poll_reset_button
  Description:    定时检测复位按键状态并采取对应动作
  Input:            无
  Output:          无
  Return:          无
  Others:          无
*************************************************/
#define FAILURE_TAG_LEN            10
#define FAILURE_TAG                "5aa5a55a"

void restore_do_tasklet(void* ptr)
{
    char cUpgradeFlags[FAILURE_TAG_LEN];
    int iRet = -1;

    printk("ly: LINE: %d\n", __LINE__);

    
    kerSysAvailSet("resdeft", 8, 2400);
    /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify end. */
    kerSysPersistentClear();
    /* begin --- Add Persistent storage interface backup flash partition by w69233 */
    kerSysPsiBackupClear();
    /* end ----- Add Persistent storage interface backup flash partition by w69233 */
    kerSysScratchPadClearAll();

    /*start of y42304 20060727 added: 恢复出厂设置时清除升级标识 */
    /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify begin:
    iRet = kerSysVariableGet(cUpgradeFlags, FAILURE_TAG_LEN, 0);
    */
    iRet = kerSysAvailGet(cUpgradeFlags, FAILURE_TAG_LEN, 0);
    /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify end. */
    if ((-1 != iRet) && (NULL != strstr(cUpgradeFlags, FAILURE_TAG)) 
        && (0 != strcmp(cUpgradeFlags, "5aa5a55a0"))) 
    {
        /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify begin:
        kerSysVariableSet("5aa5a55a0", FAILURE_TAG_LEN, 0);
        */
        kerSysAvailSet("5aa5a55a0", FAILURE_TAG_LEN, 0);
        /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify end. */
    }     
    /*start of y42304 20060727 added: 恢复出厂设置时清除升级标识 */            

    /* HUAWEI HGW s48571 2008年4月8日" AU8D00456 按reset按钮3秒以上没有完全恢复到出厂配置 add begin:*/
    char pinTemp[IFC_PIN_PASSWORD_LEN];
    char *pcBuf;
    char tmp[8];

     pcBuf = (char *)kmalloc( CWMP_PARA_LEN, GFP_KERNEL );
     memset(pcBuf, 0x00, CWMP_PARA_LEN);
     memset(tmp, 0x00, sizeof(tmp));
     memset(pinTemp, 0xff, sizeof(pinTemp));
     kerSysAvailSet(pinTemp, IFC_PIN_PASSWORD_LEN, PINCODE_OFFSET);
     kerSysAvailSet("resdeft", RESTORE_FLAG_LEN, WLAN_PARAMETERS_OFFSET + WLAN_WEP_KEY_LEN + WLAN_WPA_PSK_LEN);
     kerSysAvailSet("resetok" , UPG_INFO_STATUS, (CFG_STATE_DATA_START + UPG_INFO_STATUS) );//cwmp reset flag
     kerSysAvailSet("restore", TELNET_TAG_LEN, FLASH_VAR_START + FAILURE_TAG_LEN );
     kerSysAvailSet("resping", PING_TAG_LEN, FLASH_VAR_START + FAILURE_TAG_LEN + TELNET_TAG_LEN );
     kerSysAvailSet(pcBuf, CWMP_PARA_LEN, CWMP_PARA_OFFSET);
     kerSysAvailSet("HGReset", CWMP_PARA_RESET, CWMP_PARA_OFFSET+CWMP_PARA_LEN);
     /* BEGIN: Added by jiantengfei KF26416, 2010/4/17   PN:reset键恢复默认配置清除DSL激活状态*/
     kerSysAvailSet("0", GATEWAY_ACTIVATION_INFO_LEN,GATEWAY_ACTIVATION_INFO);
     /* END:   Added by jiantengfei KF26416, 2010/4/17 */
     
     /*BEFIN:  增加 显示VDF 升级更新文件 by DHS00169988*/
   kerSysAvailSet("0", CFG_CHG_SIG_LEN, CFG_CHG_FLAG_START);
   kerSysAvailSet("0", CFG_CHG_FLAG_LEN, (CFG_CHG_FLAG_START + CFG_CHG_SIG_LEN));
   kerSysAvailSet("0", PARTCFG_WEBPROTECTION_REMINDER_LEN, PARTCFG_WEBPROTECTION_REMINDER_CHG);
   /*END: 增加 显示VDF 升级更新文件 by DHS00169988*/
	 /* BEGIN: Added by c65985, 2010/10/18   PN:bug150*/
     char acTmp[48];
     memset(acTmp, 0, sizeof(acTmp));
     kerSysAvailSet(acTmp, IFC_IMSIMSISDNPAIR_LEN, IMSIMSISDNPAIR_OFFSET);
     sprintf(acTmp,"0");
     kerSysAvailSet(acTmp, SMS_EASYRESET_FLAG_LEN, SMS_EASYRESET_FLAG_START);
     /* END:   Added by c65985, 2010/10/18 PN:bug150*/
     g_sVoipServiceStatus = 1 ;
    /* HUAWEI HGW s48571 2008年4月8日" AU8D00456 按reset按钮3秒以上没有完全恢复到出厂配置 add end.*/

	printk("Restore default configuration and Reset the system!!!\n");
//    kerSysMipsSoftReset ();


    /*start of y42304 20070124 added: 解决按复位键无法恢复wlan led状态 . */
         #if defined (WIRELESS)      
            g_sWlanFlag = 2;      
            kerSysWakeupMonitorTask();   
         #else
            kerSysMipsSoftReset ();
         #endif
         /*end of y42304 20061201 added */

}




#ifdef CONFIG_WLAN_REVERSE_PRESS_RESET_KEY   
 
/* start of y42304 added 20060907: 支持通过按复位键10秒内开启/关闭wlan */
#define RESTORE_DEFAULT_SETTING_TIME  10
#define REVERSE_WLAN_STATE_TIME       3

static void bcm63xx_poll_reset_button(void)
{
	static int s_iResetButtoncnt = 0;
	int  iRet;
	if (FLASH_WRITING ==s_iFlashWriteFlg)
    {
        s_iResetButtoncnt = 0;
        poll_timer.expires = jiffies + POLLCNT_1SEC;
	    add_timer(&poll_timer);
        return;
    }
	
	iRet = kerSysGetResetStatus();
	
	if(iRet == 0) 
	{
		s_iResetButtoncnt++;
		if(s_iResetButtoncnt >= RESTORE_DEFAULT_SETTING_TIME) 
		{
            char cUpgradeFlags[FAILURE_TAG_LEN];
            
			printk("Restore default configuration and Reset the system!!!\n");

            g_iReset = 1; 

            kerSysPersistentClear();
            /* begin --- Add Persistent storage interface backup flash partition by w69233 */
            kerSysPsiBackupClear();
            /* end ----- Add Persistent storage interface backup flash partition by w69233 */
            kerSysScratchPadClearAll();

            /*start of y42304 20060727 added: 恢复出厂设置时清除升级标识 */
            /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify begin:
            iRet = kerSysVariableGet(cUpgradeFlags, FAILURE_TAG_LEN, 0);
            */
            iRet = kerSysAvailGet(cUpgradeFlags, FAILURE_TAG_LEN, 0);
            /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify end. */
            if ((-1 != iRet) && (NULL != strstr(cUpgradeFlags, FAILURE_TAG)) 
                && (0 != strcmp(cUpgradeFlags, "5aa5a55a0"))) 
            {
                /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify begin:
                kerSysVariableSet("5aa5a55a0", FAILURE_TAG_LEN, 0);
                */
                kerSysAvailSet("5aa5a55a0", FAILURE_TAG_LEN, 0);
                /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify end. */
            }     
            /*start of y42304 20060727 added: 恢复出厂设置时清除升级标识 */    

            
            /*start of y42304 20070124 added: 解决按复位键无法恢复wlan led状态 . */
           #if defined (WIRELESS)      
               g_sWlanFlag = 2;      
               kerSysWakeupMonitorTask();   
           #else
               kerSysMipsSoftReset ();
           #endif
		}
	} 
	else if (iRet > 0)
	{
        /* reset按键时长<3s 更改wlan开关状态 */
		if( s_iResetButtoncnt>=1 && s_iResetButtoncnt < REVERSE_WLAN_STATE_TIME ) 
		{            
            g_sWlanFlag = (0 == g_sWlanFlag)?1:0;
            kerSysWakeupMonitorTask();
		}
        s_iResetButtoncnt = 0;
    }

    g_iReset = 1;

    if(iRet >= 0)
    {
        poll_timer.expires = jiffies + POLLCNT_1SEC;
	    add_timer(&poll_timer);
    }
}

/* end of y42304 added 20060907 */
#else  

/*start of A36D02242 y42304 20060706: 将按reset键恢复出厂设置的时间由6秒改为3秒 */
/* HUAWEI HGW s48571 2008年3月28日 修改恢复出厂配置时间 modify begin:
#define RESTORE_DEFAULT_SETTING_TIME      3
*/
#define RESTORE_DEFAULT_SETTING_TIME      6
/* HUAWEI HGW s48571 2008年3月28日 修改恢复出厂配置时间 modify end. */
#define REVERSE_WLAN_STATE_TIME       3
/*end of A36D02242 y42304 20060706: */
static void bcm63xx_poll_reset_button(void)
{
	static int s_iResetButtoncnt = 0;
	int  iRet;
	static int iRestoreFlag = 0;
	
	if (FLASH_WRITING ==s_iFlashWriteFlg)
    {
        s_iResetButtoncnt = 0;
        poll_timer.expires = jiffies + POLLCNT_1SEC;
	    add_timer(&poll_timer);
        return;
    }
	
	iRet = kerSysGetResetStatus();
	
	if(iRet == 0) 
	{
		s_iResetButtoncnt++;
		if(s_iResetButtoncnt >= RESTORE_DEFAULT_SETTING_TIME) 
		{

	      if (!iRestoreFlag)
	      	{
	      		iRestoreFlag = 1;
            		//INIT_WORK(&board_work, restore_do_tasklet, NULL);
            		INIT_WORK(&board_work, restore_do_tasklet);
            		queue_work(board_workqueue, &board_work);
	      	}
            
            #if 0
            char cUpgradeFlags[FAILURE_TAG_LEN];
            printk("START to Restore default configuration, DO NOT POWER OFF!!!\n");
            /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify begin:
            kerSysVariableSet("resdeft", 8, 2400);
            */

            g_iReset = 0;
            
            kerSysAvailSet("resdeft", 8, 2400);
            /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify end. */
			/* start of maintain dying gasp by liuzhijie 00028714 2006年5月20日 */
            dg_storeDyingGaspInfo();
			/* end of maintain dying gasp by liuzhijie 00028714 2006年5月20日 */
            /* 由于写flash时进程调度可能会被暂停，所以开始写flash时需要禁止进程调度状态监控 */
            dg_setScheduleState(DG_DISABLE_SCHED_MON);
            kerSysPersistentClear();
            /* begin --- Add Persistent storage interface backup flash partition by w69233 */
            kerSysPsiBackupClear();
            /* end ----- Add Persistent storage interface backup flash partition by w69233 */
            kerSysScratchPadClearAll();

            /*start of y42304 20060727 added: 恢复出厂设置时清除升级标识 */
            /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify begin:
            iRet = kerSysVariableGet(cUpgradeFlags, FAILURE_TAG_LEN, 0);
            */
            iRet = kerSysAvailGet(cUpgradeFlags, FAILURE_TAG_LEN, 0);
            /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify end. */
            if ((-1 != iRet) && (NULL != strstr(cUpgradeFlags, FAILURE_TAG)) 
                && (0 != strcmp(cUpgradeFlags, "5aa5a55a0"))) 
            {
                /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify begin:
                kerSysVariableSet("5aa5a55a0", FAILURE_TAG_LEN, 0);
                */
                kerSysAvailSet("5aa5a55a0", FAILURE_TAG_LEN, 0);
                /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify end. */
            }     
            /*start of y42304 20060727 added: 恢复出厂设置时清除升级标识 */            

            /* HUAWEI HGW s48571 2008年4月8日" AU8D00456 按reset按钮3秒以上没有完全恢复到出厂配置 add begin:*/
            char pinTemp[IFC_PIN_PASSWORD_LEN];
            char *pcBuf;
            char tmp[8];

            pcBuf = (char *)kmalloc( CWMP_PARA_LEN, GFP_KERNEL );
             memset(pcBuf, 0x00, CWMP_PARA_LEN);
             memset(tmp, 0x00, sizeof(tmp));
             memset(pinTemp, 0xff, sizeof(pinTemp));
             kerSysAvailSet(pinTemp, IFC_PIN_PASSWORD_LEN, PINCODE_OFFSET);
             kerSysAvailSet("resdeft", RESTORE_FLAG_LEN, WLAN_PARAMETERS_OFFSET + WLAN_WEP_KEY_LEN + WLAN_WPA_PSK_LEN);
             kerSysAvailSet("resetok" , UPG_INFO_STATUS, (CFG_STATE_DATA_START + UPG_INFO_STATUS) );//cwmp reset flag
             kerSysAvailSet("restore", TELNET_TAG_LEN, FLASH_VAR_START + FAILURE_TAG_LEN );
             kerSysAvailSet("resping", PING_TAG_LEN, FLASH_VAR_START + FAILURE_TAG_LEN + TELNET_TAG_LEN );
             kerSysAvailSet(pcBuf, CWMP_PARA_LEN, CWMP_PARA_OFFSET);
             kerSysAvailSet("HGReset", CWMP_PARA_RESET, CWMP_PARA_OFFSET+CWMP_PARA_LEN);
             g_sVoipServiceStatus = 1 ;
            dg_setScheduleState(DG_ENABLE_SCHED_MON);
            /* HUAWEI HGW s48571 2008年4月8日" AU8D00456 按reset按钮3秒以上没有完全恢复到出厂配置 add end.*/
#endif

//			printk("Restore default configuration and Reset the system!!!\n");

		}
	} 
	else if (iRet > 0)
	{
		if( s_iResetButtoncnt>=1 && s_iResetButtoncnt<RESTORE_DEFAULT_SETTING_TIME ) 
		{
			printk("Reset the system!!!\n");
			kerSysMipsSoftReset ();
		}
	      s_iResetButtoncnt = 0;
        }

   if(iRet >= 0)
   {
	 poll_timer.expires = jiffies + POLLCNT_1SEC;
       add_timer(&poll_timer);
   }

}
 /*end of  GPIO 3.4.3 porting by l39225 20060504*/ 
#endif
 

void __init InitNvramInfo( void )
{
    PNVRAM_DATA pNvramData = (PNVRAM_DATA) get_nvram_start_addr();
    unsigned long ulNumMacAddrs = pNvramData->ulNumMacAddrs;

    if( ulNumMacAddrs > 0 && ulNumMacAddrs <= NVRAM_MAC_COUNT_MAX )
    {
        /*此处申请如此内存是保证4字节对齐*/
        unsigned long ulNvramInfoSize =
            sizeof(NVRAM_INFO) + ((sizeof(MAC_ADDR_INFO) - 1) * ulNumMacAddrs);

        g_pNvramInfo = (PNVRAM_INFO) kmalloc( ulNvramInfoSize, GFP_KERNEL );

        if( g_pNvramInfo )
        {
            unsigned long ulPsiSize;
            if( BpGetPsiSize( &ulPsiSize ) != BP_SUCCESS )
                ulPsiSize = NVRAM_PSI_DEFAULT;
            memset( g_pNvramInfo, 0x00, ulNvramInfoSize );
            g_pNvramInfo->ulPsiSize = ulPsiSize * 1024;
            g_pNvramInfo->ulNumMacAddrs = pNvramData->ulNumMacAddrs;
            memcpy( g_pNvramInfo->ucaBaseMacAddr, pNvramData->ucaBaseMacAddr,
                NVRAM_MAC_ADDRESS_LEN );
            g_pNvramInfo->ulSdramSize = getMemorySize();

            /* HUAWEI HGW s48571 2008年2月1日" VDF requirement: Fixed MAC add begin:*/
            int i = 0;
            unsigned long reserveId = 0;
            unsigned char acMacAddr[NVRAM_MAC_ADDRESS_LEN];
            memset(acMacAddr, 0x00,NVRAM_MAC_ADDRESS_LEN );
            for( i = 0 ; i < g_pNvramInfo->ulNumMacAddrs ; i++ )
            {
                switch(i)
                {
                    case 0:
                        reserveId = RESERVE_MAC_MASK;
                        kerSysGetMacAddress(acMacAddr, reserveId);
                        break;
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                        reserveId = RESERVE_MAC_MASK & (i << 24);
                        kerSysGetMacAddress(acMacAddr, reserveId);
                        break;
                    default:
                        break;
                }
            }
            /* HUAWEI HGW s48571 2008年2月1日" VDF requirement: Fixed MAC add end.*/
        }
        else
            printk("ERROR - Could not allocate memory for NVRAM data\n");
    }
    else
        printk("ERROR - Invalid number of MAC addresses (%ld) is configured.\n",
            ulNumMacAddrs);
}

void __exit brcm_board_cleanup( void )
{
    printk("brcm_board_cleanup()\n");
	
    if (board_major != -1) 
    {
       /*start of  GPIO 3.4.3 porting by l39225 20060504*/
      del_timer_sync(&poll_timer);
      /*end of  GPIO 3.4.3 porting by l39225 20060504*/      
#if defined (WIRELESS)    	
    	ses_board_deinit();
        del_timer_sync(&switch_wlan_timer);
#endif    	
        kerSysDeinitDyingGaspHandler();
        unregister_chrdev(board_major, "board_ioctl");
    }
} 

static BOARD_IOC* borad_ioc_alloc(void)
{
    BOARD_IOC *board_ioc =NULL;
    board_ioc = (BOARD_IOC*) kmalloc( sizeof(BOARD_IOC) , GFP_KERNEL );
    if(board_ioc)
    {
        memset(board_ioc, 0, sizeof(BOARD_IOC));
    }
    return board_ioc;
}

static void borad_ioc_free(BOARD_IOC* board_ioc)
{
    if(board_ioc)
    {
        kfree(board_ioc);
    }	
}


static int board_open( struct inode *inode, struct file *filp )
{
    filp->private_data = borad_ioc_alloc();

    if (filp->private_data == NULL)
        return -ENOMEM;
            
    return( 0 );
} 

static int board_release(struct inode *inode, struct file *filp)
{
    BOARD_IOC *board_ioc = filp->private_data;
    
    wait_event_interruptible(g_board_wait_queue, 1);    
    borad_ioc_free(board_ioc);

    return( 0 );
} 


static unsigned int board_poll(struct file *filp, struct poll_table_struct *wait)
{
    unsigned int mask = 0;
#if defined (WIRELESS)        	
    BOARD_IOC *board_ioc = filp->private_data;    	
#endif
    	
    poll_wait(filp, &g_board_wait_queue, wait);
#if defined (WIRELESS)        	
    if(board_ioc->eventmask & SES_EVENTS){
	if(WPS_FLAG)
      {
          printk("board_poll is woke up\n\r"); 
        mask |= sesBtn_poll(filp, wait);
    	}
    }			
#endif    

    return mask;
}


static ssize_t board_read(struct file *filp,  char __user *buffer, size_t count, loff_t *ppos)
{
#if defined (WIRELESS)    
    ssize_t ret=0;
    BOARD_IOC *board_ioc = filp->private_data;
    if(board_ioc->eventmask & SES_EVENTS){
	 ret=sesBtn_read(filp, buffer, count, ppos);
	 WPS_FLAG=0;
	 return ret;
    }
#endif    
    return 0;
}

//**************************************************************************************
// Utitlities for dump memory, free kernel pages, mips soft reset, etc.
//**************************************************************************************

/***********************************************************************
 * Function Name: dumpaddr
 * Description  : Display a hex dump of the specified address.
 ***********************************************************************/
void dumpaddr( unsigned char *pAddr, int nLen )
{
    static char szHexChars[] = "0123456789abcdef";
    char szLine[80];
    char *p = szLine;
    unsigned char ch, *q;
    int i, j;
    unsigned long ul;

    while( nLen > 0 )
    {
        sprintf( szLine, "%8.8lx: ", (unsigned long) pAddr );
        p = szLine + strlen(szLine);

        for(i = 0; i < 16 && nLen > 0; i += sizeof(long), nLen -= sizeof(long))
        {
            ul = *(unsigned long *) &pAddr[i];
            q = (unsigned char *) &ul;
            for( j = 0; j < sizeof(long); j++ )
            {
                *p++ = szHexChars[q[j] >> 4];
                *p++ = szHexChars[q[j] & 0x0f];
                *p++ = ' ';
            }
        }

        for( j = 0; j < 16 - i; j++ )
            *p++ = ' ', *p++ = ' ', *p++ = ' ';

        *p++ = ' ', *p++ = ' ', *p++ = ' ';

        for( j = 0; j < i; j++ )
        {
            ch = pAddr[j];
            *p++ = (ch > ' ' && ch < '~') ? ch : '.';
        }

        *p++ = '\0';
        printk( "%s\r\n", szLine );

        pAddr += i;
    }
    printk( "\r\n" );
} /* dumpaddr */


void kerSysMipsSoftReset(void)
{
#if defined(CONFIG_BCM96348)
    if (PERF->RevID == 0x634800A1) {
        typedef void (*FNPTR) (void);
        FNPTR bootaddr = (FNPTR) FLASH_BASE;
        int i;

        /* Disable interrupts. */
        cli();

        /* Reset all blocks. */
        PERF->BlockSoftReset &= ~BSR_ALL_BLOCKS;
        for( i = 0; i < 1000000; i++ )
            ;
        PERF->BlockSoftReset |= BSR_ALL_BLOCKS;
        /* Jump to the power on address. */
        (*bootaddr) ();
    }
    else
        PERF->pll_control |= SOFT_RESET;    // soft reset mips
#else
    PERF->pll_control |= SOFT_RESET;    // soft reset mips
#endif
}

#if 0
int kerSysGetMacAddress( unsigned char *pucaMacAddr, unsigned long ulId )
{
    const unsigned long constMacAddrIncIndex = 3;
    int nRet = 0;
    PMAC_ADDR_INFO pMai = NULL;
    PMAC_ADDR_INFO pMaiFreeNoId = NULL;
    PMAC_ADDR_INFO pMaiFreeId = NULL;
    unsigned long i = 0, ulIdxNoId = 0, ulIdxId = 0, baseMacAddr = 0;

    /* baseMacAddr = last 3 bytes of the base MAC address treated as a 24 bit integer */
    memcpy((unsigned char *) &baseMacAddr,
        &g_pNvramInfo->ucaBaseMacAddr[constMacAddrIncIndex],
        NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex);
    baseMacAddr >>= 8;

    for( i = 0, pMai = g_pNvramInfo->MacAddrs; i < g_pNvramInfo->ulNumMacAddrs;
        i++, pMai++ )
    {
        if( ulId == pMai->ulId || ulId == MAC_ADDRESS_ANY )
        {
            /* This MAC address has been used by the caller in the past. */
            baseMacAddr = (baseMacAddr + i) << 8;
            memcpy( pucaMacAddr, g_pNvramInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
            pMai->chInUse = 1;
            pMaiFreeNoId = pMaiFreeId = NULL;
            break;
        }
        else
            if( pMai->chInUse == 0 )
            {
                if( pMai->ulId == 0 && pMaiFreeNoId == NULL )
                {
                    /* This is an available MAC address that has never been
                     * used.
                     */
                    pMaiFreeNoId = pMai;
                    ulIdxNoId = i;
                }
                else
                    if( pMai->ulId != 0 && pMaiFreeId == NULL )
                    {
                        /* This is an available MAC address that has been used
                         * before.  Use addresses that have never been used
                         * first, before using this one.
                         */
                        pMaiFreeId = pMai;
                        ulIdxId = i;
                    }
            }
    }

    if( pMaiFreeNoId || pMaiFreeId )
    {
        /* An available MAC address was found. */
        memcpy(pucaMacAddr, g_pNvramInfo->ucaBaseMacAddr,NVRAM_MAC_ADDRESS_LEN);
        if( pMaiFreeNoId )
        {
            baseMacAddr = (baseMacAddr + ulIdxNoId) << 8;
            memcpy( pucaMacAddr, g_pNvramInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
            pMaiFreeNoId->ulId = ulId;
            pMaiFreeNoId->chInUse = 1;
        }
        else
        {
            baseMacAddr = (baseMacAddr + ulIdxId) << 8;
            memcpy( pucaMacAddr, g_pNvramInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
            pMaiFreeId->ulId = ulId;
            pMaiFreeId->chInUse = 1;
        }
    }
    else
        if( i == g_pNvramInfo->ulNumMacAddrs )
            nRet = -EADDRNOTAVAIL;

    return( nRet );
} /* kerSysGetMacAddr */
#else
int kerSysGetMacAddress( unsigned char *pucaMacAddr, unsigned long ulId )
{
    const unsigned long constMacAddrIncIndex = 3;
    int nRet = 0;
    PMAC_ADDR_INFO pMai = NULL;
    PMAC_ADDR_INFO pMaiFreeNoId = NULL;
    PMAC_ADDR_INFO pMaiFreeId = NULL;
    unsigned long i = 0, ulIdxNoId = 0, ulIdxId = 0, baseMacAddr = 0;

    unsigned long  reservedId = ulId & RESERVE_MAC_MASK ;
//start of modify by y68191
/* 
    if ( reservedId > 0 )
    {
        if( RESERVE_MAC_MASK == reservedId )
        {
            reservedId = 0;
        }
        else
        {
            reservedId = reservedId >> 24;
        }
    }
    else 
    {
        reservedId = NO_RESERVE_MAC;
    }
   */
    if( RESERVE_MAC_MASK == reservedId )
    {
        reservedId = 0;
    }
    else
    {
        reservedId = reservedId >> 24;
	  if ((reservedId != 0x02)&&(reservedId != 0x03)&&(reservedId != 0x04)&&(reservedId != 0x05))
    	  {
             reservedId = NO_RESERVE_MAC;
    	  }
    }
// end of modify by y68191
    /* baseMacAddr = last 3 bytes of the base MAC address treated as a 24 bit integer */
    memcpy((unsigned char *) &baseMacAddr,
        &g_pNvramInfo->ucaBaseMacAddr[constMacAddrIncIndex],
        NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex);
    baseMacAddr >>= 8;

    for( i = 0, pMai = g_pNvramInfo->MacAddrs; i < g_pNvramInfo->ulNumMacAddrs;
        i++, pMai++ )
    {
        if( ulId == pMai->ulId || ulId == MAC_ADDRESS_ANY )
        {
            /* This MAC address has been used by the caller in the past. */
            baseMacAddr = (baseMacAddr + i) << 8;
            memcpy( pucaMacAddr, g_pNvramInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
            pMai->chInUse = 1;
            pMaiFreeNoId = pMaiFreeId = NULL;
            break;
        }
        else
        {
            if( pMai->chInUse == 0 )
            {
                if( pMai->ulId == 0 && pMaiFreeNoId == NULL  )
                {
                    /* This is an available MAC address that has never been
                     * used.
                     */
                    if( NO_RESERVE_MAC == reservedId )
                    {
                        pMaiFreeNoId = pMai;
                        ulIdxNoId = i;
                    }
                    else if (i == reservedId)//d00104343 Ricky 预留地址和当前序号必须相同
                    {
                        pMaiFreeNoId = pMai;
                        ulIdxNoId = reservedId;
                    }
                }
                else
                    if( pMai->ulId != 0 && pMaiFreeId == NULL )
                    {
                        /* This is an available MAC address that has been used
                         * before.  Use addresses that have never been used
                         * first, before using this one.
                         */
                            if( NO_RESERVE_MAC == reservedId )
                            {
                                pMaiFreeId = pMai;
                                ulIdxId = i;
                            }
                            else if (i == reservedId)
                            {
                                pMaiFreeId = pMai;
                                ulIdxId = reservedId;
                            }
                    }
            }
        }
    }

    if( pMaiFreeNoId || pMaiFreeId )
    {
        /* An available MAC address was found. */
        memcpy(pucaMacAddr, g_pNvramInfo->ucaBaseMacAddr,NVRAM_MAC_ADDRESS_LEN);
        if( pMaiFreeNoId )
        {
            baseMacAddr = (baseMacAddr + ulIdxNoId) << 8;
            memcpy( pucaMacAddr, g_pNvramInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
            pMaiFreeNoId->ulId = ulId;
            pMaiFreeNoId->chInUse = 1;
        }
        else
        {
            baseMacAddr = (baseMacAddr + ulIdxId) << 8;
            memcpy( pucaMacAddr, g_pNvramInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
            pMaiFreeId->ulId = ulId;
            pMaiFreeId->chInUse = 1;
        }
    }
    else
        if( i == g_pNvramInfo->ulNumMacAddrs )
            nRet = -EADDRNOTAVAIL;

    return( nRet );
} /* kerSysGetMacAddr */

#endif

int kerSysReleaseMacAddress( unsigned char *pucaMacAddr )
{
    const unsigned long constMacAddrIncIndex = 3;
    int nRet = -EINVAL;
    unsigned long ulIdx = 0;
    unsigned long baseMacAddr = 0;
    unsigned long relMacAddr = 0;

    /* baseMacAddr = last 3 bytes of the base MAC address treated as a 24 bit integer */
    memcpy((unsigned char *) &baseMacAddr,
        &g_pNvramInfo->ucaBaseMacAddr[constMacAddrIncIndex],
        NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex);
    baseMacAddr >>= 8;

    /* Get last 3 bytes of MAC address to release. */
    memcpy((unsigned char *) &relMacAddr, &pucaMacAddr[constMacAddrIncIndex],
        NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex);
    relMacAddr >>= 8;

    ulIdx = relMacAddr - baseMacAddr;

    if( ulIdx < g_pNvramInfo->ulNumMacAddrs )
    {
        PMAC_ADDR_INFO pMai = &g_pNvramInfo->MacAddrs[ulIdx];
        if( pMai->chInUse == 1 )
        {
            pMai->chInUse = 0;
            nRet = 0;
        }
    }

    return( nRet );
} /* kerSysReleaseMacAddr */

int kerSysGetSdramSize( void )
{
    return( (int) g_pNvramInfo->ulSdramSize );
} /* kerSysGetSdramSize */


void kerSysLedCtrl(BOARD_LED_NAME ledName, BOARD_LED_STATE ledState)
{
/* HUAWEI HGW s48571 2008年1月18日 Hardware Porting delete begin:*/
#if 0
     /* start of y42304 added 20061128: 处理ADSL单色LED */
     if (ledName == kLedAdsl)
     {
         if (ledState == kLedStateFail)
         {
             ledState = kLedStateOff;
         }
     }    
     /* end of y42304 added 20061128*/
#endif    
/* HUAWEI HGW s48571 2008年1月18日 Hardware Porting delete end.*/

    if (g_ledInitialized)
      boardLedCtrl(ledName, ledState);
}

/**************************************************************************
 * 函数名  :   kerSysGetGPIO
 * 功能    :   读取给定GPIO的值
 *
 * 输入参数:   ucGpioNum:  GPIO号
 * 输出参数:   无
 *
 * 返回值  :    正确  :  GPIO值，高电平或低电平
 *
 * 作者    :    yuyouqing42304
 * 修改历史:    2006-05-16创建  
 ***************************************************************************/
unsigned char kerSysGetGPIO(unsigned short ucGpioNum)
{  
    unsigned char ucGpioValue = 0;

    ucGpioNum = (ucGpioNum & BP_GPIO_NUM_MASK);
    
#if !defined (CONFIG_BCM96338)
    /* 6348: GPIO32-GPIO36
     * 6358: GPIO32-GPIO37 */
    if (ucGpioNum >= 32)
    {
        ucGpioNum = ucGpioNum - 32;
    	ucGpioValue = GPIO->GPIOio_high;        
    }
    else
#endif
    {
    	ucGpioValue = GPIO->GPIOio;        
    }
    ucGpioValue = (ucGpioValue >> ucGpioNum) & 1;
    return ucGpioValue; 
}

/**************************************************************************
 * 函数名  :   kerSysGetBoardVersion
 * 功能    :   GPIO获取Board版本号(提供给内核态调用)
 *
 * 输入参数:   无           
 * 输出参数:   无
 *
 * 返回值  :    正确  :  Board版本号 
 *              错误  :  NOT_SUPPROT_GET_VERSION
 * 作者    :    yuyouqing42304
 * 修改历史:    2006-05-16创建  
     
 * GPIO8 GPIO7  GPIO6
 *   0     0     0   -->    HG52VAG                         
 *   0     0     1   -->    HG52VAGB
 *   1     0     0   -->    HG51VAG
 *   1     1     0   -->    HG51VAGI                          
 ***************************************************************************/
unsigned char kerSysGetBoardVersion(void)
{    
    int i = 0;
    int iRet = 0;      
    unsigned short usGPIOValue;
    int iBoardVerNum = 0, iTmpVersion = 0;  
    //char pBoardVersion[] = {' ','B','C','D','E', 'F', 'G', 'H', 'Z'};    

    /* 判断硬件是否支持通过读取GPIO获取Board version*/
    if(BP_SUCCESS != BpIsSupportBoardVersion())
    {
        return NOT_SUPPROT_GET_VERSION;
    }

    for (i = 0; i < BOARD_VERSION_GPIO_NUMS; i++)
    {        
        iRet = BpGetBoardVesionGpio(i, &usGPIOValue);
        if (BP_SUCCESS == iRet)
        {
            iTmpVersion = kerSysGetGPIO(usGPIOValue);
            iBoardVerNum = ((iTmpVersion << i) | iBoardVerNum);                
        }
        else
        {
            return NOT_SUPPROT_GET_VERSION;
        }
    }    
    return iBoardVerNum;        
}

/**************************************************************************
 * 函数名  :   kerSysGetBoardManufacturer
 * 功能    :   GPIO获取单板的制造商
 *
 * 输入参数:   无           
 * 输出参数:   无
 *
 * 返回值  :    单板制造商号 
 * 作者    :    yuyouqing42304
 * 修改历史:    2006-08-10创建  

 * GPIO35  GPIO33
 *   1      1        -->    Huawei
 *   1      0        -->    Alpha  
 ***************************************************************************/
unsigned char kerSysGetBoardManufacturer(void)
{
    unsigned char ucTmpManufactuer = 0;

    ucTmpManufactuer = kerSysGetGPIO(BP_GPIO_35_AH);
    ucTmpManufactuer = ((ucTmpManufactuer << 1) | kerSysGetGPIO(BP_GPIO_33_AH));    

    return  ucTmpManufactuer;
}


/**************************************************************************
 * 函数名  :   kerSysGetPCBVersion
 * 功能    :   GPIO获取PCB版本号(提供给内核态调用)
 *
 * 输入参数:   无           
 * 输出参数:   无
 *
 * 返回值  :    正确  :  PCB版本号 
 *              错误  :  NOT_SUPPROT_GET_VERSION
 * 作者    :    yuyouqing42304
 * 修改历史:    2006-05-16创建  
 ***************************************************************************/
unsigned char kerSysGetPCBVersion(void)
{    
    int i = 0;
    int iRet = 0;      
    unsigned short usGPIOValue;
    int iPCBVerNum = 0, iTmpVersion = 0;        
    char pPCBVersion[] = {'A','B','C','D','E','F', 'G','0'};
    
    /* 判断硬件是否支持通过读取GPIO获取Board version*/
    if(BP_SUCCESS != BpIsSupportBoardVersion())
    {
        printk("\r\n Not support get board version.\n");
        return NOT_SUPPROT_GET_VERSION;
    }

    for (i = 0; i < PCB_VERSION_GPIO_NUMS; i++)
    {        
        iRet = BpGetPCBVesionGpio(i, &usGPIOValue);
        if (BP_SUCCESS == iRet)
        {
            iTmpVersion = kerSysGetGPIO(usGPIOValue);
            iPCBVerNum = ((iTmpVersion << i) | iPCBVerNum);                
        }
        else
        {
            return NOT_SUPPROT_GET_VERSION;
        }
    }
    return (char)pPCBVersion[iPCBVerNum];       
}

/*Read Wlan Params data from CFE */
int kerSysGetWlanSromParams( unsigned char *wlanParams, unsigned short len)
{
    memset(wlanParams, 0, len);
    return 0;
}
EXPORT_SYMBOL(kerSysGetWlanSromParams);

unsigned int kerSysMonitorPollHook( struct file *f, struct poll_table_struct *t)
{
    int mask = (*g_orig_fop_poll) (f, t);

    if( g_wakeup_monitor == 1 && g_monitor_file == f )
    {
        /* If g_wakeup_monitor is non-0, the user mode application needs to
         * return from a blocking select function.  Return POLLPRI which will
         * cause the select to return with the exception descriptor set.
         */
        mask |= POLLPRI;
        g_wakeup_monitor = 0;
    }

    return( mask );
}

/* Put the user mode application that monitors link state on a run queue. */
void kerSysWakeupMonitorTask( void )
{
    g_wakeup_monitor = 1;
   // printk(KERN_NOTICE "monitor waked up by \"%s\" (pid %i)\n", current->comm, current->pid);
    if( g_monitor_task )
        wake_up_process( g_monitor_task );
}

PFILE_TAG getTagFromPartition(int imageNumber)
{
    static unsigned char sectAddr1[sizeof(FILE_TAG)];
    static unsigned char sectAddr2[sizeof(FILE_TAG)];
    int blk = 0;
    UINT32 crc;
    PFILE_TAG pTag = NULL;
    unsigned char *pBase = flash_get_memptr(0);
    unsigned char *pSectAddr = NULL;

    /* The image tag for the first image is always after the boot loader.
     * The image tag for the second image, if it exists, is at one half
     * of the flash size.
     */
    if( imageNumber == 1 )
    {
        blk = flash_get_blk((int) (pBase + FLASH_LENGTH_BOOT_ROM));
        pSectAddr = sectAddr1;
    }
    else
        if( imageNumber == 2 )
        {

/*Start -- Normal System & Small System -20091201*/
#ifdef SUPPORT_TMP_FIRMWARE
            blk = flash_get_blk((int) (pBase + (flash_get_total_size() / 2)));
#else
			blk = flash_get_blk((int) (pBase + FLASH_LENGTH_BOOT_ROM + FLASH_LENGTH_NORMAL_SYSTEM));	
#endif
/*End -- Normal System & Small System -20091201*/

			pSectAddr = sectAddr2;
        }

    if( blk )
    {
        memset(pSectAddr, 0x00, sizeof(FILE_TAG));
        flash_read_buf((unsigned short) blk, 0, pSectAddr, sizeof(FILE_TAG));
        crc = CRC32_INIT_VALUE;
        crc = getCrc32(pSectAddr, (UINT32)TAG_LEN-TOKEN_LEN, crc);      
        pTag = (PFILE_TAG) pSectAddr;
        if (crc != (UINT32)(*(UINT32*)(pTag->tagValidationToken)))
            pTag = NULL;
    }

    return( pTag );
}

int getPartitionFromTag( PFILE_TAG pTag )
{
    int ret = 0;

    if( pTag )
    {
        PFILE_TAG pTag1 = getTagFromPartition(1);
        PFILE_TAG pTag2 = getTagFromPartition(2);
        int sequence = simple_strtoul(pTag->imageSequence,  NULL, 10);
        int sequence1 = (pTag1) ? simple_strtoul(pTag1->imageSequence, NULL, 10)
            : -1;
        int sequence2 = (pTag2) ? simple_strtoul(pTag2->imageSequence, NULL, 10)
            : -1;

        if( pTag1 && sequence == sequence1 )
            ret = 1;
        else
            if( pTag2 && sequence == sequence2 )
                ret = 2;
    }

    return( ret );
}

static void UpdateImageSequenceNumber( unsigned char *imageSequence )
{
    int newImageSequence = 0;
    PFILE_TAG pTag = getTagFromPartition(1);

    if( pTag )
        newImageSequence = simple_strtoul(pTag->imageSequence, NULL, 10);

    pTag = getTagFromPartition(2);
    if(pTag && simple_strtoul(pTag->imageSequence, NULL, 10) > newImageSequence)
        newImageSequence = simple_strtoul(pTag->imageSequence, NULL, 10);

    newImageSequence++;
    sprintf(imageSequence, "%d", newImageSequence);
}



/*Start -- Normal System & Small System -20091121*/
void CheckTempFW(PFILE_TAG pTag)
{
	int tagVer, typeFlag;

	printk("Enter CheckTempFW....\n");
	
	if(NULL == pTag)
	{
		return;
	}

    tagVer = simple_strtoul(pTag->tagVersion, NULL, 10);
    typeFlag = simple_strtoul(pTag->reserved, NULL, 10);

	if(TAGVERSION_HW_OLD == tagVer && TAGVERSION_FLAG_TEMP_FW == typeFlag)
	{
		printk("This is a temporary firmware, need copy to partiton 1....\n");
		unsigned int baseAddr = (unsigned int) flash_get_memptr(0);
		int rootfsAddr = simple_strtoul(pTag->rootfsAddress, NULL, 10) + BOOT_OFFSET;
		int rootfsAddr_backup = rootfsAddr;
		int kernelAddr = simple_strtoul(pTag->kernelAddress, NULL, 10) + BOOT_OFFSET;
		unsigned int newImgSize = simple_strtoul(pTag->rootfsLen, NULL, 10) +
			simple_strtoul(pTag->kernelLen, NULL, 10);
		int offset = FLASH_LENGTH_BOOT_ROM;//for CEF
		UINT32 crc = CRC32_INIT_VALUE;
		unsigned int ulSectorSize = 0;
		int fsBlk = 0;
		int status = 0;
		char* pSectAddr = NULL;
		char* pSrcAddr = NULL;
		char* pDstAddr = NULL;

		/*pTag在调用UpdateImageSequenceNumber时会从flash中重新读取，
		导致以下对kerneladdress和rootfsaddress的修改丢失*/
		unsigned char tagAddr[sizeof(FILE_TAG)];

		memcpy(tagAddr, pTag, sizeof(FILE_TAG));


		sprintf(((PFILE_TAG)tagAddr)->kernelAddress, "%lu",
			(unsigned long) IMAGE_BASE + offset + (kernelAddr - rootfsAddr) + TAG_LEN);
		kernelAddr = baseAddr + offset + (kernelAddr - rootfsAddr) + TAG_LEN;

		sprintf(((PFILE_TAG)tagAddr)->rootfsAddress, "%lu",
			(unsigned long) IMAGE_BASE + offset + TAG_LEN);
		rootfsAddr = baseAddr + offset + TAG_LEN;

	    UpdateImageSequenceNumber( ((PFILE_TAG)tagAddr)->imageSequence );
	    crc = getCrc32(tagAddr, (UINT32)TAG_LEN-TOKEN_LEN, crc);      
	    *(unsigned long *) &((PFILE_TAG)tagAddr)->tagValidationToken[0] = crc;


	    /*修改写FLASH顺序，最后烧写包含TAG的一块*/
	    fsBlk = flash_get_blk(rootfsAddr-TAG_LEN);
	    ulSectorSize = flash_get_sector_size(fsBlk);

		printk("====baseAddr:%x====\n",(baseAddr));
		printk("====base block:%d====\n",flash_get_blk(baseAddr));

		printk("====rootfsAddr_backup:%x====\n",(rootfsAddr_backup));

		printk("====rootfsAddr:%x====\n",(rootfsAddr));
		printk("====kernelAddr:%x====\n",(kernelAddr));

		printk("====ptag->rootfsAddr:%x====\n",simple_strtoul(((PFILE_TAG)tagAddr)->rootfsAddress, NULL, 10) + BOOT_OFFSET);
		printk("====ptag->kernelAddr:%x====\n",simple_strtoul(((PFILE_TAG)tagAddr)->kernelAddress, NULL, 10) + BOOT_OFFSET);



		printk("====flash_start_addr:%x====\n",(rootfsAddr-TAG_LEN+ulSectorSize));

		printk("====flash_start_addr block:%d====\n",flash_get_blk(rootfsAddr-TAG_LEN+ulSectorSize));
		
		printk("====string addr:%x====\n",(rootfsAddr_backup-TAG_LEN+ulSectorSize));
		printk("====copy from block:%d====\n",flash_get_blk(rootfsAddr_backup-TAG_LEN+ulSectorSize));



		pSectAddr = retriedKmalloc(ulSectorSize);
		
		if (NULL == pSectAddr)
		{
			printk("~~~~~~~~~~Not enough memory.\r\n");
			kerSysMipsSoftReset();
			return;
		}

		pSrcAddr = rootfsAddr_backup-TAG_LEN+ulSectorSize;
		pDstAddr = rootfsAddr-TAG_LEN+ulSectorSize;
		
		int totalSize = TAG_LEN + newImgSize - ulSectorSize;

		do
		{
			kerSysReadFromFlash((void *)pSectAddr, pSrcAddr, ulSectorSize);
		    if( (status = kerSysBcmImageSet(pDstAddr, pSectAddr, ulSectorSize)) != 0 )
		    {
		        printk("Failed to copy Temporary Firmware from Partition 2 to Partition 1. Error: %d\n", status);
				kerSysMipsSoftReset();
				return;
		    }
			pSrcAddr += ulSectorSize;
			pDstAddr += ulSectorSize;
			totalSize -= ulSectorSize;
		}while( totalSize > 0 );


		kerSysReadFromFlash((void *)pSectAddr, rootfsAddr_backup-TAG_LEN, ulSectorSize);		

		memcpy(pSectAddr, tagAddr, TAG_LEN);
		
	    /*烧写包含TAG的一块*/
	    if( (status = kerSysBcmImageSet((rootfsAddr-TAG_LEN), pSectAddr, ulSectorSize)) != 0 )
	    {
	        printk("Failed to copy Temporary Firmware from Partition 2 to Partition 1. Error: %d\n", status);
			kerSysMipsSoftReset();
			return;
	    }
		
	    if (pSectAddr)
        {
            kfree(pSectAddr);
        }
		
		printk("Copy Temporary Firmware from Partition 2 to Partition 1 success!!!\n");
		kerSysMipsSoftReset();
		return;

		
	}
}

int CheckSystemCrc(PFILE_TAG pTag)
{
	int ret = -1;

	printk("\r\nCheck system ImageCrc ...\n");

	if(pTag == NULL)
	{
		return ret;
	}

	unsigned long ulFlashAddr = 0;
	int nImgLen = 0;
	unsigned long ulCrc, ulImgCrc;

	/* Check Linux file system CRC */
	ulImgCrc = *(unsigned long *) (pTag->imageValidationToken+CRC_LEN);

	printk("---DEBUG---%s,%d,ulImgCrc:%x\n",__func__,__LINE__,ulImgCrc);

	
	if( ulImgCrc )
	{
		ulFlashAddr = board_atoi(pTag->rootfsAddress) + BOOT_OFFSET;
		nImgLen = simple_strtoul(pTag->rootfsLen, NULL, 10);

		//printk("---DEBUG---%s,%d,ulFlashAddr:%x\n",__func__,__LINE__,ulFlashAddr);
		//printk("---DEBUG---%s,%d,nImgLen:%d\n",__func__,__LINE__,nImgLen);

		ulCrc = CRC32_INIT_VALUE;
		ulCrc = getCrc32((char *)ulFlashAddr, (UINT32) nImgLen, ulCrc); 	

		//printk("---DEBUG---%s,%d,ulCrc:%x\n",__func__,__LINE__,ulCrc);

		if( ulCrc != ulImgCrc)
		{
			printk("Linux file system CRC error.  Corrupted image?\n");
			ret = -1;
			return ret;
		}
	}

	/* Check Linux kernel CRC */
	ulImgCrc = *(unsigned long *)(pTag->imageValidationToken+(CRC_LEN*2));
	if( ulImgCrc )
	{
		ulFlashAddr = board_atoi(pTag->kernelAddress) + BOOT_OFFSET;
		nImgLen = simple_strtoul(pTag->kernelLen, NULL, 10);

		ulCrc = CRC32_INIT_VALUE;


		//printk("---DEBUG---%s,%d,ulFlashAddr:%x\n",__func__,__LINE__,ulFlashAddr);
		//printk("---DEBUG---%s,%d,nImgLen:%d\n",__func__,__LINE__,nImgLen);
		
		ulCrc = getCrc32((char *)ulFlashAddr, (UINT32) nImgLen, ulCrc); 	 

		
		if( ulCrc != ulImgCrc)
		{
			printk("Linux kernel CRC error.  Corrupted image?\n");
			ret = -1;
			return ret;
		}		
	}

	return 0;

}
/*End -- Normal System & Small System -20091121*/

/*Start -- Normal System & Small System -20091201*/
#ifdef	SUPPORT_TMP_FIRMWARE
static PFILE_TAG getBootImageTag(void)
{
    PFILE_TAG pTag = NULL;
    PFILE_TAG pTag1 = getTagFromPartition(1);
    PFILE_TAG pTag2 = getTagFromPartition(2);

    if( pTag1 && pTag2 )
    {
        /* Two images are flashed. */
        int sequence1 = simple_strtoul(pTag1->imageSequence, NULL, 10);
        int sequence2 = simple_strtoul(pTag2->imageSequence, NULL, 10);
        char *p;
        char bootPartition = BOOT_LATEST_IMAGE;
        NVRAM_DATA nvramData;

        memcpy((char *) &nvramData, (char *) get_nvram_start_addr(),
            sizeof(nvramData));
        for( p = nvramData.szBootline; p[2] != '\0'; p++ )
            if( p[0] == 'p' && p[1] == '=' )
            {
                bootPartition = p[2];
                break;
            }

        if( bootPartition == BOOT_LATEST_IMAGE )
            pTag = (sequence2 > sequence1) ? pTag2 : pTag1;
        else /* Boot from the image configured. */
            pTag = (sequence2 < sequence1) ? pTag2 : pTag1;

		/*Start -- Normal System & Small System -20091121*/
		int ret = 0;
		ret = CheckSystemCrc(pTag);

		if(0 != ret)
		{			
			pTag = (pTag == pTag1) ? pTag2 : pTag1;

			printk("Image CRC error.  Exchange to the other partition.\n");
			printk("boot from kernel address:0x%x...\n", board_atoi(pTag->kernelAddress) + BOOT_OFFSET);

			
		}
		else
		{
			printk("Image CRC success.\n");
		}
		/*End -- Normal System & Small System -20091121*/


		
    }
    else
        /* One image is flashed. */
        pTag = (pTag2) ? pTag2 : pTag1;

	/*Start -- Normal System & Small System -20091121*/
	if(pTag == pTag1)
	{
		printk("boot from system in Partition 1...\n");
	}
	else if(pTag == pTag2)
	{
		printk("boot from system in Partition 2...\n");
		CheckTempFW(pTag);
	}
	/*End -- Normal System & Small System -20091121*/

    return( pTag );
}
#else
static PFILE_TAG getBootImageTag(void)
{
    PFILE_TAG pTag = NULL;
    PFILE_TAG pTag1 = getTagFromPartition(1);
    PFILE_TAG pTag2 = getTagFromPartition(2);

	if( pTag1 && pTag2 )
	{
		int ret = 0;
		ret = CheckSystemCrc(pTag1);

		if(0 != ret)
		{			
			printk("Normal system Image CRC error.  Exchange to Small system.\n");
			printk("boot from kernel address:0x%x...\n", board_atoi(pTag2->kernelAddress) + BOOT_OFFSET);
			pTag = pTag2;
		}
		else
		{
			printk("Normal system Image CRC success, boot from Normal system.\n");
			pTag = pTag1;
		}
	}
	else
	{
		/* One image is flashed. */
        pTag = (pTag2) ? pTag2 : pTag1;
	}
    return( pTag );

}
#endif
/*End -- Normal System & Small System -20091201*/


/*Start -- Normal System & Small System -20091201*/
#if	0
static int flashFsKernelImage( int destAddr, unsigned char *imagePtr,
    int imageLen )
{
    int status = 0;
    PFILE_TAG pTag = (PFILE_TAG) imagePtr;
    int rootfsAddr = simple_strtoul(pTag->rootfsAddress, NULL, 10) + BOOT_OFFSET;
    int kernelAddr = simple_strtoul(pTag->kernelAddress, NULL, 10) + BOOT_OFFSET;
    char *p;
    char *tagFs = imagePtr;
    unsigned int baseAddr = (unsigned int) flash_get_memptr(0);
    unsigned int totalSize = (unsigned int) flash_get_total_size();
    unsigned int availableSizeOneImg = totalSize -
        ((unsigned int) rootfsAddr - baseAddr) - FLASH_RESERVED_AT_END;
    unsigned int reserveForTwoImages =
        (FLASH_LENGTH_BOOT_ROM > FLASH_RESERVED_AT_END)
        ? FLASH_LENGTH_BOOT_ROM : FLASH_RESERVED_AT_END;
    unsigned int availableSizeTwoImgs =
        (totalSize / 2) - reserveForTwoImages;
    unsigned int newImgSize = simple_strtoul(pTag->rootfsLen, NULL, 10) +
        simple_strtoul(pTag->kernelLen, NULL, 10);
    PFILE_TAG pCurTag = getBootImageTag();
    UINT32 crc = CRC32_INIT_VALUE;
    unsigned int curImgSize = 0;
    NVRAM_DATA nvramData;
    unsigned int ulSectorSize = 0;
    int fsBlk = 0;
    char acFlag[8];
    int  lFlag = 0;

    memset(acFlag, 0, sizeof(acFlag));
    strcpy(acFlag, "3");

    memcpy((char *)&nvramData, (char *)get_nvram_start_addr(),sizeof(nvramData));

    if( pCurTag )
    {
        curImgSize = simple_strtoul(pCurTag->rootfsLen, NULL, 10) +
            simple_strtoul(pCurTag->kernelLen, NULL, 10);
    }

    if( newImgSize > availableSizeOneImg)
    {
        printk("Illegal image size %d.  Image size must not be greater "
            "than %d.\n", newImgSize, availableSizeOneImg);
        return -1;
    }

    // If the current image fits in half the flash space and the new
    // image to flash also fits in half the flash space, then flash it
    // in the partition that is not currently being used to boot from.
    if( curImgSize <= availableSizeTwoImgs &&
        newImgSize <= availableSizeTwoImgs &&
        getPartitionFromTag( pCurTag ) == 1 )
    {
        // Update rootfsAddr to point to the second boot partition.
        int offset = (totalSize / 2) + TAG_LEN;

        printk("update slave image.......\r\n");
        lFlag = 1;
        strcpy(acFlag, "4");
        
        sprintf(((PFILE_TAG) tagFs)->kernelAddress, "%lu",
            (unsigned long) IMAGE_BASE + offset + (kernelAddr - rootfsAddr));
        kernelAddr = baseAddr + offset + (kernelAddr - rootfsAddr);

        sprintf(((PFILE_TAG) tagFs)->rootfsAddress, "%lu",
            (unsigned long) IMAGE_BASE + offset);
        rootfsAddr = baseAddr + offset;
    }

 
    kerSysVariableSet(acFlag, sizeof(acFlag), 256);


    UpdateImageSequenceNumber( ((PFILE_TAG) tagFs)->imageSequence );
    crc = getCrc32((unsigned char *)tagFs, (UINT32)TAG_LEN-TOKEN_LEN, crc);      
    *(unsigned long *) &((PFILE_TAG) tagFs)->tagValidationToken[0] = crc;

#if 0
    if( (status = kerSysBcmImageSet((rootfsAddr-TAG_LEN), tagFs,
        TAG_LEN + newImgSize)) != 0 )
    {
        printk("Failed to flash root file system. Error: %d\n", status);
        return status;
    }
#endif

    /*l65130 2008-01-17 如果写最后一块时，系统掉电，而TAG已经写完毕，那就没
      招了，BCM FLASH架构存在问题 */

    /*修改写FLASH顺序，最后烧写包含TAG的一块*/
    fsBlk = flash_get_blk(rootfsAddr-TAG_LEN);
    ulSectorSize = flash_get_sector_size(fsBlk);
    if( (status = kerSysBcmImageSet((rootfsAddr-TAG_LEN+ulSectorSize), tagFs+ulSectorSize,
        TAG_LEN + newImgSize - ulSectorSize)) != 0 )
    {
        printk("Failed to flash root file system. Error: %d\n", status);
        return status;
    }

    /*烧写包含TAG的一块*/
    if( (status = kerSysBcmImageSet((rootfsAddr-TAG_LEN), tagFs, ulSectorSize)) != 0 )
    {
        printk("Failed to flash root file system. Error: %d\n", status);
        return status;
    }


    for( p = nvramData.szBootline; p[2] != '\0'; p++ )
        if( p[0] == 'p' && p[1] == '=' && p[2] != BOOT_LATEST_IMAGE )
        {
            UINT32 crc = CRC32_INIT_VALUE;
    
            // Change boot partition to boot from new image.
            p[2] = BOOT_LATEST_IMAGE;

            nvramData.ulCheckSum = 0;
            crc = getCrc32((char *)&nvramData, (UINT32) sizeof(NVRAM_DATA), crc);      
            nvramData.ulCheckSum = crc;
            kerSysNvRamSet( (char *) &nvramData, sizeof(nvramData), 0);
            break;
        }

    if (lFlag)
    {
        strcpy(acFlag, "2");
    }
    else
    {
        strcpy(acFlag, "1");
    }

    kerSysVariableSet(acFlag, sizeof(acFlag), 256);
    
    return(status);
}
#else
static int flashFsKernelImage( int destAddr, unsigned char *imagePtr,
    int imageLen )
{
	int status = 0;
	char *p;
	char *tagFs = imagePtr;	
	PFILE_TAG pImageTag = (PFILE_TAG) imagePtr;
	int rootfsAddr = simple_strtoul(pImageTag->rootfsAddress, NULL, 10) + BOOT_OFFSET;
	int kernelAddr = simple_strtoul(pImageTag->kernelAddress, NULL, 10) + BOOT_OFFSET;
	unsigned int newImgSize = simple_strtoul(pImageTag->rootfsLen, NULL, 10) +
		simple_strtoul(pImageTag->kernelLen, NULL, 10);
	unsigned int baseAddr = (unsigned int) flash_get_memptr(0);
	unsigned int totalSize = (unsigned int) flash_get_total_size();

	int tagVer = simple_strtoul(pImageTag->tagVersion, NULL, 10);
	int typeFlag = simple_strtoul(pImageTag->reserved, NULL, 10);
	unsigned int availableTempFwSize = (totalSize / 2) - FLASH_RESERVED_AT_END;
	int offset = 0;
	UINT32 crc = CRC32_INIT_VALUE;
	NVRAM_DATA nvramData;
	unsigned int ulSectorSize = 0;
	int fsBlk = 0;


	if(TAGVERSION_HW_OLD == tagVer && TAGVERSION_FLAG_TEMP_FW == typeFlag)
	{

		printk("This is a temporary firmware.\n");

	
		if( (TAG_LEN + newImgSize) > availableTempFwSize)
		{
			printk("Illegal image size %d.	Image size must not be greater than %d.\n", newImgSize, availableTempFwSize);
			return -1;
		}
		printk("update tempporary image in partition 1.......\r\n");

		offset = FLASH_LENGTH_BOOT_ROM + TAG_LEN;

		UpdateImageSequenceNumber( ((PFILE_TAG) tagFs)->imageSequence );
	}
	else if(TAGVERSION_HW_NEW == tagVer && 
					(TAGVERSION_FLAG_NORMAL_FW == typeFlag || TAGVERSION_FLAG_PACKAGED_FW == typeFlag))
	{
		printk("This is a normal system firmware.\n");

		if( (TAG_LEN + newImgSize) > FLASH_LENGTH_NORMAL_SYSTEM)
		{
			printk("Illegal image size %d.	Image size must not be greater than %d.\n", newImgSize, FLASH_LENGTH_NORMAL_SYSTEM);
			return -1;
		}
		printk("update normal system image in partition 1.......\r\n");
		
		offset = FLASH_LENGTH_BOOT_ROM + TAG_LEN;

		UpdateImageSequenceNumber( ((PFILE_TAG) tagFs)->imageSequence );		
	}
	else if(TAGVERSION_HW_NEW == tagVer && TAGVERSION_FLAG_SMALL_FW == typeFlag)
	{
		printk("This is a small system firmware.\n");

		if( (TAG_LEN + newImgSize) > FLASH_LENGTH_SMALL_SYSTEM)
		{
			printk("Illegal image size %d.	Image size must not be greater than %d.\n", newImgSize, FLASH_LENGTH_SMALL_SYSTEM);
			return -1;
		}
		printk("update small system image in partition 2.......\r\n");

		offset = FLASH_LENGTH_BOOT_ROM + FLASH_LENGTH_NORMAL_SYSTEM + TAG_LEN;

		sprintf(((PFILE_TAG) tagFs)->imageSequence, "%d", 0);
	}
	
#ifdef	SUPPORT_TMP_FIRMWARE
	else if(TAGVERSION_HW_OLD == tagVer)
	{
		printk("This is a old version firmware(double system).\n");
	
		if( (TAG_LEN + newImgSize) > availableTempFwSize)
		{
			printk("Illegal image size %d.	Image size must not be greater than %d.\n", newImgSize, availableTempFwSize);
			return -1;
		}
		printk("update old version image in partition 2.......\r\n");

		offset = (totalSize / 2) + TAG_LEN;

		UpdateImageSequenceNumber( ((PFILE_TAG) tagFs)->imageSequence );
	}
#endif

	else
	{
		printk("[%s:%d]Illegal image.\n", __func__, __LINE__);
		return -1;
	}

	sprintf(((PFILE_TAG) tagFs)->kernelAddress, "%lu",
		(unsigned long) IMAGE_BASE + offset + (kernelAddr - rootfsAddr));
	kernelAddr = baseAddr + offset + (kernelAddr - rootfsAddr);
	
	sprintf(((PFILE_TAG) tagFs)->rootfsAddress, "%lu",
		(unsigned long) IMAGE_BASE + offset);
	rootfsAddr = baseAddr + offset;

	crc = getCrc32((unsigned char *)tagFs, (UINT32)TAG_LEN-TOKEN_LEN, crc); 	 
	*(unsigned long *) &((PFILE_TAG) tagFs)->tagValidationToken[0] = crc;

	memcpy((char *)&nvramData, (char *)get_nvram_start_addr(),sizeof(nvramData));

	/*修改写FLASH顺序，最后烧写包含TAG的一块*/
	fsBlk = flash_get_blk(rootfsAddr-TAG_LEN);
	ulSectorSize = flash_get_sector_size(fsBlk);
	if( (status = kerSysBcmImageSet((rootfsAddr-TAG_LEN+ulSectorSize), tagFs+ulSectorSize,
		TAG_LEN + newImgSize - ulSectorSize)) != 0 )
	{
		printk("Failed to flash root file system. Error: %d\n", status);
		return status;
	}

	/*烧写包含TAG的一块*/
	if( (status = kerSysBcmImageSet((rootfsAddr-TAG_LEN), tagFs, ulSectorSize)) != 0 )
	{
		printk("Failed to flash root file system. Error: %d\n", status);
		return status;
	}


	for( p = nvramData.szBootline; p[2] != '\0'; p++ )
		if( p[0] == 'p' && p[1] == '=' && p[2] != BOOT_LATEST_IMAGE )
		{
			UINT32 crc = CRC32_INIT_VALUE;
	
			// Change boot partition to boot from new image.
			p[2] = BOOT_LATEST_IMAGE;

			nvramData.ulCheckSum = 0;
			crc = getCrc32((char *)&nvramData, (UINT32) sizeof(NVRAM_DATA), crc);	   
			nvramData.ulCheckSum = crc;
			kerSysNvRamSet( (char *) &nvramData, sizeof(nvramData), 0);
			break;
		}
	
	return(status);

}

#endif
/*End -- Normal System & Small System -20091201*/

PFILE_TAG kerSysImageTagGet(void)
{
    return( getBootImageTag() );
}

//********************************************************************************************
// misc. ioctl calls come to here. (flash, led, reset, kernel memory access, etc.)
//********************************************************************************************
static int board_ioctl( struct inode *inode, struct file *flip,
                        unsigned int command, unsigned long arg )
{
    int ret = 0;
    BOARD_IOCTL_PARMS ctrlParms;
    unsigned char ucaMacAddr[NVRAM_MAC_ADDRESS_LEN];
    /* HUAWEI HGW s48571 2008年3月10日 HSPA记录log add begin:*/
    static int iHspaTrafficMode = MODE_NONE;
    /* HUAWEI HGW s48571 2008年3月10日 HSPA记录log add end.*/

	int iWriteRetryTimes = 0;
	

    switch (command) 
    {
        case BOARD_IOCTL_FLASH_INIT:
            // not used for now.  kerSysBcmImageInit();
            break;

        case BOARD_IOCTL_FLASH_WRITE:
            
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
            {
				while(FLASH_WRITING == s_iFlashWriteFlg)
				{
				
					 printk("Flash Write confliction, wait...\n");
					 iWriteRetryTimes++;
					 if(10 <= iWriteRetryTimes)
					 {
						printk("Flash Write failed over 10 times.\n");
                        ret = -1;
                        ctrlParms.result = ret;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
						return ret;
					 }
					 msleep(1000);
				}


			
                s_iFlashWriteFlg = FLASH_WRITING;
                NVRAM_DATA SaveNvramData;
                PNVRAM_DATA pNvramData = (PNVRAM_DATA) get_nvram_start_addr();

                /* start of y42304 20061230: 有VOIP业务期间禁止除了升级之外其它写FLASH操作 */
                if( (ctrlParms.action == SCRATCH_PAD) 
                   || (ctrlParms.action == PERSISTENT)                    
                   || (ctrlParms.action == VARIABLE) 
                   || (ctrlParms.action == NVRAM)
                   /* begin --- Add Persistent storage interface backup flash partition by w69233 */
                   || (ctrlParms.action == PSI_BACKUP)
                   /* end ----- Add Persistent storage interface backup flash partition by w69233 */
                   || (ctrlParms.action == FIX)
                   || (ctrlParms.action == AVAIL))
                {
                    if (g_sVoipServiceStatus == 1)
                    {
                        printk("\nSystem is on VOIP service, forbid to save flash,\nPlease try again later...\n");                                                
                        ret = -1;
                        ctrlParms.result = ret;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                        s_iFlashWriteFlg = FLASH_IDLE;                          
                       return ret;
                    }                    
                }
                /* end of y42304 20061230 */

                switch (ctrlParms.action)
                {
                    case SCRATCH_PAD:
                        
                        if (ctrlParms.offset == -1)
                        {
                              ret =  kerSysScratchPadClearAll();
                        }
                        else
                        {
                              ret = kerSysScratchPadSet(ctrlParms.string, ctrlParms.buf, ctrlParms.offset);
                        }
                        break;

                    case PERSISTENT:              
                        ret = kerSysPersistentSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);                   
                        break;

                    /* begin --- Add Persistent storage interface backup flash partition by w69233 */
                    case PSI_BACKUP:
                        ret = kerSysPsiBackupSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);                   
                        break;
                    /* end ----- Add Persistent storage interface backup flash partition by w69233 */

                     /*start of  增加FLASH VA分区porting by l39225 20060504*/
                    case  VARIABLE:
                    	      ret = kerSysVariableSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                          break;
                    /*end of  增加FLASH VA分区porting by l39225 20060504*/
                    
                    case NVRAM:                 
                        ret = kerSysNvRamSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;

                    case BCM_IMAGE_CFE:
                        g_iReset = 0;
                        if( ctrlParms.strLen <= 0 || ctrlParms.strLen > FLASH_LENGTH_BOOT_ROM )
                        {
                            printk("Illegal CFE size [%d]. Size allowed: [%d]\n",
                                ctrlParms.strLen, FLASH_LENGTH_BOOT_ROM);
                            ret = -1;
                            g_iReset = 1;
                            break;
                        }

                        // save NVRAM data into a local structure
                        memcpy( &SaveNvramData, pNvramData, sizeof(NVRAM_DATA) );

                        // set memory type field
                        BpGetSdramSize( (unsigned long *) &ctrlParms.string[SDRAM_TYPE_ADDRESS_OFFSET] );
                        // set thread number field
                        BpGetCMTThread( (unsigned long *) &ctrlParms.string[THREAD_NUM_ADDRESS_OFFSET] );

                        /* HUAWEI HGW l39225 2005年10月19日 poweron  add begin:*/                    

                        /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting modify begin:
                        kerSysLedCtrl(kLedPower,kLedStateFastBlinkContinues);
                        */
                            kerSysLedCtrl(kLedAdsl,kLedStateFastBlinkContinues);
                        /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting modify end. */

                        /* HUAWEI HGW l39225 2005年10月19日 poweron  add end:*/
                        ret = kerSysBcmImageSet(ctrlParms.offset + BOOT_OFFSET, ctrlParms.string, ctrlParms.strLen);

                        // if nvram is not valid, restore the current nvram settings
                        if( BpSetBoardId( pNvramData->szBoardId ) != BP_SUCCESS &&
                            *(unsigned long *) pNvramData == NVRAM_DATA_ID )
                        {
                            kerSysNvRamSet((char *) &SaveNvramData, sizeof(SaveNvramData), 0);
                        }

                        g_iReset = 1;
                        break;
                        
                    case BCM_IMAGE_FS:

                        g_iReset = 0;
                    	   /*start of GPIO 3.4.2 porting by l39225 20060504*/
                           /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting modify begin:
                           kerSysLedCtrl(kLedPower,kLedStateFastBlinkContinues);
                           */
                           kerSysLedCtrl(kLedAdsl,kLedStateFastBlinkContinues);
                           /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting modify end. */
                    	   /*end of GPIO 3.4.2 porting by l39225 20060504*/
                        if( (ret = flashFsKernelImage( ctrlParms.offset,
                            ctrlParms.string, ctrlParms.strLen)) == 0 )
                        {
                            kerSysMipsSoftReset();
                        }
                        g_iReset = 1;
                        break;

					/*Start -- Normal System & Small System -20091201*/						
					case BCM_IMAGE_FS_NO_REBOOT:
					
						g_iReset = 0;
						   /*start of GPIO 3.4.2 porting by l39225 20060504*/
						   /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting modify begin:
						   kerSysLedCtrl(kLedPower,kLedStateFastBlinkContinues);
						   */
						   kerSysLedCtrl(kLedAdsl,kLedStateFastBlinkContinues);
						   /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting modify end. */
						   /*end of GPIO 3.4.2 porting by l39225 20060504*/
						if( (ret = flashFsKernelImage( ctrlParms.offset,
							ctrlParms.string, ctrlParms.strLen)) == 0 )
						{
							//don't reboot, still get normal system part of packaged firmware to flash
							//kerSysMipsSoftReset();
						}
						g_iReset = 1;
						break;
					/*End -- Normal System & Small System -20091201*/

                    case BCM_IMAGE_KERNEL:  // not used for now.
                        break;
                    case BCM_IMAGE_WHOLE:
                        g_iReset = 0;
                        if(ctrlParms.strLen <= 0)
                        {
                            printk("Illegal flash image size [%d].\n", ctrlParms.strLen);
                            ret = -1;
                            break;
                        }

                        // save NVRAM data into a local structure
                        memcpy( &SaveNvramData, pNvramData, sizeof(NVRAM_DATA) );

                        if (ctrlParms.offset == 0) {
                            ctrlParms.offset = FLASH_BASE;
                        }

                        /*start of GPIO 3.4.2 porting by l39225 20060504*/
                        /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting modify begin:
                        kerSysLedCtrl(kLedPower,kLedStateFastBlinkContinues);
                        */
                        kerSysLedCtrl(kLedAdsl,kLedStateFastBlinkContinues);
                        /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting modify end. */
                    	   /*end of GPIO 3.4.2 porting by l39225 20060504*/
                        ret = kerSysBcmImageSet(ctrlParms.offset, ctrlParms.string, ctrlParms.strLen);

                        // if nvram is not valid, restore the current nvram settings
                        if( BpSetBoardId( pNvramData->szBoardId ) != BP_SUCCESS &&
                            *(unsigned long *) pNvramData == NVRAM_DATA_ID )
                        {
                            kerSysNvRamSet((char *) &SaveNvramData, sizeof(SaveNvramData), 0);
                        }
                        g_iReset = 1;
                        kerSysMipsSoftReset();
                        break;

                    /*start l65130 20080328 for more partitions*/
                    case FIX:
                        ret = kerSysFixSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;
                    case AVAIL:
                        ret = kerSysAvailSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;
                    /*end l65130 20080328 for more partitions*/
                    
                    default:
                        ret = -EINVAL;
                        printk("flash_ioctl_command: invalid command %d\n", ctrlParms.action);
                        break;
                }

                ctrlParms.result = ret;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                s_iFlashWriteFlg = FLASH_IDLE;
            }
            else
                ret = -EFAULT;
            break;

        case BOARD_IOCTL_FLASH_READ:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
                switch (ctrlParms.action)
                {
                    case SCRATCH_PAD:
                        ret = kerSysScratchPadGet(ctrlParms.string, ctrlParms.buf, ctrlParms.offset);
                        break;

                    case PERSISTENT:
                        ret = kerSysPersistentGet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;

                    /* begin --- Add Persistent storage interface backup flash partition by w69233 */
                    case PSI_BACKUP:
                        ret = kerSysPsiBackupGet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;
                    /* end ----- Add Persistent storage interface backup flash partition by w69233 */

                    /*start of  增加FLASH VA分区porting by l39225 20060504*/
                    case  VARIABLE:
                    	      ret = kerSysVariableGet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                          break;
                    /*end of  增加FLASH VA分区porting by l39225 20060504*/

                    case NVRAM:
                        ret = kerSysNvRamGet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;

                    case FLASH_SIZE:
                        ret = kerSysFlashSizeGet();
                        break;

                    /* start of y42304 added 20060814: 提供获取flash FILE_TAG的接口给应用*/                                         
                    case GET_FILE_TAG_FROM_FLASH:
                    {
                        PFILE_TAG stFileTag = getTagFromPartition(1);
                        if (NULL != stFileTag)
                        {                    
                            __copy_to_user(ctrlParms.string, stFileTag, sizeof(FILE_TAG));                    
                            ctrlParms.result = 0;
                            ret = 0;
                        }
                        else
                        {  
                            ctrlParms.result = -1;
                            ret = -EFAULT;                    
                        }                
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,  sizeof(BOARD_IOCTL_PARMS));   
                        
                        break;
                    }                               
                    /* start of y42304 added 20060814 */
                    
                    case BCM_IMAGE_FS:
                    {                        
                        ctrlParms.result = kerSysGetFsImageFromFlash(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);                                    
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,  sizeof(BOARD_IOCTL_PARMS));   
                        ret = 0;
                        break;
                    }

                    case BCM_IMAGE_KERNEL:
                    {
                        ctrlParms.result = kerSysGetKernelImageFromFlash(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);                                                            
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,  sizeof(BOARD_IOCTL_PARMS));                           
                        ret = 0;
                        break;
                    }
                    
                    /*start l65130 20080328 for more partitions*/
                    case FIX:
                        ret = kerSysFixGet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;
                    case AVAIL:
                        ret = kerSysAvailGet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;
                    /*end l65130 20080328 for more partitions*/

                    default:
                        ret = -EINVAL;
                        printk("Not supported.  invalid command %d\n", ctrlParms.action);
                        break;
                }
                ctrlParms.result = ret;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            }
            else
                ret = -EFAULT;
            break;
#if 0
        case BOARD_IOCTL_GET_NR_PAGES:
            ctrlParms.result = nr_free_pages() + get_page_cache_size();
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
            break;
#endif

        case BOARD_IOCTL_DUMP_ADDR:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
                dumpaddr( (unsigned char *) ctrlParms.string, ctrlParms.strLen );
                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            }
            else
                ret = -EFAULT;
            break;

        case BOARD_IOCTL_SET_MEMORY:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
                unsigned long  *pul = (unsigned long *)  ctrlParms.string;
                unsigned short *pus = (unsigned short *) ctrlParms.string;
                unsigned char  *puc = (unsigned char *)  ctrlParms.string;
                switch( ctrlParms.strLen )
                {
                    case 4:
                        *pul = (unsigned long) ctrlParms.offset;
                        break;
                    case 2:
                        *pus = (unsigned short) ctrlParms.offset;
                        break;
                    case 1:
                        *puc = (unsigned char) ctrlParms.offset;
                        break;
                }
                dumpaddr( (unsigned char *) ctrlParms.string, sizeof(long) );
                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            }
            else
                ret = -EFAULT;
            break;
      
        case BOARD_IOCTL_MIPS_SOFT_RESET:
            kerSysMipsSoftReset();
            break;

        case BOARD_IOCTL_LED_CTRL:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
	            kerSysLedCtrl((BOARD_LED_NAME)ctrlParms.strLen, (BOARD_LED_STATE)ctrlParms.offset);
	            ret = 0;
	        }
            break;

        case BOARD_IOCTL_GET_ID:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
                if( ctrlParms.string )
                {
                    char *p = (char *)get_system_type();                      
                    unsigned char ucHardwareType = GetHarewareType();

                    switch(ucHardwareType)
                    {    
                    case NOT_SUPPROT_GET_VERSION: /* 非华为自研硬件 */    
                        if( strlen(p) + 1 < ctrlParms.strLen )
                        {
                            ctrlParms.strLen = strlen(p) + 1;
                        }
                        __copy_to_user(ctrlParms.string, p, ctrlParms.strLen);                        
                        break;
/* HUAWEI HGW s48571 2008年1月18日 Hardware Porting modify begin:*/
 //#ifdef CONFIG_SLIC_LE88221
#if 1


                     case HG55MAGV:
                         ctrlParms.strLen = sizeof("HW553");                        
                         __copy_to_user(ctrlParms.string, "HW553", ctrlParms.strLen);
                         break;
 #else
 
                    case HG52VAG: /* 华为自研HG52VAG, HG52VAG,HG51VAG */    
                    case HG52VAGB:
                    case HG51VAG:
                    case HG51VAGI:                        
                        //printk("\n Board id is set HW6358GW_B\n");                       
                        ctrlParms.strLen = sizeof("HW6358GW_B");                        
                        __copy_to_user(ctrlParms.string, "HW6358GW_B", ctrlParms.strLen);
                        break;
                    case HG55VAGII:
                        ctrlParms.strLen = sizeof("HG550II");                        
                        __copy_to_user(ctrlParms.string, "HG550II", ctrlParms.strLen);
                        break;
                        
#endif                        
/* HUAWEI HGW s48571 2008年1月18日 Hardware Porting modify end. */
                    default: 
                        break;            
                    }    
                }            
                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            }    
            
            break;                        

       case BOARD_IOCTL_GET_MAC_ADDRESS:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
                ctrlParms.result = kerSysGetMacAddress( ucaMacAddr,
                    ctrlParms.offset );

                if( ctrlParms.result == 0 )
                {
                    __copy_to_user(ctrlParms.string, ucaMacAddr,
                        sizeof(ucaMacAddr));
                }

                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,
                    sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            }
            else
                ret = -EFAULT;
            break;

        case BOARD_IOCTL_RELEASE_MAC_ADDRESS:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
                if (copy_from_user((void*)ucaMacAddr, (void*)ctrlParms.string, \
                     NVRAM_MAC_ADDRESS_LEN) == 0) 
                {
                    ctrlParms.result = kerSysReleaseMacAddress( ucaMacAddr );
                }
                else
                {
                    ctrlParms.result = -EACCES;
                }

                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,
                    sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            }
            else
                ret = -EFAULT;
            break;

        case BOARD_IOCTL_GET_PSI_SIZE:
            ctrlParms.result = (int) g_pNvramInfo->ulPsiSize;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
            break;

        case BOARD_IOCTL_GET_SDRAM_SIZE:
            ctrlParms.result = (int) g_pNvramInfo->ulSdramSize;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
            break;

       
        /* start of BOARD update by y42304 20060519: 解决通过Telnet做装备测试*/            
        case BOARD_IOCTL_EQUIPMENT_TEST:            
        {
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
            {
                PNVRAM_DATA pNvramData = (PNVRAM_DATA) get_nvram_start_addr();
                
                /* start of y42304 20061230: 有VOIP业务期间禁止除了升级之外其它写FLASH操作 */
                if( (ctrlParms.action == SET_BASE_MAC_ADDRESS) 
                   || (ctrlParms.action == SET_MAC_AMOUNT)                    
                   || (ctrlParms.action == SET_BOARD_ID) 
                   || (ctrlParms.action == SET_SERIAL_NUMBER) )
                {
                    if (g_sVoipServiceStatus == 1)
                    {
                        printk("\nSystem is on VOIP service, forbid to save flash,\nPlease try again later...\n");                                                
                        ret = -1;
                        ctrlParms.result = ret;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                       return ret;
                    }                    
                }
                /* end of y42304 20061230 */
                
                switch (ctrlParms.action)
                {
                    case SET_BASE_MAC_ADDRESS:       /* 设置基mac地址*/
                    {      
                        NVRAM_DATA SaveNvramData;
                        unsigned long crc = CRC32_INIT_VALUE;
                 
                        memcpy((char *)&SaveNvramData, (char *)pNvramData, sizeof(SaveNvramData));
                        memset(SaveNvramData.ucaBaseMacAddr, 0, NVRAM_MAC_ADDRESS_LEN);
                        memcpy(SaveNvramData.ucaBaseMacAddr, ctrlParms.string, NVRAM_MAC_ADDRESS_LEN);
                        SaveNvramData.ulCheckSum = 0;
                        crc = getCrc32((char *)&SaveNvramData, (UINT32) sizeof(NVRAM_DATA), crc);
                        SaveNvramData.ulCheckSum = crc;
                        kerSysNvRamSet((char *)&SaveNvramData, sizeof(SaveNvramData), 0);
                 
                        ctrlParms.result = 0;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                        
                        ret = 0;                           	                        	         	
                    	break;
                    }   
                    case GET_BASE_MAC_ADDRESS:     /* 获取基mac地址 */
                    {
                    	unsigned char tmpMacAddress[NVRAM_MAC_ADDRESS_LEN+1];
                        tmpMacAddress[0]='\0';
               
                        memcpy(tmpMacAddress,(unsigned char *)(pNvramData->ucaBaseMacAddr),NVRAM_MAC_ADDRESS_LEN);
                        tmpMacAddress[NVRAM_MAC_ADDRESS_LEN]='\0';
                        
                        ctrlParms.strLen= NVRAM_MAC_ADDRESS_LEN;
                        __copy_to_user(ctrlParms.string, tmpMacAddress, NVRAM_MAC_ADDRESS_LEN);
                        ctrlParms.result = 0;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                        ret = 0;
                    	break;
                    }
                    case SET_MAC_AMOUNT:    /* 更改mac数目(11 - 32) */
                    {
                    	NVRAM_DATA SaveNvramData;
                        unsigned long crc = CRC32_INIT_VALUE;
                          
                        memcpy((char *)&SaveNvramData, (char *)pNvramData, sizeof(SaveNvramData));
                        SaveNvramData.ulNumMacAddrs = ctrlParms.offset;
                        SaveNvramData.ulCheckSum = 0;
                        crc = getCrc32((char *)&SaveNvramData, (UINT32) sizeof(NVRAM_DATA), crc);
                        SaveNvramData.ulCheckSum = crc;
                        kerSysNvRamSet((char *)&SaveNvramData, sizeof(SaveNvramData), 0);
                 
                        ctrlParms.result = 0;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                 
                        ret = 0;                    	
                    	break;
                    }
                    case GET_MAC_AMOUNT:    /* 获取mac地址数目 */
                    {
                    	ctrlParms.result = pNvramData->ulNumMacAddrs;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                 
                        ret = 0;   
                    	break;
                    }
                    case GET_ALL_BOARD_ID_NAME:
                    {
                    	ctrlParms.result = BpGetBoardIds(ctrlParms.string, ctrlParms.strLen);                 
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));

                        ret = 0; 
                    	break;
                    }
                    case GET_PCB_VERSION:
                    {
                    	ctrlParms.result  = (int)kerSysGetPCBVersion();                     
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                        
                        ret = 0; 
                    	break;
                    }
                    case GET_BOARD_VERSION:
                    {           
                        /*
                         * GPIO8 GPIO7  GPIO6
                         *   0     0     0   -->    HG52VAG                         
                         *   0     0     1   -->    HG52VAGB
                         *   1     0     0   -->    HG51VAG
                         *   1     1     0   -->    HG51VAGI
                         */
                        char tmpBoardVersion[16];                        
                        unsigned char ucHardwareType = GetHarewareType();
                        memset(tmpBoardVersion, 0, sizeof(tmpBoardVersion));
/* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add begin:*/
//#ifdef CONFIG_SLIC_LE88221
#if 1

                        switch(ucHardwareType)
                        {
                        case NOT_SUPPROT_GET_VERSION: /* 非华为自研硬件 */                                
                             memcpy(tmpBoardVersion, "HG520DEMO", sizeof("HG520DEMO"));                                                                                   
                             break;
                             
                         case HG55MAGV: /* 华为自研HG52VAG */                               
                              memcpy(tmpBoardVersion, "HG55MAGV", sizeof("HG55MAGV")); 
                              break;
                              
                         default:                             
                              memcpy(tmpBoardVersion, "N/A", sizeof("N/A"));                                 
                              break;            
                         }  
                        
#else
                        switch(ucHardwareType)
                        {
                        case NOT_SUPPROT_GET_VERSION: /* 非华为自研硬件 */                                
                             memcpy(tmpBoardVersion, "HG520DEMO", sizeof("HG520DEMO"));                                                                                   
                             break;
                             
                        case HG52VAG: /* 华为自研HG52VAG */                               
                             memcpy(tmpBoardVersion, "HG52VAG", sizeof("HG52VAG")); 
                             break;
                             
                        case HG52VAGB: /* 华为自研HG52VAGB */    
                             memcpy(tmpBoardVersion, "HG52VAGB", sizeof("HG52VAGB")); 
                             break;
                             
                        case HG51VAG:  /* 华为自研HG51VAG */  
                             memcpy(tmpBoardVersion, "HG51VAG", sizeof("HG51VAG"));                                 
                             break;

                        case HG51VAGI:  /* 华为自研HG51VAGI */  
                             memcpy(tmpBoardVersion, "HG51VAGI", sizeof("HG51VAGI"));                                 
                             break;

                        case HG55VAGII: /* HG550I ADSL版本 */
                        {
                            unsigned char ucBoardVersion = GetBoardVersion(); 

                            switch(ucBoardVersion)
                            {
                                case HG55MAGC:        
                                {                                    
                                    memcpy(tmpBoardVersion, "HG55magc", sizeof("HG55magc"));                                                      
                                    break;
                                }
                                case HG55MAGD:
                                default:                                
                                {
                                    memcpy(tmpBoardVersion, "HG55magd", sizeof("HG55magd"));                                                      
                                    break;
                                }
                            }
                            break;
                        }          

                        default:                             
                             memcpy(tmpBoardVersion, "N/A", sizeof("N/A"));                                 
                             break;            
                        }    

#endif
/* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add end.*/

                        __copy_to_user(ctrlParms.string, tmpBoardVersion, sizeof(tmpBoardVersion));
                    	ctrlParms.result = 0;              
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));

                        ret = 0;   
                    	break;
                    }
                    case SET_BOARD_ID: /* 提供给telnet更改BOARD ID */
                    {
                    	NVRAM_DATA SaveNvramData;
                        unsigned long crc = CRC32_INIT_VALUE;
                
                        memcpy((char *)&SaveNvramData, (char *)pNvramData, sizeof(SaveNvramData));
                        memset(SaveNvramData.szBoardId, 0, NVRAM_BOARD_ID_STRING_LEN);
                        memcpy(SaveNvramData.szBoardId, ctrlParms.string, NVRAM_BOARD_ID_STRING_LEN);
                        SaveNvramData.ulCheckSum = 0;
                        crc = getCrc32((char *)&SaveNvramData, (UINT32) sizeof(NVRAM_DATA), crc);
                        SaveNvramData.ulCheckSum = crc;
                        kerSysNvRamSet((char *)&SaveNvramData, sizeof(SaveNvramData), 0);
                 
                        ctrlParms.result = 0;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                 
                        ret = 0;                    	
                    	break;
                    }
                    case GET_BOARD_ID:
                    {
                    	char tmpBoardId[NVRAM_BOARD_ID_STRING_LEN+1];
                        tmpBoardId[0]='\0';
                
                        memcpy(tmpBoardId,(unsigned char *)(pNvramData->szBoardId), NVRAM_BOARD_ID_STRING_LEN);
                        tmpBoardId[NVRAM_BOARD_ID_STRING_LEN]='\0';
                        ctrlParms.strLen= NVRAM_BOARD_ID_STRING_LEN;
                        __copy_to_user(ctrlParms.string, tmpBoardId, NVRAM_BOARD_ID_STRING_LEN);
                        ctrlParms.result = 0;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                 
                        ret = 0;  
                    	break;
                    }
                    case SET_SERIAL_NUMBER:   /* 设置单板序列号 */
                    {
                        NVRAM_DATA SaveNvramData;
                        unsigned long crc = CRC32_INIT_VALUE;

                        memcpy((char *)&SaveNvramData, (char *)pNvramData, sizeof(SaveNvramData));
                        memset(SaveNvramData.szSerialNumber, 0, NVRAM_SERIAL_NUMBER_LEN);
                        memcpy(SaveNvramData.szSerialNumber, ctrlParms.string,(ctrlParms.strLen < NVRAM_SERIAL_NUMBER_LEN) ? ctrlParms.strLen : NVRAM_SERIAL_NUMBER_LEN);
                        SaveNvramData.ulCheckSum = 0;
                        crc = getCrc32((char *)&SaveNvramData, (UINT32) sizeof(NVRAM_DATA), crc);
                        SaveNvramData.ulCheckSum = crc;
                        kerSysNvRamSet((char *)&SaveNvramData, sizeof(SaveNvramData), 0);
                        
                        /* start of BOARD added by y42304 20060713: */
                        ctrlParms.result = 0;
                        /* end of BOARD added by y42304 */                        
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                        
                        ret = 0;
                        break;
                    }
                    case GET_SERIAL_NUMBER:   /* 获取单板序列号 */
                    {                  
                        char tmpSerialNumber[NVRAM_SERIAL_NUMBER_LEN+1];                        
                        memset(tmpSerialNumber, 0, NVRAM_SERIAL_NUMBER_LEN+1);
                        tmpSerialNumber[0]='\0';
                        
                        memcpy(tmpSerialNumber,(unsigned char *)(pNvramData->chReserved+sizeof(pNvramData->chReserved)),NVRAM_SERIAL_NUMBER_LEN);
                        tmpSerialNumber[NVRAM_SERIAL_NUMBER_LEN]='\0';
                        ctrlParms.strLen = NVRAM_SERIAL_NUMBER_LEN;
                        __copy_to_user(ctrlParms.string, tmpSerialNumber, NVRAM_SERIAL_NUMBER_LEN);
                        ctrlParms.result = 0;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                        
                        ret = 0;                        
                        break;
                    }       
                    case GET_CPU_REVISION_ID:
                    {                        
                        ctrlParms.result = (int)(PERF->RevID & 0xFF);
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                        ret = 0;
                        
                        break;
                    }           
                    /* 提供给硬件人员查看单板的制造商 */
                    case GET_BOARD_MANUFACTURER_NAME:
                    {                        
                        /*
                         * GPIO35  GPIO33
                         *   1      1        -->    Huawei
                         *   1      0        -->    Alpha
                         */
                        char tmpManufactuerName[16];                        
                        unsigned char ucBoardManufacturer = kerSysGetBoardManufacturer();
                        memset(tmpManufactuerName, 0, sizeof(tmpManufactuerName));

                        switch(ucBoardManufacturer)
                        {    
                        case MANUFACTURER_HUAWEI:                                
                            memcpy(tmpManufactuerName, "Huawei", sizeof("Huawei"));                                 
                            break;
                            
                        case MANUFACTURER_ALPHA:                            
                            memcpy(tmpManufactuerName, "Alpha", sizeof("Alpha"));                                 
                            break;    

                        default:
                            memcpy(tmpManufactuerName, "N/A", sizeof("N/A"));                                                             
                            break;    
                           
                        }                        
                        __copy_to_user(ctrlParms.string, tmpManufactuerName, sizeof(tmpManufactuerName));
                    	ctrlParms.result = 0;              
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));

                        ret = 0;   
                        
                        break;
                    }       
                }               
            }           
            else
            {
                ret = -EFAULT;
            }
            break;	        	
        }   
        /* end of BOARD update by y42304 20060519: 解决通过Telnet做装备测试*/            

        case BOARD_IOCTL_GET_CHIP_ID:
            ctrlParms.result = (int) (PERF->RevID & 0xFFFF0000) >> 16;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
            break;

    #ifdef WIRELESS   
        /* start of y42304 added 20060907:  通过复位键更改wlan的状态 */
        case BOARD_IOCTL_CHECK_WLAN_STATUS:            
            ctrlParms.result = g_sWlanFlag;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
            break;
        /* start of y42304 added 20060907*/            
    #endif
        
        /* start of y42304 added 20061230: 提供给TAPI 更新是否在进行VOIP业务, 期间不允许写FLASH */
        case BOARD_IOCTL_SET_VOIP_SERVICE:                           
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
                //g_sVoipServiceStatus = ctrlParms.offset;   
                g_sVoipServiceStatus = 0;// for PAF 2010.03.19
                ctrlParms.result = 0;              
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                ret = 0;      
            }
            
            break;   
        case BOARD_IOCTL_GET_VOIP_SERVICE:
            ctrlParms.result = g_sVoipServiceStatus;              
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
            ret = 0;    
            
            break;
        /* end of y42304 added 20061230 */
        case BOARD_IOCTL_GET_NUM_ENET: {
            ETHERNET_MAC_INFO EnetInfos[BP_MAX_ENET_MACS];
            int i, numeth = 0;
            if (BpGetEthernetMacInfo(EnetInfos, BP_MAX_ENET_MACS) == BP_SUCCESS) {
            for( i = 0; i < BP_MAX_ENET_MACS; i++) {
                if (EnetInfos[i].ucPhyType != BP_ENET_NO_PHY) {
                numeth++;
                }
            }
            ctrlParms.result = numeth;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,	 sizeof(BOARD_IOCTL_PARMS));   
            ret = 0;
            }
	        else {
	            ret = -EFAULT;
	        }
	        break;
            }

        case BOARD_IOCTL_GET_CFE_VER:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                char *vertag =  (char *)(FLASH_BASE + CFE_VERSION_OFFSET);
                if (ctrlParms.strLen < CFE_VERSION_SIZE) {
                    ctrlParms.result = 0;
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                    ret = -EFAULT;
                }
                else if (strncmp(vertag, "cfe-v", 5)) { // no tag info in flash
                    ctrlParms.result = 0;
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                    ret = 0;
                }
                else {
                    ctrlParms.result = 1;
                    __copy_to_user(ctrlParms.string, vertag+CFE_VERSION_MARK_SIZE, CFE_VERSION_SIZE);
                    ctrlParms.string[CFE_VERSION_SIZE] = '\0';
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                    ret = 0;
                }
            }
            else {
                ret = -EFAULT;
            }
            break;

        case BOARD_IOCTL_GET_ENET_CFG:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                ETHERNET_MAC_INFO EnetInfos[BP_MAX_ENET_MACS];
                if (BpGetEthernetMacInfo(EnetInfos, BP_MAX_ENET_MACS) == BP_SUCCESS) {
                    if (ctrlParms.strLen == sizeof(EnetInfos)) {
                        __copy_to_user(ctrlParms.string, EnetInfos, sizeof(EnetInfos));
                        ctrlParms.result = 0;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));   
                        ret = 0;
                    } else
	                    ret = -EFAULT;
                }
	            else {
	                ret = -EFAULT;
	            }
	            break;
            }
            else {
                ret = -EFAULT;
            }
            break;            

#if defined (WIRELESS)
        case BOARD_IOCTL_GET_WLAN_ANT_INUSE:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                unsigned short antInUse = 0;
                if (BpGetWirelessAntInUse(&antInUse) == BP_SUCCESS) {
                    if (ctrlParms.strLen == sizeof(antInUse)) {
                        __copy_to_user(ctrlParms.string, &antInUse, sizeof(antInUse));
                        ctrlParms.result = 0;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));   
                        ret = 0;
                    } else
	                    ret = -EFAULT;
                }
	        else {
	           ret = -EFAULT;
	        }
	        break;
            }
            else {
                ret = -EFAULT;
            }
            break;            
#endif            
        case BOARD_IOCTL_SET_TRIGGER_EVENT:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {            	
            	BOARD_IOC *board_ioc = (BOARD_IOC *)flip->private_data;            	
                ctrlParms.result = -EFAULT;
                ret = -EFAULT;
                if (ctrlParms.strLen == sizeof(unsigned long)) {                 	                    
                    board_ioc->eventmask |= *((int*)ctrlParms.string);                    
#if defined (WIRELESS)                    
                    if((board_ioc->eventmask & SES_EVENTS)) {
                            ctrlParms.result = 0;
                            ret = 0;
                    } 
#endif                                                
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));                        
                }
	        break;
            }
            else {
                ret = -EFAULT;
            }
            break;                        

        case BOARD_IOCTL_GET_TRIGGER_EVENT:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
            	BOARD_IOC *board_ioc = (BOARD_IOC *)flip->private_data;
                if (ctrlParms.strLen == sizeof(unsigned long)) {
                    __copy_to_user(ctrlParms.string, &board_ioc->eventmask, sizeof(unsigned long));
                    ctrlParms.result = 0;
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));   
                    ret = 0;
                } else
	            ret = -EFAULT;

	        break;
            }
            else {
                ret = -EFAULT;
            }
            break;                
            
        case BOARD_IOCTL_UNSET_TRIGGER_EVENT:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                if (ctrlParms.strLen == sizeof(unsigned long)) {
                    BOARD_IOC *board_ioc = (BOARD_IOC *)flip->private_data;                	
                    board_ioc->eventmask &= (~(*((int*)ctrlParms.string)));                  
                    ctrlParms.result = 0;
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));   
                    ret = 0;
                } else
	            ret = -EFAULT;

	        break;
            } 
            else {
                ret = -EFAULT;
            }
            break;      
            
#if defined (WIRELESS)

        case BOARD_IOCTL_SET_SES_LED:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                if (ctrlParms.strLen == sizeof(int)) {
                    sesLed_ctrl(*(int*)ctrlParms.string);
                    ctrlParms.result = 0;
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));   
                    ret = 0;
                } else
	            ret = -EFAULT;

	        break;
            }
            else {
                ret = -EFAULT;
            }
            break;            
#endif                                                            

        case BOARD_IOCTL_SET_MONITOR_FD:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                int fput_needed = 0;

                g_monitor_file = fget_light( ctrlParms.offset, &fput_needed );
                if( g_monitor_file ) {
                    /* Hook this file descriptor's poll function in order to set
                     * the exception descriptor when there is a change in link
                     * state.
                     */
                    g_monitor_task = current;
                    g_orig_fop_poll = g_monitor_file->f_op->poll;
                    g_monitor_file->f_op->poll = kerSysMonitorPollHook;
                }
            }
            break;

        case BOARD_IOCTL_WAKEUP_MONITOR_TASK:
            kerSysWakeupMonitorTask();
            break;

        case BOARD_IOCTL_GET_VCOPE_GPIO:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                ret = ((ctrlParms.result = BpGetVcopeGpio(ctrlParms.offset)) != BP_NOT_DEFINED) ? 0 : -EFAULT;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            }
            else {
                ret = -EFAULT;  
                ctrlParms.result = BP_NOT_DEFINED;
            }

            break;

        case BOARD_IOCTL_SET_CS_PAR: 
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                ret = ConfigCs(&ctrlParms);
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            } 
            else {
                ret = -EFAULT;  
            }
            break;

        case BOARD_IOCTL_SET_PLL: 
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                SetPll(ctrlParms.strLen, ctrlParms.offset);
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            } 
            else {
                ret = -EFAULT;  
            }
            break;

        case BOARD_IOCTL_SET_GPIO:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                SetGpio(ctrlParms.strLen, ctrlParms.offset);
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            } 
            else {
                ret = -EFAULT;  
            }
            break;
/* HUAWEI HGW s48571 2008年1月19日 Hardware Porting add begin:*/
        case BOARD_IOCTL_SET_TRAFFIC_MODE:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                if (ctrlParms.offset < MODE_UNDEFINED && ctrlParms.offset >= MODE_NONE) 
                {
                    if(( MODE_PASSBY == g_nHspaTrafficMode )&& ( MODE_NORM != ctrlParms.offset ))
                    {
                        printk(KERN_INFO "HSPA: Origin HSPA mode is PASSBY, set error.\n");
                        ret = -EFAULT;
                    }
                    else
                    {
                    g_nHspaTrafficMode = ctrlParms.offset;
                    //printk("g_nHspaTrafficMode: %d\n",g_nHspaTrafficMode);
                    if ( iHspaTrafficMode != g_nHspaTrafficMode )
                    {
                        iHspaTrafficMode = g_nHspaTrafficMode;
                        if ( MODE_NONE == iHspaTrafficMode )
                        {
                            printk(KERN_CRIT "HSPA: No Available Network.\n");
                        }
                        else if ( MODE_2G == iHspaTrafficMode )
                        {
                            printk(KERN_CRIT "HSPA: 2G Network connected.\n");
                        }
                        else if (MODE_3G == iHspaTrafficMode)
                        {
                            printk(KERN_CRIT "HSPA: 3G Network connected.\n");
                            }
                            else if (MODE_NORM == iHspaTrafficMode)
                            {
                                printk(KERN_INFO "HSPA: enter NORM state.\n");
                            }
                            else if (MODE_PASSBY == iHspaTrafficMode)
                            {
                                printk(KERN_INFO "HSPA: enter passby state.\n");
                            }
                        }
                    }
                    ret = 0;
                } 
                else
                {
    	            ret = -EFAULT;
                }
	        break;
                
            }
            else {
                ret = -EFAULT;
            }
            break;            

/* HUAWEI HGW s48571 2008年1月19日 Hardware Porting add end.*/

/* HUAWEI HGW s48571 2008年2月20日 装备测试状态标志添加 add begin:*/
          case BOARD_IOCTL_SET_EQUIPTEST_MODE:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                if ( ( ctrlParms.offset == TRUE) || ( ctrlParms.offset == FALSE )) 
                {
                    g_nEquipTestMode = ctrlParms.offset;
                    printk("g_nEquipTestMode: %d",g_nEquipTestMode);
                    ret = 0;
                } 
                else
                {
                    ret = -EFAULT;
                }
            break;
                
            }
            else {
                ret = -EFAULT;
            }
            break;            
    
/* HUAWEI HGW s48571 2008年2月20日 装备测试状态标志添加 add end.*/

        default:
            ret = -EINVAL;
            ctrlParms.result = 0;
            printk("board_ioctl: invalid command %x, cmd %d .\n",command,_IOC_NR(command));
            break;

  } /* switch */

  return (ret);

} /* board_ioctl */

/***************************************************************************
 * SES Button ISR/GPIO/LED functions.
 ***************************************************************************/
#if defined (WIRELESS) 
#if 1 /* 2008/01/28 Jiajun Weng : New code from 3.12L.01 */
static Bool sesBtn_pressed(void)
{
//start of modify by y68191 for VDFC02_WPS
/*
    unsigned long gpio_mask = GPIO_NUM_TO_MASK(sesBtn_gpio);
    volatile unsigned long *gpio_io_reg = &GPIO->GPIOio;
    if( (sesBtn_gpio & BP_GPIO_NUM_MASK) >= 32 )
    {
    
    	printk("try to  cp gpio to reg\n");
        gpio_mask = GPIO_NUM_TO_MASK_HIGH(sesBtn_gpio);
        gpio_io_reg = &GPIO->GPIOio_high;
    }	
    if (!(*gpio_io_reg & gpio_mask)){
        return POLLIN;
    }	
 */
 //end of modify by y68191 for VDFC02_WPS
     if(WPS_FLAG)
     return POLLIN;
     return 0;
}
static irqreturn_t sesBtn_isr(int irq, void *dev_id, struct pt_regs *ptregs)
{   
    if (sesBtn_pressed()){
        wake_up_interruptible(&g_board_wait_queue); 
        return IRQ_RETVAL(1);
    } else {
        return IRQ_RETVAL(0);    	
    }
	
    return IRQ_RETVAL(0);
}
static void __init sesBtn_mapGpio()
{	
    if( BpGetWirelessSesBtnGpio(&sesBtn_gpio) == BP_SUCCESS )
    {
        printk("SES: Button GPIO 0x%x is enabled\n", sesBtn_gpio);    
    }
}

static void __init sesBtn_mapIntr(int context)
{    	
    if( BpGetWirelessSesExtIntr(&sesBtn_irq) == BP_SUCCESS )
    {
    	printk("SES: Button Interrupt 0x%x is enabled\n", sesBtn_irq);
    }
    else
    	return;
    	    
    sesBtn_irq = map_external_irq (sesBtn_irq) ;
    printk("sesBtn_irq==%d\n\r",sesBtn_irq);
    		
    if (BcmHalMapInterrupt((FN_HANDLER)sesBtn_isr, context, sesBtn_irq)) {
    	printk("SES: Interrupt mapping failed\n");
    }    
    BcmHalInterruptEnable(sesBtn_irq);
}

//modify by yinheng for WPS 080316
static unsigned int sesBtn_poll(struct file *file, struct poll_table_struct *wait)
{	

    if (sesBtn_pressed()){
	 
        return POLLIN;
    }	
    return 0;
}

static ssize_t sesBtn_read(struct file *file,  char __user *buffer, size_t count, loff_t *ppos)
{
//start of modify by y68191 for VDFC02_wps
    volatile unsigned int event=0;
    ssize_t ret=0;	

    if(!sesBtn_pressed()){
	//BcmHalInterruptEnable(sesBtn_irq);		
	    return ret;
    }	
    event = SES_EVENTS;
    __copy_to_user((char*)buffer, (char*)&event, sizeof(event));	
 //   BcmHalInterruptEnable(sesBtn_irq);	
    count -= sizeof(event);
    buffer += sizeof(event);
    ret += sizeof(event);	
    return ret;	
 //end  of modify by y68191 for VDFC02_wps
}

static void __init sesLed_mapGpio()
{	
    if( BpGetWirelessSesBtnGpio(&sesLed_gpio) == BP_SUCCESS )
    {
        printk("SES: LED GPIO 0x%x is enabled\n", sesBtn_gpio);    
    }
}

static void sesLed_ctrl(int action)
{

    //char status = ((action >> 8) & 0xff); /* extract status */
    //char event = ((action >> 16) & 0xff); /* extract event */        
    //char blinktype = ((action >> 24) & 0xff); /* extract blink type for SES_LED_BLINK  */
    
    BOARD_LED_STATE led;
    
    if(sesLed_gpio == BP_NOT_DEFINED)
        return;
    	
    action &= 0xff; /* extract led */

    //printk("blinktype=%d, event=%d, status=%d\n",(int)blinktype, (int)event, (int)status);
            	
    switch (action) 
    {
        case SES_LED_ON:
            //printk("SES: led on\n");
            led = kLedStateOn;                                          
            break;
        case SES_LED_BLINK:
            //printk("SES: led blink\n");
            led = kLedStateSlowBlinkContinues;           		
            break;
        case SES_LED_OFF:
            default:
            //printk("SES: led off\n");
            led = kLedStateOff;  						
    }	
    
    kerSysLedCtrl(kLedSes, led);
}

static void __init ses_board_init()
{
    sesBtn_mapGpio();
//    sesBtn_mapIntr(0);
    sesLed_mapGpio();
}
static void __exit ses_board_deinit()
{
    if(sesBtn_irq)
        BcmHalInterruptDisable(sesBtn_irq);
}
#endif
#endif

/***************************************************************************
 * Dying gasp ISR and functions.
 ***************************************************************************/
#define KERSYS_DBG	printk

#if defined(CONFIG_BCM96348) || defined(CONFIG_BCM96338)
/* The BCM6348 cycles per microsecond is really variable since the BCM6348
 * MIPS speed can vary depending on the PLL settings.  However, an appoximate
 * value of 120 will still work OK for the test being done.
 */
#define	CYCLE_PER_US	120
#elif defined(CONFIG_BCM96358)
#define	CYCLE_PER_US	150
#endif
#define	DG_GLITCH_TO	(100*CYCLE_PER_US)
 
static void __init kerSysDyingGaspMapIntr()
{
    unsigned long ulIntr;
    	
    if( BpGetAdslDyingGaspExtIntr( &ulIntr ) == BP_SUCCESS ) 
    {
		BcmHalMapInterrupt((FN_HANDLER)kerSysDyingGaspIsr, 0, INTERRUPT_ID_DG);
		BcmHalInterruptEnable( INTERRUPT_ID_DG );
    }
} 

void kerSysSetWdTimer(ulong timeUs)
{
	TIMER->WatchDogDefCount = timeUs * (FPERIPH/1000000);
	TIMER->WatchDogCtl = 0xFF00;
	TIMER->WatchDogCtl = 0x00FF;
}

ulong kerSysGetCycleCount(void)
{
    ulong cnt; 
#ifdef _WIN32_WCE
    cnt = 0;
#else
    __asm volatile("mfc0 %0, $9":"=d"(cnt));
#endif
    return(cnt); 
}

static Bool kerSysDyingGaspCheckPowerLoss(void)
{
    ulong clk0;
    ulong ulIntr;

    ulIntr = 0;
    clk0 = kerSysGetCycleCount();

    UART->Data = 'D';
    UART->Data = '%';
    UART->Data = 'G';

    do {
        ulong clk1;
        
        clk1 = kerSysGetCycleCount();		/* time cleared */
	/* wait a little to get new reading */
        while ((kerSysGetCycleCount()-clk1) < CYCLE_PER_US*2)
            ;
     } while ((PERF->IrqStatus & (1 << (INTERRUPT_ID_DG - INTERNAL_ISR_TABLE_OFFSET))) && ((kerSysGetCycleCount() - clk0) < DG_GLITCH_TO));

    if (!(PERF->IrqStatus & (1 << (INTERRUPT_ID_DG - INTERNAL_ISR_TABLE_OFFSET)))) {
        BcmHalInterruptEnable( INTERRUPT_ID_DG );
        KERSYS_DBG(" - Power glitch detected. Duration: %ld us\n", (kerSysGetCycleCount() - clk0)/CYCLE_PER_US);
        return 0;
    }

    return 1;
}

static void kerSysDyingGaspShutdown( void )
{
    kerSysSetWdTimer(1000000);
#if defined(CONFIG_BCM96348)
    PERF->blkEnables &= ~(EMAC_CLK_EN | USBS_CLK_EN | USBH_CLK_EN | SAR_CLK_EN);
#elif defined(CONFIG_BCM96358) 
    PERF->blkEnables &= ~(EMAC_CLK_EN | USBS_CLK_EN | SAR_CLK_EN);
#endif
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
static irqreturn_t kerSysDyingGaspIsr(int irq, void * dev_id, struct pt_regs * regs)
#else
static unsigned int kerSysDyingGaspIsr(void)
#endif
{	
    struct list_head *pos;
    CB_DGASP_LIST *tmp = NULL, *dsl = NULL;	

    if (kerSysDyingGaspCheckPowerLoss()) {        

        /* first to turn off everything other than dsl */        
        list_for_each(pos, &g_cb_dgasp_list_head->list) {    	
            tmp = list_entry(pos, CB_DGASP_LIST, list);
    	    if(strncmp(tmp->name, "dsl", 3)) {
    	        (tmp->cb_dgasp_fn)(tmp->context); 
    	    }else {
    		dsl = tmp;    		    	
    	    }       
        }  
        
        /* now send dgasp */
        if(dsl)
            (dsl->cb_dgasp_fn)(dsl->context); 

        /* reset and shutdown system */
        kerSysDyingGaspShutdown();

        // If power is going down, nothing should continue!

        while (1)
            ;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
return( IRQ_HANDLED );
#else
    return( 1 );
#endif
}

static void __init kerSysInitDyingGaspHandler( void )
{
    CB_DGASP_LIST *new_node;

    if( g_cb_dgasp_list_head != NULL) {
        printk("Error: kerSysInitDyingGaspHandler: list head is not null\n");
        return;	
    }
    new_node= (CB_DGASP_LIST *)kmalloc(sizeof(CB_DGASP_LIST), GFP_KERNEL);
    memset(new_node, 0x00, sizeof(CB_DGASP_LIST));
    INIT_LIST_HEAD(&new_node->list);    
    g_cb_dgasp_list_head = new_node; 
		
} /* kerSysInitDyingGaspHandler */

static void __exit kerSysDeinitDyingGaspHandler( void )
{
    struct list_head *pos;
    CB_DGASP_LIST *tmp; 
     	
    if(g_cb_dgasp_list_head == NULL)
        return;
        
    list_for_each(pos, &g_cb_dgasp_list_head->list) {    	
    	tmp = list_entry(pos, CB_DGASP_LIST, list);
        list_del(pos);
	kfree(tmp);
    }       

    kfree(g_cb_dgasp_list_head);	
    g_cb_dgasp_list_head = NULL;
    
} /* kerSysDeinitDyingGaspHandler */

void kerSysRegisterDyingGaspHandler(char *devname, void *cbfn, void *context)
{
    CB_DGASP_LIST *new_node;

    if( g_cb_dgasp_list_head == NULL) {
        printk("Error: kerSysRegisterDyingGaspHandler: list head is null\n");	
        return;    
    }
    
    if( devname == NULL || cbfn == NULL ) {
        printk("Error: kerSysRegisterDyingGaspHandler: register info not enough (%s,%x,%x)\n", devname, (unsigned int)cbfn, (unsigned int)context);	    	
        return;
    }
       
    new_node= (CB_DGASP_LIST *)kmalloc(sizeof(CB_DGASP_LIST), GFP_KERNEL);
    memset(new_node, 0x00, sizeof(CB_DGASP_LIST));    
    INIT_LIST_HEAD(&new_node->list);
    strncpy(new_node->name, devname, IFNAMSIZ);
    new_node->cb_dgasp_fn = (cb_dgasp_t)cbfn;
    new_node->context = context;
    list_add(&new_node->list, &g_cb_dgasp_list_head->list);
    
    printk("dgasp: kerSysRegisterDyingGaspHandler: %s registered \n", devname);
        	
} /* kerSysRegisterDyingGaspHandler */

void kerSysDeregisterDyingGaspHandler(char *devname)
{
    struct list_head *pos;
    CB_DGASP_LIST *tmp;    
    
    if(g_cb_dgasp_list_head == NULL) {
        printk("Error: kerSysDeregisterDyingGaspHandler: list head is null\n");
        return;	
    }

    if(devname == NULL) {
        printk("Error: kerSysDeregisterDyingGaspHandler: devname is null\n");
        return;	
    }
    
    printk("kerSysDeregisterDyingGaspHandler: %s is deregistering\n", devname);

    list_for_each(pos, &g_cb_dgasp_list_head->list) {    	
    	tmp = list_entry(pos, CB_DGASP_LIST, list);
    	if(!strcmp(tmp->name, devname)) {
            list_del(pos);
	    kfree(tmp);
	    printk("kerSysDeregisterDyingGaspHandler: %s is deregistered\n", devname);
	    return;
	}
    }	
    printk("kerSysDeregisterDyingGaspHandler: %s not (de)registered\n", devname);
	
} /* kerSysDeregisterDyingGaspHandler */

static int ConfigCs (BOARD_IOCTL_PARMS *parms)
{
    int                     retv = 0;
#if !defined(CONFIG_BCM96338)
    int                     cs, flags;
    cs_config_pars_t        info;

    if (copy_from_user(&info, (void*)parms->buf, sizeof(cs_config_pars_t)) == 0) 
    {
        cs = parms->offset;

        MPI->cs[cs].base = ((info.base & 0x1FFFE000) | (info.size >> 13));	

        if ( info.mode == EBI_TS_TA_MODE )     // syncronious mode
            flags = (EBI_TS_TA_MODE | EBI_ENABLE);
        else
        {
            flags = ( EBI_ENABLE | \
                (EBI_WAIT_STATES  & (info.wait_state << EBI_WTST_SHIFT )) | \
                (EBI_SETUP_STATES & (info.setup_time << EBI_SETUP_SHIFT)) | \
                (EBI_HOLD_STATES  & (info.hold_time  << EBI_HOLD_SHIFT )) );
        }
        MPI->cs[cs].config = flags;
        parms->result = BP_SUCCESS;
        retv = 0;
    }
    else
    {
        retv -= EFAULT;
        parms->result = BP_NOT_DEFINED; 
    }
#endif
    return( retv );
}

static void SetPll (int pll_mask, int pll_value)
{
    PERF->pll_control &= ~pll_mask;   // clear relevant bits
    PERF->pll_control |= pll_value;   // and set desired value
}

static void SetGpio(int gpio, GPIO_STATE_t state)
{
    unsigned long gpio_mask = GPIO_NUM_TO_MASK(gpio);
    volatile unsigned long *gpio_io_reg = &GPIO->GPIOio;
    volatile unsigned long *gpio_dir_reg = &GPIO->GPIODir;
    
#if !defined (CONFIG_BCM96338)
    if( gpio >= 32 )
    {
        gpio_mask = GPIO_NUM_TO_MASK_HIGH(gpio);
        gpio_io_reg = &GPIO->GPIOio_high;
        gpio_dir_reg = &GPIO->GPIODir_high;
    }
#endif 

    *gpio_dir_reg |= gpio_mask;

    if(state == GPIO_HIGH)
        *gpio_io_reg |= gpio_mask;
    else
        *gpio_io_reg &= ~gpio_mask;
}

/* start of y42304 added 20060814: 为了实现atoi()函数的功能 */
unsigned long board_xtoi(const char *dest)
{
    unsigned long  x = 0;
    unsigned int digit;

    if ((*dest == '0') && (*(dest+1) == 'x')) dest += 2;

    while (*dest)
    {
	    if ((*dest >= '0') && (*dest <= '9'))
        {
            digit = *dest - '0';
	    }
	    else if ((*dest >= 'A') && (*dest <= 'F')) 
        {
            digit = 10 + *dest - 'A';
	    }
    	else if ((*dest >= 'a') && (*dest <= 'f')) 
        {
            digit = 10 + *dest - 'a';
	    }
	    else 
        {
            break;
	    }
    	x *= 16;
	    x += digit;
	    dest++;
	}
    return x;
}


int board_atoi(const char *dest)
{
    int x = 0;
    int digit;

    if ((*dest == '0') && (*(dest+1) == 'x')) 
    {
	    return board_xtoi(dest+2);
	}

    while (*dest) 
    {
	    if ((*dest >= '0') && (*dest <= '9')) 
        {
	        digit = *dest - '0';
	    }
    	else 
        {
            break;
	    }
   	    x *= 10;
	    x += digit;
	    dest++;
	}
    return x;
}
/* end of y42304 added 20060814: 为了实现atoi()函数的功能 */

/**************************************************************************
 * 函数名  :   kerSysGetFsImageFromFlash
 * 功能    :   提供给应用从FLASH读取fs文件系统
 *
 * 输入参数:   string:  存储内容的buf
               strLen:  buf的字节长度
               offSet:  从文件系统开始位置的偏移量
 * 输出参数:   无
 *
 * 返回值  :   0:   正确
               -1:  错误
 * 作者    :   yuyouqing42304
 * 修改历史:   2006-08-15创建  
 ***************************************************************************/
int kerSysGetFsImageFromFlash(char *string, int strLen, int offSet)
{    
    unsigned long ulFsflashAddr = 0;
    PFILE_TAG stFileTag = getTagFromPartition(1);   
    if ((string == NULL) || (stFileTag == NULL))
    {
        return -1;
    }

    ulFsflashAddr = board_atoi(stFileTag->rootfsAddress) + BOOT_OFFSET;     
   
    kerSysReadFromFlash((void *)string, ulFsflashAddr+offSet, strLen);      

    return 0;
        
}

/**************************************************************************
 * 函数名  :   kerSysGetKernelImageFromFlash
 * 功能    :   提供给应用从FLASH读取kernel
 *
 * 输入参数:   string:  存储内容的buf
               strLen:  buf的字节长度
               offSet:  从kernel开始位置的偏移量
 * 输出参数:   无
 *
 * 返回值  :   0:   正确
               -1:  错误
 * 作者    :   yuyouqing42304
 * 修改历史:   2006-08-15创建  
 ***************************************************************************/
int kerSysGetKernelImageFromFlash(char *string, int strLen, int offSet)
{    
    unsigned long ulKernelflashAddr = 0;
    PFILE_TAG stFileTag = getTagFromPartition(1);
    
    if ((string == NULL) || (stFileTag == NULL))
    {
        return -1;
    }    
    ulKernelflashAddr = board_atoi(stFileTag->kernelAddress) + BOOT_OFFSET;

    kerSysReadFromFlash((void *)string, ulKernelflashAddr+offSet, strLen);    
    
    return 0;    
}

/**************************************************************************
 * 函数名  :   kerSysSetGPIO
 * 功能    :   读取给定GPIO的值
 *
 * 输入参数:   ucGpioNum:  GPIO号
 * 输出参数:   无
 *
 * 返回值  :    正确  :  GPIO值，高电平或低电平
 *
 * 作者    :    yuyouqing42304
 * 修改历史:    2006-05-16创建  
 ***************************************************************************/
void kerSysSetGPIO(unsigned short ucGpioNum,GPIO_STATE_t state)
{  
    SetGpio( ucGpioNum, state);
}

static PFILE_TAG getBootImageTagAndSeq(unsigned long* pulSeq)
{
    PFILE_TAG pTag = NULL;
    PFILE_TAG pTag1 = getTagFromPartition(1);
    PFILE_TAG pTag2 = getTagFromPartition(2);

    if( pTag1 && pTag2 )
    {
        /* Two images are flashed. */
        int sequence1 = simple_strtoul(pTag1->imageSequence, NULL, 10);
        int sequence2 = simple_strtoul(pTag2->imageSequence, NULL, 10);
        char *p;
        char bootPartition = BOOT_LATEST_IMAGE;
        NVRAM_DATA nvramData;

        memcpy((char *) &nvramData, (char *) get_nvram_start_addr(),
            sizeof(nvramData));
        for( p = nvramData.szBootline; p[2] != '\0'; p++ )
            if( p[0] == 'p' && p[1] == '=' )
            {
                bootPartition = p[2];
                break;
            }

        if( bootPartition == BOOT_LATEST_IMAGE )
        {
            pTag    = (sequence2 > sequence1) ? pTag2 : pTag1;
            *pulSeq = (sequence2 > sequence1) ? 2 : 1;
        }
        else /* Boot from the image configured. */
        {
            pTag    = (sequence2 < sequence1) ? pTag2 : pTag1;
            *pulSeq = (sequence2 < sequence1) ? 2 : 1;
        }
    }
    else
    {
        /* One image is flashed. */
        pTag = (pTag2) ? pTag2 : pTag1;
        *pulSeq = (pTag2) ? 2 : 1;
    }

    return( pTag );
}


/*****************************************************************************
 函 数 名  : kerSysCheckFsCrc
 功能描述  : 校验文件系统crc
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月16日
    作    者   : liuyang 65130
    修改内容   : 新生成函数

*****************************************************************************/
PFILE_TAG  kerSysGetBootFs(void)
{
    u_int32_t rootfs_addr, kernel_addr, root_len;
    unsigned long seq = 1;
    PFILE_TAG pTag  = getBootImageTagAndSeq(&seq);
    char* pSectAddr = NULL;
    int blk = 0;
    UINT32 crc = CRC32_INIT_VALUE;

    if (pTag)
    {
        rootfs_addr = board_atoi(pTag->rootfsAddress) + BOOT_OFFSET;
        kernel_addr = board_atoi(pTag->kernelAddress) + BOOT_OFFSET;

        printk("^^^^^^^^TAG1 rootfs_addr: %x \r\n", rootfs_addr);
        
        root_len = board_atoi(pTag->rootfsLen);
        
        if (root_len > 0)
        {

            printk("^^^^^^^^root_len: %d \r\n", root_len);
            pSectAddr = retriedKmalloc(root_len);

            if (NULL == pSectAddr)
            {
                printk("~~~~~~~~~~Not enough memory.\r\n");
                return NULL;
            }
            memset(pSectAddr, 0, root_len);

            kerSysReadFromFlash(pSectAddr, rootfs_addr, root_len);
            crc = getCrc32(pSectAddr, root_len, crc);      
            if (crc != *(unsigned long *) (pTag->imageValidationToken+CRC_LEN))
            {

                printk("^^^^^^^^ tag%d crc wrong\r\n", seq);
                seq = ((seq == 1) ? 2 : 1);
                pTag = getTagFromPartition(&seq);
                printk("^^^^^^^^TAG2 rootfs_addr: %x \r\n", rootfs_addr);
            }

            if (pSectAddr)
            {
                kfree(pSectAddr);
            }
        }
    }
    else
    {
        seq = ((seq == 1) ? 2 : 1);
        pTag = getTagFromPartition(2);
    }
    
    printk("^^^^^^^^TAG rootfs_addr: %d \r\n", rootfs_addr);
    return pTag;
}
EXPORT_SYMBOL(kerSysGetBootFs);


/***************************************************************************
 * MACRO to call driver initialization and cleanup functions.
 ***************************************************************************/
module_init( brcm_board_init );
module_exit( brcm_board_cleanup );

EXPORT_SYMBOL(kerSysNvRamGet);
EXPORT_SYMBOL(dumpaddr);
EXPORT_SYMBOL(kerSysGetMacAddress);
EXPORT_SYMBOL(kerSysReleaseMacAddress);
EXPORT_SYMBOL(kerSysGetSdramSize);
EXPORT_SYMBOL(kerSysLedCtrl);
EXPORT_SYMBOL(kerSysLedRegisterHwHandler);
EXPORT_SYMBOL(BpGetBoardIds);
EXPORT_SYMBOL(BpGetSdramSize);
EXPORT_SYMBOL(BpGetPsiSize);
EXPORT_SYMBOL(BpGetEthernetMacInfo);
EXPORT_SYMBOL(BpGetRj11InnerOuterPairGpios);
EXPORT_SYMBOL(BpGetPressAndHoldResetGpio);
EXPORT_SYMBOL(BpGetVoipResetGpio);
EXPORT_SYMBOL(BpGetVoipIntrGpio);
EXPORT_SYMBOL(BpGetRtsCtsUartGpios);
EXPORT_SYMBOL(BpGetAdslLedGpio);
EXPORT_SYMBOL(BpGetAdslFailLedGpio);
EXPORT_SYMBOL(BpGetWirelessLedGpio);
EXPORT_SYMBOL(BpGetUsbLedGpio);
EXPORT_SYMBOL(BpGetHpnaLedGpio);
EXPORT_SYMBOL(BpGetWanDataLedGpio);
EXPORT_SYMBOL(BpGetPppLedGpio);
EXPORT_SYMBOL(BpGetPppFailLedGpio);
EXPORT_SYMBOL(BpGetVoipLedGpio);
EXPORT_SYMBOL(BpGetAdslDyingGaspExtIntr);
EXPORT_SYMBOL(BpGetVoipExtIntr);
EXPORT_SYMBOL(BpGetHpnaExtIntr);
EXPORT_SYMBOL(BpGetHpnaChipSelect);
EXPORT_SYMBOL(BpGetVoipChipSelect);
EXPORT_SYMBOL(BpGetWirelessSesBtnGpio);
EXPORT_SYMBOL(BpGetWirelessSesExtIntr);
EXPORT_SYMBOL(BpGetWirelessSesLedGpio);
EXPORT_SYMBOL(BpGetWirelessFlags);
EXPORT_SYMBOL(BpUpdateWirelessSromMap);
EXPORT_SYMBOL(kerSysRegisterDyingGaspHandler);
EXPORT_SYMBOL(kerSysDeregisterDyingGaspHandler);
EXPORT_SYMBOL(kerSysGetCycleCount);
EXPORT_SYMBOL(kerSysSetWdTimer);
EXPORT_SYMBOL(kerSysWakeupMonitorTask);
EXPORT_SYMBOL(GetBoardVersion);
EXPORT_SYMBOL(GetHarewareType);
EXPORT_SYMBOL(kerSysReadFromFlash);
EXPORT_SYMBOL(BpGetTelLine0LedGpio);
EXPORT_SYMBOL(BpGetTelLine1LedGpio);
EXPORT_SYMBOL(BpGetFxoLoopDetcGpio);
EXPORT_SYMBOL(BpGetFxoRingDetcGpio);
EXPORT_SYMBOL(BpGetFxoRelayCtlGpio);
EXPORT_SYMBOL(kerSysSetGPIO);
EXPORT_SYMBOL(kerSysGetGPIO);
EXPORT_SYMBOL(BpGetSlicType);
EXPORT_SYMBOL(BpGetDAAType);
