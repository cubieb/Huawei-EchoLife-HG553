#ifndef _BOARD_API_H
#define _BOARD_API_H
#define BOARD_IOCTL_MAGIC       'B'

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
    BOARD_IOCTL_ACTION  action;
    int result;
} BOARD_IOCTL_PARMS;

typedef enum 
{
    MODE_PASSBY = 3,
    MODE_NORM,
    MODE_UNDEFINED,                
}E_HSPA_TRAFFICMODE_VERSION;

#define BOARD_IOCTL_WAKEUP_MONITOR_TASK \
    _IOWR(BOARD_IOCTL_MAGIC, 18, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_SET_TRAFFIC_MODE \
        _IOWR(BOARD_IOCTL_MAGIC, 38, BOARD_IOCTL_PARMS)  

#endif
