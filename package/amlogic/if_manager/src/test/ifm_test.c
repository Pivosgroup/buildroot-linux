/************************************************************************
 * 
 *  Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: ifm_test for user, display the sample use of if_manager
 *
 *  Author: tianyu.li
 *
 ************************************************************************/

#include <stdio.h>
#include <string.h>

/*the ip_addr header files*/
#include <sys/socket.h>
#include <netinet/in.h>

/*if_manager interface file, it included ifm_def.h already*/
#include "if_manager.h"

typedef struct {
    char ifname[10];
    int  state_index;
}netif_node;

#define MAX_NETIF_NUM 3

netif_node netifs[MAX_NETIF_NUM];
static int i;

/*the callback to watch the netif state change*/
void SMG_notify_network_status_cb( int if_id,char *if_name, int state )
{
    /*print the state of the netif*/
    int i ;
    int new_index = 0;
    for (i = 0; i < 3; i++)
    {
        if (netifs[i].ifname[0] == 0x0 && new_index == 0)
            new_index = i;
        else if (strncmp(netifs[i].ifname, if_name, strlen(if_name)) == 0)
        {
            netifs[i].state_index = state;
            break;
        } 
    }
    if (i == 3)
    {
        if (new_index == 0)
        {
            printf("no free netif to insert!\n");
            return;
        }
        strncpy(netifs[new_index].ifname, if_name, strlen(if_name));
        netifs[new_index].state_index = state;
        i = new_index;
    }
    printf( "%s %s\n", netifs[i].ifname, ifm_if_state_descs[ netifs[i].state_index] );
    return;
}


int main()
{
    struct ifm_if_cfg if_cfg;
    int ret, state;
    memset(netifs, 0x0, sizeof(netifs));
/*to start the if_manager, you must finish the following steps:
 *first, call ifm_init() to init netif list
 *second, call ifm_register_state_changed_cb() to register callback to the netif list
 *last, call ifmd_start() to start the daemon process
 *NOTE: DO NOT change the order! and DO call them before you use if_manager*/
    ifm_init();
    ret = ifm_register_state_changed_cb( NULL, SMG_notify_network_status_cb );
    if (ret < 0)
        printf("can not register state changed cb!\n");
    int i = ifmd_start();
    if (i == IFM_IFMD_START_FAILED)
    {
        printf("wrong!wrong!wrong!\n");
        return -1;
    }
    
    sleep(3); 
    
    while(1)/*endless*/
    {
        /*test for getting interface configure*/
        for ( i = 0; i < 3; i++ ) 
        {
            if (netifs[i].ifname[0] == 0x0)
                continue;
            ret = ifm_get_if_cfg( netifs[i].ifname, &if_cfg );
            
            printf( "interface: %s\n",netifs[i].ifname );
            if (strncmp(netifs[i].ifname, "wlan0", 5) == 0)
            {
                /*test if the netif has ip*/
                if (ret == 0 && if_cfg.ip_str[0] != 0)
                    printf("wlan0 is ready!\n");
                else
                {
                    /*to get the ap scan result, you should declare a variable sc_result*, to get the list head*/
                    sc_result *p_scan_list, *p;
                    /*scan and get the result*/
                    if((ret = ifm_ap_scan(netifs[i].ifname, &p_scan_list)))
                    {
                        printf("error: %d\n", ret);
                        continue;
                    }
                    p = p_scan_list;
                    while (p != NULL)
                    {
                        printf( "ssid: %s\n", p->ssid );
                        printf( "pass_mode: %s\n", p->pass_mode );
                        printf( "strength: %d\n",p->strength );
                        if(strstr(p->ssid, "AML_BJ"))
                        {
                            /*start to connect to the chosen AP, and the network*/
                            ret = ifm_set_if_wifi_addrs(netifs[i].ifname, 1, p->ssid, p->pass_mode, "aml11", NULL, NULL, NULL, NULL, NULL);
                            printf("set wifi ret: %d\n", ret);
                        }
                        p = p->next;
                    }
                    if (p_scan_list)
                        ifm_free_ap_list(&p_scan_list);
                }
            }
            if ( 0 == ret ) 
            {
                /*print the ip information of all netif which has got ip*/
                printf( "inet addr: %s\n",if_cfg.ip_str );
                printf( "broadcast addr: %s\n",if_cfg.bc_ip_str );
                printf( "inet mask: %s\n",if_cfg.netmask_str );
                printf( "gateway: %s\n",if_cfg.gateway_str );
                if (*(if_cfg.dns1_str) != 0x0)
                    printf( "primary dns addr: %s\n",if_cfg.dns1_str );
                else
                    printf("can not got primary dns addr!\n");
                if (*(if_cfg.dns2_str) != 0x0)
                    printf( "second dns addr: %s\n",if_cfg.dns2_str );
                else
                    printf("can not got second dns addr!\n");
            }
            printf( "\n" );
        }
        printf("the network is %s\n", ifm_check_network() ? "connected" : "disconnected");
        sleep(3);
    }
}

