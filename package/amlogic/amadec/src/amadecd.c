/*
 * Audio Decoder Daemon amadecd.
 * This program is the user side daemon to manage audio decoders.
 * When the daemon program is started, it:
 * 1. sets up a listening socket to react to any control
 *    messages for audio rendering such as start, pause, resume and stop
 *    command as well as the config command which assign the decoder
 *    format and decoding informations. This is the control thread.
 * 2. spawn a new running thread for audio decoding, which is
 *    the rendering thread.
 * The rendering thread will get data from either a
 * local file, a pipe, or amadec UIO driver depending on the configuration.
 * The decoded data samples are sent to ALSA to output.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <getopt.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "./include/adecproc.h"
#include "log.h"

//#define LOCK_FN "/var/lock/amadecd.lock"
#define LOCK_FN "/tmp/amadecd.lock"
#define LOG_FN "/tmp/amadecd.log"
#define AMADECD_SOCKET_NAME "/tmp/amadec_socket"

int default_outtype = 0; 

static void signal_handler(int signum)
{
    switch(signum) {
        case SIGHUP: /*TODO: restart daemon ??? */ break;
        case SIGTERM:
       
        	
        	exit(EXIT_SUCCESS); 
        	break;
    }
}

static void daemonize()
{
    pid_t pid, sid;
    int fp_lock;

    if (getppid() == 1)
        return;

    pid = fork();
    if (pid < 0) {
        printf("daemonize failed on fork");
        exit(EXIT_FAILURE);
    }
    if (pid > 0)
        exit(EXIT_SUCCESS);

#if 0 //lock not needed?
    fp_lock = open(LOCK_FN, O_RDWR|O_CREAT, 0640);
    if (fp_lock < 0) {
        printf("unable to create lock file %s, %s", LOCK_FN, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (lockf(fp_lock, F_TLOCK, 0) < 0) {
        printf("unable to lock file %s, %s", LOCK_FN, strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif	

    sid = setsid();
    if (sid < 0)
        exit(EXIT_FAILURE);

    umask(0);

    if ((chdir("/")) < 0)
        exit(EXIT_FAILURE);

    //close(STDIN_FILENO);
    //close(STDOUT_FILENO);
    //close(STDERR_FILENO);

    signal(SIGCHLD, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE,SIG_IGN); //ignore the socket closed singnal,default is exit;
	
#if 0
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
#endif	
    lp(LOG_INFO, "daemonized");
}

const char short_options[] = "D:fh";
const struct option long_options[] = {
              {"device", 1, 0, 'D'},
		{"front", 0, 0, 'f'},	  
		{"help", 0, 0, 'h'},	  
              {0, 0, 0, 0}
     };

void print_usage(char *name)
{
	int i;
	char *p;
	printf("%s usage:\n",name);
	printf("\t-D []   --device     \t select oss or and alsa ,default is alsa\n");
	printf("\t-f 	  --font        \trun this service at front	\n");
	printf("\t-h	  --help        \tShow this message\n");
	return;
}
	 
int get_options(int argc, char *argv[],struct amadec_opt *opt)
{
     int c;
     char *out_type = NULL;
     int i; 
     while ((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1 ) {
	 	switch(c) {
			case 'D':
				out_type = optarg;
				break;
			case 'f':
				opt->front_run=1;
				break;
			case 'h':
			case ':':
				return -1;
			default:
				break;
	 	}
     	}
	 
	if (out_type) {
		if (!strncmp(out_type, "alsa", 4)) {
			opt->out_type=0;
			 printf("Select alsa for audio output type !\n");
		}
		else if (!strncmp(out_type, "oss", 3)) {
			opt->out_type=1;
			printf("Select oss for audio output type !\n");
		}
	}

	for(i=1;i<argc;i++)
		{
		if(argv[i][0]!='-')
			{
			if(strncmp(argv[i],"start",strlen("start")-1)==0)
				{
				opt->cmd_start=1;
				opt->cmd_valid=1;
				break;
				}
			if(strncmp(argv[i],"stop",strlen("stop")-1)==0)
				{
				opt->cmd_stop=1;
				opt->cmd_valid=1;
				break;
				}
			if(strncmp(argv[i],"quit",strlen("quit")-1)==0)
				{
				opt->cmd_quit=1;
				opt->cmd_valid=1;
				break;
				}
			if(strncmp(argv[i],"pause",strlen("pause")-1)==0)
				{
				opt->cmd_pause=1;
				opt->cmd_valid=1;
				break;
				}
			if(strncmp(argv[i],"resume",strlen("resume"))==0)
				{
				opt->cmd_resume=1;
				opt->cmd_valid=1;
				break;
				}
			if(strncmp(argv[i],"mute",strlen("mute"))==0)
				{
				opt->cmd_mute=1;
				opt->cmd_valid=1;
				break;
				}
			if(strncmp(argv[i],"unmute",strlen("unmute"))==0)
				{
				opt->cmd_unmute=1;
				opt->cmd_valid=1;
				break;
				}
			if(strncmp(argv[i],"volset",strlen("volset"))==0)
				{
				strcpy(opt->vol_cmd, argv[i]);
				opt->cmd_volset=1;
				opt->cmd_valid=1;
				break;
				}
			if(strncmp(argv[i],"volget",strlen("volget"))==0)
				{
				opt->cmd_volget=1;
				opt->cmd_valid=1;
				break;
				}
			if(strncmp(argv[i],"leftmono",strlen("leftmono"))==0)
				{
				opt->cmd_leftmono=1;
				opt->cmd_valid=1;
				break;
				}
			if(strncmp(argv[i],"rightmono",strlen("rightmono"))==0)
				{
				opt->cmd_rightmono=1;
				opt->cmd_valid=1;
				break;
				}
			if(strncmp(argv[i],"stereo",strlen("stereo"))==0)
				{
				opt->cmd_stereo=1;
				opt->cmd_valid=1;
				break;
				}
			if(strncmp(argv[i],"swap",strlen("swap"))==0)
				{
				opt->cmd_swap=1;
				opt->cmd_valid=1;
				break;
				}
			if(strncmp(argv[i],"automute",strlen("automute"))==0)
				{
				strcpy(opt->vol_cmd, argv[i]);
				opt->cmd_automute=1;
				opt->cmd_valid=1;
				break;
				}

                	if(strncmp(argv[i],"spectrumon",strlen("spectrumon"))==0)
			{  
			       strcpy(opt->vol_cmd, argv[i]);
        			opt->cmd_spectrum_on = 1;
        			opt->cmd_valid=1;
        			break;
			}
                    if(strncmp(argv[i],"spectrumoff",strlen("spectrumoff"))==0)
			{    
        			opt->cmd_spectrum_off=1;
        			opt->cmd_valid=1;
        			break;
			}
			}
		}
	return 0;
}
void status_check(struct amadec_opt *opt)
{
	if(opt->old_is_run)
		{
		if(!opt->cmd_valid)	
			printf("%s is running \n",opt->name);
		
		}
}
void adecd_cmd(struct amadec_opt *opt)
{
	int len,socket_fd;
	char cmd[32]="\0";
	char tmp_cmd[128];
	int rlen;
	
       socket_fd=opt->socket_fd;
	if(opt->cmd_start)
		strcpy(cmd,"start");
	else if(opt->cmd_stop)
		strcpy(cmd,"stop");
	else if(opt->cmd_quit)
		strcpy(cmd,"quit");
	else if(opt->cmd_pause)
		strcpy(cmd,"pause");
	else if(opt->cmd_resume)
		strcpy(cmd,"resume");
	else if(opt->cmd_mute)
		strcpy(cmd,"mute");
	else if(opt->cmd_unmute)
		strcpy(cmd,"unmute");
	else if(opt->cmd_volset)
		strcpy(cmd, opt->vol_cmd);
	else if(opt->cmd_volget)
		strcpy(cmd,"volget");
	else if(opt->cmd_leftmono)
		strcpy(cmd,"leftmono");
	else if(opt->cmd_rightmono)
		strcpy(cmd,"rightmono");
	else if(opt->cmd_stereo)
		strcpy(cmd,"stereo");
	else if(opt->cmd_swap)
		strcpy(cmd,"swap");
	else if(opt->cmd_automute)
		strcpy(cmd,opt->vol_cmd);
      else if(opt->cmd_spectrum_on)
       {
            //strcpy(cmd,"spectrumon");
            strcpy(cmd, opt->vol_cmd);
           
      }
      else if(opt->cmd_spectrum_off)
        {
            strcpy(cmd,"spectrumoff");           
        }
	if(cmd[0]!='\0')
		{
		 len = strlen(cmd) + 1;
	        write(socket_fd, &len, sizeof(len));
	        write(socket_fd, cmd, len);
		}
	rlen= read(socket_fd, &len, sizeof(len));
	 if(rlen>0 && len <128) 
	 	{
		   rlen= read(socket_fd, &tmp_cmd, len);
		   printf("%s\n",tmp_cmd);
	 }
	return ;
}

int socket_init(struct amadec_opt *opt)
{
	int socket_fd=-1;
	int server_is_run=0;
	struct sockaddr_un name;
	socket_fd = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (socket_fd < 0) {
		printf("daemon socket creation failed\n");
		return -1;
	}

	name.sun_family = AF_LOCAL;
	strncpy(name.sun_path, AMADECD_SOCKET_NAME,
	sizeof(name.sun_path) - 1);

	if (connect(socket_fd, (struct sockaddr *)&name,SUN_LEN(&name)) == 0) {
				opt->old_is_run=1;
				opt->server_mode=0;
				opt->front_run=1;
	   }
	else
	{/*can't connect to server,maybe server is out without del the socket file */
		remove(AMADECD_SOCKET_NAME);
	}

	if(!opt->old_is_run)
		{ 
			if (bind(socket_fd, (struct sockaddr *)&name,SUN_LEN(&name)) < 0) 
			{
				printf("socket bind failed(%s), %s\n", AMADECD_SOCKET_NAME,strerror(errno));
				return -1;
			}
			opt->server_mode=1;
		}
	return socket_fd;
}


int main(int argc, char *argv[])
{

 struct amadec_opt opt={0};
 opt.server_mode=1;
 opt.name=argv[0];
if(get_options(argc, argv,&opt)<0)
	{
		print_usage(argv[0]);
		return 0;
    	}
    default_outtype=opt.out_type;
    opt.socket_fd=socket_init(&opt);
    status_check(&opt);
    if(opt.socket_fd<0)
    	{
    		return 0;
    	}
   if(!opt.front_run)
   	{
    	
	#if ENABLE_SYSLOG
    		openlog("amadecd", LOG_PID, LOG_LOCAL5);
	#else
    		log_open(LOG_FN);
		printf("log to file %s\n",LOG_FN);
	#endif
		daemonize();
   	}
   else
   	{
   		log_open(NULL);
   	}
   
   if(opt.server_mode && opt.cmd_quit!=1)
   	{
    		adecd_process(&opt);
   	}
   else
   	{
   		adecd_cmd(&opt);
   	}

    
    close(opt.socket_fd);
    if(opt.server_mode)	
    	unlink(AMADECD_SOCKET_NAME);

    lp(LOG_INFO, "Terminated");

#if ENABLE_SYSLOG
    closelog();
#else
    log_close();
#endif

    return 0;
}
