/*******************************************************************
 * 
 *  Copyright C 2005 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: 
 *
 *  Author: Amlogic Software
 *  Created: Fri Nov 11 00:13:05 2005
 *
 *******************************************************************/
#include "unicode.h"
#include <endian.h>

char Unicode2Ansi(u_int16_t uni_code)
{
#if BYTE_ORDER == LITTLE_ENDIAN
	return (uni_code>>8);
#else
	return uni_code;
#endif
}

void StrUnicode2Ansi(char *dest, const u_int16_t *src, int n)  
{  
	u_int16_t 	code;  
	int i, cur;
	
	for (i = 0, cur = 0; cur < n; i++, cur++)
	{  
#if BYTE_ORDER == LITTLE_ENDIAN
		code = *(src+cur)>>8;	// little endian
#else
		code = (*(src+cur));	// big endian
#endif // LITTLE_ENDIAN
		
		if (code>0x7F && code<0xFF)
		{  
			*(dest+i) = '?';
		}  
		else if (code>0x00 && code<=0x7F)	// Ascii
		{
			*(dest+i) = (char)code;
		}  
	}  
	*(dest+i)='\0';  
}

u_int16_t Ansi2Unicode(char ansi_code)
{
#if BYTE_ORDER == LITTLE_ENDIAN
	return (ansi_code<<8);
#else
	return ansi_code;
#endif
}

void StrAnsi2Unicode(u_int16_t *dest, const char *src, int n)  
{  
	u_int16_t 	code;  
	int i, cur;
	
	for (i = 0, cur = 0; cur < n; i++, cur++)
	{  
#if BYTE_ORDER == LITTLE_ENDIAN
		code = *(src+cur);	// little endian
#else
		code = (*(src+cur)<<8);	// big endian
#endif // LITTLE_ENDIAN
		*(dest+i) = (u_int16_t)code;
	}  
	*(dest+i)='\0';  
}


