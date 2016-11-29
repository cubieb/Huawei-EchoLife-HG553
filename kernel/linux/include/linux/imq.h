#ifndef _IMQ_H
#define _IMQ_H

#define IMQ_MAX_DEVS   16

#define IMQ_F_IFMASK   0x0f
#define IMQ_F_ENQUEUE  0x80
//#define QOS_DEFAULT_MARK  0x100
#define QOS_DEV_IMQ0      0x0
#define QOS_DEV_IMQ1      0x1
#define QOS_DEV_IMQ2      0x2
#define QOS_DEV_IMQ3      0x3
/* start no queue marked packets to default queue */
#define QOS_PRIORITY_DEFAULT   0x80
/* end no queue marked packets to default queue */
extern int qos_enable;

/*队列优先级*/
typedef enum 
{
    EN_QOS_EF = 1,
    EN_QOS_AF1,
    EN_QOS_AF2,
    EN_QOS_BE,
    EN_QOS_AF3,
    EN_QOS_AF4
}QOS_QUEUE_MARK;

/*atm队列优先级*/
#define QOS_PRIORITY_LOW        1
#define QOS_PRIORITY_MED        2
#define QOS_PRIORITY_HIGH       3
#define QOS_PRIORITY_HIGHEST    4


#define IS_ETN_WAN(name)  (('n' == name[0]) && ('0' == name[3]))

extern int imq_nf_queue(struct sk_buff *skb);
#endif /* _IMQ_H */
