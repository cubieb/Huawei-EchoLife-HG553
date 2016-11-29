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
/***********************************************************************/
/*                                                                     */
/*   MODULE:  board.h                                                  */
/*   PURPOSE: Board specific information.  This module should include  */
/*            all base device addresses and board specific macros.     */
/*                                                                     */
/***********************************************************************/
#ifndef _BOARD_H
#define _BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#define	DYING_GASP_API

/*****************************************************************************/
/*                    Physical Memory Map                                    */
/*****************************************************************************/

#define PHYS_DRAM_BASE          0x00000000     /* Dynamic RAM Base */
#define PHYS_FLASH_BASE         0x1FC00000     /* Flash Memory     */

#define HPI_BASE	        0x1D000000     /* Host Processor Interface base address */
#define HPI_SIZE	        0x8000         /* 32K */

/*****************************************************************************/
/* Note that the addresses above are physical addresses and that programs    */
/* have to use converted addresses defined below:                            */
/*****************************************************************************/
#define DRAM_BASE           (0x80000000 | PHYS_DRAM_BASE)   /* cached DRAM */
#define DRAM_BASE_NOCACHE   (0xA0000000 | PHYS_DRAM_BASE)   /* uncached DRAM */

/* Binary images are always built for a standard MIPS boot address */
#define IMAGE_BASE          (0xA0000000 | 0x1FC00000)

/* Some chips support alternative boot vector */
#if defined(_BCM96348_) || defined(CONFIG_BCM96348) || defined(_BCM96358_) || defined(CONFIG_BCM96358)
#define FLASH_BASE          (0xA0000000 | (MPI->cs[0].base & 0xFFFFFF00))
#define BOOT_OFFSET         (FLASH_BASE - IMAGE_BASE)
#else
#define FLASH_BASE          (0xA0000000 | PHYS_FLASH_BASE)  /* uncached Flash  */
#define BOOT_OFFSET         0
#endif

/*****************************************************************************/
/*  Select the PLL value to get the desired CPU clock frequency.             */
/*****************************************************************************/
#define FPERIPH            50000000

/*****************************************************************************/
/* Board memory type offset                                                  */
/*****************************************************************************/
#define SDRAM_TYPE_ADDRESS_OFFSET       16
#define THREAD_NUM_ADDRESS_OFFSET       20
#define NVRAM_DATA_OFFSET               0x0580
#define NVRAM_DATA_ID                   0x0f1e2d3c
#define BOARD_SDRAM_TYPE                *(unsigned long *)(FLASH_BASE + SDRAM_TYPE_ADDRESS_OFFSET)
#define BOARD_SDRAM_TYPE_ADDRESS        (0xA0000000 + PHYS_FLASH_BASE + SDRAM_TYPE_ADDRESS_OFFSET)
#define BOARD_THREAD_NUM                *(unsigned long *)(FLASH_BASE + THREAD_NUM_ADDRESS_OFFSET)
#define BOARD_THREAD_NUM_ADDRESS        (0xA0000000 + PHYS_FLASH_BASE + THREAD_NUM_ADDRESS_OFFSET)
#define BOARD_IMAGE_DOWNLOAD_ADDRESS    \
    ((cfe_sdramsize > 0x00800000) ? 0x80800000 : 0x80000000) 
#define BOARD_IMAGE_DOWNLOAD_SIZE       \
    ((cfe_sdramsize > 0x00800000) ? 0x00800000 : 0x00400000) 

/*****************************************************************************/
/*       NVRAM Offset and definition                                         */
/*****************************************************************************/
#define NVRAM_PSI_DEFAULT               64              // default psi in K byes
#define ONEK                            1024
#define FLASH_LENGTH_BOOT_ROM           (128*ONEK)

//#define FLASH_RESERVED_AT_END           (64*ONEK) /*reserved for PSI, scratch pad*/
#define FLASH_RESERVED_AT_END           (7*128*ONEK) /*reserved for parameter area*/

#define NVRAM_LENGTH                    ONEK            // 1k nvram 
#define NVRAM_VERSION_NUMBER            2
#define NVRAM_VERSION_NUMBER_ADDRESS    0

#define NVRAM_BOOTLINE_LEN              256
#define NVRAM_BOARD_ID_STRING_LEN       16
#define NVRAM_MAC_ADDRESS_LEN           6
#define NVRAM_MAC_COUNT_MAX             32
#define NVRAM_MAC_COUNT_DEFAULT         0 
#define NVRAM_SERIAL_NUMBER_LEN         32

/*Start -- Normal System & Small System -20091201*/
#define FLASH_LENGTH_NORMAL_SYSTEM           (80*128*ONEK)//10M
#define FLASH_LENGTH_SMALL_SYSTEM           (37*128*ONEK)//37 blocks, 4.5M
/*
 * Flash Maps (16 MB Total)
 *
 |---------------------------------------|
 | CFE                      (128KB)       |
 |---------------------------------------|
 |                                       |
 | Normal System         (80*128KB)     |
 |                                       |
 |---------------------------------------|
 |                                       |
 | Small System          (37*128KB)    |
 |                                       |
 |---------------------------------------|
 | Reserved for Parm     (3*128 KB)      |
 |---------------------------------------|
 | Parm Area			(7*128KB)
 |---------------------------------------|
 *
 */

#define TAGVERSION_HW_OLD 7			//old hg553 TagVersion 
#define TAGVERSION_HW_NEW 8			//new hg553 TagVersion for using Normal & Small System
typedef enum 
{
	TAGVERSION_FLAG_NORMAL_FW = 1,
	
	TAGVERSION_FLAG_SMALL_FW,
	
	TAGVERSION_FLAG_PACKAGED_FW,	//both Normal and Small system inside
	
	TAGVERSION_FLAG_TEMP_FW,
}TAGVERSION_FLAG;



/*End -- Normal System & Small System -20091201*/

/*****************************************************************************/
/*       Misc Offsets                                                        */
/*****************************************************************************/
#define CFE_VERSION_OFFSET           0x0570
#define CFE_VERSION_MARK_SIZE        5
#define CFE_VERSION_SIZE             9

#define BOOT_LATEST_IMAGE   '0'
#define BOOT_PREVIOUS_IMAGE '1'

typedef struct
{
    unsigned long ulVersion;
    char szBootline[NVRAM_BOOTLINE_LEN];
    char szBoardId[NVRAM_BOARD_ID_STRING_LEN];
    unsigned long ulReserved1[2];
    unsigned long ulNumMacAddrs;
    unsigned char ucaBaseMacAddr[NVRAM_MAC_ADDRESS_LEN];
    char chReserved[2];
    char szSerialNumber[NVRAM_SERIAL_NUMBER_LEN];
    unsigned long ulCheckSum;
} NVRAM_DATA, *PNVRAM_DATA;

/*****************************************************************************/
/*       Reserve MAC for VDF requirement                                     */
/*****************************************************************************/
#define RESERVE_MAC_MASK 0xFF000000
#define NO_RESERVE_MAC 0xFFFFFFFF

/*****************************************************************************/
/*          board ioctl calls for flash, led and some other utilities        */
/*****************************************************************************/


/* Defines. for board driver */
#define BOARD_IOCTL_MAGIC       'B'
#define BOARD_DRV_MAJOR          206
#define NOT_SUPPROT_GET_VERSION  0xFF

#define MAC_ADDRESS_ANY         (unsigned long) -1

#define BOARD_IOCTL_FLASH_INIT \
    _IOWR(BOARD_IOCTL_MAGIC, 0, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_FLASH_WRITE \
    _IOWR(BOARD_IOCTL_MAGIC, 1, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_FLASH_READ \
    _IOWR(BOARD_IOCTL_MAGIC, 2, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_NR_PAGES \
    _IOWR(BOARD_IOCTL_MAGIC, 3, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_DUMP_ADDR \
    _IOWR(BOARD_IOCTL_MAGIC, 4, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_SET_MEMORY \
    _IOWR(BOARD_IOCTL_MAGIC, 5, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_MIPS_SOFT_RESET \
    _IOWR(BOARD_IOCTL_MAGIC, 6, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_LED_CTRL \
    _IOWR(BOARD_IOCTL_MAGIC, 7, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_ID \
    _IOWR(BOARD_IOCTL_MAGIC, 8, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_MAC_ADDRESS \
    _IOWR(BOARD_IOCTL_MAGIC, 9, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_RELEASE_MAC_ADDRESS \
    _IOWR(BOARD_IOCTL_MAGIC, 10, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_PSI_SIZE \
    _IOWR(BOARD_IOCTL_MAGIC, 11, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_SDRAM_SIZE \
    _IOWR(BOARD_IOCTL_MAGIC, 12, BOARD_IOCTL_PARMS)

/* 2008/01/28 Jiajun Weng : Modify its definition for WPS PBC trigger */

#define BOARD_IOCTL_SET_MONITOR_FD \
    _IOWR(BOARD_IOCTL_MAGIC, 17, BOARD_IOCTL_PARMS)
    
#define BOARD_IOCTL_WAKEUP_MONITOR_TASK \
    _IOWR(BOARD_IOCTL_MAGIC, 18, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_BOOTLINE \
    _IOWR(BOARD_IOCTL_MAGIC, 15, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_SET_BOOTLINE \
    _IOWR(BOARD_IOCTL_MAGIC, 16, BOARD_IOCTL_PARMS)
/* 2008/01/28 Jiajun Weng : Modify its definition for WPS PBC trigger */


#define BOARD_IOCTL_GET_BASE_MAC_ADDRESS \
    _IOWR(BOARD_IOCTL_MAGIC, 13, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_CHIP_ID \
    _IOWR(BOARD_IOCTL_MAGIC, 14, BOARD_IOCTL_PARMS)

/* 2008/01/28 Jiajun Weng : Modify its definition for WPS PBC trigger */
#define BOARD_IOCTL_GET_NUM_ENET \
    _IOWR(BOARD_IOCTL_MAGIC, 23, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_CFE_VER \
    _IOWR(BOARD_IOCTL_MAGIC, 24, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_ENET_CFG \
    _IOWR(BOARD_IOCTL_MAGIC, 25, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_WLAN_ANT_INUSE \
    _IOWR(BOARD_IOCTL_MAGIC, 22, BOARD_IOCTL_PARMS)

/* 2008/01/28 Jiajun Weng : Modify its definition for WPS PBC trigger */
#define BOARD_IOCTL_SET_TRIGGER_EVENT \
    _IOWR(BOARD_IOCTL_MAGIC, 19, BOARD_IOCTL_PARMS)        

#define BOARD_IOCTL_GET_TRIGGER_EVENT \
    _IOWR(BOARD_IOCTL_MAGIC, 20, BOARD_IOCTL_PARMS)        

#define BOARD_IOCTL_UNSET_TRIGGER_EVENT \
    _IOWR(BOARD_IOCTL_MAGIC, 21, BOARD_IOCTL_PARMS) 

#define BOARD_IOCTL_SET_SES_LED \
    _IOWR(BOARD_IOCTL_MAGIC, 26, BOARD_IOCTL_PARMS)
    
#define BOARD_IOCTL_GET_VCOPE_GPIO \
    _IOWR(BOARD_IOCTL_MAGIC, 27, BOARD_IOCTL_PARMS)
    
#define BOARD_IOCTL_SET_CS_PAR \
    _IOWR(BOARD_IOCTL_MAGIC, 28, BOARD_IOCTL_PARMS)
    
#define BOARD_IOCTL_SET_PLL \
    _IOWR(BOARD_IOCTL_MAGIC, 29, BOARD_IOCTL_PARMS)
    
#define BOARD_IOCTL_SET_GPIO \
    _IOWR(BOARD_IOCTL_MAGIC, 30, BOARD_IOCTL_PARMS)
  
#define BOARD_IOCTL_GET_DYINGGASP \
    _IOWR(BOARD_IOCTL_MAGIC, 39, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_PUT_WEB_OP \
    _IOWR(BOARD_IOCTL_MAGIC, 31, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_PUT_CLI_OP \
    _IOWR(BOARD_IOCTL_MAGIC, 32, BOARD_IOCTL_PARMS)
    
#define BOARD_IOCTL_PUT_ITF_STATE \
    _IOWR(BOARD_IOCTL_MAGIC, 33, BOARD_IOCTL_PARMS)
    
#define  BOARD_IOCTL_EQUIPMENT_TEST \
    _IOWR(BOARD_IOCTL_MAGIC, 34, BOARD_IOCTL_PARMS)     

#define BOARD_IOCTL_CHECK_WLAN_STATUS \
    _IOWR(BOARD_IOCTL_MAGIC, 35, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_SET_VOIP_SERVICE \
    _IOWR(BOARD_IOCTL_MAGIC, 36, BOARD_IOCTL_PARMS)          

#define BOARD_IOCTL_GET_VOIP_SERVICE \
    _IOWR(BOARD_IOCTL_MAGIC, 37, BOARD_IOCTL_PARMS)          

/* HUAWEI HGW s48571 2008��1��19�� Hardware Porting add begin:*/
#define BOARD_IOCTL_SET_TRAFFIC_MODE \
        _IOWR(BOARD_IOCTL_MAGIC, 38, BOARD_IOCTL_PARMS)  

/* HUAWEI HGW s48571 2008��1��19�� Hardware Porting add end.*/

/* HUAWEI HGW s48571 2008��2��20�� װ������״̬��־��� add begin:*/
#define BOARD_IOCTL_SET_EQUIPTEST_MODE \
        _IOWR(BOARD_IOCTL_MAGIC, 40, BOARD_IOCTL_PARMS)  
/* HUAWEI HGW s48571 2008��2��20�� װ������״̬��־��� add end.*/

     
// for the action in BOARD_IOCTL_PARMS for flash operation
typedef enum 
{
    PERSISTENT,
    NVRAM,
    BCM_IMAGE_CFE,
    BCM_IMAGE_FS,
    BCM_IMAGE_KERNEL,
    BCM_IMAGE_WHOLE,
    SCRATCH_PAD,
    FLASH_SIZE,
    SET_CS_PARAM,
    GET_FILE_TAG_FROM_FLASH,
    VARIABLE,  
    /* begin --- Add Persistent storage interface backup flash partition by w69233 */
    PSI_BACKUP,
    /* end ----- Add Persistent storage interface backup flash partition by w69233 */
    /* begin --- Add partition by l65130 */
    FIX,
    AVAIL,
    /* end ----- Add partition by l65130 */

	/*Start -- Normal System & Small System -20091201*/						
	BCM_IMAGE_FS_NO_REBOOT,
	/*End -- Normal System & Small System -20091201*/
	
} BOARD_IOCTL_ACTION;

/* start of BOARD update by y42304 20060623: ��װ�������йصĶ��� */
typedef enum 
{
    SET_BASE_MAC_ADDRESS,
    GET_BASE_MAC_ADDRESS,
    SET_MAC_AMOUNT,
    GET_MAC_AMOUNT,
    GET_ALL_BOARD_ID_NAME,
    GET_PCB_VERSION,
    GET_BOARD_VERSION,
    SET_BOARD_ID,
    GET_BOARD_ID,
    SET_SERIAL_NUMBER,
    GET_SERIAL_NUMBER,
    GET_CPU_REVISION_ID,
    GET_BOARD_MANUFACTURER_NAME,
    BUTT,    
} BOARD_EQUIPMENT_TEST_ACTION;
/* end of equip test added by y42304 20060623*/
    
    
typedef struct boardIoctParms
{
    char *string;
    char *buf;
    int strLen;
    int offset;
    BOARD_IOCTL_ACTION  action;        /* flash read/write: nvram, persistent, bcm image */
    int result;
} BOARD_IOCTL_PARMS;


// LED defines 
typedef enum
{   
    kLedAdsl,
    kLedWireless,
    kLedUsb,
    kLedHpna,
    kLedWanData,
    kLedWanDataFail,
    kLedPPP,
    kLedVoip,
    kLedSes,
    kLedPower,
    kLedLine0,
    kLedLine1,    
    kLedPSTN,
    kLedInternet,
    kLedHspa,
    kLedEnd,                // NOTE: Insert the new led name before this one.  Alway stay at the end.
} BOARD_LED_NAME;

typedef enum
{
    kLedStateOff,                        /* turn led off */
    kLedStateOn,                         /* turn led on */
    kLedStateFail,                       /* turn led on red */
    kLedStateBlinkOnce,                  /* blink once, ~100ms and ignore the same call during the 100ms period */
    kLedStateSlowFlickerOnce,            /* Slow Flicker once, VDF customized and ignore the same call during blink period */
    kLedStateFastFlickerOnce,            /* Fast Flicker once, VDF customized and ignore the same call during blink period */
    kLedStateVerySlowBlinkContiunes,     /* Jiajun 12/18/2007: slow blink contines at ~500ms interval */
    kLedStateSlowBlinkContinues,         /* slow blink continues at ~600ms interval */
    kLedStateFastBlinkContinues,         /* fast blink continues at ~200ms interval */
    kLedStateSlowFlickerOnce_Pause,            /* Slow Flicker once with pause, VDF customized and ignore the same call during blink period */
    kLedStateFastFlickerOnce_Pause,            /* Fast Flicker once with pause, VDF customized and ignore the same call during blink period */    
} BOARD_LED_STATE;

typedef enum
{
  GPIO_LOW = 0,
  GPIO_HIGH,
} GPIO_STATE_t;


typedef enum
{
    GPIO_LOW32 = 0,   /* GPIO��ΪGPIO0-GPIO31 */
    GPIO_HIGH_BIT,    /* GPIO��ΪGPI32������ */
}GPIO_NUM;

/*start of board added by y42304 20060708: Ӳ���汾��*/
typedef enum 
{
    /*
     *  GPIO8 GPIO7 GPIO6
     *    0    0     0   --> HG52VAG     
     *    0    0     1   --> HG52VAGB        
     *    1    0     0   --> HG51VAG
     *    1    1     0   --> HG51VAGI
     */
    HG52VAG  = 0,   
    HG52VAGB = 1,            
    HG51VAG  = 4,
    HG51VAGI = 6,
}E_BOARD_VERSION;

typedef enum 
{
    /*
     *  GPIO8 GPIO7 GPIO6
     *    0    0     0   --> HG55magc
     *    0    0     1   --> HG55magd        
     ��������
     HG55magcʵ�֣�BCM6358 + BCM5325 + BCM4318 + Si3210 + BCM6301
     HG55magdʵ�֣�BCM6358 + ADM6996 + BCM4318 + Si3215 + BCM6301 + Si3050(+Si3011)       
     */
    HG55MAGC  = 0,   
    HG55MAGD  = 1,           
    HG55VAGII = 0x1F,
}E_BOARD_ADSL_VERSION;
/*end of board added by y42304 20060708: Ӳ���汾��*/

/* HUAWEI HGW s48571 2008��1��18�� Hardware Porting add begin:*/
typedef enum 
{
    /*
     *   GPIO7 GPIO6
     *    0     0   --> HG55MAGV
     ��������
     HG55magcʵ�֣�BCM6358 + BCM5325 + BCM4318 + LE88221 
     ����VDF�������������
     */
    HG55MAGV  = 0,   
}E_BOARD_VDF_VERSION;

typedef enum 
{
    MODE_NONE = 0,
    MODE_2G,
    MODE_3G,
    MODE_PASSBY,
    MODE_NORM,
    MODE_UNDEFINED,                
}E_HSPA_TRAFFICMODE_VERSION;



/* HUAWEI HGW s48571 2008��1��18�� Hardware Porting add end.*/



/*start of board added by y42304 20060810: Ӳ�������̺� */
typedef enum 
{
    /*
     *  GPIO35 GPIO33
     *    1    1         --> Huawei
     *    1    0         --> Alpha    
     */     
    MANUFACTURER_ALPHA   = 2,    
    MANUFACTURER_HUAWEI  = 3,   
}E_BOARD_MANUFACTURER;
/*end of board added by y42304 20060810: Ӳ�������̺� */


// virtual and physical map pair defined in board.c
typedef struct ledmappair
{
    BOARD_LED_NAME ledName;         // virtual led name
    BOARD_LED_STATE ledInitState;   // initial led state when the board boots.
    unsigned long ledMask;         // physical GPIO pin mask
    unsigned short ledActiveLow;    // reset bit to turn on LED
    unsigned short ledSerial;       // indicated that LED is driven via serial output
    unsigned long ledMaskFail;     // physical GPIO pin mask for state failure
    unsigned short ledActiveLowFail;// reset bit to turn on LED
    unsigned short ledSerialFail;   // indicated that LED is driven via serial output
    /* start of LED added by y42304 20060517: Ϊ�˽������GPIO32�����ϵ�Ƶ����� */    
    int ledlow32GPIO;               // 0: ��ʾ�õ�GPIO0-GPIO31; 1: ��ʾ�õ���GPIO32�����ϵ�GPIO
    /* end of LED added by y42304 20060517 */
} LED_MAP_PAIR, *PLED_MAP_PAIR;

typedef void (*HANDLE_LED_FUNC)(BOARD_LED_NAME ledName, BOARD_LED_STATE ledState);

/* Flash storage address information that is determined by the flash driver. */
typedef struct flashaddrinfo
{
    int flash_persistent_start_blk;
    int flash_persistent_number_blk;
    int flash_persistent_length;
    unsigned long flash_persistent_blk_offset;
    /* begin --- Add Persistent storage interface backup flash partition by w69233 */
    int flash_psi_backup_start_blk;
    int flash_psi_backup_number_blk;
    int flash_psi_backup_length;
    unsigned long flash_psi_backup_blk_offset;
    /* end ----- Add Persistent storage interface backup flash partition by w69233 */

    /*start of ����FLASH FIX�ķ��� porting by l65130 20080328*/
    int flash_fix_start_blk;
    int flash_fix_number_blk;
    int flash_fix_length;
    unsigned long flash_fix_blk_offset;
    /*end of ����FLASH FIX�ķ��� porting by l65130 20080328*/

    /*start of ����FLASH avail�ķ��� porting by l65130 20080328*/
    int flash_avail_start_blk;
    int flash_avail_number_blk;
    int flash_avail_length;
    unsigned long flash_avail_blk_offset;
    /*end of ����FLASH avail�ķ��� porting by l65130 20080328*/

    /*start of ����FLASH VA�ķ��� porting by l39225 20060504*/
    int flash_variable_start_blk;
    int flash_variable_number_blk;
    int flash_variable_length;
    unsigned long flash_variable_blk_offset;
    /*end of ����FLASH VA�ķ��� porting by l39225 20060504*/
    int flash_scratch_pad_start_blk;         // start before psi (SP_BUF_LEN)
    int flash_scratch_pad_number_blk;
    int flash_scratch_pad_length;
    unsigned long flash_scratch_pad_blk_offset;
    int flash_nvram_start_blk;
    int flash_nvram_number_blk;
    int flash_nvram_length;
    unsigned long flash_nvram_blk_offset;
} FLASH_ADDR_INFO, *PFLASH_ADDR_INFO;

// scratch pad defines
/* SP - Persisten Scratch Pad format:
       sp header        : 32 bytes
       tokenId-1        : 8 bytes
       tokenId-1 len    : 4 bytes
       tokenId-1 data    
       ....
       tokenId-n        : 8 bytes
       tokenId-n len    : 4 bytes
       tokenId-n data    
*/

#define MAGIC_NUM_LEN       8
//#define MAGIC_NUMBER        "gOGoBrCm"
/* start of board y42304 modified 20060720: ���D012��D020�汾�л�����shell������*/
#define MAGIC_NUMBER        "gOGoBD20"
/* end of board y42304 modified 20060720*/
#define TOKEN_NAME_LEN      16
#define SP_VERSION          1
#define SP_MAX_LEN          8 * 1024            // 8k buf before psi
#define SP_RESERVERD        20
/*start of ����FLASH VA�ķ��� porting by l39225 20060504*/
#define VA_MAX_LEN          8*1024
/*end of ����FLASH VA�ķ��� porting by l39225 20060504*/


/*start of ����FLASH VA�ķ��� porting by l65130 20080328*/
#define FIX_MAX_LEN         8*1024
#define AVAIL_MAX_LEN       16*1024
/*end of ����FLASH VA�ķ��� porting by l65130 20080328*/


typedef struct _SP_HEADER
{
    char SPMagicNum[MAGIC_NUM_LEN];             // 8 bytes of magic number
    int SPVersion;                              // version number
    char SPReserved[SP_RESERVERD];              // reservied, total 32 bytes
} SP_HEADER, *PSP_HEADER;

typedef struct _TOKEN_DEF
{
    char tokenName[TOKEN_NAME_LEN];
    int tokenLen;
} SP_TOKEN, *PSP_TOKEN;

typedef struct cs_config_pars_tag
{
  int   mode;
  int   base;
  int   size;
  int   wait_state;
  int   setup_time;
  int   hold_time;
} cs_config_pars_t;

/*l65130 start ����FLASH - VAR����ʹ�� 2008-01-17*/
#define FLASH_VAR_START         0
/*����ʧ�ܴ���*/
#define FAILURE_TAG_LEN         10

/* HUAWEI HGW s48571 2008��2��20�� Ĭ�Ϲر�telnet���� add begin:*/
/*Telnet�رձ�־*/
#define TELNET_TAG_LEN         8
/* HUAWEI HGW s48571 2008��2��20�� Ĭ�Ϲر�telnet���� add end.*/
/* HUAWEI HGW s48571 2008��3��10�� Ĭ�Ϲر�ping���� add begin:*/
/*Ping�رձ�־*/
#define PING_TAG_LEN         8
/* HUAWEI HGW s48571 2008��3��10�� Ĭ�Ϲر�ping���� add end.*/


/*�������к�*/
#define SYS_PARAMETERS_OFFSET   32
#define BOARD_SERIAL_NUMBER_LEN 32
#define BOARD_MAGIC_LEN         8

/*WLAN WEP KEY*/
#define WLAN_PARAMETERS_OFFSET      (CFG_STATE_DATA_START + CWMP_STATE_DATA_LEN)
#define WLAN_WEP_KEY_LEN            32
#define WLAN_WPA_PSK_LEN            64
#define RESTORE_FLAG_LEN            8
#define WLAN_SSID_LEN               64
#define WLAN_PARA_LEN               (WLAN_WEP_KEY_LEN + WLAN_WPA_PSK_LEN + RESTORE_FLAG_LEN + WLAN_SSID_LEN)

/*��������*/
#define CFG_STATE_DATA_START    264//(256+8) 2007-3-19 ly65130�������ͻ,���Ǩ��8���ֽ�
/*�����ɹ�ʧ����Ϣ*/
#define UPG_INFO_STATUS         8
/*������ʶ*/
#define MAX_UPGRADE_FLAG        8
#define CWMP_STATE_DATA_LEN     2040//(2048-8) 2007-3-19 ly65130�������ͻ,�������8���ֽ�
/*����֪ͨ���Ա��泤��*/
#define CWMP_PARA_OFFSET            8*1024
#define CWMP_PARA_LEN               3072
#define CWMP_PARA_RESET             8

//��־�ֶΣ���ʾ�����Ƿ��Ѵ��ϰ汾������֧�������ļ����ܵİ汾
//�����ȡ��һ�������ַ������ʾ���������������������ĵ�����
//�����ʾ��һ����������������������
#define  SEC_UPLOAD_FLAG    (CWMP_PARA_OFFSET + CWMP_PARA_LEN + CWMP_PARA_RESET)
#define  SEC_UPLOAD_FLAG_LEN       8

/*SSL֤���ļ�*/
#define CFG_STATE_SSL_STAT      4096
#define CWMP_SSL_CERT_LEN       2048


/*��������ʹ��*/
/* HUAWEI HGW s48571 2008��2��20�� Ĭ�Ϲر�telnet���� modify begin:
#define FLASH_VAR_1_START       (FLASH_VAR_START + FAILURE_TAG_LEN)
*/
#define FLASH_VAR_1_START       (FLASH_VAR_START + FAILURE_TAG_LEN + TELNET_TAG_LEN + PING_TAG_LEN)
/* HUAWEI HGW s48571 2008��2��20�� Ĭ�Ϲر�telnet���� modify end. */
#define FLASH_VAR_1_LEN         (SYS_PARAMETERS_OFFSET - FLASH_VAR_1_START) //22

#define FLASH_VAR_2_START       (SYS_PARAMETERS_OFFSET + BOARD_SERIAL_NUMBER_LEN + BOARD_MAGIC_LEN)
#define FLASH_VAR_2_LEN         (CFG_STATE_DATA_START - FLASH_VAR_2_START) //216

#define FLASH_VAR_3_START       (WLAN_PARAMETERS_OFFSET + WLAN_PARA_LEN)
#define FLASH_VAR_3_LEN         (CFG_STATE_SSL_STAT - FLASH_VAR_3_START) //1792

#define FLASH_VAR_4_START       (CFG_STATE_SSL_STAT + CWMP_SSL_CERT_LEN)
#define FLASH_VAR_4_LEN         (8192 - FLASH_VAR_4_START)     //2048

/* BEGIN: Added by chenzhigang, 2008/3/14   ���ⵥ��:CWʹ�ܹر����ն�������*/
#define FLASH_VAR_VOICE_SERVICE_MANAGE_START   128
#define FLASH_VAR_VOICE_SERVICE_MANAGE_LEN     8
/* END:   Added by chenzhigang, 2008/3/14 */



/* BEGIN: Modified by w00135358, 2009/12/22:HG DSL Link Active Flag*/
#define GATEWAY_ACTIVATION_INFO CFG_STATE_DATA_START + UPG_INFO_STATUS + UPG_INFO_STATUS       //280~281
#define GATEWAY_ACTIVATION_INFO_LEN 2                                            //2���ֽ�
/* END: Modified by w00135358, 2009/12/22:HG DSL Link Active Flag*/
/* BEGIN: Added by y67514, 2010/9/2   PN:FTTX management*/
#define FTTX_WANMODE_INFO   GATEWAY_ACTIVATION_INFO + GATEWAY_ACTIVATION_INFO_LEN
#define FTTX_WANMODE_INFO_LEN       2
/* END:   Added by y67514, 2010/9/2 */
/* BEGIN: Modified by w00135358, 2010/02/21: TR069 FIRMWARE UPGRADE Fault Code-ACS���������ϱ�9010����*/
#define CWMP_LOAD_FAIL_INFO FTTX_WANMODE_INFO+FTTX_WANMODE_INFO_LEN     //282~409
#define CWMP_LOAD_FAIL_INFO_LEN 128                                            //128���ֽ�
/* END: Modified by w00135358, 2010/02/21: TR069 FIRMWARE UPGRADE Fault Code-ACS���������ϱ�9010����*/

/* BEGIN: Added by c65985, 2010/10/18   PN:bug150*/
#define SMS_EASYRESET_FLAG_START    (CWMP_LOAD_FAIL_INFO+CWMP_LOAD_FAIL_INFO_LEN)
#define SMS_EASYRESET_FLAG_LEN          4

#define IMSIMSISDNPAIR_OFFSET (SMS_EASYRESET_FLAG_START + SMS_EASYRESET_FLAG_LEN)
#define  IFC_IMSIMSISDNPAIR_LEN 48
/* END:   Added by c65985, 2010/10/18 PN:bug150*/

/*start of ���� ��ʾVDF ���������ļ� by dKF19971*/
//#define CFG_CHG_FLAG_START  288
#define CFG_CHG_FLAG_START  (CWMP_PARA_OFFSET + CWMP_PARA_LEN + 16)
#define CFG_CHG_SIG_LEN     4
#define CFG_CHG_FLAG_LEN    12
/*end of ���� ��ʾVDF ���������ļ� by dKF19971*/

/*BEGIN: add the webprotection  flash flag by DHS001699888; 2010-11-29  */
#define PARTCFG_WEBPROTECTION_REMINDER_CHG    (CFG_CHG_FLAG_START+ CFG_CHG_SIG_LEN +CFG_CHG_FLAG_LEN)
#define PARTCFG_WEBPROTECTION_REMINDER_LEN     4
/*END: add the webprotection  flash flag by DHS001699888; 2010-11-29  */


/*l65130 end ����FLASH - VAR����ʹ�� 2008-01-17*/
#define IFC_PIN_PASSWORD_LEN   16          //PIN����������볤��
#define IFC_PUK_PASSWORD_LEN   16          //PUK����������볤��
#define PINCODE_OFFSET         80          //��Ա����洢���׵�ַ��ƫ����
#define IFC_IMEI_LEN           16          //IMEI��������С
#define IMEI_OFFSET            96          //IMEI��Ա����洢���׵�ַ��ƫ����

/*****************************************************************************/
/*          Function Prototypes                                              */
/*****************************************************************************/
#if !defined(__ASM_ASM_H)
void dumpaddr( unsigned char *pAddr, int nLen );

void kerSysFlashAddrInfoGet(PFLASH_ADDR_INFO pflash_addr_info);
int kerSysNvRamGet(char *string, int strLen, int offset);
int kerSysNvRamSet(char *string, int strLen, int offset);
int kerSysVariableGet(char *string, int strLen, int offset);
int kerSysVariableSet(char *string, int strLen, int offset);
int kerSysPersistentGet(char *string, int strLen, int offset);
int kerSysPersistentSet(char *string, int strLen, int offset);
int kerSysPersistentClear(void);
/* begin --- Add Persistent storage interface backup flash partition by w69233 */
int kerSysPsiBackupGet(char *string, int strLen, int offset);
int kerSysPsiBackupSet(char *string, int strLen, int offset);
int kerSysPsiBackupClear(void);
/* end ----- Add Persistent storage interface backup flash partition by w69233 */


/* begin --- Add partition by l65130 */
int kerSysFixGet(char *string, int strLen, int offset);
int kerSysFixSet(char *string, int strLen, int offset);
int kerSysFixClear(void);

int kerSysAvailGet(char *string, int strLen, int offset);
int kerSysAvailSet(char *string, int strLen, int offset);
int kerSysAvailClear(void);
/* end --- Add partition by l65130 */

int kerSysScratchPadGet(char *tokName, char *tokBuf, int tokLen);
int kerSysScratchPadSet(char *tokName, char *tokBuf, int tokLen);
int kerSysScratchPadClearAll(void);
int kerSysBcmImageSet( int flash_start_addr, char *string, int size);
int kerSysGetMacAddress( unsigned char *pucaAddr, unsigned long ulId );
int kerSysReleaseMacAddress( unsigned char *pucaAddr );
int kerSysGetSdramSize( void );
void kerSysGetBootline(char *string, int strLen);
void kerSysSetBootline(char *string, int strLen);
void kerSysMipsSoftReset(void);
void kerSysLedCtrl(BOARD_LED_NAME, BOARD_LED_STATE);
void kerSysLedRegisterHwHandler( BOARD_LED_NAME, HANDLE_LED_FUNC, int );
int kerSysFlashSizeGet(void);
int kerSysMemoryMappedFlashSizeGet(void);
unsigned long kerSysReadFromFlash( void *toaddr, unsigned long fromaddr,
    unsigned long len );
void kerSysRegisterDyingGaspHandler(char *devname, void *cbfn, void *context);
void kerSysDeregisterDyingGaspHandler(char *devname);    
void kerSysWakeupMonitorTask( void );
int kerConfigCs(BOARD_IOCTL_PARMS *parms);
void kerSetPll(int pll_mask, int pll_value);
void kerSetGpio(int gpio, GPIO_STATE_t state);
unsigned char kerSysGetGPIO(unsigned short ucGpioNum);
unsigned char GetBoardVersion(void);
unsigned char GetHarewareType(void);

int kerSysGetFsImageFromFlash(char *string, int strLen, int offSet);
int kerSysGetKernelImageFromFlash(char *string, int strLen, int offSet);

#endif

#ifdef __cplusplus
}
#endif

#endif /* _BOARD_H */

