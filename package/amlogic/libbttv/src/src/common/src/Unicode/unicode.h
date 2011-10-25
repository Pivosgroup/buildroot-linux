/*******************************************************************
 * 
 *  Copyright C 2005 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: 
 *
 *  Author: Amlogic Software
 *  Created: Fri Nov 11 00:13:19 2005
 *
 *******************************************************************/
#ifndef UNICODE_H
#define UNICODE_H
#include "nls_uni.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Because we want to be compatible with old version and try to modify fewer codes in other files other than that of unicode.lib ,so don't modify
  * these function name here.But we must know  that GB don't mean just GB ,may mean other standards of unicode such as iso8859-1,iso8859-2 etc
  *Forgive me.
  */
void init_nls_table(char *charset);
int  register_all_nls_tables();
u_int16_t 	Unicode2GB(u_int16_t unicode);
u_int16_t 	GB2Unicode(u_int16_t othercode);
u_int16_t get_lower_charset(u_int16_t charcode);
u_int16_t get_upper_charset(u_int16_t charcode);
int         StrnGB2Unicode(char *dest, const char *src, int dest_len, int src_len);
int         get_cur_unitable_flag();
int         StrnReplaceUnknown(char *src,int src_len);
#ifdef __cplusplus
}
#endif
#endif
