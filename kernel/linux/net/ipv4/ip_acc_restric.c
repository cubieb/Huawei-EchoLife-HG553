
/*
接入用户限制 : 判断lan侧来的报文是否应该转发到wan侧
  1、接入用户总数限制
  2、按lan侧设备类型进行用户数限制
  3、接入总数与按设备类型限制的数量取交集进行限制
  4、如果总数未限制则仅按设备类型限制
  5、如果按设备类型未限制则按总数限制

                            ---------------
                            |             |--------------
                            |  Restrict   |             |
                            |             |<---------   |
                            ---------------         |   |
                      allow  ->|      |<- crnt      |   |
                               |      |             |   |
                               V      V             |   V
                               ========          ---------
                               | dev  | -------->| type  |
                               ========          ---------
                                      |
                                      |      
                                      V
                               ========
                               | dev  |
                               ========
  
*/

#include <linux/ip.h>
#include <net/ip.h>
#include <net/ip_acc_restric.h>
#include <linux/if_arp.h>
#ifdef CONFIG_ATP_BRIDGE_BIND
#include <net/wan_bind.h>
#endif


static struct acc_restric_hdr g_AccRestric = {.inited = ACC_FALSE};
/*add by z67625 2007-08-28 start*/
static int g_Wantype = ACC_FALSE;
struct list_head g_allWan;
/*add by z67625 2007-08-28 start*/

/*add by z67625 2007-08-27 start*/
extern struct sk_buff* (*ipacc_restric_check_hook)(struct sk_buff *skb);
extern int (*acc_rstric_regdev_hook)(char __user *optval);
extern int (*acc_rstric_unregdev_hook)(char __user *optval);
extern int (*acc_rstric_dump_hook)(char __user *optval);
extern int (*acc_rstric_addwan_hook)(char __user *optval);
extern int (*acc_rstric_delwan_hook)(char __user *optval);
extern int (*arp_ipacc_reply_hook)(struct sk_buff *skb);
extern int (*acc_restric_set_hook)(char __user *optval);
extern int (*acc_restric_modi_by_lan_hook)(char __user *optval);

static int mark_lan_name(unsigned long nfmark, char *devname);


#ifdef CONFIG_ATP_BRIDGE_BIND
extern struct WanBindMap g_stBindInfo[WAN_LAN_NUM];
#endif

 

int ipacc_rstric_addwan(char __user *optval);
int ipacc_rstric_delwan(char __user *optval);

/*add by z67625 2007-08-27 end*/

/*调试开关*/
#define ACC_DEBUG_OFF  1

#if  ACC_DEBUG_OFF
#define ACC_DEBUGP(format, args...)
#else
#define ACC_DEBUGP(format, args...) printk(format,## args)
#endif


/*初始化wan类型结构链表头*/
int ipacc_rstric_initwan(void)
{
    if(ACC_TRUE == g_Wantype)
    {
        ACC_DEBUGP("ipacc_rstric_initwan acc_true =======>\n");
        return 1;
    }
    ACC_DEBUGP("ipacc_rstric_initwan =======>\n");
    INIT_LIST_HEAD(&g_allWan);
    g_Wantype = ACC_TRUE;

    return 1;
}
/*在wan类型结构链表中根据wanname获取wan节点*/
struct acc_rstric_wantype *ipacc_rstric_findwanbyname(char *wanname)
{
    
    struct acc_rstric_wantype *wantype = NULL;
    if(ACC_FALSE == g_Wantype)
    {
        ACC_DEBUGP("ipacc_rstric_findwanbyname acc_false =======>\n");
        return NULL;
    }
    if(NULL == wanname)
    {
        ACC_DEBUGP("ipacc_rstric_findwanbyname null =======>\n");
        return NULL;
    }
      ACC_DEBUGP("ipacc_rstric_findwanbyname =======>\n");

    list_for_each_entry_rcu(wantype, &g_allWan, list)
    {
        if(!strcmp(wanname,wantype->wan_name))
        {
            return wantype;
        }
    }
    return NULL;
}

/*更新wan类型结构链表中的节点*/
int ipacc_rstric_updatewan(struct acc_rstric_wantype *wan_type,int wantype)
{
    int ret = 0;
    if(ACC_FALSE == g_Wantype)
    {
        ACC_DEBUGP("ipacc_rstric_updatewan acc_false =======>\n");
        return NULL;
    }
    if(NULL == wan_type)
    {
        ACC_DEBUGP("ipacc_rstric_updatewan null =======>\n");
        return ret;
    }
        ACC_DEBUGP("ipacc_rstric_updatewan =======>\n");
    wan_type->wan_type = wantype;
    ret = 1;
    return ret;
}

/*增加wan类型结构链表节点*/
int ipacc_rstric_addwan(char __user *optval)
{
    int ret = 0;
    struct acc_wan wantype;
    struct acc_rstric_wantype *acc_wantype = NULL;
    
    ACC_DEBUGP("ipacc_rstric_addwan =======>\n");

    memset(&wantype,0,sizeof(struct acc_wan));
    if (copy_from_user(&wantype, optval, sizeof(struct acc_wan))) 
    {
        ret = 1;
        return ret;
     }
    if(NULL != (acc_wantype = ipacc_rstric_findwanbyname(wantype.wan_name)))
    {
        ipacc_rstric_updatewan(acc_wantype,wantype.wan_type);
        //ret = 1;
        return ret;
    }
    acc_wantype = kmalloc(sizeof(struct acc_rstric_wantype), GFP_KERNEL);
    if(NULL == acc_wantype)
    {
        ret = 1;
          return ret;
    }
    memset(acc_wantype, 0, sizeof(struct acc_rstric_wantype));
    strcpy(acc_wantype->wan_name, wantype.wan_name);
    strcpy(acc_wantype->bindlist, wantype.bindlist);
    acc_wantype->wan_type = wantype.wan_type;
    list_add_rcu(&(acc_wantype->list),&g_allWan);
    //ret = 1;
    return ret;
}

int ipacc_dump_wan(void)
{
    struct acc_rstric_wantype *wantype = NULL;
    if(ACC_FALSE == g_Wantype)
    {
        ACC_DEBUGP("ipacc_dump_wan wan list is NULL =======>\n");
        return 0;
    }    
    ACC_DEBUGP("ipacc_dump_wan =======>\n");
    printk("WanName,\tWanType\n");
    list_for_each_entry_rcu(wantype, &g_allWan, list)
    {
        printk("  %s  \t%d\n", 
                wantype->wan_name,
                wantype->wan_type);
    }

}

/*删除链表中的节点*/
int ipacc_rstric_delwan(char __user *optval)
{
    int ret = 1;
    struct acc_wan wantype;
    struct acc_rstric_wantype *acc_wantype = NULL;
    ACC_DEBUGP("ipacc_rstric_delwan =======>\n");
    if(ACC_FALSE == g_Wantype)
    {
        return ret;
    }
    memset(&wantype,0,sizeof(struct acc_wan));
    if (copy_from_user(&wantype, optval, sizeof(struct acc_wan))) 
    {
        return ret;
     }
    if(NULL != (acc_wantype = ipacc_rstric_findwanbyname(wantype.wan_name)))
    {
        list_del_rcu(&acc_wantype->list);
        kfree((void*)acc_wantype);
        ret = 0;
        return ret;
    }
    else
    {
        return ret;
    }
}

int ipacc_is_inited(void)
{
    ACC_DEBUGP("ipacc_is_inited =======>\n");
    return g_AccRestric.inited;
}

__inline__ int ipacc_put_dev(struct acc_device *dev)
{
    atomic_dec(&dev->refcnt);
    ACC_DEBUGP("ipacc_put_dev =======>\n");
    if(atomic_read(&dev->dirty) && !atomic_read(&dev->refcnt))
        kfree(dev);

    return ACC_TRUE;
}

__inline__ int ipacc_get_from_allow(unsigned int addr, struct acc_device **pdev)
{
    struct acc_device *dev = NULL;
    ACC_DEBUGP("ipacc_get_from_allow =======>\n");
    list_for_each_entry_rcu(dev, &g_AccRestric.allow_devs[ACC_DEVADDR_HASH_KEY(addr)], list_allow)
    {
        if(addr == dev->addr)
        {
            atomic_inc(&dev->refcnt);
            *pdev = dev;
            return ACC_TRUE;
        }
    }

    return ACC_FALSE;
}

__inline__ int ipacc_get_from_crnt(unsigned int addr, struct acc_device **pdev)
{
    struct acc_device *dev = NULL;
    ACC_DEBUGP("ipacc_get_from_crnt =======>\n");
    if(ACC_TRUE != ipacc_is_inited())
    {
        return ACC_FALSE;
    }

    list_for_each_entry_rcu(dev, &g_AccRestric.crnt_devs[ACC_DEVADDR_HASH_KEY(addr)], list_crnt)
    {
        if(addr == dev->addr)
        {
            atomic_inc(&dev->refcnt);
            *pdev = dev;
            return ACC_TRUE;
        }
    }
    return ACC_FALSE;
}

__inline__ void ipacc_dev_retimer(struct acc_device *dev)
{
    if(ACC_DEV_LIVE_ALWAYS == dev->live_time)
        return;
    ACC_DEBUGP("ipacc_dev_retimer =======>\n");
    mod_timer(&dev->timer, jiffies + dev->live_time*HZ);
}

void ipacc_arp_probe_reply(unsigned int addr)
{
    struct acc_device *dev = NULL;
    ACC_DEBUGP("ipacc_arp_probe_reply =======>\n");
    if(ACC_FALSE == ipacc_get_from_crnt(addr, &dev))
        return;

    if(0 == dev->arp_probe.times)
    {
        ipacc_put_dev(dev);
        return;
    }
    
    spin_lock_bh(&dev->arp_probe.lock);
    dev->arp_probe.times = 0;
    del_timer(&dev->arp_probe.timer);
    spin_unlock_bh(&dev->arp_probe.lock);
    
    ipacc_put_dev(dev);
}

int ipacc_arp_reply(struct sk_buff *skb)
{
     struct arphdr *arp;
     struct net_device *dev = skb->dev;
     unsigned char *arp_ptr;
     u32 sip;
     unsigned char *sha;
     char devname[IFNAMSIZ] = "";
     int i = 0;

    ACC_DEBUGP("ipacc_arp_reply =======>\n");

     struct acc_rstric_wantype *wantype = NULL;
     
     arp = skb->nh.arph;
     arp_ptr= (unsigned char *)(arp+1);
     sha     = arp_ptr;
     arp_ptr += dev->addr_len;
     memcpy(&sip, arp_ptr, 4);

    if(skb->mark & MARK_LAN_MASK)
     {
        i = mark_name(skb->mark);
      // mark_name(skb->nfmark,devname);
       /*end of HG255 by z67625 2009.02.24 HG255V100R001C01B015 AU8D01741*/
     }
     
    if(arp->ar_op == htons(ARPOP_REPLY))
    {
        /*start of HG255 by z67625 2009.02.24 HG255V100R001C01B015 AU8D01741*/
        //if(strcmp(devname,""))
        if(i >= 0)
        /*end of HG255 by z67625 2009.02.24 HG255V100R001C01B015 AU8D01741*/
        {
        #ifdef CONFIG_ATP_BRIDGE_BIND
            //没绑定则学习
            if(g_stBindInfo[i].ulBindFlag == 0)
            {
                  ipacc_arp_probe_reply(sip);
            }
            else
            {   //绑定了，但是没有绑桥
                if(strcmp(g_stBindInfo[i].acConnType,"IP_Bridged") && 
                    strcmp(g_stBindInfo[i].acConnType,"PPPoE_Bridged"))
                    ipacc_arp_probe_reply(sip);
            }
        #else
            ipacc_arp_probe_reply(sip);
        #endif /* CONFIG_ATP_BRIDGE_BIND */

        }
    }

    return 0;
}
/*begin, s63873, AU8D01410, 20081229*/
int ipacc_clear_from_crnt(unsigned int type)
{
    int i = 0;
    struct acc_device *dev = NULL;
    
    if((type >= ACC_DEVTYPE_BUTT) || (ACC_TRUE != ipacc_is_inited()))
        return ACC_FALSE;

    for(i = 0; i < ACC_DEVADDR_BUF_MAX; i++)
    {
restart:
        list_for_each_entry_rcu(dev, &g_AccRestric.crnt_devs[i], list_crnt)
        {
            if(!dev->dev_type || (type != dev->dev_type->type))
                continue;
            
           /*del the dev*/     
            atomic_inc(&dev->refcnt);
            spin_lock_bh(&g_AccRestric.lock_crnt);
            atomic_dec(&dev->refcnt);
            list_del_rcu(&dev->list_crnt);
            g_AccRestric.total_crnt --;
            dev->dev_type->total_crnt --;
            if(ACC_DEV_LIVE_ALWAYS != dev->live_time)
            {
                atomic_dec(&dev->refcnt);
                del_timer(&dev->timer);
            }
            spin_unlock_bh(&g_AccRestric.lock_crnt);

            spin_lock_bh(&dev->arp_probe.lock);
            if(dev->arp_probe.times > 0)
            {
                del_timer(&dev->arp_probe.timer);
                dev->arp_probe.times = 0;
            }
            spin_unlock_bh(&dev->arp_probe.lock);
            
            ipacc_put_dev(dev);
            
            goto restart;
        }
    }
    return ACC_TRUE;
}
/*end, s63873, AU8D01410, 20081229*/

int ipacc_set_restric(struct acc_restric_desc *acc_restric)
{
    int i;

    ACC_DEBUGP("ipacc_set_restric =======>\n");
    if(NULL == acc_restric)
        return ACC_FALSE;
    g_AccRestric.enable = acc_restric->enable;
    g_AccRestric.total_allow = acc_restric->total_allow;
    g_AccRestric.lan_host = acc_restric->lan_host;
    g_AccRestric.lan_mask = acc_restric->lan_mask;
    strncpy(g_AccRestric.lan_name, acc_restric->lan_name, IFNAMSIZ);

    for(i = 0; i < ACC_DEVTYPE_BUTT; i++)
    {
        g_AccRestric.acc_devtype[i].enable = acc_restric->acc_devtype[i].enable;
        g_AccRestric.acc_devtype[i].type = acc_restric->acc_devtype[i].type;
        /*begin, s63873, AU8D01410, 20081229*/
        if(g_AccRestric.acc_devtype[i].total_allow > acc_restric->acc_devtype[i].total_allow)
             (void)ipacc_clear_from_crnt(g_AccRestric.acc_devtype[i].type);
        /*end, s63873, AU8D01410, 20081229*/
        g_AccRestric.acc_devtype[i].total_allow = acc_restric->acc_devtype[i].total_allow;
    }
    
    return ACC_TRUE;

}

/*根据用户态参数设置用户数限制属性*/
int ipacc_rstric_set(char __user *optval)
{

    ACC_DEBUGP("ipacc_rstric_set =======> 1\n");
    int ret = 0;
    struct acc_restric_desc acc_restric;
    memset(&acc_restric, 0, sizeof(struct acc_restric_desc));
     if (copy_from_user(&acc_restric, optval, sizeof(struct acc_restric_desc))) 
    {
        ret = 1;
        return ret;
     }
    ACC_DEBUGP("ipacc_rstric_set =======> 2\n");
    if(ACC_FALSE == ipacc_set_restric(&acc_restric ))
    {
        ret = 1;
    }
    return ret;
}

int ipacc_rm_from_allow(unsigned int addr)
{
    struct acc_device *dev = NULL;
    ACC_DEBUGP("ipacc_rm_from_allow =======>\n");
    if(ACC_FALSE == ipacc_get_from_allow(addr, &dev))
        return ACC_FALSE;

    spin_lock_bh(&g_AccRestric.lock_allow);
    atomic_dec(&dev->refcnt);
    list_del_rcu(&dev->list_allow);
    atomic_set(&dev->dirty, 1);
    spin_unlock_bh(&g_AccRestric.lock_allow);

    ipacc_put_dev(dev);
    return ACC_TRUE;
}

int ipacc_rm_from_crnt(unsigned int addr)
{
    struct acc_device *dev = NULL;
    ACC_DEBUGP("ipacc_rm_from_crnt =======>\n");
    if(ACC_FALSE == ipacc_get_from_crnt(addr, &dev))
        return ACC_FALSE;

    spin_lock_bh(&g_AccRestric.lock_crnt);
    atomic_dec(&dev->refcnt);
    list_del_rcu(&dev->list_crnt);
    g_AccRestric.total_crnt --;
    dev->dev_type->total_crnt --;
    if(ACC_DEV_LIVE_ALWAYS != dev->live_time)
    {
        atomic_dec(&dev->refcnt);
        del_timer(&dev->timer);
    }
    spin_unlock_bh(&g_AccRestric.lock_crnt);

    spin_lock_bh(&dev->arp_probe.lock);
    if(dev->arp_probe.times > 0)
    {
        del_timer(&dev->arp_probe.timer);
        dev->arp_probe.times = 0;
    }
    spin_unlock_bh(&dev->arp_probe.lock);
    
    ipacc_put_dev(dev);
    return ACC_TRUE;
}

/* start static ip restric A36D04768 20080623 */
int ipacc_add2allow(struct acc_dev_desc *dev_desc, unsigned int ip_flg)
{
    struct acc_device *dev = NULL;
    ACC_DEBUGP("ipacc_add2allow =======>\n");
    if(NULL == dev_desc)
        return ACC_FALSE;

    if(dev_desc->type >= ACC_DEVTYPE_BUTT)
        return ACC_FALSE;

    if(ACC_TRUE == ipacc_get_from_allow(dev_desc->addr, &dev))
    {
        ipacc_put_dev(dev);
        return ACC_FALSE;
    }

    if(NULL == (dev = (struct acc_device*)kmalloc(sizeof(struct acc_device), GFP_KERNEL)))
    {
        return ACC_FALSE;
    }

    INIT_LIST_HEAD(&dev->list_allow);
    INIT_LIST_HEAD(&dev->list_crnt);
    init_timer(&dev->timer);
     atomic_set(&dev->refcnt, 0);
     atomic_set(&dev->dirty, 0);
    dev->addr = dev_desc->addr;
    dev->live_time = dev_desc->live_time;
    dev->ip_flg = ip_flg;
    dev->dev_type = &g_AccRestric.acc_devtype[dev_desc->type];

    init_timer(&dev->arp_probe.timer);
    spin_lock_init(&dev->arp_probe.lock);
    dev->arp_probe.parent = dev;
    dev->arp_probe.times = 0;
    
    spin_lock_bh(&g_AccRestric.lock_allow);
    atomic_inc(&dev->refcnt);
    list_add_rcu(&dev->list_allow, &g_AccRestric.allow_devs[ACC_DEVADDR_HASH_KEY(dev->addr)]);
    spin_unlock_bh(&g_AccRestric.lock_allow);
    
    return ACC_TRUE;
}
/* end static ip restric A36D04768 20080623 */
void ipacc_dev_timeout(unsigned long ptr)
{
    struct acc_device *dev = (struct acc_device *)ptr;
    ACC_DEBUGP("ipacc_dev_timeout =======>\n");
    ipacc_rm_from_crnt(dev->addr);
}

int ipacc_add2crnt(unsigned int addr)
{
    struct acc_device *dev = NULL;
    ACC_DEBUGP("ipacc_add2crnt =======>\n");
    if(ACC_TRUE == ipacc_get_from_crnt(addr, &dev))
    {
        ipacc_put_dev(dev);
        return ACC_FALSE;
    }

    if(ACC_FALSE == ipacc_get_from_allow(addr, &dev))
    {
        if(NULL == (dev = (struct acc_device*)kmalloc(sizeof(struct acc_device), GFP_KERNEL))) 
            return ACC_FALSE;

        INIT_LIST_HEAD(&dev->list_allow);
        INIT_LIST_HEAD(&dev->list_crnt);
        init_timer(&dev->timer);
         atomic_set(&dev->refcnt, 0);
         atomic_set(&dev->dirty, 0);
        dev->addr = addr;
        dev->live_time = ACC_DEV_LIVE_ALWAYS; //ACC_DEV_LIVE_DEFAULT;
        dev->dev_type = &g_AccRestric.acc_devtype[ACC_DEVTYPE_OTHER];

        init_timer(&dev->arp_probe.timer);
        spin_lock_init(&dev->arp_probe.lock);
        dev->arp_probe.parent = dev;
        dev->arp_probe.times = 0;
    }
    
    spin_lock_bh(&g_AccRestric.lock_crnt);
    atomic_inc(&dev->refcnt);
    list_add_rcu(&dev->list_crnt, &g_AccRestric.crnt_devs[ACC_DEVADDR_HASH_KEY(dev->addr)]);
    g_AccRestric.total_crnt ++;
    dev->dev_type->total_crnt ++;
    if(ACC_DEV_LIVE_ALWAYS != dev->live_time)
    {
        atomic_inc(&dev->refcnt);    
        dev->timer.expires = jiffies + dev->live_time*HZ;
        dev->timer.function = ipacc_dev_timeout;
        dev->timer.data = (unsigned long)dev;
        add_timer(&dev->timer);
    }
    spin_unlock_bh(&g_AccRestric.lock_crnt);
    
    return ACC_TRUE;
}


int ipacc_rstric_regdev(char __user *optval)
{
    int ret = 0;
    struct acc_dev_desc dev_desc;
    ACC_DEBUGP("ipacc_rstric_regdev =======> 2\n");
    if(ACC_TRUE != ipacc_is_inited())
    {
        ret = 1;
        return ret;
    }
    memset(&dev_desc, 0, sizeof(struct acc_dev_desc));
    if (copy_from_user(&dev_desc, optval, sizeof(struct acc_dev_desc)))
    {
          ret = 1;
          return ret;
     }
    /* start static ip restric A36D04768 20080623 */
    ipacc_add2allow(&dev_desc, ACC_DYNAMIC_IP); 
    /* end static ip restric A36D04768 20080623 */
    return ret;
}

int ipacc_rstric_unregdev(char __user *optval)
{
    int ret = 0;
    struct acc_dev_desc dev_desc;
    ACC_DEBUGP("ipacc_rstric_unregdev =======> 2\n");
    if(ACC_TRUE != ipacc_is_inited())
    {
        ret = 1;
        return ret;
    }   
    memset(&dev_desc, 0, sizeof(struct acc_dev_desc));
    if (copy_from_user(&dev_desc, optval, sizeof(struct acc_dev_desc)))
    {
          ret = 1;
          return ret;
     }
    ipacc_rm_from_allow(dev_desc.addr);
    ipacc_rm_from_crnt(dev_desc.addr);                
    return ret;
}

void ipacc_dump_restric(void)
{
    int i;
    
    printk("ACC_RSTRIC: dump restric settings\n");
    printk("enable      : %u\n", g_AccRestric.enable);
    printk("total_allow : %u\n", g_AccRestric.total_allow);
    printk("total_crnt  : %u\n", g_AccRestric.total_crnt);
    printk("lan_name    : %s\n", g_AccRestric.lan_name);
    printk("lan_host    : <%u.%u.%u.%u>\n", NIPQUAD(g_AccRestric.lan_host));
    printk("lan_mask    : <%u.%u.%u.%u>\n", NIPQUAD(g_AccRestric.lan_mask));
    printk("devtype--->\n");
    printk("Enable,\tType,\tTotalNum,\tTotalCrnt\n");
    for(i = 0; i < ACC_DEVTYPE_BUTT; i++)
    {
        printk("  %u    \t%s   \t%u         \t %u\n", 
                g_AccRestric.acc_devtype[i].enable,
                ((i == ACC_DEVTYPE_STB)?"STB\t":((i==ACC_DEVTYPE_PC)?"Computer":((i==ACC_DEVTYPE_WIFI)?"WiFi\t":((i==ACC_DEVTYPE_CAMERA)?"Camera":"Other")))),
                g_AccRestric.acc_devtype[i].total_allow,
                g_AccRestric.acc_devtype[i].total_crnt);
    }
    printk("\n");
}

void ipacc_dump_allow(void)
{
    int i;
    struct acc_device *dev = NULL;

    printk("ACC_RSTRIC: dump allow list\n");
    printk("Dirty,    Addr,     LiveTime,      DevType\n");
    for(i = 0; i < ACC_DEVADDR_BUF_MAX; i++)
    {
        list_for_each_entry_rcu(dev, &g_AccRestric.allow_devs[i], list_allow)
        {
            printk("%u   <%u.%u.%u.%u>     %u    %u\n", 
                   atomic_read(&dev->dirty), NIPQUAD(dev->addr), 
                   dev->live_time, dev->dev_type->type);
        }
    }
    printk("\n");
}

void ipacc_dump_crnt(void)
{
    int i;
    struct acc_device *dev = NULL;

    printk("ACC_RSTRIC: dump current list\n");
    for(i = 0; i < ACC_DEVADDR_BUF_MAX; i++)
    {
        list_for_each_entry_rcu(dev, &g_AccRestric.crnt_devs[i], list_crnt)
        {
            if (NULL != dev && NULL != dev->dev_type)
            {
                printk("(dirty,addr,live_time,dev_type)=> %u, <%u.%u.%u.%u>, %u, %u\n", 
                       atomic_read(&dev->dirty), NIPQUAD(dev->addr), 
                       dev->live_time, dev->dev_type->type);
            }
        }
    }
    printk("\n");
}

int ipacc_rstric_dump(char __user *optval)
{
    unsigned int dump_flag = ACC_DUMP_BUTT;
    int ret = 1;  
    ACC_DEBUGP("ipacc_rstric_dump =======>\n");
    copy_from_user(&dump_flag, optval, sizeof(unsigned int));
    switch(dump_flag)
    {
        case ACC_DUMP_RESTRIC:
           #if ACC_DEBUG_OFF
             ipacc_dump_restric();
           #endif
            break;
        case ACC_DUMP_ALLOW:
            ipacc_dump_allow();
            break;
        case ACC_DUMP_CRNT:
            ipacc_dump_crnt();
            break;
        /* more add here */
        case ACC_DUMP_ALL:
            ipacc_dump_restric();
            ipacc_dump_allow();
            ipacc_dump_crnt();
            ipacc_dump_wan();
            break;
        default:;
            printk("ACC_RSTRIC: unknown dump flag [%x]\n", dump_flag);
    }
    return ret;
}

void ipacc_arp_probe_send(unsigned int addr)
{
    struct net_device *dev = NULL;
    ACC_DEBUGP("ipacc_arp_probe_send =======>\n");
    dev = dev_get_by_name(g_AccRestric.lan_name);
    
    if(!dev)
        return;
    
    ACC_DEBUGP("send arp probe: dst=<%u.%u.%u.%u>, dev=[%s], lan_host=<%u.%u.%u.%u>\n", 
        NIPQUAD(addr), g_AccRestric.lan_name, NIPQUAD(g_AccRestric.lan_host));
        
    arp_send(ARPOP_REQUEST, ETH_P_ARP, addr, dev, g_AccRestric.lan_host, NULL, NULL, NULL);   

    dev_put(dev);
}

void ipacc_arp_probe_timeout(unsigned long ptr)
{
    struct acc_arp_probe  *prb = (struct acc_arp_probe*)ptr;
    unsigned int should_kicked = ACC_FALSE;
    ACC_DEBUGP("ipacc_arp_probe_timeout =======>\n");
    
    spin_lock_bh(&prb->lock);
    if(prb->times > ACC_ARP_PROBE_TIMES_MAX)
    {
        should_kicked = ACC_TRUE;
    }
    else
    {
        should_kicked = ACC_FALSE;        
        ipacc_arp_probe_send(prb->parent->addr);
        prb->times++;
        mod_timer(&prb->timer, jiffies + ACC_ARP_PROBE_INTERVAL*HZ);        
    }
    spin_unlock_bh(&prb->lock);

    if(should_kicked)
    {
        ipacc_rm_from_crnt(prb->parent->addr);
        /* start static ip restric A36D04768 20080623 */
        if (ACC_STATIC_IP == prb->parent->ip_flg)
        {
            ACC_DEBUGP("%s static ip timeout, prb->parent->addr[%x]\n", __FUNCTION__, prb->parent->addr);
            ipacc_rm_from_allow(prb->parent->addr);
        }
        /* end static ip restric A36D04768 20080623 */
    }
}

/*没有完全优化，改动太大了！！*/
int ipacc_lan_port_up_down(struct acc_devtype_hdr *dev_type)
{
    struct acc_device *dev = NULL;
    int i;
//    printk("ipacc_lan_port_up_down =======>\n");
    
    spin_lock_bh(&g_AccRestric.lock_crnt);    
    
        for(i = 0; i < ACC_DEVADDR_BUF_MAX; i++)
        {
            list_for_each_entry_rcu(dev, &g_AccRestric.crnt_devs[i], list_crnt)
            {
                if((NULL == dev) || (NULL == dev->dev_type))
                {
                    printk("acc dev or dev->dev_type is null<null>\r\n");
                    continue;
                }
                
                if(dev->arp_probe.times > 0)
                {
                    continue;
                }
                
                ipacc_arp_probe_send(dev->addr); 
                
                dev->arp_probe.timer.function = ipacc_arp_probe_timeout;
                dev->arp_probe.timer.data = (unsigned long)&dev->arp_probe;
                dev->arp_probe.timer.expires = jiffies + ACC_ARP_PROBE_INTERVAL*HZ;
                add_timer(&dev->arp_probe.timer);
            }
        }
 
    spin_unlock_bh(&g_AccRestric.lock_crnt);
    
    return ACC_FALSE;
}

/*
dev_type: NULL, search idle dev in all devs authrized
          not NULL, search idle dev in special type devs authrized
*/
int ipacc_kick_idle_outoff(struct acc_devtype_hdr *dev_type)
{
    struct acc_device *dev = NULL;
    int i;
    ACC_DEBUGP("ipacc_kick_idle_outoff =======>\n");
    for(i = 0; i < ACC_DEVADDR_BUF_MAX; i++)
    list_for_each_entry_rcu(dev, &g_AccRestric.crnt_devs[i], list_crnt)
    {
        if((NULL != dev_type) && (dev_type->type != dev->dev_type->type))
            continue;

        if(dev->arp_probe.times > 0)
            continue;
        
        ipacc_arp_probe_send(dev->addr); 
        
        dev->arp_probe.timer.function = ipacc_arp_probe_timeout;
        dev->arp_probe.timer.data = (unsigned long)&dev->arp_probe;
        dev->arp_probe.timer.expires = jiffies + ACC_ARP_PROBE_INTERVAL*HZ;
        add_timer(&dev->arp_probe.timer);
    }
    
    return ACC_FALSE;
}

#ifdef CONFIG_ATP_BRIDGE_BIND 
#define LAN1_MARK           0x00100000
#define LAN2_MARK           0x00200000
#define LAN3_MARK           0x00300000
#define LAN4_MARK           0x00400000
#define WLAN1_MARK       0x00500000
#define WLAN2_MARK       0x00600000
#define WLAN3_MARK       0x00700000
#define WLAN4_MARK       0x00800000

static int mark_lan_name(unsigned long nfmark, char *devname)
{
    int lRet = 0;
    switch(nfmark & MARK_LAN_MASK)
    {
        case LAN1_MARK:
            strcpy(devname, "eth0.2");
            lRet = 0;
            break;  
        case LAN2_MARK:
            strcpy(devname, "eth0.3");
            lRet = 1;
            break;  
        case LAN3_MARK:
            strcpy(devname, "eth0.4");
            lRet = 2;
            break; 
        case LAN4_MARK:
            strcpy(devname, "eth0.5");
            lRet = 3;
            break;  
        case WLAN1_MARK:
            strcpy(devname, "wl0");
            lRet = 4;
            break;  
        case WLAN2_MARK:
            strcpy(devname, "wl0.1");
            lRet = 5;
            break;  
        case WLAN3_MARK:
            strcpy(devname, "wl0.2");
            lRet = 6;
            break;  
        case WLAN4_MARK:
            strcpy(devname, "wl0.3");
            lRet = 7;
            break; 
        default:
            strcpy(devname, "");
            lRet = -1;
            break;
    }
    return lRet;
}
#endif

struct sk_buff* ipacc_restric_check(struct sk_buff *skb)
{
    ACC_DEBUGP("ipacc_restric_check =======>\n");
    struct iphdr *iph = NULL;
    struct acc_device *dev = NULL;
    struct acc_dev_desc dev_desc;   
    char wanname[IFNAMSIZ] ="";// "nas";
    char devname[IFNAMSIZ] = "";
    //char *p = NULL;
    int i = 0;
    struct acc_rstric_wantype *wantype = NULL;
    //p = strstr(skb->dst->dev->name,"_");
    strcpy(wanname,skb->dst->dev->name);
  #ifdef CONFIG_ATP_BRIDGE_BIND  
    if(skb->mark & MARK_LAN_MASK)
    {
       i = mark_lan_name(skb->mark, devname);
    }
  #endif
      
    if(ACC_TRUE != ipacc_is_inited())
    {
        return skb;
    }
    if(ACC_MODE_DISABLE == g_AccRestric.enable)
    {
         return skb;
    }
    
    if(NULL == (iph = skb->nh.iph))
    {
         return skb;
    }

    ACC_DEBUGP("ACC_RSTRIC: srcip <%u.%u.%u.%u>, dstip <%u.%u.%u.%u>\n", NIPQUAD(iph->saddr), NIPQUAD(iph->daddr));

    /* wan->lan or lan->lan, pass directly */
    if((iph->saddr & g_AccRestric.lan_mask) != (g_AccRestric.lan_host & g_AccRestric.lan_mask))
    {
        return skb;
    }
    
    /*add by z67625 2007-08-28 start*/
    /*通过skb查找对应的wan的类型，如果为桥则不进行限制*/
    if(NULL != (wantype = ipacc_rstric_findwanbyname(wanname)))
    {
  #ifdef CONFIG_ATP_BRIDGE_BIND  
        //判断绑定关系
        if(strcmp(devname,""))
        {
            //wan 绑定了，判断是否绑定lan
            if(strcmp(wantype->bindlist,""))
            {	
             /*A36D04181 acc没有绑定的失效 start by wangyi*/
             //   if(NULL == strstr(wantype->bindlist,devname))
             //   {printk("in return skb!\n");
                    //return skb;
             //   }
            /*A36D04181 acc没有绑定的失效 start by wangyi*/
            }
            //wan没绑定，判断lan是否绑定
            else
            {
                if(g_stBindInfo[i].ulBindFlag == 1)
                {
                    return skb;
                }
            }
        }
 #endif       
        if(ACC_WAN_BRIDGE == wantype->wan_type)
        {
            return skb;
        }
    }
    /*add by z67625 2007-08-28 end*/
    /* skb authrized before, pass directly */
    if(ACC_TRUE == ipacc_get_from_crnt(iph->saddr, &dev))
    {
        ipacc_dev_retimer(dev);    
        ipacc_put_dev(dev);
        ACC_DEBUGP("ACC_RSTRIC: skb is authrized before\n");
        return skb;
    }
    switch(g_AccRestric.enable)
    {
         case ACC_MODE_BYTOTAL:         
             if( (ACC_FALSE == ipacc_kick_idle_outoff(NULL))
                 &&(g_AccRestric.total_crnt >= g_AccRestric.total_allow))
             {
                 kfree_skb(skb);
                 ACC_DEBUGP("ACC_RSTRIC: full by total [total = %u]\n", g_AccRestric.total_crnt);
                 return NULL;
             }

             ipacc_add2crnt(iph->saddr);
              break;
              
         case ACC_MODE_BYTYPE:
             if(ACC_TRUE == ipacc_get_from_allow(iph->saddr, &dev))
             {
                 switch(dev->dev_type->type)
                 {
                 case ACC_DEVTYPE_STB:
                 case ACC_DEVTYPE_PC:
                 case ACC_DEVTYPE_WIFI:
                 case ACC_DEVTYPE_CAMERA:/* more type, add here */
                     do{
                         if(ACC_TRUE == dev->dev_type->enable
                          && (dev->dev_type->total_crnt >= dev->dev_type->total_allow)
                          && (ACC_FALSE == ipacc_kick_idle_outoff(dev->dev_type)))
                         {
                             kfree_skb(skb);
                             skb = NULL;
                             ACC_DEBUGP("ACC_RSTRIC: full by dev type [type = %u]\n", dev->dev_type->type);
                             break;
                         }
                         
                         ipacc_add2crnt(iph->saddr);
                     }while(0);
                     break;
                 case ACC_DEVTYPE_OTHER:
                 default:/* do it as pc */
                     kfree_skb(skb);
                    skb = NULL;
                     break;
                 }
          
                 ipacc_put_dev(dev);
                 return skb;
             }
             else
             {
                /* start static ip restric A36D04768 20080623 */
                memset(&dev_desc, 0, sizeof(struct acc_dev_desc));
                dev_desc.addr = iph->saddr;
                dev_desc.type = ACC_DEVTYPE_PC;
                dev_desc.live_time = ACC_DEV_LIVE_ALWAYS;
                ipacc_add2allow(&dev_desc, ACC_STATIC_IP); 
                if (ACC_TRUE == ipacc_get_from_allow(iph->saddr, &dev)) 
                {
                    do {
                         if((ACC_TRUE == dev->dev_type->enable)
                          && (dev->dev_type->total_crnt >= dev->dev_type->total_allow)
                          && (ACC_FALSE == ipacc_kick_idle_outoff(dev->dev_type)))
                         {
                             kfree_skb(skb);
                             skb = NULL;
                             break;
                         }
                         ipacc_add2crnt(iph->saddr);
                     }while(0);                
                }
                else
                {
                     kfree_skb(skb);
                     skb = NULL;
                }
                /* end static ip restric A36D04768 20080623 */
             }
             break;
             
        default:;/* do nothing */
     }
    
    return skb;
}

/*给函数指针赋值*/
void ipacc_hook_init(void)
{
    ACC_DEBUGP("ipacc_hook_init =======>\n");
    ipacc_restric_check_hook = ipacc_restric_check;
    acc_rstric_regdev_hook = ipacc_rstric_regdev;
    acc_rstric_unregdev_hook = ipacc_rstric_unregdev;
    acc_rstric_dump_hook = ipacc_rstric_dump;
    acc_rstric_addwan_hook = ipacc_rstric_addwan;
    acc_rstric_delwan_hook = ipacc_rstric_delwan;
    arp_ipacc_reply_hook = ipacc_arp_reply;
    acc_restric_set_hook = ipacc_rstric_set;
    acc_restric_modi_by_lan_hook = ipacc_kick_idle_outoff;
}

void ipacc_init(void)
{
    int i;
    ACC_DEBUGP("ipacc_init =======>\n");


    if(ACC_TRUE == g_AccRestric.inited)
        return;

    memset(&g_AccRestric, 0, sizeof(struct acc_restric_hdr));

    g_AccRestric.enable = ACC_FALSE;
    g_AccRestric.total_allow = 0;
    g_AccRestric.total_crnt = 0;
    spin_lock_init(&g_AccRestric.lock_allow);
    spin_lock_init(&g_AccRestric.lock_crnt);   
    
    for(i = 0; i < ACC_DEVADDR_BUF_MAX; i++)
    {
        INIT_LIST_HEAD(&g_AccRestric.allow_devs[i]);
        INIT_LIST_HEAD(&g_AccRestric.crnt_devs[i]);
    }
    
    for(i = 0; i < ACC_DEVTYPE_BUTT; i++)
    {
        g_AccRestric.acc_devtype[i].type = i;
        g_AccRestric.acc_devtype[i].parent_hdr = &g_AccRestric;
    }
    
    g_AccRestric.inited = ACC_TRUE;
}

void __init ipacc_restric_init(void)
{
    ipacc_hook_init();
    ipacc_init();
    ipacc_rstric_initwan();
}


