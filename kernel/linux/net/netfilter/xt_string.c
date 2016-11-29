/* String matching match for iptables
 *
 * (C) 2005 Pablo Neira Ayuso <pablo@eurodev.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_string.h>
#include <linux/textsearch.h>

MODULE_AUTHOR("Pablo Neira Ayuso <pablo@eurodev.net>");
MODULE_DESCRIPTION("IP tables string match module");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ipt_string");
MODULE_ALIAS("ip6t_string");

static int match(const struct sk_buff *skb,
		 const struct net_device *in,
		 const struct net_device *out,
		 const struct xt_match *match,
		 const void *matchinfo,
		 int offset,
		 unsigned int protoff,
		 int *hotdrop)
{
	const struct xt_string_info *conf = matchinfo;
	struct ts_state state;
/* BEGIN: Added by weishi kf33269, 2010/11/25   PN:url路径大小写不敏感*/
	bool invert;
	invert = conf->invert & XT_STRING_FLAG_INVERT;
/* END:   Added by weishi kf33269, 2010/11/25 */
	memset(&state, 0, sizeof(struct ts_state));
/* BEGIN: Modified by weishi kf33269, 2010/11/25   PN:url路径大小写不敏感*/
	return (skb_find_text((struct sk_buff *)skb, conf->from_offset,
			     conf->to_offset, conf->config, &state)
			     != UINT_MAX) ^ invert;
/* END:   Modified by weishi kf33269, 2010/11/25 */
}

#define STRING_TEXT_PRIV(m) ((struct xt_string_info *) m)

static int checkentry(const char *tablename,
		      const void *ip,
		      const struct xt_match *match,
		      void *matchinfo,
		      unsigned int hook_mask)
{
	struct xt_string_info *conf = matchinfo;
	struct ts_config *ts_conf;
/* BEGIN: Added by weishi kf33269, 2010/11/25   PN:url路径大小写不敏感*/
	int flags = TS_AUTOLOAD;
/* END:   Added by weishi kf33269, 2010/11/25 */
	/* Damn, can't handle this case properly with iptables... */
	if (conf->from_offset > conf->to_offset)
		return 0;
	if (conf->algo[XT_STRING_MAX_ALGO_NAME_SIZE - 1] != '\0')
		return 0;
	if (conf->patlen > XT_STRING_MAX_PATTERN_SIZE)
		return 0;
/* BEGIN: Added by weishi kf33269, 2010/11/25   PN:url路径大小写不敏感*/
	if (conf->invert &
	    ~(XT_STRING_FLAG_IGNORECASE | XT_STRING_FLAG_INVERT))
		return -EINVAL;
	if (conf->invert & XT_STRING_FLAG_IGNORECASE)
		flags |= TS_IGNORECASE;

/* END:   Added by weishi kf33269, 2010/11/25 */
/* BEGIN: Modified by weishi kf33269, 2010/11/25   PN:url路径大小写不敏感*/
	ts_conf = textsearch_prepare(conf->algo, conf->pattern, conf->patlen,
				     GFP_KERNEL, flags);
/* END:   Modified by weishi kf33269, 2010/11/25 */
	if (IS_ERR(ts_conf))
		return 0;

	conf->config = ts_conf;

	return 1;
}

static void destroy(const struct xt_match *match, void *matchinfo)
{
	textsearch_destroy(STRING_TEXT_PRIV(matchinfo)->config);
}

static struct xt_match xt_string_match[] = {
	{
		.name 		= "string",
/* BEGIN: Added by weishi kf33269, 2010/11/25   PN: 在iptables 中，通过这个值 判断是否支持url路径大小写不敏感*/
		//.revision   = 1,
/* END:   Added by weishi kf33269, 2010/11/25 */
		.family		= AF_INET,
		.checkentry	= checkentry,
		.match 		= match,
		.destroy 	= destroy,
		.matchsize	= sizeof(struct xt_string_info),
		.me 		= THIS_MODULE
	},
	{
		.name 		= "string",
		.family		= AF_INET6,
		.checkentry	= checkentry,
		.match 		= match,
		.destroy 	= destroy,
		.matchsize	= sizeof(struct xt_string_info),
		.me 		= THIS_MODULE
	},
};

static int __init xt_string_init(void)
{
	return xt_register_matches(xt_string_match, ARRAY_SIZE(xt_string_match));
}

static void __exit xt_string_fini(void)
{
	xt_unregister_matches(xt_string_match, ARRAY_SIZE(xt_string_match));
}

module_init(xt_string_init);
module_exit(xt_string_fini);
