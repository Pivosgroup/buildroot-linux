#include <stdio.h>
#include <player.h>
#include <unistd.h>
#include <controler.h>
/*
arc-linux-uclibc-gcc   -I?/rootfs/include -L?/rootfs/lib -Wall -Werror   \
	-lamplayer -lamcontroler -lamcodec -lavformat -lavutil -lavcodec -lpthread -lz \
	simple_player.c 

*/
int main(int argc,char *argv[])
{
	thread_ctrl_para_t init_para;
	int pid;
	if(argc<2)
		{
		printf("usage:player file\n");
		return -1;
		}
	player_init();	
	init_para.play_ctrl_para.file_name=argv[1];
	pid=player_start(&init_para.play_ctrl_para,0);
	if(pid<0)
		{
		printf("player start failed!error=%d\n",pid);
		return -1;
		}
    init_para.pid = pid;
	while(player_get_state(pid)!=PLAYER_STOPED)
		{
		sleep(3);
		}
	player_progress_exit();
	return 0;
}

