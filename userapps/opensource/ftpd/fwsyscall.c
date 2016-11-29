/*****************************************************************************
//
//  Copyright (c) 2000-2003  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
******************************************************************************
//
//  Filename:       fwsyscall.c
//  Author:         seanl
//  Creation Date:  10/10/03
//                  The system call for kill process and firmware flashing 
//                  functions (from syscall.c and dltftp.c)
//
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <pwd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <net/if.h>
#include <ctype.h>

#include "fwsyscall.h"
#define  BCMTAG_EXE_USE
#include "bcmTag.h"

extern char **environ;
char glbIfName[IFC_NAME_LEN];

#define READ_BUF_SIZE       128
#define CMD_LEN             256

#define SYS_CMD_LEN        512
#define CLI_MAX_BUF_SZ       512

#define TRUE 1
#define FALSE 0

typedef enum _psiStatus {
   PSI_STS_OK = 0,
   PSI_STS_ERR_GENERAL,
   PSI_STS_ERR_MEMORY,
   PSI_STS_ERR_OBJECT_NOT_FOUND,
   PSI_STS_ERR_APP_NOT_FOUND,
   PSI_STS_ERR_OBJECT_EXIST,
   PSI_STS_ERR_APP_EXIST,
   PSI_STS_ERR_INVALID_LENGTH,
   PSI_STS_ERR_INVALID_VALUE,
   PSI_STS_ERR_INVALID_CRC,
   PSI_STS_ERR_INVALID_MAGIC_VALUE,
   PSI_STS_ERR_INVALID_PROFILE_NUM,
   PSI_STS_ERR_PROFILE_NAME,
   PSI_STS_ERR_PROFILE_TOO_LARGE,
   PSI_STS_ERR_CONFIGFILE_UNKNOWN,
   PSI_STS_ERR_CONFIGFILE_SIZE
} PSI_STATUS;
/***************************************************************************
// Function Name: bcmSocketIfPid().
// Description  : return the socket if pid
// Parameters   : none.
// Returns      : pid
****************************************************************************/
int bcmSocketIfPid(void)
{
   int pid = 0;
   char buffer[CMD_LEN];
   char cmd[CMD_LEN];
   FILE *fs;

   if (glbIfName[0] == '\0')
      return pid;    

   sprintf(cmd, "/proc/var/fyi/wan/%s/pid", glbIfName);
   if ((fs = fopen(cmd, "r")) != NULL)
   {
      fgets(cmd, SYS_CMD_LEN, fs);
      pid = atoi(cmd);
      fclose(fs);
   }

   if (strstr(glbIfName, "ppp") != NULL)     // only do it if it is ppp
   {
      // add the default route with the ifc the socket is sitting on
      sprintf(buffer, "route add default dev %s", glbIfName);
      bcmSystem(buffer);
   }

   return pid;
}

int boardIoctl(int board_ioctl, BOARD_IOCTL_ACTION action, char *string, int strLen, int offset, char *buf)
{
    BOARD_IOCTL_PARMS IoctlParms;
    int boardFd = 0;

    boardFd = open("/dev/brcmboard", O_RDWR);
    if ( boardFd != -1 ) {
        IoctlParms.string = string;
        IoctlParms.strLen = strLen;
        IoctlParms.offset = offset;
        IoctlParms.action = action;
        IoctlParms.buf    = buf;
        ioctl(boardFd, board_ioctl, &IoctlParms);
        close(boardFd);
        boardFd = IoctlParms.result;
    } else
        printf("Unable to open device /dev/brcmboard.\n");

    return boardFd;
}

int bcmSystem (char *command) {
   int pid = 0, status = 0;

   if ( command == 0 )
      return 1;

   pid = fork();
   if ( pid == -1 )
      return -1;

   if ( pid == 0 ) 
   {
      char *argv[4];
      argv[0] = "sh";
      argv[1] = "-c";
      argv[2] = command;
      argv[3] = 0;
      execvp("/bin/sh", argv);
      exit(127);
   }

   /* wait for child process return */
   do 
   {
      if ( waitpid(pid, &status, 0) == -1 ) 
      {
         if ( errno != EINTR )
         {
            return -1;
         }
      }
      else
      {
         return status;
      }
   } while ( 1 );

   return status;
}

int bcmIsModuleInserted(char *modName) {
   int ret = FALSE;
   char buf[SYS_CMD_LEN];
   FILE* fs = fopen("/proc/modules", "r");

   if ( fs != NULL ) {
      while ( fgets(buf, SYS_CMD_LEN, fs) > 0 )
         if ( strstr(buf, modName) != NULL ) {
            ret = TRUE;
            break;
         }
      fclose(fs);
   }

   return ret;
}

int bcmCheckForRedirect(void)
{
   char col[11][32];
   char line[512];
   FILE* fs;  
   int count = 0;

   if (bcmIsModuleInserted("iptable_nat") == FALSE)
      return FALSE;

   bcmSystem("iptables -t nat -L PREROUTING_UTILITY > /var/nat_redirect");

   fs = fopen("/var/nat_redirect", "r");
   if ( fs != NULL ) {
      while ( fgets(line, sizeof(line), fs) ) {
         // read pass 3 header lines
         if ( count++ < 3 )
            continue;
         sscanf(line, "%s %s %s %s %s %s %s %s %s %s %s",
               col[0], col[1], col[2], col[3], col[4], col[5],
               col[6], col[7], col[8], col[9], col[10]);
        if ((strcmp(col[0], "REDIRECT") == 0) && (strcmp(col[1], "tcp") == 0) && (strcmp(col[8], "ports") == 0))
          if (strcmp(col[9], "80") == 0 || strcmp(col[9], "23") == 0 || strcmp(col[9], "21") == 0 ||
              strcmp(col[9], "22") == 0 || strcmp(col[9], "69") == 0) {
              return TRUE;
          }
      }
      fclose(fs);
   }
   unlink("/var/nat_redirect");
   return FALSE;
}


void bcmRemoveModules(int lanIf)
{
   char *modList[]=
   {
      "bcm_enet",
      "bcm_usb",
      "ipt_state",
      "ipt_mark",
      "ipt_limit",
      "ipt_tcpmss_j",
      "ipt_REDIRECT",
      "ipt_MASQUERADE",
      "ipt_mark_j",
      "ipt_LOG",
      "ipt_FTOS",
      "ip_nat_tftp",
      "ip_nat_irc",
      "ip_nat_ftp",
      "ip_nat_h323",
      "ip_nat_pptp",
      "ip_nat_gre",
      "ip_nat_rtsp",
      "ip_nat_ipsec",
      "ip_conntrack_tftp",
      "ip_conntrack_irc",
      "ip_conntrack_ftp",
      "ip_conntrack_h323",
      "ip_conntrack_pptp",
      "ip_conntrack_gre",
      "ip_conntrack_rtsp",
      "ip_conntrack_ipsec",
      "iptable_mangle",
      "ip_conntrack",
      "ip_tables",
      NULL,
   };

   char cmd[SYS_CMD_LEN];
   int i = 0;
   int saveNat = FALSE;

   if (lanIf == 0)         // if lan, do not kill bcm_usb and bcm_enet
      i = 2;
   else // if in ipow mode, leave bcm_enet out   
   {
      FILE *fs = fopen("/proc/var/fyi/wan/eth0/pid", "r");
      if (fs != NULL) 
      {
         i = 1;        
         fclose(fs);
      }
   }

   saveNat = bcmCheckForRedirect();
   if (bcmIsModuleInserted("iptable_filter") == TRUE)
   {  
       strncpy(cmd, "iptables -t filter -F", SYS_CMD_LEN-1);
       bcmSystem(cmd);
   }
   if (bcmIsModuleInserted("iptable_nat") == TRUE)
   {  
       strncpy(cmd, "iptables -t nat -F", SYS_CMD_LEN-1);
       bcmSystem(cmd);
   }
   if (bcmIsModuleInserted("iptable_mangle") == TRUE)
   {  
       strncpy(cmd, "iptables -t mangle -F", SYS_CMD_LEN-1);
       bcmSystem(cmd);
   }

   while (modList[i] != NULL)
   {
      if (bcmIsModuleInserted(modList[i]) == TRUE) 
      {
         if (!(saveNat && strcmp(modList[i], "iptable_nat") == 0))
         {
            sprintf(cmd, "rmmod %s", modList[i]);
            bcmSystem(cmd);
         }
      }
      i++;
   }

   sleep(1);
}

/***************************************************************************
// Function Name: killAllApps().
// Description  : kill all available applications.
// Parameters   : socketPid    - if none 0, the pid for the interface the applicat
//							     is using.
//                skipApp      - check if tftp is on telnet or sshd and skip the app kill
//                telnetIfcPid, ssshdIfcPid  - the pid of the interface telnet or sshd is on
// Returns      : none.
****************************************************************************/
void killAllApps(int socketPid, int skipApp, int telnetIfcPid, int sshdIfcPid) {
   // NOTE: Add the app name before NULL. Order is important
   char *apps[]=
   {
      "telnetd",        // NOTE: has to be the first for tftp client (no kill)
      "sshd",           //       and sshd on the second (will not kill if sshd exist)
      /* start of maintain Auto Upgrade by zhangliang 60003055 2006年5月24日
      为了能在升级时弹出警告窗口，不杀掉httpd
      //       and httpd on the third -- will not kill if update xml file (processed in httpd)
      "httpd",          
      end of maintain Auto Upgrade by zhangliang 60003055 2006年5月24日 */
      "bftpd",
      "tftpd",
      "snmp",
      "upnp",
      "sntp",
      "ddnsd",
      "reaim",
      "klogd",
      "syslogd",
      "ripd",
      "zebra ",
      /* start of maintain Auto Upgrade 升级时不能断掉ppp连接 by zhangliang 60003055 2006年5月8日"
      "pppd",
      end of maintain Auto Upgrade by zhangliang 60003055 2006年5月8日" */
      "dnsprobe",
      "dhcpc",
      "igmp", 
      "radiusd",
      NULL,
   };
   char cmd[CMD_LEN], app[CMD_LEN];
   char buffer[CMD_LEN];
   char pidStr[CMD_LEN];
   int pid = 0;
   int i = 0;
   FILE *fs;
   int curPid = getpid();
   char psList[] = "/var/pslist";

   bcmSystem("ps > /var/pslist");
   fs = fopen(psList, "r");
   //bcmSystem("cat /var/pslist");
   

   // get ps line and parse for the pid for killing
   do 
   {
      if (i == 0 && skipApp & SKIP_TELNETD_APP)
          continue;
      if (i == 1 && skipApp & SKIP_SSHD_APP)
          continue;
      if (i == 2 && skipApp & SKIP_HTTPD_APP)
          continue;
      strcpy(app, apps[i]);
      while (fgets(buffer, CMD_LEN, fs) > 0) 
      {
                  
         if (strstr(buffer, app) != NULL) // found command line with match app name
         {  
            // find pid string
            sscanf(buffer, "%s\n", pidStr);
            pid = atoi(pidStr);
            if (pid != curPid && pid != socketPid && pid != telnetIfcPid && pid !=sshdIfcPid)
            {
               printf("kill process [pid: %d] [name: %s]...\n", pid, app);
               sprintf(cmd, "kill -9 %d", pid);
               bcmSystem(cmd);
            }
         }
      }
      rewind(fs);
   } while (apps[++i] != NULL);

   fclose(fs);
   unlink(psList);
   bcmRemoveModules(socketPid);

}

/***************************************************************************
// Function Name: getCrc32
// Description  : caculate the CRC 32 of the given data.
// Parameters   : pdata - array of data.
//                size - number of input data bytes.
//                crc - either CRC32_INIT_VALUE or previous return value.
// Returns      : crc.
****************************************************************************/
UINT32 getCrc32(byte *pdata, UINT32 size, UINT32 crc)
{
    while (size-- > 0)
        crc = (crc >> 8) ^ Crc32_table[(crc ^ *pdata++) & 0xff];

    return crc;
}


/***************************************************************************
// Function Name: verifyTag
// Description  : check if the image is brcm image or not. If passNumber == 1,
//                just check the tag crc for memory allocation.  If passNumber == 2, 
//                check the image crc and tagVersion.
// Parameters   : pTag and passNumber.
// Returns      : UPLOAD_OK or UPLOAD_FAIL_ILLEGAL_IMAGE
****************************************************************************/
UPLOAD_RESULT verifyTag(PFILE_TAG pTag, int passNumber)
{
    UINT32 crc;
    UPLOAD_RESULT status = UPLOAD_OK;
    int totalImageSize;
    int tagVer, curVer, tagVerLast;

    // check tag validate token first
    crc = CRC32_INIT_VALUE;
    crc = getCrc32((char*) pTag, (UINT32)TAG_LEN-TOKEN_LEN, crc);

    if (crc != (UINT32)(*(UINT32*)(pTag->tagValidationToken)))
    {
        printf("Not regular image file\n");
        return UPLOAD_FAIL_ILLEGAL_IMAGE;
    }
    if (passNumber == 1)
        return status;

    // if second pass, check imageCrc and tagVersion
    totalImageSize = atoi(pTag->totalImageLen);
    crc = CRC32_INIT_VALUE;
    crc = getCrc32(((char*)pTag + TAG_LEN), (UINT32) totalImageSize, crc);      
    if (crc != (UINT32) (*(UINT32*)(pTag->imageValidationToken)))
        return UPLOAD_FAIL_ILLEGAL_IMAGE;

    // only allow same or greater tag versions (up to tagVerLast - 26) to be uploaded
    tagVer = atoi(pTag->tagVersion);
    curVer = atoi(BCM_TAG_VER);
    tagVerLast = atoi(BCM_TAG_VER_LAST);

    if (tagVer < curVer || tagVer > tagVerLast)
    {
       printf("Firmware tag version [%d] is not compatible with the current Tag version [%d]\n", tagVer, curVer);
       return UPLOAD_FAIL_ILLEGAL_IMAGE;
    }

    return status;
}



/***************************************************************************
// Function Name: psiVerify
// Description  : Verify whether it is a kind of valid xml file by just check <psitree> </psitree>
// Parameters   : psiBuf.
// Returns      : PSI_STS_OK -- psi kind of verified -- need xmlparse in cfm.
****************************************************************************/

PSI_STATUS psiVerify(char *psiBuf, int imageSize) {

   if (imageSize > boardIoctl(BOARD_IOCTL_GET_PSI_SIZE, 0, "", 0, 0, ""))
       return PSI_STS_ERR_INVALID_LENGTH;
 
   // if <psitree>  </psitree> cannot be found then return
   if ((memcmp(psiBuf, XML_PSI_START_TAG, strlen(XML_PSI_START_TAG)) != 0) || !(strstr(psiBuf, XML_PSI_END_TAG)))
       return PSI_STS_ERR_GENERAL;

   return PSI_STS_OK;
}

// create the xml file and signal httpd to process it
// return: UPLOAD_RESULT
UPLOAD_RESULT createXmlAndSignalUser(char *imagePtr, int imageLen)
{
    char xmlFileName[] = "/var/psi_rcv.xml";
    char httpPidFile[] = "/var/run/httpd_pid";
    FILE *fp = NULL;
    int httpdPid =0;
    char cmd[CMD_LEN];

    if (access(xmlFileName, F_OK) == 0) 
        unlink(xmlFileName);
    if ((fp = fopen(xmlFileName, "w")) == NULL)
    {
        printf("Failed to create %s\n", xmlFileName);
        return UPLOAD_FAIL_FTP;
    }
       
    if (fwrite(imagePtr, 1, imageLen, fp) != imageLen)
    {
        printf("Failed to write %s\n", xmlFileName);
        fclose(fp);
        return UPLOAD_FAIL_FTP;
    }    
    fclose(fp);
    
    // need to signal httpd to process this xml file
    if ((fp = fopen(httpPidFile, "r")) == NULL)
    {
        printf("Failed to read %s\n", httpPidFile);
        return UPLOAD_FAIL_FTP;
    }
    else 
    {
        fgets(cmd, CMD_LEN, fp);
        httpdPid = atoi(cmd);
        fclose(fp);
    }

    if (httpdPid > 0)
    {
        kill(httpdPid, SIGUSR1);
        return UPLOAD_OK;
    }
    else
        return UPLOAD_FAIL_FTP;
}

int sysFlashImageSet(void *image, int size, int addr,
    BOARD_IOCTL_ACTION imageType)
{
    int result;

    result = boardIoctl(BOARD_IOCTL_FLASH_WRITE, imageType, image, size, addr, "");

    return(result);
}

// depending on the image type, do the brcm image or whole flash image and the configuration data
// return: UPLOAD_RESULT
UPLOAD_RESULT flashImage(char *imagePtr, PARSE_RESULT imageType, int imageLen)
{
    PFILE_TAG pTag = (PFILE_TAG) imagePtr;
    int cfeSize, rootfsSize, kernelSize;
    unsigned long cfeAddr, rootfsAddr, kernelAddr;
    UPLOAD_RESULT status = UPLOAD_OK;
    PSI_STATUS psiStatus = PSI_STS_OK;

    switch (imageType) 
    {
    case PSI_IMAGE_FORMAT:
        if (imageLen > boardIoctl(BOARD_IOCTL_GET_PSI_SIZE, 0, "", 0, 0, "") || (psiStatus = psiVerify(imagePtr, imageLen)) != PSI_STS_OK)
        {
            printf("Failed to verify configuration data\n");
            status = UPLOAD_FAIL_FLASH;
        }
        if ((status = createXmlAndSignalUser(imagePtr, imageLen)) == UPLOAD_OK)
           return status;
        else
           printf("\nFailed to create configuration file and signal user - error[%d]\n", (int) status);
        break;

    case FLASH_IMAGE_FORMAT:
        printf("\nFlash whole image...");
        // Pass zero for the base address of whole image flash. It will be filled by kernel code
        if ((sysFlashImageSet(imagePtr, imageLen-TOKEN_LEN, 0, BCM_IMAGE_WHOLE)) == -1)
        {
            printf("Failed to flash whole image\n");
            status = UPLOAD_FAIL_FLASH;
        }
        break;

    case BROADCOM_IMAGE_FORMAT:
        cfeSize = rootfsSize = kernelSize = 0;
        // must be tagged Broadcom image.  Check cfe's existence
        cfeAddr = (unsigned long) strtoul(pTag->cfeAddress, NULL, 10);
        cfeSize = atoi(pTag->cfeLen);
        // check kernel existence
        kernelAddr = (unsigned long) strtoul(pTag->kernelAddress, NULL, 10);
        kernelSize = atoi(pTag->kernelLen);
        // check root filesystem existence
        rootfsAddr = (unsigned long) strtoul(pTag->rootfsAddress, NULL, 10);
        rootfsSize = atoi(pTag->rootfsLen);
        if (cfeAddr) 
        {
           printf("\nFlashing CFE...\n");
           if ((sysFlashImageSet(imagePtr+TAG_LEN, cfeSize, (int) cfeAddr,
                 BCM_IMAGE_CFE)) == -1)
           {
                 printf("Failed to flash CFE\n");
                 status = UPLOAD_FAIL_FLASH;
           }
        }
        if (rootfsAddr && kernelAddr) 
        {
           char *tagFs = imagePtr;
           // tag is alway at the sector start of fs
           if (cfeAddr)
           {
               tagFs = imagePtr + cfeSize;       // will trash cfe memory, but cfe is already flashed
               memcpy(tagFs, imagePtr, TAG_LEN);
           }
           printf("Flashing root file system and kernel...\n");
           if ((sysFlashImageSet(tagFs, TAG_LEN+rootfsSize+kernelSize, 
              (int)(rootfsAddr-TAG_LEN), BCM_IMAGE_FS)) == -1)      
           {
               printf("Failed to flash root file system\n");
               status = UPLOAD_FAIL_FLASH;
           }
        }
        break;

    default:
        printf("Illegal image data\n");
        break;
    }

    if (status != UPLOAD_OK)
        printf("**** IMAGE FLASH FAILED ****\n");
    else 
        printf("Image flash done.\n");
    
    return status;
}


// Check for configuration data
PARSE_RESULT checkConfigData(char *image_start_ptr, int imageSize)
{
    PARSE_RESULT result = NO_IMAGE_FORMAT;
    PSI_STATUS psiResult = PSI_STS_ERR_GENERAL;

    if ((psiResult = psiVerify(image_start_ptr, imageSize)) == PSI_STS_OK) 
    {
        printf("Could be configuration data...\n");
	    result = PSI_IMAGE_FORMAT;
    }
    return result;
}

// Check Broadcom image format
PARSE_RESULT checkBrcmImage(char *image_start_ptr)
{
    PARSE_RESULT result = NO_IMAGE_FORMAT;

    /* start of maintain Auto Upgrade by zhangliang 60003055 2006年5月8日"
    if (verifyTag((FILE_TAG *) image_start_ptr, 2) == UPLOAD_OK)
    {
        printf("Broadcom format verified.\n");
        result = BROADCOM_IMAGE_FORMAT;
    }
    */
    PFILE_TAG filetag_ptr = (PFILE_TAG)image_start_ptr;
    if (verifyTag((FILE_TAG *) image_start_ptr, 2) == UPLOAD_OK)
    {
        printf("CRC check verified.\n");
//        printf("Leo_DBG:%s\n", filetag_ptr->productName);
        if (0 == strcmp(filetag_ptr->productName, HGW_PRODUCTNAME))
        {
            printf("Broadcom format verified.\n");
            result = BROADCOM_IMAGE_FORMAT;
        }
        else
        {
            printf("Product type not matched!\n");
        }
    }
    /* end of maintain Auto Upgrade by zhangliang 60003055 2006年5月8日" */
    return result;
}


// Check flash image format
PARSE_RESULT checkFlashImage(char *image_start_ptr, int bufSize)
{
    unsigned char *image_ptr = image_start_ptr;
    unsigned long image_len = bufSize - TOKEN_LEN;
    unsigned long crc = CRC32_INIT_VALUE;
    PARSE_RESULT result = NO_IMAGE_FORMAT;
    
    /* start of maintain Auto Upgrade by zhangliang 60003055 2006年5月8日"
    crc = getCrc32(image_ptr, image_len, crc);      
    if (memcmp(&crc, image_ptr + image_len, CRC_LEN) == 0)
    {
        printf("Flash image format verified.\n");
        result = FLASH_IMAGE_FORMAT;
    }
    */
    // FLASH 镜像前端有64kB 用来保存CFE 程序,需要跳过这段再获取文件头信息
    PFILE_TAG filetag_ptr = (PFILE_TAG)(image_start_ptr + 64*1024);
    crc = getCrc32(image_ptr, image_len, crc);      
    if (memcmp(&crc, image_ptr + image_len, CRC_LEN) == 0)
    {
        printf("CRC check verified.\n");
//        printf("Leo_DBG:%s\n", filetag_ptr->productName);
        if (0 == strcmp(filetag_ptr->productName, HGW_PRODUCTNAME))
        {
            printf("Flash image format verified.\n");
            result = FLASH_IMAGE_FORMAT;
        }
        else
        {
            printf("Product type not matched!\n");
        }
    }
    /* end of maintain Auto Upgrade by zhangliang 60003055 2006年5月8日" */
    return result;
}


// parseImageData
// parse the image data to see if it Broadcom flash format or flash image format file or psi data
// fBufType definition: BUF_ALL_TYPES:   check all types (2 images and psi)
//                      BUF_IMAGES:      check images only
//                      BUF_CONFIG_DATA: check psi only
// return: NO_IMAGE_FORMAT  
//         BROADCOM_IMAGE_FORMAT       
//         FLASH_IMAGE_FORMAT          
//         PSI_IMAGE_FORMAT

PARSE_RESULT parseImageData(char *image_start_ptr, int bufSize, BUFFER_TYPE fBufType)
{
    PARSE_RESULT result = NO_IMAGE_FORMAT;

    switch (fBufType) 
    {
    case BUF_ALL_TYPES:
        if ((result = checkBrcmImage(image_start_ptr)) == BROADCOM_IMAGE_FORMAT)
            break;
        if ((result = checkFlashImage(image_start_ptr, bufSize)) == FLASH_IMAGE_FORMAT)
            break;
        if ((result = checkConfigData(image_start_ptr, bufSize)) == PSI_IMAGE_FORMAT)
            break;

    case BUF_IMAGES:
        if ((result = checkBrcmImage(image_start_ptr)) == BROADCOM_IMAGE_FORMAT)
            break;
        if ((result = checkFlashImage(image_start_ptr, bufSize)) == FLASH_IMAGE_FORMAT)
            break;
    case BUF_CONFIG_DATA:
        result = checkConfigData(image_start_ptr, bufSize);
        break;
    
    default:
        printf("Illegal buffer type?\n");
        break;
    }

    return result;
}


/***************************************************************************
// Function Name: BcmScm_isServiceEnabled.
// Description  : check the given service is enabled.
// Parameters   : name -- the given service name.
//                where -- local or remote.
// Returns      : TRUE or FALSE.
****************************************************************************/
int BcmScm_isServiceEnabled(char *name, int where) {
   int enbl = FALSE, len = 0, lan = 0, wan = 0;
   char line[CLI_MAX_BUF_SZ];
   FILE* fs = fopen("/var/services.conf", "r");

   // if list is empty then return default which is
   // return true if access from lan and
   // return false if access from wan
   if ( fs == NULL ) {
      if ( where == CLI_ACCESS_LOCAL )
         return TRUE;
      else
         return FALSE;
   }

   len = strlen(name);

   while ( fscanf(fs, "%s\t%d\t%d", line, &lan, &wan) != EOF )
      if ( strncasecmp(line, name, len) == 0 ) {
         if ( where == CLI_ACCESS_LOCAL )
            enbl = lan;
         else if ( where == CLI_ACCESS_REMOTE )
            enbl = wan;
         break;
      }

   fclose(fs);

   return enbl;
}



/***************************************************************************
// Function Name: BcmScm_isInAccessControlList.
// Description  : check the given IP address in the access control list.
// Parameters   : addr -- the given IP address.
// Returns      : TRUE or FALSE.
****************************************************************************/
int BcmScm_isInAccessControlList(char * addr) {
   int in = TRUE, len = 0;
   char line[SEC_BUFF_MAX_LEN];
   FILE* fs = fopen("/var/acl.conf", "r");

    /* add by sxg (60000658), ACL IP地址支持按网段设置,2006/02/20,  begin*/
    char *pszIPEnd = NULL;
    struct in_addr stIPStart = {0L};
    struct in_addr stIPEnd = {0L};
    struct in_addr stIP = {0L};  
    /* add by sxg (60000658), ACL IP地址支持按网段设置,2006/02/20,  end*/
   // if list is empty then return true
   // since allow to access for all by default
   if ( fs == NULL )
      return in;

   // get access control mode
   fgets(line, SEC_BUFF_MAX_LEN, fs);

   // if mode is disabled (zero) then return true
   // since allow to access for all by default
   if ( atoi(line) == 0 ) {
      fclose(fs);
      return in;
   }

   // mode is enabled then access control list is active
   // so in should be initialized to false
   in = FALSE;
   len = strlen(addr);

   while ( fgets(line, SEC_BUFF_MAX_LEN, fs) ){
    /* modify by sxg (60000658), ACL IP地址支持按网段设置,2006/02/22,  begin*/
        /*只定义起始地址*/
        if (NULL == (pszIPEnd = strstr(line, "-")))
        {
            if(inet_aton( line, &stIPStart ) 
                && inet_aton( addr, &stIP ))
            {
                if ((stIP.s_addr) == (stIPStart.s_addr))
                {
                    in = TRUE;
                    break;
                }
            }
        }
        else/*起始地址和结束地址都存在*/
        {
            *pszIPEnd = '\0';
            if (inet_aton( line, &stIPStart ) 
                && inet_aton( pszIPEnd + 1, &stIPEnd ) 
                && inet_aton( addr, &stIP ))
            {                
                if ( ((stIPEnd.s_addr <= stIP.s_addr) && (stIP.s_addr <= stIPStart.s_addr))
                    ||((stIPEnd.s_addr >= stIP.s_addr) && (stIP.s_addr >= stIPStart.s_addr)))
                {
                    in = TRUE;
                    *pszIPEnd = '-';
                    break;
                }
            }
            
            *pszIPEnd = '-';
        }
        /* modify by sxg (60000658), ACL IP地址支持按网段设置,2006/02/22,  end*/
    }
   fclose(fs);

   return in;
}

static void getIpExtIp(char *buf)
{
   FILE* fs;
   char wan[64], gateway[64], dns[64], str[256];

   if ( buf == NULL ) return;

   buf[0] = '\0';
   fs = fopen("/var/ipextinfo", "r");
   if ( fs != NULL ) {
      fgets(str, 256, fs);
      fclose(fs);
      sscanf(str, "%s %s %s\n", wan, gateway, dns);
      strcpy(buf, wan);
   }
}

static int isIpExtension(void)
{
   FILE *fp;
   int ipextension = 0;

   if ((fp=fopen("/var/ipextension","r")) != NULL) {
      fscanf(fp,"%d",&ipextension);
      fclose(fp);
   }

   return ipextension;
}

/***************************************************************************
// Function Name: bcmCheckInterfaceUp().
// Description  : check status of interface.
// Parameters   : devname - name of device.
// Returns      : 1 - UP.
//                0 - DOWN.
****************************************************************************/
int bcmCheckInterfaceUp(char *devname) {
   int  skfd;
   int  ret;
   struct ifreq intf;

   if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      return 0;
   }

   strcpy(intf.ifr_name, devname);

   // if interface is br0:0 and
   // there is no binding IP address then return down
   if ( strchr(devname, ':') != NULL ) {
      if (ioctl(skfd, SIOCGIFADDR, &intf) < 0) {
         close(skfd);
         return 0;
      }
   }

   // if interface flag is down then return down
   if (ioctl(skfd, SIOCGIFFLAGS, &intf) == -1) {
      ret = 0;
   } else {
      if ( (intf.ifr_flags & IFF_UP) != 0)
         ret = 1;
      else
         ret = 0;
   }

   close(skfd);

   return ret;
}

static int getLanInfo(char *lan_ifname, struct in_addr *lan_ip, struct in_addr *lan_subnetmask)
{
   int socketfd;
   struct ifreq lan;

   if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
     printf("app: Error openning socket when getting LAN info\n");
     return -1;
   }

   strcpy(lan.ifr_name,lan_ifname);
   if (ioctl(socketfd,SIOCGIFADDR,&lan) < 0) {
     printf("app: Error getting LAN IP address\n");
     close(socketfd);
     return -1;
   }
   *lan_ip = ((struct sockaddr_in *)&(lan.ifr_addr))->sin_addr;

   if (ioctl(socketfd,SIOCGIFNETMASK,&lan) < 0) {
     printf("app: Error getting LAN subnet address\n");
     close(socketfd);
     return -1;
   }

   *lan_subnetmask = ((struct sockaddr_in *)&(lan.ifr_netmask))->sin_addr;

   close(socketfd);
   return 0;
}

int isAccessFromLan(struct in_addr clntAddr)
{
   int ret = 0;
   struct in_addr inAddr, inMask;
   char wan[64];

   getLanInfo("br0", &inAddr, &inMask);
   /* check ip address of support user to see it is in LAN or not */
   if ( (clntAddr.s_addr & inMask.s_addr) == (inAddr.s_addr & inMask.s_addr) )
      ret = 1;
   else {
      /* check ip address of support user to see if it is from secondary LAN */
      if (bcmCheckInterfaceUp("br0:0")) {
         getLanInfo("br0:0", &inAddr, &inMask);
         if ( (clntAddr.s_addr & inMask.s_addr) == (inAddr.s_addr & inMask.s_addr) )
            ret = 1;
      }

      /* Last option it must be from WAN side */
      if (isIpExtension()) {
         getIpExtIp(wan);
      if ( clntAddr.s_addr == inet_addr(wan) )
         ret = 1;
      }
   }

   return ret;
}

//**************************************************************************
// Function Name: bcmcheck_enable
// Description  : check the appName with ip address against the psi 
//                for access mode
// Parameters   : appName - application name in the acl.conf (telnet, ssh, etc.)
//                clntAddr - incoming ip address 
// Returns      : access mode - CLI_ACCESS_LOCAL, CLI_REMOTE_LOCAL, CLI_ACCESS_DISABLED
//**************************************************************************
int bcmCheckEnable(char *appName, struct in_addr clntAddr)
{
   // retrieve PSI from flash to make sure it's up-to-date
   //BcmPsi_profileRetrieve(0);

   // is client address in Access Control List ?
   /*del by sxg(60000658) ACL IP 仅对wan侧进行限制, 2006/02/22, begin*/
   /*if ( BcmScm_isInAccessControlList(inet_ntoa(clntAddr)) == FALSE )
      return CLI_ACCESS_DISABLED;
      */
  /*del by sxg(60000658) ACL IP 仅对wan侧进行限制, 2006/02/22, end*/

   if ( isAccessFromLan(clntAddr) == TRUE ) {
      // is enabled from lan ?
      if ( BcmScm_isServiceEnabled(appName, CLI_ACCESS_LOCAL) == FALSE )
         return CLI_ACCESS_DISABLED;
      else
         return CLI_ACCESS_LOCAL;
   } else {
      // is enabled from wan ?
      if ( BcmScm_isServiceEnabled(appName, CLI_ACCESS_REMOTE) == FALSE )
         return CLI_ACCESS_DISABLED;
      /*add by sxg(60000658) ACL IP 仅对wan侧进行限制, 2006/02/22, begin*/
      else if ( BcmScm_isInAccessControlList(inet_ntoa(clntAddr)) == FALSE )
          return CLI_ACCESS_DISABLED;
      /*add by sxg(60000658) ACL IP 仅对wan侧进行限制, 2006/02/22, end*/
      else
         return CLI_ACCESS_REMOTE;
   }
}


