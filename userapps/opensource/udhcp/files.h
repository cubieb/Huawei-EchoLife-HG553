/* files.h */
#ifndef _FILES_H
#define _FILES_H

struct config_keyword {
	char keyword[20];
	int (*handler)(char *line, void *var);
	void *var;
	char def[40];
};


int read_config(char *file);
void write_leases(int dummy);
void read_leases(char *file);
/*start DHCP Server支持第二地址池, s60000658, 20060616*/
void write_leases2(int dummy);
void read_leases2(char *file);
/*end DHCP Server支持第二地址池, s60000658, 20060616*/
// BRCM
int read_vendor_id_config(char *file);
#ifdef VDF_OPTION
//add for option125
void write_viTable();
#endif

/*start of HG553 2009.05.26 V100R001C02B052 AU8D02419  */
void setIPPhoneNtpCfg(int sig);
/*end of HG553 2009.05.26 V100R001C02B052 AU8D02419  */
#endif
