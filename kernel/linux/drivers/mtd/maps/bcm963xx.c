/*
 * A simple flash mapping code for BCM963xx board flash memory
 * It is simple because it only treats all the flash memory as ROM
 * It is used with chips/map_rom.c
 *
 *  Song Wang (songw@broadcom.com)
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
//#include <linux/config.h>

#include <board.h>
#include <bcmTag.h>
#include <bcm_map_part.h>
//#include "atpflash.h"
#include <bcm_hwdefs.h>

static int  getBootsystem();

#define  BCM_MTD_VERSION	"2.0"	/* NOR FLASH ONLY */

const char * null_mtd_c = "NULLMTD";

/*
 * MTD Driver Entry Points using kerSys interface to flash_api
 *
 * Assumption:
 * - Single region with all sectors of the same size per MTD device registered.
 * - BankSize = 2
 *
 * Runtime spcification of device size, offset using Flash Partition Info.
 */

static int bcm63xx_erase_invalid(struct mtd_info *mtd, struct erase_info *instr)
{
    char * mtdname_p = (char*)null_mtd_c;
    if ( mtd ) mtdname_p = mtd->name;
    printk("ERROR: bcm63xx_erase_invalid( mtd[%s])\n", mtdname_p );

    /* Proceed as if done */
    instr->state = MTD_ERASE_DONE;
    mtd_erase_callback( instr );

    return (0);
}

static int bcm63xx_erase(struct mtd_info *mtd, struct erase_info *instr)
{
    unsigned long flash_base;
    //  printk("\r\n bcm63xx_erase 111111111111\n");
    if ( mtd == (struct mtd_info *)NULL )
    {
        printk("ERROR: bcm63xx_erase( mtd[%s])\n", null_mtd_c);
        return (-EINVAL);
    }

    if ( instr->addr + instr->len > mtd->size )
    {
        printk("ERROR: bcm63xx_erase( mtd[%s]) invalid region\n", mtd->name);
        return (-EINVAL);
    }

    flash_base = (unsigned long)mtd->priv;

    if ( kerSysEraseFlash( flash_base + instr->addr, instr->len) )
        return (-EINVAL);

    instr->state = MTD_ERASE_DONE;
    mtd_erase_callback( instr );

    return (0);
}

static int bcm63xx_point_invalid(struct mtd_info *mtd, loff_t from,
			size_t len, size_t *retlen, u_char **mtdbuf)
{
    return (-EINVAL);
}

static void bcm63xx_unpoint_invalid(struct mtd_info *mtd, u_char * addr,
			loff_t from, size_t len)
{
}

static int bcm63xx_read(struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf)
{
    unsigned long flash_base;
    *retlen = 0;
    if ( mtd == (struct mtd_info *)NULL )
    {
        printk("ERROR: bcm63xx_read( mtd[%s])\n", null_mtd_c);
        return (-EINVAL);
    }
    //    flash_base = (unsigned long)mtd->priv + FLASH_BASE;
    flash_base = (unsigned long)mtd->priv;

    *retlen = kerSysReadFromFlash(buf, flash_base + from, len); 
    if ( 0 == *retlen)
    {
    //    printk("\n %s %d retlen=0x%x\n",__FILE__,__LINE__,*retlen);
    *retlen = len;
    }
    return 0;
}

static int bcm63xx_write_invalid(struct mtd_info *mtd, loff_t to, size_t len,
			size_t *retlen, const u_char *buf)
{
    char * mtdname_p = (char*)null_mtd_c;
    *retlen = 0;
    if ( mtd ) mtdname_p = mtd->name;
    printk("ERROR: bcm63xx_write_invalid( mtd[%s])\n", mtdname_p );

    return (-EINVAL);
}

static int bcm63xx_write(struct mtd_info *mtd, loff_t to, size_t len,
			size_t *retlen, const u_char *buf)
{
    unsigned long flash_base;
    int bytesRemaining = 0;

    //   printk("\r\n bcm63xx_write 111111111111\n");

    *retlen = 0;
    if ( mtd == (struct mtd_info *)NULL )
    {
        printk("ERROR: bcm63xx_write( mtd[%s])\n", null_mtd_c);
        return (-EINVAL);
    }
    //    flash_base = (unsigned long)mtd->priv + FLASH_BASE;
    flash_base = (unsigned long)mtd->priv;
    bytesRemaining = kerSysWriteToFlash(flash_base+to, (char*)buf, len);
    //write ok
    if ( 0 == bytesRemaining)
    {
        *retlen = len;
    }
    //    printk("\r\n bcm63xx_write 22222222222222\n");

    return 0;
}

static void bcm63xx_noop(struct mtd_info *mtd)
{
	/* NO OPERATION */
}

/*---------------------------------------------------------------------------
 *	List of Broadcom MTD Devices per supported Flash File Systems
 *
 * - Non WRITEABLE MTD device for the Root FileSystem/kernel
 *   [e.g. the RootFS MTD may host SquashFS]
 * - WRITEABLE MTD device for an Auxillary FileSystem, if configured
 *   [e.g. the AuxFS MTD may host JFFS2]
 *---------------------------------------------------------------------------*/

static struct mtd_info mtdRootFS =
{
	.name		= "BCM63XX RootFS",
	.index		= -1,			/* not registered */
	.type		= MTD_NORFLASH,
	.flags		= 0,/* No capability: i.e. CLEAR/SET BITS, ERASEABLE */
	.size		= 0,
	.erasesize	= 0,				/* NO ERASE */
	.numeraseregions= 0,
	.eraseregions	= (struct mtd_erase_region_info*) NULL,
	.bank_size	= 2,
	.read		= bcm63xx_read,
	.erase		= bcm63xx_erase_invalid,	/* READONLY */
	.write		= bcm63xx_write_invalid,	/* READONLY */
	.point		= bcm63xx_point_invalid,	/* No XIP */
	.unpoint	= bcm63xx_unpoint_invalid,	/* No XIP */
	.sync		= bcm63xx_noop,
	// NAND Flash Devices not supported: ecc, oob, kvec read/write
	.priv		= (void*) NULL,
	.owner		= THIS_MODULE
};

#ifdef CONFIG_AUXFS_JFFS2
struct mtd_erase_region_info merAuxFS =
{
	.offset = 0,
	.erasesize = 0,
	.numblocks = 0
};

static struct mtd_info mtdAuxFS =
{
#if 1
	.name		= "BCM63XX AuxFS",
	.index		= -1,			/* not registered */
	.type		= MTD_NORFLASH,
	.flags		= MTD_CAP_NORFLASH, /* MTD_CLEAR_BITS | MTD_ERASEABLE */
				/* No SET_BITS, WRITEB_WRITEABLE, MTD_OOB */
        //   .flags            = MTD_CAP_NORFLASH,
	.numeraseregions= 1,
	.eraseregions	= (struct mtd_erase_region_info*) &merAuxFS,
	.bank_size	= 2,
	.read		= bcm63xx_read,
	.erase		= bcm63xx_erase,
	.write		= bcm63xx_write, 
	 .point		= bcm63xx_point_invalid,	/* No XIP */
	 .unpoint	= bcm63xx_unpoint_invalid,	/* No XIP */
	.sync		= bcm63xx_noop,
	// NAND Flash Devices not supported: ecc, oob, kvec read/write
	.priv		= (void*) NULL,
	.owner		= THIS_MODULE
#else
	.name		= "BCM63XX AuxFS",
	.index		= -1,			/* not registered */
	.type		= MTD_NORFLASH,
	.flags		= 0,/* No capability: i.e. CLEAR/SET BITS, ERASEABLE */
	.size		= 0,
	.erasesize	= 0,				/* NO ERASE */
	.numeraseregions= 0,
	.eraseregions	= (struct mtd_erase_region_info*) NULL,
	.bank_size	= 2,
	.read		= bcm63xx_read,
	.erase		= bcm63xx_erase_invalid,	/* READONLY */
	.write		= bcm63xx_write_invalid,	/* READONLY */
	.point		= bcm63xx_point_invalid,	/* No XIP */
	.unpoint	= bcm63xx_unpoint_invalid,	/* No XIP */
	.sync		= bcm63xx_noop,
	// NAND Flash Devices not supported: ecc, oob, kvec read/write
	.priv		= (void*) NULL,
	.owner		= THIS_MODULE
#endif	
};
#endif

static int __init init_brcm_physmap(void)
{
        unsigned int rootfs_addr, kernel_addr;
        PFILE_TAG pTag = (PFILE_TAG)NULL;
        //   printk("\n===start to init mtd=11111========\n");
        kerSysFlashInit();
        printk("\n===start to init mtd=222========\n");
#ifdef CONFIG_AUXFS_JFFS2
        //	//FLASH_PARTITION_INFO fPartAuxFS;	/* Runtime partitioning info */
#endif

	printk("bcm963xx_mtd driver v%s\n", BCM_MTD_VERSION);

	/*
	 * Data fill the runtime configuration of MTD RootFS Flash Device
	 */
	//if (! (pTag = kerSysImageTagGet()) )
#ifdef  CONFIG_ATP_SUPPORT_DOUBLEMIDD	
    if (1 == getBootsystem())
    {
    
          printk("\n==boot from slave kernel==\n");
         
#if 0          

        rootfs_addr = (unsigned int)FLASH_BASE + (unsigned int)BHAL_SLAVE_FS_START_OFFSET+(unsigned int)TAG_LEN;
        
        
         printk("\n==bcm963xx 222===FLASH_BASE=0x%x ==BHAL_SLAVE_KERNEL_START_OFFSET=0x%x=TAG_LEN=0x%x==\n",(unsigned int)FLASH_BASE,(unsigned int)BHAL_SLAVE_KERNEL_START_OFFSET,(unsigned int)TAG_LEN);
        kernel_addr = (unsigned int)FLASH_BASE + (unsigned int)BHAL_SLAVE_KERNEL_START_OFFSET+(unsigned int)TAG_LEN;
        
        printk("\n==bcm963xx 111===rootfs_addr=0x%x kernel_addr=0x%x===\n",rootfs_addr,kernel_addr);
        if ((mtdRootFS.size = (unsigned int)(BHAL_SLAVE_FS_SIZE)) <= 0)
       {
		printk("Invalid RootFs size\n");
		return -EIO;
       }
#endif        
        /*Start modified by lvxin 00135113@20090309 for AU4D01446*/
        
    #if 0
        if (! (pTag = bhalGetTagByFlashOffset(BHAL_SLAVE_KERNEL_START_OFFSET)))
        {
        printk("Failed to read image tag from slave system.\n");
        return -EIO;
        }
    #endif
        if (! (pTag = bhalReadSlaveTag()))
        {
            printk("Failed to read image tag from slave system.\n");
            return -EIO;
        }
      
        printk("\n===start to init mtd=333========\n");
            rootfs_addr = (unsigned int) 
            simple_strtoul(pTag->rootfsAddress, NULL, 10)+(BHAL_SLAVE_KERNEL_START_OFFSET-BHAL_MAIN_KERNEL_START_OFFSET)
                    + BOOT_OFFSET;
            kernel_addr = (unsigned int)
            simple_strtoul(pTag->kernelAddress, NULL, 10)+(BHAL_SLAVE_KERNEL_START_OFFSET-BHAL_MAIN_KERNEL_START_OFFSET)
                    + BOOT_OFFSET;
            if ((mtdRootFS.size = (unsigned int)(kernel_addr - rootfs_addr)) <= 0)
            {
                printk("Invalid RootFs size\n");
                return -EIO;
            }
        printk("\n==pTag->rootfsAddress=0x%x pTag->kernelAddress=0x%x==\n",(unsigned int)simple_strtoul(pTag->rootfsAddress, NULL, 10),
            (unsigned int)simple_strtoul(pTag->kernelAddress, NULL, 10));
        /*End modified by lvxin 00135113@20090309 for AU4D01446*/

    }
    else
#endif
     {
            printk("\n==boot from main kernel==\n");
            if (! (pTag = kerSysImageTagGet()) )
            {
                printk("Failed to read image tag from flash\n");
                return -EIO;
            }

            //      printk("\n===start to init mtd=333========\n");
            rootfs_addr = (unsigned int) 
            simple_strtoul(pTag->rootfsAddress, NULL, 10)
                    + BOOT_OFFSET;
            kernel_addr = (unsigned int)
            simple_strtoul(pTag->kernelAddress, NULL, 10)
                    + BOOT_OFFSET;
            if ((mtdRootFS.size = (unsigned int)(kernel_addr - rootfs_addr)) <= 0)
            {
                printk("Invalid RootFs size\n");
                return -EIO;
            }

     }

  
        // printk("\n==bcm963xx  222===rootfs_addr=0x%x kernel_addr=0x%x===\n",rootfs_addr,kernel_addr);


        // printk("\n==bcm963xx  333===mtdRootFS.size =%u==\n",mtdRootFS.size );
        //  printk("\n===bcm=555========\n");
	/*
	 * CAUTION:
	 * rootfs_addr is NOT ALIGNED WITH a sector boundary.
	 * As, RootFS MTD is not writeable and not explicit erase capability
	 * this is not an issue.
	 * Support for writeable RootFS mtd would need to take into account
	 * the offset of rootfs_addr from the sector base.
	 */
        //	mtdRootFS.priv = (void*)rootfs_addr - FLASH_BASE;
	mtdRootFS.priv = (void*)rootfs_addr;
	if ( add_mtd_device( &mtdRootFS ) )	/* Register Device RootFS */
	{
		printk("Failed to register device mtd[%s]\n", mtdRootFS.name);
		return -EIO;
	}
        //   printk("\n===bcm=666========\n");
	printk("Registered device mtd[%s] dev[%d] Flash[0x%08x,%u]\n",
		mtdRootFS.name, mtdRootFS.index,
		(int)mtdRootFS.priv, mtdRootFS.size);


#ifdef CONFIG_AUXFS_JFFS2 && CONFIG_ATP_SUPPORT_HG553
       /*only the hg553 project support jffs2 fs */
	/*
	 * Data fill the runtime configuration of MTD AuxFS Flash Device
	 */
        //printk("\n===bcm=777========\n");
#if 0
	/* Read the flash memory partition map. */
	kerSysFlashPartInfoGet( JFFS_AUXFS, & fPartAuxFS );

	/*
	 * Assuming a single eraseregion with all sectors of the same size!!!
	 */
	if ( fPartAuxFS.sect_size != 0 ) /* Check assumption */
	{
		mtdAuxFS.priv = (void*)fPartAuxFS.mem_base;
		mtdAuxFS.size = fPartAuxFS.mem_length;

		mtdAuxFS.erasesize = fPartAuxFS.sect_size;

		mtdAuxFS.numeraseregions = 1;
		mtdAuxFS.eraseregions->offset = 0;
		mtdAuxFS.eraseregions->erasesize = fPartAuxFS.sect_size;
		mtdAuxFS.eraseregions->numblocks = fPartAuxFS.number_blk;
#else

	/*
	 * Assuming a single eraseregion with all sectors of the same size!!!
	 */
	//if ( fPartAuxFS.sect_size != 0 ) /* Check assumption */
	{
#if 0	
	       unsigned int middsize,flashbase,midds;
//		mtdAuxFS.priv = (void*)BHAL_MIDDLEWARE_FS_START_OFFSET + BHAL_MIDDLEWARE_FS_SIZE;
            middsize = (unsigned int)simple_strtoul(BHAL_MIDDLEWARE_FS_START_OFFSET, NULL, 10);
            flashbase = (unsigned int)simple_strtoul(FLASH_BASE, NULL, 10);
            midds = (unsigned int)simple_strtoul(BHAL_MIDDLEWARE_FS_SIZE, NULL, 10);
		mtdAuxFS.priv =  (unsigned int)(middsize + flashbase);
		mtdAuxFS.size = midds;
            printk("\n %s %d mtdAuxFS.priv:0x%x midds:0x%x\n",__FILE__,__LINE__,mtdAuxFS.priv,midds);
		mtdAuxFS.erasesize = (unsigned int)simple_strtoul(0x20000, NULL, 10);//(unsigned int)(0x20000);

		mtdAuxFS.numeraseregions = 1;
		mtdAuxFS.eraseregions->offset = 0;
//		mtdAuxFS.eraseregions->offset = BHAL_MIDDLEWARE_FS_START_OFFSET;
		mtdAuxFS.eraseregions->erasesize = mtdAuxFS.erasesize;
		mtdAuxFS.eraseregions->numblocks = mtdAuxFS.size/mtdAuxFS.erasesize;
#endif		
            //printk("\n===bcm=888========\n");
            mtdAuxFS.priv = (unsigned int)(BHAL_MIDDLEWARE_FS_START_OFFSET) + (unsigned int)(FLASH_BASE);
            mtdAuxFS.size = (unsigned int)(BHAL_MIDDLEWARE_FS_SIZE);
            //mtdAuxFS.ecctype = MTD_ECC_NONE;//add by l63336
#if 1
            //printk("\n===bcm=999========\n");
            mtdAuxFS.erasesize = (unsigned int)0x20000;
            mtdAuxFS.numeraseregions = 1;
            mtdAuxFS.eraseregions->offset = 0;
            //mtdAuxFS.eraseregions->offset = BHAL_MIDDLEWARE_FS_START_OFFSET;
            mtdAuxFS.eraseregions->erasesize = mtdAuxFS.erasesize;
            //mtdAuxFS.eraseregions->numblocks = mtdAuxFS.size/mtdAuxFS.erasesize;
            mtdAuxFS.eraseregions->numblocks = mtdAuxFS.size/mtdAuxFS.erasesize;
#endif
#endif
            //printk("\n===bcm=000111========\n");
		if ( add_mtd_device( & mtdAuxFS ) ) /*Register Device AuxFS */
		{
			printk("Failed to register device mtd[%s]\n",
				 mtdAuxFS.name);
			return -EIO;
		}
                        printk("\n===bcm=mtdaufs.size=%u========\n",mtdAuxFS.size);
		printk("Registered device mtd[%s] dev[%d] Flash[0x%08x,%u]\n",
			mtdAuxFS.name, mtdAuxFS.index,
			(int)mtdAuxFS.priv, mtdAuxFS.size);
             //printk("\n===bcm=000333111 ========\n");

	}
#endif
       //printk("\n===bcm=000444111 =end======\n");
	return 0;
}

static void __exit cleanup_brcm_physmap(void)
{
	if (mtdRootFS.index >= 0)
	{
		mtdRootFS.index = -1;

		del_mtd_device( &mtdRootFS );

        	mtdRootFS.size = 0;
		mtdRootFS.priv = (void*)NULL;
	}

#ifdef CONFIG_AUXFS_JFFS2
	if (mtdAuxFS.index >= 0)
	{
		mtdAuxFS.index = -1;

		del_mtd_device( &mtdAuxFS );

		mtdAuxFS.size = 0;
		mtdAuxFS.priv = (void*)NULL;
		mtdAuxFS.erasesize = 0;
		mtdAuxFS.numeraseregions = 0;
		mtdAuxFS.eraseregions->offset = 0;
		mtdAuxFS.eraseregions->erasesize = 0;
		mtdAuxFS.eraseregions->numblocks = 0;
	}
#endif
}

#ifdef CONFIG_ATP_SUPPORT_DOUBLEMIDD 

static int getBootsystem()
{
    int	    flashTotalSize =  0;
    unsigned char        pszStartInfo[BOOT_TAG_SIZE];
    signed int            lRet;
    
    memset (pszStartInfo,0,sizeof(pszStartInfo));
    flashTotalSize= bhalGetFlashSize();
    //假如FLASH小于16M，就不进行双系统升级
    if (flashTotalSize < DOUBLE_FLASHSIZE)
    {
        printk("\n %s %d\n",__FILE__,__LINE__);
        return 0 ;
    }
    lRet = bhalReadFlash((void *)pszStartInfo,
                             FLASH_BASE + flashTotalSize - BOOT_TAG_SIZE,
                             BOOT_TAG_SIZE);     

    /*IMAGE_SLAVE_S/IMAGE_MAIN_F需要从MAIN启动*/
    if ((NULL != strstr(pszStartInfo, IMAGE_SLAVE_S)) || (NULL != strstr(pszStartInfo, IMAGE_MAIN_F)) )
    {
    	 return 1;
    }   

   return 0;     
}
#endif

module_init(init_brcm_physmap);
module_exit(cleanup_brcm_physmap);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Song Wang songw@broadcom.com");
MODULE_DESCRIPTION("Configurable MTD Driver for Flash File System");
