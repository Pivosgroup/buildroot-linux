

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

int PlayerGetVFilterFormat()
{
	char value[1024];
	int filter_fmt = 0;
	
    if (GetSystemSettingString("media.amplayer.disable-vcodecs", value, NULL) > 0) {
		log_print("[%s:%d]disable_vdec=%s\n", __FUNCTION__, __LINE__, value);
		if (strstr(value,"MPEG12") != NULL || strstr(value,"mpeg12") != NULL) {
			filter_fmt |= FILTER_VFMT_MPEG12;
		} 
		if (strstr(value,"MPEG4") != NULL || strstr(value,"mpeg4") != NULL) {
			filter_fmt |= FILTER_VFMT_MPEG4;
		} 
		if (strstr(value,"H264") != NULL || strstr(value,"h264") != NULL) {
			filter_fmt |= FILTER_VFMT_H264;
		} 
		if (strstr(value,"MJPEG") != NULL || strstr(value,"mjpeg") != NULL) {
			filter_fmt |= FILTER_VFMT_MJPEG;
		} 
		if (strstr(value,"REAL") != NULL || strstr(value,"real") != NULL) {
			filter_fmt |= FILTER_VFMT_REAL;
		} 
		if (strstr(value,"JPEG") != NULL || strstr(value,"jpeg") != NULL) {
			filter_fmt |= FILTER_VFMT_JPEG;
		} 
		if (strstr(value,"VC1") != NULL || strstr(value,"vc1") != NULL) {
			filter_fmt |= FILTER_VFMT_VC1;
		} 
		if (strstr(value,"AVS") != NULL || strstr(value,"avs") != NULL) {
			filter_fmt |= FILTER_VFMT_AVS;
		} 
		if (strstr(value,"SW") != NULL || strstr(value,"sw") != NULL) {
			filter_fmt |= FILTER_VFMT_SW;
		}
    }
	log_print("[%s:%d]filter_vfmt=%x\n", __FUNCTION__, __LINE__, filter_fmt);
    return filter_fmt;
}

#define FILTER_AFMT_MPEG		(1 << 0)
#define FILTER_AFMT_PCMS16L	    (1 << 1)
#define FILTER_AFMT_AAC			(1 << 2)
#define FILTER_AFMT_AC3			(1 << 3)
#define FILTER_AFMT_ALAW		(1 << 4)
#define FILTER_AFMT_MULAW		(1 << 5)
#define FILTER_AFMT_DTS			(1 << 6)
#define FILTER_AFMT_PCMS16B		(1 << 7)
#define FILTER_AFMT_FLAC		(1 << 8)
#define FILTER_AFMT_COOK		(1 << 9)
#define FILTER_AFMT_PCMU8		(1 << 10)
#define FILTER_AFMT_ADPCM		(1 << 11)
#define FILTER_AFMT_AMR			(1 << 12)
#define FILTER_AFMT_RAAC		(1 << 13)
#define FILTER_AFMT_WMA			(1 << 14)
#define FILTER_AFMT_WMAPRO		(1 << 15)
#define FILTER_AFMT_PCMBLU		(1 << 16)
#define FILTER_AFMT_ALAC		(1 << 17)
#define FILTER_AFMT_VORBIS		(1 << 18)

int PlayerGetAFilterFormat()
{
	char value[1024];
	int filter_fmt = 0;	
	
    if (GetSystemSettingString("media.amplayer.disable-acodecs", value, NULL) > 0) {
		log_print("[%s:%d]disable_adec=%s\n", __FUNCTION__, __LINE__, value);
		if (strstr(value,"mpeg") != NULL || strstr(value,"MPEG") != NULL) {
			filter_fmt |= FILTER_AFMT_MPEG;
		} 
		if (strstr(value,"pcms16l") != NULL || strstr(value,"PCMS16L") != NULL) {
			filter_fmt |= FILTER_AFMT_PCMS16L;
		} 
		if (strstr(value,"aac") != NULL || strstr(value,"AAC") != NULL) {
			filter_fmt |= FILTER_AFMT_AAC;
		} 
		if (strstr(value,"ac3") != NULL || strstr(value,"AC#") != NULL) {
			filter_fmt |= FILTER_AFMT_AC3;
		} 
		if (strstr(value,"alaw") != NULL || strstr(value,"ALAW") != NULL) {
			filter_fmt |= FILTER_AFMT_ALAW;
		} 
		if (strstr(value,"mulaw") != NULL || strstr(value,"MULAW") != NULL) {
			filter_fmt |= FILTER_AFMT_MULAW;
		} 
		if (strstr(value,"dts") != NULL || strstr(value,"DTS") != NULL) {
			filter_fmt |= FILTER_AFMT_DTS;
		} 
		if (strstr(value,"pcms16b") != NULL || strstr(value,"PCMS16B") != NULL) {
			filter_fmt |= FILTER_AFMT_PCMS16B;
		} 
		if (strstr(value,"flac") != NULL || strstr(value,"FLAC") != NULL) {
			filter_fmt |= FILTER_AFMT_FLAC;
		}
		if (strstr(value,"cook") != NULL || strstr(value,"COOK") != NULL) {
			filter_fmt |= FILTER_AFMT_COOK;
		} 
		if (strstr(value,"pcmu8") != NULL || strstr(value,"PCMU8") != NULL) {
			filter_fmt |= FILTER_AFMT_PCMU8;
		} 
		if (strstr(value,"adpcm") != NULL || strstr(value,"ADPCM") != NULL) {
			filter_fmt |= FILTER_AFMT_ADPCM;
		} 
		if (strstr(value,"amr") != NULL || strstr(value,"AMR") != NULL) {
			filter_fmt |= FILTER_AFMT_AMR;
		} 
		if (strstr(value,"raac") != NULL || strstr(value,"RAAC") != NULL) {
			filter_fmt |= FILTER_AFMT_RAAC;
		}
		if (strstr(value,"wma") != NULL || strstr(value,"WMA") != NULL) {
			filter_fmt |= FILTER_AFMT_WMA;
		} 
		if (strstr(value,"wmapro") != NULL || strstr(value,"WMAPRO") != NULL) {
			filter_fmt |= FILTER_AFMT_WMAPRO;
		} 
		if (strstr(value,"pcmblueray") != NULL || strstr(value,"PCMBLUERAY") != NULL) {
			filter_fmt |= FILTER_AFMT_PCMBLU;
		} 
		if (strstr(value,"alac") != NULL || strstr(value,"ALAC") != NULL) {
			filter_fmt |= FILTER_AFMT_ALAC;
		} 
		if (strstr(value,"vorbis") != NULL || strstr(value,"VORBIS") != NULL) {
			filter_fmt |= FILTER_AFMT_VORBIS;
		}
    }
	log_print("[%s:%d]filter_afmt=%x\n", __FUNCTION__, __LINE__, filter_fmt);
    return filter_fmt;
}


