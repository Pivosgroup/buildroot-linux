/*******************************************************************
 * 
 *  Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: Multi netif management module
 *
 *  Author: Gaokj tianyu.li
 *
 *******************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/ioctl.h>

typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned char u8;

#include <linux/route.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <pthread.h>
#include "network_setting.h"
#include "if_manager.h"


#define NETWORKMON_PLATFORM_ARC

#define ERR_GOTO(express, err_info, LABEL, ret_val) \
        do {  \
            if ((express)) { \
                gotoRet = (ret_val); \
                goto LABEL; \
            } \
        } while (0)


#define ERR_RET(express, err_info, ret_val) \
        do {  \
            if ((express)) { \
                return (ret_val); \
            } \
        } while (0)

struct ifm_if_node {
    struct ifm_if_node *next;
    char if_name[IFNAMSIZ];
    int if_state;
    if_state_changed_cb if_sc_cb;
    if_check_device_cb  if_cd_cb;

    struct ifm_if_cfg cfg;
};


/*you'd better add the new function here when you add a new interface to the list*/
int ifm_check_ether_cable( int if_id, char *if_name, int *state );
int ifm_check_wifi_cable(int if_id, char *if_name, int *state);
int ifm_check_ppp_cable(int if_id, char *if_name, int *state);
static struct ifm_if_node ifm_if_node_tbl[] =                    /*i can be bigger*/
{
    {NULL, "eth0",  IFM_IF_STATE_REMOVED, NULL, ifm_check_ether_cable, },
 //   {NULL, "eth1",  IFM_IF_STATE_REMOVED, NULL, ifm_check_ether_cable, },
    {NULL, "wlan0", IFM_IF_STATE_REMOVED, NULL, ifm_check_wifi_cable, },
 //   {NULL, "wlan1", IFM_IF_STATE_REMOVED, NULL, NULL, },
    {NULL, "ppp0",  IFM_IF_STATE_INSERTED, NULL, NULL, },
    //{NULL, "lo",    IFM_IF_STATE_REMOVED, NULL, NULL, },
};


#define IFM_IF_NUM_MAX (sizeof(ifm_if_node_tbl) / sizeof(ifm_if_node_tbl[0]))

static int g_ifm_inited = 0;


int ifm_register_check_device_cb( char *if_name, if_check_device_cb cb)
{
    int i;
    struct ifm_if_node *p;

    if ( !g_ifm_inited )
        return IFM_NOT_INITED;
    
    if ( !if_name || !cb )
        return IFM_PARAM_INVALID;

    for ( i = 0, p = ifm_if_node_tbl; i < IFM_IF_NUM_MAX; i++, p++ ) {
        if( 0 == strcmp( if_name, p->if_name ) ) {
            p->if_cd_cb = cb;
            return 0;
        }
    }

    return (!if_name) ? 0 : IFM_IF_NOT_FOUND;
}



int ifm_register_state_changed_cb( char *if_name, if_state_changed_cb cb)
{
    int i;
    struct ifm_if_node *p;

    if ( !g_ifm_inited )
        return IFM_NOT_INITED;
    
    if ( !cb )
        return IFM_PARAM_INVALID;

    for ( i = 0, p = ifm_if_node_tbl; i < IFM_IF_NUM_MAX; i++, p++ ) {
        if ( !if_name ) {
            p->if_sc_cb = cb;
        }
        else if( 0 == strcmp( if_name, p->if_name ) ) {
            p->if_sc_cb = cb;
            return 0;
        }
    }

    return (!if_name) ? 0 : IFM_IF_NOT_FOUND;
}



void ifm_init()
{
    int i = 0;

    if ( g_ifm_inited )
        return;

    for ( i = 0; i < IFM_IF_NUM_MAX - 1; i++ ) {
        ifm_if_node_tbl[ i ].next = &ifm_if_node_tbl[ i + 1 ];
    }
    ifm_if_node_tbl[ i ].next = NULL;

    g_ifm_inited = 1;
    IFM_PRINT("ifm_inited OK!\n");
    return;
}


static struct ifm_if_node* ifm_find_if( char *if_name)
{
    int i;
    struct ifm_if_node *p;
    ERR_RET(!if_name, "invalid if_name", NULL);
    for ( i = 0, p = &(ifm_if_node_tbl[0]); i < IFM_IF_NUM_MAX; i++, p++ ) {
        if ( 0 == strcmp( if_name, p->if_name ) ) {
            return ( IFM_IF_STATE_REMOVED == p->if_state ) ? NULL : p;
        }
    }

    return NULL;
}


static struct ifm_if_node* ifm_add_if( char *if_name, struct ifm_if_cfg *if_cfg)
{
    int i;
    struct ifm_if_node *p;
    ERR_RET(!if_name, "invalid if_name", NULL);
    ERR_RET(!if_cfg, "invalid if_cfg", NULL);
    for ( i = 0, p = &(ifm_if_node_tbl[0]); i < IFM_IF_NUM_MAX; i++, p++ ) {
        if ( 0 == strcmp( if_name, p->if_name ) ) {
            ERR_RET( ( IFM_IF_STATE_REMOVED != p->if_state ),
                      "ifm_add_if error: existed", NULL );
            
            memcpy( &p->cfg, if_cfg, sizeof(*if_cfg) );
            p->if_state = IFM_IF_STATE_INSERTED;

            return p;
        }
    }

    return NULL;
}


static int ifm_delete_if( char *if_name)
{
    int i;
    struct ifm_if_node *p;
    ERR_RET(!if_name, "invalid if_name", IFM_PARAM_INVALID);
    for ( i = 0, p = &(ifm_if_node_tbl[0]); i < IFM_IF_NUM_MAX; i++, p++ ) {
        if ( 0 == strcmp( if_name, p->if_name ) ) {
            ERR_RET( ( IFM_IF_STATE_REMOVED == p->if_state ),
                      "ifm_delete_if error: deleted", IFM_IF_DELETED );

            p->if_state = IFM_IF_STATE_REMOVED;
        }
    }

    return 0;
}



int ifm_get_if_cfg( char *if_name, struct ifm_if_cfg *if_cfg )
{
    struct sockaddr_in *addr;
    struct ifreq ifr;
    char *addr_str;
    int sockfd;
    int ret, gotoRet = 0;
    memset(if_cfg, 0x0, sizeof(struct ifm_if_cfg));
    ERR_RET( !if_name, "invalid interface name", IFM_PARAM_INVALID );

    strncpy( ifr.ifr_name, if_name, IFNAMSIZ - 1 );

    sockfd = socket( AF_INET, SOCK_DGRAM, 0 );
    ERR_RET( sockfd < 0, "socket failed", IFM_SOCKET_CREATE_FAILED );

    ret = ioctl( sockfd, SIOCGIFFLAGS, &ifr );
    ERR_GOTO( -1 == ret, "ioctl error: Get IF FLAGS failed", EXIT_0, IFM_IOCTL_WRONG );

    if_cfg->flags = ifr.ifr_flags;

    ret = ioctl( sockfd, SIOCGIFHWADDR, &ifr);
    ERR_GOTO( -1 == ret, "ioctl error: Get IF Hard Address failed", EXIT_0, IFM_IOCTL_WRONG );

    memset( if_cfg->hwaddr, 0, 32 );
    char buf[8];
    memcpy(buf, ifr.ifr_hwaddr.sa_data, 8);
    snprintf( if_cfg->hwaddr, sizeof(if_cfg->hwaddr), "%02x:%02x:%02x:%02x:%02x:%02x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

    ret = ioctl( sockfd, SIOCGIFADDR, &ifr );
    ERR_GOTO( 0 != ret, "ioctl : Get IF Address failed", EXIT_0, ERR_OK );
    
    addr = (struct sockaddr_in *)&(ifr.ifr_addr);
    memcpy( &if_cfg->ip, addr, sizeof(struct sockaddr_in) );
    addr_str = inet_ntoa(addr->sin_addr);
    strncpy( if_cfg->ip_str, addr_str, IP_ADDRESS_LEN );

    ret = ioctl( sockfd, SIOCGIFBRDADDR, &ifr );
    if (ret < 0)
        IFM_PRINT("ioctl : %s Get Broadcast Address failed", if_name);
    else
    {
        addr = (struct sockaddr_in *)&ifr.ifr_broadaddr;
        memcpy( &if_cfg->bc_ip, addr, sizeof(struct sockaddr_in) );
        addr_str = inet_ntoa(addr->sin_addr);
        strncpy( if_cfg->bc_ip_str, addr_str, IP_ADDRESS_LEN );
    }
    ret = ioctl( sockfd, SIOCGIFNETMASK, &ifr );
    if (ret < 0)
        IFM_PRINT("ioctl : %s Get Netmask Address failed", if_name);
    else
    {
        addr = (struct sockaddr_in *)&ifr.ifr_addr;
        memcpy( &if_cfg->netmask, addr, sizeof(struct sockaddr_in) );
        addr_str = inet_ntoa(addr->sin_addr);
        strncpy( if_cfg->netmask_str, addr_str, IP_ADDRESS_LEN );
    }
    gotoRet = getDNSServer(if_cfg->dns1_str, if_cfg->dns2_str);

EXIT_0:    
    close( sockfd );
    return gotoRet;
}

/* unused
int ifm_if_set_gateway(const char * gw)
{
    int sockfd;
    struct rtentry rm;
    struct sockaddr_in ic_gateway ;
    int err;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        IFM_PRINT("socket is -1\n");
        return IFM_SOCKET_CREATE_FAILED;
    }

    memset(&rm, 0, sizeof(rm));

    ic_gateway.sin_family = AF_INET;
    ic_gateway.sin_addr.s_addr = inet_addr(gw);
    ic_gateway.sin_port = 0;

    (( struct sockaddr_in*)&rm.rt_dst)->sin_family = AF_INET;
    (( struct sockaddr_in*)&rm.rt_dst)->sin_addr.s_addr = 0;
    (( struct sockaddr_in*)&rm.rt_dst)->sin_port = 0;

    (( struct sockaddr_in*)&rm.rt_genmask)->sin_family = AF_INET;
    (( struct sockaddr_in*)&rm.rt_genmask)->sin_addr.s_addr = 0;
    (( struct sockaddr_in*)&rm.rt_genmask)->sin_port = 0;

    memcpy((void *) &rm.rt_gateway, &ic_gateway, sizeof(ic_gateway));
    rm.rt_flags = RTF_UP | RTF_GATEWAY;
    if ((err = ioctl(sockfd, SIOCADDRT, &rm)) < 0)
    {
        IFM_PRINT("SIOCADDRT failed , ret->%d\n",err);
        return IFM_IOCTL_WRONG;
    }
    
    return 0;
}
*/

int ifm_get_ap_scan_result(char *if_name, sc_result **p_scan_list)
{
    int gotoRet = 0;
    ERR_RET( !if_name, "invalid interface name", IFM_PARAM_INVALID );
    ERR_RET( !p_scan_list, "invalid scan_list pointer", IFM_PARAM_INVALID );
    gotoRet = APScan(if_name, p_scan_list);

    return gotoRet;
}

/*
flag:
1 -- UP
0 -- DOWN
*/
int ifm_if_up_down ( char* if_name, int flag )
{
    struct ifreq ifr;
    int sockfd;
    int ret, gotoRet = 0;

    ERR_RET( !if_name, "invalid interface name", IFM_PARAM_INVALID );

    sockfd = socket( AF_INET, SOCK_DGRAM, 0 );
    ERR_RET( sockfd < 0, "socket failed", IFM_SOCKET_CREATE_FAILED );


    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);

    ret = ioctl( sockfd, SIOCGIFFLAGS, &ifr );
    ERR_GOTO( -1 == ret,"ioctl error: Get flags failed", EXIT_0, IFM_IOCTL_WRONG );

    strncpy( ifr.ifr_name, if_name, IFNAMSIZ );
    if ( 0 == flag )
        ifr.ifr_flags &= ~IFF_UP;
    else
        ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);

    ioctl( sockfd, SIOCSIFFLAGS, &ifr );

EXIT_0:
    close( sockfd );
    return gotoRet;
}



//same as "ifconfig [if_name] up"
int ifm_if_up ( char* if_name )
{
    return ifm_if_up_down( if_name, 1 );
}


//same as "ifconfig [if_name] down"
int ifm_if_down ( char* if_name )
{
    return ifm_if_up_down( if_name, 0 );
}


/*
udhcpc -i [if_name]
ifconfig [if_name] [ipaddr] netmask [netmaskaddr]
route add default gw [gatewayaddr]
*/
int ifm_set_if_ether_addrs( char *if_name, 
                            int dhcp_enable,
                            char* ipaddr,
                            char* subnet_mask,
                            char* gateway_addr,
                            char* primary_dns,
                            char* second_dns )
{
    int gotoRet = 0;    
    ERR_RET( !if_name, "invalid interface name", IFM_PARAM_INVALID );
    if (dhcp_enable == 0)
    {
        ERR_RET( !ipaddr, "invalid ipaddr", IFM_PARAM_INVALID );
        ERR_RET( !subnet_mask, "invalid subnet_mask", IFM_PARAM_INVALID );
        ERR_RET( !gateway_addr, "invalid gateway_addr", IFM_PARAM_INVALID );
        ERR_RET( !primary_dns, "invalid primary_dns", IFM_PARAM_INVALID );
        if (!second_dns)
            second_dns = primary_dns;
    }
    struct ifm_if_node* if_node = ifm_find_if(if_name);
    ERR_RET( !if_node, "netif not found", IFM_IF_NOT_FOUND );
    ifm_if_down(if_name);
    if (if_node->if_sc_cb)
        if_node->if_sc_cb( 0, if_name, IFM_IF_STATE_DOWN );
    gotoRet = setEtherIpAddr(if_name, dhcp_enable, ipaddr, subnet_mask, gateway_addr, primary_dns, second_dns);

    return gotoRet;
}

int ifm_set_if_wifi_addrs(char* if_name,
                            int dhcp_enable,
                          char* SSID,
                          char* pass_mode,
                          char* password,
                          char* ipaddr,
                          char* subnet_mask,
                          char* gateway_addr,
                          char* primary_dns,
                          char* second_dns
                         )
{
    int gotoRet = 0;
    ERR_RET( !if_name, "invalid interface name", IFM_PARAM_INVALID );
    ERR_RET( !SSID, "invalid SSID", IFM_PARAM_INVALID );
    ERR_RET( !pass_mode, "invalid password mode", IFM_PARAM_INVALID );
    ERR_RET( !password, "invalid password", IFM_PARAM_INVALID );
    if (dhcp_enable == 0)
    {
        ERR_RET( !ipaddr, "invalid ipaddr", IFM_PARAM_INVALID );
        ERR_RET( !subnet_mask, "invalid subnet_mask", IFM_PARAM_INVALID );
        ERR_RET( !gateway_addr, "invalid gateway_addr", IFM_PARAM_INVALID );
        ERR_RET( !primary_dns, "invalid primary_dns", IFM_PARAM_INVALID );
        if (!second_dns)
            second_dns = primary_dns;
    }
    struct ifm_if_node* if_node = ifm_find_if(if_name);
    ERR_RET( !if_node, "netif not found", IFM_IF_NOT_FOUND );
    ifm_if_down(if_name);
    if (if_node->if_sc_cb)
        if_node->if_sc_cb( 0, if_name, IFM_IF_STATE_DOWN );
    gotoRet = setWiFiIpAddr(if_name, dhcp_enable, SSID, pass_mode, password, ipaddr, subnet_mask, gateway_addr, primary_dns, second_dns);

    return gotoRet;
}

int ifm_if_ppp_connect(char* if_name,
                   char* user_name,
                   char* password
                  )
{
    int gotoRet = 0;
    ERR_RET( !if_name, "invalid interface name", IFM_PARAM_INVALID );
    ERR_RET( !user_name, "invalid user_name", IFM_PARAM_INVALID );
    ERR_RET( !password, "invalid password", IFM_PARAM_INVALID );
    gotoRet = PPPoEConnect(if_name, user_name, password);

    return gotoRet;
}

int ifm_if_ppp_disconnect(char* if_name)
{
    int gotoRet = 0;
    ERR_RET( !if_name, "invalid interface name", IFM_PARAM_INVALID );
    gotoRet = PPPoEDisconnect(if_name);

    return gotoRet;
}



/*
state:
1  -- READY
0  -- UP
-1 -- DOWN
*/
int ifm_if_is_ready( char* if_name, int *state )
{
    struct ifreq ifr;
    int sockfd;
    int ret, gotoRet = 0;

    ERR_RET( !if_name, "invalid interface name", IFM_PARAM_INVALID );
    ERR_RET( !state, "invalid parameter state", IFM_PARAM_INVALID );

    sockfd = socket( AF_INET, SOCK_DGRAM, 0 );
    ERR_RET( sockfd < 0, "socket failed", IFM_SOCKET_CREATE_FAILED );

    
    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
    
    ret = ioctl( sockfd, SIOCGIFFLAGS, &ifr );
    ERR_GOTO( -1 == ret,"ioctl error: Get flags failed", EXIT_0, IFM_IOCTL_WRONG );

    *state = ((ifr.ifr_flags) & IFF_UP) ? 0 : -1;
    ERR_GOTO( -1 == *state,"ifm: netif not up", EXIT_0, ERR_OK );
    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
    ifr.ifr_addr.sa_family = AF_INET;
    ret = ioctl( sockfd, SIOCGIFADDR, &ifr );
    ERR_GOTO( 0 != ret,"ioctl : Get ip addr failed", EXIT_0, ERR_OK );
    *state = 1;
        
EXIT_0:
    close( sockfd );
    return gotoRet;
}

int ifm_check_network()
{
    int i, state, ret, gotoRet = 0;
    for (i = 0; i < IFM_IF_NUM_MAX; i++)
    {
        ret = ifm_if_is_ready(ifm_if_node_tbl[i].if_name, &state);
        if (ret == 0 && state == 1)
            break;
        else
            state = 0;
    }
    if (state == 1)
        return 1;
    else
        return 0;
}



int ifm_ap_scan(char* if_name, sc_result **p_scan_list)
{
    int state;
    int ret;
    ret = ifm_check_wifi_cable(0, if_name, &state);
    ERR_RET(0 > ret || state == IFM_IF_STATE_REMOVED, "netif not inserted", IFM_WIFI_NOT_INSERTED);
    return APScan(if_name, p_scan_list);   
}

void ifm_free_ap_list(sc_result** p_scan_list)
{
    freeAPList(p_scan_list);
}


/*
Check that ether cable is inserted or remvoed by MII method.

MII: Media-Independant Interface

Macros below are defined in /usr/include/linux/sockios.h
SIOCGMIIPHY Get address of MII PHY in use.
SIOCGMIIREG Read MII PHY register.
SIOCSMIIREG Write MII PHY register.

Return Value:
0 -- OK
-1: failed;
-2: Unsupported

state:
if check ok, returns IFM_IF_STATE_REMOVED/IFM_IF_STATE_INSERTED
*/
static int check_ether_cable_by_mii( char *if_name, int *state )
{
    struct ifreq ifr;
    u16 *data, mii_val;
    unsigned phy_id;
    int sockfd;
    int ret, gotoRet = 0;

    ERR_RET( !if_name, "check_ether_cable_by_mii: invalid interface name", IFM_PARAM_INVALID );

    sockfd = socket( AF_INET, SOCK_DGRAM, 0 );
    ERR_RET( sockfd < 0, "check_ether_cable_by_mii: Create socket failed", IFM_SOCKET_CREATE_FAILED );

    strncpy( ifr.ifr_name, if_name, IFNAMSIZ );

    ret = ioctl( sockfd, SIOCGMIIPHY, &ifr );
    ERR_GOTO( -1 == ret,"ioctl error: SIOCGMIIPHY failed", EXIT_0, IFM_IOCTL_WRONG );
    
    data = (u16 *)(&ifr.ifr_data);
    phy_id = data[0];
    data[1] = 1;


    ret = ioctl( sockfd, SIOCGMIIREG, &ifr );
    ERR_GOTO( -1 == ret,"ioctl error: SIOCGMIIREG failed", EXIT_0, IFM_IOCTL_WRONG );
    
    mii_val = data[3];

    *state = (((mii_val & 0x0016) == 0x0004) ? IFM_IF_STATE_INSERTED : IFM_IF_STATE_REMOVED );

EXIT_0:
    close(sockfd);
    return gotoRet;
}


/*
Check that ether cable is inserted or remvoed by rthtool method.

Return Value:
0 -- OK
-1: failed;
-2: Unsupported

state:
if check ok, returns IFM_IF_STATE_REMOVED/IFM_IF_STATE_INSERTED
*/
#ifndef NETWORKMON_PLATFORM_ARC
static int check_ether_cable_by_ethtool( char *if_name, int *state )
{
    struct ifreq ifr;
    struct ethtool_value edata;
    int sockfd;
    int ret, gotoRet = 0;

    ERR_RET( !if_name, "check_ether_cable_by_ethtool: invalid interface name", IFM_PARAM_INVALID );

    sockfd = socket( AF_INET, SOCK_DGRAM, 0 );
    ERR_RET( sockfd < 0, "check_ether_cable_by_ethtool: Create socket failed", IFM_SOCKET_CREATE_FAILED );

    memset(&ifr, 0, sizeof(ifr));

    edata.cmd = ETHTOOL_GLINK;
    ifr.ifr_data = (char *) &edata;

    strncpy( ifr.ifr_name, if_name, IFNAMSIZ );

    ret = ioctl( sockfd, SIOCETHTOOL, &ifr );
    ERR_GOTO( -1 == ret,"ioctl error: SIOCETHTOOL failed", EXIT_0, IFM_IOCTL_WRONG );

    *state = (edata.data ? IFM_IF_STATE_INSERTED : IFM_IF_STATE_REMOVED);
    
EXIT_0:
    close(sockfd);
    return gotoRet;
}
#endif

int ifm_check_ether_cable( int if_id, char *if_name, int *state )
{
    int ret;
    
    if_id = if_id;
    
#ifdef NETWORKMON_PLATFORM_ARC
    ret = check_ether_cable_by_mii( if_name, state );
#else
    ret = check_ether_cable_by_ethtool( if_name, state );
#endif

    return ret;
}

int ifm_check_wifi_cable(int if_id, char *if_name, int *state)
{
    int ret;
    if_id = if_id;
    ret = check_wifi_cable(if_name);
    *state = (ret == 0 ? IFM_IF_STATE_INSERTED : IFM_IF_STATE_REMOVED);
    return 0;
}
//unused
/*int check_ppp_cable(char *if_name)
{
    char tmpCmd[128];
    memset(tmpCmd, 0x0, sizeof(tmpCmd));
    snprintf(tmpCmd, sizeof(tmpCmd), "ifconfig %s >> /tmp/temp", if_name);
    int ret = c_system(tmpCmd, 0);
    if (ret != 0)
    {
        
    }
    c_system("rm -f /tmp/temp", 0);
    return ret;
}

int ifm_check_ppp_cable(int if_id, char *if_name, int *state)
{
    int ret;
    if_id = if_id;
    ret = check_ppp_cable(if_name);
}
*/

void ifmd_polling_check()
{
    struct ifm_if_cfg if_cfg;
    int ret;
    int i;
/*    if ((pid = fork()) < 0)
    {
        return IFM_IFMD_START_FAILED;
    }
    else if (pid != 0)
    {   
        
        sleep(2);
        return 0;
    }
    setsid();
*/
    if (g_ifm_inited != 1)
    {
        perror("ifm not inited!\n");
        return;
    }
    while ( 1 ) {
        int state;
        char *if_name = NULL;

        for ( i = 0; i < IFM_IF_NUM_MAX; i++ ) {
            
            if_name = ifm_if_node_tbl[i].if_name;
            
            if ( !ifm_if_node_tbl[i].if_cd_cb ) {
                IFM_PRINT("%s check device callback undefined\n", if_name);
                continue;
            }

            ret = ifm_if_node_tbl[i].if_cd_cb( 0, if_name, &state );
            if ( 0 != ret ) {
                IFM_PRINT("failed to check %s INSERTED/REMOVED\n", if_name);
                continue;
            }
            
            struct ifm_if_node * if_node = ifm_find_if( if_name );

            if ( (IFM_IF_STATE_REMOVED == state) && !if_node ) {
                //do nothing
            }
            else if ( IFM_IF_STATE_REMOVED == state ) {
                if_node->if_state = IFM_IF_STATE_REMOVED;
                if (strncmp(if_name, "wlan0", 5) == 0)
                {
                    killWifiDhcp(if_name);
                }
                if (if_node->if_sc_cb)
                    if_node->if_sc_cb( 0, if_name, IFM_IF_STATE_REMOVED );
                else
                    IFM_PRINT("failed to notify %s's state changed to removed\n", if_name);
                ifm_delete_if( if_name );
            }
            else if ( /*IFM_IF_STATE_REMOVED != state && */ if_node ) {
                //do nothing
            }
            else/*( IFM_IF_STATE_REMOVED != state && !if_node )*/ {
                if_node = ifm_add_if( if_name, &if_cfg );
                if ( !if_node ) {
                    IFM_PRINT("ERROR: ifm_add_if(%s) failed when check INSERTED/REMOVED\n", if_name);
                    continue;
                }
                if_node->if_state = IFM_IF_STATE_INSERTED;
                if (strncmp(if_name, "wlan0", 5) == 0)
                    wpa_up();
                if (if_node->if_sc_cb)
                    if_node->if_sc_cb( 0, if_name, IFM_IF_STATE_INSERTED );
                else
                    IFM_PRINT("failed to notify %s's state changed to inserted\n", if_name);

            }
        }

        for ( i = 0; i < IFM_IF_NUM_MAX; i++ ) {
            if_name = ifm_if_node_tbl[i].if_name;
            
            ret = ifm_if_is_ready( if_name, &state );
            if ( 0 != ret ) {
                IFM_PRINT("failed to check %s ready/down\n", if_name);
                continue;
            }
            
            struct ifm_if_node * if_node = ifm_find_if( if_name );
            if ( !if_node ) {
                IFM_PRINT("ERROR: %s can be checked ready/down, but no record in IFM\n", if_name);
                continue;
            }

            if ( (0 >= state) && 
                ( IFM_IF_STATE_INSERTED == if_node->if_state || IFM_IF_STATE_READY == if_node->if_state) ) {
                if_node->if_state = IFM_IF_STATE_DOWN;
                if (if_node->if_sc_cb)
                    if_node->if_sc_cb( 0, if_name, IFM_IF_STATE_DOWN );
                else
                    IFM_PRINT("failed to notify %s's state changed to down\n", if_name);

            }

            if ( (1 == state) && 
                ( IFM_IF_STATE_INSERTED == if_node->if_state || IFM_IF_STATE_DOWN == if_node->if_state) ) {
                if_node->if_state = IFM_IF_STATE_READY;
                if (if_node->if_sc_cb)
                    if_node->if_sc_cb( 0, if_name, IFM_IF_STATE_READY );
                else
                    IFM_PRINT("failed to notify %s's state changed to ready\n", if_name);

            }
        }
        sleep(1);		
    }
    return;
}

int ifmd_start()
{
    pthread_t pid;
    int ret = pthread_create(&pid, NULL, (void *)ifmd_polling_check, NULL);
    if (ret != 0)
        return IFM_IFMD_START_FAILED;
    return ERR_OK;
}                 
