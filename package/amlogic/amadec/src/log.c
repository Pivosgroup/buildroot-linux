#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "log.h"

static int log_fd = -1;
static char timebuf[20];
#define MAX_LOG_SIZE (10*1024)
static char *get_system_time(void)
{
    time_t cur_time;
    struct tm * timeinfo;

    time(&cur_time);
    timeinfo = localtime ( &cur_time );
    strftime(timebuf, 20, "%H:%M:%S", timeinfo);

    return timebuf;
}

int log_open(const char *name)
{
    if(name==NULL)
	{
		/*print to console*/
		log_fd=-1;
		return 0;
	}
	
    if ((log_fd = open(name, O_CREAT|O_RDWR | O_TRUNC, 0644)) < 0)
        return -1;

    return 0;
}

void log_close(void)
{
    if (log_fd >= 0)
        close(log_fd);
}
static void check_file_size(void)
{
	int size;
	size=lseek(log_fd,0,SEEK_CUR);
	if(size>MAX_LOG_SIZE)
		{
			lseek(log_fd,0,SEEK_SET);
		}
}

__attribute__ ((format (printf, 2, 3)))
void log_print(const int level, const char *fmt, ...)
{
    char *buf = NULL;
    char *systime;
    va_list ap;
	
	va_start(ap, fmt);
	vasprintf(&buf, fmt, ap);
	va_end(ap);
	if(log_fd>0)
	{	
		check_file_size();
		systime = get_system_time();
		write(log_fd, systime, strlen(systime));
		write(log_fd, " ", 1);
		write(log_fd, buf, strlen(buf));
		write(log_fd, "\n", 1);
		fsync(log_fd);
	}
	else
	{
		systime = get_system_time();
		fprintf(stderr,"%s: %s\n",systime,buf);
		fflush(stderr);
	}
    if (buf)
        free(buf);
}
