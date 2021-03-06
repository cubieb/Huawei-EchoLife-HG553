/* vi: set sw=4 ts=4: */
/*
 * Utility routines.
 *
 * Connect to host at port using address resolution from getaddrinfo
 *
 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "libbb.h"

/* Return network byte ordered port number for a service.
 * If "port" is a number use it as the port.
 * If "port" is a name it is looked up in /etc/services, if it isnt found return
 * default_port
 */
unsigned short bb_lookup_port(const char *port, const char *protocol, unsigned short default_port)
{
	unsigned short port_nr = htons(default_port);
	if (port) {
		char *endptr;
		int old_errno;
		long port_long;

		/* Since this is a lib function, we're not allowed to reset errno to 0.
		 * Doing so could break an app that is deferring checking of errno. */
		old_errno = errno;
		errno = 0;
		port_long = strtol(port, &endptr, 10);
		if (errno != 0 || *endptr!='\0' || endptr==port || port_long < 0 || port_long > 65535) {
			struct servent *tserv = getservbyname(port, protocol);
			if (tserv) {
				port_nr = tserv->s_port;
			}
		} else {
			port_nr = htons(port_long);
		}
		errno = old_errno;
	}
	return port_nr;
}

void bb_lookup_host(struct sockaddr_in *s_in, const char *host)
{
	struct hostent *he;

	memset(s_in, 0, sizeof(struct sockaddr_in));
	s_in->sin_family = AF_INET;
	he = xgethostbyname(host);
	memcpy(&(s_in->sin_addr), he->h_addr_list[0], he->h_length);
}

int xconnect(struct sockaddr_in *s_addr)
{
	int s = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(s, (struct sockaddr_in *)s_addr, sizeof(struct sockaddr_in)) < 0)
	{
		bb_perror_msg_and_die("Unable to connect to remote host (%s)",
				inet_ntoa(s_addr->sin_addr));
	}
	return s;
}

int xbind_connect(struct sockaddr_in *s_addr, const char *ip)
{
    struct sockaddr_in stBindAddr;
     struct timeval tv;

	int s = socket(AF_INET, SOCK_STREAM, 0);
    /* HUAWEI HGW s53329 2008年6月5日" 通过TR69升级软件出现异常后升级灯不能恢复成ADSL正常状态 add begin:*/
    tv.tv_sec = 20;
    tv.tv_usec = 0;
    setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
    setsockopt(s,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv));
    /* HUAWEI HGW s53329 2008年6月5日" 通过TR69升级软件出现异常后升级灯不能恢复成ADSL正常状态 add end.*/

    if (NULL != ip)
    {
        memset((void *)(&stBindAddr), 0, sizeof(stBindAddr));
        stBindAddr.sin_family = AF_INET;
        stBindAddr.sin_addr.s_addr = inet_addr(ip);
        //stBindAddr.sin_port = htons(9092);
        bind(s, (struct sockaddr *)(&stBindAddr), sizeof(stBindAddr));
    }

	if (connect(s, (struct sockaddr_in *)s_addr, sizeof(struct sockaddr_in)) < 0)
	{
		bb_perror_msg_and_die("Unable to connect to remote host (%s)",
				inet_ntoa(s_addr->sin_addr));
	}
	return s;
}

