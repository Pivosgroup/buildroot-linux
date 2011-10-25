/*******************************************************************
 * 
 *  Copyright (C) 2007 by Sympeer, Inc. All Rights Reserved.
 *
 *  Description: the ABox UStat tool
 *
 *  Author: Sympeer Software
 *
 *******************************************************************/


#ifndef _ABX_USTAT_H_
#define	_ABX_USTAT_H_

#include "abx_socket.h"
#include "abx_thread.h"


#define ABX_USTAT_MAX_PKTSIZE 1444
#define ABX_USTAT_DEFAULT_PORT 5707


#pragma  pack(push, 1)


/// the header structure of a UStat packet
typedef struct {
    unsigned char seqno : 4;
    unsigned char is_text: 1;
    unsigned char need_ack : 1;
    unsigned char compressed : 1;
    unsigned char reserved : 1;
} abx_ustat_header_t;



typedef struct {
    abx_socket_t sock;
    struct sockaddr_in servaddr;
    int curlen;
    abx_sema_t sema;
    abx_ustat_header_t header;
    char buf[ABX_USTAT_MAX_PKTSIZE + 1];
} abx_usclient_t;

#pragma pack(pop)


abx_usclient_t * abx_usclient_new(const char * ip, unsigned short port /* in host order */, unsigned int is_text, unsigned int need_ack, unsigned int need_compress);
void abx_usclient_release(abx_usclient_t * client);
abx_errid_t abx_usclient_append(abx_usclient_t * client, const void * buf, int len);
abx_errid_t abx_usclient_appendstr(abx_usclient_t * client, const char * format, ...);
abx_errid_t abx_usclient_send(abx_usclient_t * client);


// short macros for convenience
extern abx_usclient_t * ustat_client;
void _abx_usclient_new();
void _abx_usclient_appendstr(const char * format, ...);
void _abx_usclient_send();

#ifdef ENABLE_ABX_USTAT

#define ABX_USCLIENT_NEW()              _abx_usclient_new()
#define ABX_USCLIENT_APPENDSTR(x)   _abx_usclient_appendstr x
#define ABX_USCLIENT_SEND()             _abx_usclient_send()
#define ABX_USCLIENT_SENDSTR(x)     do {_abx_usclient_appendstr x; _abx_usclient_send(); } while(0)

#else   // ENABLE_ABX_USTAT

#define ABX_USCLIENT_NEW()
#define ABX_USCLIENT_APPENDSTR(x)
#define ABX_USCLIENT_SEND()
#define ABX_USCLIENT_SENDSTR(x)

#endif  // ENABLE_ABX_USTAT


#endif //_ABX_USTAT_H_

