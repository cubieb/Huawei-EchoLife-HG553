/***********************************************************************
  ��Ȩ��Ϣ : ��Ȩ����(C) 1988-2005, ��Ϊ�������޹�˾.
  �ļ���   : utils_lib.c
  ����     : lichangqing
  �汾     : V500R003
  �������� : 2005-8-8
  ������� : 
  �������� : 
      
  ��Ҫ�����б�: 
      
  �޸���ʷ��¼�б�: 
    <��  ��>    <�޸�ʱ��>  <�汾>  <�޸�����>
    l45517      20050816    0.0.1    ��ʼ���
  ��ע: 
************************************************************************/

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>

#include "pppd.h"
#include "utils_lib.h"

// A64D01145 ADD (by l45517 2006��5��12�� ) BEGIN
#include "at_thread.h"
extern int g_main_thread_killed;
// A64D01145 ADD (by l45517 2006��5��12�� ) END

/*------------------------------------------------------------
  ����ԭ�� : int __msleep(int millisecond )
  ����     : ʵ���Ժ���λ��λ�Ĳ������ѵĵȴ�
  ����     : millisecond(˯��ʱ��)
  ���     : ��
  ����ֵ   : 0 �ɹ�
-------------------------------------------------------------*/
int __msleep(int millisecond) 
{
    int ret;
    struct timeval tvSelect;
    
    tvSelect.tv_sec  = millisecond / 1000;
    tvSelect.tv_usec = (millisecond % 1000) * 1000;

#ifdef DEBUG_UTILS
    struct timeval tm;
    gettimeofday(&tm, (struct timezone *)NULL);
#endif
    UTILS_DEBUG("__usleep begin time is : sec = %d, ms = %d, us = %d ", 
        tm.tv_sec, (tm.tv_usec / 1000), (tm.tv_usec % 1000));

    while (1)
    {
        ret = select(0, (fd_set *)0, (fd_set *)0, (fd_set *)0,
        	(struct timeval*)&tvSelect);
        if (ret < 0)
        {
            if (EINTR == errno)
            {
                continue;
            }
            FATAL("msleep : select error \n");
        }
        break;
    }
    
#ifdef DEBUG_UTILS
    gettimeofday(&tm, (struct timezone *)NULL);
#endif
    UTILS_DEBUG("__usleep end   time is : sec = %d, ms = %d, us = %d ", 
        tm.tv_sec, (tm.tv_usec / 1000), (tm.tv_usec % 1000));

    return 0;
}


/*------------------------------------------------------------
  ����ԭ�� : int __init_wait(WAIT_S *wait, const char* key)
  ����     : ��ʼ��һ��wait_t�ṹ
  ����     : wait(��ʼ���Ķ���); key(wait�Ĺؼ���)
  ���     : ��
  ����ֵ   : 0 �ɹ�
-------------------------------------------------------------*/
int __init_wait(WAIT_S *wait, const char* key)
{
    int ret;

    // ����fifo
    unlink(key);
    if ((ret = mkfifo(key, 
    	(O_CREAT | O_TRUNC | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP))) < 0)
    {
        if (ret != EEXIST)
        {
            FATAL("cannot create fifoserver key = %s \n", key);
        }
    }

    // ����fifo�ؼ���
    ret = strlen(key) + 1;
    wait->key = (char*)malloc(ret);
    if (!wait->key)
    {
        FATAL("malloc failed \n");
    }
    
    strcpy(wait->key, key);
    return 0;
}


/*------------------------------------------------------------
  ����ԭ�� : int __destroy_wait(WAIT_S *wait)
  ����     : ע��һ��wait_t�ṹ
  ����     : wait(��ע����ָ��)
  ���     : ��
  ����ֵ   : 0 �ɹ�
-------------------------------------------------------------*/
int __destroy_wait(WAIT_S *wait)
{
    // ȡ��fifo
    unlink(wait->key);
    free(wait->key);
    return 0;
}


/*------------------------------------------------------------
  ����ԭ�� : int __sleep_wait(WAIT_S* wait, int millisecond)
  ����     : ��wait��ʵ�ֵȴ�����
  ����     : wait(�ȴ��ṹ); millisecond(��ʱʱ��, ��λ����)
  ���     : ��
  ����ֵ   : 1 �ɹ���0 ��ʱ
-------------------------------------------------------------*/
int __sleep_wait(WAIT_S* wait, int millisecond)
{
    int    i_ret;
    char   ch[4];
    int    fifo_fd;
    fd_set read_set;
    struct timeval tvSelect;

#ifdef DEBUG_UTILS
	struct timeval tm;
#endif

    // ��fifo
    fifo_fd = open(wait->key, O_RDONLY | O_NONBLOCK);
    if (fifo_fd < 0)
    {
        ERROR("fifo : open failed<%s>", wait->key);
        return 1;
    }

    // ���õȴ�ʱ��
    tvSelect.tv_sec  = millisecond / 1000;
    tvSelect.tv_usec = (millisecond % 1000) * 1000;

    // select fifo �ȴ�����
#ifdef DEBUG_UTILS
    gettimeofday(&tm, (struct timezone *)NULL);
#endif

    UTILS_DEBUG("sleep time is : sec = %d, ms = %d, us = %d ", 
        tm.tv_sec, (tm.tv_usec / 1000), (tm.tv_usec % 1000));
    
    while (1) {
        FD_ZERO(&read_set);
        FD_SET(fifo_fd, &read_set);
        i_ret = select(fifo_fd + 1, &read_set, NULL, NULL, &tvSelect);
        if (i_ret < 0)
        {
            // A64D01038 ADD (by l45517 2006��4��10�� ) BEGIN
            if (EINTR == errno) 
            {
                extern int g_main_thread_killed;
                if (g_main_thread_killed)
                {
                    return i_ret;
                }
                UTILS_DEBUG("fifo : select EINTR");
                continue;
            }
            // A64D01038 ADD (by l45517 2006��4��10�� ) END
            ERROR("fifo : select fifo error  \n");
            close(fifo_fd);
            return i_ret;
        }

        if (!i_ret)
        {
#ifdef DEBUG_UTILS
            gettimeofday(&tm, (struct timezone *)NULL);
            UTILS_DEBUG("timeout time is : sec = %d, ms = %d, us = %d ", 
                tm.tv_sec, (tm.tv_usec / 1000), (tm.tv_usec % 1000));
#endif
            close(fifo_fd);
            return 0; //��ʱ����
        }
        break;
    }

#ifdef DEBUG_UTILS
    gettimeofday(&tm, (struct timezone *)NULL);
    UTILS_DEBUG("wakeup time is : sec = %d, ms = %d, us = %d ", 
        tm.tv_sec, (tm.tv_usec / 1000), (tm.tv_usec % 1000));
#endif

    // i_ret����0����ʾ�̱߳�����
    i_ret = read(fifo_fd, ch, 4);
    if (4 != i_ret)
    {
        WARN("fifo : have not read enougn bytes \n");
    }

    close(fifo_fd);
    return 1;
}


/*------------------------------------------------------------
  ����ԭ�� : int __wakeup(WAIT_S *wait)
  ����     : ������wait�ϵȴ����߳�
  ����     : wait(�ȴ��Ľṹ)
  ���     : ��
  ����ֵ   : 0 �ɹ�
-------------------------------------------------------------*/
int __wakeup(WAIT_S *wait)
{
    int i_ret;
    int fifo_fd;
    char tmp[4] = {'A','T','S','B'};
    fd_set  write_set;
    struct  timeval tvSelect;

    // ��fifo
    fifo_fd = open(wait->key, (O_WRONLY | O_NONBLOCK), 0);

    if (fifo_fd < 0)
    {
        if (ENXIO == errno)
        {
            WARN("fifo : no thread waiting");
        }
        else
        {
            ERROR("fifo : open failed<%s>", wait->key);
        }
        return 0;
    }
    
    tvSelect.tv_sec  = 1; // �ȴ�1�볬ʱ
    tvSelect.tv_usec = 0;
    UTILS_DEBUG("fifo : opened key = <%s>", wait->key);


    // �ȴ�fifo��д
    /*while (1) {
        FD_ZERO(&write_set);
        FD_SET(fifo_fd, &write_set);
        i_ret = select(fifo_fd + 1, NULL, &write_set, NULL, &tvSelect);
        if (i_ret < 0)
        {
            if (EINTR == errno)
            {
                continue;
            }
            error("[%s : %d] select fifo error \n", __FILE__, __LINE__);
            close(fifo_fd);
            return 0;
        }
        
        if (0 == i_ret) // ��һ����FIFO������д��˵����ʱ
        {
            warn("[%s : %d] select fifo timeout \n", __FILE__, __LINE__);
            close(fifo_fd);
            return 0;
        }
    }*/

    // ���ѵȴ��߳�
    UTILS_DEBUG("fifo wake up waiting thread");

    while(1) {
        i_ret = write(fifo_fd, tmp, sizeof(tmp));
        if (i_ret < 0)
        {
            if (EINTR == errno)
            {
                continue;
            }
            
            if (EPIPE == errno)
            {
                WARN("fifo : no thread waiting \n");
            }
            else 
            {
                ERROR("fifo : write fifo failed \n");
            }
            
            close(fifo_fd);
            return 0;
        }

        // д��Ϊ0��������д
        if (0 == i_ret)
        {
            UTILS_DEBUG("fifo write failed, retrying");
            continue; 
        }
        
        break;
    }
    
    UTILS_DEBUG("fifo wake up waiting thread successfully");

    if (sizeof(tmp) != i_ret)
    {
        WARN("fifo : do not write enough bytes\n");
    }

    close(fifo_fd);
    return 1;
}

#if 0
/*------------------------------------------------------------
  ����ԭ�� : void __init_mutex(MUTEX_S * mutex)
  ����     : ��ʼ��һ��mutex�ṹ
  ����     : mutex(����ʼ����ָ��); key_str(�ź������ַ����ؼ�); key2(�ؼ���2)
  ���     : ��
  ����ֵ   : ��
-------------------------------------------------------------*/
void __init_mutex(MUTEX_S * mutex, char* key, int key2)
{
    key_t key_val;
    union semun un;

    UTILS_DEBUG("key = %s, key = %d", key, key2);
    key_val = ftok((const char*)key, key2);
    UTILS_DEBUG("ftok key = %d", key_val);

    mutex->mutexid = semget(key_val, 1,	IPC_CREAT |	00666);
    if (mutex->mutexid == -1)
    {
        ERROR("mutex : get semaphore error");
        // A64D01145 ADD (by l45517 2006��5��10�� ) BEGIN
        g_main_thread_killed = AT_MAIN_EXITED;
        return;
        // A64D01145 ADD (by l45517 2006��5��10�� ) END
    }

    un.val = 1;
    semctl(mutex->mutexid, 0, SETVAL, un);

    mutex->p_sem.sem_num = 0;
    mutex->p_sem.sem_op  = -1;
    mutex->p_sem.sem_flg = 0;

    mutex->v_sem.sem_num = 0;
    mutex->v_sem.sem_op  = 1;
    mutex->v_sem.sem_flg = 0;
    return;
}


/*------------------------------------------------------------
  ����ԭ�� : __destroy_mutex(MUTEX_S* mutex)
  ����     : ע��һ��mutex
  ����     : mutex(ע���Ľṹ��ָ��)
  ���     : ��
  ����ֵ   : ��
-------------------------------------------------------------*/
void  __destroy_mutex(MUTEX_S* mutex)
{
    if (semctl(mutex->mutexid, 0, IPC_RMID) < 0)
    {
        ERROR("mutex : destory semaphore error \n");
    }
}


/*------------------------------------------------------------
  ����ԭ�� : void __p(MUTEX_S *mutex)
  ����     : mutex��pԭ��
  ����     : mutex(�����Ľṹ)
  ���     : ��
  ����ֵ   : ��
-------------------------------------------------------------*/
void __p(MUTEX_S *mutex)
{
    if(-1 == semop(mutex->mutexid, &mutex->p_sem, 1))
    {
        if (EINVAL == errno)
        {
            ERROR("mutex : v semaphore error(semaphore do not exit) \n");
            // A64D01145 ADD (by l45517 2006��5��10�� ) BEGIN
            g_main_thread_killed = AT_MAIN_EXITED;
            return;
            // A64D01145 ADD (by l45517 2006��5��10�� ) END
        }
		ERROR("mutex : p semaphore error \n");
        if (-1 == semctl(mutex->mutexid, 0, IPC_RMID))
        {
            ERROR("mutex : rm semaphore error \n");
        }
        // A64D01145 ADD (by l45517 2006��5��10�� ) BEGIN
        g_main_thread_killed = AT_MAIN_EXITED;
        return;
        // A64D01145 ADD (by l45517 2006��5��10�� ) END
    }
}


/*------------------------------------------------------------
  ����ԭ�� : void __v(MUTEX_S *mutex)
  ����     : mutex��vԭ��
  ����     : mutex(�����Ľṹ)
  ���     : ��
  ����ֵ   : ��
-------------------------------------------------------------*/
void __v(MUTEX_S *mutex)
{
    if(-1 == semop(mutex->mutexid, &mutex->v_sem, 1))
    {
        if (EINVAL == errno)
        {
            ERROR("mutex : v semaphore error(semaphore do not exit) \n");
            // A64D01145 ADD (by l45517 2006��5��10�� ) BEGIN
            g_main_thread_killed = AT_MAIN_EXITED;
            return;
            // A64D01145 ADD (by l45517 2006��5��10�� ) END
        }
		ERROR("mutex : v semaphore error \n");
        if (-1 == semctl(mutex->mutexid, 0, IPC_RMID))
        {
            ERROR("mutex : rm semaphore error \n");
        }
        // A64D01145 ADD (by l45517 2006��5��10�� ) BEGIN
        g_main_thread_killed = AT_MAIN_EXITED;
        return;
        // A64D01145 ADD (by l45517 2006��5��10�� ) END
    }
}
#endif

/*------------------------------------------------------------
  ����ԭ�� : int __start_timer(AT_TIMER_S* timer, int millisecond,  void (*timer_handler)(int))
  ����     : ����һ����ʱ��
  ����     : timer(��ʱ���ṹ); millisecond(��ʱ��ʱ��); timer_handler(��ʱ���ص�����)
  ���     : ��
  ����ֵ   : 0(�ɹ�)
-------------------------------------------------------------*/
static int g_is_timer_started = 0;

int __start_timer(AT_TIMER_S* timer, int millisecond,  
	void (*timer_handler)(int))
{
    sigset_t mask;
    
    if (g_is_timer_started)
    {
        WARN("timer : started \n");
        return SUCCESS;
    }

    g_is_timer_started = 1;

    sigemptyset(&mask);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGCHLD);
    sigaddset(&mask, SIGUSR2);

    timer->new_sigalrm.sa_handler = timer_handler;
    // sigemptyset(&timer->new_sigalrm.sa_mask);
    timer->new_sigalrm.sa_mask  = mask;
    timer->new_sigalrm.sa_flags = 0;

    // ����SIGALRM�źŵ��źŴ�����
    sigaction(SIGALRM, &timer->new_sigalrm, &timer->old_sigalrm);

    // ���ö�ʱ��ʱ��
    timer->new_timer.it_interval.tv_sec  = millisecond / 1000;
    timer->new_timer.it_value   .tv_sec  = millisecond / 1000;
    timer->new_timer.it_interval.tv_usec = millisecond % 1000 * 1000;
    timer->new_timer.it_value   .tv_usec = millisecond % 1000 * 1000;

    // ������ʱ��
    setitimer(ITIMER_REAL, &timer->new_timer, &timer->old_timer);

    UTILS_DEBUG("cm500 shakehand timer started");
    return SUCCESS;
}


/*------------------------------------------------------------
  ����ԭ�� : int __stop_timer(AT_TIMER_S *timer)
  ����     : ȡ����ʱ��
  ����     : timer(��ʱ���ṹ)
  ���     : ��
  ����ֵ   : 0(�ɹ�)
-------------------------------------------------------------*/
int __stop_timer(AT_TIMER_S *timer)
{
    if (!g_is_timer_started)
    {
        // INFO("timer : has not started\n");
        return 0;
    }

    g_is_timer_started = 0;
    setitimer(ITIMER_REAL, &timer->old_timer,   NULL);
    sigaction(SIGALRM,     &timer->old_sigalrm, NULL);
    INFO("timer : has stopped\n");
    return 0;
}


/*------------------------------------------------------------
  ����ԭ�� : int __init_thread_t(THREAD_S * pthread, int (*thread_fn)(void *arg), void* arg
  ����     : ��ʼ���߳̽ṹ
  ����     : ��
  ���     : ��
  ����ֵ   : 1(�ɹ�)��0(ʧ��)
-------------------------------------------------------------*/
int __init_thread_t(THREAD_S * pthread, int (*thread_fn)(void *arg), void* arg
#if 0
, void* stack, unsigned int stack_size
#endif
)
{
    int * t;
     //printf("Caoxiang :%s : %s()���̺�:%d\r\n",__FILE__,__FUNCTION__,  getpid());
    // assert(pthread, "the pthread is null.");
    // assert(thread_fn,    "the thread_fn is   null.");
    pthread->thread_fn = thread_fn;
    pthread->arg = arg;
    t = malloc(sizeof(int) * 1024   * 16);
    if (!t)
    {
        FATAL("thread : molloc THREAD_S failed \n");
        return FAILED;
    }
    pthread->stack_p = t;
    pthread->stack = &t[1024 * 16   - 1];   
    return SUCCESS;
}


/*------------------------------------------------------------
  ����ԭ�� : int __start_thread(THREAD_S *pthread)
  ����     : �����߳�
  ����     : pthread(Ҫ�������̵߳Ľṹ)
  ���     : ��
  ����ֵ   : �̺߳�
-------------------------------------------------------------*/
int __start_thread(THREAD_S *pthread)
{
    int flag = CLONE_VM | CLONE_FS | CLONE_FILES | SIGCHLD;
     //printf("Caoxiang :%s : %s()���̺�:%d\r\n",__FILE__,__FUNCTION__,  getpid());
    return pthread->thread_id = 
    	clone(pthread->thread_fn, pthread->stack, flag, pthread->arg);  
}


/*------------------------------------------------------------
  ����ԭ�� : int __destroy_thread_t(THREAD_S *pthread_t)
  ����     : �߳̽ṹע��
  ����     : pthread(ע�����̵߳Ľṹ)
  ���     : ��
  ����ֵ   : 0 (�ɹ�)
-------------------------------------------------------------*/
int __destroy_thread_t(THREAD_S *pthread)
{
    if (pthread->stack_p)
    {
        free(pthread->stack_p);
        pthread->stack = NULL;
    }
    return 0;
}


/*------------------------------------------------------------
  ����ԭ�� : int get_thread_esp(int* esp_val)
  ����     : ��ȡ�̵߳ĵ�ǰ��ջָ��
  ����     : esp_val : local����ָ��
  ���     : ��
  ����ֵ   : �̶߳�ջָ��
-------------------------------------------------------------*/
int* __get_thread_esp(int* esp_val)
{

// ��ദ�������
#if 0
    void* p_esp;

    __asm__ __volatile__(
        ".set push \n"
    	".set noat \n"
    	"mov %0, $29"
        ".set pop \n"
        :"=r" (p_esp)
    );

    return p_esp;
#else

    return esp_val;

#endif
}


/*------------------------------------------------------------
  ����ԭ�� : int* __errno_location()
  ����     : ����c�⺯����֧�ֶ��̵߳ĳ������errno,�ú�������ŵ��ļ��Ľ�β
  ����     : esp_val : local����ָ��
  ���     : ��
  ����ֵ   : �̶߳�ջָ��
-------------------------------------------------------------*/
#undef errno

int g_main_errno = 0;
int g_ppp_errno  = 0;
int g_at_errno   = 0;

int* __errno_location()
{

    int esp_test;
    void* p_esp_val;
   
    p_esp_val = __get_thread_esp(&esp_test);

    if ((p_esp_val <= g_ppp_thread.stack  ) &&
        (p_esp_val >= g_ppp_thread.stack_p))
    {
        return &g_ppp_errno;
    }

    if ((p_esp_val <= g_st_at_report_thread.stack  ) &&
        (p_esp_val >= g_st_at_report_thread.stack_p))
    {
        return &g_at_errno;
    }

    return &g_main_errno;	
}
