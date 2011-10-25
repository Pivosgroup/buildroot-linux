/*******************************************************************
 * 
 *  Copyright (C) 2007 by Sympeer, Inc. All Rights Reserved.
 *
 *  Description: the interface of ABoxBase strings
 *
 *  Author: Sympeer Software
 *
 *******************************************************************/

#ifndef _ABX_STRING_H_
#define _ABX_STRING_H_

#include "abx_common.h"

#ifdef AVOS
    #include "os/prf.h"
#endif

#ifdef ABX_UNICODE
    #define ABX_TCHAR		ABX_WCHAR
    #define ABX_TStrLen	    ABX_WStrLen
#else
    #define ABX_TCHAR 		char
    #define ABX_TStrLen	    strlen
#endif


//typedef uint16 ABX_WCHAR;
/* TBD
int ABX_WStrLen(const ABX_WCHAR * s);
int ABX_WStrCat
int ABX_WStrCmp
int ABX_WStrNCmp
int ABX_WStrCpy
int ABX_WStrNCpy
*/


#ifdef AVOS
    #define abx_vsnprintf   vsnprintf
#else
    #include <stdio.h>
    #define abx_vsnprintf   _vsnprintf
    #define snprintf        _snprintf
    #define strncasecmp     _strnicmp
    #define strcasecmp      _stricmp
#endif

/// duplicate a string, the result should be freed by abx_free
char* abx_strdup(const char * src);

/**
 * convert UTF-8 to GB
 * if dst is set and dst_len is long enough, the source string will be converted to GB2312 and stored in dst buffer
 * if dst is 0 or dst_len is 0 or short, the function only returns the size of the converted string
 * @param src                       [in]    source string in UTF-8
 * @param src_len                   [in]    source string length. -1 indicates it is a NULL-terminated string
 * @param dst                       [in]    destination buffer
 * @param dst_len                   [in]    destination buffer size, including the ending 0
 * @return                                  the actual destination length including the ending 0
 */
int abx_utf8_2_gb(char * src, int src_len, char * dst, int dst_len);


#endif //_ABX_STRING_H_


