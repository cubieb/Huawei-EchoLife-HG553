#ifndef _XT_STRING_H
#define _XT_STRING_H
/* BEGIN: Added by weishi kf33269, 2010/11/25   PN:url路径大小写不敏感*/
#include <linux/types.h>
/* END:   Added by weishi kf33269, 2010/11/25 */
#define XT_STRING_MAX_PATTERN_SIZE 257
#define XT_STRING_MAX_ALGO_NAME_SIZE 16
/* BEGIN: Added by weishi kf33269, 2010/11/25   PN:url路径大小写不敏感*/
enum {
	XT_STRING_FLAG_INVERT		= 0x01,
	XT_STRING_FLAG_IGNORECASE	= 0x02
};
/* END:   Added by weishi kf33269, 2010/11/25 */
struct xt_string_info
{
	u_int16_t from_offset;
	u_int16_t to_offset;
	char	  algo[XT_STRING_MAX_ALGO_NAME_SIZE];
	char 	  pattern[XT_STRING_MAX_PATTERN_SIZE];
	u_int16_t  patlen;
	u_int8_t  invert;
	struct ts_config __attribute__((aligned(8))) *config;
};

#endif /*_XT_STRING_H*/
