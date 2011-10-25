/*******************************************************************
 * 
 *  Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: the interface of network_setting
 *
 *  Author: tianyu.li
 *
 *******************************************************************/

#ifndef __NETWORK_SETTING_H__
#define __NETWORK_SETTING_H__
#include "ifm_def.h"
#if 0
typedef struct {
        char name[IF_NAME_SIZE];                /* interface name        */
        short type;                             /* if type               */
        short flags;                            /* various flags         */
        char addr[IP_ADDR_SIZE];                /* IP address            */
        char broadaddr[IP_ADDR_SIZE];           /* IP broadcast address  */
        char netmask[IP_ADDR_SIZE];             /* IP network mask       */
        int has_ip;
        char hwaddr[32];                        /* HW address            */
}if_stats;
#endif



int setEtherIpAddr(char* netif_id,
    		     int dhcp_enable,
    		   char* ipaddr,
    		   char* subnet_mask,
    		   char* gateway_addr,
    		   char* primary_dns,
    		   char* second_dns
		);

int setWiFiIpAddr(char* netif_id,
                    int dhcp_enable,
		  char* SSID,
                  char* pass_mode,
                  char* password,
                  char* ipaddr,
                  char* subnet_mask,
                  char* gateway_addr,
                  char* primary_dns,
                  char* second_dns
		);

int PPPoEConnect(char* netif_id,
		 char* user_name,
		 char* password 
		);
int PPPoEDisconnect();

int getDNSSever(char *dns1, char *dns2);

int wpa_up();

void killWifiDhcp(char* netif_id);

int APScan(char* netif_id, sc_result** p_scan_list);

void freeAPList(sc_result **p_scan_list);

int check_wifi_cable(char *if_name);

//int check_ppp_cable(char *if_name);

#endif /*__NETWORK_SETTING_H__*/
