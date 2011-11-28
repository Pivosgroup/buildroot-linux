

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <log_print.h>
#include "systemsetting.h"

int PlayerSettingIsEnable(const char* path)
{
    char value[1024];
    if (GetSystemSettingString(path, value, NULL) > 0) {
        if ((!strcmp(value, "1") || !strcmp(value, "true") || !strcmp(value, "enable"))) {
            log_print("%s is enabled\n", path);
            return 1;
        }
    }
    log_print("%s is disabled\n", path);
    return 0;
}


float PlayerGetSettingfloat(const char* path)
{
    char value[1024];
    float ret = 0.0;
    if (GetSystemSettingString(path, value, NULL) > 0) {
        if ((sscanf(value, "%f", &ret)) > 0) {
            log_print("%s is set to %f\n", path, ret);
            return ret;
        }
    }
    log_print("%s is not set\n", path);
    return ret;
}

#define FILTER_VFMT_MPEG12	(1 << 0)
#define FILTER_VFMT_MPEG4	(1 << 1)
#define FILTER_VFMT_H264	(1 << 2)
#define FILTER_VFMT_MJPEG	(1 << 3)
#define FILTER_VFMT_REAL	(1 << 4)
#define FILTER_VFMT_JPEG	(1 << 5)
#define FILTER_VFMT_VC1		(1 << 6)
#define FILTER_VFMT_AVS		(1 << 7)
#define FILTER_VFMT_SW		(1 << 8)

int PlayerGetVFilterFormat(const char* path)
{
	char value[1024];
	int filter_fmt = 0;
	
	log_print("[%s:%d]path=%s\n", __FUNCTION__, __LINE__, path);
	
    if (GetSystemSettingString(path, value, NULL) > 0) {
		log_print("[%s:%d]disable_vdec=%s\n", __FUNCTION__, __LINE__, value);
		if (strstr(value,"MPEG12") != NULL) {
			filter_fmt |= FILTER_VFMT_MPEG12;
		} 
		if (strstr(value,"MPEG4") != NULL) {
			filter_fmt |= FILTER_VFMT_MPEG4;
		} 
		if (strstr(value,"H264") != NULL) {
			filter_fmt |= FILTER_VFMT_H264;
		} 
		if (strstr(value,"MJPEG") != NULL) {
			filter_fmt |= FILTER_VFMT_MJPEG;
		} 
		if (strstr(value,"REAL") != NULL) {
			filter_fmt |= FILTER_VFMT_REAL;
		} 
		if (strstr(value,"JPEG") != NULL) {
			filter_fmt |= FILTER_VFMT_JPEG;
		} 
		if (strstr(value,"VC1") != NULL) {
			filter_fmt |= FILTER_VFMT_VC1;
		} 
		if (strstr(value,"AVS") != NULL) {
			filter_fmt |= FILTER_VFMT_AVS;
		} 
		if (strstr(value,"SW") != NULL) {
			filter_fmt |= FILTER_VFMT_SW;
		}
    }
	log_print("[%s:%d]filter_vfmt=%x\n", __FUNCTION__, __LINE__, filter_fmt);
    return filter_fmt;
}
