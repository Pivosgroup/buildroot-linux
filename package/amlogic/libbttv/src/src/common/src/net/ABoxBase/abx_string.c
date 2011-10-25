/*******************************************************************
 * 
 *  Copyright (C) 2007 by Sympeer, Inc. All Rights Reserved.
 *
 *  Description: the realization of ABoxBase strings
 *
 *  Author: Sympeer Software
 *
 *******************************************************************/


#include "abx_string.h"
#include "abx_mem.h"
#include "abx_dbg.h"
#include <string.h>

#ifdef AVOS
#include <Unicode.h>
#else
#include <windows.h>
#endif


char* abx_strdup(const char * src)
{
    size_t len;
    char * result;
    ABX_ASSERT(src);
    len = strlen(src);
    result = abx_malloc(len + 1);
    result[len] = '\0';
    strncpy(result, src, len);
    return result;
}



int abx_utf8_2_gb(char * src, int src_len, char * dst, int dst_len)
{
#ifdef AVOS
    int len;
    if (src_len == -1) {
        len = GetUTF8Len(src, strlen(src)) + 1;
    } else {
        len = GetUTF8Len(src, src_len) + 1;
    }

    if (!dst || dst_len < len)
        return len;

    len = utf8_2_gb(dst, src, src_len);
    dst[len] = '\0';
    return len + 1;
#else
    int len,len1;
    WCHAR * temp;

    if (src == NULL || src_len < -1 || src_len == 0)
        return -1;

    len = MultiByteToWideChar(CP_UTF8, 0, src, src_len, 0, 0);
    temp = (WCHAR *)malloc(sizeof(WCHAR) * len);
    if (!temp)
        return -1;

    MultiByteToWideChar(CP_UTF8, 0, src, src_len, temp, len);
    len1 = WideCharToMultiByte(CP_ACP, 0, temp, len, 0, 0, 0, 0);
    if (!dst || dst_len < len1)
        return len1;

    WideCharToMultiByte(CP_ACP, 0, temp, len, dst, len1, 0, 0);
    free(temp);
    return len1;
#endif
}

