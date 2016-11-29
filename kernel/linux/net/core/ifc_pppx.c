#include <net/ifc_pppx.h>
static spinlock_t        g_wanLock;
static struct list_head g_wanDevs;
static int               g_inited = PPPX_MAC_FALSE;

/* 修改此处接口时，注意修改相应的判断，是否存在 0-7 的范围判断 */
static struct pppx_macmap g_macTbl[] = 
{
	{"eth0.2", {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
	{"eth0.3", {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
	{"eth0.4", {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
	{"eth0.5", {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
	{"wl0", {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
	{"wl0.1", {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
	{"wl0.2", {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
	{"wl0.3", {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
	{NULL, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}
};

int pppx_get_lanname( void **trace_info, int *len)
{
	struct pppx_macmap *pmark = NULL;
	struct pppx_ppptrace *trace = NULL;

	if(NULL == trace_info || NULL == len)
		return PPPX_MAC_FALSE;

	*len = sizeof(struct pppx_ppptrace) * (sizeof(g_macTbl)/sizeof(struct pppx_macmap));
	*trace_info = (void *)kmalloc(*len, GFP_KERNEL);

	if(NULL == *trace_info)
		return PPPX_MAC_FALSE;
	else
		memset(*trace_info, 0, *len);
	
	for(pmark = g_macTbl, trace = *trace_info; pmark->name; pmark++, trace++)
    {
    	strcpy(trace->lan_name, pmark->name);
    	memcpy(trace->rmt_mac, pmark->rmt_mac, ETH_ALEN);
    }
    
	return PPPX_MAC_TRUE;
}

void pppx_bindmac(struct sk_buff *skb, int ifc)
{
    int i = 0;
    /* 在函数入口判断收报的接口 */
    if (ifc < 0 || ifc > 7)
    {
        printk("file:[%s, %d] this would't occur.....ifc:[%d] \r\n", __FILE__, __LINE__, ifc);
        return;
    }
    if (skb->protocol == __constant_htons(ETH_P_PPP_DISC))
    {
        memcpy(g_macTbl[ifc].rmt_mac, eth_hdr(skb)->h_source, ETH_ALEN);
        for (i = 0; g_macTbl[i].name ; i++)
        {
            /* ppptrace clear */
        	if((0 == memcmp(g_macTbl[i].rmt_mac, eth_hdr(skb)->h_source, ETH_ALEN))
                && (i !=  ifc))
            {   
        		memset(g_macTbl[i].rmt_mac, 0, ETH_ALEN);
            }
        }
    }
    
#if 0
    struct pppx_macmap *pmark = NULL;

    
    if(strncmp(skb->dev->name, "eth", 3) != 0 
     && strncmp(skb->dev->name, "wl", 2) != 0)
        return;

    for( pmark = g_macTbl;
         pmark->name; 
         pmark++)
    {
    	if(skb->protocol != __constant_htons(ETH_P_PPP_DISC))
    		continue;
    	
		/* ppptrace clear */
    	if(0 == memcmp(pmark->rmt_mac, skb->mac.ethernet->h_source, ETH_ALEN))
    		memset(pmark->rmt_mac, 0, ETH_ALEN);
    }

    if(NULL == pmark->name)
    {
        for(pmark = g_macTbl;
             pmark->name && (0 != strncmp(skb->dev->name, pmark->name, strlen(pmark->name))); 
             pmark++)
        {/* do nothing */}
    }

    if(NULL == pmark->name)
        return;

	/* for ppp trace */
    if(skb->protocol == __constant_htons(ETH_P_PPP_DISC))
		memcpy(pmark->rmt_mac, skb->mac.ethernet->h_source, ETH_ALEN);

    //printk("marked: dev=[%s] nfmark=[%x]\n", skb->dev->name, skb->nfmark);
#endif
}
