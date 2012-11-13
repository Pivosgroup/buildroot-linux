
#define LOG_TAG "amavutils"

#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <strings.h>
#include <cutils/log.h>
#include <sys/ioctl.h>
#include "include/Amvideoutils.h"
#include "include/Amsysfsutils.h"
#include "include/Amdisplayutils.h"

#include "amports/amstream.h"
#include "ppmgr/ppmgr.h"


#define SYSCMD_BUFSIZE 40
#define DISP_DEVICE_PATH "/sys/class/video/device_resolution"
#define FB_DEVICE_PATH   "/sys/class/graphics/fb0/virtual_size"
#define ANGLE_PATH       "/dev/ppmgr"
#define VIDEO_PATH       "/dev/amvideo"
#define VIDEO_GLOBAL_OFFSET_PATH "/sys/class/video/global_offset"
#define FREE_SCALE_PATH  "/sys/class/graphics/fb0/free_scale"

static int rotation = 0;
static int disp_width = 1920;
static int disp_height = 1080;

//#define LOG_FUNCTION_NAME LOGI("%s-%d\n",__FUNCTION__,__LINE__);
#define LOG_FUNCTION_NAME


int  amvideo_utils_get_global_offset(void)
{
    LOG_FUNCTION_NAME
    int offset = 0;
    char buf[SYSCMD_BUFSIZE];
    int ret;
    ret = amsysfs_get_sysfs_str(VIDEO_GLOBAL_OFFSET_PATH, buf, SYSCMD_BUFSIZE);
    if (ret < 0) {
        return offset;
    }
    if (sscanf(buf, "%d", &offset) == 1) {
        LOGI("video global_offset %d\n", offset);
    }
    return offset;
}

int amvideo_utils_set_virtual_position(int32_t x, int32_t y, int32_t w, int32_t h, int rotation)
{
    LOG_FUNCTION_NAME
    int video_fd;
    int dev_fd = -1, dev_w, dev_h, disp_w, disp_h, video_global_offset;
    int dst_x, dst_y, dst_w, dst_h;
    char buf[SYSCMD_BUFSIZE];
    int angle_fd = -1;
    int ret = -1;
    int axis[4];

    LOGI("amvideo_utils_set_virtual_position:: x=%d y=%d w=%d h=%d\n", x, y, w, h);

    bzero(buf, SYSCMD_BUFSIZE);

    dst_x = x;
    dst_y = y;
    dst_w = w;
    dst_h = h;

    video_fd = open(VIDEO_PATH, O_RDWR);
    if (video_fd < 0) {
        goto OUT;
    }

    dev_fd = open(DISP_DEVICE_PATH, O_RDONLY);
    if (dev_fd < 0) {
        goto OUT;
    }

    read(dev_fd, buf, SYSCMD_BUFSIZE);

    if (sscanf(buf, "%dx%d", &dev_w, &dev_h) == 2) {
        LOGI("device resolution %dx%d\n", dev_w, dev_h);
    } else {
        ret = -2;
        goto OUT;
    }

    amdisplay_utils_get_size(&disp_w, &disp_h);
    video_global_offset = amvideo_utils_get_global_offset();

    /* if we are doing video output to a second display device with
     * a different resolution, scale all the numbers.
     * E.g. when a MID pad is connected to a HDMI output.
     */
    if (((disp_w != dev_w) || (disp_h / 2 != dev_h)) &&
        (video_global_offset == 0)) {
        char val[256];
        int free_scale_enable = 0;

        memset(val, 0, sizeof(val));
        if (amsysfs_get_sysfs_str(FREE_SCALE_PATH, val, sizeof(val)) == 0) {
            /* the returned string should be "free_scale_enable:[0x%x]" */
            free_scale_enable = (val[21] == '0') ? 0 : 1;
        }

        if (free_scale_enable == 0) {
            dst_x = dst_x * dev_w / disp_w;
            dst_y = dst_y * dev_h / disp_h;
            dst_w = dst_w * dev_w / disp_w;
            dst_h = dst_h * dev_h / disp_h;
        }
    }

    angle_fd = open(ANGLE_PATH, O_WRONLY);
    if (angle_fd >= 0) {
        ioctl(angle_fd, PPMGR_IOC_SET_ANGLE, (rotation/90) & 3);
        LOGI("set ppmgr angle %d\n", (rotation/90) & 3);
    }

    /* this is unlikely and only be used when ppmgr does not exist
     * to support video rotation. If that happens, we convert the window
     * position to non-rotated window position.
     * On ICS, this might not work at all because the transparent UI
     * window is still drawn is it's direction, just comment out this for now.
     */
#if 0
    if (((rotation == 90) || (rotation == 270)) && (angle_fd < 0)) {
        if (dst_h == disp_h) {
            int center = x + w / 2;

            if (abs(center - disp_w / 2) < 2) {
                /* a centered overlay with rotation, change to full screen */
                dst_x = 0;
                dst_y = 0;
                dst_w = dev_w;
                dst_h = dev_h;

                LOGI("centered overlay expansion");
            }
        }
    }
#endif

    axis[0] = dst_x;
    axis[1] = dst_y;
    axis[2] = dst_x + dst_w - 1;
    axis[3] = dst_y + dst_h - 1;
    
    ioctl(video_fd, AMSTREAM_IOC_SET_VIDEO_AXIS, &axis[0]);

    ret = 0;
OUT:
    if (video_fd >= 0) {
        close(video_fd);
    }
    
    if (dev_fd >= 0) {
        close(dev_fd);
    }
    
    if (angle_fd >= 0) {
        close(angle_fd);
    }
    LOGI("amvideo_utils_set_virtual_position (corrected):: x=%d y=%d w=%d h=%d\n", dst_x, dst_y, dst_w, dst_h);

    return ret;
}

int amvideo_utils_set_absolute_position(int32_t x, int32_t y, int32_t w, int32_t h, int rotation)
{
    LOG_FUNCTION_NAME
    int video_fd;
    int angle_fd = -1;
    int axis[4];

    LOGI("amvideo_utils_set_absolute_position:: x=%d y=%d w=%d h=%d\n", x, y, w, h);

    video_fd = open(VIDEO_PATH, O_RDWR);
    if (video_fd < 0) {
        return -1;
    }

    angle_fd = open(ANGLE_PATH, O_WRONLY);
    if (angle_fd >= 0) {
        ioctl(angle_fd, PPMGR_IOC_SET_ANGLE, (rotation/90) & 3);
        LOGI("set ppmgr angle %d\n", (rotation/90) & 3);
        close(angle_fd);
    }

    axis[0] = x;
    axis[1] = y;
    axis[2] = x + w - 1;
    axis[3] = y + h - 1;
    
    ioctl(video_fd, AMSTREAM_IOC_SET_VIDEO_AXIS, &axis[0]);

    close(video_fd);

    return 0;
}

int amvideo_utils_get_position(int32_t *x, int32_t *y, int32_t *w, int32_t *h)
{
    LOG_FUNCTION_NAME
    int video_fd;
    int axis[4];

    video_fd = open(VIDEO_PATH, O_RDWR);
    if (video_fd < 0) {
        return -1;
    }

    ioctl(video_fd, AMSTREAM_IOC_GET_VIDEO_AXIS, &axis[0]);

    close(video_fd);

    *x = axis[0];
    *y = axis[1];
    *w = axis[2] - axis[0] + 1;
    *h = axis[3] - axis[1] + 1;

    return 0;
}

int amvideo_utils_get_screen_mode(int *mode)
{
    LOG_FUNCTION_NAME
    int video_fd;
    int screen_mode = 0;

    video_fd = open(VIDEO_PATH, O_RDWR);
    if (video_fd < 0) {
        return -1;
    }

    ioctl(video_fd, AMSTREAM_IOC_GET_SCREEN_MODE, &screen_mode);

    close(video_fd);

    *mode = screen_mode;

    return 0;
}

int amvideo_utils_set_screen_mode(int mode)
{
    LOG_FUNCTION_NAME
    int screen_mode = mode;
    int video_fd;

    video_fd = open(VIDEO_PATH, O_RDWR);
    if (video_fd < 0) {
        return -1;
    }

    ioctl(video_fd, AMSTREAM_IOC_SET_SCREEN_MODE, &screen_mode);

    close(video_fd);

    return 0;
}

