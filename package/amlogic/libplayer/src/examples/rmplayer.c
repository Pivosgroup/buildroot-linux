/**************************************************
* example based on amcodec
**************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <codec.h>
#include <stdbool.h>
#include <ctype.h>

#define READ_SIZE (64 * 1024)
#define AUDIO_INFO_SIZE 4096
#define UNIT_FREQ       96000
#define PTS_FREQ        90000
#define AV_SYNC_THRESH    PTS_FREQ*30

static codec_para_t codec_para;
static codec_para_t *pcodec;
static char *filename;
FILE* fp = NULL;
static char audio_info_buf[AUDIO_INFO_SIZE] = {0};
static int axis[8] = {0};

int osd_blank(char *path,int cmd)
{
    int fd;
    char  bcmd[16];
    fd = open(path, O_CREAT|O_RDWR | O_TRUNC, 0644);

    if(fd>=0) {
        sprintf(bcmd,"%d",cmd);
        write(fd,bcmd,strlen(bcmd));
        close(fd);
        return 0;
    }

    return -1;
}

int set_tsync_enable(int enable)
{
    int fd;
    char *path = "/sys/class/tsync/enable";
    char  bcmd[16];
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        sprintf(bcmd, "%d", enable);
        write(fd, bcmd, strlen(bcmd));
        close(fd);
        return 0;
    }
    
    return -1;
}

int parse_para(const char *para, int para_num, int *result)
{
    char *endp;
    const char *startp = para;
    int *out = result;
    int len = 0, count = 0;

    if (!startp) {
        return 0;
    }

    len = strlen(startp);

    do {
        //filter space out
        while (startp && (isspace(*startp) || !isgraph(*startp)) && len) {
            startp++;
            len--;
        }

        if (len == 0) {
            break;
        }

        *out++ = strtol(startp, &endp, 0);

        len -= endp - startp;
        startp = endp;
        count++;

    } while ((endp) && (count < para_num) && (len > 0));

    return count;
}

int set_display_axis(int recovery)
{
    int fd;
    char *path = "/sys/class/display/axis";
    char str[128];
    int count, i;
    fd = open(path, O_CREAT|O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        if (!recovery) {
            read(fd, str, 128);
            printf("read axis %s, length %d\n", str, strlen(str));
            count = parse_para(str, 8, axis);
        }
        if (recovery) {
            sprintf(str, "%d %d %d %d %d %d %d %d", 
                axis[0],axis[1], axis[2], axis[3], axis[4], axis[5], axis[6], axis[7]);
        } else {
            sprintf(str, "2048 %d %d %d %d %d %d %d", 
                axis[1], axis[2], axis[3], axis[4], axis[5], axis[6], axis[7]);
        }
        write(fd, str, strlen(str));
        close(fd);
        return 0;
    }

    return -1;
}

static void signal_handler(int signum)
{   
    printf("Get signum=%x\n",signum);
    codec_close(pcodec);
    fclose(fp);
    set_display_axis(1);
    signal(signum, SIG_DFL);
    raise (signum);
}

int main(int argc,char *argv[])
{
    int ret = CODEC_ERROR_NONE;
    char buffer[READ_SIZE];
    int total_rev_bytes = 0, rev_byte = 0;
    unsigned int dataoffset = 0;

    int len = 0;
    int size = READ_SIZE;
    uint32_t Readlen;
    uint32_t isize;
    struct buf_status vbuf;

    if (argc < 10) {
        printf("Corret command: rmplay <filename> <vid> <width> <height> <fps> <aid> <channel> <samplerate> <dataoffset>\n");
        return -1;
    }
    osd_blank("/sys/class/graphics/fb0/blank",1);
    osd_blank("/sys/class/graphics/fb1/blank",0);
    set_display_axis(0);

    pcodec = &codec_para;
    memset(pcodec, 0, sizeof(codec_para_t ));

    pcodec->noblock = 0;
    pcodec->has_video = 1;
    pcodec->video_pid = atoi(argv[2]);
    pcodec->video_type = VFORMAT_REAL;
    pcodec->am_sysinfo.format = VIDEO_DEC_FORMAT_REAL_9; // take real9 for example
    pcodec->am_sysinfo.rate = 96000 / atoi(argv[5]);
    pcodec->am_sysinfo.height = atoi(argv[4]);
    pcodec->am_sysinfo.width = atoi(argv[3]);

    pcodec->has_audio = 1;
    pcodec->audio_type = AFORMAT_COOK; // take cook for example
    pcodec->audio_pid = atoi(argv[6]);
    pcodec->audio_channels = 2; // only support 2 channels
    pcodec->audio_samplerate = atoi(argv[8]);
    pcodec->audio_info.channels = 2;
    pcodec->audio_info.sample_rate = pcodec->audio_samplerate;
    pcodec->audio_info.valid = 1;

    pcodec->stream_type = STREAM_TYPE_RM;

    printf("\n*********CODEC PLAYER DEMO************\n\n");
    filename = argv[1];
    printf("file %s to be played\n", filename);

    if((fp = fopen(filename,"rb")) == NULL)
    {
       printf("open file error!\n");
       return -1;
    }

#if 1
    /* get the cook header */
    do {
        rev_byte = fread(&audio_info_buf[total_rev_bytes], 1, (AUDIO_INFO_SIZE - total_rev_bytes), fp);
        printf("[%s:%d]rev_byte=%d total=%d\n", __FUNCTION__, __LINE__, rev_byte, total_rev_bytes);
        if (rev_byte < 0) {
            printf("audio codec init faile--can't get real_cook decode info!\n");
            return -1;
        } else {
            total_rev_bytes += rev_byte;
            if (total_rev_bytes == AUDIO_EXTRA_DATA_SIZE) {
                memcpy(pcodec->audio_info.extradata, &audio_info_buf, AUDIO_INFO_SIZE);
                pcodec->audio_info.extradata_size = AUDIO_INFO_SIZE;
                break;
            } else if (total_rev_bytes > AUDIO_INFO_SIZE) {
                printf("[%s:%d]real cook info too much !\n", __FUNCTION__, __LINE__);
                return -1;
            }
        }
    } while (1);
#endif

    ret = codec_init(pcodec);
    if(ret != CODEC_ERROR_NONE)
    {
        printf("codec init failed, ret=-0x%x", -ret);
        return -1;
    }

    printf("rm codec ok!\n");

    //codec_set_cntl_avthresh(vpcodec, AV_SYNC_THRESH);
    //codec_set_cntl_syncthresh(vpcodec, 0);

    set_tsync_enable(1);

    /* For codec, it needs to seek to DATA+18 bytes, which can be got from ffmpeg,
       just use the parameter */
    dataoffset = atoi(argv[9]);
    printf("seek to DATA offset 0x%x\n", dataoffset);
    fseek(fp, dataoffset, SEEK_SET);

    while(!feof(fp))
    {
        Readlen = fread(buffer, 1, READ_SIZE,fp);
        //printf("Readlen %d\n", Readlen);
        if(Readlen <= 0)
        {
            printf("read file error!\n");
            rewind(fp);
        }

        isize = 0;
        do{
            ret = codec_write(pcodec, buffer+isize, Readlen);
            if (ret < 0) {
                if (errno != EAGAIN) {
                    printf("write data failed, errno %d\n", errno);
                    goto error;
                }
                else {
                    continue;
                }
            }
            else {
                isize += ret;
            }
            //printf("ret %d, isize %d\n", ret, isize);
        }while(isize < Readlen);	 

        signal(SIGCHLD, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGHUP, signal_handler);
        signal(SIGTERM, signal_handler);
        signal(SIGSEGV, signal_handler);
        signal(SIGINT, signal_handler);
        signal(SIGQUIT, signal_handler);
    }	

    do {
        ret = codec_get_vbuf_state(pcodec, &vbuf);
        if (ret != 0) {
            printf("codec_get_vbuf_state error: %x\n", -ret);
            goto error;
        }        
    } while (vbuf.data_len > 0x100);
    
error:
    codec_close(pcodec);
    fclose(fp);
    set_display_axis(1);
    
    return 0;
}

