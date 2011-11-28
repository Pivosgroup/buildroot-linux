/****************************************
 * file: player_set_disp.c
 * description: set disp attr when playing
****************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <log_print.h>
#include <player_type.h>
#include <player_set_sys.h>
#include <linux/fb.h>

static freescale_setting_t freescale_setting[] = {
    {
        DISP_MODE_480P,
        {0, 0, 800, 480, 0, 0, 18, 18},
        {0, 0, 800, 480, 0, 0, 18, 18},
        {20, 10, 700, 470},
        800,
        480,
        800,
        480
    },
    {
        DISP_MODE_720P,
        {240, 120, 800, 480, 240, 120, 18, 18},
        {0, 0, 800, 480, 0, 0, 18, 18},
        {40, 15, 1240, 705},
        800,
        480,
        800,
        480
    },
    {
        DISP_MODE_1080I,
        {560, 300, 800, 480, 560, 300, 18, 18},
        {0, 0, 800, 480, 0, 0, 18, 18},
        {40, 20, 1880, 1060},
        800,
        480,
        800,
        480
    },
    {
        DISP_MODE_1080P,
        {560, 300, 800, 480, 560, 300, 18, 18},
        {0, 0, 800, 480, 0, 0, 18, 18},
        {40, 20, 1880, 1060},
        800,
        480,
        800,
        480
    }
};

int set_sysfs_str(const char *path, const char *val)
{
    int fd;
    int bytes;
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        bytes = write(fd, val, strlen(val));
        close(fd);
        return 0;
    } else {
    }
    return -1;
}
int  get_sysfs_str(const char *path, char *valstr, int size)
{
    int fd;
    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        read(fd, valstr, size - 1);
        valstr[strlen(valstr)] = '\0';
        close(fd);
    } else {
        sprintf(valstr, "%s", "fail");
        return -1;
    };
    log_print("get_sysfs_str=%s\n", valstr);
    return 0;
}

int set_sysfs_int(const char *path, int val)
{
    int fd;
    int bytes;
    char  bcmd[16];
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        sprintf(bcmd, "%d", val);
        bytes = write(fd, bcmd, strlen(bcmd));
        close(fd);
        return 0;
    }
    return -1;
}
int get_sysfs_int(const char *path)
{
    int fd;
    int val = 0;
    char  bcmd[16];
    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        read(fd, bcmd, sizeof(bcmd));
        val = strtol(bcmd, NULL, 16);
        close(fd);
    }
    return val;
}


int set_black_policy(int blackout)
{
    return set_sysfs_int("/sys/class/video/blackout_policy", blackout);
}

int get_black_policy()
{
    return get_sysfs_int("/sys/class/video/blackout_policy") & 1;
}

int check_audiodsp_fatal_err()
{
    int fatal_err = 0;
    fatal_err = get_sysfs_int("/sys/class/audiodsp/codec_fatal_err") & 0xf;
    if (fatal_err != 0) {
        log_print("[%s]get audio dsp error:%d!\n", __FUNCTION__, fatal_err);
    }
    return fatal_err;
}

int set_tsync_enable(int enable)
{
    return set_sysfs_int("/sys/class/tsync/enable", enable);

}
int set_tsync_discontinue(int discontinue)      //kernel set to 1,player clear to 0
{
    return set_sysfs_int("/sys/class/tsync/discontinue", discontinue);

}
int get_pts_discontinue()
{
    return get_sysfs_int("/sys/class/tsync/discontinue") & 1;
}

int set_subtitle_num(int num)
{
    return set_sysfs_int("/sys/class/subtitle/total", num);

}

int set_subtitle_curr(int num)
{
	return set_sysfs_int("/sys/class/subtitle/curr", num);
}
int set_subtitle_enable(int num)
{
    return set_sysfs_int("/sys/class/subtitle/enable", num);

}

int set_subtitle_fps(int fps)
{
    return  set_sysfs_int("/sys/class/subtitle/fps", fps);

}

int set_subtitle_subtype(int subtype)
{
    return  set_sysfs_int("/sys/class/subtitle/subtype", subtype);
}

int av_get_subtitle_curr()
{
    return get_sysfs_int("/sys/class/subtitle/curr");
}

int set_subtitle_startpts(int pts)
{
    return  set_sysfs_int("/sys/class/subtitle/startpts", pts);
}

int set_fb0_blank(int blank)
{
    return  set_sysfs_int("/sys/class/graphics/fb0/blank", blank);
}

int set_fb1_blank(int blank)
{
    return  set_sysfs_int("/sys/class/graphics/fb1/blank", blank);
}

static int get_last_file(char *filename, int nsize)
{
	return get_sysfs_str("/sys/class/video/file_name", filename, nsize);
}

static int set_last_file(char *filename)
{
    return set_sysfs_str("/sys/class/video/file_name", filename);
}

int check_file_same(char *filename2) 
{
   char filename1[512];
   int len1 = 0, len2 = 0;
   get_last_file(filename1, 512);
   
   len1 = strlen(filename1);
   len2 = strlen(filename2);
   
   log_print("[%s]file1=%d:%s\n", __FUNCTION__, len1 ,filename1);
   log_print("[%s]file2=%d:%s\n", __FUNCTION__, len2 ,filename2);

   set_last_file(filename2);
   
   if (((len1 -1)!= len2) && (len1 < 512)) {
       log_print("[%s]not match,1:%d 2:%d\n", __FUNCTION__, len1, len2);
       return 0;
   } else if (!strncmp(filename1, filename2, len2)) {
       log_print("[%s]match,return 1\n", __FUNCTION__);
       return 1;
   }       
       return 0;
}



void get_display_mode(char *mode)
{
    int fd;
    char *path = "/sys/class/display/mode";
    if (!mode) {
        log_error("[get_display_mode]Invalide parameter!");
        return;
    }
    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        read(fd, mode, 16);
        log_print("[get_display_mode]mode=%s strlen=%d\n", mode, strlen(mode));
        mode[strlen(mode)] = '\0';
        close(fd);
    } else {
        sprintf(mode, "%s", "fail");
    };
    log_print("[get_display_mode]display_mode=%s\n", mode);
    return ;
}

int set_fb0_freescale(int freescale)
{
    return  set_sysfs_int("/sys/class/graphics/fb0/free_scale", freescale);
}

int set_fb1_freescale(int freescale)
{
    return  set_sysfs_int("/sys/class/graphics/fb1/free_scale", freescale);

}

int set_display_axis(int *coordinate)
{
    int fd;
    char *path = "/sys/class/display/axis" ;
    char  bcmd[32];
    int x00, x01, x10, x11, y00, y01, y10, y11;
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        if (coordinate) {
            x00 = coordinate[0];
            y00 = coordinate[1];
            x01 = coordinate[2];
            y01 = coordinate[3];
            x10 = coordinate[4];
            y10 = coordinate[5];
            x11 = coordinate[6];
            y11 = coordinate[7];
            sprintf(bcmd, "%d %d %d %d %d %d %d %d", x00, y00, x01, y01, x10, y10, x11, y11);
            write(fd, bcmd, strlen(bcmd));
        }
        close(fd);
        return 0;
    }
    return -1;
}

int set_video_axis(int *coordinate)
{
    int fd;
    char *path = "/sys/class/video/axis" ;
    char  bcmd[32];
    int x0, y0, x1, y1;
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        if (coordinate) {
            x0 = coordinate[0];
            y0 = coordinate[1];
            x1 = coordinate[2];
            y1 = coordinate[3];
            sprintf(bcmd, "%d %d %d %d", x0, y0, x1, y1);
            write(fd, bcmd, strlen(bcmd));
        }
        close(fd);
        return 0;
    }
    return -1;
}

int set_fb0_scale_width(int width)
{
    int fd;
    char *path = "/sys/class/graphics/fb0/scale_width" ;
    char  bcmd[16];
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        sprintf(bcmd, "%d", width);
        write(fd, bcmd, strlen(bcmd));
        close(fd);
        return 0;
    }
    return -1;
}
int set_fb0_scale_height(int height)
{
    int fd;
    char *path = "/sys/class/graphics/fb0/scale_height" ;
    char  bcmd[16];
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        sprintf(bcmd, "%d", height);
        write(fd, bcmd, strlen(bcmd));
        close(fd);
        return 0;
    }
    return -1;
}
int set_fb1_scale_width(int width)
{
    int fd;
    char *path = "/sys/class/graphics/fb1/scale_width" ;
    char  bcmd[16];
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        sprintf(bcmd, "%d", width);
        write(fd, bcmd, strlen(bcmd));
        close(fd);
        return 0;
    }
    return -1;
}
int set_fb1_scale_height(int height)
{
    int fd;
    char *path = "/sys/class/graphics/fb1/scale_height" ;
    char  bcmd[16];
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        sprintf(bcmd, "%d", height);
        write(fd, bcmd, strlen(bcmd));
        close(fd);
        return 0;
    }
    return -1;
}

static int display_mode_convert(char *disp_mode)
{
    int ret = 0xff;
    log_print("[display_mode_convert]disp_mode=%s\n", disp_mode);
    if (!disp_mode) {
        ret = 0xeeee;
    } else if (!strncmp(disp_mode, "480i", 4)) {
        ret = DISP_MODE_480I;
    } else if (!strncmp(disp_mode, "480p", 4)) {
        ret = DISP_MODE_480P;
    } else if (!strncmp(disp_mode, "576i", 4)) {
        ret = DISP_MODE_576I;
    } else if (!strncmp(disp_mode, "576p", 4)) {
        ret = DISP_MODE_576P;
    } else if (!strncmp(disp_mode, "720p", 4)) {
        ret = DISP_MODE_720P;
    } else if (!strncmp(disp_mode, "1080i", 5)) {
        ret = DISP_MODE_1080I;
    } else if (!strncmp(disp_mode, "1080p", 5)) {
        ret = DISP_MODE_1080P;
    } else {
        ret = 0xffff;
    }
    log_print("[display_mode_convert]disp_mode=%s-->%x\n", disp_mode, ret);
    return ret;
}
//////////////////////////////////////////////
static void get_display_axis()
{
    int fd;
    int discontinue = 0;
    char *path = "/sys/class/display/axis";
    char  bcmd[32];
    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        read(fd, bcmd, sizeof(bcmd));
        bcmd[31] = '\0';
        log_print("[get_disp_axis]%s\n", bcmd);
        close(fd);
    } else {
        log_error("[%s:%d]open %s failed!\n", __FUNCTION__, __LINE__, path);
    }
}
static void get_video_axis()
{
    int fd;
    int discontinue = 0;
    char *path = "/sys/class/video/axis";
    char  bcmd[32];
    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        read(fd, bcmd, sizeof(bcmd));
        bcmd[31] = '\0';
        log_print("[get_video_axis]%sn", bcmd);
        close(fd);
    } else {
        log_error("[%s:%d]open %s failed!\n", __FUNCTION__, __LINE__, path);
    }
}
//////////////////////////////////////////////
struct fb_var_screeninfo vinfo;
void update_freescale_setting(void)
{
    int fd_fb;
    int osd_width, osd_height;
    int i;
    int num = sizeof(freescale_setting) / sizeof(freescale_setting[0]);

    if ((fd_fb = open("/dev/graphics/fb0", O_RDWR)) < 0) {
        log_error("open /dev/graphics/fb0 fail.");
        return;
    }

    if (ioctl(fd_fb, FBIOGET_VSCREENINFO, &vinfo) == 0) {
        osd_width = vinfo.xres;
        osd_height = vinfo.yres;
        log_print("osd_width = %d", osd_width);
        log_print("osd_height = %d", osd_height);

        for (i = 0; i < num; i ++) {
            if (freescale_setting[i].disp_mode == DISP_MODE_480P) {
                //freescale_setting[i].osd_disble_coordinate[0] = 0;
                //freescale_setting[i].osd_disble_coordinate[1] = 0;
                freescale_setting[i].osd_disble_coordinate[2] = osd_width;
                freescale_setting[i].osd_disble_coordinate[3] = osd_height;
                //freescale_setting[i].osd_disble_coordinate[4] = 0;
                //freescale_setting[i].osd_disble_coordinate[5] = 0;
                //freescale_setting[i].osd_disble_coordinate[6] = 18;
                //freescale_setting[i].osd_disble_coordinate[7] = 18;

            } else if (freescale_setting[i].disp_mode == DISP_MODE_720P) {
                freescale_setting[i].osd_disble_coordinate[0] = (1280 - osd_width) / 2;
                freescale_setting[i].osd_disble_coordinate[1] = (720 - osd_height) / 2;
                freescale_setting[i].osd_disble_coordinate[2] = osd_width;
                freescale_setting[i].osd_disble_coordinate[3] = osd_height;
                freescale_setting[i].osd_disble_coordinate[4] = (1280 - osd_width) / 2;
                freescale_setting[i].osd_disble_coordinate[5] = (720 - osd_height) / 2;
                //freescale_setting[i].osd_disble_coordinate[6] = 18;
                //freescale_setting[i].osd_disble_coordinate[7] = 18;

            } else if (freescale_setting[i].disp_mode == DISP_MODE_1080I ||
                       freescale_setting[i].disp_mode == DISP_MODE_1080P) {
                freescale_setting[i].osd_disble_coordinate[0] = (1920 - osd_width) / 2;
                freescale_setting[i].osd_disble_coordinate[1] = (1080 - osd_height) / 2;
                freescale_setting[i].osd_disble_coordinate[2] = osd_width;
                freescale_setting[i].osd_disble_coordinate[3] = osd_height;
                freescale_setting[i].osd_disble_coordinate[4] = (1920 - osd_width) / 2;
                freescale_setting[i].osd_disble_coordinate[5] = (1080 - osd_height) / 2;
                //freescale_setting[i].osd_disble_coordinate[6] = 18;
                //freescale_setting[i].osd_disble_coordinate[7] = 18;

            }

            //freescale_setting[i].osd_enable_coordinate[0] = 0;
            //freescale_setting[i].osd_enable_coordinate[1] = 0;
            freescale_setting[i].osd_enable_coordinate[2] = osd_width;
            freescale_setting[i].osd_enable_coordinate[3] = osd_height;
            //freescale_setting[i].osd_enable_coordinate[4] = 0;
            //freescale_setting[i].osd_enable_coordinate[5] = 0;
            //freescale_setting[i].osd_enable_coordinate[6] = 18;
            //freescale_setting[i].osd_enable_coordinate[6] = 18;

            freescale_setting[i].fb0_freescale_width = osd_width;
            freescale_setting[i].fb0_freescale_height = osd_height;
            freescale_setting[i].fb1_freescale_width = osd_width;
            freescale_setting[i].fb1_freescale_height = osd_height;
        }

    } else {
        log_error("get FBIOGET_VSCREENINFO fail.");
    }

    close(fd_fb);
    return;
}

/* 
 * Sync from HdmiSwitch/jni/hdmiswitchjni.c
 */
#define  FBIOPUT_OSD_FREE_SCALE_ENABLE	0x4504
#define  FBIOPUT_OSD_FREE_SCALE_WIDTH	0x4505
#define  FBIOPUT_OSD_FREE_SCALE_HEIGHT	0x4506

struct fb_var_screeninfo vinfo;
char daxis_str[32];

int DisableFreeScale(display_mode mode) {
	int fd0 = -1, fd1 = -1;
	int fd_daxis = -1, fd_vaxis = -1;
	int fd_ppmgr = -1,fd_ppmgr_rect = -1;
	int fd_video = -1;
	int osd_width = 0, osd_height = 0;	
	int ret = -1;
	
	//log_print("DisableFreeScale: mode=%d", mode);
	if (mode < DISP_MODE_480I || mode > DISP_MODE_1080P)
		return 0;
		
	if((fd0 = open("/dev/graphics/fb0", O_RDWR)) < 0) {
		log_print("open /dev/graphics/fb0 fail.");
		goto exit;
	}
	if((fd1 = open("/dev/graphics/fb1", O_RDWR)) < 0) {
		log_print("open /dev/graphics/fb1 fail.");
		goto exit;	
	}
		
	if((fd_daxis = open("/sys/class/display/axis", O_RDWR)) < 0) {
		log_print("open /sys/class/display/axis fail.");
		goto exit;
	}

	if((fd_ppmgr = open("/sys/class/ppmgr/ppscaler", O_RDWR)) < 0) {
		log_print("open /sys/class/ppmgr/ppscaler fail.");	
	}

	if((fd_ppmgr_rect = open("/sys/class/ppmgr/ppscaler_rect", O_RDWR)) < 0) {
		log_print("open /sys/class/ppmgr/ppscaler_rect fail.");	
	}

	if((fd_video = open("/sys/class/video/disable_video", O_RDWR)) < 0) {
		log_print("open /sys/class/video/disable_video fail.");
	}

	if((fd_vaxis = open("/sys/class/video/axis", O_RDWR)) < 0) {
		log_print("open /sys/class/video/axis fail.");
	}

	memset(daxis_str,0,32);	
	if(ioctl(fd0, FBIOGET_VSCREENINFO, &vinfo) == 0) {
		osd_width = vinfo.xres;
		osd_height = vinfo.yres;

		//log_print("osd_width = %d", osd_width);
		//log_print("osd_height = %d", osd_height);
	} else {
		log_print("get FBIOGET_VSCREENINFO fail.");
		goto exit;
	}
		
	switch(mode) {
		//log_print("set mid mode=%d", mode);

		case DISP_MODE_480P: //480p		
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "0", strlen("0"));
			if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));		
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);			
			sprintf(daxis_str, "0 0 %d %d 0 0 18 18", vinfo.xres, vinfo.yres);
			write(fd_daxis, daxis_str, strlen(daxis_str));		
			if(fd_ppmgr_rect >= 0)
				write(fd_ppmgr_rect, "0 0 0 0 1", strlen("0 0 0 0 1"));
			if(fd_vaxis >= 0) write(fd_vaxis, "0 0 0 0", strlen("0 0 0 0"));
			ret = 0;
			break;
		case DISP_MODE_720P: //720p
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "0", strlen("0"));
			if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));	
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			sprintf(daxis_str, "%d %d %d %d %d %d 18 18", 1280>vinfo.xres ? (1280-vinfo.xres)/2 : 0, 
				720>vinfo.yres ? (720-vinfo.yres)/2 : 0,
				vinfo.xres, 
				vinfo.yres,
				1280>vinfo.xres ? (1280-vinfo.xres)/2 : 0,
				720>vinfo.yres ? (720-vinfo.yres)/2 : 0);
			write(fd_daxis, daxis_str, strlen(daxis_str));
			if(fd_ppmgr_rect >= 0)
				write(fd_ppmgr_rect, "0 0 0 0 1", strlen("0 0 0 0 1"));
			if(fd_vaxis >= 0) write(fd_vaxis, "0 0 0 0", strlen("0 0 0 0"));
			ret = 0;
			break;
		case DISP_MODE_1080I: //1080i			
		case DISP_MODE_1080P: //1080p
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "0", strlen("0"));
			if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			sprintf(daxis_str, "%d %d %d %d %d %d 18 18", 1920>vinfo.xres ? (1920-vinfo.xres)/2 : 0, 
				1080>vinfo.yres ? (1080-vinfo.yres)/2 : 0,
				vinfo.xres, 
				vinfo.yres,
				1920>vinfo.xres ? (1920-vinfo.xres)/2 : 0,
				1080>vinfo.yres ? (1080-vinfo.yres)/2 : 0);
			write(fd_daxis, daxis_str, strlen(daxis_str));	
			if(fd_ppmgr_rect >= 0)
				write(fd_ppmgr_rect, "0 0 0 0 1", strlen("0 0 0 0 1"));
			if(fd_vaxis >= 0) write(fd_vaxis, "0 0 0 0", strlen("0 0 0 0"));
			ret = 0;
			break;	
		default:			
			break;					
	}	
	
exit:	
	close(fd0);
	close(fd1);
	close(fd_daxis);
	close(fd_vaxis);
	close(fd_ppmgr);
	close(fd_video);
	close(fd_ppmgr_rect);
	return ret;;

}

int EnableFreeScale(display_mode mode) {
	int fd0 = -1, fd1 = -1;
	int fd_daxis = -1, fd_vaxis = -1;
	int fd_ppmgr = -1,fd_ppmgr_rect = -1;
	int fd_video = -1;
 	int osd_width = 0, osd_height = 0;	
	int ret = -1;
	
	//log_print("EnableFreeScale: mode=%d", mode);	
	if (mode < DISP_MODE_480I || mode > DISP_MODE_1080P)
		return 0;	
		
	if((fd0 = open("/dev/graphics/fb0", O_RDWR)) < 0) {
		log_print("open /dev/graphics/fb0 fail.");
		goto exit;
	}
	if((fd1 = open("/dev/graphics/fb1", O_RDWR)) < 0) {
		log_print("open /dev/graphics/fb1 fail.");
		goto exit;	
	}
	if((fd_vaxis = open("/sys/class/video/axis", O_RDWR)) < 0) {
		log_print("open /sys/class/video/axis fail.");
		goto exit;		
	}
		
	if((fd_daxis = open("/sys/class/display/axis", O_RDWR)) < 0) {
		log_print("open /sys/class/display/axis fail.");
		goto exit;
	}

	if((fd_video = open("/sys/class/video/disable_video", O_RDWR)) < 0) {
		log_print("open /sys/class/video/disable_video fail.");
	}

	if((fd_ppmgr = open("/sys/class/ppmgr/ppscaler", O_RDWR)) < 0) {
		log_print("open /sys/class/ppmgr/ppscaler fail.");	
	}

	if((fd_ppmgr_rect = open("/sys/class/ppmgr/ppscaler_rect", O_RDWR)) < 0) {
		log_print("open /sys/class/ppmgr/ppscaler_rect fail.");	
	}

	memset(daxis_str,0,32);	
	if(ioctl(fd0, FBIOGET_VSCREENINFO, &vinfo) == 0) {
		osd_width = vinfo.xres;
		osd_height = vinfo.yres;
		sprintf(daxis_str, "0 0 %d %d 0 0 18 18", vinfo.xres, vinfo.yres);
		
		//log_print("osd_width = %d", osd_width);
		//log_print("osd_height = %d", osd_height);
	} else {
		log_print("get FBIOGET_VSCREENINFO fail.");
		goto exit;
	}
		
	switch(mode) {
		//log_print("set mid mode=%d", mode);

		case DISP_MODE_480P: //480p				
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "1", strlen("1"));
			if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));
			if(fd_ppmgr_rect >= 0){
				write(fd_ppmgr_rect, "20 10 700 470 0", strlen("20 10 700 470 0"));
				if(fd_vaxis>=0) write(fd_vaxis, "0 0 0 0", strlen("0 0 0 0"));				
			}else if(fd_vaxis >= 0){
				write(fd_vaxis, "20 10 700 470", strlen("20 10 700 470"));
			}
			write(fd_daxis, daxis_str, strlen(daxis_str));
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_WIDTH,osd_width);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_HEIGHT,osd_height); 
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_WIDTH,osd_width);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_HEIGHT,osd_height);	
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
			if ((fd_video >= 0)&&(fd_ppmgr >= 0)) 	write(fd_video, "2", strlen("2"));			
			ret = 0;
			break;
		case DISP_MODE_720P: //720p
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "1", strlen("1"));
			if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));
			if(fd_ppmgr_rect >= 0){
				write(fd_ppmgr_rect, "40 15 1240 705 0", strlen("40 15 1240 705 0"));
				if(fd_vaxis>=0) write(fd_vaxis, "0 0 0 0", strlen("0 0 0 0"));				
			}else if(fd_vaxis >= 0){
				write(fd_vaxis, "40 15 1240 705", strlen("40 15 1240 705"));
			}
			write(fd_daxis, daxis_str, strlen(daxis_str));
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_WIDTH,osd_width);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_HEIGHT,osd_height); 
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_WIDTH,osd_width);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_HEIGHT,osd_height);	
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
			if ((fd_video >= 0)&&(fd_ppmgr >= 0)) 	write(fd_video, "2", strlen("2"));
			ret = 0;
			break;
		case DISP_MODE_1080I: //1080i			
		case DISP_MODE_1080P: //1080p
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "1", strlen("1"));
			if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));
			if(fd_ppmgr_rect >= 0){
				write(fd_ppmgr_rect, "40 20 1880 1060 0", strlen("40 20 1880 1060 0"));
				if(fd_vaxis>=0) write(fd_vaxis, "0 0 0 0", strlen("0 0 0 0"));				
			}else if(fd_vaxis >= 0){
				write(fd_vaxis, "40 20 1880 1060", strlen("40 20 1880 1060"));
			}
			write(fd_daxis, daxis_str, strlen(daxis_str));
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_WIDTH,osd_width);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_HEIGHT,osd_height); 
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_WIDTH,osd_width);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_HEIGHT,osd_height);	
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
			if ((fd_video >= 0)&&(fd_ppmgr >= 0)) 	write(fd_video, "2", strlen("2"));		
			ret = 0;
			break;	
		default:			
			break;					
	}	

exit:	
	close(fd0);
	close(fd1);
	close(fd_vaxis);
	close(fd_daxis);	
	close(fd_ppmgr);
	close(fd_video);
	close(fd_ppmgr_rect);
	return ret;

}

int disable_freescale(int cfg)
{
#ifndef ENABLE_FREE_SCALE
	log_print("ENABLE_FREE_SCALE not define!\n");
	return 0;
#endif
	char mode[16];
	display_mode disp_mode;
	
	get_display_mode(mode);
	if (strncmp(mode, "fail", 4)) { //mode !=fail
		disp_mode = display_mode_convert(mode);
		if (disp_mode >= DISP_MODE_480I && disp_mode <= DISP_MODE_1080P) {
			DisableFreeScale(disp_mode);
		}
	}
	log_print("[disable_freescale]");
	return 0;
}
int enable_freescale(int cfg)
{
#ifndef ENABLE_FREE_SCALE
	log_print("ENABLE_FREE_SCALE not define!\n");
	return 0;
#endif
	char mode[16];
	display_mode disp_mode;
	
	get_display_mode(mode);
	if (strncmp(mode, "fail", 4)) { //mode !=fail
		disp_mode = display_mode_convert(mode);
		if (disp_mode >= DISP_MODE_480I && disp_mode <= DISP_MODE_1080P) {
			EnableFreeScale(disp_mode);
		}
	}
	log_print("[enable_freescale]");
	return 0;
}

/*
int disable_freescale(int cfg)
{
#ifndef ENABLE_FREE_SCALE
	log_print("ENABLE_FREE_SCALE not define!\n");
	return 0;
#endif	
    char mode[16];
    freescale_setting_t *setting;
    display_mode disp_mode;
    int i;
    int num;

    log_print("[disable_freescale]cfg = 0x%x\n", cfg);
    //if(cfg == MID_800_400_FREESCALE)
    {
        log_print("[disable_freescale]mid 800*400, do config...\n");
        get_display_mode(mode);
        log_print("[disable_freescale]display_mode=%s \n", mode);
        if (strncmp(mode, "fail", 4)) { //mode !=fail
            disp_mode = display_mode_convert(mode);
            update_freescale_setting();
            num = sizeof(freescale_setting) / sizeof(freescale_setting[0]);
            log_print("[%s:%d]num=%d\n", __FUNCTION__, __LINE__, num);
            if (disp_mode >= DISP_MODE_480I && disp_mode <= DISP_MODE_1080P) {
                for (i = 0; i < num; i ++) {
                    if (disp_mode == freescale_setting[i].disp_mode) {
                        setting = &freescale_setting[i];
                        break;
                    }
                }
                if (i == num) {
                    log_error("[%s:%d]display_mode [%s:%d] needn't set!\n", __FUNCTION__, __LINE__, mode, disp_mode);
                    return 0;
                }
                set_fb0_freescale(0);
                set_fb1_freescale(0);
                set_display_axis(setting->osd_disble_coordinate);
                log_print("[disable_freescale]mid 800*400 config success!\n");
            } else {
                log_error("[disable_freescale]mid 800*400 config failed, display mode invalid\n");
            }
            return 0;
        } else {
            log_error("[disable_freescale]get display mode failed\n");
        }
    }
    log_print("[disable_freescale]do not need config freescale, exit!");
    return -1;
}

int enable_freescale(int cfg)
{
#ifndef ENABLE_FREE_SCALE
	log_print("ENABLE_FREE_SCALE not define!\n");
	return 0;
#endif
    char mode[16];
    freescale_setting_t *setting;
    display_mode disp_mode;
    int i;
    int num;

    log_print("[enable_freescale]cfg = 0x%x\n", cfg);
    //if(cfg == MID_800_400_FREESCALE)
    {
        log_print("[enable_freescale]mid 800*400, do config...\n");
        get_display_mode(mode);
        log_print("[enable_freescale]display_mode=%s \n", mode);
        if (strncmp(mode, "fail", 4)) {
            disp_mode = display_mode_convert(mode);
            update_freescale_setting();
            num = sizeof(freescale_setting) / sizeof(freescale_setting[0]);
            log_print("[%s:%d]num=%d\n", __FUNCTION__, __LINE__, num);
            if (disp_mode >= DISP_MODE_480I && disp_mode <= DISP_MODE_1080P) {
                for (i = 0; i < num; i ++) {
                    if (disp_mode == freescale_setting[i].disp_mode) {
                        setting = &freescale_setting[i];
                        break;
                    }
                }
                if (i == num) {
                    log_error("[%s:%d]display_mode [%s:%d] needn't set!\n", __FUNCTION__, __LINE__, mode, disp_mode);
                    return 0;
                }
                set_video_axis(setting->video_enablecoordinate);
                set_display_axis(setting->osd_enable_coordinate);
                set_fb0_freescale(0);
                set_fb1_freescale(0);
                set_fb0_scale_width(setting->fb0_freescale_width);
                set_fb0_scale_height(setting->fb0_freescale_height);
                set_fb1_scale_width(setting->fb1_freescale_width);
                set_fb1_scale_height(setting->fb1_freescale_height);
                set_fb0_freescale(1);
                set_fb1_freescale(1);
                log_print("[enable_freescale]mid 800*400 config success!\n");
            } else {
                log_error("[enable_freescale]mid 800*400 config failed, display mode invalid\n");
            }
            return 0;
        } else {
            log_error("[enable_freescale]get display mode failed\n");
        }
    }
    log_print("[enable_freescale]do not need config freescale, exit!");
    return -1;
}
*/

int set_stb_source_hiu()
{
    int fd;
    char *path = "/sys/class/stb/source";
    char  bcmd[16];
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        sprintf(bcmd, "%s", "hiu");
        write(fd, bcmd, strlen(bcmd));
        close(fd);
        return 0;
    }
    return -1;
}

int set_stb_demux_source_hiu()
{
    int fd;
    char *path = "/sys/class/stb/demux1_source"; // use demux0 for record, and demux1 for playback
    char  bcmd[16];
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        sprintf(bcmd, "%s", "hiu");
        write(fd, bcmd, strlen(bcmd));
        close(fd);
        return 0;
    }
    return -1;
}
