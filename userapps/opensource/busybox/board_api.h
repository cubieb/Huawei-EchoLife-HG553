#if !defined(_BOARD_API_H_)
#define _BOARD_API_H_

#if defined(__cplusplus)
extern "C" {
#endif


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
    PSI_BACKUP,
    FIX,
    AVAIL,
	BCM_IMAGE_FS_NO_REBOOT,

} BOARD_IOCTL_ACTION;  
    
typedef struct boardIoctParms
{
    char *string;
    char *buf;
    int strLen;
    int offset;
    BOARD_IOCTL_ACTION  action;        /* flash read/write: nvram, persistent, bcm image */
    int result;
} BOARD_IOCTL_PARMS;

#define BOARD_IOCTL_MAGIC       'B'


#define BOARD_IOCTL_FLASH_WRITE \
    _IOWR(BOARD_IOCTL_MAGIC, 1, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_FLASH_READ \
    _IOWR(BOARD_IOCTL_MAGIC, 2, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_MIPS_SOFT_RESET \
    _IOWR(BOARD_IOCTL_MAGIC, 6, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_PSI_SIZE \
    _IOWR(BOARD_IOCTL_MAGIC, 11, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_CHIP_ID \
    _IOWR(BOARD_IOCTL_MAGIC, 14, BOARD_IOCTL_PARMS)


int boardIoctl(int board_ioctl, BOARD_IOCTL_ACTION action, char *string, int strLen, int offset, char *buf);


int sysFlashImageSet(void *image, int size, int addr,
    BOARD_IOCTL_ACTION imageType);


#if defined(__cplusplus)
}
#endif

#endif /* _BOARD_API_H_ */

