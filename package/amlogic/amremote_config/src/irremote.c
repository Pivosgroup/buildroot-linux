#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "remote_config.h"
#include "keydefine.h"

#define DEVICE_NAME "/dev/amremote"

unsigned short key_map[256], mouse_map[10];

unsigned short default_key_map[256] = {
KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, /*0~7*/
KEY_8, KEY_9, KEY_BACK, KEY_UP, KEY_BACKSPACE, KEY_REPLY, KEY_DOWN, KEY_MENU, /*8~f*/
KEY_LEFT, KEY_RIGHT, KEY_R, KEY_S, KEY_U, KEY_G, KEY_K, KEY_L, /*10~17*/
KEY_M, KEY_N, KEY_D, KEY_T, KEY_H, KEY_B, KEY_I, KEY_J, /*18~1f*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*20~27*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*28~2f*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*30~37*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*38~3f*/

KEY_C, KEY_F, KEY_E, KEY_P, KEY_V, KEY_A, KEY_Q, KEY_O, /*40~47*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*48~4f*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_W, KEY_Z, KEY_RESERVED, KEY_RESERVED, KEY_Y, /*50~57*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_X, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*58~5f*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*60~67*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*68~6f*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*70~77*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*78~7f*/

KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*80~87*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*88~8f*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*90~97*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*98~9f*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*a0~a7*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*a8~af*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*b0~b7*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*b8~bf*/

KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*c0~c7*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*c8~cf*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*d0~d7*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*d8~df*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*e0~e7*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*e8~ef*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, /*f0~f7*/
KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED /*f8~ff*/
};

unsigned short default_mouse_map[10] = {
 //0x10, 0x11, 0x0b, 0x0e 
 0xffff, 0xffff, 0xffff, 0xffff,
 0xffff, 0xffff,
 0xffff, 0xffff, 0xffff, 0xffff
};


int main(int argc, char* argv[])
{
    remote_config_t *remote;
    FILE *fp;
    int ret, i;
    int device_fd;
    unsigned int val;

    for(i =0; i < 256; i++)
        key_map[i] = KEY_RESERVED;
    for(i =0; i < 10; i++)
        mouse_map[i] = 0xffff;
    remote = (remote_config_t *)malloc(sizeof(remote_config_t));
    if(!remote){
        printf("out of memory !\n");
        return -1;
        }
    memset((unsigned char*)remote, 0xff, sizeof(remote_config_t));
    remote->key_map = key_map;
    remote->mouse_map = mouse_map;
    device_fd = open(DEVICE_NAME, O_RDWR);
    if(device_fd < 0){
        printf("Can't open %s .\n", DEVICE_NAME);
        return -2;
        }
    if(argc < 2){
        remote->factory_code = 0x40400001;
        remote->work_mode = 1;
        remote->repeat_enable = 1;
        remote->release_delay = 150;
        remote->debug_enable = 1;
        remote->reg_control = 0xfbe40;
        remote->repeat_delay = 250;
        remote->repeat_peroid = 33;
        memcpy(key_map, default_key_map, sizeof(key_map));
        memcpy(mouse_map, default_mouse_map, sizeof(mouse_map));
        }
    else if(argv[1][0]=='-'){
        printf("Usage : %s configfile\n", argv[0]);
        return -3;
        }
    else{
        fp=fopen(argv[1], "r");
        if(!fp){
            printf("Open file %s is failed!!!\n", argv[1]);
            return -4;
            }
        ret = get_config_from_file(fp, remote);
        fclose(fp);
        }
    remote->factory_code >>= 16;
    ret = set_config(remote, device_fd);
    ioctl(device_fd, REMOTE_IOC_RESET_KEY_MAPPING, NULL);
    for(i = 0; i < 256; i++)
        if(key_map[i] != KEY_RESERVED){
            val = (i<<16) | key_map[i];
            ioctl(device_fd, REMOTE_IOC_SET_KEY_MAPPING, &val);
            }
    for(i = 0; i < 10; i++)
        if(mouse_map[i] != 0xffff){
            val = (i<<16) | mouse_map[i];
            ioctl(device_fd, REMOTE_IOC_SET_MOUSE_MAPPING, &val);
            }
    close(device_fd);
    return 0;
}
