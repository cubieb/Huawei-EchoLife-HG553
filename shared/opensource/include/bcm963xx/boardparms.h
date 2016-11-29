/*
<:copyright-gpl 

 Copyright 2003 Broadcom Corp. All Rights Reserved. 
 
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
/**************************************************************************
 * File Name  : boardparms.h
 *
 * Description: This file contains definitions and function prototypes for
 *              the BCM63xx board parameter access functions.
 * 
 * Updates    : 07/14/2003  Created.
 ***************************************************************************/

#if !defined(_BOARDPARMS_H)
#define _BOARDPARMS_H

#ifdef __cplusplus
extern "C" {
#endif


/* Return codes. */
#define BP_SUCCESS                              0
#define BP_BOARD_ID_NOT_FOUND                   1
#define BP_VALUE_NOT_DEFINED                    2
#define BP_BOARD_ID_NOT_SET                     3

/* Values for BpGetSdramSize. */
#define BP_MEMORY_8MB_1_CHIP                    0
#define BP_MEMORY_16MB_1_CHIP                   1
#define BP_MEMORY_32MB_1_CHIP                   2
#define BP_MEMORY_64MB_2_CHIP                   3
#define BP_MEMORY_32MB_2_CHIP                   4
#define BP_MEMORY_16MB_2_CHIP                   5
/*s48571 add for VDF HG553 hardware porting 20080104 begin*/
#define BP_MEMORY_64MB_1_CHIP                   6
/*s48571 add for VDF HG553 hardware porting 20080104 end*/
/* Values for EthernetMacInfo PhyType. */
#define BP_ENET_NO_PHY                          0
#define BP_ENET_INTERNAL_PHY                    1
#define BP_ENET_EXTERNAL_PHY                    2
#define BP_ENET_EXTERNAL_SWITCH                 3
#define BP_ENET_INTERNAL_PHY_SWITCH             4 /* it is for cpu internal phy connects to port 4 of 5325e */

/* Values for EthernetMacInfo Configuration type. */
#define BP_ENET_CONFIG_MDIO                     0       /* Internal PHY, External PHY, Switch+(no GPIO, no SPI, no MDIO Pseudo phy */
#define BP_ENET_CONFIG_MDIO_PSEUDO_PHY          1
#define BP_ENET_CONFIG_SPI_SSB_0                2
#define BP_ENET_CONFIG_SPI_SSB_1                3
#define BP_ENET_CONFIG_SPI_SSB_2                4
#define BP_ENET_CONFIG_SPI_SSB_3                5
#define BP_ENET_CONFIG_MMAP                     6
#define BP_ENET_CONFIG_SMI                      7
/* Values for EthernetMacInfo Reverse MII. */
#define BP_ENET_NO_REVERSE_MII                  0
#define BP_ENET_REVERSE_MII                     1

/* Values for VoIPDSPInfo DSPType. */
#define BP_VOIP_NO_DSP                          0
#define BP_VOIP_DSP                             1
#define BP_VOIP_MIPS                            2

/* Values for GPIO pin assignments (AH = Active High, AL = Active Low). */
#define BP_GPIO_NUM_MASK                        0x00FF
#define BP_ACTIVE_MASK                          0x8000
#define BP_ACTIVE_HIGH                          0x0000
#define BP_ACTIVE_LOW                           0x8000
#define BP_GPIO_SERIAL                          0x4000

#define BP_GPIO_0_AH                            (0)
#define BP_GPIO_0_AL                            (0  | BP_ACTIVE_LOW)
#define BP_GPIO_1_AH                            (1)
#define BP_GPIO_1_AL                            (1  | BP_ACTIVE_LOW)
#define BP_GPIO_2_AH                            (2)
#define BP_GPIO_2_AL                            (2  | BP_ACTIVE_LOW)
#define BP_GPIO_3_AH                            (3)
#define BP_GPIO_3_AL                            (3  | BP_ACTIVE_LOW)
#define BP_GPIO_4_AH                            (4)
#define BP_GPIO_4_AL                            (4  | BP_ACTIVE_LOW)
#define BP_GPIO_5_AH                            (5)
#define BP_GPIO_5_AL                            (5  | BP_ACTIVE_LOW)
#define BP_GPIO_6_AH                            (6)
#define BP_GPIO_6_AL                            (6  | BP_ACTIVE_LOW)
#define BP_GPIO_7_AH                            (7)
#define BP_GPIO_7_AL                            (7  | BP_ACTIVE_LOW)
#define BP_GPIO_8_AH                            (8)
#define BP_GPIO_8_AL                            (8  | BP_ACTIVE_LOW)
#define BP_GPIO_9_AH                            (9)
#define BP_GPIO_9_AL                            (9  | BP_ACTIVE_LOW)
#define BP_GPIO_10_AH                           (10)
#define BP_GPIO_10_AL                           (10 | BP_ACTIVE_LOW)
#define BP_GPIO_11_AH                           (11)
#define BP_GPIO_11_AL                           (11 | BP_ACTIVE_LOW)
#define BP_GPIO_12_AH                           (12)
#define BP_GPIO_12_AL                           (12 | BP_ACTIVE_LOW)
#define BP_GPIO_13_AH                           (13)
#define BP_GPIO_13_AL                           (13 | BP_ACTIVE_LOW)
#define BP_GPIO_14_AH                           (14)
#define BP_GPIO_14_AL                           (14 | BP_ACTIVE_LOW)
#define BP_GPIO_15_AH                           (15)
#define BP_GPIO_15_AL                           (15 | BP_ACTIVE_LOW)
#define BP_GPIO_16_AH                           (16)
#define BP_GPIO_16_AL                           (16 | BP_ACTIVE_LOW)
#define BP_GPIO_17_AH                           (17)
#define BP_GPIO_17_AL                           (17 | BP_ACTIVE_LOW)
#define BP_GPIO_18_AH                           (18)
#define BP_GPIO_18_AL                           (18 | BP_ACTIVE_LOW)
#define BP_GPIO_19_AH                           (19)
#define BP_GPIO_19_AL                           (19 | BP_ACTIVE_LOW)
#define BP_GPIO_20_AH                           (20)
#define BP_GPIO_20_AL                           (20 | BP_ACTIVE_LOW)
#define BP_GPIO_21_AH                           (21)
#define BP_GPIO_21_AL                           (21 | BP_ACTIVE_LOW)
#define BP_GPIO_22_AH                           (22)
#define BP_GPIO_22_AL                           (22 | BP_ACTIVE_LOW)
#define BP_GPIO_23_AH                           (23)
#define BP_GPIO_23_AL                           (23 | BP_ACTIVE_LOW)
#define BP_GPIO_24_AH                           (24)
#define BP_GPIO_24_AL                           (24 | BP_ACTIVE_LOW)
#define BP_GPIO_25_AH                           (25)
#define BP_GPIO_25_AL                           (25 | BP_ACTIVE_LOW)
#define BP_GPIO_26_AH                           (26)
#define BP_GPIO_26_AL                           (26 | BP_ACTIVE_LOW)
#define BP_GPIO_27_AH                           (27)
#define BP_GPIO_27_AL                           (27 | BP_ACTIVE_LOW)
#define BP_GPIO_28_AH                           (28)
#define BP_GPIO_28_AL                           (28 | BP_ACTIVE_LOW)
#define BP_GPIO_29_AH                           (29)
#define BP_GPIO_29_AL                           (29 | BP_ACTIVE_LOW)
#define BP_GPIO_30_AH                           (30)
#define BP_GPIO_30_AL                           (30 | BP_ACTIVE_LOW)
#define BP_GPIO_31_AH                           (31)
#define BP_GPIO_31_AL                           (31 | BP_ACTIVE_LOW)
#define BP_GPIO_32_AH                           (32)
#define BP_GPIO_32_AL                           (32 | BP_ACTIVE_LOW)
#define BP_GPIO_33_AH                           (33)
#define BP_GPIO_33_AL                           (33 | BP_ACTIVE_LOW)
#define BP_GPIO_34_AH                           (34)
#define BP_GPIO_34_AL                           (34 | BP_ACTIVE_LOW)
#define BP_GPIO_35_AH                           (35)
#define BP_GPIO_35_AL                           (35 | BP_ACTIVE_LOW)
#define BP_GPIO_36_AH                           (36)
#define BP_GPIO_36_AL                           (36 | BP_ACTIVE_LOW)
#define BP_GPIO_37_AH                           (37)
#define BP_GPIO_37_AL                           (37 | BP_ACTIVE_LOW)

#define BP_SERIAL_GPIO_0_AH                     (0  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_0_AL                     (0  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_1_AH                     (1  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_1_AL                     (1  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_2_AH                     (2  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_2_AL                     (2  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_3_AH                     (3  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_3_AL                     (3  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_4_AH                     (4  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_4_AL                     (4  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_5_AH                     (5  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_5_AL                     (5  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_6_AH                     (6  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_6_AL                     (6  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_7_AH                     (7  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_7_AL                     (7  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)

/* Values for external interrupt assignments. */
#define BP_EXT_INTR_0                           0
#define BP_EXT_INTR_1                           1
#define BP_EXT_INTR_2                           2
#define BP_EXT_INTR_3                           3
#define BP_EXT_INTR_4                           4
#define BP_EXT_INTR_5                           5

/* Values for chip select assignments. */
#define BP_CS_0                                 0
#define BP_CS_1                                 1
#define BP_CS_2                                 2
#define BP_CS_3                                 3

#define BP_OVERLAY_PCI                          (1<<0)
#define BP_OVERLAY_CB                           (1<<1)
#define BP_OVERLAY_SPI_EXT_CS                   (1<<2)
#define BP_OVERLAY_MII2                         (1<<3)
#define BP_OVERLAY_UTOPIA                       (1<<4)
#define BP_OVERLAY_UART1                        (1<<5)
#define BP_OVERLAY_LED                          (1<<6)
#define BP_OVERLAY_PHY                          (1<<7)
#define BP_OVERLAY_SERIAL_LEDS                  (1<<8)
#define BP_OVERLAY_EPHY_LED_0                   (1<<9)
#define BP_OVERLAY_EPHY_LED_1                   (1<<10)
#define BP_OVERLAY_EPHY_LED_2                   (1<<11)
#define BP_OVERLAY_EPHY_LED_3                   (1<<12)
#define BP_OVERLAY_INET_LED                     (1<<13)

/* Value for GPIO and external interrupt fields that are not used. */
#define BP_NOT_DEFINED                          0xffff
#define BP_HW_DEFINED                           0xfff0
#define BP_UNEQUIPPED                           0xfff1

#define BP_HW_DEFINED                           0xfff0
#define BP_HW_DEFINED_AL                        0xfff0
#define BP_HW_DEFINED_AH                        0xfff2

/* Maximum size of the board id string. */
#define BP_BOARD_ID_LEN                         16

/* Maximum number of Ethernet MACs. */
#define BP_MAX_ENET_MACS                        2
#define BP_MAX_SWITCH_PORTS                     8
/* Maximum number of VoIP DSPs. */
#define BP_MAX_VOIP_DSP                         2

/* Wireless Antenna Settings. */
#define BP_WLAN_ANT_MAIN                        0
#define BP_WLAN_ANT_AUX                         1
#define BP_WLAN_ANT_BOTH                        3

/* Wireless FLAGS */
#define BP_WLAN_MAC_ADDR_OVERRIDE               0x0001   /* use kerSysGetMacAddress for mac address */
#define BP_WLAN_EXCLUDE_ONBOARD                 0x0002   /* exclude onboard wireless  */

/*start of cfe added by y42304 20060512: ��Ӳ��������ص����� */
#define GPIO_SERIAL_MODE_MASK           0x1
#define GPIO_LED_MODE_MMASK             0x2
#define GPIO_GET_PCB_VER_MASK           0x4
#define GPIO_GET_BOARD_VER_MASK         0x8
/*end of cfe added by y42304 20060512 */

#define BP_WLAN_NVRAM_NAME_LEN      16
#define BP_WLAN_MAX_PATCH_ENTRY     8

/*start of cfe added by y42304 20060515: PCB version λ */
#define PCB_VERSION_GPIO_NUMS            2
#define PCB_VERSION_BIT0                 0
#define PCB_VERSION_BIT1                 1
/*end of cfe added by y42304 20060515 */

/*start of cfe added by y42304 20060515: Board version λ */
#define BOARD_VERSION_GPIO_NUMS          3
#define BOARD_VERSION_BIT0               0
#define BOARD_VERSION_BIT1               1
#define BOARD_VERSION_BIT2               2
/*end of cfe added by y42304 20060515 */

/*start of cfe modified by y42304 20060516: ���ö����bcm63xx_led.c����ֲ������ */
#define MAX_VIRT_LEDS                   15      /* ����֧�ֵ�LED��Ŀ */
/*end of cfe modified by y42304 20060516: Board version λ */

/* AFE IDs */
#define BP_AFE_DEFAULT              0

#define BP_AFE_CHIP_INT             (1 << 28)
#define BP_AFE_CHIP_6505            (2 << 28)
#define BP_AFE_CHIP_6306            (3 << 28)

#define BP_AFE_LD_ISIL1556          (1 << 21)
#define BP_AFE_LD_6302              (3 << 21)

#define BP_AFE_FE_ANNEXA            (1 << 15)
#define BP_AFE_FE_ANNEXB            (2 << 15)

#define BP_AFE_FE_REV_ISIL_REV1     (1 << 8)
#define BP_AFE_FE_REV_6302_REV1     (1 << 8)
typedef struct {
  int port_map;
  int phy_id[BP_MAX_SWITCH_PORTS];
} ETHERNET_SW_INFO;

#define c0(n) (((n) & 0x55555555) + (((n) >> 1) & 0x55555555))
#define c1(n) (((n) & 0x33333333) + (((n) >> 2) & 0x33333333))
#define c2(n) (((n) & 0x0f0f0f0f) + (((n) >> 4) & 0x0f0f0f0f))
#define bitcount(r, n) {r = n; r = c0(r); r = c1(r); r = c2(r); r %= 255;}
#if !defined(__ASSEMBLER__)

/* Information about an Ethernet MAC.  If ucPhyType is BP_ENET_NO_PHY,
 * then the other fields are not valid.
 */
typedef struct EthernetMacInfo
{
    unsigned char ucPhyType;                    /* BP_ENET_xxx             */
    unsigned char ucPhyAddress;                 /* 0 to 31                 */
    unsigned short usGpioPhyReset;              /* GPIO pin or not defined */
    unsigned short usGpioPhyLinkSpeed;          /* GPIO pin or not defined */
    unsigned short usConfigType;                /* Configuration type */
    unsigned short usReverseMii;                /* Reverse MII */
    ETHERNET_SW_INFO sw;                        /* switch information */
} ETHERNET_MAC_INFO, *PETHERNET_MAC_INFO;

typedef struct WlanSromEntry {
    char name[BP_WLAN_NVRAM_NAME_LEN];
    unsigned short wordOffset;
    unsigned short value;
} WLAN_SROM_ENTRY;

typedef struct WlanSromPatchInfo {
    char szboardId[BP_BOARD_ID_LEN];
    unsigned short usWirelessChipId;
    unsigned short usNeededSize;
    WLAN_SROM_ENTRY entries[BP_WLAN_MAX_PATCH_ENTRY];
} WLAN_SROM_PATCH_INFO, *PWLAN_SROM_PATCH_INFO;

/* HUAWEI VHG WangXinfeng 69233 2008-10-13 Add begin */
/* Add: Support HG255 (China Telecom) hardware requirement */
typedef struct LanSwitchPortLedGpio
{
    unsigned short usGpioLedLan1;
    unsigned short usGpioLedLan2;
    unsigned short usGpioLedLan3;
    unsigned short usGpioLedLan4;
}LSW_PORT_LED_GPIO, *PLSW_PORT_LED_GPIO;
/* HUAWEI VHG WangXinfeng 69233 2008-10-13 Add end */

/*   SLIC types 
 */
typedef enum
{    
    Silicon_Labs_3210,
    Silicon_Labs_3215,
//modify by guwenbin 2008.10.21 change 89116
    Legerity_88221,
    Legerity_89116,
    Slic_Not_Defined     
} SLIC_TYPE;


/*   DAA types 
 */
typedef enum
{
    DAA_CIRCUIT,
    Silicon_Labs_3050,
    Legerity_88010, 
    DAA_Not_Defined 
} DAA_TYPE;

/* Information about VoIP DSPs.  If ucDspType is BP_VOIP_NO_DSP,
 * then the other fields are not valid.
 */
typedef struct VoIPDspInfo
{
    unsigned char  ucDspType;
    unsigned char  ucDspAddress;
    unsigned short usExtIntrVoip;
    unsigned short usGpioVoipReset;
    unsigned short usGpioVoipIntr;
    unsigned short usGpioLedVoip;    

/* start of y42304 added 20061130: ֧��PSTN LED, slic ��·LED */    
    unsigned short usGpioLedPSTN;    
    unsigned short usGpioLedLine0;    
    unsigned short usGpioLedLine1;    
/* end of y42304 added 20061130 */        
    unsigned short usCsVoip;

    unsigned short usGpioRelayCtl;
    unsigned short usGpioRingDetc;
    unsigned short usGpioLoopDetc;

    SLIC_TYPE slicType;     /* SLIC manufacturer & model */
	DAA_TYPE  daaType;		/* DAA manufacturer & model*/
} VOIP_DSP_INFO;

/*   VCOPE board definitions 
 */
typedef enum
{
    BCM6505_RESET_GPIO = 1,
    VCOPE_RELAY_GPIO,
    HPI_CS,
    VCOPE_BOARD_REV, 
} VCOPE_PIN_TYPES;

/* start of cfe update by y42304 20060512: ֧��PCB�汾��BOARD�汾 */
typedef struct PCBVersion
{    
    unsigned short usPcbVer_Low;  
    unsigned short usPcbVer_High;      
}PCB_VERSION;

typedef struct BOARDVersion
{
    unsigned short usBoardVer_Low; 
    unsigned short usBoardVer_Mid; 
    unsigned short usBoardVer_High;
    unsigned short usReserve;
}BOARD_VERSION;
/* end of cfe update by y42304 20060512: */


/* start of cfe update by y42304 20060509: ���Ľṹ��Ķ����boardparams.c����ֲ������ */
typedef struct boardparameters
{
    char szBoardId[BP_BOARD_ID_LEN];        /* board id string */
    unsigned short usGPIOOverlay;            /* enabled interfaces */
    ETHERNET_MAC_INFO EnetMacInfos[BP_MAX_ENET_MACS];
    VOIP_DSP_INFO VoIPDspInfo[BP_MAX_VOIP_DSP];
    unsigned short usSdramSize;             /* SDRAM size and type */
    unsigned short usPsiSize;               /* persistent storage in K bytes */
    unsigned short usGpioRj11InnerPair;     /* GPIO pin or not defined */
    unsigned short usGpioRj11OuterPair;     /* GPIO pin or not defined */
    unsigned short usGpioPressAndHoldReset; /* GPIO pin or not defined */
    unsigned short usGpioUartRts;           /* GPIO pin or not defined */
    unsigned short usGpioUartCts;           /* GPIO pin or not defined */
    unsigned short usGpioLedAdsl;           /* GPIO pin or not defined */
    unsigned short usGpioLedAdslFail;       /* GPIO pin or not defined */
    /* HUAWEI HGW s48571 2008��1��18�� Hardware Porting] add begin:*/
    unsigned short usGpioLedWireless;       /* GPIO pin or not defined */
    /* HUAWEI HGW s48571 2008��1��18�� Hardware Porting] add end.*/
    unsigned short usGpioLedWirelessFail;   /* GPIO pin or not defined */
    unsigned short usGpioLedUsb;            /* GPIO pin or not defined */
    unsigned short usGpioLedHpna;           /* GPIO pin or not defined */
    unsigned short usGpioLedWanData;        /* GPIO pin or not defined */    
    unsigned short usGpioLedWanDataFail;    /* y42304 added: ֧��INET failʱLED�Ŀ�����ʾ*/
    /* HUAWEI HGW s48571 2008��1��18�� Hardware Porting] add begin:*/
    unsigned short usGpioLedHspa;            /* GPIO pin or not defined */
    unsigned short usGpioLedHspaFail;        /* GPIO pin or not defined */
    unsigned short usGpioLedInternet;        /* GPIO pin or not defined */
    unsigned short usGpioLedInternetFail;    /* GPIO pin or not defined */
    /* HUAWEI HGW s48571 2008��1��18�� Hardware Porting] add end.*/
    unsigned short usGpioLedPpp;            /* GPIO pin or not defined */
    unsigned short usGpioLedPppFail;        /* GPIO pin or not defined */
    unsigned short usGpioLedBlPowerOn;      /* GPIO pin or not defined */
    unsigned short usGpioLedBlResetCfg;     /* GPIO pin or not defined */
    unsigned short usGpioLedBlStop;         /* GPIO pin or not defined */
    unsigned short usExtIntrAdslDyingGasp;  /* ext intr or not defined */
    unsigned short usExtIntrHpna;           /* ext intr or not defined */
    unsigned short usCsHpna;                /* chip select not defined */
    unsigned short usAntInUseWireless;	    /* antenna in use or not defined */
    unsigned short usGpioSesBtnWireless;    /* GPIO pin or not defined */
    unsigned short usExtIntrSesBtnWireless; /* ext intr or not defined */        
    unsigned short usGpioLedSesWireless;    /* GPIO pin or not defined */        
    unsigned short usCsHpi;                 /* HPI chip select */
    unsigned short usVcopeResetGpio;        /* VCOPE reset gpio number */
    unsigned short usVcopeRelayGpio;        /* VCOPE releay gpio number */
    unsigned short usVcopeBoardRev;         /* VCOPE board revision */
    unsigned short usCMTThead;              /* Main Execution Thread   */  
    unsigned short usHardwareCfg;           /* ��Ӳ�������ص�����:1��ʾ֧��
                                             * bit0: �Ƿ�֧��serial GPIO ģʽ
                                             * bit1: GPIO0-GPIO3�Ƿ�����ΪLEDģʽ 
                                             * bit2: �Ƿ�֧��ͨ��GPIO��ȡPCB�汾��                                             
                                             * bit3: �Ƿ�֧��ͨ��GPIO��ȡBOARD�汾��
                                             * bit4-bit31: ��ʱ����δ��
                                             */  
    PCB_VERSION   stPcbVersion;             /*y42304 added: ֧��PCB�汾�� */
    BOARD_VERSION stBoardVersion;           /*y42304 added: ֧��Board�汾�� */
    unsigned short usWirelessFlags;         /* WLAN flags */   
} BOARD_PARAMETERS, *PBOARD_PARAMETERS;
/* end of Borad update by y42304 20060509 */


/**************************************************************************
 * Name       : BpSetBoardId
 *
 * Description: This function find the BOARD_PARAMETERS structure for the
 *              specified board id string and assigns it to a global, static
 *              variable.
 *
 * Parameters : [IN] pszBoardId - Board id string that is saved into NVRAM.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_FOUND - Error, board id input string does not
 *                  have a board parameters configuration record.
 ***************************************************************************/
int BpSetBoardId( char *pszBoardId );

/**************************************************************************
 * Name       : BpGetBoardIds
 *
 * Description: This function returns all of the supported board id strings.
 *
 * Parameters : [OUT] pszBoardIds - Address of a buffer that the board id
 *                  strings are returned in.  Each id starts at BP_BOARD_ID_LEN
 *                  boundary.
 *              [IN] nBoardIdsSize - Number of BP_BOARD_ID_LEN elements that
 *                  were allocated in pszBoardIds.
 *
 * Returns    : Number of board id strings returned.
 ***************************************************************************/
int BpGetBoardIds( char *pszBoardIds, int nBoardIdsSize );

int BpGetGPIOverlays( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetSdramSize
 *
 * Description: This function returns a constant that describees the board's
 *              SDRAM type and size.
 *
 * Parameters : [OUT] pulSdramSize - Address of short word that the SDRAM size
 *                  is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 ***************************************************************************/
int BpGetSdramSize( unsigned long *pulSdramSize );

/**************************************************************************
 * Name       : BpGetPsiSize
 *
 * Description: This function returns the persistent storage size in K bytes.
 *
 * Parameters : [OUT] pulPsiSize - Address of short word that the persistent
 *                  storage size is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 ***************************************************************************/
int BpGetPsiSize( unsigned long *pulPsiSize );

/**************************************************************************
 * Name       : BpGetRj11InnerOuterPairGpios
 *
 * Description: This function returns the GPIO pin assignments for changing
 *              between the RJ11 inner pair and RJ11 outer pair.
 *
 * Parameters : [OUT] pusInner - Address of short word that the RJ11 inner pair
 *                  GPIO pin is returned in.
 *              [OUT] pusOuter - Address of short word that the RJ11 outer pair
 *                  GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, values are returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetRj11InnerOuterPairGpios( unsigned short *pusInner,
    unsigned short *pusOuter );

/**************************************************************************
 * Name       : BpGetPressAndHoldResetGpio
 *
 * Description: This function returns the GPIO pin assignment for the press
 *              and hold reset button.
 *
 * Parameters : [OUT] pusValue - Address of short word that the press and hold
 *                  reset button GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetPressAndHoldResetGpio( unsigned short *pusValue );

/* HUAWEI VHG WangXinfeng 69233 2008-10-13 Modify begin */
/* Modify: Support HG255 (China Telecom) hardware requirement */
/**************************************************************************
 * Name       : BpGetLan1LedGpio
 *
 * Description: This function returns the GPIO pin assignment for the LED of 
 *              lanswitch port 1.
 *
 * Parameters : [OUT] pusValue - Address of short word that the GPIO pin of 
 *              lanswitch port 1 LED is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetLan1LedGpio( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetLan2LedGpio
 *
 * Description: This function returns the GPIO pin assignment for the LED of 
 *              lanswitch port 2.
 *
 * Parameters : [OUT] pusValue - Address of short word that the GPIO pin of 
 *              lanswitch port 2 LED is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetLan2LedGpio( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetLan3LedGpio
 *
 * Description: This function returns the GPIO pin assignment for the LED of 
 *              lanswitch port 3.
 *
 * Parameters : [OUT] pusValue - Address of short word that the GPIO pin of 
 *              lanswitch port 3 LED is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetLan3LedGpio( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetLan4LedGpio
 *
 * Description: This function returns the GPIO pin assignment for the LED of 
 *              lanswitch port 4.
 *
 * Parameters : [OUT] pusValue - Address of short word that the GPIO pin of 
 *              lanswitch port 4 LED is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetLan4LedGpio( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetWpsLedGpio
 *
 * Description: This function returns the GPIO pin assignment for the WPS LED.
 *
 * Parameters : [OUT] pusValue - Address of short word that the WPS LED
 *              GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetWpsLedGpio( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetWpsBtnGpio
 *
 * Description: This function returns the GPIO pin assignment for the WPS 
 *              button.
 *
 * Parameters : [OUT] pusValue - Address of short word that the WPS button
 *              GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetWpsBtnGpio( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetUsbEnableGpio
 *
 * Description: This function returns the GPIO pin assignment for enabling Usb.
 *
 * Parameters : [OUT] pusValue - Address of short word that the Usb enable
 *              GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetUsbEnableGpio( unsigned short *pusValue );
/* HUAWEI VHG WangXinfeng 69233 2008-10-13 Modify end */

/**************************************************************************
 * Name       : BpGetVoipResetGpio
 *
 * Description: This function returns the GPIO pin assignment for the VOIP
 *              Reset operation.
 *
 * Parameters : [OUT] pusValue - Address of short word that the VOIP reset
 *                  GPIO pin is returned in.
 *              [IN] dspNum - Address of the DSP to query.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetVoipResetGpio( unsigned char dspNum, unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetVoipIntrGpio
 *
 * Description: This function returns the GPIO pin assignment for VoIP interrupt.
 *
 * Parameters : [OUT] pusValue - Address of short word that the VOIP interrupt
 *                  GPIO pin is returned in.
 *              [IN] dspNum - Address of the DSP to query.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetVoipIntrGpio( unsigned char dspNum, unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetUartRtsCtsGpios
 *
 * Description: This function returns the GPIO pin assignments for RTS and CTS
 *              UART signals.
 *
 * Parameters : [OUT] pusRts - Address of short word that the UART RTS GPIO
 *                  pin is returned in.
 *              [OUT] pusCts - Address of short word that the UART CTS GPIO
 *                  pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, values are returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetRtsCtsUartGpios( unsigned short *pusRts, unsigned short *pusCts );

/**************************************************************************
 * Name       : BpGetAdslLedGpio
 *
 * Description: This function returns the GPIO pin assignment for the ADSL
 *              LED.
 *
 * Parameters : [OUT] pusValue - Address of short word that the ADSL LED
 *                  GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetAdslLedGpio( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetAdslFailLedGpio
 *
 * Description: This function returns the GPIO pin assignment for the ADSL
 *              LED that is used when there is a DSL connection failure.
 *
 * Parameters : [OUT] pusValue - Address of short word that the ADSL LED
 *                  GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetAdslFailLedGpio( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetWirelessLedGpio
 *
 * Description: This function returns the GPIO pin assignment for the Wireless
 *              LED.
 *
 * Parameters : [OUT] pusValue - Address of short word that the Wireless LED
 *                  GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetWirelessLedGpio( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetWirelessAntInUse
 *
 * Description: This function returns the antennas in use for wireless
 *
 * Parameters : [OUT] pusValue - Address of short word that the Wireless Antenna
 *                  is in use.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetWirelessAntInUse( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetWirelessFlags
 *
 * Description: This function returns optional control flags for wireless
 *
 * Parameters : [OUT] pusValue - Address of short word control flags
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetWirelessFlags( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetWirelessSesBtnGpio
 *
 * Description: This function returns the GPIO pin assignment for the Wireless
 *              Ses Button.
 *
 * Parameters : [OUT] pusValue - Address of short word that the Wireless Ses
 *                  Button GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetWirelessSesBtnGpio( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetWirelessSesExtIntr
 *
 * Description: This function returns the external interrupt number for the 
 *              Wireless Ses Button.
 *
 * Parameters : [OUT] pusValue - Address of short word that the Wireless Ses
 *                  external interrup is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetWirelessSesExtIntr( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetWirelessSesLedGpio
 *
 * Description: This function returns the GPIO pin assignment for the Wireless
 *              Ses Led.
 *
 * Parameters : [OUT] pusValue - Address of short word that the Wireless Ses
 *                  Led GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetWirelessSesLedGpio( unsigned short *pusValue );

/* 2008/01/28 Jiajun Weng : New code from 3.12L.01 */
int BpUpdateWirelessSromMap(unsigned short chipID, unsigned short* pBase, int sizeInWords);

/**************************************************************************
 * Name       : BpGetUsbLedGpio
 *
 * Description: This function returns the GPIO pin assignment for the USB
 *              LED.
 *
 * Parameters : [OUT] pusValue - Address of short word that the USB LED
 *                  GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetUsbLedGpio( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetHpnaLedGpio
 *
 * Description: This function returns the GPIO pin assignment for the HPNA
 *              LED.
 *
 * Parameters : [OUT] pusValue - Address of short word that the HPNA LED
 *                  GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetHpnaLedGpio( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetWanDataLedGpio
 *
 * Description: This function returns the GPIO pin assignment for the WAN Data
 *              LED.
 *
 * Parameters : [OUT] pusValue - Address of short word that the WAN Data LED
 *                  GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetWanDataLedGpio( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetPppLedGpio
 *
 * Description: This function returns the GPIO pin assignment for the PPP
 *              LED.
 *
 * Parameters : [OUT] pusValue - Address of short word that the PPP LED
 *                  GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetPppLedGpio( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetPppFailLedGpio
 *
 * Description: This function returns the GPIO pin assignment for the PPP
 *              LED that is used when there is a PPP connection failure.
 *
 * Parameters : [OUT] pusValue - Address of short word that the PPP LED
 *                  GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetPppFailLedGpio( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetVoipLedGpio
 *
 * Description: This function returns the GPIO pin assignment for the VOIP
 *              LED.
 *
 * Parameters : [OUT] pusValue - Address of short word that the VOIP LED
 *                  GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetVoipLedGpio( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetBootloaderPowerOnLedGpio
 *
 * Description: This function returns the GPIO pin assignment for the power
 *              on LED that is set by the bootloader.
 *
 * Parameters : [OUT] pusValue - Address of short word that the alarm LED
 *                  GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetBootloaderPowerOnLedGpio( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetBootloaderResetCfgLedGpio
 *
 * Description: This function returns the GPIO pin assignment for the reset
 *              configuration LED that is set by the bootloader.
 *
 * Parameters : [OUT] pusValue - Address of short word that the reset
 *                  configuration LED GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetBootloaderResetCfgLedGpio( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetBootloaderStopLedGpio
 *
 * Description: This function returns the GPIO pin assignment for the break
 *              into bootloader LED that is set by the bootloader.
 *
 * Parameters : [OUT] pusValue - Address of short word that the break into
 *                  bootloader LED GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetBootloaderStopLedGpio( unsigned short *pusValue );

/**************************************************************************
 * Name       : BpGetAdslDyingGaspExtIntr
 *
 * Description: This function returns the ADSL Dying Gasp external interrupt
 *              number.
 *
 * Parameters : [OUT] pulValue - Address of short word that the ADSL Dying Gasp
 *                  external interrupt number is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetAdslDyingGaspExtIntr( unsigned long *pulValue );

/**************************************************************************
 * Name       : BpGetVoipExtIntr
 *
 * Description: This function returns the VOIP external interrupt number.
 *
 * Parameters : [OUT] pulValue - Address of short word that the VOIP
 *                  external interrupt number is returned in.
 *              [IN] dspNum - Address of the DSP to query.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetVoipExtIntr( unsigned char dspNum, unsigned long *pulValue );

/**************************************************************************
 * Name       : BpGetHpnaExtIntr
 *
 * Description: This function returns the HPNA external interrupt number.
 *
 * Parameters : [OUT] pulValue - Address of short word that the HPNA
 *                  external interrupt number is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetHpnaExtIntr( unsigned long *pulValue );

/**************************************************************************
 * Name       : BpGetHpnaChipSelect
 *
 * Description: This function returns the HPNA chip select number.
 *
 * Parameters : [OUT] pulValue - Address of short word that the HPNA
 *                  chip select number is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetHpnaChipSelect( unsigned long *pulValue );

/**************************************************************************
 * Name       : BpGetVoipChipSelect
 *
 * Description: This function returns the VOIP chip select number.
 *
 * Parameters : [OUT] pulValue - Address of short word that the VOIP
 *                  chip select number is returned in.
 *              [IN] dspNum - Address of the DSP to query.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetVoipChipSelect( unsigned char dspNum, unsigned long *pulValue );

/**************************************************************************
 * Name       : BpGetCMTThread
 *
 * Description: This function returns CMT thread number to run main code
 *
 * Parameters : [OUT] pulValue - CMT thread number to run main code
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetCMTThread( unsigned long *pulValue );

/**************************************************************************
 * Name       : BpGetHpiChipSelect
 *
 * Description: This function returns the HPI chip select number.
 *
 * Parameters : [OUT] pulValue - Address of short word that the HPNA
                chip select number is returned in.
 *
 * Returns    : BP_SUCCESS           - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET  - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                                     for the board.
 ***************************************************************************/
int BpGetHpiChipSelect( unsigned long *pulValue );

int BpGetEthernetMacInfo( PETHERNET_MAC_INFO pEnetInfos, int nNumEnetInfos );
/**************************************************************************
 * Name       : BpGetVcopeGpio 
 *
 * Description: This function returns the GPIO number of VCOPE board
 *
 * Parameters : [OUT] - GPIO index (name)
 *
 * Returns    : pin number      - Success
 *              BP_NOT_DEFINED  - any kind of Error
 ***************************************************************************/
int BpGetVcopeGpio(int pio_idx);

/* start of BOARD added by y42304 20060516 */
int BpGetWanDataFailLedGpio(unsigned short *pusValue);
int BpIsSupportSerialGpioMode(void);
int BpIsSupportLedGpioMode(void);
int BpIsSupportPcbVersion(void);
int BpIsSupportBoardVersion(void);
int BpGetBoardVesionGpio(unsigned char ucBoardVerBit, unsigned short *pusValue );
int BpGetPCBVesionGpio(unsigned char ucPcbVerBit, unsigned short *pusValue );
int BpGetTelLine0LedGpio(unsigned short *pusValue );
int BpGetTelLine1LedGpio(unsigned short *pusValue );
int BpGetPstnLedGpio(unsigned short *pusValue );
/* end of BOARD added by y42304 20060516 */

/* START ADD: Add for voip fxo 2006-12-4 liujianfeng*/
int BpGetFxoLoopDetcGpio(unsigned short *pusValue );
int BpGetFxoRingDetcGpio(unsigned short *pusValue );
int BpGetFxoRelayCtlGpio(unsigned short *pusValue );
/* END ADD: Add for voip fxo 2006-12-4 liujianfeng*/
/* start of BOARD added by y42304 20061220:��ȡslic��DAA������ */
int BpGetSlicType( unsigned short *slicType );
int BpGetDAAType( unsigned short *daaType );    
/* end of BOARD added by y42304 20061220 */

/* HUAWEI HGW s48571 2008��1��18�� Hardware Porting add begin:*/
int BpGetHspaLedGpio( unsigned short *pusValue );
int BpGetHspaFailLedGpio( unsigned short *pusValue );
int BpGetInternetLedGpio( unsigned short *pusValue );
int BpGetInternetFailLedGpio( unsigned short *pusValue );
int BpGetWirelessFailLedGpio( unsigned short *pusValue );
/* HUAWEI HGW s48571 2008��1��18�� Hardware Porting add end.*/
#endif /* __ASSEMBLER__ */

#ifdef __cplusplus
}
#endif

#endif /* _BOARDPARMS_H */

