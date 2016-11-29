#include "spi_flash.h"
#if 0
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kmod.h>
//#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>

#include <linux/mtd/map.h>
#include <linux/mtd/gen_probe.h>

#include <asm/io.h>
#endif
#include <linux/mtd/map.h>
#include <linux/mtd/gen_probe.h>
#include <linux/spinlock.h>
//#include <linux/wait.h>
//tylo, for ic ver. detect
#define SCCR	0xb8003200
unsigned char ICver=0;
#define IC8672 	0
#define IC8671B 	1
#define IC8671B_costdown 	2
/* SPI Flash Controller */
unsigned int SFCR=0;
unsigned int SFCSR=0;
unsigned int SFDR=0;

#define LENGTH(i)       SPI_LENGTH(i)
#define CS(i)           SPI_CS(i)
#define RD_ORDER(i)     SPI_RD_ORDER(i)
#define WR_ORDER(i)     SPI_WR_ORDER(i)
#define READY(i)        SPI_READY(i)
#define CLK_DIV(i)      SPI_CLK_DIV(i)
#define RD_MODE(i)      SPI_RD_MODE(i)
#define SFSIZE(i)       SPI_SFSIZE(i)
#define TCS(i)          SPI_TCS(i)
#define RD_OPT(i)       SPI_RD_OPT(i)

/*
 * SPI Flash Info
 */
const struct spi_flash_db   spi_flash_known[] =
{
   {0x01, 0x02,   1}, /* Spansion */
   {0xC2, 0x20,   0}, /* MXIC */
   //ql 20090203 add below flash type
   {0x1C, 0x31,  0}, /* EON */
   {0x8C, 0x20,  0}, /* F25L016A */
   {0xEF, 0x30,  0}, /* W25X16 */
   {0x1F, 0x46,  0}, /* AT26DF161 */
   {0xBF, 0x25,  0}, /* 25VF016B-50-4c-s2AF */
};

//#define SPI_DEBUG

/*
 * SPI Flash Info
 */
struct spi_flash_type   spi_flash_info[2];


/*
 * SPI Flash APIs
 */

/*
 * This function shall be called when switching from MMIO to PIO mode
 */
// #define __TESSPIIRAM		__attribute__ ((__section__ (".iram")))
//__TESSPIIRAM 
void spi_pio_init(void)
{
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(0) | READY(1);

   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(0) | READY(1);

   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
}
void spi_pio_init_ready(void)
{
   spi_ready();
}
void spi_pio_toggle1(void)
{
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
}
void spi_pio_toggle2(void)
{
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
}
void spi_read(unsigned int chip, unsigned int address, unsigned int *data_out)
{
   /* De-Select Chip */
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

   /* RDSR Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

   *(volatile unsigned int *) SFDR = 0x05 << 24;

   while (1)
   {
      unsigned int status;

      status = *(volatile unsigned int *) SFDR;

      /* RDSR Command */
      if ( (status & 0x01000000) == 0x00000000)
      {
         break;
      }
   }

   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* READ Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);

   *(volatile unsigned int *) SFDR = (0x03 << 24) | (address & 0xFFFFFF);

   /* Read Data Out */
   *data_out = *(volatile unsigned int *) SFDR;

   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
}

#if 0
__TESSPIIRAM void spi_write(unsigned int chip, unsigned int address, unsigned int data_in)
{
   /* De-select Chip */
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* RDSR Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
   *(volatile unsigned int *) SFDR = 0x05 << 24;

   while (1)
   {
      unsigned int status;

      status = *(volatile unsigned int *) SFDR;

      /* RDSR Command */
      if ( (status & 0x01000000) == 0x00000000)
      {
         break;
      }
   }

   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* WREN Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
   *(volatile unsigned int *) SFDR = 0x06 << 24;

   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* PP Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
   *(volatile unsigned int *) SFDR = (0x02 << 24) | (address & 0xFFFFFF);
   *(volatile unsigned int *) SFDR = data_in;
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
}
#endif

 void spi_erase_chip(unsigned int chip)
{
   /* De-select Chip */
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* RDSR Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
   *(volatile unsigned int *) SFDR = 0x05 << 24;

   while (1)
   {
      /* RDSR Command */
      if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
      {
         break;
      }
   }

   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* WREN Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
   *(volatile unsigned int *) SFDR = 0x06 << 24;
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* BE Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
   *(volatile unsigned int *) SFDR = (0xC7 << 24);
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
}

 void spi_ready(void)
{
   while (1)
   {
      if ( (*(volatile unsigned int *) SFCSR) & READY(1))
         break;
   }
}

#if 0
 void spi_erase_sector(int sector){
	int chip=0;
	
      /* WREN Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

      *(volatile unsigned int *) SFDR = 0x06 << 24;
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

      /* SE Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = (0xD8 << 24) | (sector * 65536);
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

      /* RDSR Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = 0x05 << 24;

      while (1)
      {
         /* RDSR Command */
         if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
         {
            break;
         }
      }

      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
}
#endif
void spi_cp_probe(void)
{
   unsigned int cnt, i;
   unsigned int temp;

   for (cnt = 0; cnt < 2; cnt++)
   {
      /* Here set the default setting */
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
      //ql 20090203 for some flash, CS# should be hold high for a moment, so I repeat above statement.
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);

      //ql 20090203 it is not necessary to do double toggle
      /* One More Toggle (May not Necessary) */
      //*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
      //*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);

      /* RDID Command */
      *(volatile unsigned int *) SFDR = 0x9F << 24;
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+cnt) | READY(1);
      temp = *(volatile unsigned int *) SFDR;

      spi_flash_info[cnt].maker_id = (temp >> 24) & 0xFF;
      spi_flash_info[cnt].type_id = (temp >> 16) & 0xFF;
      spi_flash_info[cnt].capacity_id = (temp >> 8) & 0xFF;
      spi_ready();

      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

      //ql 20090203 for AT26DF161 and 25VF016B, its capacity_id is meaningless, I modify its value now.
      if (((spi_flash_info[cnt].type_id == 0x46) && (spi_flash_info[cnt].maker_id == 0x1F))/*AT26DF161*/
	  ||((spi_flash_info[cnt].type_id == 0x25) && (spi_flash_info[cnt].maker_id == 0xBF)))
		spi_flash_info[cnt].capacity_id = 21;
	  
      /* Iterate Each Maker ID/Type ID Pair */
      for (i = 0; i < sizeof(spi_flash_known) / sizeof(struct spi_flash_db); i++)
      {
         if ( (spi_flash_info[cnt].maker_id == spi_flash_known[i].maker_id) &&
              (spi_flash_info[cnt].type_id == spi_flash_known[i].type_id) )
         {
            spi_flash_info[cnt].device_size = (unsigned char)((signed char)spi_flash_info[cnt].capacity_id + spi_flash_known[i].size_shift);
         }
      }

      spi_flash_info[cnt].sector_cnt = 1 << (spi_flash_info[cnt].device_size - 16);
   }
   for(i=0;i<2;i++){
	//printk("get SPI CS%d\n\r",i);
	//printk("maker:%x  type:%x  sector_cnt:%d\n",spi_flash_info[i].maker_id,spi_flash_info[i].type_id,spi_flash_info[i].sector_cnt);
   }
}

#if 0
 void spi_burn_image(unsigned int chip, unsigned char *image_addr, unsigned int image_size)
{
   unsigned int temp;
   unsigned int i, j, k;
   unsigned char *cur_addr;
   unsigned int cur_size;
   unsigned int cnt;

   cur_addr = image_addr;
   cur_size = image_size;

   /* Iterate Each Sector */
   for (i = 0; i < spi_flash_info[chip].sector_cnt; i++)
   {
      //unsigned int spi_data;

      /* WREN Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

      *(volatile unsigned int *) SFDR = 0x06 << 24;
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

      /* SE Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = (0xD8 << 24) | (i * 65536);
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

      /* RDSR Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = 0x05 << 24;

      while (1)
      {
         /* RDSR Command */
         if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
         {
            break;
         }
      }

      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

#if 1
      printk("Erase Sector: %d\n", i);
#endif

      /* Iterate Each Page */
      for (j = 0; j < 256; j++)
      {
         if (cur_size == 0)
            break;

         /* WREN Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
         *(volatile unsigned int *) SFDR = 0x06 << 24;
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

         /* PP Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
         *(volatile unsigned int *) SFDR = (0x02 << 24) | (i * 65536) | (j * 256);

         for (k = 0; k < 64; k++)
         {
            temp = (*(cur_addr)) << 24 | (*(cur_addr + 1)) << 16 | (*(cur_addr + 2)) << 8 | (*(cur_addr + 3));

            spi_ready();
            if (cur_size >= 4)
            {
               *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
               cur_size -= 4;
            }
            else
            {
               *(volatile unsigned int *) SFCSR = LENGTH(cur_size-1) | CS(1+chip) | READY(1);
               cur_size = 0;
            }

            *(volatile unsigned int *) SFDR = temp;

            cur_addr += 4;

            if (cur_size == 0)
               break;
         }

         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

         /* RDSR Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
         *(volatile unsigned int *) SFDR = 0x05 << 24;

         cnt = 0;
         while (1)
         {
            unsigned int status = *(volatile unsigned int *) SFDR;

            /* RDSR Command */
            if ((status & 0x01000000) == 0x00000000)
            {
                break;
            }

            if (cnt > 2000)
            {
               printk("\nBusy Loop for RSDR: %d, Address at 0x%08X\n", status, i*65536+j*256);
busy:
               goto busy;
            }
            cnt++;
         }

         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

         /* Verify Burned Image */
         /* READ Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
         *(volatile unsigned int *) SFDR = (0x03 << 24) | (i * 65536) | (j * 256);

         for (k = 0; k < 64; k++)
         {
            unsigned int data;

            temp = (*(cur_addr -256 + (k<<2) )) << 24 | (*(cur_addr -256 + (k<<2) + 1)) << 16 | (*(cur_addr - 256 + (k<<2) + 2)) << 8 | (*(cur_addr - 256 + (k<<2) + 3));

            data = *(volatile unsigned int *) SFDR;

            if ((data != temp))
            {
               printk("\nVerify Error at 0x%08X: Now 0x%08X, Expect 0x%08X",
                      i*65536+j*256+(k<<2), data, temp);
halt_here:
               goto halt_here;
            }
         }

         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
      }

      if (cur_size == 0)
         break;
   } /* Iterate Each Sector */
}
#endif
//new SPI driver


//#define MTD_SPI_SUZAKU_DEBUG

#if defined(MTD_SPI_SUZAKU_DEBUG)
#define KDEBUG(args...) printk(args)
#else
#define KDEBUG(args...)
#endif

#define write32(a, v)       __raw_writel(v, a)
#define read32(a)           __raw_readl(a)

/* Proto-type declarations */
static u8 spi_read_status(void);
static void spi_set_cs(u32);

#define SPI_ERASING 1

static int spi_state = 0;
static spinlock_t spi_mutex = SPIN_LOCK_UNLOCKED;
//static wait_queue_head_t spi_wq;

#ifdef CONFIG_RTK_VOIP
spinlock_t spi_lock = SPIN_LOCK_UNLOCKED;
#endif


/**
 * select which cs (chip select) line to activate
 */
inline static void spi_set_cs(u32 cs)
{
	;//write32(REG_SPISSR, !cs);
}


 static u32 spi_copy_to_dram(const u32 from, const u32 to, const u32 size)
{
	memcpy(to,from|0xbfc00000,size);
	
	return 0;
}




static u32 do_spi_read(u32 from, u32 to, u32 size)
{
	//DECLARE_WAITQUEUE(wait, current);
	//unsigned long timeo;
	u32 ret;
#ifdef CONFIG_RTK_VOIP
	int flags;
	spin_lock_irqsave(spi_lock,flags);
#endif
	//ql commented: if size larger than 1024, then how to get the rest???
	if (from>0x10000)
	    size=(size<=1024)?size:1024;
	else
	    size=(size<=4096)?size:4096;
	ret = spi_copy_to_dram(from, to, size);
	spi_pio_init();
#ifdef CONFIG_RTK_VOIP
	spin_unlock_irqrestore(spi_lock,flags);
#endif
	return ret;
}


static u32 do_spi_write(u32 from, u32 to, u32 size)
{
   unsigned int temp;
	unsigned int  remain;
   unsigned int cur_addr;
   unsigned int cur_size ,flash_addr;
   unsigned int cnt;
   unsigned int next_page_addr;
 
#ifdef CONFIG_RTK_VOIP
	int flags;
	spin_lock_irqsave(spi_lock,flags);
#endif
   cur_addr = from;
   flash_addr = to;
   cur_size = size;

#ifdef SPI_DEBUG
	printk("\r\n do_spi_write : from :[%x] to:[%x], size:[%x]  ", from, to, size);
#endif

   	   spi_pio_init();
      next_page_addr = ((flash_addr >> 8) +1) << 8;

      while (cur_size > 0)
      {
	   /* De-select Chip */
	   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	  
         /* WREN Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
         *(volatile unsigned int *) SFDR = 0x06 << 24;
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
         /* PP Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
         *(volatile unsigned int *) SFDR = (0x02 << 24) | (flash_addr & 0xFFFFFF);

	   while (flash_addr != next_page_addr)
	   {
		remain = (cur_size > 4)?4:cur_size;		
		temp = *((int*)cur_addr);
		
            spi_ready();
			
            *(volatile unsigned int *) SFCSR = LENGTH(remain-1) | CS(1) | READY(1);                     
            *(volatile unsigned int *) SFDR = temp;
		
		cur_size -= remain;
		cur_addr += remain;
		flash_addr+=remain;
		
            if (cur_size == 0)
               break;;
	   }
		next_page_addr = flash_addr + 256;
         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

         /* RDSR Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
         *(volatile unsigned int *) SFDR = 0x05 << 24;

         cnt = 0;
         while (1)
         {
            unsigned int status = *(volatile unsigned int *) SFDR;

            /* RDSR Command */
            if ((status & 0x01000000) == 0x00000000)
            {
                break;
            }

            if (cnt > 200000)
            {
#ifdef CONFIG_RTK_VOIP
	spin_unlock_irqrestore(spi_lock,flags);
#endif

            		return -EINVAL;
            }
            cnt++;
         }

         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
      }
#ifdef CONFIG_RTK_VOIP
	spin_unlock_irqrestore(spi_lock,flags);
#endif

	return 0;
}




/*Notice !!!
 * To comply current design, the erase function will implement sector erase
*/
static int do_spi_erase(u32 addr)
{
	int chip=0;

#ifdef SPI_DEBUG
	printk("\r\n do_spi_erase : [%x] ", addr);
#endif
#ifdef CONFIG_RTK_VOIP
	int flags;
	spin_lock_irqsave(spi_lock,flags);
#endif
	spi_pio_init();
	
      /* WREN Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

      *(volatile unsigned int *) SFDR = 0x06 << 24;
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

      /* SE Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = (0x20 << 24) | addr;
//	  *(volatile unsigned int *) SFDR = (0xD8 << 24) | addr;
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

      /* RDSR Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = 0x05 << 24;

      while (1)
      {
         /* RDSR Command */
         if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
         {
            break;
         }
      }

      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
#ifdef CONFIG_RTK_VOIP
	spin_unlock_irqrestore(spi_lock,flags);
#endif

	return 0;
}
/*
 The Block Erase function
*/
static int do_spi_block_erase(u32 addr)
{
	int chip=0;

#ifdef SPI_DEBUG
	printk("\r\n do_spi_block_erase : [%x] ", addr);
#endif
#ifdef CONFIG_RTK_VOIP
	int flags;
	spin_lock_irqsave(spi_lock,flags);
#endif
	spi_pio_init();

	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	
      /* WREN Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

      *(volatile unsigned int *) SFDR = 0x06 << 24;
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

      /* SE Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = (0xD8 << 24) | addr;
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

      /* RDSR Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = 0x05 << 24;

      while (1)
      {
         /* RDSR Command */
         if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
         {
            break;
         }
      }

      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

#ifdef CONFIG_RTK_VOIP
	spin_unlock_irqrestore(spi_lock,flags);
#endif

	return 0;
}

static int do_spi_block_erase_spansion(u32 addr)
{
	int chip=0;

#ifdef SPI_DEBUG
	printk("\r\n do_spi_block_erase : [%x] ", addr);
#endif
#ifdef CONFIG_RTK_VOIP
	int flags;
	spin_lock_irqsave(spi_lock,flags);
#endif
	spi_pio_init();

	/* De-select Chip */
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	/* RDSR Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
	*(volatile unsigned int *) SFDR = 0x05 << 24;

	while (1)
	{
		unsigned int status;
		status = *(volatile unsigned int *) SFDR;

		/* RDSR Command */
		if ( (status & 0x01000000) == 0x00000000)
		{
			//ql: if block protected, then write status register 0
			if (status & (0x1C<<24))
			{
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				/*WREN Command*/
				spi_ready();
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
				*(volatile unsigned int *) SFDR = 0x06 << 24;
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				spi_ready();
				//WRSR command
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
				*(volatile unsigned int *) SFDR = (0x01 << 24);
				*(volatile unsigned int *) SFDR = 0;
			}

			break;
		}
	}

	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	/* WREN Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

	*(volatile unsigned int *) SFDR = 0x06 << 24;
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	/* SE Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
	*(volatile unsigned int *) SFDR = (0xD8 << 24) | addr;
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

	/* RDSR Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
	*(volatile unsigned int *) SFDR = 0x05 << 24;

	while (1)
	{
		/* RDSR Command */
		if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
		{
			break;
		}
	}

	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

#ifdef CONFIG_RTK_VOIP
	spin_unlock_irqrestore(spi_lock,flags);
#endif

	return 0;
}

static int do_spi_block_erase_atmel(u32 addr)
{
	int chip=0;

#ifdef SPI_DEBUG
	printk("\r\n do_spi_block_erase : [%x] ", addr);
#endif
#ifdef CONFIG_RTK_VOIP
	int flags;
	spin_lock_irqsave(spi_lock,flags);
#endif
	spi_pio_init();

	/* De-select Chip */
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	/* RDSR Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
	*(volatile unsigned int *) SFDR = 0x05 << 24;

	while (1)
	{
		unsigned int status;
		status = *(volatile unsigned int *) SFDR;

		/* RDSR Command */
		if ( (status & 0x01000000) == 0x00000000)
		{
			//ql: if block protected, then write status register 0
			if (status & (0x0C<<24))
			{
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				/*WREN Command*/
				spi_ready();
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
				*(volatile unsigned int *) SFDR = 0x06 << 24;
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				spi_ready();
				//WRSR command
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
				*(volatile unsigned int *) SFDR = (0x01 << 24);
				*(volatile unsigned int *) SFDR = 0;
			}

			break;
		}
	}

	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	/* WREN Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

	*(volatile unsigned int *) SFDR = 0x06 << 24;
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	/* SE Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
	*(volatile unsigned int *) SFDR = (0xD8 << 24) | addr;
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

	/* RDSR Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
	*(volatile unsigned int *) SFDR = 0x05 << 24;

	while (1)
	{
		/* RDSR Command */
		if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
		{
			break;
		}
	}

	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

#ifdef CONFIG_RTK_VOIP
	spin_unlock_irqrestore(spi_lock,flags);
#endif

	return 0;
}

static int do_spi_block_erase_other(u32 addr)
{
	int chip=0;

#ifdef SPI_DEBUG
	printk("\r\n do_spi_block_erase : [%x] ", addr);
#endif
#ifdef CONFIG_RTK_VOIP
	int flags;
	spin_lock_irqsave(spi_lock,flags);
#endif
	spi_pio_init();

	/* De-select Chip */
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	/* RDSR Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
	*(volatile unsigned int *) SFDR = 0x05 << 24;

	while (1)
	{
		unsigned int status;
		status = *(volatile unsigned int *) SFDR;

		/* RDSR Command */
		if ( (status & 0x01000000) == 0x00000000)
		{
			//ql: if block protected, then write status register 0
			if (status & (0x1C<<24))
			{
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				/* WREN Command */
				spi_ready();
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
				*(volatile unsigned int *) SFDR = 0x06 << 24;
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				/*EWSR Command*/
				spi_ready();
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
				*(volatile unsigned int *) SFDR = 0x50 << 24;
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				spi_ready();
				//WRSR command
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
				*(volatile unsigned int *) SFDR = (0x01 << 24);
				*(volatile unsigned int *) SFDR = 0;
			}

			break;
		}
	}

	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	
      /* WREN Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

      *(volatile unsigned int *) SFDR = 0x06 << 24;
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

      /* SE Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = (0xD8 << 24) | addr;
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

      /* RDSR Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = 0x05 << 24;

      while (1)
      {
         /* RDSR Command */
         if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
         {
            break;
         }
      }

      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

#ifdef CONFIG_RTK_VOIP
	spin_unlock_irqrestore(spi_lock,flags);
#endif

	return 0;
}

static u32 do_spi_block_write(u32 from, u32 to, u32 size)
{
	unsigned char *ptr;

	//don't support write through 1st block
	if ((to < SIZE_64KiB) && ((to+size) > SIZE_64KiB))
		return -EINVAL;
	if (to < SIZE_64KiB)
	{
		ptr = kmalloc(SIZE_64KiB,GFP_KERNEL );
		if (!ptr)
			return -EINVAL;
		memcpy(ptr,0xbfc00000, SIZE_64KiB);
		do_spi_block_erase(0); // erase 1 sector
		memcpy(ptr+to,from , size);
		do_spi_write(ptr, 0 , SIZE_64KiB);
		kfree(ptr);
		return  0 ;
	}
	else 
		return do_spi_write(from , to, size);
}

static u32 spi_write_oneByte(unsigned int address, unsigned char data_in)
{
#ifdef CONFIG_RTK_VOIP
   int flags;
   spin_lock_irqsave(spi_lock,flags);
#endif
   spi_pio_init();

   /* De-select Chip */
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* RDSR Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
   *(volatile unsigned int *) SFDR = 0x05 << 24;

   while (1)
   {
      unsigned int status;
      status = *(volatile unsigned int *) SFDR;

      /* RDSR Command */
      if ( (status & 0x01000000) == 0x00000000)
      {
         //ql: if block protected, then write status register 0
	 if (status & (0x1C<<24))
	 {
	      	 *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
		 
		 /* WREN Command */
		 spi_ready();
		 *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
		 *(volatile unsigned int *) SFDR = 0x06 << 24;
		 *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
		 /*EWSR Command*/
		 spi_ready();
		 *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
		 *(volatile unsigned int *) SFDR = 0x50 << 24;
		 *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

		 spi_ready();
		 //WRSR command
		 *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
		 *(volatile unsigned int *) SFDR = (0x01 << 24);
		 *(volatile unsigned int *) SFDR = 0;
	  }
	  
          break;
      }
   }

   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* WREN Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
   *(volatile unsigned int *) SFDR = 0x06 << 24;

   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* PP Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
   *(volatile unsigned int *) SFDR = (0x02 << 24) | (address & 0xFFFFFF);
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
   *(volatile unsigned int *) SFDR = (data_in<<24) | 0xFFFFFF;
   
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
   
   /* RDSR Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
   *(volatile unsigned int *) SFDR = 0x05 << 24;

   while (1)
   {
      unsigned int status;

      status = *(volatile unsigned int *) SFDR;

      /* RDSR Command */
      if ( (status & 0x01000000) == 0x00000000)
      {
         break;
      }
   }
    *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

#ifdef CONFIG_RTK_VOIP
   spin_unlock_irqrestore(spi_lock,flags);
#endif

   return 0;
}

static u32 do_spi_write_sst(u32 from, u32 to, u32 size)
{
	unsigned int idx;
	unsigned char *ptr=(unsigned char *)from;

	for (idx=0; idx<size; idx++)
		spi_write_oneByte(to++, ptr[idx]);
}

static u32 do_spi_write_esmt(u32 from, u32 to, u32 size)
{
	unsigned int idx;
	unsigned char *ptr=(unsigned char *)from;

	for (idx=0; idx<size; idx++)
		spi_write_oneByte(to++, ptr[idx]);
}

static u32 do_spi_block_write_sst(u32 from, u32 to, u32 size)
{
	unsigned char *ptr;

	//don't support write through 1st block
	if ((to < SIZE_64KiB) && ((to+size) > SIZE_64KiB))
		return -EINVAL;
	
	spi_pio_init();

	/* De-select Chip */
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	/* RDSR Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
	*(volatile unsigned int *) SFDR = 0x05 << 24;

	while (1)
	{
		unsigned int status;
		status = *(volatile unsigned int *) SFDR;

		/* RDSR Command */
		if ( (status & 0x01000000) == 0x00000000)
		{
			//ql: if block protected, then write status register 0
			if (status & (0x1C<<24))
			{
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				/* WREN Command */
				spi_ready();
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
				*(volatile unsigned int *) SFDR = 0x06 << 24;
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				/*EWSR Command*/
				spi_ready();
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
				*(volatile unsigned int *) SFDR = 0x50 << 24;
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				spi_ready();
				//WRSR command
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
				*(volatile unsigned int *) SFDR = (0x01 << 24);
				*(volatile unsigned int *) SFDR = 0;
			}

			break;
		}
	}
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	if (to < SIZE_64KiB)
	{
		ptr = kmalloc(SIZE_64KiB,GFP_KERNEL );
		if (!ptr)
			return -EINVAL;
		memcpy(ptr,0xbfc00000, SIZE_64KiB);
		do_spi_block_erase(0); // erase 1 sector
		memcpy(ptr+to,from , size);
		do_spi_write_sst(ptr, 0 , SIZE_64KiB);
		kfree(ptr);
		return  0 ;
	}
	else 
		return do_spi_write_sst(from , to, size);
}

static u32 do_spi_block_write_esmt(u32 from, u32 to, u32 size)
{
	unsigned char *ptr;

	//don't support write through 1st block
	if ((to < SIZE_64KiB) && ((to+size) > SIZE_64KiB))
		return -EINVAL;
	
	spi_pio_init();

	/* De-select Chip */
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	/* RDSR Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
	*(volatile unsigned int *) SFDR = 0x05 << 24;

	while (1)
	{
		unsigned int status;
		status = *(volatile unsigned int *) SFDR;

		/* RDSR Command */
		if ( (status & 0x01000000) == 0x00000000)
		{
			//ql: if block protected, then write status register 0
			if (status & (0x1C<<24))
			{
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				/* WREN Command */
				spi_ready();
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
				*(volatile unsigned int *) SFDR = 0x06 << 24;
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				/*EWSR Command*/
				spi_ready();
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
				*(volatile unsigned int *) SFDR = 0x50 << 24;
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				spi_ready();
				//WRSR command
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
				*(volatile unsigned int *) SFDR = (0x01 << 24);
				*(volatile unsigned int *) SFDR = 0;
			}

			break;
		}
	}
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	if (to < SIZE_64KiB)
	{
		ptr = kmalloc(SIZE_64KiB,GFP_KERNEL );
		if (!ptr)
			return -EINVAL;
		memcpy(ptr,0xbfc00000, SIZE_64KiB);
		do_spi_block_erase(0); // erase 1 sector
		memcpy(ptr+to,from , size);
		do_spi_write_esmt(ptr, 0 , SIZE_64KiB);
		kfree(ptr);
		return  0 ;
	}
	else 
		return do_spi_write_esmt(from , to, size);
}

static u32 do_spi_block_write_other(u32 from, u32 to, u32 size)
{
	unsigned char *ptr;

	//don't support write through 1st block
	if ((to < SIZE_64KiB) && ((to+size) > SIZE_64KiB))
		return -EINVAL;
	
	spi_pio_init();

	/* De-select Chip */
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	/* RDSR Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
	*(volatile unsigned int *) SFDR = 0x05 << 24;

	while (1)
	{
		unsigned int status;
		status = *(volatile unsigned int *) SFDR;

		/* RDSR Command */
		if ( (status & 0x01000000) == 0x00000000)
		{
			//ql: if block protected, then write status register 0
			if (status & (0x0C<<24))
			{
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				/* WREN Command */
				spi_ready();
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
				*(volatile unsigned int *) SFDR = 0x06 << 24;
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				/*EWSR Command*/
				spi_ready();
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
				*(volatile unsigned int *) SFDR = 0x50 << 24;
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				spi_ready();
				//WRSR command
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
				*(volatile unsigned int *) SFDR = (0x01 << 24);
				*(volatile unsigned int *) SFDR = 0;
			}

			break;
		}
	}
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	
	if (to < SIZE_64KiB)
	{
		ptr = kmalloc(SIZE_64KiB,GFP_KERNEL );
		if (!ptr)
			return -EINVAL;
		memcpy(ptr,0xbfc00000, SIZE_64KiB);
		do_spi_block_erase(0); // erase 1 sector
		memcpy(ptr+to,from , size);
		do_spi_write(ptr, 0 , SIZE_64KiB);
		kfree(ptr);
		return  0 ;
	}
	else 
		return do_spi_write(from , to, size);
}

static u32 do_spi_block_write_atmel(u32 from, u32 to, u32 size)
{
	unsigned char *ptr;

	//don't support write through 1st block
	if ((to < SIZE_64KiB) && ((to+size) > SIZE_64KiB))
		return -EINVAL;
	
	spi_pio_init();

	/* De-select Chip */
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	/* RDSR Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
	*(volatile unsigned int *) SFDR = 0x05 << 24;

	while (1)
	{
		unsigned int status;
		status = *(volatile unsigned int *) SFDR;

		/* RDSR Command */
		if ( (status & 0x01000000) == 0x00000000)
		{
			//ql: if block protected, then write status register 0
			if (status & (0x0C<<24))
			{
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				/*WREN Command*/
				spi_ready();
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
				*(volatile unsigned int *) SFDR = 0x06 << 24;
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				spi_ready();
				//WRSR command
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
				*(volatile unsigned int *) SFDR = (0x01 << 24);
				*(volatile unsigned int *) SFDR = 0;
			}

			break;
		}
	}
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	
	if (to < SIZE_64KiB)
	{
		ptr = kmalloc(SIZE_64KiB,GFP_KERNEL );
		if (!ptr)
			return -EINVAL;
		memcpy(ptr,0xbfc00000, SIZE_64KiB);
		do_spi_block_erase(0); // erase 1 sector
		memcpy(ptr+to,from , size);
		do_spi_write(ptr, 0 , SIZE_64KiB);
		kfree(ptr);
		return  0 ;
	}
	else 
		return do_spi_write(from , to, size);
}

static u32 do_spi_block_write_spansion(u32 from, u32 to, u32 size)
{
	unsigned char *ptr;

	//don't support write through 1st block
	if ((to < SIZE_64KiB) && ((to+size) > SIZE_64KiB))
		return -EINVAL;
	
	spi_pio_init();

	/* De-select Chip */
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	/* RDSR Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
	*(volatile unsigned int *) SFDR = 0x05 << 24;

	while (1)
	{
		unsigned int status;
		status = *(volatile unsigned int *) SFDR;

		/* RDSR Command */
		if ( (status & 0x01000000) == 0x00000000)
		{
			//ql: if block protected, then write status register 0
			if (status & (0x1C<<24))
			{
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				/*WREN Command*/
				spi_ready();
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
				*(volatile unsigned int *) SFDR = 0x06 << 24;
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				spi_ready();
				//WRSR command
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
				*(volatile unsigned int *) SFDR = (0x01 << 24);
				*(volatile unsigned int *) SFDR = 0;
			}

			break;
		}
	}
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	
	if (to < SIZE_64KiB)
	{
		ptr = kmalloc(SIZE_64KiB,GFP_KERNEL );
		if (!ptr)
			return -EINVAL;
		memcpy(ptr,0xbfc00000, SIZE_64KiB);
		do_spi_block_erase(0); // erase 1 sector
		memcpy(ptr+to,from , size);
		do_spi_write(ptr, 0 , SIZE_64KiB);
		kfree(ptr);
		return  0 ;
	}
	else 
		return do_spi_write(from , to, size);
}

//type of SPI flash we support
static const struct spi_flash_info flash_tables[] = {

	{
		mfr_id: SPANSION,
		dev_id: SPI,
		name: "spansion",
		DeviceSize: SIZE_2MiB,
		EraseSize: SIZE_64KiB,
	},
// Support for MX2fL series flash 
	{
		mfr_id: 0xC2,
		dev_id: 0x20,
		name: "mxic",
		DeviceSize: 0x200000,
		//EraseSize: 4096,
		EraseSize: SIZE_64KiB,
	},
	//ql 20090203 add below flash
	{
		mfr_id: 0x1C,
		dev_id: 0x31,
		name: "eon",
		DeviceSize: SIZE_2MiB,
		EraseSize: SIZE_64KiB,
	},
	{
		mfr_id: 0x8C,
		dev_id: 0x20,
		name: "esmt",
		DeviceSize: SIZE_2MiB,
		EraseSize: SIZE_64KiB,
	},
	{
		mfr_id: 0xEF,
		dev_id: 0x30,
		name: "winbond",
		DeviceSize: SIZE_2MiB,
		EraseSize: SIZE_64KiB,
	},
	{
		mfr_id: 0x1F,
		dev_id: 0x46,
		name: "atmel",
		DeviceSize: SIZE_2MiB,
		EraseSize: SIZE_64KiB,
	},
	{
		mfr_id: 0xBF,
		dev_id: 0x25,
		name: "sst",
		DeviceSize: SIZE_2MiB,
		EraseSize: SIZE_64KiB,
	},
};
static struct spi_chip_info *spi_suzaku_setup(struct map_info *map)
{
	struct spi_chip_info *chip_info;

	chip_info = kmalloc(sizeof(*chip_info), GFP_KERNEL);
	if (!chip_info) {
		printk(KERN_WARNING "Failed to allocate memory for MTD device\n");
		return NULL;
	}

	memset(chip_info, 0, sizeof(struct spi_chip_info));

	return chip_info;
}
static void spi_suzaku_destroy(struct spi_chip_info *chip_info)
{
	printk("spi destroy!\n");
}

//tylo, for 8671b, test IC version
void checkICverSPI(void){
	unsigned int sccr;
	
	sccr=*(volatile unsigned int*)SCCR;
	if((sccr & 0x00100000) == 0){
		ICver = IC8671B_costdown;
	}
	else
		ICver = IC8671B;

	//set SPI related register
	if(ICver == IC8671B){
		SFCR = 0xB8001200;
		SFCSR= 0xB8001204;
		SFDR = 0xB8001208;
	}
	else if(ICver == IC8671B_costdown){ // 8671B costdown
		SFCR = 0xB8001200;
		SFCSR = 0xB8001208;
		SFDR = 0xB800120C;
	}
	else{
		printk("can not detect spi controller!");
	}

}

struct spi_chip_info *spi_probe_flash_chip(struct map_info *map, struct chip_probe *cp)
{
	int i;
	struct spi_chip_info *chip_info = NULL;
	
#ifdef CONFIG_RTK_VOIP
	int flags;
	spin_lock_irqsave(spi_lock,flags);
#endif
	checkICverSPI();
	spi_pio_init();
	//*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR |SPI_CLK_DIV(2);//
	*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR & 0x1fffffff;
	*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR |SPI_CLK_DIV(1);
	*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR  &(~(1<<26));
	spi_cp_probe();
	for (i=0; i < (sizeof(flash_tables)/sizeof(struct spi_flash_info)); i++) {
		//printk("ID:%x  device: %x\n",spi_flash_info[0].maker_id ,spi_flash_info[0].type_id );
		if ( (spi_flash_info[0].maker_id == spi_flash_known[i].maker_id) &&
              		(spi_flash_info[0].type_id == spi_flash_known[i].type_id) ) {
			chip_info = spi_suzaku_setup(map);
			if (chip_info) {
				chip_info->flash      = &flash_tables[i];
				if (spi_flash_info[0].maker_id == 0xC2){
					printk("\r\nMXIC matched!!");
					chip_info->flash->DeviceSize = 1 << spi_flash_info[0].capacity_id;
				}
				
				chip_info->destroy    = spi_suzaku_destroy;

				chip_info->read       = do_spi_read;
				
				if (flash_tables[i].EraseSize == 4096) //sector or block erase
				{
					chip_info->erase      = do_spi_erase;
					chip_info->write      = do_spi_write;
				}
				else
				{
					//chip_info->erase      = do_spi_block_erase;
					//chip_info->write      = do_spi_block_write;
					switch(flash_tables[i].mfr_id)
					{
					case 0x1F://atmel
						chip_info->erase = do_spi_block_erase_atmel;
						break;
					case 0x01://spansion
						chip_info->erase = do_spi_block_erase_spansion;
						break;
					default:
						chip_info->erase = do_spi_block_erase_other;
						break;
					}
					
					switch(flash_tables[i].mfr_id)
					{
					case 0x1F://atmel
						chip_info->write = do_spi_block_write_atmel;
						break;
					case 0x01://spansion
						chip_info->write = do_spi_block_write_spansion;
						break;
					case 0xBF://sst
						chip_info->write = do_spi_block_write_sst;
						break;
					case 0x8C://esmt
						chip_info->write = do_spi_block_write_esmt;
						break;
					default:
						chip_info->write = do_spi_block_write_other;
						break;
					}
				}
			}		
			printk("get SPI chip driver!\n");
#ifdef CONFIG_RTK_VOIP
			spin_unlock_irqrestore(spi_lock,flags);
#endif
			
			return chip_info;
		}
		else{
			//printk("can not get SPI chip driver!\n");
		}
	}
	printk("can not get SPI chip driver!\n");
#ifdef CONFIG_RTK_VOIP
	spin_unlock_irqrestore(spi_lock,flags);
#endif

	return NULL;
}
EXPORT_SYMBOL(spi_probe_flash_chip);
//module_exit(spi_suzaku_destroy);