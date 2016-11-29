/* ralink-flash.c
 *
 * Steven Liu <steven_liu@ralinktech.com.tw>:
 *   - initial approach
 *
 * Winfred Lu <winfred_lu@ralinktech.com.tw>:
 *   - 32MB flash support for RT3052
 *   - flash API
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/err.h>

#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/concat.h>
#include <linux/mtd/partitions.h>


#ifndef CONFIG_RT2880_FLASH_32M 
#define WINDOW_ADDR		CONFIG_MTD_PHYSMAP_START
#define WINDOW_SIZE		CONFIG_MTD_PHYSMAP_LEN
#define NUM_FLASH_BANKS		1
#else
#define WINDOW_ADDR_0		CONFIG_MTD_PHYSMAP_START
#define WINDOW_ADDR_1		0xBB000000
#define WINDOW_SIZE		(CONFIG_MTD_PHYSMAP_LEN / 2)
#define NUM_FLASH_BANKS		2
#endif

#define BUSWIDTH		CONFIG_MTD_PHYSMAP_BUSWIDTH


static struct mtd_info *ralink_mtd[NUM_FLASH_BANKS];
#ifndef CONFIG_RT2880_FLASH_32M 
static struct map_info ralink_map[] = {
	{
	.name = "Ralink SoC physically mapped flash",
	.bankwidth = BUSWIDTH,
	.size = WINDOW_SIZE,
	.phys = WINDOW_ADDR
	}
};
#else
static struct mtd_info *merged_mtd;
static struct map_info ralink_map[] = {
	{
	.name = "Ralink SoC physically mapped flash bank 0",
	.bankwidth = BUSWIDTH,
	.size = WINDOW_SIZE,
	.phys = WINDOW_ADDR_0
	},
	{
	.name = "Ralink SoC physically mapped flash bank 1",
	.bankwidth = BUSWIDTH,
	.size = WINDOW_SIZE,
	.phys = WINDOW_ADDR_1
	}
};
#endif

#if defined (CONFIG_RT2880_FLASH_32M) && defined (CONFIG_RALINK_RT3052_MP2)
static struct mtd_partition rt2880_partitions[] = {
        {
                name:           "Bootloader",  /* mtdblock0 */
                size:           0x40000,  /* 192K */
                offset:         0,
        }, {
#ifdef CONFIG_RT2880_ROOTFS_IN_FLASH
                name:           "Kernel",
                size:           CONFIG_MTD_KERNEL_PART_SIZ,
                offset:         MTDPART_OFS_APPEND,
        }, {
                name:           "RootFS",
                size:           (0x2000000 - 0x80000 - CONFIG_MTD_KERNEL_PART_SIZ),
                offset:         MTDPART_OFS_APPEND,
        }, {
#else //CONFIG_RT2880_ROOTFS_IN_RAM
                name:           "Kernel",
                size:           (0x2000000 - 0x80000),
                offset:         MTDPART_OFS_APPEND,
        }, {
#endif
                name:           "Config",
                size:           0x20000,  /* 128K */
                offset:         MTDPART_OFS_APPEND
        }, {
                name:           "Factory",
                size:           0x20000,  /* 128K */
                offset:         MTDPART_OFS_APPEND
        }
};
#else //not 32M flash
static struct mtd_partition rt2880_partitions[] = {
        {
                name:           "Bootloader",  /* mtdblock0 */
                size:           0x20000,  /* 128K */
                offset:         0,
        }, {
                name:           "Factory", /* mtdblock1 */
                size:           0x10000,  /* 64K */
                offset:         MTDPART_OFS_APPEND
        }, {
#ifdef CONFIG_RT2880_ROOTFS_IN_FLASH
                name:           "Kernel",
                size:           CONFIG_MTD_KERNEL_PART_SIZ,
                offset:         MTDPART_OFS_APPEND,
        }, {
                name:           "RootFS",
                size:           MTDPART_SIZ_FULL,
                offset:         MTDPART_OFS_APPEND,
        },
        {
                name:           "Protect",
                size:            0x40000,
                offset:         MTDPART_OFS_APPEND,
        }
#else //CONFIG_RT2880_ROOTFS_IN_RAM
                name:           "Kernel",
                size:           MTDPART_SIZ_FULL,
                offset:         MTDPART_OFS_APPEND,
        },
        {
                name:           "Protect",
                size:           0x40000,
                offset:         MTDPART_OFS_APPEND,
        }
#endif
};
#endif

static int ralink_lock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	return 0;
}

static int ralink_unlock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	return 0;
}

#ifdef CONFIG_RT2880_ROOTFS_IN_FLASH
typedef struct _image_header {
	uint8_t ih_unused[60];
	uint32_t ih_ksz;		/* Kernel Part Size             */
} _im_hdr_t;
#endif

int __init rt2880_mtd_init(void)
{
	int ret = -ENXIO;
	int i, found = 0;
#ifdef CONFIG_RT2880_ROOTFS_IN_FLASH
	_im_hdr_t hdr;
	void *ptr;

	//winfred: parse kernel size and update rt2880_partitions
	ptr = CONFIG_MTD_PHYSMAP_START + rt2880_partitions[0].size + rt2880_partitions[1].size;
	memcpy(&hdr, ptr, sizeof(_im_hdr_t));
	//printk("!!!! kern size %d\n", ntohl(hdr.ih_ksz));
	rt2880_partitions[2].size = ntohl(hdr.ih_ksz);
#endif
	rt2880_partitions[3].size = 0x400000 - 0x20000- 0x10000 - 0x40000 - rt2880_partitions[2].size;

	for (i = 0; i < NUM_FLASH_BANKS; i++) {
		printk(KERN_NOTICE "ralink flash device: 0x%x at 0x%x\n",
				ralink_map[i].size, ralink_map[i].phys);

		ralink_map[i].virt = ioremap(ralink_map[i].phys, ralink_map[i].size);
		if (!ralink_map[i].virt) {
			printk("Failed to ioremap\n");
			return -EIO;
		}
		simple_map_init(&ralink_map[i]);

		ralink_mtd[i] = do_map_probe("cfi_probe", &ralink_map[i]);
		if (ralink_mtd[i]) {
			ralink_mtd[i]->owner = THIS_MODULE;
			ralink_mtd[i]->lock = ralink_lock;
			ralink_mtd[i]->unlock = ralink_unlock;
			++found;
		}
		else
			iounmap(ralink_map[i].virt);
	}
	if (found == NUM_FLASH_BANKS) {
#ifdef CONFIG_RT2880_FLASH_32M
		merged_mtd = mtd_concat_create(ralink_mtd, NUM_FLASH_BANKS,
				"Ralink Merged Flash");
		ret = add_mtd_partitions(merged_mtd, rt2880_partitions,
				ARRAY_SIZE(rt2880_partitions));
#else
		ret = add_mtd_partitions(ralink_mtd[0], rt2880_partitions,
				ARRAY_SIZE(rt2880_partitions));
#endif
		if (ret) {
			for (i = 0; i < NUM_FLASH_BANKS; i++)
				iounmap(ralink_map[i].virt);
			return ret;
		}
	}
	else {
		printk("Error: %d flash device was found\n", found);
		return -ENXIO;
	}
	return 0;
}

static void __exit rt2880_mtd_cleanup(void)
{
	int i;

#ifdef CONFIG_RT2880_FLASH_32M 
	if (merged_mtd) {
		del_mtd_device(merged_mtd);
		mtd_concat_destroy(merged_mtd);
	}
#endif
	for (i = 0; i < NUM_FLASH_BANKS; i++) {
		if (ralink_mtd[i])
			map_destroy(ralink_mtd[i]);
		if (ralink_map[i].virt) {
			iounmap(ralink_map[i].virt);
			ralink_map[i].virt = NULL;
		}
	}
}

module_init(rt2880_mtd_init);
module_exit(rt2880_mtd_cleanup);

MODULE_AUTHOR("Steven Liu <steven_liu@ralinktech.com.tw>");
MODULE_DESCRIPTION("Ralink APSoC Flash Map");
MODULE_LICENSE("GPL");

/*
 * Flash API: ra_mtd_read, ra_mtd_write
 * Arguments:
 *   - num: specific the mtd number
 *   - to/from: the offset to read from or written to
 *   - len: length
 *   - buf: data to be read/written
 * Returns:
 *   - return -errno if failed
 *   - return the number of bytes read/written if successed
 */
int ra_mtd_write(int num, loff_t to, size_t len, const u_char *buf)
{
	int ret = -1;
	size_t rdlen, wrlen;
	struct mtd_info *mtd;
	struct erase_info ei;
	u_char *bak = NULL;

	mtd = get_mtd_device(NULL, num);
	if (IS_ERR(mtd))
		return (int)mtd;
	if (len > mtd->erasesize) {
		put_mtd_device(mtd);
		return -E2BIG;
	}

	bak = kmalloc(mtd->erasesize, GFP_KERNEL);
	if (bak == NULL) {
		put_mtd_device(mtd);
		return -ENOMEM;
	}

	ret = mtd->read(mtd, 0, mtd->erasesize, &rdlen, bak);
	if (ret != 0) {
		put_mtd_device(mtd);
		kfree(bak);
		return ret;
	}
	if (rdlen != mtd->erasesize)
		printk("warning: ra_mtd_write: rdlen is not equal to erasesize\n");

	memcpy(bak + to, buf, len);

	ei.mtd = mtd;
	ei.callback = NULL;
	ei.addr = 0;
	ei.len = mtd->erasesize;
	ei.priv = 0;
	ret = mtd->erase(mtd, &ei);
	if (ret != 0) {
		put_mtd_device(mtd);
		kfree(bak);
		return ret;
	}

	ret = mtd->write(mtd, 0, mtd->erasesize, &wrlen, bak);

	put_mtd_device(mtd);
	kfree(bak);
	return ret;
}

int ra_mtd_read(int num, loff_t from, size_t len, u_char *buf)
{
	int ret;
	size_t rdlen;
	struct mtd_info *mtd;

	mtd = get_mtd_device(NULL, num);
	if (IS_ERR(mtd))
		return (int)mtd;

	ret = mtd->read(mtd, from, len, &rdlen, buf);
	if (rdlen != len)
		printk("warning: ra_mtd_read: rdlen is not equal to len\n");

	put_mtd_device(mtd);
	return ret;
}

EXPORT_SYMBOL(ra_mtd_write);
EXPORT_SYMBOL(ra_mtd_read);

