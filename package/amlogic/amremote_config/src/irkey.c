#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <time.h>
#include <sys/queue.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/wait.h> 
#include <linux/input.h>



static unsigned int 
translate_event( const struct input_event *levt)
{

     switch (levt->type) {
          case EV_KEY:
               return levt->code;

          case EV_REL:
          case EV_ABS:
          default:
	    break ;	  	
               ;
     }

     return -1;
}

int main(int argc, char* argv[])
{
	int  intput_fd=-1;
	fd_set rfds,wfds;
	struct timeval tv;
	int retval ;
	int exit_enable=0;
	int lace=0x1000;
	unsigned int i,readlen;
	
       struct input_event levt[64];
	if(argc > 1)
	    intput_fd=open(argv[1],O_RDWR);//如果设备节点创建在/dev/input目录下，就是/dev/input/event0
	else
           intput_fd=open("/dev/input/event0",O_RDWR);
	if(intput_fd < 0)
		printf("can not open event device....\n");
	else
		printf("open event success\n");

	while(!exit_enable)
	{
		tv.tv_sec = 1;
		tv.tv_usec = 10;
		FD_ZERO(&rfds);
		FD_SET(intput_fd, &rfds);
		retval = select(intput_fd+1, &rfds, NULL, NULL, &tv);  //wait demux data
		if(retval==-1)
		{
			exit_enable=1;
		}
		else if(retval)
		{
			if(FD_ISSET(intput_fd,&rfds)) 
			{
				readlen =read(intput_fd,levt, sizeof(levt));
				 for (i=0; i<readlen / sizeof(levt[0]); i++) {
				 	printf("----type = %d, code = 0x%x, value = %d-------------\n", levt[i].type, levt[i].code, levt[i].value);
				 }
			}
		}
	}	
	
	return 0;
}

