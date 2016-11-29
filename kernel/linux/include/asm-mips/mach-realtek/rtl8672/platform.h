#ifndef _PLATFORM_H
#define _PLATFORM_H


/*
 *  =============
 *  Utilty Macros
 *  =============
 */
#define REG8(reg)    (*(volatile unsigned char *)((unsigned int)reg))
#ifndef REG16	
#define REG16(reg)	 (*(volatile unsigned short *)(reg) ) 
#endif
#define REG32(reg)   (*(volatile unsigned int *)((unsigned int)reg))

//cathy
#define IS_6028A		(((REG32(0xB8003200) & 0x00110000) == 0x00110000) ? 1:0)	//b20=1, b16=1
#define IS_6028B		(((REG32(0xB8003200) & 0x00110000) == 0x00100000) ? 1:0)	//b20=1, b16=0
#define IS_6028C		(((REG32(0xB8003200) & 0x00110000) == 0x00110000) ? 1:0)	//b20=1, b16=1
#define IS_6085		(((REG32(0xB8003200) & 0x00110000) == 0x00010000) ? 1:0)	//b20=0, b16=1

/*
 *  ====================================
 *  Platform Configurable Common Options
 *  ====================================
 */

#define PROM_DEBUG      0

//#define MHZ             200  
#define MHZ             175
#define SYSCLK          MHZ * 1000 * 1000

#define BAUDRATE        115200  /* ex. 19200 or 38400 or 57600 or 115200 */ 
                               /* For Early Debug */

/*
 * Interrupt IRQ Assignments
 */
#define PTM_IRQ         31
#define LBCTMOs2_IRQ    30
#define LBCTMOs1_IRQ    29
#define PKT_IRQ         28
#define SPI_IRQ         27
#define NIC100_IRQ      26
#define SAR_IRQ         25
#define DMT_IRQ         24
#define PKT_NIC100_IRQ         23
#define PKT_NIC100MII_IRQ         22
#define GDMA_IRQ        21
#define SECURITY_IRQ    20
#define PCM_IRQ         19
#define PKT_SAR_IRQ		18
#define GPIO_EFGH_IRQ   17
#define GPIO_ABCD_IRQ   16
#define SW_IRQ          15
#define PCI_IRQ         14
#define UART1_IRQ       13
#define UART0_IRQ       12
#define USB_D_IRQ       11
#define USB_H_IRQ       10
#define TC1_IRQ         9
#define TC0_IRQ         8
#define LBCTMOm2_IRQ    7
#define LBCTMOm1_IRQ    6
#define SPEED_IRQ       5
#define LBCTMOs0_IRQ    4
#define LBCTMOm0_IRQ    3
#define OCPTMO_IRQ      2
#define PCIB0TO_IRQ     0

/*
 * Interrupt Routing Selection
 */
#define PTM_RS          2
#define LBCTMOs2_RS     2
#define LBCTMOs1_RS     2
#define PKT_RS          2
#define SPI_RS          2
#define NIC100_RS       2
#define SAR_RS          2
#define DMT_RS          2
#define PKT_NIC100_RS          2
#define PKT_NIC100MII_RS          2
#define PKT_SAR_RS          2
#define GDMA_RS         2
#define SECURITY_RS     2
#define PCM_RS          2
#define GPIO_EFGH_RS    2
#define GPIO_ABCD_RS    2
#define SW_RS           6
#define PCI_RS          5
#define UART1_RS        2
#define UART0_RS        3
#define USB_D_RS        2
//cathy
//#define USB_H_RS        4
#define USB_H_RS        2
#define TC1_RS          2
#define TC0_RS          7
#define LBCTMOm2_RS     2
#define LBCTMOm1_RS     2
#define SPEED_RS        2
#define LBCTMOs0_RS     2
#define LBCTMOm0_RS     2
#define OCPTMO_RS       2
#define PCIB0TO_RS      2

#define DIVISOR         1000

#if DIVISOR > (1 << 16)
#error "Exceed the Maximum Value of DivFactor"
#endif

/*
 *  ==========================
 *  Platform Register Settings
 *  ==========================
 */

/*
 * CPU
 */
#define IMEM_BASE       0x00C00000
#define IMEM_TOP        0x00C03FFF

#define DMEM_BASE       0x00C04000
#define DMEM_TOP        0x00C05FFF

/*
 * Memory Controller
 */
#define MC_MCR          0xB8001000
   #define MC_MCR_VAL      0x92A28000

#define MC_MTCR0        0xB8001004
   #define MC_MTCR0_VAL    0x12120000

#define MC_MTCR1        0xB8001008
   #define MC_MTCR1_VAL    0x00000FEB

#define MC_PFCR         0xB8001010
   #define MC_PFCR_VAL     0x00000101


#define MC_BASE         0xB8001000
#define NCR             (MC_BASE + 0x100)
#define NSR             (MC_BASE + 0x104)
#define NCAR            (MC_BASE + 0x108)
#define NADDR           (MC_BASE + 0x10C)
#define NDR             (MC_BASE + 0x110)

#define SFCR            (MC_BASE + 0x200)
#define SFDR            (MC_BASE + 0x204)

/*
 * UART
 */
#define UART0_BASE      0xB8002000
#define UART0_RBR       (UART0_BASE + 0x000)
#define UART0_THR       (UART0_BASE + 0x000)
#define UART0_DLL       (UART0_BASE + 0x000)
#define UART0_IER       (UART0_BASE + 0x004)
#define UART0_DLM       (UART0_BASE + 0x004)
#define UART0_IIR       (UART0_BASE + 0x008)
#define UART0_FCR       (UART0_BASE + 0x008)
#define UART0_LCR       (UART0_BASE + 0x00C)
#define UART0_MCR       (UART0_BASE + 0x010)
#define UART0_LSR       (UART0_BASE + 0x014)

#define UART1_BASE      0xB8002100
#define UART1_RBR       (UART1_BASE + 0x000)
#define UART1_THR       (UART1_BASE + 0x000)
#define UART1_DLL       (UART1_BASE + 0x000)
#define UART1_IER       (UART1_BASE + 0x004)
#define UART1_DLM       (UART1_BASE + 0x004)
#define UART1_IIR       (UART1_BASE + 0x008)
#define UART1_FCR       (UART1_BASE + 0x008)
   #define FCR_EN          0x01
   #define FCR_RXRST       0x02
   #define     RXRST             0x02
   #define FCR_TXRST       0x04
   #define     TXRST             0x04
   #define FCR_DMA         0x08
   #define FCR_RTRG        0xC0
   #define     CHAR_TRIGGER_01   0x00
   #define     CHAR_TRIGGER_04   0x40
   #define     CHAR_TRIGGER_08   0x80
   #define     CHAR_TRIGGER_14   0xC0
#define UART1_LCR       (UART1_BASE + 0x00C)
   #define LCR_WLN         0x03
   #define     CHAR_LEN_5        0x00
   #define     CHAR_LEN_6        0x01
   #define     CHAR_LEN_7        0x02
   #define     CHAR_LEN_8        0x03
   #define LCR_STB         0x04
   #define     ONE_STOP          0x00
   #define     TWO_STOP          0x04
   #define LCR_PEN         0x08
   #define     PARITY_ENABLE     0x01
   #define     PARITY_DISABLE    0x00
   #define LCR_EPS         0x30
   #define     PARITY_ODD        0x00
   #define     PARITY_EVEN       0x10
   #define     PARITY_MARK       0x20
   #define     PARITY_SPACE      0x30
   #define LCR_BRK         0x40
   #define LCR_DLAB        0x80
   #define     DLAB              0x80
#define UART1_MCR       (UART1_BASE + 0x010)
#define UART1_LSR       (UART1_BASE + 0x014)
   #define LSR_DR          0x01
   #define     RxCHAR_AVAIL      0x01
   #define LSR_OE          0x02
   #define LSR_PE          0x04
   #define LSR_FE          0x08
   #define LSR_BI          0x10
   #define LSR_THRE        0x20
   #define     TxCHAR_AVAIL      0x00
   #define     TxCHAR_EMPTY      0x20
   #define LSR_TEMT        0x40
   #define LSR_RFE         0x80


/*
 * Interrupt Controller
 */
#define GIMR            0xB8003000
   #define PTM_IE          (1 << 31)
   #define LBCTMOs2_IE     (1 << 30)
   #define LBCTMOs1_IE     (1 << 29)
   #define PKT_IE          (1 << 28)
   #define SPI_IE          (1 << 27)
   #define NIC100_IE       (1 << 26)
   #define SAR_IE          (1 << 25)
   #define DMT_IE          (1 << 24)
   #define PKT_NIC100_IE          (1 << 23)
   #define PKT_NIC100MII_IE          (1 << 22)
   #define GDMA_IE         (1 << 21)
   #define SECURITY_IE     (1 << 20)
   #define PCM_IE          (1 << 19)
   #define PKT_SAR_IE          (1 << 18)
   #define GPIO_EFGH_IE    (1 << 17)
   #define GPIO_ABCD_IE    (1 << 16)
   #define SW_IE           (1 << 15)
   #define PCI_IE          (1 << 14)
   #define UART1_IE        (1 << 13)
   #define UART0_IE        (1 << 12)
   #define USB_D_IE        (1 << 11)
   #define USB_H_IE        (1 << 10)
   #define TC1_IE          (1 << 9)
   #define TC0_IE          (1 << 8)
   #define LBCTMOm2_IE     (1 << 7)
   #define LBCTMOm1_IE     (1 << 6)
   #define SPEED_IE        (1 << 5)
   #define LBCTMOs0_IE     (1 << 4)
   #define LBCTMOm0_IE     (1 << 3)
   #define OCPTMO_IE       (1 << 2)
//   #define PCIB1TO_IE      (1 << 1)
   #define PCIB0TO_IE      (1 << 0)

#define GISR            0xB8003004
   #define PTM_IP          (1 << 31)
   #define LBCTMOs2_IP     (1 << 30)
   #define LBCTMOs1_IP     (1 << 29)
   #define PKT_IP          (1 << 28)
   #define SPI_IP          (1 << 27)
   #define NIC100_IP       (1 << 26)
   #define SAR_IP          (1 << 25)
   #define DMT_IP          (1 << 24)
   #define PKT_NIC100_IP          (1 << 23)
   #define PKT_NIC100MII_IP          (1 << 22)
   #define GDMA_IP         (1 << 21)
   #define SECURITY_IP     (1 << 20)
   #define PCM_IP          (1 << 19)
   #define PKT_SAR_IP          (1 << 18)
   #define GPIO_EFGH_IP    (1 << 17)
   #define GPIO_ABCD_IP    (1 << 16)
   #define SW_IP           (1 << 15)
   #define PCI_IP          (1 << 14)
   #define UART1_IP        (1 << 13)
   #define UART0_IP        (1 << 12)
   #define USB_D_IP        (1 << 11)
   #define USB_H_IP        (1 << 10)
   #define TC1_IP          (1 << 9)
   #define TC0_IP          (1 << 8)
   #define LBCTMOm2_IP     (1 << 7)
   #define LBCTMOm1_IP     (1 << 6)
   #define SPEED_IP        (1 << 5)
   #define LBCTMOs0_IP     (1 << 4)
   #define LBCTMOm0_IP     (1 << 3)
   #define OCPTMO_IP       (1 << 2)
//   #define PCIB1TO_IP      (1 << 1)
   #define PCIB0TO_IP      (1 << 0)

#define IRR0            0xB8003008
#define IRR0_SETTING    ((LBCTMOm2_RS << 28) | \
                         (LBCTMOm1_RS << 24) | \
                         (SPEED_RS    << 20) | \
                         (LBCTMOs0_RS << 16) | \
                         (LBCTMOm0_RS << 12) | \
                         (OCPTMO_RS   << 8)  | \
                         (PCIB0TO_RS  << 0)    \
                        )

#define IRR1            0xB800300C
#define IRR1_SETTING    ((SW_RS    << 28) | \
                         (PCI_RS   << 24) | \
                         (UART1_RS << 20) | \
                         (UART0_RS << 16) | \
                         (USB_D_RS << 12) | \
                         (USB_H_RS << 8)  | \
                         (TC1_RS   << 4)  | \
                         (TC0_RS   << 0)    \
                        )

#define IRR2            0xB8003010
#define IRR2_SETTING    ((PKT_NIC100_RS       << 28) | \
                         (PKT_NIC100MII_RS        << 24) | \
                         (GDMA_RS      << 20) | \
                         (SECURITY_RS  << 16) | \
                         (PCM_RS       << 12) | \
                         (PKT_SAR_RS	<< 8) | \
                         (GPIO_EFGH_RS << 4)  | \
                         (GPIO_ABCD_RS << 0)    \
                        )

#define IRR3            0xB8003014
#define IRR3_SETTING    ((PTM_RS      << 28) | \
                         (LBCTMOs2_RS << 24) | \
                         (LBCTMOs1_RS << 20) | \
                         (PKT_RS      << 16) | \
                         (SPI_RS      << 12) | \
                         (NIC100_RS   << 8)  | \
                         (SAR_RS      << 4)  | \
                         (DMT_RS      << 0)    \
                        )

/*
 * Timer/Counter
 */
#define TC_BASE         0xB8003100
#define TC0DATA         (TC_BASE + 0x00)
#define TC1DATA         (TC_BASE + 0x04)
   #define TCD_OFFSET      8
#define TC0CNT          (TC_BASE + 0x08)
#define TC1CNT          (TC_BASE + 0x0C)
#define TCCNR           (TC_BASE + 0x10)
   #define TC0EN           (1 << 31)
   #define TC0MODE_TIMER   (1 << 30)
   #define TC1EN           (1 << 29)
   #define TC1MODE_TIMER   (1 << 28)
#define TCIR            (TC_BASE + 0x14)
   #define TC0IE           (1 << 31)
   #define TC1IE           (1 << 30)
   #define TC0IP           (1 << 29)
   #define TC1IP           (1 << 28)
#define CDBR            (TC_BASE + 0x18)
   #define DIVF_OFFSET     16
#define WDTCNR          (TC_BASE + 0x1C)
#define WDTE_OFFSET                         24              /* Watchdog enable */
#define WDSTOP_PATTERN                      0xA5            /* Watchdog stop pattern */
#define WDTCLR                              (1 << 23)       /* Watchdog timer clear */
#define OVSEL_13                            0               /* Overflow select count 2^13 */
/*
 * System MISC Control Register
 */
#define MISCCR_BASE	0xB8003300
#define MISC_PINMUX	(MISCCR_BASE+0x00)
#define MISC_PINOCR	(MISCCR_BASE+0x04)
#define MISC_PINSR	(MISCCR_BASE+0x08)


/* 
 *GPIO control registers 
*/
#define GPIOCR_BASE 0xB8003500
/*Port A,B,C,D*/
#define GPIO_PABCD_CNR		(GPIOCR_BASE+0x00)	/*Port A,B,C,D control register*/
#define GPIO_PABCD_PTYPE	(GPIOCR_BASE+0x04)	/*Port A,B,C,D peripheral type control register*/
#define GPIO_PABCD_DIR		(GPIOCR_BASE+0x08)	/*Port A,B,C,D direction */
#define GPIO_PABCD_DAT		(GPIOCR_BASE+0x0C)	/*Port A,B,C,D data register*/
#define GPIO_PABCD_ISR		(GPIOCR_BASE+0x10)	/*Port A,B,C,D interrupt status register*/
#define GPIO_PAB_IMR		(GPIOCR_BASE+0x14)	/*Port A,B interrupt mask register*/
#define GPIO_PCD_IMR		(GPIOCR_BASE+0x18)	/*Port C,D interrupt mask register*/





/*Port A*/
#define GPIO_PADIR	(GPIOCR_BASE+0x00L)	/*Port A direction register*/
#define GPIO_PADAT	(GPIOCR_BASE+0x04L)	/*Port A data register*/
#define GPIO_PAISR	(GPIOCR_BASE+0x08L)	/*Port A interrupt status register*/
#define GPIO_PAIMR	(GPIOCR_BASE+0x0CL)	/*Port A interrupt mask register*/
/*Port B*/
#define GPIO_PBDIR	(GPIOCR_BASE+0x10L)	/*Port B direction register*/
#define GPIO_PBDAT	(GPIOCR_BASE+0x14L)	/*Port B data register*/
#define GPIO_PBISR	(GPIOCR_BASE+0x18L)	/*Port B interrupt status register*/
#define GPIO_PBIMR	(GPIOCR_BASE+0x1CL)	/*Port B interrupt mask register*/


#define RT8670_SICR_BASE	0xB9C04000	//tylo,0xBD800000	/*System interface Configuration Register*/
#define RT8670_SICR_SPI 	0x00000040	/*SPI interface,  1:use SPI, 0:use SPI as GPIOA0~2,B7*/
#define RT8670_SICR_JTAG	0x00000010	/*JTAG interface, 1:use JTAG, 0:use JTAG as GPIOA3~7*/
#define RT8670_SICR_UART	0x00000008	/*UART interface, 1:use UART, 0:use UART as GPIOB3~6*/

/* ADSL State */
#define C_AMSW_IDLE                0
#define C_AMSW_L3                  1
#define C_AMSW_ACTIVATING          3
#define C_AMSW_INITIALIZING        6
#define C_AMSW_SHOWTIME_L0         9
#define C_AMSW_END_OF_LD     	15

#endif /* _PLATFORM_H */
