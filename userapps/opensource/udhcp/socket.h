/* socket.h */
#ifndef _SOCKET_H
#define _SOCKET_H
int read_interface(char *interface, int *ifindex, u_int32_t *addr, unsigned char *arp);
int listen_socket(unsigned int ip, int port, char *inf);
/* start of HG553_protocal : Added by c65985, 2009.12.16  bug112 of ip phone arp HG553V100R001 */
int dhcpd_listen_socket(unsigned int ip, int port, char *inf);
/* end of HG553_protocal : Added by c65985, 2009.12.16  bug112 of ip phone arp HG553V100R001 */
int raw_socket(int ifindex);

#endif
