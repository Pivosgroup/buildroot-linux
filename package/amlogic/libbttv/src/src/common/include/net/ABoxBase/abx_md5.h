/*******************************************************************
 * 
 *  Copyright (C) 2007 by Sympeer, Inc. All Rights Reserved.
 *
 *  Description: the interface of MD5 for ABoxBase
 *
 *  Author: Sympeer Software
 *
 *******************************************************************/

/*
 * Copyright (c) 2004-2005 Sergey Lyubka <valenok@gmail.com>
 * All rights reserved
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Sergey Lyubka wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */

#ifndef _ABX_MD5_HEADER_INCLUDED_H_
#define	_ABX_MD5_HEADER_INCLUDED_H_

#include "abx_common.h"

typedef struct MD5Context {
    uint32_t        buf[4];
    uint32_t        bits[2];
    unsigned char   in[64];
} MD5_CTX;

extern void ABoxMD5Init(MD5_CTX *ctx);
extern void ABoxMD5Update(MD5_CTX *ctx, unsigned char const *buf, unsigned len);
extern void ABoxMD5Final(unsigned char digest[16], MD5_CTX *ctx);

#endif /* _ABX_MD5_HEADER_INCLUDED_H_ */
