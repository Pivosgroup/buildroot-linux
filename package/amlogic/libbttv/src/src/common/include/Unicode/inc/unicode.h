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

#include "Unicode/inc/nls_uni.h"

typedef unsigned int UI32, U32,u32;
/*
 Signed 32-bit integer value
 */
typedef signed int SI32, S32,s32;
/*
 Unsigned 16-bit integer value
 */
typedef unsigned short UI16, U16, u16;
/*
 Signed 16-bit integer value
 */
typedef signed short SI16, S16, s16;
/*
 Unsigned 8-bit integer value
 */
typedef unsigned char UI8, U8, u8;
/*
 Signed 8-bit integer value
 */
typedef signed char SI8, S8, s8;

typedef unsigned int  ucs4_t;
typedef UI8 UTF8;
typedef UI16 UTF16;
typedef UI32 UTF32;
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
int         StrnGB2Unicode(char *dest, const char *src, int dest_len, int src_len) ;
int get_cur_unitable_flag();

u_int16_t GetUTF8Len(const char *utf, int utf8size);
int utf8_2_gb(char* gb_buf, char* utf8_buf, int utf8size);
int gb_2_utf8(char* utf8_buf, char* gb_buf, int gbsize);
u_int16_t UTF8ToUnicode(char *uc, const char *utf, char big_endian, int utf8size);

int StrnReplaceUnknown(char *src,int src_len);

int 
native_2_utf8(char * utf, char * native, int native_len);

int 
utf8_2_native(char * native, char * utf8, int utf8_len);

int uni_cp936_mbtowc(ucs4_t *pwc, const unsigned char *s, int n);
int uni_cp936_to_ucs2(const UI8 *src, int src_len, UI16 *dest, int dest_len);
UI32 uni_Utf8Count_fast(const UI8* str, SI32 blen);
int uni_Utf8Count(const UTF8 *in, int inLen);
int uni_Utf16toUtf8(const UTF16 *in, int inLen, UTF8 *out, int outMax);
int uni_Utf8ToUtf16(const UTF8 *in, int inLen, UTF16 *out, int outMax);

int uni_MbtoUtf16(UI8 *src, int src_len, UI16 *dest, int dest_len);
int uni_MbCount(UI8 *in , int in_len);
#ifdef __cplusplus
}
#endif

#endif
