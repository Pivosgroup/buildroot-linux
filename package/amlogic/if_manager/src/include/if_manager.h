/*******************************************************************
 *
 *   Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
 *
 *   Description: the interface of if_manager for UI
 *
 *   Author: Gaokj tianyu.li
 *
 *******************************************************************/

/*!\file if_manager.h
 * \brief the interface of if_manager for UI
 * \author Gaokj tianyu.li
 * \date 2010.3.10
 * \example ifm_test.c
 * the example of using if_manager
 */

#ifndef __IF_MANAGER_H__  
#define __IF_MANAGER_H__

#include "ifm_def.h"


/*!
 * register a callback for a netif, when the state of the netif changed, it will be called
 */
int ifm_register_state_changed_cb( char *if_name, if_state_changed_cb cb);

/*!
 * register a callback to check the status of the netif, the eth,wlan and ppp have registered the default check_device_cb
 * if you want to change it, or you want to add a new netif to the netif_list, you should use this function 
 * else pls do not change the default ones 
 */
int ifm_register_check_device_cb( char *if_name, if_check_device_cb cb);

/*!
 * get the infomation of a netif
 * \param if_name [I]: the name of the netif
 * \param if_cfg [I]: the information struct
 */
int ifm_get_if_cfg( char *if_name, struct ifm_if_cfg *if_cfg );

/*!
 * set the ethernet ip addrs
 */
int ifm_set_if_ether_addrs( char* if_name, 
                              int dhcp_enable,
                            char* ipaddr,
                            char* subnet_mask,
                            char* gateway_addr,
                            char* primary_dns,
                            char* second_dns);
/*!
 * set the wifi ip addrs
 */
int ifm_set_if_wifi_addrs(char* if_name,
                            int dhcp_enable,
                          char* SSID,
                          char* pass_mode,
                          char* password,
                          char* ipaddr,
                          char* subnet_mask,
                          char* gateway_addr,
                          char* primary_dns,
                          char* second_dns);

/*!
 * setup PPPoE and start the connection
 */
int ifm_if_ppp_connect(char* if_name,
                       char* user_name,
                       char* password);

/*!
 * stop PPPoE connection
 */
int ifm_if_ppp_disconnect(char* if_name);


/*!
 * get the result list of ap_scan
 * \param if_name [I]: the name of the netif
 * \param p_scan_list [I]: the pointer to the list node struct pointer. if get the list, *p_scan_list is the first node's pointer. if not, *p_scan_list is NULL.
 */
int ifm_get_ap_scan_result(char* if_name, sc_result** p_scan_list);

/*!
 * free the ap scan result list.
 * after UI showed the list, you must free the list by calling this function
 * \param p_scan_list [I]: p_scan_list, the pointer to the list node struct pointer. after free the list, *p_scan_list should be NULL. 
 */
void ifm_free_ap_list(sc_result** p_scan_list);

/*!
 * up the interface, equal to "ifconfig [if_name] up"
 */
int ifm_if_up ( char* if_name);

/*!
 * down the interface, equal to "ifconfig [if_name] down"
 */
int ifm_if_down ( char* if_name );

/*!
 * check the interface 
 * \param if_name [I]: the name of the interface
 * \param state [O]: 1 -- READY, 0 -- UP, -1 -- DOWN 
 */
int ifm_if_is_ready( char* if_name, int *state );

/*!
 * check the network ,if one netif is ready, state will be connected, else it will be disconnected.
 * \return : 1 -- connected, 0 -- disconnected
 */
int ifm_check_network( void );


/*!
 * init the if_manager, call it first and then you can use the other functions
 */
void ifm_init();

/*!
 * start the if_manager daemon, must be called after ifm_init() and register the state_changed_cb
 */
int ifmd_start();

#endif/*__IF_MANAGER_H__*/



