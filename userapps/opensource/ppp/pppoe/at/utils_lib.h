/***********************************************************************
  ��Ȩ��Ϣ : ��Ȩ����(C) 1988-2005, ��Ϊ�������޹�˾.
  �ļ���   : utils_lib.h
  ����     : lichangqing
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

#ifndef __UTILS_H
#define __UTILS_H

//#include <linux/sem.h>
#include <sched.h>

#include <sys/time.h> 
#include <signal.h> 

// ����Ϊ��λ�ĵȴ�����
int __msleep(int millisecond);


// wait����
typedef struct
{
    char* key;
} WAIT_S;

int __init_wait(WAIT_S *wait, const char* key);
int __destroy_wait(WAIT_S *wait);
int __sleep_wait(WAIT_S* wait,int millisecond);
int __wakeup(WAIT_S *wait);


// mutex ����

/*
typedef pthread_mutex_t MUTEX_S;
void __init_mutex(MUTEX_S *mutex);
void __destroy_mutex(MUTEX_S*);
void __p(MUTEX_S *mutex);
void __v(MUTEX_S *mutex);
*/
#if 0

typedef struct mutex {
    int mutexid;
    struct sembuf p_sem;
    struct sembuf v_sem;
} MUTEX_S;


void __init_mutex(MUTEX_S *mutex, char* key, int key2);
void __destroy_mutex(MUTEX_S*);
void __p(MUTEX_S *mutex);
void __v(MUTEX_S *mutex);
#endif
// ��ʱ������
typedef struct 
{
    struct sigaction new_sigalrm;
    struct sigaction old_sigalrm;
    struct itimerval new_timer;
    struct itimerval old_timer;
} AT_TIMER_S;
typedef void (*timer_callback_t)(int k);

int __start_timer(AT_TIMER_S* timer, int millisecond,  
	void (*timer_handler)(int));
int __stop_timer(AT_TIMER_S *timer);


// �߳�����
typedef struct
{
    int thread_id;
    int (*thread_fn)(void *arg);
    void* arg;
    void* stack_p;
    void * stack;
#if 0
    unsigned int stack_size;   // Ĭ��64k
#endif
} THREAD_S;

int __init_thread_t(THREAD_S * pthread, int (*thread_fn)(void *arg), void* arg
#if 0
, void* stack, unsigned int stack_size
#endif
);
int __start_thread(THREAD_S *pthread_t);
int __destroy_thread_t(THREAD_S *pthread_t);

#if 0//del by sxg, no interfaces
// zhouxuezhong �ṩ�Ŀ⺯������
extern void cm500_poweron();
extern void cm500_poweroff();
extern void rssi_ledshow (unsigned int value);
extern unsigned int bcm_powerstatus();
extern void wan_ledon(void);
extern void wan_ledoff(void);
#endif//0
// ȫ�ֱ���
extern THREAD_S g_ppp_thread;          // PPP�߳������ṹ
extern THREAD_S g_st_at_report_thread; // AT�ϱ������߳������ṹ

#endif // __UTILS_H
