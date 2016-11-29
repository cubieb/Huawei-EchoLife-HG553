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
 * File Name  : boardparms.c
 *
 * Description: This file contains the implementation for the BCM63xx board
 *              parameter access functions.
 * 
 * Updates    : 07/14/2003  Created.
 ***************************************************************************/

/* Includes. */
#include "boardparms.h"

/* Defines. */

/* Default psi size in K bytes */
#define BP_PSI_DEFAULT_SIZE                     64 

/* Variables */
#if defined(_BCM96338_) || defined(CONFIG_BCM96338)
static BOARD_PARAMETERS g_bcm96338sv =
{
    "96338SV",                               /* szBoardId */
    {{BP_ENET_INTERNAL_PHY,                 /* ucPhyType */
      0x01,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x01,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO,                  /* usConfigType */
      BP_NOT_DEFINED},                      /* usReverseMii */
     {BP_ENET_NO_PHY}},                     /* ucPhyType */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_16MB_1_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_NOT_DEFINED,                         /* usGpioLedWanData */    
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_NOT_DEFINED,                         /* usGpioLedPpp */
    BP_NOT_DEFINED,                         /* usGpioLedPppFail */
    BP_NOT_DEFINED,                         /* usGpioLedBlPowerOn */
    BP_NOT_DEFINED,                         /* usGpioLedBlResetCfg */
    BP_NOT_DEFINED,                         /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    BP_NOT_DEFINED,                         /* usGpioSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */     
    BP_NOT_DEFINED,                         /* usCsHpi */
    BP_NOT_DEFINED,                         /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    BP_NOT_DEFINED,                         /* usVcopeBoardRev */
    0,                                      /* usCMTThead */    
    0,                                      /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};
static BOARD_PARAMETERS g_bcm96338l2m8m =
{
    "96338L-2M-8M",                         /* szBoardId */
    {{BP_ENET_INTERNAL_PHY,                 /* ucPhyType */
      0x01,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x01,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO,                  /* usConfigType */
      BP_NOT_DEFINED},                      /* usReverseMii */
     {BP_ENET_NO_PHY}},                     /* ucPhyType */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_8MB_1_CHIP,                   /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_GPIO_2_AL,                           /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_3_AL,                           /* usGpioLedWanData */    
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_GPIO_3_AL,                           /* usGpioLedPpp */
    BP_GPIO_4_AL,                           /* usGpioLedPppFail */
    BP_GPIO_0_AL,                           /* usGpioLedBlPowerOn */
    BP_GPIO_3_AL,                           /* usGpioLedBlResetCfg */
    BP_GPIO_1_AL,                           /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    BP_NOT_DEFINED,                         /* usGpioSesBtnWireless */    
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */     
    BP_NOT_DEFINED,                         /* usCsHpi */
    BP_NOT_DEFINED,                         /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    BP_NOT_DEFINED,                         /* usVcopeBoardRev */
    0,                                      /* usCMTThead */    
    0,                                      /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};
static BOARD_PARAMETERS g_bcm96338gw =
{
    "96338W",                               /* szBoardId */
    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
      0x0,                                  /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x04,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO_PSEUDO_PHY,       /* usConfigType */
      BP_ENET_REVERSE_MII},                 /* usReverseMii */
     {BP_ENET_NO_PHY}},                     /* ucPhyType */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_16MB_1_CHIP,                   /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_GPIO_2_AL,                           /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_4_AL,                           /* usGpioLedWanData */
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */    
    BP_GPIO_4_AL,                           /* usGpioLedPpp */
    BP_GPIO_3_AL,                           /* usGpioLedPppFail */
    BP_GPIO_1_AL,                           /* usGpioLedBlPowerOn */
    BP_GPIO_4_AL,                           /* usGpioLedBlResetCfg */
    BP_GPIO_0_AL,                           /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    BP_GPIO_5_AL,                           /* usGpioSesBtnWireless */    
    BP_EXT_INTR_0,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */     
    BP_NOT_DEFINED,                         /* usCsHpi */
    BP_NOT_DEFINED,                         /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    BP_NOT_DEFINED,                         /* usVcopeBoardRev */
    0,                                      /* usCMTThead */    
    0,                                      /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};
static BOARD_PARAMETERS g_bcm96338e4 =
{
    "96338E4",                              /* szBoardId */
    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
      0x0,                                  /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x04,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO_PSEUDO_PHY,       /* usConfigType */
      BP_ENET_REVERSE_MII},                 /* usReverseMii */
     {BP_ENET_NO_PHY}},                     /* ucPhyType */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_8MB_1_CHIP,                   /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_GPIO_2_AL,                           /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_4_AL,                           /* usGpioLedWanData */    
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_GPIO_4_AL,                           /* usGpioLedPpp */
    BP_GPIO_3_AL,                           /* usGpioLedPppFail */
    BP_GPIO_1_AL,                           /* usGpioLedBlPowerOn */
    BP_GPIO_4_AL,                           /* usGpioLedBlResetCfg */
    BP_GPIO_0_AL,                           /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    BP_NOT_DEFINED,                         /* usGpioSesBtnWireless */    
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */     
    BP_NOT_DEFINED,                         /* usCsHpi */
    BP_NOT_DEFINED,                         /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    BP_NOT_DEFINED,                         /* usVcopeBoardRev */
    0,                                      /* usCMTThead */    
    0,                                      /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};
static PBOARD_PARAMETERS g_BoardParms[] =
    {&g_bcm96338sv, &g_bcm96338l2m8m, &g_bcm96338gw, &g_bcm96338e4, 0};
#endif

#if defined(_BCM96348_) || defined(CONFIG_BCM96348)

/* y42304 added: Ϊ�����主ʿ��HG520 */
static BOARD_PARAMETERS g_96348HG520_F =
{
    "96348HG520_F",                         /* szBoardId */ 
    {{BP_ENET_NO_PHY},                      /* ucPhyType */ 
     {BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */ 
      0x00,                                 /* ucPhyAddress */ 
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */ 
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */ 
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */ 
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */ 
      BP_NOT_DEFINED,                       /* usGpioPhyReset */ 
      0x04,                                 /* numSwitchPorts */ 
      BP_ENET_CONFIG_SPI_SSB_0,             /* usConfigType */ 
      BP_ENET_REVERSE_MII}},                /* usReverseMii */ 
     {{BP_VOIP_NO_DSP},                      /* ucDspType */
      {BP_VOIP_NO_DSP}},                     /* ucDspType */
      BP_MEMORY_16MB_1_CHIP,                /* usSdramSize */ 
      BP_PSI_DEFAULT_SIZE,                  /* usPsiSize */       
      BP_NOT_DEFINED,                       /* usGpioRj11InnerPair */ 
      BP_NOT_DEFINED,                       /* usGpioRj11OuterPair */ 
      BP_GPIO_33_AL,                        /* usGpioPressAndHoldReset */    
      BP_NOT_DEFINED,                       /* usGpioUartRts */ 
      BP_NOT_DEFINED,                       /* usGpioUartCts */ 
      BP_NOT_DEFINED,                       /* usGpioLedAdsl */ 
      BP_GPIO_2_AL,                         /* usGpioLedAdslFail */ 
      BP_NOT_DEFINED,                       /* usGpioLedWireless */      
      BP_NOT_DEFINED,                       /* usGpioLedUsb */ 
      BP_NOT_DEFINED,                       /* usGpioLedHpna */ 
      BP_GPIO_3_AL,                         /* usGpioLedWanData */    
      BP_NOT_DEFINED,                       /* usGpioLedWanDataFail */
      BP_GPIO_3_AL,                         /* usGpioLedPpp */ 
      BP_GPIO_4_AL,                         /* usGpioLedPppFail */     
      BP_GPIO_0_AL,                         /* usGpioLedBlPowerOn */ 
      BP_GPIO_3_AL,                         /* usGpioLedBlResetCfg */ 
      BP_GPIO_1_AL,                         /* usGpioLedBlStop */     
      BP_HW_DEFINED,                        /* usExtIntrAdslDyingGasp: y42304 */             
      BP_NOT_DEFINED,                       /* usExtIntrHpna */ 
      BP_NOT_DEFINED,                       /* usCsHpna */ 
      BP_NOT_DEFINED,                       /* usAntInUseWireless */
      BP_NOT_DEFINED,                       /* usGpioSesBtnWireless */
      BP_NOT_DEFINED,                       /* usExtIntrSesBtnWireless */
      BP_NOT_DEFINED,                       /* usGpioLedSesWireless */
      BP_CS_1,                              /* usCsHpi */
      BP_GPIO_22_AL,                        /* usVcopeResetGpio */
      BP_GPIO_21_AL,                        /* usVcopeRelayGpio */
      3,                                    /* usVcopeBoardRev */
      0,                                    /* usCMTThead */
      GPIO_SERIAL_MODE_MASK,                /* Hardware config */
      {BP_NOT_DEFINED,                      /* PCB version low bit GPIO */
       BP_NOT_DEFINED},                     /* PCB version high bit GPIO */
      {BP_NOT_DEFINED,                      /* Board version low bit GPIO */
       BP_NOT_DEFINED,                      /* Board version middle bit GPIO */                   
       BP_NOT_DEFINED,                      /* Board version high bit GPIO */       
       BP_NOT_DEFINED}                      /* usReserve */      
}; 


static BOARD_PARAMETERS g_bcm96348r =
{
    "96348R",                               /* szBoardId */
    {{BP_ENET_INTERNAL_PHY,                 /* ucPhyType */
      0x01,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x01,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO,                  /* usConfigType */
      BP_NOT_DEFINED},                      /* usReverseMii */
     {BP_ENET_NO_PHY}},                     /* ucPhyType */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_8MB_1_CHIP,                   /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_GPIO_7_AH,                           /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_GPIO_2_AL,                           /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_3_AL,                           /* usGpioLedWanData */
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */ 
    BP_GPIO_3_AL,                           /* usGpioLedPpp */
    BP_GPIO_4_AL,                           /* usGpioLedPppFail */
    BP_GPIO_0_AL,                           /* usGpioLedBlPowerOn */
    BP_GPIO_3_AL,                           /* usGpioLedBlResetCfg */
    BP_GPIO_1_AL,                           /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    BP_NOT_DEFINED,                         /* usGpioSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */     
    BP_NOT_DEFINED,                         /* usCsHpi */
    BP_NOT_DEFINED,                         /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    BP_NOT_DEFINED,                         /* usVcopeBoardRev */
    0,                                      /* usCMTThead */
    0,                                      /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
        
};

static BOARD_PARAMETERS g_bcm96348lv =
{
    "96348LV",                               /* szBoardId */
    {{BP_ENET_INTERNAL_PHY,                 /* ucPhyType */
      0x01,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x01,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO,                  /* usConfigType */
      BP_NOT_DEFINED},                      /* usReverseMii */
     {BP_ENET_EXTERNAL_PHY,                 /* ucPhyType */
      0x02,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_GPIO_5_AL,                         /* usGpioPhyReset */
      0x01,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO,                  /* usConfigType */
      BP_NOT_DEFINED}},                     /* usReverseMii */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_16MB_2_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_GPIO_7_AH,                           /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_GPIO_2_AL,                           /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_3_AL,                           /* usGpioLedWanData */  
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_GPIO_3_AL,                           /* usGpioLedPpp */
    BP_GPIO_4_AL,                           /* usGpioLedPppFail */
    BP_GPIO_0_AL,                           /* usGpioLedBlPowerOn */
    BP_GPIO_3_AL,                           /* usGpioLedBlResetCfg */
    BP_GPIO_1_AL,                           /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    BP_NOT_DEFINED,                         /* usGpioSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */     
    BP_NOT_DEFINED,                         /* usCsHpi */
    BP_NOT_DEFINED,                         /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    BP_NOT_DEFINED,                         /* usVcopeBoardRev */
    0,                                      /* usCMTThead */    
    0,                                      /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};

static BOARD_PARAMETERS g_bcm96348gw =
{
    "96348GW-HG520-1",                              /* szBoardId */
    {{BP_ENET_NO_PHY},                      /* usReverseMii */
     {BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
      0x00,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x04,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_SPI_SSB_0,             /* usConfigType */
      BP_ENET_REVERSE_MII}},                /* usReverseMii */
    {{BP_VOIP_DSP,                          /* ucDspType */
      0x00,                                 /* ucDspAddress */
      BP_EXT_INTR_2,                        /* usExtIntrVoip */
      BP_GPIO_6_AH,                         /* usGpioVoipReset */
      BP_GPIO_34_AH,                        /* usGpioVoipIntr */
      BP_NOT_DEFINED,                       /* usGpioLedVoip */
      BP_CS_2},                             /* usCsVoip */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_16MB_1_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_GPIO_31_AL,                          /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_GPIO_2_AL,                           /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_3_AL,                           /* usGpioLedWanData */ 
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_GPIO_3_AL,                           /* usGpioLedPpp */
    BP_GPIO_4_AL,                           /* usGpioLedPppFail */
    BP_GPIO_0_AL,                           /* usGpioLedBlPowerOn */
    BP_GPIO_3_AL,                           /* usGpioLedBlResetCfg */
    BP_GPIO_1_AL,                           /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    BP_GPIO_35_AL,                          /* usGpioSesBtnWireless */
    BP_EXT_INTR_3,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED, /* BP_GPIO_0_AL   */    /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usCsHpi */
    BP_NOT_DEFINED,                         /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    BP_NOT_DEFINED,                         /* usVcopeBoardRev */
    0,                                      /* usCMTThead */    
    0,                                      /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};

static BOARD_PARAMETERS g_bcm96348w2 =
{
    "96348GW-HG520-4",                              /* szBoardId */
    {{BP_ENET_NO_PHY},                      /* usReverseMii */
     {BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
      0x00,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x04,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_SPI_SSB_1,             /* usConfigType */
      BP_ENET_REVERSE_MII}},                /* usReverseMii */
    {{BP_VOIP_DSP,                          /* ucDspType */
      0x00,                                 /* ucDspAddress */
      BP_EXT_INTR_2,                        /* usExtIntrVoip */
      BP_NOT_DEFINED, //BP_GPIO_6_AH,                         /* usGpioVoipReset */
      BP_GPIO_34_AH,                        /* usGpioVoipIntr */
      BP_NOT_DEFINED,                       /* usGpioLedVoip */
      BP_CS_2},                             /* usCsVoip */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_16MB_2_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_GPIO_33_AL,                          /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_GPIO_2_AL,                           /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_3_AL,                           /* usGpioLedWanData */    
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_GPIO_3_AL,                           /* usGpioLedPpp */
    BP_GPIO_4_AL,                           /* usGpioLedPppFail */
    BP_GPIO_0_AL,                           /* usGpioLedBlPowerOn */
    BP_GPIO_3_AL,                           /* usGpioLedBlResetCfg */
    BP_GPIO_1_AL,                           /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_WLAN_ANT_AUX,                        /* usAntInUseWireless */
    BP_GPIO_35_AL,                          /* usGpioSesBtnWireless */
    BP_EXT_INTR_3,                          /* usExtIntrSesBtnWireless */
    BP_GPIO_6_AL,                           /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usCsHpi */
    BP_NOT_DEFINED,                         /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    BP_NOT_DEFINED,                         /* usVcopeBoardRev */
    0,                                      /* usCMTThead */    
    0,                                      /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};

static BOARD_PARAMETERS g_bcm96348w3 =
{
    "96348W3",                              /* szBoardId */
    {{BP_ENET_INTERNAL_PHY,                 /* ucPhyType */
      0x01,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x01,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO,                  /* usConfigType */
      BP_NOT_DEFINED},                      /* usReverseMii */
     {BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
      0x00,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x03,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_SPI_SSB_0,             /* usConfigType */
      BP_ENET_REVERSE_MII}},                /* usReverseMii */
    {{BP_VOIP_DSP,                          /* ucDspType */
      0x00,                                 /* ucDspAddress */
      BP_EXT_INTR_2,                        /* usExtIntrVoip */
      BP_GPIO_6_AH,                         /* usGpioVoipReset */
      BP_GPIO_34_AH,                        /* usGpioVoipIntr */
      BP_NOT_DEFINED,                       /* usGpioLedVoip */
      BP_CS_2},                             /* usCsVoip */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_16MB_2_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_GPIO_33_AL,                          /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_GPIO_2_AL,                           /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_3_AL,                           /* usGpioLedWanData */    
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_GPIO_3_AL,                           /* usGpioLedPpp */
    BP_GPIO_4_AL,                           /* usGpioLedPppFail */
    BP_GPIO_0_AL,                           /* usGpioLedBlPowerOn */
    BP_GPIO_3_AL,                           /* usGpioLedBlResetCfg */
    BP_GPIO_1_AL,                           /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_WLAN_ANT_AUX,                        /* usAntInUseWireless */
    BP_GPIO_35_AL,                          /* usGpioSesBtnWireless */
    BP_EXT_INTR_3,                          /* usExtIntrSesBtnWireless */
    BP_GPIO_6_AL,                           /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usCsHpi */
    BP_NOT_DEFINED,                         /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    BP_NOT_DEFINED,                         /* usVcopeBoardRev */
    0,                                      /* usCMTThead */    
    0,                                      /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};

static BOARD_PARAMETERS g_bcm96348gw_10 =
{
    "96348GW-10",                           /* szBoardId */
    {{BP_ENET_INTERNAL_PHY,                 /* ucPhyType */
      0x01,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x01,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO,                  /* usConfigType */
      BP_NOT_DEFINED},                      /* usReverseMii */
     {BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
      0x00,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x03,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_SPI_SSB_1,             /* usConfigType */
      BP_ENET_REVERSE_MII}},                /* usReverseMii */
    {{BP_VOIP_DSP,                          /* ucDspType */
      0x00,                                 /* ucDspAddress */
      BP_EXT_INTR_2,                        /* usExtIntrVoip */
      BP_GPIO_6_AH,                         /* usGpioVoipReset */
      BP_GPIO_34_AH,                        /* usGpioVoipIntr */
      BP_NOT_DEFINED,                       /* usGpioLedVoip */
      BP_CS_2},                             /* usCsVoip */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_16MB_2_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_GPIO_33_AL,                          /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_GPIO_2_AL,                           /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_3_AL,                           /* usGpioLedWanData */    
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_GPIO_3_AL,                           /* usGpioLedPpp */
    BP_GPIO_4_AL,                           /* usGpioLedPppFail */
    BP_GPIO_0_AL,                           /* usGpioLedBlPowerOn */
    BP_GPIO_3_AL,                           /* usGpioLedBlResetCfg */
    BP_GPIO_1_AL,                           /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    BP_NOT_DEFINED,                         /* usGpioSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */     
    BP_NOT_DEFINED,                         /* usCsHpi */
    BP_NOT_DEFINED,                         /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    BP_NOT_DEFINED,                         /* usVcopeBoardRev */
    0,                                      /* usCMTThead */    
    0,                                      /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};

static BOARD_PARAMETERS g_bcm96348gw_11 =
{
    "96348GW-11",                           /* szBoardId */
    {{BP_ENET_NO_PHY},                      /* ucPhyType */
     {BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
      0x00,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x04,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_SPI_SSB_1,             /* usConfigType */
      BP_ENET_REVERSE_MII}},                /* usReverseMii */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_16MB_2_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_GPIO_33_AL,                          /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_GPIO_2_AL,                           /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_3_AL,                           /* usGpioLedWanData */    
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_GPIO_3_AL,                           /* usGpioLedPpp */
    BP_GPIO_4_AL,                           /* usGpioLedPppFail */
    BP_GPIO_0_AL,                           /* usGpioLedBlPowerOn */
    BP_GPIO_3_AL,                           /* usGpioLedBlResetCfg */
    BP_GPIO_1_AL,                           /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    BP_NOT_DEFINED,                         /* usGpioSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */     
    BP_NOT_DEFINED,                         /* usCsHpi */
    BP_NOT_DEFINED,                         /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    BP_NOT_DEFINED,                         /* usVcopeBoardRev */
    0,                                      /* usCMTThead */    
    0,                                      /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};

static BOARD_PARAMETERS g_bcm96348sv =
{
    "96348SV",                              /* szBoardId */
    {{BP_ENET_INTERNAL_PHY,                 /* ucPhyType */
      0x01,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x01,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO,                  /* usConfigType */
      BP_NOT_DEFINED},                      /* usReverseMii */
     {BP_ENET_EXTERNAL_PHY,                 /* ucPhyType */
      0x1f,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x01,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO,                  /* usConfigType */
      BP_NOT_DEFINED}},                     /* usReverseMii */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_32MB_2_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_NOT_DEFINED,                         /* usGpioLedWanData */    
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_NOT_DEFINED,                         /* usGpioLedPpp */
    BP_NOT_DEFINED,                         /* usGpioLedPppFail */
    BP_NOT_DEFINED,                         /* usGpioLedBlPowerOn */
    BP_NOT_DEFINED,                         /* usGpioLedBlResetCfg */
    BP_NOT_DEFINED,                         /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    BP_NOT_DEFINED,                         /* usGpioSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */     
    BP_NOT_DEFINED,                         /* usCsHpi */
    BP_NOT_DEFINED,                         /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    BP_NOT_DEFINED,                         /* usVcopeBoardRev */
    0,                                      /* usCMTThead */    
    0,                                      /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};


static BOARD_PARAMETERS g_bcm96348gw_dualDsp =
{
    "96348GW-DualDSP",                      /* szBoardId */
    {{BP_ENET_INTERNAL_PHY,                 /* ucPhyType */
      0x01,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x01,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO,                  /* usConfigType */
      BP_NOT_DEFINED},                      /* usReverseMii */
     {BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
      0x00,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x03,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_SPI_SSB_1,             /* usConfigType */
      BP_ENET_REVERSE_MII}},                /* usReverseMii */
    {{BP_VOIP_DSP,                          /* ucDspType */
      0x00,                                 /* ucDspAddress */
      BP_EXT_INTR_2,                        /* usExtIntrVoip */
      BP_UNEQUIPPED,                        /* usGpioVoipReset */
      BP_GPIO_34_AH,                        /* usGpioVoipIntr */
      BP_NOT_DEFINED,                       /* usGpioLedVoip */
      BP_CS_2},                             /* usCsVoip */
     {BP_VOIP_DSP,                          /* ucDspType */
      0x01,                                 /* ucDspAddress */
      BP_EXT_INTR_3,                        /* usExtIntrVoip */
      BP_UNEQUIPPED ,                       /* usGpioVoipReset */
      BP_GPIO_35_AH,                        /* usGpioVoipIntr */
      BP_NOT_DEFINED,                       /* usGpioLedVoip */
      BP_CS_3}},                            /* usCsVoip */
    BP_MEMORY_16MB_2_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_GPIO_33_AL,                          /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_GPIO_2_AL,                           /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_3_AL,                           /* usGpioLedWanData */    
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_GPIO_3_AL,                           /* usGpioLedPpp */
    BP_GPIO_4_AL,                           /* usGpioLedPppFail */
    BP_GPIO_0_AL,                           /* usGpioLedBlPowerOn */
    BP_GPIO_3_AL,                           /* usGpioLedBlResetCfg */
    BP_GPIO_1_AL,                           /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    BP_NOT_DEFINED,                         /* usGpioSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */     
    BP_NOT_DEFINED,                         /* usCsHpi */
    BP_NOT_DEFINED,                         /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    BP_NOT_DEFINED,                         /* usVcopeBoardRev */
    0,                                      /* usCMTThead */    
    0,                                      /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};


static BOARD_PARAMETERS g_bcmCustom_01 =
{
     "BCMCUST_01",                          /* szBoardId */
    {{BP_ENET_INTERNAL_PHY,                 /* ucPhyType */
      0x01,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x01,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO,                  /* usConfigType */
      BP_NOT_DEFINED},                      /* usReverseMii */
     {BP_ENET_NO_PHY,                       /* ucPhyType */
      0x00,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x01,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_SPI_SSB_1,             /* usConfigType */
      BP_ENET_REVERSE_MII}},                /* usReverseMii */
    {{BP_VOIP_DSP,                          /* ucDspType */
      0x00,                                 /* ucDspAddress */
      BP_EXT_INTR_2,                        /* usExtIntrVoip */
      BP_GPIO_36_AH,                        /* usGpioVoipReset */
      BP_GPIO_34_AL,                        /* usGpioVoipIntr */
      BP_NOT_DEFINED,                       /* usGpioLedVoip */
      BP_CS_2},                             /* usCsVoip */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_16MB_2_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_GPIO_33_AL,                          /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_GPIO_2_AL,                           /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_3_AL,                           /* usGpioLedWanData */    
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_GPIO_3_AL,                           /* usGpioLedPpp */
    BP_GPIO_4_AL,                           /* usGpioLedPppFail */
    BP_GPIO_0_AL,                           /* usGpioLedBlPowerOn */
    BP_GPIO_3_AL,                           /* usGpioLedBlResetCfg */
    BP_GPIO_1_AL,                           /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    BP_NOT_DEFINED,                         /* usGpioSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */     
    BP_NOT_DEFINED,                         /* usCsHpi */
    BP_NOT_DEFINED,                         /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    BP_NOT_DEFINED,                         /* usVcopeBoardRev */
    0,                                      /* usCMTThead */    
    0,                                      /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};

static BOARD_PARAMETERS g_bcm96348m =
{
    "96348M",                               /* szBoardId */
    {{BP_ENET_INTERNAL_PHY,                 /* ucPhyType */
      0x01,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x01,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO,                  /* usConfigType */
      BP_NOT_DEFINED},                      /* usReverseMii */
     {BP_ENET_NO_PHY}},                     /* ucPhyType */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_16MB_2_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_GPIO_33_AL,                          /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_GPIO_18_AL,                          /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_19_AL,                          /* usGpioLedWanData */    
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_GPIO_19_AL,                          /* usGpioLedPpp */
    BP_GPIO_20_AL,                          /* usGpioLedPppFail */
    BP_GPIO_16_AL,                          /* usGpioLedBlPowerOn */
    BP_GPIO_19_AL,                          /* usGpioLedBlResetCfg */
    BP_GPIO_17_AL,                          /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    BP_NOT_DEFINED,                         /* usGpioSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */
    BP_CS_1,                                /* usCsHpi */
    BP_GPIO_22_AL,                          /* usVcopeResetGpio */
    BP_GPIO_21_AL,                          /* usVcopeRelayGpio */
    3,                                      /* usVcopeBoardRev */
    0,                                      /* usCMTThead */    
    0,                                      /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};

static BOARD_PARAMETERS g_bcm96348mc =
{
    "96348MC",                              /* szBoardId */
    {{BP_ENET_INTERNAL_PHY,                 /* ucPhyType */
      0x01,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x01,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO,                  /* usConfigType */
      BP_NOT_DEFINED},                      /* usReverseMii */
     {BP_ENET_NO_PHY}},                     /* ucPhyType */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_16MB_2_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_GPIO_33_AL,                          /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_GPIO_27_AL,                          /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_NOT_DEFINED,                         /* usGpioLedWanData */    
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_NOT_DEFINED,                         /* usGpioLedPpp */
    BP_NOT_DEFINED,                         /* usGpioLedPppFail */
    BP_GPIO_25_AL,                          /* usGpioLedBlPowerOn */
    BP_NOT_DEFINED,                         /* usGpioLedBlResetCfg */
    BP_NOT_DEFINED,                         /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    BP_NOT_DEFINED,                         /* usGpioSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */
    BP_CS_1,                                /* usCsHpi */
    BP_GPIO_22_AL,                          /* usVcopeResetGpio */
    BP_GPIO_36_AL,                          /* usVcopeRelayGpio */
    3,                                      /* usVcopeBoardRev */
    0,                                      /* usCMTThead */    
    0,                                      /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};

static PBOARD_PARAMETERS g_BoardParms[] =
    {&g_96348HG520_F, &g_bcm96348r, &g_bcm96348lv, &g_bcm96348gw, &g_bcm96348w2, &g_bcm96348w3, &g_bcm96348gw_10,
     &g_bcm96348gw_11, &g_bcm96348sv, &g_bcm96348gw_dualDsp,
     &g_bcmCustom_01, &g_bcm96348m, &g_bcm96348mc, 0};
#endif

#if defined(_BCM96358_) || defined(CONFIG_BCM96358)

/* start of cfe added by y42304 20060512: ֧�ֻ�Ϊ96358��BroadcomӲ������һ����20Ƭ���� */
static BOARD_PARAMETERS g_hw96358gw_a =
{
    "HW6358GW_A",                           /* szBoardId */
    {{BP_ENET_INTERNAL_PHY,                 /* ucPhyType */
      0x01,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x01,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO,                  /* usConfigType */
      BP_NOT_DEFINED},                      /* usReverseMii */
     {BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
      0x0,                                  /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x03,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO_PSEUDO_PHY,       /* usConfigType */
      BP_ENET_REVERSE_MII}},                /* usReverseMii */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_32MB_1_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedWirelessFail */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_2_AH,                           /* usGpioLedWanData */     
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_NOT_DEFINED,                         /* usGpioLedHspa */
    BP_NOT_DEFINED,                         /* usGpioLedHspaFail */
    BP_NOT_DEFINED,                         /* usGpioLedInternet*/
    BP_NOT_DEFINED,                         /* usGpioLedInternetFail*/
    BP_NOT_DEFINED,                         /* usGpioLedPpp */
    BP_SERIAL_GPIO_2_AH,                    /* usGpioLedPppFail */
    BP_SERIAL_GPIO_0_AH,                    /* usGpioLedBlPowerOn */
    BP_NOT_DEFINED,                         /* usGpioLedBlResetCfg */
    BP_SERIAL_GPIO_1_AH,                    /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    BP_GPIO_37_AL,                          /* usGpioSesBtnWireless */
    BP_EXT_INTR_3,                          /* usExtIntrSesBtnWireless */
    BP_GPIO_11_AL,                          /* usGpioLedSesWireless */ 
    BP_CS_1,                                /* usCsHpi */
    BP_GPIO_9_AL,                           /* usVcopeResetGpio */
    BP_GPIO_10_AL,                          /* usVcopeRelayGpio */
    3,                                      /* usVcopeBoardRev */
    0,                                      /* usCMTThead */    
    GPIO_SERIAL_MODE_MASK,                  /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};
/* end of cfe added by y42304 20060512 */

/* start of cfe added by y42304 20060512: ֧�ֻ�Ϊ96358��BroadcomӲ������ܴ�ĵ��� */
static BOARD_PARAMETERS g_huawei96358gw_b =
{
    "HW6358GW_B",                             /* szBoardId */
    {{BP_ENET_NO_PHY},                      /* usReverseMii */
     {BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
      0x0,                                  /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x04,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO_PSEUDO_PHY,       /* usConfigType */
      BP_ENET_REVERSE_MII}},                /* usReverseMii */
    {{BP_VOIP_NO_DSP},                     /* ucDspType */  
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_32MB_1_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_GPIO_37_AL,                          /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedWirelessFail */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_32_AL,                          /* usGpioLedWanData */     
    BP_NOT_DEFINED,   //BP_GPIO_5_AL        /* usGpioLedWanDataFail: Ӳ��ȥ��INET ��� */
    BP_NOT_DEFINED,                         /* usGpioLedHspa */
    BP_NOT_DEFINED,                         /* usGpioLedHspaFail */
    BP_NOT_DEFINED,                         /* usGpioLedInternet*/
    BP_NOT_DEFINED,                         /* usGpioLedInternetFail*/
    BP_NOT_DEFINED,                         /* usGpioLedPpp */
    BP_NOT_DEFINED,                         /* usGpioLedPppFail */
    BP_NOT_DEFINED,                         /* usGpioLedBlPowerOn */
    BP_NOT_DEFINED,                         /* usGpioLedBlResetCfg */
    BP_NOT_DEFINED,                         /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    BP_NOT_DEFINED,                         /* usGpioSesBtnWireless */
    BP_EXT_INTR_3,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */ 
    BP_CS_1,                                /* usCsHpi */
    BP_GPIO_9_AL,                           /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    3,                                      /* usVcopeBoardRev */
    0,                                      /* usCMTThead */    
    GPIO_GET_PCB_VER_MASK | GPIO_GET_BOARD_VER_MASK,         /* Hardware config */
    {BP_GPIO_10_AL,                         /* PCB version low bit GPIO */
     BP_GPIO_11_AL},                        /* PCB version high bit GPIO */
    {BP_GPIO_6_AL,                          /* Board version low bit GPIO */
     BP_GPIO_7_AL,                          /* Board version middle bit GPIO */                   
     BP_GPIO_8_AL,                          /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};
/* end of cfe added by y42304 20060512 */

/* start of cfe added by y42304 20061108: ֧�ֻ�ΪHUAWEI HG550II */
static BOARD_PARAMETERS g_huawei96358gw_HG550 =
{
    "HW550",                                /* szBoardId */
    {{BP_ENET_NO_PHY},                      /* usReverseMii */
     {BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
      0x0,                                  /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x04,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO_PSEUDO_PHY,       /* usConfigType */
      BP_ENET_REVERSE_MII}},                /* usReverseMii */
    {{BP_VOIP_DSP,                          /* ucDspType */
      0,                                    /* ucDspAddress */
      0,                                    /* usExtIntrVoip */
      0,                                    /* usGpioVoipReset */
      0,                                    /* usGpioVoipIntr */
      BP_GPIO_23_AL,                        /* usGpioLedVoip */
      BP_GPIO_4_AL,                         /* usGpioLedPSTN  */
      BP_GPIO_25_AL,                        /* usGpioLedLine0 */        
      BP_GPIO_26_AL,                        /* usGpioLedLine1 */      
      0,                                    /* usCsVoip */  
      BP_GPIO_14_AH,                        /* usGpioRelayCtl*/
      BP_GPIO_15_AH,                        /* usGpioRingDetc*/
      BP_GPIO_16_AL,                        /* usGpioLoopDetc*/      
      Silicon_Labs_3210,                    /* slic type */
      DAA_Not_Defined},                     /* DAA  type */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_32MB_1_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_GPIO_37_AL,                          /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_GPIO_22_AL,                          /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedWirelessFail */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_34_AL,                          /* usGpioLedWanData */     
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_NOT_DEFINED,                         /* usGpioLedHspa */
    BP_NOT_DEFINED,                         /* usGpioLedHspaFail */
    BP_NOT_DEFINED,                         /* usGpioLedInternet*/
    BP_NOT_DEFINED,                         /* usGpioLedInternetFail*/
    BP_NOT_DEFINED,                         /* usGpioLedPpp */
    BP_NOT_DEFINED,                         /* usGpioLedPppFail */
    BP_NOT_DEFINED,                         /* usGpioLedBlPowerOn */
    BP_NOT_DEFINED,                         /* usGpioLedBlResetCfg */
    BP_NOT_DEFINED,                         /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    BP_NOT_DEFINED,                         /* usGpioSesBtnWireless */
    BP_EXT_INTR_3,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */ 
    BP_NOT_DEFINED,                         /* usCsHpi */
    BP_NOT_DEFINED,                         /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    BP_NOT_DEFINED,                         /* usVcopeBoardRev */
    1,                                      /* usCMTThead */    
    GPIO_GET_PCB_VER_MASK | GPIO_GET_BOARD_VER_MASK,      /* Hardware config */
    {BP_GPIO_10_AL,                         /* PCB version low bit GPIO */
     BP_GPIO_11_AL},                        /* PCB version high bit GPIO */
    {BP_GPIO_6_AL,                          /* Board version low bit GPIO */
     BP_GPIO_7_AL,                          /* Board version middle bit GPIO */                   
     BP_GPIO_8_AL,                          /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};
/* end of cfe added by y42304 20060512 */

/* start of cfe added by y42304 20061108: ֧�ֻ�ΪHUAWEI HG550 ADM6996 lanswitch */
static BOARD_PARAMETERS g_huawei96358gw_HG550_FXO =
{
    "HW550_FXO",                            /* szBoardId */
    {{BP_ENET_NO_PHY},                      /* usReverseMii */
     {BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
      0x20,                                 /* ucPhyAddress */    /*fix 0x20*/
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x04,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_SMI,                   /* usConfigType */
      BP_ENET_REVERSE_MII}},                /* usReverseMii */
    {{BP_VOIP_DSP,                          /* ucDspType */
      0,                                    /* ucDspAddress */
      0,                                    /* usExtIntrVoip */
      0,                                    /* usGpioVoipReset */
      0,                                    /* usGpioVoipIntr */
      BP_GPIO_23_AL,                        /* usGpioLedVoip */
      BP_GPIO_4_AL,                         /* usGpioLedPSTN  */
      BP_GPIO_25_AL,                        /* usGpioLedLine0 */        
      BP_GPIO_26_AL,                        /* usGpioLedLine1 */
      0,                                    /* usCsVoip */  
      BP_GPIO_14_AH,                        /* usGpioRelayCtl*/
      BP_GPIO_35_AH,                        /* usGpioRingDetc*/
      BP_GPIO_36_AH,                        /* usGpioLoopDetc*/      
      Silicon_Labs_3215,                    /* slic type */
      DAA_CIRCUIT},                         /* DAA  type */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_32MB_1_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_GPIO_37_AL,                          /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_GPIO_22_AL,                          /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_GPIO_9_AL,                           /* wlan key  */
    BP_NOT_DEFINED,                         /* usGpioLedWirelessFail */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_34_AL,                          /* usGpioLedWanData */     
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_NOT_DEFINED,                         /* usGpioLedHspa */
    BP_NOT_DEFINED,                         /* usGpioLedHspaFail */
    BP_NOT_DEFINED,                         /* usGpioLedInternet*/
    BP_NOT_DEFINED,                         /* usGpioLedInternetFail*/
    BP_NOT_DEFINED,                         /* usGpioLedPpp */
    BP_NOT_DEFINED,                         /* usGpioLedPppFail */
    BP_NOT_DEFINED,                         /* usGpioLedBlPowerOn */
    BP_NOT_DEFINED,                         /* usGpioLedBlResetCfg */
    BP_NOT_DEFINED,                         /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    BP_NOT_DEFINED,                         /* usGpioSesBtnWireless */
    BP_EXT_INTR_3,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */ 
    BP_NOT_DEFINED,                         /* usCsHpi */
    BP_NOT_DEFINED,                         /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    BP_NOT_DEFINED,                         /* usVcopeBoardRev */
    1,                                      /* usCMTThead */    
    GPIO_GET_PCB_VER_MASK | GPIO_GET_BOARD_VER_MASK | GPIO_LED_MODE_MMASK,   /* Hardware config */
    {BP_GPIO_10_AL,                         /* PCB version low bit GPIO */
     BP_GPIO_11_AL},                        /* PCB version high bit GPIO */
    {BP_GPIO_6_AL,                          /* Board version low bit GPIO */
     BP_GPIO_7_AL,                          /* Board version middle bit GPIO */                   
     BP_GPIO_8_AL,                          /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};
/* end of cfe added by y42304 20060512 */

#define SW_INFO_DEFAULT(n) { \
  (0x00000001 << (n)) - 1,      /* port_map */ \
  {0, 0, 0, 0, 0, 0, 0, 0}      /* phy_id */ \
}

/* s48571 add for VDF HG553 porting 20080104 begin*/
static BOARD_PARAMETERS g_huawei96358gw_HG553 =
{
    "HW553",                            /* szBoardId */
    (BP_OVERLAY_PCI | 
        BP_OVERLAY_SPI_EXT_CS),                /* usGPIOOverlay */
    {{BP_ENET_NO_PHY} ,                  /* sw */
     {BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
      0x0,                                  /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      BP_NOT_DEFINED,                       /* usGpioPhyLinkSpeed */
      BP_ENET_CONFIG_MDIO_PSEUDO_PHY,       /* usConfigType */
      BP_ENET_REVERSE_MII,                   /* usReverseMii */
      SW_INFO_DEFAULT(4)}},                 /* sw */
    {{BP_VOIP_DSP,                          /* ucDspType */
      0,                                    /* ucDspAddress */
      0,                                    /* usExtIntrVoip */
      0,                                    /* usGpioVoipReset */
      0,                                    /* usGpioVoipIntr */
      BP_NOT_DEFINED,                        /* usGpioLedVoip */
      BP_NOT_DEFINED,                         /* usGpioLedPSTN  */
      BP_NOT_DEFINED,                        /* usGpioLedLine0 */        
      BP_NOT_DEFINED,                        /* usGpioLedLine1 */
      0,                                    /* usCsVoip */  
      BP_NOT_DEFINED,                        /* usGpioRelayCtl*/
      BP_NOT_DEFINED,                        /* usGpioRingDetc*/
      BP_NOT_DEFINED,                        /* usGpioLoopDetc*/      
      Legerity_88221,                    /* slic type */
      DAA_Not_Defined},                         /* DAA  type */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_64MB_1_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_GPIO_37_AL,                          /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_GPIO_23_AL,                          /* usGpioLedAdsl */
    BP_GPIO_22_AL,                         /* usGpioLedAdslFail */
    BP_GPIO_9_AL,                           /* wlan key  */
    BP_GPIO_25_AL,                         /* usGpioLedWirelessFail */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_NOT_DEFINED,                          /* usGpioLedWanData */     
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_GPIO_13_AL,                         /* usGpioLedHspa */
    BP_GPIO_12_AL,                         /* usGpioLedHspaFail */
    BP_GPIO_35_AL,                         /* usGpioLedInternet*/
    BP_GPIO_34_AL,                         /* usGpioLedInternetFail*/
    BP_NOT_DEFINED,                         /* usGpioLedPpp */
    BP_NOT_DEFINED,                         /* usGpioLedPppFail */
    BP_GPIO_4_AL,                         /* usGpioLedBlPowerOn */
    BP_NOT_DEFINED,                         /* usGpioLedBlResetCfg */
    BP_GPIO_5_AL,                         /* usGpioLedBlStop */
    BP_NOT_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    BP_GPIO_9_AL,                         /* usGpioSesBtnWireless */
    BP_EXT_INTR_3,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */ 
    BP_NOT_DEFINED,                         /* usCsHpi */
    BP_NOT_DEFINED,                         /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    BP_NOT_DEFINED,                         /* usVcopeBoardRev */
    1,                                      /* usCMTThead */    
    GPIO_GET_BOARD_VER_MASK | GPIO_LED_MODE_MMASK,   /* Hardware config */
    {BP_NOT_DEFINED,                         /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                        /* PCB version high bit GPIO */
    {BP_GPIO_7_AL,                          /* Board version low bit GPIO */
     BP_NOT_DEFINED,                          /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                          /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};

/* s48571 add for VDF HG553 porting 20080104 end */


static BOARD_PARAMETERS g_bcm96358vw =
{
    "96358VW",                              /* szBoardId */
    {{BP_ENET_INTERNAL_PHY,                 /* ucPhyType */
      0x01,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x01,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO,                  /* usConfigType */
      BP_NOT_DEFINED},                      /* usReverseMii */
     {BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
      0x0,                                  /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x03,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO_PSEUDO_PHY,       /* usConfigType */
      BP_ENET_REVERSE_MII}},                /* usReverseMii */
    {{BP_VOIP_DSP,                          /* ucDspType */
      0,                                    /* ucDspAddress */
      0,                                    /* usExtIntrVoip */
      0,                                    /* usGpioVoipReset */
      0,                                    /* usGpioVoipIntr */
      BP_GPIO_23_AL,                        /* usGpioLedVoip */
      BP_GPIO_4_AL,                         /* usGpioLedPSTN  */
      BP_GPIO_25_AL,                        /* usGpioLedLine0 */        
      BP_GPIO_26_AL,                        /* usGpioLedLine1 */
      BP_GPIO_14_AH,                        /*usGpioRelayCtl*/
      BP_GPIO_15_AH,                        /*usGpioRingDetc*/
      BP_GPIO_16_AL,                        /*usGpioLoopDetc*/      
      Legerity_88221,                       /* slic type */
      DAA_Not_Defined},                     /* DAA  type */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */                   /* ucDspType */
    BP_MEMORY_32MB_1_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_GPIO_15_AL,                          /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedWirelessFail */
    BP_GPIO_3_AL,                           /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_22_AL,                          /* usGpioLedWanData */    
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_NOT_DEFINED,                         /* usGpioLedHspa */
    BP_NOT_DEFINED,                         /* usGpioLedHspaFail */
    BP_NOT_DEFINED,                         /* usGpioLedInternet*/
    BP_NOT_DEFINED,                         /* usGpioLedInternetFail*/
    BP_GPIO_22_AL,                          /* usGpioLedPpp */
    BP_GPIO_23_AL,                          /* usGpioLedPppFail */
    BP_GPIO_4_AL,                           /* usGpioLedBlPowerOn */
    BP_GPIO_22_AL,                          /* usGpioLedBlResetCfg */
    BP_GPIO_5_AL,                           /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    BP_GPIO_37_AL,                          /* usGpioSesBtnWireless */
    BP_EXT_INTR_3,                          /* usExtIntrSesBtnWireless */
    BP_SERIAL_GPIO_7_AL,                    /* usGpioLedSesWireless */     
    BP_NOT_DEFINED,                         /* usCsHpi */
    BP_NOT_DEFINED,                         /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    BP_NOT_DEFINED,                         /* usVcopeBoardRev */
    1,                                      /* usCMTThead */    
    0,                                      /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};

static BOARD_PARAMETERS g_bcm96358vw16 =
{
    "96358VW-16",                           /* szBoardId */
    {{BP_ENET_INTERNAL_PHY,                 /* ucPhyType */
      0x01,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x01,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO,                  /* usConfigType */
      BP_NOT_DEFINED},                      /* usReverseMii */
     {BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
      0x0,                                  /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x03,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO_PSEUDO_PHY,       /* usConfigType */
      BP_ENET_REVERSE_MII}},                /* usReverseMii */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_16MB_1_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_GPIO_15_AL,                          /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedWirelessFail */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_22_AL,                          /* usGpioLedWanData */    
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_NOT_DEFINED,                         /* usGpioLedHspa */
    BP_NOT_DEFINED,                         /* usGpioLedHspaFail */
    BP_NOT_DEFINED,                         /* usGpioLedInternet*/
    BP_NOT_DEFINED,                         /* usGpioLedInternetFail*/
    BP_GPIO_22_AL,                          /* usGpioLedPpp */
    BP_GPIO_23_AL,                          /* usGpioLedPppFail */
    BP_GPIO_4_AL,                           /* usGpioLedBlPowerOn */
    BP_GPIO_22_AL,                          /* usGpioLedBlResetCfg */
    BP_GPIO_5_AL,                           /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    BP_NOT_DEFINED,                          /* usGpioSesBtnWireless */
    BP_EXT_INTR_3,                          /* usExtIntrSesBtnWireless */
    BP_SERIAL_GPIO_7_AL,                    /* usGpioLedSesWireless */     
    BP_NOT_DEFINED,                         /* usCsHpi */
    BP_NOT_DEFINED,                         /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    BP_NOT_DEFINED,                         /* usVcopeBoardRev */
    1,                                      /* usCMTThead */    
    0,                                      /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};

static BOARD_PARAMETERS g_bcm96358gw =
{
    "96358GW",                              /* szBoardId */
    {{BP_ENET_INTERNAL_PHY,                 /* ucPhyType */
      0x01,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x01,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO,                  /* usConfigType */
      BP_NOT_DEFINED},                      /* usReverseMii */
     {BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
      0x0,                                  /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x03,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO_PSEUDO_PHY,       /* usConfigType */
      BP_ENET_REVERSE_MII}},                /* usReverseMii */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_32MB_1_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_GPIO_15_AL,                          /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedWirelessFail */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_22_AL,                          /* usGpioLedWanData */    
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_NOT_DEFINED,                         /* usGpioLedHspa */
    BP_NOT_DEFINED,                         /* usGpioLedHspaFail */
    BP_NOT_DEFINED,                         /* usGpioLedInternet*/
    BP_NOT_DEFINED,                         /* usGpioLedInternetFail*/
    BP_GPIO_22_AL,                          /* usGpioLedPpp */
    BP_GPIO_23_AL,                          /* usGpioLedPppFail */
    BP_GPIO_4_AL,                           /* usGpioLedBlPowerOn */
    BP_GPIO_22_AL,                          /* usGpioLedBlResetCfg */
    BP_GPIO_5_AL,                           /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    BP_GPIO_37_AL,                          /* usGpioSesBtnWireless */
    BP_EXT_INTR_3,                          /* usExtIntrSesBtnWireless */
    BP_SERIAL_GPIO_7_AL,                    /* usGpioLedSesWireless */     
    BP_NOT_DEFINED,                         /* usCsHpi */
    BP_NOT_DEFINED,                         /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    BP_NOT_DEFINED,                         /* usVcopeBoardRev */
    0,                                      /* usCMTThead */    
    0,                                      /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};

static BOARD_PARAMETERS g_bcm96358gw16 =
{
    "96358GW-16",                           /* szBoardId */
    {{BP_ENET_INTERNAL_PHY,                 /* ucPhyType */
      0x01,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x01,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO,                  /* usConfigType */
      BP_NOT_DEFINED},                      /* usReverseMii */
     {BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
      0x0,                                  /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x03,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO_PSEUDO_PHY,       /* usConfigType */
      BP_ENET_REVERSE_MII}},                /* usReverseMii */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_16MB_1_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_GPIO_15_AL,                          /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedWirelessFail */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_22_AL,                          /* usGpioLedWanData */    
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_NOT_DEFINED,                         /* usGpioLedHspa */
    BP_NOT_DEFINED,                         /* usGpioLedHspaFail */
    BP_NOT_DEFINED,                         /* usGpioLedInternet*/
    BP_NOT_DEFINED,                         /* usGpioLedInternetFail*/
    BP_GPIO_22_AL,                          /* usGpioLedPpp */
    BP_GPIO_23_AL,                          /* usGpioLedPppFail */
    BP_GPIO_4_AL,                           /* usGpioLedBlPowerOn */
    BP_GPIO_22_AL,                          /* usGpioLedBlResetCfg */
    BP_GPIO_5_AL,                           /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    BP_GPIO_37_AL,                          /* usGpioSesBtnWireless */
    BP_EXT_INTR_3,                          /* usExtIntrSesBtnWireless */
    BP_SERIAL_GPIO_7_AL,                    /* usGpioLedSesWireless */     
    BP_NOT_DEFINED,                         /* usCsHpi */
    BP_NOT_DEFINED,                         /* usVcopeResetGpio */
    BP_NOT_DEFINED,                         /* usVcopeRelayGpio */
    BP_NOT_DEFINED,                         /* usVcopeBoardRev */
    0,                                      /* usCMTThead */    
    0,                                      /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};

static BOARD_PARAMETERS g_bcm96358m =
{
    "96358M",                               /* szBoardId */
    {{BP_ENET_INTERNAL_PHY,                 /* ucPhyType */
      0x01,                                 /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x01,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO,                  /* usConfigType */
      BP_NOT_DEFINED},                      /* usReverseMii */
     {BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
      0x0,                                  /* ucPhyAddress */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSck */
      BP_NOT_DEFINED,                       /* usGpioPhySpiSs */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMosi */
      BP_NOT_DEFINED,                       /* usGpioPhySpiMiso */
      BP_NOT_DEFINED,                       /* usGpioPhyReset */
      0x03,                                 /* numSwitchPorts */
      BP_ENET_CONFIG_MDIO_PSEUDO_PHY,       /* usConfigType */
      BP_ENET_REVERSE_MII}},                /* usReverseMii */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
     {BP_VOIP_NO_DSP}},                     /* ucDspType */
    BP_MEMORY_32MB_1_CHIP,                  /* usSdramSize */
    BP_PSI_DEFAULT_SIZE,                    /* usPsiSize */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioPressAndHoldReset */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_SERIAL_GPIO_3_AL,                    /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedWireless */
    BP_NOT_DEFINED,                         /* usGpioLedWirelessFail */
    BP_NOT_DEFINED,                         /* usGpioLedUsb */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_2_AH,                           /* usGpioLedWanData */     
    BP_NOT_DEFINED,                         /* usGpioLedWanDataFail */
    BP_NOT_DEFINED,                         /* usGpioLedHspa */
    BP_NOT_DEFINED,                         /* usGpioLedHspaFail */
    BP_NOT_DEFINED,                         /* usGpioLedInternet*/
    BP_NOT_DEFINED,                         /* usGpioLedInternetFail*/
    BP_GPIO_2_AH,                           /* usGpioLedPpp */
    BP_SERIAL_GPIO_3_AL,                    /* usGpioLedPppFail */
    BP_SERIAL_GPIO_0_AL,                    /* usGpioLedBlPowerOn */
    BP_SERIAL_GPIO_2_AL,                    /* usGpioLedBlResetCfg */
    BP_SERIAL_GPIO_1_AL,                    /* usGpioLedBlStop */
    BP_HW_DEFINED,                          /* usExtIntrAdslDyingGasp */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    BP_GPIO_37_AL,                          /* usGpioSesBtnWireless */
    BP_EXT_INTR_3,                          /* usExtIntrSesBtnWireless */
    BP_GPIO_11_AL,                          /* usGpioLedSesWireless */ 
    BP_CS_1,                                /* usCsHpi */
    BP_GPIO_9_AL,                           /* usVcopeResetGpio */
    BP_GPIO_10_AL,                          /* usVcopeRelayGpio */
    3,                                      /* usVcopeBoardRev */
    0,                                      /* usCMTThead */    
    GPIO_SERIAL_MODE_MASK,                  /* Hardware config */
    {BP_NOT_DEFINED,                        /* PCB version low bit GPIO */
     BP_NOT_DEFINED},                       /* PCB version high bit GPIO */
    {BP_NOT_DEFINED,                        /* Board version low bit GPIO */
     BP_NOT_DEFINED,                        /* Board version middle bit GPIO */                   
     BP_NOT_DEFINED,                        /* Board version high bit GPIO */       
     BP_NOT_DEFINED}                        /* usReserve */
};

/* s48571 modify for VDF HG553 porting 20080104 begin*/
static PBOARD_PARAMETERS g_BoardParms[] =
    {&g_hw96358gw_a, &g_huawei96358gw_b, &g_huawei96358gw_HG550,&g_huawei96358gw_HG553,&g_huawei96358gw_HG550_FXO, &g_bcm96358vw, &g_bcm96358vw16, &g_bcm96358gw, &g_bcm96358gw16, &g_bcm96358m, 0};
/* s48571 modify for VDF HG553 porting 20080104 end*/

#endif

static PBOARD_PARAMETERS g_pCurrentBp = 0;

/**************************************************************************
 * Name       : bpstrcmp
 *
 * Description: String compare for this file so it does not depend on an OS.
 *              (Linux kernel and CFE share this source file.)
 *
 * Parameters : [IN] dest - destination string
 *              [IN] src - source string
 *
 * Returns    : -1 - dest < src, 1 - dest > src, 0 dest == src
 ***************************************************************************/
static int bpstrcmp(const char *dest,const char *src);
static int bpstrcmp(const char *dest,const char *src)
{
    while (*src && *dest)
    {
        if (*dest < *src) return -1;
        if (*dest > *src) return 1;
        dest++;
        src++;
    }

    if (*dest && !*src) return 1;
    if (!*dest && *src) return -1;
    return 0;
} /* bpstrcmp */

/**************************************************************************
 * Name       : BpGetVoipDspConfig
 *
 * Description: Gets the DSP configuration from the board parameter
 *              structure for a given DSP index.
 *
 * Parameters : [IN] dspNum - DSP index (number)
 *
 * Returns    : Pointer to DSP configuration block if found/valid, NULL
 *              otherwise.
 ***************************************************************************/
VOIP_DSP_INFO *BpGetVoipDspConfig( unsigned char dspNum );
VOIP_DSP_INFO *BpGetVoipDspConfig( unsigned char dspNum )
{
    VOIP_DSP_INFO *pDspConfig = 0;
    int i;

    if( g_pCurrentBp )
    {
        for( i = 0 ; i < BP_MAX_VOIP_DSP ; i++ )
        {
            if( g_pCurrentBp->VoIPDspInfo[i].ucDspType != BP_VOIP_NO_DSP &&
                g_pCurrentBp->VoIPDspInfo[i].ucDspAddress == dspNum )
            {
                pDspConfig = &g_pCurrentBp->VoIPDspInfo[i];
                break;
            }
        }
    }

    return pDspConfig;
}


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
int BpSetBoardId( char *pszBoardId )
{
    int nRet = BP_BOARD_ID_NOT_FOUND;
    PBOARD_PARAMETERS *ppBp;

    for( ppBp = g_BoardParms; *ppBp; ppBp++ )
    {
        if( !bpstrcmp((*ppBp)->szBoardId, pszBoardId) )
        {
            g_pCurrentBp = *ppBp;
            nRet = BP_SUCCESS;
            break;
        }
    }

    return( nRet );
} /* BpSetBoardId */

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
int BpGetBoardIds( char *pszBoardIds, int nBoardIdsSize )
{
    PBOARD_PARAMETERS *ppBp;
    int i;
    char *src;
    char *dest;

    for( i = 0, ppBp = g_BoardParms; *ppBp && nBoardIdsSize;
        i++, ppBp++, nBoardIdsSize--, pszBoardIds += BP_BOARD_ID_LEN )
    {
        dest = pszBoardIds;
        src = (*ppBp)->szBoardId;
        while( *src )
            *dest++ = *src++;
        *dest = '\0';
    }

    return( i );
} /* BpGetBoardIds */

/**************************************************************************
 * Name       : BpGetGPIOverlays
 *
 * Description: This function GPIO overlay configuration
 *
 * Parameters : [OUT] pusValue - Address of short word that interfaces in use.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetGPIOverlays( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGPIOOverlay;

        if( g_pCurrentBp->usGPIOOverlay != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetGPIOverlays */

/**************************************************************************
 * Name       : BpGetEthernetMacInfo
 *
 * Description: This function returns all of the supported board id strings.
 *
 * Parameters : [OUT] pEnetInfos - Address of an array of ETHERNET_MAC_INFO
 *                  buffers.
 *              [IN] nNumEnetInfos - Number of ETHERNET_MAC_INFO elements that
 *                  are pointed to by pEnetInfos.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 ***************************************************************************/
int BpGetEthernetMacInfo( PETHERNET_MAC_INFO pEnetInfos, int nNumEnetInfos )
{
    int i, nRet;

    if( g_pCurrentBp )
    {
        for( i = 0; i < nNumEnetInfos; i++, pEnetInfos++ )
        {
            if( i < BP_MAX_ENET_MACS )
            {
                unsigned char *src = (unsigned char *)
                    &g_pCurrentBp->EnetMacInfos[i];
                unsigned char *dest = (unsigned char *) pEnetInfos;
                int len = sizeof(ETHERNET_MAC_INFO);
                while( len-- )
                    *dest++ = *src++;
            }
            else
                pEnetInfos->ucPhyType = BP_ENET_NO_PHY;
        }

        nRet = BP_SUCCESS;
    }
    else
    {
        for( i = 0; i < nNumEnetInfos; i++, pEnetInfos++ )
            pEnetInfos->ucPhyType = BP_ENET_NO_PHY;

        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetEthernetMacInfo */

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
int BpGetSdramSize( unsigned long *pulSdramSize )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pulSdramSize = g_pCurrentBp->usSdramSize;
        nRet = BP_SUCCESS;
    }
    else
    {
        *pulSdramSize = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetSdramSize */

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
int BpGetPsiSize( unsigned long *pulPsiSize )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pulPsiSize = g_pCurrentBp->usPsiSize;
        nRet = BP_SUCCESS;
    }
    else
    {
        *pulPsiSize = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetPsiSize */

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
    unsigned short *pusOuter )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusInner = g_pCurrentBp->usGpioRj11InnerPair;
        *pusOuter = g_pCurrentBp->usGpioRj11OuterPair;

        if( g_pCurrentBp->usGpioRj11InnerPair != BP_NOT_DEFINED &&
            g_pCurrentBp->usGpioRj11OuterPair != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusInner = *pusOuter = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetRj11InnerOuterPairGpios */

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
int BpGetPressAndHoldResetGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioPressAndHoldReset;

        if( g_pCurrentBp->usGpioPressAndHoldReset != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetPressAndHoldResetGpio */

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
int BpGetVoipResetGpio( unsigned char dspNum, unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        VOIP_DSP_INFO *pDspInfo = BpGetVoipDspConfig( dspNum );

        if( pDspInfo )
        {
           *pusValue = pDspInfo->usGpioVoipReset;

           if( *pusValue != BP_NOT_DEFINED ||
               *pusValue == BP_UNEQUIPPED )
           {
              nRet = BP_SUCCESS;
           }
           else
           {
              nRet = BP_VALUE_NOT_DEFINED;
           }
        }
        else
        {
           *pusValue = BP_NOT_DEFINED;
           nRet = BP_BOARD_ID_NOT_FOUND;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetVoipResetGpio */

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
int BpGetVoipIntrGpio( unsigned char dspNum, unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        VOIP_DSP_INFO *pDspInfo = BpGetVoipDspConfig( dspNum );

        if( pDspInfo )
        {
           *pusValue = pDspInfo->usGpioVoipIntr;

           if( *pusValue != BP_NOT_DEFINED )
           {
              nRet = BP_SUCCESS;
           }
           else
           {
              nRet = BP_VALUE_NOT_DEFINED;
           }
        }
        else
        {
           *pusValue = BP_NOT_DEFINED;
           nRet = BP_BOARD_ID_NOT_FOUND;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetVoipIntrGpio */

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
 *              BP_BOARD_ID_NOT_SET - Error, board id input string does not
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetRtsCtsUartGpios( unsigned short *pusRts, unsigned short *pusCts )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusRts = g_pCurrentBp->usGpioUartRts;
        *pusCts = g_pCurrentBp->usGpioUartCts;

        if( g_pCurrentBp->usGpioUartRts != BP_NOT_DEFINED &&
            g_pCurrentBp->usGpioUartCts != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusRts = *pusCts = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetUartRtsCtsGpios */

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
int BpGetAdslLedGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedAdsl;

        if( g_pCurrentBp->usGpioLedAdsl != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetAdslLedGpio */

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
int BpGetAdslFailLedGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedAdslFail;

        if( g_pCurrentBp->usGpioLedAdslFail != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetAdslFailLedGpio */

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
int BpGetWirelessLedGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedWireless;

        if( g_pCurrentBp->usGpioLedWireless != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetWirelessLedGpio */

/* HUAWEI HGW s48571 2008��1��18�� Hardware Porting add begin:*/
/**************************************************************************
 * Name       : BpGetWirelessFailLedGpio
 *
 * Description: This function returns the GPIO pin assignment for the WirelessFail
 *              LED.
 *
 * Parameters : [OUT] pusValue - Address of short word that the WirelessFail LED
 *                  GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetWirelessFailLedGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedWirelessFail;

        if( g_pCurrentBp->usGpioLedWirelessFail!= BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetWirelessLedGpio */
/* HUAWEI HGW s48571 2008��1��18�� Hardware Porting add end.*/

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
int BpGetWirelessAntInUse( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usAntInUseWireless;

        if( g_pCurrentBp->usAntInUseWireless != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );	
} /* BpGetWirelessAntInUse */
/*MODIFY BY YINHENG FOR 3.12 WIFIF DRIVER*/
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
int BpGetWirelessFlags( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usWirelessFlags;

        if( g_pCurrentBp->usWirelessFlags != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );	
} /* BpGetWirelessAntInUse */


/**************************************************************************
 * Name       : BpGetWirelessSesBtnGpio
 *
 * Description: This function returns the GPIO pin assignment for the Wireless
 *              Ses Button.
 *
 * Parameters : [OUT] pusValue - Address of short word that the Wireless LED
 *                  GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetWirelessSesBtnGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioSesBtnWireless;

        if( g_pCurrentBp->usGpioSesBtnWireless != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );	
} /* BpGetWirelessSesBtnGpio */

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
int BpGetWirelessSesExtIntr( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usExtIntrSesBtnWireless;

        if( g_pCurrentBp->usExtIntrSesBtnWireless != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );	
		
} /* BpGetWirelessSesExtIntr */

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
int BpGetWirelessSesLedGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedSesWireless;

        if( g_pCurrentBp->usGpioLedSesWireless != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
	
} /* BpGetWirelessSesLedGpio */


#if 1 /* 2008/01/28 Jiajun Weng : New code from 3.12L.01 */
/**************************************************************************
 * Name       : BpUpdateWirelessSromMap
 *
 * Description: This function patch wireless PA values
 *
 * Parameters : [IN] unsigned short chipID
 *              [IN/OUT] unsigned short* pBase - base of srom map
 *              [IN/OUT] int size - size of srom map
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpUpdateWirelessSromMap(unsigned short chipID, unsigned short* pBase, int sizeInWords)
{
    int nRet = BP_BOARD_ID_NOT_FOUND;
    int i = 0;
    int j = 0;
    
    /* this data structure could be moved to boardparams structure in the future */
    /* does not require to rebuild cfe here if more srom entries are needed */
    WLAN_SROM_PATCH_INFO wlanPaInfo[]={{"96348W3-10", 0x4318, 49, {{"pa0b0", 47, 0x14ae},
                                                                   {"pa0b1", 48, 0xfad4},
                                                                   {"pa0b2", 49, 0xfe82},
                                                                   {"",       0,      0}}},
                                                                   
                                       {"96348W3",    0x4318, 49, {{"pa0b0", 47, 0x14ae},
                                                                   {"pa0b1", 48, 0xfad4},
                                                                   {"pa0b2", 49, 0xfe82},
                                                                   {"",       0,      0}}},
                                       {"",                 0, 0, {{"",       0,      0}}}, /* last entry*/
                                       };
                                
    if(chipID == 0 || pBase == 0 || sizeInWords <= 0 )
        return nRet;
        
    i = 0;
    while ( wlanPaInfo[i].szboardId[0] != 0 ) {
    	/* check boardId */
        if ( !bpstrcmp(g_pCurrentBp->szBoardId, wlanPaInfo[i].szboardId) ) {
            /* check chipId */
            if ( (wlanPaInfo[i].usWirelessChipId == chipID) && (wlanPaInfo[i].usNeededSize <= sizeInWords) ){
                /* valid , patch entry */
            	while ( wlanPaInfo[i].entries[j].name[0] ) {
	            pBase[wlanPaInfo[i].entries[j].wordOffset] = wlanPaInfo[i].entries[j].value;
	            j++;
            	}
	        nRet = BP_SUCCESS;            	
                goto srom_update_done;
            }
        }
        i++;
    }

srom_update_done:
    
    return( nRet );
	
} /* BpUpdateWirelessSromMap */
#endif

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
int BpGetUsbLedGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedUsb;

        if( g_pCurrentBp->usGpioLedUsb != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetUsbLedGpio */

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
int BpGetHpnaLedGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedHpna;

        if( g_pCurrentBp->usGpioLedHpna != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetHpnaLedGpio */

 /**************************************************************************
 * ������  :   BpGetWanDataFailLedGpio
 * ����    :   ��ȡΪWAN fail data LED�����ŵ�GPIO��
 *
 * �������:   ��           
 * �������:   pusValue: ��ʾWAN fail data LED��16λ��ֵַ
 *
 * ����ֵ  :    BP_SUCCESS           :  �ɹ� 
 *              BP_VALUE_NOT_DEFINED : ��֧��
 *              BP_BOARD_ID_NOT_FOUND: ����, ������board ID������ 
 * ����    :    yuyouqing42304
 * �޸���ʷ:    2006-05-16����  
 ***************************************************************************/
int BpGetWanDataFailLedGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedWanDataFail;

        if( g_pCurrentBp->usGpioLedWanDataFail != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetWanDataFailLedGpio */

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
int BpGetWanDataLedGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedWanData;

        if( g_pCurrentBp->usGpioLedWanData != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetWanDataLedGpio */
/* HUAWEI HGW s48571 2008��1��18�� Hardware Porting add begin:*/
/**************************************************************************
 * Name       : BpGetHspaLedGpio
 *
 * Description: This function returns the GPIO pin assignment for the HSPA
 *              LED.
 *
 * Parameters : [OUT] pusValue - Address of short word that the HSPA LED
 *                  GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetHspaLedGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedHspa;

        if( g_pCurrentBp->usGpioLedHspa != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetPppLedGpio */

/**************************************************************************
 * Name       : BpGetHspaFailLedGpio
 *
 * Description: This function returns the GPIO pin assignment for the HSPA
 *              LED that is used when there is a HSPA connection failure.
 *
 * Parameters : [OUT] pusValue - Address of short word that the HSPA LED
 *                  GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetHspaFailLedGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedHspaFail;

        if( g_pCurrentBp->usGpioLedHspaFail != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetPppFailLedGpio */
/**************************************************************************
 * Name       : BpGetInternetLedGpio
 *
 * Description: This function returns the GPIO pin assignment for the Internet
 *              LED.
 *
 * Parameters : [OUT] pusValue - Address of short word that the Internet LED
 *                  GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetInternetLedGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedInternet;

        if( g_pCurrentBp->usGpioLedInternet != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetPppLedGpio */

/**************************************************************************
 * Name       : BpGetInternetFailLedGpio
 *
 * Description: This function returns the GPIO pin assignment for the Internet
 *              LED that is used when there is a PPP connection failure.
 *
 * Parameters : [OUT] pusValue - Address of short word that the Internet LED
 *                  GPIO pin is returned in.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 *                  for the board.
 ***************************************************************************/
int BpGetInternetFailLedGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedInternetFail;

        if( g_pCurrentBp->usGpioLedInternetFail != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetPppFailLedGpio */

/* HUAWEI HGW s48571 2008��1��18�� Hardware Porting add end.*/

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
int BpGetPppLedGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedPpp;

        if( g_pCurrentBp->usGpioLedPpp != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetPppLedGpio */

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
int BpGetPppFailLedGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedPppFail;

        if( g_pCurrentBp->usGpioLedPppFail != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetPppFailLedGpio */

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
int BpGetBootloaderPowerOnLedGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedBlPowerOn;

        if( g_pCurrentBp->usGpioLedBlPowerOn != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetBootloaderPowerOn */

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
int BpGetBootloaderResetCfgLedGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedBlResetCfg;

        if( g_pCurrentBp->usGpioLedBlResetCfg != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetBootloaderResetCfgLedGpio */

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
int BpGetBootloaderStopLedGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedBlStop;

        if( g_pCurrentBp->usGpioLedBlStop != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetBootloaderStopLedGpio */

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
 *
 * Note       : The VoIP structure would allow for having one LED per DSP
 *              however, the board initialization function assumes only one
 *              LED per functionality (ie one LED for VoIP).  Therefore in
 *              order to keep this tidy and simple we do not make usage of the
 *              one-LED-per-DSP function.  Instead, we assume that the LED for
 *              VoIP is unique and associated with DSP 0 (always present on
 *              any VoIP platform).  If changing this to a LED-per-DSP function
 *              then one need to update the board initialization driver in
 *              bcmdrivers\opensource\char\board\bcm963xx\impl1
 ***************************************************************************/
int BpGetVoipLedGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        VOIP_DSP_INFO *pDspInfo = BpGetVoipDspConfig( 0 );

        if( pDspInfo )
        {
           *pusValue = pDspInfo->usGpioLedVoip;

           if( *pusValue != BP_NOT_DEFINED )
           {
              nRet = BP_SUCCESS;
           }
           else
           {
              nRet = BP_VALUE_NOT_DEFINED;
           }
        }
        else
        {
           *pusValue = BP_NOT_DEFINED;
           nRet = BP_BOARD_ID_NOT_FOUND;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetVoipLedGpio */

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
int BpGetAdslDyingGaspExtIntr( unsigned long *pulValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pulValue = g_pCurrentBp->usExtIntrAdslDyingGasp;

        if( g_pCurrentBp->usExtIntrAdslDyingGasp != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pulValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetAdslDyingGaspExtIntr */

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
int BpGetVoipExtIntr( unsigned char dspNum, unsigned long *pulValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        VOIP_DSP_INFO *pDspInfo = BpGetVoipDspConfig( dspNum );

        if( pDspInfo )
        {
           *pulValue = pDspInfo->usExtIntrVoip;

           if( *pulValue != BP_NOT_DEFINED )
           {
              nRet = BP_SUCCESS;
           }
           else
           {
              nRet = BP_VALUE_NOT_DEFINED;
           }
        }
        else
        {
           *pulValue = BP_NOT_DEFINED;
           nRet = BP_BOARD_ID_NOT_FOUND;
        }
    }
    else
    {
        *pulValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetVoipExtIntr */

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
int BpGetHpnaExtIntr( unsigned long *pulValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pulValue = g_pCurrentBp->usExtIntrHpna;

        if( g_pCurrentBp->usExtIntrHpna != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pulValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetHpnaExtIntr */

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
int BpGetHpnaChipSelect( unsigned long *pulValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pulValue = g_pCurrentBp->usCsHpna;

        if( g_pCurrentBp->usCsHpna != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pulValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetHpnaChipSelect */

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
int BpGetVoipChipSelect( unsigned char dspNum, unsigned long *pulValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        VOIP_DSP_INFO *pDspInfo = BpGetVoipDspConfig( dspNum );

        if( pDspInfo )
        {
           *pulValue = pDspInfo->usCsVoip;

           if( *pulValue != BP_NOT_DEFINED )
           {
              nRet = BP_SUCCESS;
           }
           else
           {
              nRet = BP_VALUE_NOT_DEFINED;
           }
        }
        else
        {
           *pulValue = BP_NOT_DEFINED;
           nRet = BP_BOARD_ID_NOT_FOUND;
        }
    }
    else
    {
        *pulValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetVoipChipSelect */

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
int BpGetCMTThread( unsigned long *pulValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pulValue = g_pCurrentBp->usCMTThead;

        if( g_pCurrentBp->usCMTThead != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pulValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetCMTThread */


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
int BpGetVcopeGpio (int pio_idx)
{
    int     nRet = BP_SUCCESS;

    if( g_pCurrentBp )
    {
        if( pio_idx == BCM6505_RESET_GPIO )
            nRet = g_pCurrentBp->usVcopeResetGpio;
        else if( pio_idx == VCOPE_RELAY_GPIO)
            nRet = g_pCurrentBp->usVcopeRelayGpio;
        else if( pio_idx == HPI_CS )
            nRet = g_pCurrentBp->usCsHpi;
        else if( pio_idx == VCOPE_BOARD_REV )
            nRet = g_pCurrentBp->usVcopeBoardRev;
        else
            nRet = BP_NOT_DEFINED;
    }        
    else
        nRet = BP_NOT_DEFINED;
    
   return(nRet);
}


/**************************************************************************
 * ������  :   BpGetPCBVesionGpio
 * ����    :   ͨ����������ucPcbVerBit���õ���ʾλucPcbVerBit��GPIO��
 *             ���뵽�������pusValue   
 * �������:   ucPcbVerBit  :  ��ʾPCB�汾��Ϣĳλbit0��bit2��             
 * �������:   pusValue     :  ���GPIO              
 *
 * ����ֵ  :    BP_SUCCESS:  �ɹ�
 *              BP_BOARD_ID_NOT_FOUND: ����, ������board ID������
 * ����    :    yuyouqing42304
 * �޸���ʷ:    2006-05-16����  
 ***************************************************************************/

int BpGetPCBVesionGpio(unsigned char ucPcbVerBit, unsigned short *pusValue )
{
    int nRet;

    if (!g_pCurrentBp)
    {        
        *pusValue = BP_NOT_DEFINED;
        return BP_BOARD_ID_NOT_SET;        
    }

    switch(ucPcbVerBit)
    {
    case PCB_VERSION_BIT0:
        *pusValue = g_pCurrentBp->stPcbVersion.usPcbVer_Low;        
        break;            
        
    case PCB_VERSION_BIT1:
        *pusValue = g_pCurrentBp->stPcbVersion.usPcbVer_High;      
        break;            

    default:
        return BP_VALUE_NOT_DEFINED;
    }
  

    if( *pusValue != BP_NOT_DEFINED )
    {
        nRet = BP_SUCCESS;
    }
    else
    {
        nRet = BP_VALUE_NOT_DEFINED;
    }
 
    return( nRet );
} 

/**************************************************************************
 * ������  :   BpGetBoardVesionGpio
 * ����    :   ͨ����������ucBoardVerBit���õ���ʾλucBoardVerBit��GPIO��
 *             ���뵽�������pusValue   
 * �������:   ucBoardVerBit:  ��ʾBoard�汾��Ϣĳλbit0��bit2��             
 * �������:   pusValue     :  ���GPIO              
 *
 * ����ֵ  :    BP_SUCCESS:  �ɹ�
 *              BP_BOARD_ID_NOT_FOUND: ����, ������board ID������ 
 * ����    :    yuyouqing42304
 * �޸���ʷ:    2006-05-16����  
 ***************************************************************************/
int BpGetBoardVesionGpio(unsigned char ucBoardVerBit, unsigned short *pusValue )
{
    int nRet;

    *pusValue = 0;
    if (!g_pCurrentBp)
    {        
        *pusValue = BP_NOT_DEFINED;
        return BP_BOARD_ID_NOT_SET;        
    }

    switch(ucBoardVerBit)
    {
    case BOARD_VERSION_BIT0:
        *pusValue = g_pCurrentBp->stBoardVersion.usBoardVer_Low;      
        break;            
        
    case BOARD_VERSION_BIT1:
        *pusValue = g_pCurrentBp->stBoardVersion.usBoardVer_Mid;       
        break; 

    case BOARD_VERSION_BIT2:
        *pusValue = g_pCurrentBp->stBoardVersion.usBoardVer_High;       
        break;          

    default:
        return BP_VALUE_NOT_DEFINED;
        
    }
  
    if( *pusValue != BP_NOT_DEFINED )
    {
        nRet = BP_SUCCESS;
    }
    else
    {
        nRet = BP_VALUE_NOT_DEFINED;
    }
      
    return( nRet );
} 


/**************************************************************************
 * ������  :   BpIsSupportSerialGpioMode
 * ����    :   �ж�Ӳ���Ƿ�֧��serial GPIOģʽ
 *
 * �������:   ��           
 * �������:   ��
 *
 * ����ֵ  :    BP_SUCCESS           :  �ɹ� 
 *              BP_VALUE_NOT_DEFINED : ��֧��
 *              BP_BOARD_ID_NOT_FOUND: ����, ������board ID������ 
 * ����    :    yuyouqing42304
 * �޸���ʷ:    2006-05-16����  
 ***************************************************************************/
int BpIsSupportSerialGpioMode(void)
{  
    if ( g_pCurrentBp )
    {
        if ((g_pCurrentBp->usHardwareCfg) & GPIO_SERIAL_MODE_MASK )
        {
            return BP_SUCCESS;            
        }
        else
        {
            return BP_VALUE_NOT_DEFINED;
        }
    }
    return BP_BOARD_ID_NOT_SET;
}

/**************************************************************************
 * ������  :   BpIsSupportLedGpioMode
 * ����    :   �ж�Ӳ���Ƿ�֧��LED GPIOģʽ
 *
 * �������:   ��           
 * �������:   ��
 *
 * ����ֵ  :    BP_SUCCESS           :  �ɹ� 
 *              BP_VALUE_NOT_DEFINED : ��֧��
 *              BP_BOARD_ID_NOT_FOUND: ����, ������board ID������ 
 * ����    :    yuyouqing42304
 * �޸���ʷ:    2006-05-16����  
 ***************************************************************************/
int BpIsSupportLedGpioMode(void)
{
    if ( g_pCurrentBp )
    {
        if ((g_pCurrentBp->usHardwareCfg) & GPIO_LED_MODE_MMASK )
        {
            return BP_SUCCESS;            
        }
        else
        {
            return BP_VALUE_NOT_DEFINED;
        }
    }
    return BP_BOARD_ID_NOT_SET;    
}

/**************************************************************************
 * ������  :   BpIsSupportPcbVersion
 * ����    :   �ж�Ӳ���Ƿ�֧�ֶ�GPIO��ȡPCB�汾��
 *
 * �������:   ��           
 * �������:   ��
 *
 * ����ֵ  :    BP_SUCCESS           :  �ɹ� 
 *              BP_VALUE_NOT_DEFINED : ��֧��
 *              BP_BOARD_ID_NOT_FOUND: ����, ������board ID������ 
 * ����    :    yuyouqing42304
 * �޸���ʷ:    2006-05-16����  
 ***************************************************************************/
int BpIsSupportPcbVersion(void)
{    
    if ( g_pCurrentBp )
    {
        if ((g_pCurrentBp->usHardwareCfg) & GPIO_GET_PCB_VER_MASK )
        {
            return BP_SUCCESS;            
        }
        else
        {
            return BP_VALUE_NOT_DEFINED;
        }
    }
    return BP_BOARD_ID_NOT_SET;    
}

/**************************************************************************
 * ������  :   BpIsSupportBoardVersion
 * ����    :   �ж�Ӳ���Ƿ�֧�ֶ�GPIO��ȡBoard�汾��
 *
 * �������:   ��           
 * �������:   ��
 *
 * ����ֵ  :    BP_SUCCESS           :  �ɹ� 
 *              BP_VALUE_NOT_DEFINED : ��֧��
 *              BP_BOARD_ID_NOT_FOUND: ����, ������board ID������ 
 * ����    :    yuyouqing42304
 * �޸���ʷ:    2006-05-16����  
 ***************************************************************************/
int BpIsSupportBoardVersion(void)
{
    if ( g_pCurrentBp )
    {
        if ((g_pCurrentBp->usHardwareCfg) & GPIO_GET_BOARD_VER_MASK )
        {
            return BP_SUCCESS;            
        }
        else
        {
            return BP_VALUE_NOT_DEFINED;
        }
    }
    return BP_BOARD_ID_NOT_SET;        
}

/**************************************************************************
 * ������  :   BpGetTelLine0LedGpio
 * ����    :   ��ȡ��Ӧ��·��0 LED GPIO�� 
 * �������:   ��           
 * �������:   pusValue     :  ���GPIO              
 *
 * ����ֵ  :    BP_SUCCESS:  �ɹ�
 *              BP_BOARD_ID_NOT_FOUND: ����, ������board ID������
 * ����    :    yuyouqing42304
 * �޸���ʷ:    2006-11-09����  
 ***************************************************************************/

int BpGetTelLine0LedGpio(unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        VOIP_DSP_INFO *pDspInfo = BpGetVoipDspConfig( 0 );

        if( pDspInfo )
        {
           *pusValue = pDspInfo->usGpioLedLine0;

           if( *pusValue != BP_NOT_DEFINED )
           {
              nRet = BP_SUCCESS;
           }
           else
           {
              nRet = BP_VALUE_NOT_DEFINED;
           }
        }
        else
        {
           *pusValue = BP_NOT_DEFINED;
           nRet = BP_BOARD_ID_NOT_FOUND;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} 

/**************************************************************************
 * ������  :   BpGetTelLine1LedGpio
 * ����    :   ��ȡ��Ӧ��·��1 LED GPIO�� 
 * �������:   ��           
 * �������:   pusValue     :  ���GPIO              
 *
 * ����ֵ  :    BP_SUCCESS:  �ɹ�
 *              BP_BOARD_ID_NOT_FOUND: ����, ������board ID������
 * ����    :    yuyouqing42304
 * �޸���ʷ:    2006-11-09����  
 ***************************************************************************/

int BpGetTelLine1LedGpio(unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        VOIP_DSP_INFO *pDspInfo = BpGetVoipDspConfig( 0 );

        if( pDspInfo )
        {
           *pusValue = pDspInfo->usGpioLedLine1;

           if( *pusValue != BP_NOT_DEFINED )
           {
              nRet = BP_SUCCESS;
           }
           else
           {
              nRet = BP_VALUE_NOT_DEFINED;
           }
        }
        else
        {
           *pusValue = BP_NOT_DEFINED;
           nRet = BP_BOARD_ID_NOT_FOUND;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} 

/**************************************************************************
 * ����    :   ��ȡPSTN LED GPIO�� 
 * �������:   ��           
 * �������:   pusValue     :  ���GPIO              
 *
 * ����ֵ  :    BP_SUCCESS:  �ɹ�
 *              BP_BOARD_ID_NOT_FOUND: ����, ������board ID������
 * ����    :    yuyouqing42304
 * �޸���ʷ:    2006-11-09����  
 ***************************************************************************/

int BpGetPstnLedGpio(unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        VOIP_DSP_INFO *pDspInfo = BpGetVoipDspConfig( 0 );

        if( pDspInfo )
        {
           *pusValue = pDspInfo->usGpioLedPSTN;

           if( *pusValue != BP_NOT_DEFINED )
           {
              nRet = BP_SUCCESS;
           }
           else
           {
              nRet = BP_VALUE_NOT_DEFINED;
           }
        }
        else
        {
           *pusValue = BP_NOT_DEFINED;
           nRet = BP_BOARD_ID_NOT_FOUND;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} 

/**************************************************************************
 * ������  :   BpGetFxoRelayCtlGpio
 * ����    :   ��ȡ�̵����Ŀ���IO
 * �������:   ��           
 * �������:   pusValue     :  ���GPIO              
 *
 * ����ֵ  :    BP_SUCCESS:  �ɹ�
 *              BP_BOARD_ID_NOT_FOUND: ����, ������board ID������
 * ����    :    liujianfeng 37298
 * �޸���ʷ:    2006-12-04����
 ***************************************************************************/
int BpGetFxoRelayCtlGpio(unsigned short *pusValue )
{
    int nRet = BP_BOARD_ID_NOT_SET;
    if( g_pCurrentBp )
    {
        
        VOIP_DSP_INFO *pDspInfo = BpGetVoipDspConfig( 0 );
        if( pDspInfo )
        {            
            *pusValue = pDspInfo->usGpioRelayCtl;
            
            /*
            *Dangance,the complier may be set the variable to 0 if it's not 
            *be set a default value.In this case ,we get wrong value.
            */
            if( *pusValue != BP_NOT_DEFINED )
            {
                nRet = BP_SUCCESS;
            }
            else
            {
                nRet = BP_VALUE_NOT_DEFINED;
            }
        }        
        else
        {            
            *pusValue = 0xFF;
            nRet = BP_VALUE_NOT_DEFINED;
        }   
    }
    else
    {
        return BP_BOARD_ID_NOT_SET;
    }

    return nRet;
}


/**************************************************************************
 * ������  :   BpGetFxoRingDetcGpio
 * ����    :   ��ȡ�������IO
 * �������:   ��           
 * �������:   pusValue     :  ���GPIO              
 *
 * ����ֵ  :    BP_SUCCESS:  �ɹ�
 *              BP_BOARD_ID_NOT_FOUND: ����, ������board ID������
 * ����    :    liujianfeng 37298
 * �޸���ʷ:    2006-12-04����  
 ***************************************************************************/
int BpGetFxoRingDetcGpio(unsigned short *pusValue )
{
    int nRet = BP_BOARD_ID_NOT_SET;
    if( g_pCurrentBp )
    {        
        VOIP_DSP_INFO *pDspInfo = BpGetVoipDspConfig( 0 );        
        if( pDspInfo )
        {            
            *pusValue = pDspInfo->usGpioRingDetc;   

            /*
            *Dangance,the complier may be set the variable to 0 if it's not 
            *be set a default value.In this case ,we get wrong value.
            */
            if( *pusValue != BP_NOT_DEFINED )
            {
                nRet = BP_SUCCESS;
            }
            else
            {
                nRet = BP_VALUE_NOT_DEFINED;
            }
        }
        else
        {            
            *pusValue = 0xFF;
            nRet = BP_VALUE_NOT_DEFINED;
        }   
    }
    else
    {
        return BP_BOARD_ID_NOT_SET;
    }

    return nRet;
}


/**************************************************************************
 * ������  :   BpGetFxoLoopDetcGpio
 * ����    :   ��ȡ��·״̬����IO
 * �������:   ��           
 * �������:   pusValue     :  ���GPIO              
 *
 * ����ֵ  :    BP_SUCCESS:  �ɹ�
 *              BP_BOARD_ID_NOT_FOUND: ����, ������board ID������
 * ����    :    liujianfeng 37298
 * �޸���ʷ:    2006-12-04����  
 ***************************************************************************/
int BpGetFxoLoopDetcGpio(unsigned short *pusValue )
{
    int nRet = BP_BOARD_ID_NOT_SET;
    if( g_pCurrentBp )
    {        
        VOIP_DSP_INFO *pDspInfo = BpGetVoipDspConfig( 0 );
        if( pDspInfo )
        {            
            *pusValue = pDspInfo->usGpioLoopDetc;
            
            /*
            *Dangance,the complier may be set the variable to 0 if it's not 
            *be set a default value.In this case ,we get wrong value.
            */
            if( *pusValue != BP_NOT_DEFINED )
            {
                nRet = BP_SUCCESS;
            }
            else
            {
                nRet = BP_VALUE_NOT_DEFINED;
            }            
        }        
        else
        {            
            *pusValue = 0xFF;
            nRet = BP_VALUE_NOT_DEFINED;
        }   

    }
    else
    {
        return BP_BOARD_ID_NOT_SET;
    }

    return nRet;
}

/**************************************************************************
 * ����    :   ��ȡslic������ 
 * �������:   ��           
 * �������:   slicType     :  slic������           
 *
 * ����ֵ  :    BP_SUCCESS:  �ɹ�
 *              BP_BOARD_ID_NOT_FOUND: ����, ������board ID������
 * ����    :    yuyouqing42304
 * �޸���ʷ:    2006-12-20����  
 ***************************************************************************/
int BpGetSlicType( unsigned short *slicType )
{
    int nRet;

    if( g_pCurrentBp )
    {
        VOIP_DSP_INFO *pDspInfo = BpGetVoipDspConfig( 0 );

        if( pDspInfo )
        {
           *slicType = pDspInfo->slicType;

           if( *slicType != Slic_Not_Defined )
           {
              nRet = BP_SUCCESS;
           }
           else
           {
              nRet = BP_VALUE_NOT_DEFINED;
           }
        }
        else
        {
           *slicType = Slic_Not_Defined;
           nRet = BP_BOARD_ID_NOT_FOUND;
        }
    }
    else
    {
        *slicType = Slic_Not_Defined;
        nRet = BP_BOARD_ID_NOT_SET;
    }
    
    return( nRet );
} /* BpGetSlicType */


/**************************************************************************
 * ����    :   ��ȡDAA������ 
 * �������:   ��           
 * �������:   daaType     :  DAA������           
 *
 * ����ֵ  :    BP_SUCCESS:  �ɹ�
 *              BP_BOARD_ID_NOT_FOUND: ����, ������board ID������
 * ����    :    yuyouqing42304
 * �޸���ʷ:    2006-12-20����  
 ***************************************************************************/
int BpGetDAAType( unsigned short *daaType )
{
    int nRet;

    if( g_pCurrentBp )
    {
        VOIP_DSP_INFO *pDspInfo = BpGetVoipDspConfig( 0 );

        if( pDspInfo )
        {
           *daaType = pDspInfo->daaType;
            
           if( *daaType != DAA_Not_Defined )
           {
              nRet = BP_SUCCESS;
           }
           else
           {
              nRet = BP_VALUE_NOT_DEFINED;
           }
        }
        else
        {
           *daaType = DAA_Not_Defined;
           nRet = BP_BOARD_ID_NOT_FOUND;
        }
    }
    else
    {
        *daaType = DAA_Not_Defined;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );    
} /* BpGetDAAType */
