/***********************************************************************
  ��Ȩ��Ϣ : ��Ȩ����(C) 1988-2005, ��Ϊ�������޹�˾.
  �ļ���   : pppd_thread.h
  ����     : lichangqing 45517
  �汾     : V500R003
  �������� : 2005-8-8
  ������� : 2005-8-10
  �������� : ͷ�ļ�
      
  ��Ҫ�����б�: ��
      
  �޸���ʷ��¼�б�: 
    <��  ��>    <�޸�ʱ��>  <�汾>  <�޸�����>
    l45517      20050816    0.0.1    ��ʼ���
  ��ע: 
************************************************************************/

#ifndef __PPPD_THREAD_H
#define __PPPD_THREAD_H

//A064D00348 qinzhiyuan begin
//int pppd_thread_initialize(int args,char** argv);
int pppd_thread_initialize(int args, const char** argv);
//A064D00348 qinzhiyuan end

int pppd_thread_destroy();

int pppd_thread_at_dail(int modem_fd);
int pppd_thread_ath(int modem_fd);

#endif //PPPD_THREAD_H

