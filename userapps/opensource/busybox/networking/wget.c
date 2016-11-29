/* vi: set sw=4 ts=4: */
/*
 * wget - retrieve a file using HTTP or FTP
 *
 * Chip Rosenthal Covad Communications <chip@laserlink.net>
 *
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#include <fcntl.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <getopt.h>

#include "busybox.h"
#include "openssl/md5.h"

struct host_info {
	char *host;
	char *path;
	char *user;
    char *password;
    char *localFile;
    int is_https;
    unsigned short port;
};

static void parse_url(char *url, struct host_info *h);
static int open_socket(struct sockaddr_in *s_in, FILE **fIn, FILE **fOut);
static char *gethdr(char *buf, size_t bufsiz, FILE *fp, int *istrunc);

/* Globals (can be accessed from signal handlers */
static off_t filesize = 0;		/* content-length of the file */
static int chunked = 0;			/* chunked transfer encoding */
/* start of HG553_protocal : Added by c65985, 2010.2.27  banner on the page HG553V100R001 */
static const char *bannerflag = "0";
static char bannername[128] = {0};

/* end of HG553_protocal : Added by c65985, 2010.2.27  banner on the page HG553V100R001 */
#ifdef CONFIG_FEATURE_WGET_STATUSBAR
static void progressmeter(int flag);
static char *curfile;			/* Name of current file being transferred. */
static struct timeval start;	/* Time a transfer started. */
static volatile unsigned long statbytes = 0; /* Number of bytes transferred so far. */
/* For progressmeter() -- number of seconds before xfer considered "stalled" */
static const int STALLTIME = 5;
#endif

static char *m_pcLocalIP = NULL;

static void close_and_delete_outfile(FILE* output)
{
	if (output != stdout) {
		fclose(output);
	}
}

/* Read NMEMB elements of SIZE bytes into PTR from STREAM.  Returns the
 * number of elements read, and a short count if an eof or non-interrupt
 * error is encountered.  */
static size_t safe_fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t ret = 0;

	do {
		clearerr(stream);
		ret += fread((char *)ptr + (ret * size), size, nmemb - ret, stream);
	} while (ret < nmemb && ferror(stream) && errno == EINTR);

	return ret;
}

/* Write NMEMB elements of SIZE bytes from PTR to STREAM.  Returns the
 * number of elements written, and a short count if an eof or non-interrupt
 * error is encountered.  */
static size_t safe_fwrite(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t ret = 0;

	do {
		clearerr(stream);
		ret += fwrite((char *)ptr + (ret * size), size, nmemb - ret, stream);
	} while (ret < nmemb && ferror(stream) && errno == EINTR);

	return ret;
}

/* Read a line or SIZE - 1 bytes into S, whichever is less, from STREAM.
 * Returns S, or NULL if an eof or non-interrupt error is encountered.  */
static char *safe_fgets(char *s, int size, FILE *stream)
{
	char *ret;

	do {
		clearerr(stream);
		ret = fgets(s, size, stream);
	} while (ret == NULL && ferror(stream) && errno == EINTR);

	return ret;
}

/* BEGIN: Added by c65985, 2010/11/15   PN:banner down fail*/
#define close_delete_and_die(s...) { \
	if (1 == atoi(bannerflag)) \
	{system("echo 2 > /var/bannerflag");} \
	close_and_delete_outfile(localFile); \
	bb_error_msg_and_die(s);}
/* END:   Added by c65985, 2010/11/15 PN:banner down fail*/

#define close_and_die(s...) {   \
	close(localFile);           \
	bb_error_msg_and_die(s); }


#ifdef CONFIG_FEATURE_WGET_AUTHENTICATION
#define HTTP_HD_AUTHORIZATION           "Authorization"
#define HTTP_HD_AUTHORIZATION_SIZE      (13)

#define HTTPAUTH_TYPE_BASIC_STR         "Basic"     // Case-insensitive
#define HTTPAUTH_TYPE_BASIC_STR_LEN     5           // Const length, calculated manually for efficiency
#define HTTPAUTH_TYPE_DIGEST_STR        "Digest"
#define HTTPAUTH_TYPE_DIGEST_STR_LEN    6           // Const length, calculated manually for efficiency

#define HTTPAUTH_DEFAULT_REALM          "Huawei"

#define HASHLEN 16
typedef char HASH[HASHLEN];
#define HASHHEXLEN 32
typedef char HASHHEX[HASHHEXLEN+1];

/*
 *  Base64-encode character string
 *  oops... isn't something similar in uuencode.c?
 *  It would be better to use already existing code
 */
char *base64enc(unsigned char *p, char *buf, int len) {

        char al[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
                    "0123456789+/";
		char *s = buf;

        while(*p) {
				if (s >= buf+len-4)
					bb_error_msg_and_die("buffer overflow");
                *(s++) = al[(*p >> 2) & 0x3F];
                *(s++) = al[((*p << 4) & 0x30) | ((*(p+1) >> 4) & 0x0F)];
                *s = *(s+1) = '=';
                *(s+2) = 0;
                if (! *(++p)) break;
                *(s++) = al[((*p << 2) & 0x3C) | ((*(p+1) >> 6) & 0x03)];
                if (! *(++p)) break;
                *(s++) = al[*(p++) & 0x3F];
        }

		return buf;
}

#define HTTPDigest_SkipSpace(x) while ((isspace((unsigned char)(*(x)))) && ('\0' != (*(x)))) {(x)++;} \
                                        if ('\0' == (*(x))) {bb_error_msg_and_die("syntax error");}

// All the names are case insensitive
static char *HTTPDigest_FieldNames[] =
{
    "realm",                // The coresponding value is case-sensitive
    "nonce",                // The coresponding value is case-sensitive
    "qop",                  // The coresponding value is case-sensitive
    "username",             // The coresponding value is case-sensitive
    "uri",                  // The coresponding value is case-sensitive
    "cnonce",               // The coresponding value is case-sensitive
    "nc",                   // The coresponding value is case-sensitive
    "response",             // The coresponding value is case-sensitive
    "algorithm",            // The coresponding value is case-sensitive
    "opaque",               // The coresponding value is case-sensitive
    "stale",                // TRUE: digest is OK, but username/password not OK;
                            // FALSE: username/password not OK
    NULL
};

/*
 *  Local types
 */
typedef enum tagHTTPDigest_FieldIndex
{
    HTTPDigest_Field_realm,
    HTTPDigest_Field_nonce,
    HTTPDigest_Field_qop,
    HTTPDigest_Field_username,
    HTTPDigest_Field_uri,
    HTTPDigest_Field_cnonce,
    HTTPDigest_Field_nc,
    HTTPDigest_Field_response,
    HTTPDigest_Field_algorithm,
    //lint -save -e749
    HTTPDigest_Field_opaque,
    //lint -restore
    HTTPDigest_Field_End
//lint -save -e751
} HTTPDigest_FieldIndex;
//lint -restore
typedef char *HTTPDigest_Fields[HTTPDigest_Field_End];

#define HTTPAuth_FieldNameMaxLen           32
#define HTTPAuth_FieldValueMaxLen          128

void CvtHex(HASH Bin, HASHHEX Hex)
{
    unsigned short i;
    unsigned char j;

    //lint -save -e734
    for (i = 0; i < HASHLEN; i++)
    {
        // Higher 4 bits
        //lint -save -e702
        j = (Bin[i] >> 4) & 0xf;
        //lint -restore
        if (j <= 9)
        {
            Hex[i*2] = (j + '0');
        }
        else
        {
            Hex[i*2] = (j + 'a' - 10);
        }

        // Lower 4 bits
        j = Bin[i] & 0xf;
        if (j <= 9)
        {
            Hex[i*2+1] = (j + '0');
        }
        else
        {
            Hex[i*2+1] = (j + 'a' - 10);
        }
    };
    Hex[HASHHEXLEN] = '\0';
    //lint -restore
}

/* calculate H(A1) as per spec */
void DigestCalcHA1(
            const char   *pszAlg,
            const char   *pszUserName,
            const char   *pszRealm,
            const char   *pszPassword,
            const char   *pszNonce,
            const char   *pszCNonce,
            HASHHEX     SessionKey)
{
    MD5_CTX Md5Ctx;
    HASH HA1;

    MD5_Init(&Md5Ctx);
    MD5_Update(&Md5Ctx, (unsigned char *)pszUserName, (unsigned int)strlen(pszUserName));
    MD5_Update(&Md5Ctx, (unsigned char *)":", 1);
    MD5_Update(&Md5Ctx, (unsigned char *)pszRealm, (unsigned int)strlen(pszRealm));
    MD5_Update(&Md5Ctx, (unsigned char *)":", 1);
    MD5_Update(&Md5Ctx, (unsigned char *)pszPassword, (unsigned int)strlen(pszPassword));
    MD5_Final((unsigned char *)HA1, &Md5Ctx);

    // Calculate SessionKey if use the MD5-sess algorithm
#ifdef WIN32
    if ((NULL != pszAlg) && (0 == stricmp(pszAlg, "md5-sess")))
#else
    if ((NULL != pszAlg) && (0 == strcasecmp(pszAlg, "md5-sess")))
#endif
    {
        MD5_Init(&Md5Ctx);
        MD5_Update(&Md5Ctx, (unsigned char *)HA1, HASHLEN);
        MD5_Update(&Md5Ctx, (unsigned char *)":", 1);
        MD5_Update(&Md5Ctx, (unsigned char *)pszNonce, (unsigned int)strlen(pszNonce));
        MD5_Update(&Md5Ctx, (unsigned char *)":", 1);
        MD5_Update(&Md5Ctx, (unsigned char *)pszCNonce, (unsigned int)strlen(pszCNonce));
        MD5_Final((unsigned char *)HA1, &Md5Ctx);
    };

    CvtHex(HA1, SessionKey);
}

/* calculate request-digest/response-digest as per HTTP Digest spec */
void DigestCalcResponse(
        HASHHEX      HA1,           /* H(A1) */
        const char   *pszNonce,     /* nonce from server */
        const char   *pszNonceCount,/* 8 hex digits */
        const char   *pszCNonce,    /* client nonce */
        const char   *pszQop,       /* qop-value: "", "auth", "auth-int" */
        const char   *pszMethod,    /* method from the request */
        const char   *pszDigestUri, /* requested URL */
        HASHHEX      HEntity,       /* H(entity body) if qop="auth-int" */
        HASHHEX     Response       /* request-digest or response-digest */
        )
{
    MD5_CTX Md5Ctx;
    HASH HA2;
    HASH RespHash;
    HASHHEX HA2Hex;

    // Calculate H(A2)
    MD5_Init(&Md5Ctx);
    MD5_Update(&Md5Ctx, (unsigned char *)pszMethod, (unsigned int)strlen(pszMethod));
    MD5_Update(&Md5Ctx, (unsigned char *)":", 1);
    MD5_Update(&Md5Ctx, (unsigned char *)pszDigestUri, (unsigned int)strlen(pszDigestUri));
#ifdef WIN32
    if ((NULL != pszQop) && (0 == stricmp(pszQop, "auth-int")))
#else
    if ((NULL != pszQop) && (0 == strcasecmp(pszQop, "auth-int")))
#endif
    {
        MD5_Update(&Md5Ctx, (unsigned char *)":", 1);
        MD5_Update(&Md5Ctx, (unsigned char *)HEntity, HASHHEXLEN);
    };
    MD5_Final((unsigned char *)HA2, &Md5Ctx);
    CvtHex(HA2, HA2Hex);

    // Calculate response
    MD5_Init(&Md5Ctx);
    MD5_Update(&Md5Ctx, (unsigned char *)HA1, HASHHEXLEN);
    MD5_Update(&Md5Ctx, (unsigned char *)":", 1);
    MD5_Update(&Md5Ctx, (unsigned char *)pszNonce, (unsigned int)strlen(pszNonce));
    MD5_Update(&Md5Ctx, (unsigned char *)":", 1);
    if ((NULL != pszQop) && ('\0' != (*pszQop))) // If qop exist, need to digest nc and cnonce
    {
        MD5_Update(&Md5Ctx, (unsigned char *)pszNonceCount, (unsigned int)strlen(pszNonceCount));
        MD5_Update(&Md5Ctx, (unsigned char *)":", 1);
        MD5_Update(&Md5Ctx, (unsigned char *)pszCNonce, (unsigned int)strlen(pszCNonce));
        MD5_Update(&Md5Ctx, (unsigned char *)":", 1);
        MD5_Update(&Md5Ctx, (unsigned char *)pszQop, (unsigned int)strlen(pszQop));
        MD5_Update(&Md5Ctx, (unsigned char *)":", 1);
    };
    MD5_Update(&Md5Ctx, (unsigned char *)HA2Hex, HASHHEXLEN);
    MD5_Final((unsigned char *)RespHash, &Md5Ctx);
    CvtHex(RespHash, Response);
}

static int digest_parse_auth_header(
                                            const char          *pcSrc,
                                            HTTPDigest_Fields   pstDigestFields)
{
    const char *pcPos;
    unsigned char idx;
    int bQuote;

    char pcKey[HTTPAuth_FieldNameMaxLen];
    char pcValue[HTTPAuth_FieldValueMaxLen];

    if ((NULL == pcSrc) || (NULL == pstDigestFields))
    {
        return -1;
    }
    //lint -save -e682
    memset(pstDigestFields, 0, HTTPDigest_Field_End*sizeof(char *));
    //lint -restore
    pcPos = pcSrc;
    while ('\0' != (*pcPos))
    {
        // Parse field name
        HTTPDigest_SkipSpace(pcPos);
        idx = 0;
        while (('\0' != (*pcPos)) && ('=' != (*pcPos)) && (!isspace(*pcPos)) && (idx < HTTPAuth_FieldNameMaxLen))
        {
            pcKey[idx] = (*pcPos);
            idx++;
            pcPos++;
        }
        if (('\0' == (*pcPos)) && (idx >= HTTPAuth_FieldNameMaxLen))
        {
            return -1;
        }
        pcKey[idx] = '\0';
        if ('=' == (*pcPos))
        {
            pcPos++;
        }

        // Parse the field value
        HTTPDigest_SkipSpace(pcPos);
        bQuote = 0;
        if ('"' == (*pcPos))            // Skip the "
        {
            pcPos++;
            bQuote = 1;
        }
        idx = 0;
        if (0 == bQuote)
        {
            while (('\0' != (*pcPos)) && ('"' != (*pcPos)) && (',' != (*pcPos)) &&
                        (!isspace(*pcPos)) && (idx < HTTPAuth_FieldValueMaxLen))
            {
                pcValue[idx] = (*pcPos);
                idx++;
                pcPos++;
            }
        }
        else
        {
            /*star of 解决http 认证失败问题 by s53329 at  20080704  
            while (('\0' != (*pcPos)) && ('"' != (*pcPos)) && (',' != (*pcPos)) &&
                   (idx < HTTPAuth_FieldValueMaxLen))
            */
             while (('\0' != (*pcPos)) && ('"' != (*pcPos)) && 
                   (idx < HTTPAuth_FieldValueMaxLen))
            /*end of 解决http 认证失败问题 by s53329 at  20080704*/
            {
                pcValue[idx] = (*pcPos);
                idx++;
                pcPos++;
            }
        }
        if (idx >= HTTPAuth_FieldValueMaxLen)
        {
            return -1;
        }
        pcValue[idx] = '\0';

        if ('"' == (*pcPos))
        {
            pcPos++;
            while (isspace(*pcPos))
            {
                pcPos++;
            }
        }

        if (',' == (*pcPos))
        {
            pcPos++;
        }

        idx = 0;
        for (idx = 0; idx < HTTPDigest_Field_End; idx++)
        {
            if (NULL != pstDigestFields[idx])
            {
                continue;
            }
            
            if (0 == strncasecmp(HTTPDigest_FieldNames[idx], pcKey, strlen(HTTPDigest_FieldNames[idx])))
            {
                pstDigestFields[idx] = strdup(pcValue);
                if (NULL == pstDigestFields[idx])
                {
                    return -1;
                }
                break;
            }
        }

        if ('\0' == (*pcPos))
        {
            break;
        }
    }

    return 0;
}

static char *http_auth_gen_nonce()
{
    char    acTimeStr[32];
    long    ulNow;
    HASHHEX pcNonce;

    HASH acHash;
    MD5_CTX stMd5Ctx;
    char acRand[32];

    // Time as the seed of random data
    time((time_t *)(&ulNow));
    sprintf(acTimeStr, "%ld", ulNow);
    srand((unsigned int)ulNow);

    MD5_Init(&stMd5Ctx);
    MD5_Update(&stMd5Ctx, (unsigned char *)acTimeStr, (unsigned int)strlen(acTimeStr));
    MD5_Update(&stMd5Ctx, (unsigned char *)":", 1);
    sprintf(acRand, "%ld", (long)rand());

    MD5_Update(&stMd5Ctx, (unsigned char *)acRand, (unsigned int)strlen(acRand));
    MD5_Update(&stMd5Ctx, (unsigned char *)":", 1);
    MD5_Update(&stMd5Ctx, (unsigned char *)HTTPAUTH_DEFAULT_REALM,
                       (unsigned char)strlen(HTTPAUTH_DEFAULT_REALM));

    MD5_Final((unsigned char *)acHash, &stMd5Ctx);

    CvtHex(acHash, pcNonce);

    return bb_xstrdup(pcNonce);
}

char *build_auth_header(
                            const char              *challenge,
                            struct host_info        *target,
                            int                     trans_dir)
{
    char    buf[512];
    char    valueBuf[256];
    int     len;
    HTTPDigest_Fields digestFields;
    char    nonceCnt[12];      // nonce count value is 8LHEX
    char    *pcNonce;
    HASHHEX hSessionKey;
    HASHHEX Response;

    // Basic authorization
    if (0 == strncasecmp(challenge, HTTPAUTH_TYPE_BASIC_STR, HTTPAUTH_TYPE_BASIC_STR_LEN))
    {
        len = snprintf(buf, sizeof(buf), "%s ", HTTPAUTH_TYPE_BASIC_STR);
        snprintf(valueBuf, sizeof(valueBuf), "%s:%s", target->user, target->password);
        base64enc(valueBuf, (buf + len), (sizeof(buf) - len));
        return bb_xstrdup(buf);
    }

    // Must be Digest authorization
    if (0 != strncasecmp(challenge, HTTPAUTH_TYPE_DIGEST_STR, HTTPAUTH_TYPE_DIGEST_STR_LEN))
        bb_error_msg_and_die("no response from server");

    // Parse
    challenge += HTTPAUTH_TYPE_DIGEST_STR_LEN;
    if (digest_parse_auth_header(challenge, digestFields) < 0)
        bb_error_msg_and_die("parse auth header error");
    // Check required fields
    if ((NULL == digestFields[HTTPDigest_Field_realm])  ||
        (NULL == digestFields[HTTPDigest_Field_nonce]) )
        bb_error_msg_and_die("lack fields");

    snprintf(nonceCnt, sizeof(nonceCnt), "%08x", 1);
    pcNonce = http_auth_gen_nonce();
    DigestCalcHA1(digestFields[HTTPDigest_Field_algorithm],
                        target->user,
                        digestFields[HTTPDigest_Field_realm],
                        target->password,
                        digestFields[HTTPDigest_Field_nonce],
                        pcNonce,
                        hSessionKey);
    DigestCalcResponse(
                        hSessionKey,
                        digestFields[HTTPDigest_Field_nonce],
                        nonceCnt,
                        pcNonce,
                        digestFields[HTTPDigest_Field_qop],
                        (2 == trans_dir) ? "POST" : "GET",
                        target->path,
                        "",       // auth-int not implemented yet, Entity should be an empty string
                        Response);

    //2 Build username, realm, nonce and uri fields
    len = snprintf(buf, sizeof(buf),
                   "%s %s=\"%s\", %s=\"%s\", %s=\"%s\", %s=\"%s\", ",
                   HTTPAUTH_TYPE_DIGEST_STR,
                   HTTPDigest_FieldNames[HTTPDigest_Field_username], target->user,
                   HTTPDigest_FieldNames[HTTPDigest_Field_realm], digestFields[HTTPDigest_Field_realm],
                   HTTPDigest_FieldNames[HTTPDigest_Field_nonce], digestFields[HTTPDigest_Field_nonce],
                   HTTPDigest_FieldNames[HTTPDigest_Field_uri], target->path);

    //2 Build the qop, nc and cnonce fields
    if (NULL != digestFields[HTTPDigest_Field_qop])    // Need to fill the nc, cnonce and qop fields
    {
        len += snprintf((buf + len), (sizeof(buf) - len), "%s=%s, %s=\"%s\", %s=\"%s\", ",
                HTTPDigest_FieldNames[HTTPDigest_Field_nc], nonceCnt,
                HTTPDigest_FieldNames[HTTPDigest_Field_cnonce], pcNonce,
                HTTPDigest_FieldNames[HTTPDigest_Field_qop], digestFields[HTTPDigest_Field_qop]);
    }

    //2 Build the algorithm field
    if (NULL != digestFields[HTTPDigest_Field_algorithm])    // Need to fill the algorithm field
    {
        len += snprintf((buf + len), (sizeof(buf) - len), "%s=\"%s\", ",
                HTTPDigest_FieldNames[HTTPDigest_Field_algorithm], digestFields[HTTPDigest_Field_algorithm]);
    }

    // Build the response field
    snprintf((buf + len), (sizeof(buf) - len), "%s=\"%s\"",
                HTTPDigest_FieldNames[HTTPDigest_Field_response], Response);

    return bb_xstrdup(buf);
}

#endif

#define HTTP_UPLOAD_BOUNDARY        "HuaweiBusyboxHttpUploader"
#define HTTP_UPLOAD_BOUNDARY_LEN    (25)

/*
 *  Prefix:
 *
 -----------------------------7d61ffc140e5a

 Content-Disposition: form-data; name="Huawei"; filename="curcfg.xml"
 Content-Type: application/octet-stream
 -----------------------------7d61ffc140e5a--
 */
static void http_upload(struct host_info        *target,
                          struct sockaddr_in    *s_in)
{
    int n, try = 5, status;
    char buf[512];
    char *s;
    int  authCnt;
    int  fileLen;
    int  localFile;             /* local file descriptor            */
    const char *pcFileName;
#ifdef CONFIG_FEATURE_WGET_AUTHENTICATION
    char *pcAuthHeader = NULL;
#endif
    struct stat statbuf;

    int fd;
    FILE *sockOut = NULL;			/* socket write to web/ftp server			*/
    FILE *sockIn  = NULL;			/* socket read from web/ftp server			*/

    localFile = bb_xopen(target->localFile, O_RDONLY);
    if (fstat(localFile, &statbuf) < 0)
        close_and_die("no response from server");
    fileLen = (int)statbuf.st_size;
    close(localFile);

    pcFileName = strrchr(target->localFile, '/');
    if (NULL == pcFileName)
    {
        pcFileName = target->localFile;
    }
    else
    {
        pcFileName += 1;
    }

    // Boundary length and actual file length
    fileLen += HTTP_UPLOAD_BOUNDARY_LEN + 4 + 60 + strlen(pcFileName) +
               42 + HTTP_UPLOAD_BOUNDARY_LEN + 8;

    authCnt = 0;
    fd = -1;
    /*
     *  HTTP Upload session
     */
    do {

        if (! --try)
            bb_error_msg_and_die("too many redirections");

        /*
         * Open socket to http server
         */
        if (sockIn) fclose(sockIn);
        if (sockOut) fclose(sockOut);
        if (-1 != fd)
            close(fd);
        fd = open_socket(s_in, &sockIn, &sockOut);

        localFile = bb_xopen(target->localFile, O_RDONLY);

        /*
         * Send HTTP request.
         */
        fprintf(sockOut, "POST %s HTTP/1.1\r\n", target->path);

        fprintf(sockOut, "Host: %s\r\nUser-Agent: Wget\r\n",
                target->host);

        fprintf(sockOut, "Content-Type: multipart/form-data; boundary=%s\r\nCache-Control:no-cache\r\n", HTTP_UPLOAD_BOUNDARY);

    #ifdef CONFIG_FEATURE_WGET_AUTHENTICATION
        if (NULL != pcAuthHeader) {
            fprintf(sockOut, "Authorization: %s\r\n", pcAuthHeader);
        }
    #endif

        fprintf(sockOut, "Host: %s\r\nUser-Agent: Wget\r\nContent-Length: %d\r\n\r\n",
                target->host, fileLen);

        //fprintf(sfp, "Connection: close\r\n\r\n");

        // HTTP_UPLOAD_BOUNDARY_LEN+4+60+strlen(filename)
        fprintf(sockOut, "--%s\r\nContent-Disposition: form-data; name=\"Huawei\"; filename=\"%s\"\r\n", HTTP_UPLOAD_BOUNDARY, pcFileName);
        // 42 Bytes
        fprintf(sockOut, "Content-Type: application/octet-stream\r\n\r\n");

        fflush(sockOut);

        /*
    	 * Transfer file
    	 */
    	if (bb_copyfd_eof(localFile, fileno(sockOut)) == -1) {
    		exit(ATP_TRANS_SYS_ERR);
    	}
        close(localFile);
        localFile = -1;

        // HTTP_UPLOAD_BOUNDARY_LEN+8
        fprintf(sockOut, "\r\n--%s--\r\n", HTTP_UPLOAD_BOUNDARY);
        fflush(sockOut);

        /*
         * Retrieve HTTP response line and check for "200" status code.
         */
read_response:
        if (fgets(buf, sizeof(buf), sockIn) == NULL)
            bb_error_msg_and_die("no response from server");

        for (s = buf ; *s != '\0' && !isspace(*s) ; ++s)
        ;
        for ( ; isspace(*s) ; ++s)
        ;
        switch (status = atoi(s)) {
            case 0:
            case 100:
                while (gethdr(buf, sizeof(buf), sockIn, &n) != NULL);
                goto read_response;
            case 200:
                break;
            case 300:   /* redirection */
            case 301:
            case 302:
            case 303:
                authCnt = 0;
                break;
            case 401:
                authCnt++;
                break;
                /*FALLTHRU*/
            default:
                chomp(buf);
                bb_error_msg_and_die("server returned error %d: %s", atoi(s), buf);
        }

        /*
         * Retrieve HTTP headers.
         */
        while ((s = gethdr(buf, sizeof(buf), sockIn, &n)) != NULL) {
        #ifdef CONFIG_FEATURE_WGET_AUTHENTICATION
            if (strcasecmp(buf, "WWW-Authenticate") == 0) {
                //unsigned long value;
                //if (safe_strtoul(s, &value));
                // Auth error
                bb_default_error_retval = ATP_TRANS_AUTH_ERR;
                if ((0 == authCnt)          || (authCnt > 2) ||
                    (NULL == target->user)  || (NULL == target->password))
                {
                    bb_error_msg_and_die("auth error");
                }
                authCnt++;
                /*start  of Global AU8D01245 解决http 认证失败问题 by c00131380 at 081121  */
                if (NULL == pcAuthHeader)
                {
                    pcAuthHeader = build_auth_header(s, target, 2);
                }
                /*end  of Global AU8D01245 解决http 认证失败问题 by c00131380 at 081121  */
                bb_default_error_retval = ATP_TRANS_SYS_ERR;
                continue;
            }
        #endif
            if (strcasecmp(buf, "location") == 0) {
                if (s[0] == '/')
                    target->path = bb_xstrdup(s+1);
                else {
                    parse_url(bb_xstrdup(s), target);
                    bb_lookup_host(s_in, target->host);
                    s_in->sin_port = target->port;
                    break;
                }
            }
        }
    } while(status >= 300);
}

static void http_download(struct host_info      *target,
	                         struct sockaddr_in *s_in)
{
    int n, try=5, status;
    char buf[512];
    char *s;
    int got_clen = 0;			/* got content-length: from server	*/
    FILE *localFile;			/* local file						*/
#ifdef CONFIG_FEATURE_WGET_AUTHENTICATION
    char *pcAuthHeader = NULL;
#endif
    int authCnt;

    FILE *sockOut = NULL;			/* socket write to web/ftp server			*/
    FILE *sockIn  = NULL;			/* socket read from web/ftp server			*/

    /*
	 * Open the download local file stream
	 */
	localFile = bb_xfopen(target->localFile, "w");

    authCnt = 0;
    /*
     *  HTTP Download session
     */
    do {
        got_clen = chunked = 0;

        if (! --try)
            close_delete_and_die("too many redirections");

        /*
         * Open socket to http server
         */
        if (sockIn) fclose(sockIn);
        if (sockOut) fclose(sockOut);
        open_socket(s_in, &sockIn, &sockOut);

        /*
         * Send HTTP request.
         */
        fprintf(sockOut, "GET %s HTTP/1.1\r\n", target->path);

        fprintf(sockOut, "Host: %s\r\nUser-Agent: Wget\r\n", target->host);

#ifdef CONFIG_FEATURE_WGET_AUTHENTICATION
        if (NULL != pcAuthHeader) {
            fprintf(sockOut, "Authorization: %s\r\n", pcAuthHeader);
        }
#endif

        fprintf(sockOut, "Connection: close\r\n\r\n");
        fflush(sockOut);


        /*
        * Retrieve HTTP response line and check for "200" status code.
        */
read_response:
        if (fgets(buf, sizeof(buf), sockIn) == NULL)
            close_delete_and_die("no response from server");

        for (s = buf ; *s != '\0' && !isspace(*s) ; ++s)
        ;
        for ( ; isspace(*s) ; ++s)
        ;
        switch (status = atoi(s)) {
            case 0:
            case 100:
                while (gethdr(buf, sizeof(buf), sockIn, &n) != NULL);
                goto read_response;
            case 200:
                break;
            case 300:   /* redirection */
            case 301:
            case 302:
            case 303:
                authCnt = 0;
                break;
            case 404:
                bb_default_error_retval = ATP_TRANS_FILE_ERR;
                close_delete_and_die("404 error");
                break;
            case 401:
                authCnt++;
                break;
                /*FALLTHRU*/
            default:
                chomp(buf);
                close_delete_and_die("server returned error %d: %s", atoi(s), buf);
        }

        /*
         * Retrieve HTTP headers.
         */

        while ((s = gethdr(buf, sizeof(buf), sockIn, &n)) != NULL) {
        #ifdef CONFIG_FEATURE_WGET_AUTHENTICATION
            if (strcasecmp(buf, "WWW-Authenticate") == 0) {
                //unsigned long value;
                //if (safe_strtoul(s, &value));
                // Auth error
                bb_default_error_retval = ATP_TRANS_AUTH_ERR;
                if ((0 == authCnt)          || (authCnt > 2) ||
                    (NULL == target->user)  || (NULL == target->password))
                {
                    close_delete_and_die("auth error");
                }
                authCnt++;
                 /*start  of 解决http 认证失败问题 by s53329 at  20080704  */
                if (NULL == pcAuthHeader)
                {
                /*end  of 解决http 认证失败问题 by s53329 at  20080704  */
                    pcAuthHeader = build_auth_header(s, target, 1);
                }

                bb_default_error_retval = ATP_TRANS_SYS_ERR;
                continue;
            }
        #endif
            if (strcasecmp(buf, "content-length") == 0) {
                unsigned long value;
                if (safe_strtoul(s, &value)) {
                    close_delete_and_die("content-length %s is garbage", s);
                }
                filesize = value;
                got_clen = 1;
                continue;
            }
            if (strcasecmp(buf, "transfer-encoding") == 0) {
                if (strcasecmp(s, "chunked") == 0) {
                    chunked = got_clen = 1;
                } else {
                close_delete_and_die("server wants to do %s transfer encoding", s);
                }
            }
            if (strcasecmp(buf, "location") == 0) {
                if (s[0] == '/')
                    target->path = bb_xstrdup(s+1);
                else {
                    parse_url(bb_xstrdup(s), target);
                    bb_lookup_host(s_in, target->host);
                    s_in->sin_port = target->port;
                    break;
                }
            }
			/* start of HG553_protocal : Added by c65985, 2010.2.27  banner on the page HG553V100R001 */
		    if (1 == atoi(bannerflag))
		    {
		        if (strcasecmp(buf, "Content-Type") == 0) 
				{
				    FILE * fd = NULL;	
				    char acTmp[128] = {0};
					char acFile[128] = {0};
				    char acType[64] = {0};

					if (NULL != strstr(s, "svg") || NULL != strstr(s, "SVG"))
					{
					    strcpy(acType, "svg");
					}
					else if (NULL != strstr(s, "image") || NULL != strstr(s, "IMAGE"))
					{
					    sscanf(s, "%[^/]/%[^;]", acTmp, acType);
					}
					else if (NULL != strstr(s, "flash") || NULL != strstr(s, "application"))
					{
					    strcpy(acType, "swf");
					}
                    else
                    {
                        strcpy(acType, "unk");
                    }
					
					sprintf(bannername, "%s.%s",  target->localFile, acType);
					sscanf(bannername, "/var/%s", acFile);

				    if (fd=fopen("/var/bannertype", "w+")) 
				    {
				        fwrite(acFile, strlen(acFile), 1, fd);
				        fclose(fd);
				    }
                }
		    }
		    /* end of HG553_protocal : Added by c65985, 2010.2.27  banner on the page HG553V100R001 */
        }
    } while(status >= 300);

    /*
	 * Retrieve file
	 */
	if (chunked) {
		fgets(buf, sizeof(buf), sockIn);
		filesize = strtol(buf, (char **) NULL, 16);
	}
#ifdef CONFIG_FEATURE_WGET_STATUSBAR
	progressmeter(-1);
#endif
	do {
		while ((filesize > 0 || !got_clen) && (n = safe_fread(buf, 1, ((chunked || got_clen) && (filesize < sizeof(buf)) ? filesize : sizeof(buf)), sockIn)) > 0) {
			if (safe_fwrite(buf, 1, n, localFile) != n) {
				bb_perror_msg_and_die("write error");
			}
#ifdef CONFIG_FEATURE_WGET_STATUSBAR
			statbytes+=n;
#endif
			if (got_clen) {
				filesize -= n;
			}
		}

		if (chunked) {
			safe_fgets(buf, sizeof(buf), sockIn); /* This is a newline */
			safe_fgets(buf, sizeof(buf), sockIn);
			filesize = strtol(buf, (char **) NULL, 16);
			if (filesize==0) {
				chunked = 0; /* all done! */
			}
		}

		if (n == 0 && ferror(sockIn)) {
			bb_perror_msg_and_die("network read error");
		}
	} while (chunked);
#ifdef CONFIG_FEATURE_WGET_STATUSBAR
	progressmeter(1);
#endif

    fclose(localFile);

    /* start of HG553_protocal : Added by c65985, 2010.2.27  banner on the page HG553V100R001 */
    if (1 == atoi(bannerflag))
    {
        rename(target->localFile, bannername);
    }
	/* end of HG553_protocal : Added by c65985, 2010.2.27  banner on the page HG553V100R001 */
}

#define WGET_OPT_DOWNLOAD	1
#define WGET_OPT_UPLOAD	    2
#define WGET_OPT_QUIET	    4

#define WGET_OPT_CONTINUE	1
#define WGET_OPT_PASSIVE	4
#define WGET_OPT_OUTNAME	8
#define WGET_OPT_HEADER	16
#define WGET_OPT_PREFIX	32
#define WGET_OPT_PROXY	64

static const struct option wget_long_options[] = {
    {"download", 0, NULL, 'g'},
	{"upload",   0, NULL, 's'},
	{"username", 1, NULL, 'u'},
	{"password", 1, NULL, 'p'},
	{"local",    1, NULL, 'l'},
	{"remote",   1, NULL, 'r'},
	{"port",     1, NULL, 'P'},
	{"bind",     1, NULL, 'B'},
	{"addr",     1, NULL, 'A'},
	{0, 0, 0, 0}
};

int wget_main(int argc, char **argv)
{
	unsigned long opt;
	struct host_info target;
	struct sockaddr_in s_in;

    int trans_dir = 0;          /* 0: Error; 1: Download; 2: Upload */
    const char *pcRemotePort    = NULL;
    const char *pcRemoteIp      = NULL;
    
    const char *pcTimeout       = NULL;
    unsigned int timeOut = 0;

    /*
     * Default values
     */
    bb_default_error_retval = ATP_TRANS_SYS_ERR;

    memset((void *)(&target), 0, sizeof(target));
    target.port = 80;

	/*
	 * Crack command line.
	 */
	/*下载的两边路径都是文件名，上传的时候，
    本地是文件名，那边是路径名，例如 test/handy/ 或者 test/handy/*/
	bb_applet_long_options = bb_transtool_long_options;
	/* start of HG553_protocal : Added by c65985, 2010.2.27  banner on the page HG553V100R001 */
	opt = bb_getopt_ulflags(argc, argv, "gsu:p:r:l:P:B:A:D:T:",
                            &target.user,
                            &target.password,
                            &target.path,
                            &target.localFile,
                            &pcRemotePort,
                            &m_pcLocalIP,
                            &pcRemoteIp,
                            &bannerflag,
                            &pcTimeout);
	/* end of HG553_protocal : Added by c65985, 2010.2.27  banner on the page HG553V100R001 */
    target.port = bb_lookup_port(pcRemotePort, "tcp", 80);

	if (opt & WGET_OPT_DOWNLOAD) {
		trans_dir = 1;
	}
    if (opt & WGET_OPT_UPLOAD) {
		trans_dir = 2;
	}
	if ((argc - optind != 1) || (0 == trans_dir) || (NULL == target.localFile))
		bb_show_usage();

    target.host = argv[optind];

	/* We want to do exactly _one_ DNS lookup, since some
	 * sites (i.e. ftp.us.debian.org) use round-robin DNS
	 * and we want to connect to only one IP... */
	if (NULL == pcRemoteIp)
	{
	    bb_lookup_host(&s_in, target.host);
    }
    else
    {
        bb_lookup_host(&s_in, pcRemoteIp);
    }
	s_in.sin_port = target.port;

    if (NULL != pcTimeout)
    {
        timeOut = atoi(pcTimeout);
        /* start of HG553_protocal : Added by c65985, 2010.2.27  banner on the page HG553V100R001 */
        if (1 == atoi(bannerflag))
        {
            alarm(timeOut);
        }
        /* end of HG553_protocal : Added by c65985, 2010.2.27  banner on the page HG553V100R001 */
    }

    if (1 == trans_dir)
        http_download(&target, &s_in);
    else
        http_upload(&target, &s_in);

#ifdef CONFIG_FEATURE_WGET_STATUSBAR
	progressmeter(1);
#endif
    /* start of HG553_protocal : Added by c65985, 2010.2.27  banner on the page HG553V100R001 */
    if (1 == atoi(bannerflag))
    {
        system("echo 1 > /var/bannerflag");
    }
    /* end of HG553_protocal : Added by c65985, 2010.2.27  banner on the page HG553V100R001 */
	exit(ATP_TRANS_OK);
}


void parse_url(char *url, struct host_info *h)
{
	char *cp, *sp, *up, *pp;

	if (strncmp(url, "https://", 8) == 0) {
		h->port = bb_lookup_port("https", "tcp", 443);
		h->host = url + 8;
        h->is_https = 0;
	} else if (strncmp(url, "http://", 7) == 0) {
		h->port = bb_lookup_port("http", "tcp", 80);
		h->host = url + 7;
		h->is_https = 1;
	} else
		bb_error_msg_and_die("not an https or http url: %s", url);

	sp = strchr(h->host, '/');
	if (sp) {
		*sp++ = '\0';
		h->path = sp;
	} else
		h->path = bb_xstrdup("");

	up = strrchr(h->host, '@');
	if (up != NULL) {
		h->user = h->host;
		*up++ = '\0';
		h->host = up;
	} else
		h->user = NULL;

	pp = h->host;

#ifdef CONFIG_FEATURE_WGET_IP6_LITERAL
	if (h->host[0] == '[') {
		char *ep;

		ep = h->host + 1;
		while (*ep == ':' || isxdigit (*ep))
			ep++;
		if (*ep == ']') {
			h->host++;
			*ep = '\0';
			pp = ep + 1;
		}
	}
#endif

	cp = strchr(pp, ':');
	if (cp != NULL) {
		*cp++ = '\0';
		h->port = htons(atoi(cp));
	}
}


int open_socket(struct sockaddr_in *s_in, FILE **fIn, FILE **fOut)
{
	FILE *fp;
    int fd;

    bb_default_error_retval = ATP_TRANS_TIMEOUT;
    fd = xbind_connect(s_in, m_pcLocalIP);
	(*fIn) = fdopen(fd, "r");
    (*fOut) = fdopen(fd, "w");
	if (((*fIn) == NULL) || ((*fOut) == NULL))
		bb_perror_msg_and_die("fdopen()");
    bb_default_error_retval = ATP_TRANS_SYS_ERR;

	return fd;
}


char *gethdr(char *buf, size_t bufsiz, FILE *fp, int *istrunc)
{
	char *s, *hdrval;
	int c;

	*istrunc = 0;

	/* retrieve header line */
	if (fgets(buf, bufsiz, fp) == NULL)
		return NULL;

	/* see if we are at the end of the headers */
	for (s = buf ; *s == '\r' ; ++s)
		;
	if (s[0] == '\n')
		return NULL;

	/* convert the header name to lower case */
	for (s = buf ; isalnum(*s) || *s == '-' ; ++s)
		*s = tolower(*s);
       /*start of 修改问题单AU8D00786 http 认证失败 by s53329  at  20080707
	/* verify we are at the end of the header name */
       /*
	if (*s != ':')
		bb_error_msg_and_die("bad header line: %s", buf);
       end  of 修改问题单AU8D00786 http 认证失败 by s53329  at  20080707*/

	/* locate the start of the header value */
	for (*s++ = '\0' ; *s == ' ' || *s == '\t' ; ++s)
		;
	hdrval = s;

	/* locate the end of header */
	while (*s != '\0' && *s != '\r' && *s != '\n')
		++s;

	/* end of header found */
	if (*s != '\0') {
		*s = '\0';
		return hdrval;
	}

	/* Rats!  The buffer isn't big enough to hold the entire header value. */
	while (c = getc(fp), c != EOF && c != '\n')
		;
	*istrunc = 1;
	return hdrval;
}

#ifdef CONFIG_FEATURE_WGET_STATUSBAR
/* Stuff below is from BSD rcp util.c, as added to openshh.
 * Original copyright notice is retained at the end of this file.
 *
 */


static int
getttywidth(void)
{
	int width=0;
	get_terminal_width_height(0, &width, NULL);
	return (width);
}

static void
updateprogressmeter(int ignore)
{
	int save_errno = errno;

	progressmeter(0);
	errno = save_errno;
}

static void
alarmtimer(int wait)
{
	struct itimerval itv;

	itv.it_value.tv_sec = wait;
	itv.it_value.tv_usec = 0;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}


static void
progressmeter(int flag)
{
	static const char prefixes[] = " KMGTP";
	static struct timeval lastupdate;
	static off_t lastsize, totalsize;
	struct timeval now, td, wait;
	off_t cursize, abbrevsize;
	double elapsed;
	int ratio, barlength, i, remaining;
	char buf[256];

	if (flag == -1) {
		(void) gettimeofday(&start, (struct timezone *) 0);
		lastupdate = start;
		lastsize = 0;
		totalsize = filesize; /* as filesize changes.. */
	}

	(void) gettimeofday(&now, (struct timezone *) 0);
	cursize = statbytes;
	if (totalsize != 0 && !chunked) {
		ratio = 100.0 * cursize / totalsize;
		ratio = MAX(ratio, 0);
		ratio = MIN(ratio, 100);
	} else
		ratio = 100;

	snprintf(buf, sizeof(buf), "\r%-20.20s %3d%% ", curfile, ratio);

	barlength = getttywidth() - 51;
	if (barlength > 0) {
		i = barlength * ratio / 100;
		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
			 "|%.*s%*s|", i,
			 "*****************************************************************************"
			 "*****************************************************************************",
			 barlength - i, "");
	}
	i = 0;
	abbrevsize = cursize;
	while (abbrevsize >= 100000 && i < sizeof(prefixes)) {
		i++;
		abbrevsize >>= 10;
	}
	snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), " %5d %c%c ",
	     (int) abbrevsize, prefixes[i], prefixes[i] == ' ' ? ' ' :
		 'B');

	timersub(&now, &lastupdate, &wait);
	if (cursize > lastsize) {
		lastupdate = now;
		lastsize = cursize;
		if (wait.tv_sec >= STALLTIME) {
			start.tv_sec += wait.tv_sec;
			start.tv_usec += wait.tv_usec;
		}
		wait.tv_sec = 0;
	}
	timersub(&now, &start, &td);
	elapsed = td.tv_sec + (td.tv_usec / 1000000.0);

	if (wait.tv_sec >= STALLTIME) {
		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
			 " - stalled -");
	} else if (statbytes <= 0 || elapsed <= 0.0 || cursize > totalsize || chunked) {
		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
			 "   --:-- ETA");
	} else {
		remaining = (int) (totalsize / (statbytes / elapsed) - elapsed);
		i = remaining / 3600;
		if (i)
			snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
				 "%2d:", i);
		else
			snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
				 "   ");
		i = remaining % 3600;
		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
			 "%02d:%02d ETA", i / 60, i % 60);
	}
	write(STDERR_FILENO, buf, strlen(buf));

	if (flag == -1) {
		struct sigaction sa;
		sa.sa_handler = updateprogressmeter;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = SA_RESTART;
		sigaction(SIGALRM, &sa, NULL);
		alarmtimer(1);
	} else if (flag == 1) {
		alarmtimer(0);
		statbytes = 0;
		putc('\n', stderr);
	}
}
#endif

/* Original copyright notice which applies to the CONFIG_FEATURE_WGET_STATUSBAR stuff,
 * much of which was blatantly stolen from openssh.  */

/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. <BSD Advertising Clause omitted per the July 22, 1999 licensing change
 *		ftp://ftp.cs.berkeley.edu/pub/4bsd/README.Impt.License.Change>
 *
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING ANY WAY
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$Id: wget.c,v 1.4 2010/11/22 06:15:53 l43571 Exp $
 */



/*
Local Variables:
c-file-style: "linux"
c-basic-offset: 4
tab-width: 4
End:
*/
