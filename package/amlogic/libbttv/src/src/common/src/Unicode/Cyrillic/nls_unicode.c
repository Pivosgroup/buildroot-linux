#include "errno.h"
#include <string.h>
#include "../nls_uni.h"
#include "../nls_cp1251.h"

static struct nls_table *tables;
static struct nls_table *cur_table;

int register_nls(struct nls_table * nls)
{
	struct nls_table ** tmp = &tables;

	if (!nls)
		return -EINVAL;
	if (nls->next)
		return -EBUSY;
	
	while (*tmp) {
		if (nls == *tmp) {
			
			return -EBUSY;
		}
		tmp = &(*tmp)->next;
	}
	nls->next = tables;
	tables = nls;
	
	return 0;	
}
int unregister_nls(struct nls_table * nls)
{
	struct nls_table ** tmp = &tables;

	
	while (*tmp) {
		if (nls == *tmp) {
			*tmp = nls->next;
			
			return 0;
		}
		tmp = &(*tmp)->next;
	}
	
	return -EINVAL;
}
static struct nls_table *find_nls(char *charset)
{
	struct nls_table *nls=NULL;
	
	for (nls = tables; nls; nls = nls->next)
		if (! strcmp(nls->charset, charset))
			break;
		
	return nls;
}
/* set current unicode2char table
 *
 */
void init_nls_table(char *charset)
{
    cur_table = find_nls(charset);
}
int  register_all_nls_tables()
{
    init_nls_cp1251();
    return 0;
}
u_int16_t Unicode2GB(u_int16_t unicode)
{
	if(cur_table){
	    return cur_table->uni2char(unicode);
    }
	
	return 0; // Failed  
}
u_int16_t 	GB2Unicode(u_int16_t othercode)
{
    if(cur_table){
	    return cur_table->char2uni(othercode);
    }
	
	return 0; // Failed  
}

u_int16_t get_lower_charset(u_int16_t charcode)
{
	unsigned char *p;
	
	if(cur_table){
		if(charcode>0xff)
			return charcode;
		
		p = (unsigned char *)cur_table->cs2low;
		return *(p+charcode);
    	}
	
	return charcode; // Failed  

}

u_int16_t get_upper_charset(u_int16_t charcode)
{
	unsigned char *p;
	
	if(cur_table){
		if(charcode>0xff)
			return charcode;
		
		p = (unsigned char *)cur_table->cs2up;
		return *(p+charcode);
    	}
	
	return charcode; // Failed  
}

int get_cur_unitable_flag()
{
    if(!cur_table)
        return UNICODE_NULL;
    else
        return cur_table->flag;
}

int StrnGB2Unicode(char *dest, const char *src, int dest_len, int src_len)
{
	int src_index = 0, dest_index = 0;
	u_int16_t 	code, *dest_str = (u_int16_t *)dest;
	dest_len = dest_len >> 1 ;
	
	while((src_index < src_len)&&(dest_index < dest_len))
	{  
		code = src[src_index++] ;
		if(get_cur_unitable_flag()==UNICODE_ONE_BYTE)
		{
		     code = GB2Unicode(code);
		}
		else 
		{
		if(code > 0x7f){ 
		    code = (code << 8) | src[src_index++];
		    code = GB2Unicode(code);
		}
		}
		if(code)
		    dest_str[dest_index++] = code ;
		else
		    dest_str[dest_index++] = '?' ;
	}
	return (dest_index);
}

