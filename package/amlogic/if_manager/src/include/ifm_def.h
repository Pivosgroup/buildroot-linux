/******************************************************************
 *
 *  Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: the data structure define header file
 *
 *  Author: tianyu.li
 *
 ******************************************************************/

/*!\file ifm_def.h
 * \brief if_manager common types defination
 *
 * \author tianyu.li
 * \date 2010.3.10
 */

#ifndef __IFM_DEF_H__
#define __IFM_DEF_H__

/*netif states*/
#define IFM_IF_STATE_REMOVED       0x1
#define IFM_IF_STATE_INSERTED      0x2
#define IFM_IF_STATE_DOWN          0x3
#define IFM_IF_STATE_READY         0x4




#define ERR_OK                     0  /*!< return successfully */

/*IFM_ERRORS*/
#define IFM_NOT_INITED           -20
#define IFM_PARAM_INVALID        -30
#define IFM_SOCKET_CREATE_FAILED -40
#define IFM_IOCTL_WRONG          -50
#define IFM_IF_DELETED           -60
#define IFM_IF_NOT_FOUND         -70
#define IFM_IFMD_START_FAILED    -80
#define IFM_WIFI_NOT_INSERTED    -90

/*network ERRORS*/
#define SSID_TOO_LONG             -1
#define PASSWORD_TOO_LONG         -2
#define AP_CONN_FAILED            -3
#define PPPOE_CONN_FAILED         -4
#define PASSWORD_ILEGAL           -5
#define PASSWORD_NUM_ERR          -6
#define NO_PASSWORD               -7

#define OPEN_DNS_CONF_FAILED      -8
#define WRITE_DNS_CONF_FAILED     -9
#define READ_DNS_CONF_FAILED     -10
#define CLOSE_DNS_CONF_FAILED    -11
#define DNS2_FORMAT_ILEGAL       -12
#define DNS_POINTER_NULL         -13

#define SOCKET_CREATE_FAILED     -14
#define GET_STATS_FAILED         -15

#define FORMAT_WRONG             -16

#define INTERFACE_NOT_FOUND      -17
#define DHCP_TIMEOUT             -18
#define DHCP_FAILED              -19

#define IP_FORMAT_ILEGAL         -21

#define OPEN_AP_RESULT_WRONG     -22
#define READ_AP_RESULT_WRONG     -23



/*some size defines*/
#define SSID_SIZE                 20
#define PASS_MODE_SIZE             5
#define IP_ADDRESS_LEN            32

/*debug print mux*/
//#define DEBUG_PRINT
#ifdef DEBUG_PRINT
#define IFM_PRINT printf    /*!< open the debug print */
#else
#define IFM_PRINT(...)      /*!< close the debug print */
#endif

//#define USE_NAND          /*!< FOR double system only: get the network setting params from NAND */


/*!
 * \brief the strings of the states of netif
 */
static char *ifm_if_state_descs[] = {
    "UNUESD",
    "REMOVED",
    "INSERTED",
    "DOWN",
    "READY",
};

typedef void (*if_state_changed_cb)( int if_id, char *if_name, int state );
typedef int (*if_check_device_cb)( int if_id, char *if_name, int *state );

/*!
 * \brief the struct of the full information of one netif
 */
struct ifm_if_cfg {

    short flags;                            /*!< the state flags of the interface*/

    struct sockaddr_in ip;                  /*!< ip address*/
    char ip_str[IP_ADDRESS_LEN];            /*!< the string of ip address*/
    
    struct sockaddr_in bc_ip;               /*!< broadcast address*/
    char bc_ip_str[IP_ADDRESS_LEN];         /*!< the string of broadcast address*/
    
    struct sockaddr_in gateway;             /*!< gateway address*/
    char gateway_str[IP_ADDRESS_LEN];       /*!< the string of gateway address*/

    struct sockaddr_in netmask;             /*!< netmask address*/
    char netmask_str[IP_ADDRESS_LEN];       /*!< the string of netmask address*/
    
    char dns1_str[IP_ADDRESS_LEN];          /*!< the string of primary dns server address*/
    char dns2_str[IP_ADDRESS_LEN];          /*!< the string of second dns server address*/

    char hwaddr[32];                        /*!< HW address*/
};

/*!
 * \brief the result list of AP scanning
 */
typedef struct scan_result{

    struct scan_result *next;               /*!< pointer to the next node*/

    char ssid[SSID_SIZE];                   /*!< ssid*/

    char pass_mode[PASS_MODE_SIZE];         /*!< pass_mode:now should be NONE, WEP, WPA*/

    int strength;                           /*!< the signal level of the AP, should between 0 and 100*/

} sc_result;

/*!
 * set the network by using the settings of AVOS, be used when the system startup
 */
int setNetwork(void);

#endif /*__IFM_DEF_H__*/
