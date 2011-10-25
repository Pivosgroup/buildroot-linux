#include <stdio.h>
#include <player.h>
#include <unistd.h>
#include <signal.h>
#include <controler.h>

/*
arc-linux-uclibc-gcc   -I?/rootfs/include -L?/rootfs/lib -Wall -Werror   \
	-lamplayer -lamcontroler -lamcodec -lavformat -lavutil -lavcodec -lpthread -lz \
	muti_threadplayer.c 

only support two seprate file for audio and video
*/

static void signal_handler(int signum)
{
	printf("get signum=%d\n",signum);
	player_progress_exit();/*stop the audio,we need to call it in Ctrl+C or Other errors exit*/
	signal(signum, SIG_DFL);
	raise (signum);
}

int main(int argc,char *argv[])
{
	thread_ctrl_para_t init_para;
	thread_ctrl_para_t init_para2;
	int pid1,pid2;
	player_status status1,status2;
	
	if(argc<3)
		{
		printf("usage:player videofile musicfile\n");
		return -1;
		}

    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
	signal(SIGSEGV, signal_handler);
	signal(SIGINT, signal_handler);
	player_init();	
	init_para.play_ctrl_para.file_name=argv[1];
	init_para.play_ctrl_para.nosound=1;
	pid1=player_start(&init_para.play_ctrl_para,0);
	if(pid1<0)
		{
		printf("player start failed!error=%d\n",pid1);
		return -1;
		}
    init_para.pid = pid1;
	init_para2.play_ctrl_para.file_name=argv[2];
	init_para2.play_ctrl_para.novideo=1;
	pid2=player_start(&init_para2.play_ctrl_para,0);
	if(pid2<0)
		{
		printf("player start failed!error=%d\n",pid2);
		return -1;
		}
    init_para2.pid = pid2;
	printf("start two thread OK!\n");
	do{
		sleep(1);
		status1=player_get_state(pid1);
		if(status1==PLAYER_STOPED)
			printf("%s-%d:play end \n",argv[1],pid1);
		status2=player_get_state(pid2);
		if(status2==PLAYER_STOPED)
			printf("%s-%d:play end \n",argv[2],pid1);
		
		}while(status1!=PLAYER_STOPED|| status2!=PLAYER_STOPED);

	
	player_progress_exit();/*stop the audio,we need to call it in Ctrl+C or Other errors exit*/
	return 0;
}


