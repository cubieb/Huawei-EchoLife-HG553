#ifndef FW_SYSCALL_H
#define FW_SYSCALL_H

#include <net/if.h>

 /* modify by sxg (60000658), ACL IP地址支持按网段设置,2006/02/20,  begin*/
//#define SEC_BUFF_MAX_LEN     16     // from secdefs.h
#define SEC_BUFF_MAX_LEN     32     // from secdefs.h// from secdefs.h   192.168.111.112-192.168.111.113  最大长度32个字符邋(含'\0')
 /* modify by sxg (60000658), ACL IP地址支持按网段设置,2006/02/20,  end*/

#define IFC_NAME_LEN		     16		// from ifcdefs.h
#define BOARD_IOCTL_MAGIC       'B'
#ifndef _UINT32_
#define _UINT32_
typedef unsigned long   UINT32;
#endif
typedef unsigned char   byte;

#define XML_PSI_START_TAG      "<psitree>"
#define XML_PSI_END_TAG        "</psitree>"

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

#define BOARD_IOCTL_FLASH_READ \
    _IOWR(BOARD_IOCTL_MAGIC, 2, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_PSI_SIZE \
    _IOWR(BOARD_IOCTL_MAGIC, 11, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_CHIP_ID \
    _IOWR(BOARD_IOCTL_MAGIC, 14, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_MIPS_SOFT_RESET \
    _IOWR(BOARD_IOCTL_MAGIC, 6, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_FLASH_WRITE \
    _IOWR(BOARD_IOCTL_MAGIC, 1, BOARD_IOCTL_PARMS)
    
int boardIoctl(int board_ioctl, BOARD_IOCTL_ACTION action, char *string, int strLen, int offset, char *buf);
/* HTTP upload image formats. */
typedef enum
{
    NO_IMAGE_FORMAT,
    BROADCOM_IMAGE_FORMAT,
    FLASH_IMAGE_FORMAT,
    PSI_IMAGE_FORMAT,
} PARSE_RESULT;

typedef enum {
   CLI_ACCESS_DISABLED = 0,
   CLI_ACCESS_LOCAL,
   CLI_ACCESS_REMOTE,
   CLI_ACCESS_CONSOLE,
   CLI_ACCESS_REMOTE_SSH,     
   CLI_ACCESS_REMOTE_TELNET,
} CLI_ACCESS_WHERE;


typedef enum
{
    BUF_ALL_TYPES,
    BUF_IMAGES,
    BUF_CONFIG_DATA,
} BUFFER_TYPE;

typedef enum
{
    UPLOAD_OK,
    UPLOAD_FAIL_NO_MEM,
    UPLOAD_FAIL_ILLEGAL_IMAGE,
    UPLOAD_FAIL_FLASH,
    UPLOAD_FAIL_FTP,
} UPLOAD_RESULT;

#define     SKIP_NONE_APP       0x00000000
#define     SKIP_TELNETD_APP    0x00000001
#define     SKIP_SSHD_APP       0x00000002
#define     SKIP_HTTPD_APP      0x00000004


#if defined(__cplusplus)
extern "C" {
#endif
PARSE_RESULT parseImageData(char *image_start_ptr, int bufSize, BUFFER_TYPE fBufType);
UPLOAD_RESULT flashImage(char *imagePtr, PARSE_RESULT imageType, int imageLen);
int bcmCheckEnable(char *appName, struct in_addr clntAddr);
int bcmSocketIfPid(void);
void killAllApps(int socketPid, int skipApp, int telnetIfcPid, int sshdIfcPid);
UINT32 getCrc32(byte *pdata, UINT32 size, UINT32 crc);
#if defined(__cplusplus)
}
#endif   // defined(__cplusplus)

#endif
