/* files.h */
#ifndef _AT_FILES_H_
#define _AT_FILES_H_

#define ATCFG_BUF_LEN                          300
#define ATCFG_PROFILE                          "/var/at_profile"

/*��Ӫ��ѡ��ʽ*/
enum
{
	ATCFG_OPTOR_AUTO   = 0,                     //���Զ�ѡ����Ӫ��
	ATCFG_OPTOR_MANUAL                          //��ѡ����Ӫ��
};

/*��������*/
enum
{
    ATCFG_CONNTYPE_GPRS_FIRST = 0,              //GPRS����
    ATCFG_CONNTYPE_3G_FIRST,                    // 3G����
    ATCFG_CONNTYPE_GPRS_ONLY,                   //����ʹ��GPRS
    ATCFG_CONNTYPE_3G_ONLY,                       //����ʹ��3G
    ATCFG_CONNTYPE_AUTO,                            //�Զ�
};

/*ͨ��ѡ������*/
/*
enum
{
    ATCFG_CHANNEL_UNLIMITED = 0,                //ͨ��������
    ATCFG_CHANNEL_GSM900_GSM1800_WCDMA2100,   //GSM900/GSM1800/WCDMA2100
    ATCFG_CHANNEL_GSM1900                        //GSM1900
};
*/


struct config_keyword {
	char *keyword;
	int (*handler)(char *line, void *var);
	void *var;
	char *def;
};

int read_config(char *file);
#endif
