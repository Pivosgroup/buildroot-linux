/*******************************************************************
 * 
 *  Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: network param setting functions
 *
 *  Author: tianyu.li
 *
 *******************************************************************/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "network_setting.h"


#ifdef USE_NAND
#include "get_netconfig.h"
#endif

#define MAX_SSID_LEN 40
#define MAX_PASS_LEN 40

/*modify the system() func
 *param: in: cmdString, the command string
 *       in: timeout, the timeout to stop the command when calling process can not stand, counted in seconds
 */
static int c_system(const char* cmdString, int timeout)
{
    pid_t pid;
    int status;
    if (cmdString == NULL)
        return (1);
    if ((pid = fork()) < 0)
    {
        status = -1;
    }
    else if (pid == 0)
    {
        execl("/bin/sh", "sh", "-c", cmdString, (char*)0);
        _exit(127);
    }
    else
    {
        int timer = 0;
        int ret;
        if (timeout == 0)
        {
            while(waitpid(pid, &status, 0) < 0)
            {
                if (errno != EINTR)
                {
                    status = -1;
                    break;
                }
            }
            return status;
        }
        while(timer <= timeout)
        {
            if ((ret = waitpid(pid, &status, WNOHANG)) == 0)
            {
                if (timer >= timeout)
                {
                    /*kill the child process when timeout*/
                    ret = kill(pid, SIGKILL);
                    if (ret < 0)
                    {
                        IFM_PRINT("c_system(): kill %d failed!\n", pid);
                        return 127;
                    }
                    else
                    {
                        IFM_PRINT("c_system(): %s running timed out, killed it! pid = %d\n", cmdString, pid);
                        return 128;
                    }
                }
                else
                {
                    sleep(1);
                    timer++;
                }
            }
            else if (ret < 0)
            {
                if (errno != EINTR)
                {
                    status = -1;
                    break;
                }
            }
            else  //cmdString exited
                break;
        }
    }
    return status;
}

/* set the wifi ip address. if the param--dhcp_enable is 1, the function only check the first 5 params, other wise it will 
 * check the whole params
 * the function uses wpa_cli to set the ssid, password and passmode
 * udhcpc -i [if_name]
 * ifconfig [if_name] [ipaddr] netmask [netmaskaddr]
 * route add default gw [gatewayaddr]
 */
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
                )
{
    char tmpCmd[128]; //for assembling shell command
    c_system("killall udhcpc", 0);
    if (SSID && (SSID[0] != 0))
    {
        if (strlen(SSID) >= MAX_SSID_LEN)
        {
            IFM_PRINT("SSID is too long!\n");
            return SSID_TOO_LONG;
        }
        c_system("wpa_cli -p/var/run/wpa_supplicant remove_network 0", 0);
        c_system("wpa_cli -p/var/run/wpa_supplicant add network", 0);
        memset(tmpCmd, 0x0, sizeof(tmpCmd));
        snprintf(tmpCmd, sizeof(tmpCmd), "wpa_cli -p/var/run/wpa_supplicant set_network 0 ssid '\"%s\"'", SSID);
        c_system(tmpCmd, 0);
        if (strncmp(pass_mode, "NONE", 4) == 0)
        {
            c_system("wpa_cli -p/var/run/wpa_supplicant set_network 0 key_mgmt NONE", 0);
        }
        else if (password)
        {
            if (strlen(password) >= MAX_PASS_LEN)
            {
                IFM_PRINT("Password is too long!\n");
                return PASSWORD_TOO_LONG;
            }
            memset(tmpCmd, 0x0, sizeof(tmpCmd));
            c_system("wpa_cli -p/var/run/wpa_supplicant set_network 0 key_mgmt NONE", 0);
            if (strncmp(pass_mode, "WEP", 3) == 0)
            {
                int num = strlen(password);
                if(num==5 || num==13)
                {   
                    snprintf(tmpCmd, sizeof(tmpCmd), "wpa_cli -p/var/run/wpa_supplicant set_network 0 wep_key0 '\"%s\"'", password);
                    c_system(tmpCmd, 0);
                }
                else if(num==10 ||num==26)
                {
                    int i;
                    for(i=0;i<num;i++)
                    {
                        if(!((('0'<=*(password+i))&&(*(password+i)<='9'))||('A'<=*(password+i)&&(*(password+i)<='F'))||(('a'<=*(password+i))&&(*(password+i)<='f'))))
                        {
                            return PASSWORD_ILEGAL;//ilegal character
                        }
                    }
                    snprintf(tmpCmd, sizeof(tmpCmd), "wpa_cli -p/var/run/wpa_supplicant set_network 0 wep_key0 %s", password);
                    c_system(tmpCmd, 0);
                }		
                else
                {
                    return PASSWORD_NUM_ERR;//erro number	
                }
                c_system("wpa_cli -p/var/run/wpa_supplicant set_network 0 wep_tx_keyidx 0", 0);
            }
            else if (strncmp(pass_mode, "WPA", 3) == 0)
            {
                snprintf(tmpCmd, sizeof(tmpCmd), "wpa_cli -p/var/run/wpa_supplicant set_network 0 psk '\"%s\"'", password);
                c_system(tmpCmd, 0);
            }
        }
        else
            return NO_PASSWORD;
        c_system("wpa_cli -p/var/run/wpa_supplicant select_network 0", 0);
        if (dhcp_enable == 1)
        {
            memset(tmpCmd, 0x0, sizeof(tmpCmd));
            snprintf(tmpCmd, sizeof(tmpCmd), "udhcpc -i %s", netif_id);
            int ret = c_system(tmpCmd, 20);
            if (ret == 128)
            {
                c_system("killall udhcpc", 0);
                IFM_PRINT("dhcp time out\n");
                return DHCP_TIMEOUT;
            }
            else if (ret != 0)
                return DHCP_FAILED;
            return ret;
        }
        else
        {
            if (inet_addr(ipaddr) == -1 || inet_addr(subnet_mask) == -1 || inet_addr(gateway_addr) == -1 || inet_addr(primary_dns) == -1 || inet_addr(second_dns) == -1)
                return IP_FORMAT_ILEGAL;
            memset(tmpCmd, 0x0, sizeof(tmpCmd));
            snprintf(tmpCmd, sizeof(tmpCmd), "ifconfig %s %s netmask %s", netif_id, ipaddr, subnet_mask);
            c_system(tmpCmd, 0);
            memset(tmpCmd, 0x0, sizeof(tmpCmd));
            snprintf(tmpCmd, sizeof(tmpCmd), "route add default gw %s", gateway_addr);
            c_system(tmpCmd, 0);
            int fd = open("/etc/resolver.conf", O_WRONLY | O_CREAT, 0);
            if (fd >= 0)
            {
                memset(tmpCmd, 0x0, sizeof(tmpCmd));
                snprintf(tmpCmd, sizeof(tmpCmd), "nameserver %s\nnameserver %s", primary_dns, second_dns);
                int nbytes = write(fd, tmpCmd, strlen(tmpCmd));
                if (nbytes >= 0)
                {
                    int err = close(fd);
                    if (err != 0)
                        return CLOSE_DNS_CONF_FAILED;
                    return ERR_OK;
                }
                else
                {
                    close(fd);
                    return WRITE_DNS_CONF_FAILED;
                }   
            }
            else
                return OPEN_DNS_CONF_FAILED;
        }
    }
    return ERR_OK;
}

/* set the ether ip address. if the param--dhcp_enable is 1, the function only check the first 2 params, other wise it will 
 * check the whole params
 * udhcpc -i [if_name]
 * ifconfig [if_name] [ipaddr] netmask [netmaskaddr]
 * route add default gw [gatewayaddr]
 */
int setEtherIpAddr(char* netif_id,
                     int dhcp_enable,
                   char* ipaddr,
                   char* subnet_mask,
                   char* gateway_addr,
                   char* primary_dns,
                   char* second_dns
                )
{
    char tmpCmd[256]; //for assembling shell commands
    c_system("killall udhcpc", 0);
    if (dhcp_enable == 1)
    {
        memset(tmpCmd, 0x0, sizeof(tmpCmd));
        snprintf(tmpCmd, sizeof(tmpCmd), "udhcpc -i %s", netif_id);
        int ret = c_system(tmpCmd, 20);
        if (ret == 128)
        {
            c_system("killall udhcpc", 0);
            IFM_PRINT("dhcp time out\n");
            return DHCP_TIMEOUT;
        }
        else if (ret != 0)
            return DHCP_FAILED;

        return ret;
    }
    else
    {
        if (inet_addr(ipaddr) == -1 || inet_addr(subnet_mask) == -1 || inet_addr(gateway_addr) == -1 || inet_addr(primary_dns) == -1 || inet_addr(second_dns) == -1)
            return IP_FORMAT_ILEGAL;
        memset(tmpCmd, 0x0, sizeof(tmpCmd));
        snprintf(tmpCmd, sizeof(tmpCmd), "ifconfig %s %s netmask %s", netif_id, ipaddr, subnet_mask);
        c_system(tmpCmd, 0);
        memset(tmpCmd, 0x0, sizeof(tmpCmd));
        snprintf(tmpCmd, sizeof(tmpCmd), "route add default gw %s", gateway_addr);
        c_system(tmpCmd, 0);
        int fd = open("/etc/resolver.conf", O_WRONLY | O_CREAT, 0);
        if (fd >= 0)
        {
            memset(tmpCmd, 0x0, sizeof(tmpCmd));
            snprintf(tmpCmd, sizeof(tmpCmd), "nameserver %s\nnameserver %s", primary_dns, second_dns);
            int nbytes = write(fd, tmpCmd, strlen(tmpCmd));
            if (nbytes >= 0)
            {
                int err = close(fd);
                if (err != 0)
                    return CLOSE_DNS_CONF_FAILED;
                return ERR_OK;
            }
            else
            {
                close(fd);
                return WRITE_DNS_CONF_FAILED;
            }
        }
        else
            return OPEN_DNS_CONF_FAILED;
    }
}

int getDNSServer(char *dns1, char *dns2)
{
    if (dns1 == NULL || dns2 == NULL)
        return DNS_POINTER_NULL;
    c_system("cat /etc/resolv.conf | grep nameserver >>/tmp/temp", 0);
    int fd = open("/tmp/temp", O_RDONLY, 0);
    if (fd >= 0)
    {
        char buf[128];
        int nbytes = read(fd, buf, sizeof(buf));
        if (nbytes <= buf)
        {
            char *dlim = " \n\t";
            char *tok;
            char *start = strstr(buf, "nameserver ");
            close(fd);
            if (start)
            {
                tok = strtok(start, dlim);
                if (strncmp(tok, "nameserver", 10) == 0)
                {
                    tok = strtok(NULL, dlim);
                    if (inet_addr(tok) == -1)
                    {
                        *dns1 = 0x0;
                        return IP_FORMAT_ILEGAL;
                    }
                    strncpy(dns1, tok, strlen(tok) + 1);
                    tok = strtok(NULL, dlim);
                    if (strncmp(tok, "nameserver", 10) == 0)
                    {
                        tok = strtok(NULL, dlim);
                        if (inet_addr(tok) == -1)
                        {
                            *dns2 = 0x0;
                            return DNS2_FORMAT_ILEGAL;
                        }
                        strncpy(dns2, tok, strlen(tok) + 1);
                        return ERR_OK;
                    }
                    else
                        return FORMAT_WRONG;
                }
                else
                    return FORMAT_WRONG;
            }
            else
                return FORMAT_WRONG; 
        }
        else
        {
            close(fd);
            return READ_DNS_CONF_FAILED;
        }
    }
    return OPEN_DNS_CONF_FAILED;
}


/*call pppoe-setup and pppoe-start scripts*/
int PPPoEConnect(char* netif_id,
                 char* user_name,
                 char* password
                )
{
    char tmpCmd[128]; //for assembling shell conmands
    snprintf(tmpCmd, sizeof(tmpCmd), "pppoe-setup %s %s %s", user_name, password, netif_id);
    int ret = c_system(tmpCmd, 0);
    if (ret == 0)
    {
        ret = c_system("pppoe-start", 0);
    }
    return ret;
}

int PPPoEDisconnect()
{
    int ret = c_system("pppoe-stop", 0);
    return ret;
}


/*for ap scan list*/
void insertNode(sc_result *p_scan_list, sc_result *p_scan_node)
{
    if (p_scan_list == p_scan_node)
        return;
    p_scan_node->next = p_scan_list->next;
    p_scan_list->next = p_scan_node;
    return;
}

void freeAPList(sc_result **p_scan_list)
{
    sc_result *p = *p_scan_list;
    sc_result *temp;
    *p_scan_list = NULL;
    while (p != NULL)
    {
        temp = p;
        p  = p->next;
        free(temp);
    }
}

static int parseScanResult(char *buf, sc_result **p_scan_list)
{
    char *dlim = "\t\n";
    char *start, *p;
    start = strchr(buf, ':');
    IFM_PRINT("buf = %s\n", buf);
    sc_result *p_result_node;
    if (start)
    {
        IFM_PRINT("start = %s\n", start);
        p = strtok(start, dlim);
        IFM_PRINT("p = %s\n\n", p);
        p = strtok(NULL, dlim);
        IFM_PRINT("p = %s\n\n", p);
        int isFirst = 1;
        while(p)
        {
            p = strtok(NULL, dlim);
            IFM_PRINT("p = %s\n\n", p);
            p_result_node = (sc_result*)malloc(sizeof(sc_result));
            if (isFirst)
            {
           	 *p_scan_list = p_result_node;
                 isFirst = 0;
            }
            memset(p_result_node, 0x0, sizeof(sc_result));
            int strength = atoi(p);
            if (strength >= 0 &&strength <= 100)
            {    
                p_result_node->strength = atoi(p);
                p = strtok(NULL, dlim);
                if (p && *p == '[')
                {
                    IFM_PRINT("p = %s\n\n", p);
                    strncpy(p_result_node->pass_mode, p+1, 3);
                    p = strtok(NULL, dlim);
                    strncpy(p_result_node->ssid, p, strlen(p));
                }
                else if (p)
                {
                    strncpy(p_result_node->pass_mode, "NONE", 4);
                    strncpy(p_result_node->ssid, p, strlen(p));
                }
                insertNode(*p_scan_list, p_result_node);
            }
            p = strtok(NULL, dlim);
            p = strtok(NULL, dlim);
        }
        return ERR_OK;
    }
    *p_scan_list = NULL;
    return FORMAT_WRONG;
}


int APScan(char* netif_id, sc_result **p_scan_list)
{
    int ret = c_system("wpa_cli -p/var/run/wpa_supplicant ap_scan 1", 0);
    if (ret == 0) 
    {
        /*use /tmp to save the result in memmory*/
        ret = c_system("wpa_cli -p/var/run/wpa_supplicant scan_result >> /tmp/scan_result", 0);
        if (ret == 0)
        {
            sleep(1);
            int fd = open("/tmp/scan_result", O_RDONLY, 0);
            if (fd >= 0)
            {
                int size = lseek(fd, 0, SEEK_END);
                lseek(fd, 0, SEEK_SET);
                char *buf = (char*)malloc(sizeof(char)*(size+4));
                int bytes = read(fd, buf, size);
                if (bytes == size)
                {
                    buf[size] = 0;
                    ret = parseScanResult(buf, p_scan_list);
                }
                else
                    ret = READ_AP_RESULT_WRONG;
                free(buf);
                close(fd);
                /*delete the temp file*/
                c_system("rm -f /tmp/scan_result", 0);
            }
            else
                ret = OPEN_AP_RESULT_WRONG;
        }
    }
    return ret;
}

/*init wpa_supplicant, can be called more than once*/
int wpa_up()
{
    int ret = c_system("ifconfig wlan0 up", 0);
    if (ret != 0)
        return INTERFACE_NOT_FOUND;
    ret = c_system("wpa_supplicant -g/var/run/wpa_supplicant-global -B", 0);
    if (ret != 0)
        IFM_PRINT("wpa return %d!\n", ret);
    ret = c_system("wpa_cli -g/var/run/wpa_supplicant-global interface_add wlan0  \"\" wext /var/run/wpa_supplicant", 0);
    return ret;
}

/*to avoid rubbish udhcpc process take the memmory when wifi dongle is removed*/
void killWifiDhcp(char *netif_id)
{
    char tempCmd[64];
    memset(tempCmd, 0x0, 64);
    snprintf(tempCmd, sizeof(tempCmd), "killall udhcpc -i %s", netif_id);
    c_system(tempCmd, 0);
}

int setMacAddr(char *ifname, char *hwaddr)
{
    char tempCmd[512];
    memset(tempCmd, 0x0, 512);
    snprintf(tempCmd, sizeof(tempCmd), "ifconfig %s down", ifname);
    int ret = c_system(tempCmd, 0);
    if (ret == 0)
    {
        memset(tempCmd, 0x0, 512);
        snprintf(tempCmd, sizeof(tempCmd), "ifconfig %s hw ether %s", ifname, hwaddr);
        ret = c_system(tempCmd, 0);
        if (ret == 0)
        {
            memset(tempCmd, 0x0, 512);
            snprintf(tempCmd, sizeof(tempCmd), "ifconfig %s up", ifname);
            ret = c_system(tempCmd, 0);
        }
    }
    return ret;
}

int setNetwork()
{

#ifdef USE_NAND //data was read from nand
    int size;
    char *buf = get_ifconfig(&size);
#else           //data was formed by const string
    char *buf = malloc(1024);
#endif
//    strcpy(buf, "ifname:eth0 mac:00:21:32:96:fa:cc dhcp:0 ip:10.68.11.123 netmask:255.255.255.0 gateway:10.68.11.1 dns1:202.106.196.115\n");
//    strcpy(buf, "ifname:ppp0 mac:00:21:32:96:fa:cc username:gaokj password:fatpig\n");
//    strcpy(buf, "ifname:wlan0 ssid:AML_QA passmode:WPA password:11111111 dhcp:1\n");
    char *p;
    char *dlim = " \n";
    int ret;
    char *mac, *ip, *netmask, *gateway, *dns1, *ssid, *passmode, *password, *username;
    p = strtok(buf, dlim);
    if (p && strncmp(p, "ifname:", 7) == 0)
    {
        p += 7;
        if (strncmp(p, "eth0", 4) == 0)
        {
            p = strtok(NULL, dlim);
            if (p && strncmp(p, "mac:", 4) == 0)
            {
                mac = p + 4;
                ret = setMacAddr("eth0", mac);
                if (ret < 0)
                {
                    free(buf);
                    return ret;
                }
                p = strtok(NULL, dlim);
                if (p && strncmp(p, "dhcp:", 5) == 0)
                {
                    if (*(p+5) == '1')
                    {
                        ret = setEtherIpAddr("eth0", 1, NULL, NULL, NULL, NULL, NULL);
                        free(buf);
                        return ret;	    
                    }
                    else if (*(p+5) == '0')
                    {
                        p = strtok(NULL, dlim);
                        if (p && strncmp(p, "ip:", 3) == 0)
                        {
                            ip = p + 3;
                            p = strtok(NULL, dlim);
                            if (p && strncmp(p, "netmask:", 8) == 0)
                            {
                                netmask = p + 8;
                                p = strtok(NULL, dlim);
                                if (p && strncmp(p, "gateway:", 8) == 0)
                                {
                                    gateway = p + 8;
                                    p = strtok(NULL, dlim);
                                    if (p && strncmp(p, "dns1:", 5) == 0)
                                    {
                                        dns1 = p + 5;
                                        if ((p = strtok(NULL, dlim)))
                                            goto WRONG;
                                        else
                                        {
                                            ret = setEtherIpAddr("eth0", 0, ip, netmask, gateway, dns1, dns1);
                                            free(buf);
                                            return ret;
                                        }
                                    }
                                    else
                                        goto WRONG;
                                }
                                else
                                    goto WRONG;
                            }
                            else
                                goto WRONG;
                        }
                        else
                            goto WRONG;
                    }
                    else
                        goto WRONG;
                }
                else
                    goto WRONG;
            }
            else
                goto WRONG;
        }
        else if (strncmp(p, "wlan0", 5) == 0)
        {
            p = strtok(NULL, dlim);
            if (p && strncmp(p, "ssid:", 5) == 0)
            {
                ssid = p + 5;
                p = strtok(NULL, dlim);
                if (p && strncmp(p, "passmode:", 9) == 0)
                {
                    passmode = p + 9;
                    p = strtok(NULL, dlim);
                    if (p && strncmp(p, "password:", 9) == 0)
                    {
                        password = p + 9;
                        p = strtok(NULL, dlim);
                        if (p && strncmp(p, "dhcp:", 5) == 0)
                        {
                            if (*(p+5) == '1')
                            {
                                ret = wpa_up();
                                if (ret == 0)
                                {
                                    ret = setWiFiIpAddr("wlan0", 1, ssid, passmode, password, NULL, NULL, NULL, NULL, NULL);
                                }    
                                free(buf);
                                return ret;
                            }
                            else if (*(p+5) == '0')
                            {
                                p = strtok(NULL, dlim);
                                if (p && strncmp(p, "ip:", 3) == 0)
                                {
                                    ip = p + 3;
                                    p = strtok(NULL, dlim);
                                    if (p && strncmp(p, "netmask:", 8) == 0)
                                    {
                                        netmask = p + 8;
                                        p = strtok(NULL, dlim);
                                        if (p && strncmp(p, "gateway:", 8) == 0)
                                        {
                                            gateway = p + 8;
                                            p = strtok(NULL, dlim);
                                            if (p && strncmp(p, "dns1:", 5) == 0)
                                            {
                                                dns1 = p + 5;
                                                if ((p = strtok(NULL, dlim)))
                                                    goto WRONG;
                                                else
                                                {
                                                    ret = wpa_up();
                                                    if (ret == 0)
                                                    {
                                                        ret = setWiFiIpAddr("wlan0", 0, ssid, passmode, password, ip, netmask, gateway, dns1, dns1);
                                                    }
                                                    free(buf);
                                                    return ret;
                                                }
                                            }
                                            else
                                                goto WRONG;
                                        }
                                        else
                                            goto WRONG;
                                    }
                                    else
                                        goto WRONG;
                                }
                                else
                                    goto WRONG;
                            }
                            else
                                goto WRONG;
                        }
                        else
                            goto WRONG;
                    }
                    else
                        goto WRONG;
                }
                else
                    goto WRONG;
            }
            else
                goto WRONG;
        }
        else if (strncmp(p, "ppp0", 4) == 0)
        {
            p = strtok(NULL, dlim);
            if (p && strncmp(p, "mac:", 4) == 0)
            {
                mac = p + 4;
                ret = setMacAddr("eth0", mac);
                if (ret < 0)
                {
                    free(buf);
                    return ret;
                }
                p = strtok(NULL, dlim);
                if (p && strncmp(p, "username:", 9) == 0)
                {
                    username = p + 9;
                    p = strtok(NULL, dlim);
                    if (p && strncmp(p, "password:", 9) == 0)
                    {
                        password = p + 9;
                        if (strtok(NULL, dlim))
                            goto WRONG;
                        else
                        {
                            ret = PPPoEConnect("eth0", username, password);
                            free(buf);
                            return ret;
                        }
                    }
                    else 
                        goto WRONG;
                }
                else
                    goto WRONG;
            }
            else
                goto WRONG;
        }
        else
            goto WRONG;
    }

WRONG:
    free(buf);
    return FORMAT_WRONG;
}

int check_wifi_cable(char *if_name)
{
    char tmpCmd[128];
    memset(tmpCmd, 0x0, sizeof(tmpCmd));
    snprintf(tmpCmd, sizeof(tmpCmd), "ifconfig %s up >> /tmp/temp", if_name);
    int ret = c_system(tmpCmd, 0);
    if (ret != 0)
    {
        memset(tmpCmd, 0x0, sizeof(tmpCmd));
        snprintf(tmpCmd, sizeof(tmpCmd), "ifconfig %s >> /tmp/temp", if_name);
        ret = c_system(tmpCmd, 0);
        c_system("rm -f /tmp/temp", 0);
    }
    return ret;
}



