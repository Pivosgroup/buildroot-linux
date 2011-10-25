/*******************************************************************
 * 
 *  Copyright (C) 2007 by Sympeer, Inc. All Rights Reserved.
 *
 *  Description: the interface of ABoxBase sockets
 *
 *  Author: Sympeer Software
 *
 *******************************************************************/


#ifndef _ABX_SOCKET_H_
#define	_ABX_SOCKET_H_

#include "abx_common.h"
#include "abx_error.h"


#ifdef AVOS
    struct timeval;
    #include <sysdefine.h>
    #include <net/lwip/src/include/lwip/sockets.h>
    #include <net/resolver/resolver.h>

    #define abx_accept(a,b,c)           lwip_accept((a),(b),(c))
    #define abx_bind(a,b,c)             lwip_bind((a),(b),(c))
    #define abx_shutdown(a,b)           lwip_shutdown((a),(b))
    #define abx_closesocket(s)          lwip_close(s)
    #define abx_connect(a,b,c)          lwip_connect((a),(b),(c))
    #define abx_getsockname(a,b,c)      lwip_getsockname((a),(b),(c))
    #define abx_getpeername(a,b,c)      lwip_getpeername((a),(b),(c))
    #define abx_setsockopt(a,b,c,d,e)   lwip_setsockopt((a),(b),(c),(d),(e))
    #define abx_getsockopt(a,b,c,d,e)   lwip_getsockopt((a),(b),(c),(d),(e))
    #define abx_listen(a,b)             lwip_listen((a),(b))
    #define abx_recv(a,b,c,d)           lwip_recv((a),(b),(c),(d))
    #define abx_read(a,b,c)             lwip_read((a),(b),(c))
    #define abx_recvfrom(a,b,c,d,e,f)   lwip_recvfrom((a),(b),(c),(d),(e),(f))
    #define abx_send(a,b,c,d)           lwip_send((a),(b),(c),(d))
    #define abx_sendto(a,b,c,d,e,f)     lwip_sendto((a),(b),(c),(d),(e),(f))
    #define abx_socket(a,b,c)           lwip_socket((a),(b),(c))
    #define abx_write(a,b,c)            lwip_write((a),(b),(c))
    #define abx_select(a,b,c,d,e)       lwip_select((a),(b),(c),(d),(e))
    #define abx_ioctlsocket(a,b,c)      lwip_ioctl((a),(b),(c))

    typedef int abx_socket_t;
    #define abx_lastsockerror()         lwip_errno
    #define ABX_INVALID_SOCKET          -1
    #define SOCKET_ERROR                -1
    #define ABX_EINPROGRESS             EINPROGRESS
    #define ABX_EWOULDBLOCK	            EWOULDBLOCK

#else   // AVOS

    #define FD_SETSIZE                  256
    #include <Winsock2.h>

    #define abx_accept(a,b,c)           accept((a),(b),(c))
    #define abx_bind(a,b,c)             bind((a),(b),(c))
    #define abx_shutdown(a,b)           assert(0)
    #define abx_closesocket(s)          closesocket(s)
    #define abx_connect(a,b,c)          connect((a),(b),(c))
    #define abx_getsockname(a,b,c)      getsockname((a),(b),(c))
    #define abx_getpeername(a,b,c)      getpeername((a),(b),(c))
    #define abx_setsockopt(a,b,c,d,e)   setsockopt((a),(b),(c),(d),(e))
    #define abx_getsockopt(a,b,c,d,e)   getsockopt((a),(b),(c),(d),(e))
    #define abx_listen(a,b)             listen((a),(b))
    #define abx_recv(a,b,c,d)           recv((a),(b),(c),(d))
    #define abx_read(a,b,c)             read((a),(b),(c))
    #define abx_recvfrom(a,b,c,d,e,f)   recvfrom((a),(b),(c),(d),(e),(f))
    #define abx_send(a,b,c,d)           send((a),(b),(c),(d))
    #define abx_sendto(a,b,c,d,e,f)     sendto((a),(b),(c),(d),(e),(f))
    #define abx_socket(a,b,c)           socket((a),(b),(c))
    #define abx_write(a,b,c)            write((a),(b),(c))
    #define abx_select(a,b,c,d,e)       select((a),(b),(c),(d),(e))
    #define abx_ioctlsocket(a,b,c)      ioctlsocket((a),(b),(c))

    typedef SOCKET abx_socket_t;
    #define abx_lastsockerror()         WSAGetLastError()
    #define ABX_INVALID_SOCKET          INVALID_SOCKET
    #define ABX_EINPROGRESS             WSAEWOULDBLOCK      /* in Windows Socket, nonblocking operation results are identified by WSAEWOULDBLOCK */
    #define ABX_EWOULDBLOCK	            WSAEWOULDBLOCK
#endif  // AVOS


/**
 * the callback function for asynchronous gethostbyname()
 * parameter name & host_result should be copied out if needed
 */
typedef void (* abx_resolver_cb_t)(const char * name, void * arg, const struct hostent *host_result);

abx_errid_t abx_gethostbyname_a(const char * name, void * arg, abx_resolver_cb_t callback);


#endif //ABX_SOCKET_H

